/* The T2 TFHE circuit simulator (S12 U3, Q-S12-6 (a)). For each row NAME of
 * ROWS it reads DIR/NAME.jsonl (the `langc tfhe` text; DIR is the argument,
 * default test/tfhe) and test/report/NAME.json (the `langc report` text, T4).
 * Each circuit line must agree with the T4 rule line of the same name and
 * type (classes, min_count, the first min_listed min vectors), have well
 * formed gate rows, accept exactly the member assignments whose class counts
 * reach every m_j of one min vector (all 2^inputs assignments, member bits in
 * class order), and have the depth that its rows give (an input 0, a
 * two-input gate 1 + the larger depth of its inputs, NOT the depth of its
 * input), with depth <= bound. A plain Boolean simulation: no TFHE library. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsonin.h"

/* The tool runs all 2^inputs assignments, 64 in one word. */
#define SIM_MAX_INPUTS 24u
#define ARENA_LIMIT_BYTES ((size_t)1 << 26)
#define PATH_BYTES 4096u

typedef struct {
  const char *name;
  int four_of_eight; /* wide8: each circuit also accepts iff at least 4 of its 8 bits are 1 */
} Row;

static const Row ROWS[] = {
    {"honest", 0}, {"log2", 0}, {"minlast", 0}, {"oneofn", 0}, {"rule", 0}, {"unanimity", 0}, {"wide8", 1},
};

typedef enum { OP_AND, OP_OR, OP_XOR, OP_NAND, OP_NOR, OP_XNOR, OP_NOT, OP_NONE } Op;

static const char *const OP_NAMES[] = {"AND", "OR", "XOR", "NAND", "NOR", "XNOR", "NOT"};

/* One circuit line, read. Wire w < inputs is member bit w; gate g writes
   wire inputs + g. */
typedef struct {
  const char *file;
  const char *name;
  uint32_t k;
  uint32_t *n;
  uint32_t inputs;
  uint32_t *min; /* min_len vectors of k counts */
  uint32_t min_len;
  uint32_t gate_count;
  Op *op;
  uint32_t *a;
  uint32_t *b;
  uint32_t out;
} Circuit;

/* Writes a FAIL line and returns 0. */
static int fail(const char *file, const char *name, const char *what) {
  fprintf(stderr, "FAIL tfhe sim %s %s: %s\n", file, name, what);
  return 0;
}

static Op op_parse(const char *text) {
  uint32_t i;
  for (i = 0; text != NULL && i < (uint32_t)OP_NONE; i++)
    if (strcmp(text, OP_NAMES[i]) == 0)
      return (Op)i;
  return OP_NONE;
}

static uint64_t op_eval(Op op, uint64_t a, uint64_t b) {
  switch (op) {
    case OP_AND:
      return a & b;
    case OP_OR:
      return a | b;
    case OP_XOR:
      return a ^ b;
    case OP_NAND:
      return ~(a & b);
    case OP_NOR:
      return ~(a | b);
    case OP_XNOR:
      return ~(a ^ b);
    case OP_NOT:
      return ~a;
    case OP_NONE:
      return 0u;
  }
  return 0u;
}

/* The value of V when V is a decimal integer in [0, 2^32 - 1]. */
static int get_u32(const JsonValue *v, uint32_t *out) {
  char *end = NULL;
  unsigned long long x;
  if (v == NULL || v->kind != JSON_NUMBER || v->len == 0u || v->text[0] < '0' || v->text[0] > '9')
    return 0;
  x = strtoull(v->text, &end, 10);
  if (end != v->text + v->len || x > 0xFFFFFFFFull)
    return 0;
  *out = (uint32_t)x;
  return 1;
}

static int member_u32(const JsonValue *object, const char *key, uint32_t *out) {
  return get_u32(json_member(object, key), out);
}

/* 1 when member KEY of OBJECT is the string TEXT. */
static int member_is(const JsonValue *object, const char *key, const char *text) {
  const char *got = json_get_string(json_member(object, key));
  return got != NULL && text != NULL && strcmp(got, text) == 0;
}

/* 1 when X and Y are arrays of the same integers. */
static int same_ints(const JsonValue *x, const JsonValue *y) {
  const JsonValue *p;
  const JsonValue *q;
  uint32_t u;
  uint32_t v;
  if (x == NULL || y == NULL || x->kind != JSON_ARRAY || y->kind != JSON_ARRAY || x->count != y->count)
    return 0;
  for (p = x->first, q = y->first; p != NULL && q != NULL; p = p->next, q = q->next)
    if (!get_u32(p, &u) || !get_u32(q, &v) || u != v)
      return 0;
  return 1;
}

/* Reads the file at PATH as JSON Lines into *LINES and *COUNT. Each line,
   the last too, ends with '\n'. Returns 1, or 0 after a FAIL line. */
static int read_lines(Arena *arena, const char *file, const char *path, JsonValue **lines, size_t *count) {
  FILE *f = fopen(path, "rb");
  Diag diag;
  char *text = NULL;
  long size = -1;
  size_t at = 0;
  size_t i;
  diag_init(&diag);
  if (f != NULL && fseek(f, 0L, SEEK_END) == 0)
    size = ftell(f);
  if (size >= 0 && fseek(f, 0L, SEEK_SET) == 0)
    text = arena_alloc(arena, (size_t)size + 1u);
  if (text != NULL && fread(text, 1u, (size_t)size, f) != (size_t)size)
    text = NULL;
  if (f != NULL)
    fclose(f);
  if (text == NULL)
    return fail(file, path, "cannot read the file");
  if (size == 0 || text[size - 1] != '\n')
    return fail(file, path, "the file is empty or does not end with a newline");
  *count = 0;
  for (i = 0; i < (size_t)size; i++)
    *count += text[i] == '\n' ? 1u : 0u;
  *lines = arena_alloc(arena, (*count + 1u) * sizeof **lines);
  if (*lines == NULL)
    return fail(file, path, "out of memory");
  for (i = 0; i < *count; i++) {
    size_t end = at;
    while (text[end] != '\n')
      end++;
    if (!json_parse(arena, text + at, end - at, &(*lines)[i], &diag) || (*lines)[i].kind != JSON_OBJECT)
      return fail(file, path, "a line is not one JSON object");
    at = end + 1u;
  }
  return 1;
}

/* The T4 rule line of NAME and TYPE in T4, or NULL. */
static const JsonValue *t4_line(const JsonValue *t4, size_t t4_count, const char *name, const char *type) {
  size_t i;
  for (i = 0; i < t4_count; i++)
    if (member_is(&t4[i], "kind", "rule") && member_is(&t4[i], "name", name) && member_is(&t4[i], "type", type))
      return &t4[i];
  return NULL;
}

/* Reads the classes and the min vectors of LINE into C and checks them
   against the T4 line. Returns 1, or 0 after a FAIL line. */
static int read_counts(Arena *arena, const JsonValue *line, const JsonValue *t4, uint32_t min_cap, Circuit *c) {
  const JsonValue *classes = json_member(line, "classes");
  const JsonValue *min = json_member(line, "min");
  const JsonValue *t4_min = json_member(t4, "min");
  const JsonValue *p;
  const JsonValue *q;
  uint32_t sum = 0;
  uint32_t min_count;
  uint32_t t4_count;
  uint32_t listed;
  uint32_t i = 0;
  uint32_t j;
  if (!same_ints(classes, json_member(t4, "classes")) || classes->count == 0u)
    return fail(c->file, c->name, "classes are not the T4 classes");
  c->k = (uint32_t)classes->count;
  c->n = arena_alloc(arena, ((size_t)c->k + 1u) * sizeof *c->n);
  if (c->n == NULL)
    return fail(c->file, c->name, "out of memory");
  for (p = classes->first, j = 0; p != NULL; p = p->next, j++) {
    get_u32(p, &c->n[j]);
    sum += c->n[j];
  }
  if (!member_u32(line, "inputs", &c->inputs) || c->inputs != sum)
    return fail(c->file, c->name, "inputs is not the sum of the classes");
  if (c->inputs > SIM_MAX_INPUTS)
    return fail(c->file, c->name, "more than 24 inputs");
  if (!member_u32(line, "min_count", &min_count) || !member_u32(t4, "min_count", &t4_count) || min_count != t4_count)
    return fail(c->file, c->name, "min_count is not the T4 min_count");
  c->min_len = min_count < min_cap ? min_count : min_cap;
  if (min == NULL || min->kind != JSON_ARRAY || min->count != c->min_len)
    return fail(c->file, c->name, "min does not hold min(min_count, min_cap) vectors");
  c->min = arena_alloc(arena, ((size_t)c->min_len * c->k + 1u) * sizeof *c->min);
  if (c->min == NULL)
    return fail(c->file, c->name, "out of memory");
  for (p = min->first; p != NULL; p = p->next, i++) {
    if (p->kind != JSON_ARRAY || p->count != c->k)
      return fail(c->file, c->name, "a min vector does not have k counts");
    for (q = p->first, j = 0; q != NULL; q = q->next, j++)
      if (!get_u32(q, &c->min[(size_t)i * c->k + j]) || c->min[(size_t)i * c->k + j] > c->n[j])
        return fail(c->file, c->name, "a min count is not in [0, n_j]");
  }
  if (!member_u32(t4, "min_listed", &listed) || t4_min == NULL || t4_min->kind != JSON_ARRAY ||
      t4_min->count != listed || listed > c->min_len)
    return fail(c->file, c->name, "the T4 min list does not have min_listed vectors");
  for (p = min->first, q = t4_min->first; q != NULL; p = p->next, q = q->next)
    if (!same_ints(p, q))
      return fail(c->file, c->name, "the first min_listed min vectors are not the T4 min");
  return 1;
}

/* Reads the gate rows of LINE into C: a known op, inputs below the gate's
   own wire, NOT with b = a, and the nots and out fields. Returns 1, or 0
   after a FAIL line. */
static int read_gates(Arena *arena, const JsonValue *line, Circuit *c) {
  const JsonValue *gates = json_member(line, "gates");
  const JsonValue *row;
  uint32_t nots = 0;
  uint32_t want;
  uint32_t g = 0;
  if (gates == NULL || gates->kind != JSON_ARRAY)
    return fail(c->file, c->name, "gates is not an array");
  c->gate_count = (uint32_t)gates->count;
  if (!member_u32(line, "gate_count", &want) || want != c->gate_count)
    return fail(c->file, c->name, "gate_count is not the count of gate rows");
  c->op = arena_alloc(arena, ((size_t)c->gate_count + 1u) * sizeof *c->op);
  c->a = arena_alloc(arena, ((size_t)c->gate_count + 1u) * sizeof *c->a);
  c->b = arena_alloc(arena, ((size_t)c->gate_count + 1u) * sizeof *c->b);
  if (c->op == NULL || c->a == NULL || c->b == NULL)
    return fail(c->file, c->name, "out of memory");
  for (row = gates->first; row != NULL; row = row->next, g++) {
    const JsonValue *op = row->kind == JSON_ARRAY && row->count == 3u ? row->first : NULL;
    if (op == NULL || !get_u32(op->next, &c->a[g]) || !get_u32(op->next->next, &c->b[g]))
      return fail(c->file, c->name, "a gate row is not [op, a, b]");
    c->op[g] = op_parse(json_get_string(op));
    if (c->op[g] == OP_NONE)
      return fail(c->file, c->name, "a gate row has an unknown op");
    if (c->a[g] >= c->inputs + g || c->b[g] >= c->inputs + g)
      return fail(c->file, c->name, "a gate row reads a wire at or after its own");
    if (c->op[g] == OP_NOT && c->b[g] != c->a[g])
      return fail(c->file, c->name, "a NOT row has b != a");
    nots += c->op[g] == OP_NOT ? 1u : 0u;
  }
  if (!member_u32(line, "nots", &want) || want != nots)
    return fail(c->file, c->name, "nots is not the count of NOT rows");
  if (!member_u32(line, "out", &c->out) || c->out >= c->inputs + c->gate_count)
    return fail(c->file, c->name, "out is not a wire of the circuit");
  return 1;
}

/* Checks the depth and bound fields: the depth of the out wire from the
   rows, and depth <= bound. Returns 1, or 0 after a FAIL line. */
static int check_depth(Arena *arena, const JsonValue *line, const Circuit *c) {
  uint32_t *d = arena_alloc(arena, ((size_t)c->gate_count + 1u) * sizeof *d);
  uint32_t depth;
  uint32_t bound;
  uint32_t g;
  if (d == NULL)
    return fail(c->file, c->name, "out of memory");
  for (g = 0; g < c->gate_count; g++) {
    uint32_t da = c->a[g] < c->inputs ? 0u : d[c->a[g] - c->inputs];
    uint32_t db = c->b[g] < c->inputs ? 0u : d[c->b[g] - c->inputs];
    d[g] = c->op[g] == OP_NOT ? da : 1u + (da > db ? da : db);
  }
  if (!member_u32(line, "depth", &depth) || depth != (c->out < c->inputs ? 0u : d[c->out - c->inputs]))
    return fail(c->file, c->name, "depth is not the depth of the rows");
  if (!member_u32(line, "bound", &bound) || depth > bound)
    return fail(c->file, c->name, "depth is above bound");
  return 1;
}

static uint32_t ones(uint32_t x) {
  uint32_t count = 0;
  for (; x != 0u; x &= x - 1u)
    count++;
  return count;
}

/* The oracle for assignment X: 1 when the class counts of X reach every m_j
   of one min vector. */
static int oracle(const Circuit *c, uint32_t x) {
  uint32_t v;
  for (v = 0; v < c->min_len; v++) {
    uint32_t lo = 0;
    uint32_t j;
    int all = 1;
    for (j = 0; j < c->k && all; j++) {
      uint32_t mask = c->n[j] == 0u ? 0u : ((uint32_t)-1 >> (32u - c->n[j])) << lo;
      all = ones(x & mask) >= c->min[(size_t)v * c->k + j];
      lo += c->n[j];
    }
    if (all)
      return 1;
  }
  return 0;
}

/* Runs all 2^inputs assignments, 64 to a word: the out bit must be the
   oracle (and, for FOUR, at least 4 of 8 bits). Returns 1, or 0 after a
   FAIL line. */
static int simulate(Arena *arena, const Circuit *c, int four) {
  static const uint64_t LANE[6] = {0xAAAAAAAAAAAAAAAAull, 0xCCCCCCCCCCCCCCCCull, 0xF0F0F0F0F0F0F0F0ull,
                                   0xFF00FF00FF00FF00ull, 0xFFFF0000FFFF0000ull, 0xFFFFFFFF00000000ull};
  uint64_t *w = arena_alloc(arena, ((size_t)c->inputs + c->gate_count + 1u) * sizeof *w);
  uint32_t lanes = c->inputs < 6u ? 1u << c->inputs : 64u;
  uint32_t blocks = c->inputs < 6u ? 1u : 1u << (c->inputs - 6u);
  uint32_t block;
  if (w == NULL)
    return fail(c->file, c->name, "out of memory");
  if (four && c->inputs != 8u)
    return fail(c->file, c->name, "the 4 of 8 check needs 8 inputs");
  for (block = 0; block < blocks; block++) {
    uint32_t i;
    uint32_t lane;
    for (i = 0; i < c->inputs; i++)
      w[i] = i < 6u ? LANE[i] : (((block >> (i - 6u)) & 1u) != 0u ? ~(uint64_t)0 : 0u);
    for (i = 0; i < c->gate_count; i++)
      w[c->inputs + i] = op_eval(c->op[i], w[c->a[i]], w[c->b[i]]);
    for (lane = 0; lane < lanes; lane++) {
      uint32_t x = block * 64u + lane;
      int got = (int)((w[c->out] >> lane) & 1u);
      if (got != oracle(c, x))
        return fail(c->file, c->name, "the out bit is not the min vector oracle");
      if (four && got != (ones(x) >= 4u))
        return fail(c->file, c->name, "the out bit is not 'at least 4 of 8'");
    }
  }
  return 1;
}

/* Checks one circuit LINE of FILE. Returns 1, or 0 after a FAIL line. */
static int check_circuit(Arena *arena, const char *file, const JsonValue *line, const JsonValue *t4, size_t t4_count,
                         uint32_t min_cap, uint32_t gate_max, int four) {
  Circuit c;
  const JsonValue *rule;
  memset(&c, 0, sizeof c);
  c.file = file;
  c.name = json_get_string(json_member(line, "name"));
  if (c.name == NULL || !member_is(line, "kind", "circuit"))
    return fail(file, "-", "a line after the first is not a named circuit line");
  rule = t4_line(t4, t4_count, c.name, json_get_string(json_member(line, "type")));
  if (rule == NULL)
    return fail(file, c.name, "no T4 rule line has this name and type");
  if (!read_counts(arena, line, rule, min_cap, &c) || !read_gates(arena, line, &c) || !check_depth(arena, line, &c))
    return 0;
  if (c.gate_count > gate_max)
    return fail(file, c.name, "gate_count is above gate_max");
  return simulate(arena, &c, four);
}

/* Writes DIR/NAME.EXT into PATH. */
static int path_of(char *path, const char *dir, const char *name, const char *ext) {
  int len = snprintf(path, PATH_BYTES, "%s/%s%s", dir, name, ext);
  return len > 0 && (unsigned)len < PATH_BYTES;
}

/* Checks the circuits of ROW in DIR and adds their count to *SIMS. Returns
   1, or 0 after a FAIL line. */
static int run(const Row *row, const char *dir, size_t *sims) {
  Arena arena;
  char tfhe_path[PATH_BYTES];
  char t4_path[PATH_BYTES];
  JsonValue *lines = NULL;
  JsonValue *t4 = NULL;
  size_t count = 0;
  size_t t4_count = 0;
  uint32_t circuits = 0;
  uint32_t gate_max = 0;
  uint32_t min_cap = 0;
  size_t i;
  int ok;
  arena_init(&arena, ARENA_LIMIT_BYTES);
  ok = path_of(tfhe_path, dir, row->name, ".jsonl") && path_of(t4_path, "test/report", row->name, ".json");
  ok = ok && read_lines(&arena, row->name, tfhe_path, &lines, &count) &&
       read_lines(&arena, row->name, t4_path, &t4, &t4_count);
  if (ok && (!member_is(&lines[0], "kind", "program") || !member_u32(&lines[0], "circuits", &circuits) ||
             circuits + 1u != count || !member_u32(&lines[0], "gate_max", &gate_max) ||
             !member_u32(&lines[0], "min_cap", &min_cap)))
    ok = fail(row->name, "-", "the first line is not the program line of the circuit lines");
  for (i = 1; ok && i < count; i++) {
    ok = check_circuit(&arena, row->name, &lines[i], t4, t4_count, min_cap, gate_max, row->four_of_eight);
    *sims += 1u;
  }
  arena_release(&arena);
  return ok;
}

int main(int argc, char **argv) {
  const char *dir = argc > 1 ? argv[1] : "test/tfhe";
  size_t count = sizeof ROWS / sizeof ROWS[0];
  size_t sims = 0;
  size_t passed = 0;
  size_t i;
  for (i = 0; i < count; i++)
    passed += (size_t)run(&ROWS[i], dir, &sims);
  if (passed != count)
    return 1;
  printf("tfhe sims: %u checked\n", (unsigned)sims);
  return 0;
}
