/* S12 U1: the T2 TFHE circuit builder and the depth pass (tfhe.h). */
#include <stdint.h>
#include <string.h>

#include "front/tfhe.h"

/* The constant wires (no row reads one) and the unset comparator slot. */
#define TFHE_K0 0xFFFFFFFEu
#define TFHE_K1 0xFFFFFFFFu
#define TFHE_UNSET 0xFFFFFFFDu
/* b(n) for a uint32_t n is at most 32. */
#define TFHE_BITS 32u

/* The gate table under build. GATES is NULL in the count pass; OVER marks
   a gate past TFHE_GATE_MAX. */
typedef struct {
  TfheGate *gates;
  uint32_t inputs;
  uint32_t count;
  int over;
} TfheBuild;

int tfhe_op_eval(TfheOp op, int a, int b) {
  switch (op) {
    case TFHE_AND:
      return a & b;
    case TFHE_OR:
      return a | b;
    case TFHE_XOR:
      return a ^ b;
    case TFHE_NAND:
      return 1 - (a & b);
    case TFHE_NOR:
      return 1 - (a | b);
    case TFHE_XNOR:
      return 1 - (a ^ b);
    case TFHE_NOT:
      return 1 - a;
  }
  return 0;
}

const char *tfhe_op_name(TfheOp op) {
  switch (op) {
    case TFHE_AND:
      return "AND";
    case TFHE_OR:
      return "OR";
    case TFHE_XOR:
      return "XOR";
    case TFHE_NAND:
      return "NAND";
    case TFHE_NOR:
      return "NOR";
    case TFHE_XNOR:
      return "XNOR";
    case TFHE_NOT:
      return "NOT";
  }
  return "";
}

/* b(n) = ceil(log2(n + 1)). */
static uint32_t tfhe_bits(uint32_t n) {
  uint32_t w = 0;
  uint64_t v = n;
  while (v != 0u) {
    w++;
    v >>= 1;
  }
  return w;
}

/* ceil(log2 x) for x >= 1, 0 for x = 0. */
static uint32_t tfhe_clog2(uint64_t x) {
  uint32_t r = 0;
  while (r < 64u && ((uint64_t)1 << r) < x)
    r++;
  return r;
}

/* P(n) of tfhe.h: P(n) = P(ceil(n / 2)) + 2 (b(n) - 1) for n >= 2. */
static uint32_t tfhe_pop_depth(uint32_t n) {
  uint32_t p = 0;
  while (n >= 2u) {
    p += 2u * (tfhe_bits(n) - 1u);
    n -= n / 2u;
  }
  return p;
}

uint32_t tfhe_bound(const RuleFacts *facts) {
  uint32_t t = 0;
  uint32_t j;
  for (j = 0; j < facts->k; j++) {
    uint32_t n = facts->n[j];
    uint32_t tj = n == 0u ? 0u : tfhe_pop_depth(n) + tfhe_bits(n) - 1u;
    t = tj > t ? tj : t;
  }
  return t + tfhe_clog2(facts->k) + tfhe_clog2(facts->min_count);
}

static int tfhe_is_const(uint32_t w) {
  return w >= TFHE_K0;
}

static int tfhe_const_bit(uint32_t w) {
  return w == TFHE_K1 ? 1 : 0;
}

static uint32_t tfhe_emit(TfheBuild *b, TfheOp op, uint32_t x, uint32_t y) {
  if (b->over || b->count >= TFHE_GATE_MAX) {
    b->over = 1;
    return TFHE_K0;
  }
  if (b->gates != NULL) {
    b->gates[b->count].op = op;
    b->gates[b->count].a = x;
    b->gates[b->count].b = y;
  }
  b->count++;
  return b->inputs + b->count - 1u;
}

static uint32_t tfhe_not(TfheBuild *b, uint32_t x) {
  if (tfhe_is_const(x))
    return tfhe_const_bit(x) ? TFHE_K0 : TFHE_K1;
  return tfhe_emit(b, TFHE_NOT, x, x);
}

/* The other input fixed: F0 = the gate at 0, F1 the gate at 1. */
static uint32_t tfhe_unary(TfheBuild *b, uint32_t x, int f0, int f1) {
  if (f0 == f1)
    return f0 ? TFHE_K1 : TFHE_K0;
  return f0 == 0 ? x : tfhe_not(b, x);
}

/* A two-input gate, folded: a constant input or X = Y names a function of
   one wire (a constant, the wire, or NOT the wire). */
static uint32_t tfhe_gate(TfheBuild *b, TfheOp op, uint32_t x, uint32_t y) {
  if (tfhe_is_const(x) && tfhe_is_const(y))
    return tfhe_op_eval(op, tfhe_const_bit(x), tfhe_const_bit(y)) ? TFHE_K1 : TFHE_K0;
  if (tfhe_is_const(x))
    return tfhe_unary(b, y, tfhe_op_eval(op, tfhe_const_bit(x), 0), tfhe_op_eval(op, tfhe_const_bit(x), 1));
  if (tfhe_is_const(y))
    return tfhe_unary(b, x, tfhe_op_eval(op, 0, tfhe_const_bit(y)), tfhe_op_eval(op, 1, tfhe_const_bit(y)));
  if (x == y)
    return tfhe_unary(b, x, tfhe_op_eval(op, 0, 0), tfhe_op_eval(op, 1, 1));
  return tfhe_emit(b, op, x, y);
}

/* The count of the LEN member bits from wire LO into OUT, LSB first; returns
   b(LEN). The halves ceil(LEN / 2) and floor(LEN / 2) add with a
   ripple-carry adder; the carry out of the top bit is 0 and not built.
   Each gate call is its own statement: the row order is fixed. */
static uint32_t tfhe_popcount(TfheBuild *b, uint32_t lo, uint32_t len, uint32_t *out) {
  uint32_t left[TFHE_BITS];
  uint32_t right[TFHE_BITS];
  uint32_t width = tfhe_bits(len);
  uint32_t half = len - len / 2u;
  uint32_t carry = TFHE_K0;
  uint32_t wl;
  uint32_t wr;
  uint32_t i;
  if (len == 0u || b->over)
    return 0u;
  if (len == 1u) {
    out[0] = lo;
    return 1u;
  }
  wl = tfhe_popcount(b, lo, half, left);
  wr = tfhe_popcount(b, lo + half, len / 2u, right);
  for (i = 0; i < width; i++) {
    uint32_t x = i < wl ? left[i] : TFHE_K0;
    uint32_t y = i < wr ? right[i] : TFHE_K0;
    uint32_t t = tfhe_gate(b, TFHE_XOR, x, y);
    out[i] = tfhe_gate(b, TFHE_XOR, t, carry);
    if (i + 1u < width) {
      uint32_t both = tfhe_gate(b, TFHE_AND, x, y);
      uint32_t pass = tfhe_gate(b, TFHE_AND, carry, t);
      carry = tfhe_gate(b, TFHE_OR, both, pass);
    }
  }
  return width;
}

/* c >= M over the WIDTH count bits C, LSB up (tfhe.h). */
static uint32_t tfhe_at_least(TfheBuild *b, const uint32_t *c, uint32_t width, uint32_t m) {
  uint32_t g = TFHE_K1;
  uint32_t i;
  for (i = 0; i < width; i++)
    g = tfhe_gate(b, ((m >> i) & 1u) != 0u ? TFHE_AND : TFHE_OR, c[i], g);
  return g;
}

/* The balanced OP tree over the COUNT wires of W, in place: each round
   pairs neighbours, so the depth is clog2(COUNT). No leaf gives the unit. */
static uint32_t tfhe_tree(TfheBuild *b, TfheOp op, uint32_t *w, uint32_t count) {
  uint32_t i;
  if (count == 0u)
    return op == TFHE_AND ? TFHE_K1 : TFHE_K0;
  while (count > 1u) {
    uint32_t half = count / 2u;
    for (i = 0; i < half; i++)
      w[i] = tfhe_gate(b, op, w[2u * i], w[2u * i + 1u]);
    if ((count & 1u) != 0u)
      w[half] = w[count - 1u];
    count -= half;
  }
  return w[0];
}

/* One pass over the stages into B; the accept wire goes to OUT. The count
   pass and the store pass run the same steps, so they emit the same rows.
   Returns 0 on OOM. */
static int tfhe_pass(Arena *arena, const RuleFacts *facts, TfheBuild *b, uint32_t *out) {
  uint32_t k = facts->k;
  uint32_t *bits = arena_alloc(arena, ((size_t)k * TFHE_BITS + 1u) * sizeof *bits);
  uint32_t *width = arena_alloc(arena, ((size_t)k + 1u) * sizeof *width);
  uint32_t *base = arena_alloc(arena, ((size_t)k + 1u) * sizeof *base);
  uint32_t *terms = arena_alloc(arena, ((size_t)k + 1u) * sizeof *terms);
  uint32_t *ors = arena_alloc(arena, ((size_t)facts->min_kept + 1u) * sizeof *ors);
  uint32_t *memo;
  uint32_t lo = 0;
  uint32_t vecs = 0;
  size_t slots = 0;
  size_t s;
  uint32_t j;
  uint32_t v;
  if (bits == NULL || width == NULL || base == NULL || terms == NULL || ors == NULL)
    return 0;
  for (j = 0; j < k; j++) {
    width[j] = tfhe_popcount(b, lo, facts->n[j], bits + (size_t)j * TFHE_BITS);
    lo += facts->n[j];
  }
  *out = TFHE_K0;
  if (b->over)
    return 1;
  for (j = 0; j < k; j++) {
    base[j] = (uint32_t)slots;
    slots += (size_t)facts->n[j] + 1u;
  }
  memo = arena_alloc(arena, (slots + 1u) * sizeof *memo);
  if (memo == NULL)
    return 0;
  for (s = 0; s < slots; s++)
    memo[s] = TFHE_UNSET;
  for (v = 0; v < facts->min_kept && !b->over; v++) {
    const uint32_t *m = facts->min + (size_t)v * k;
    uint32_t t = 0;
    for (j = 0; j < k; j++) {
      uint32_t *slot = memo + base[j] + m[j];
      if (m[j] == 0u)
        continue;
      if (*slot == TFHE_UNSET)
        *slot = tfhe_at_least(b, bits + (size_t)j * TFHE_BITS, width[j], m[j]);
      terms[t++] = *slot;
    }
    ors[vecs++] = tfhe_tree(b, TFHE_AND, terms, t);
  }
  *out = tfhe_tree(b, TFHE_OR, ors, vecs);
  return 1;
}

static uint32_t tfhe_wire_depth(const TfheCircuit *circuit, const uint32_t *d, uint32_t w) {
  return w < circuit->inputs ? 0u : d[w - circuit->inputs];
}

int tfhe_depth(Arena *arena, const TfheCircuit *circuit, uint32_t *depth, uint32_t *nots) {
  uint32_t *d = arena_alloc(arena, ((size_t)circuit->gate_count + 1u) * sizeof *d);
  uint32_t g;
  if (d == NULL)
    return 0;
  *nots = 0;
  for (g = 0; g < circuit->gate_count; g++) {
    const TfheGate *row = circuit->gates + g;
    uint32_t da = tfhe_wire_depth(circuit, d, row->a);
    uint32_t db = tfhe_wire_depth(circuit, d, row->b);
    d[g] = row->op == TFHE_NOT ? da : 1u + (da > db ? da : db);
    *nots += row->op == TFHE_NOT ? 1u : 0u;
  }
  *depth = tfhe_wire_depth(circuit, d, circuit->out);
  return 1;
}

TfheResult tfhe_build(Arena *arena, const RuleFacts *facts, TfheCircuit *circuit) {
  TfheBuild b;
  uint32_t out = TFHE_K0;
  uint32_t j;
  memset(circuit, 0, sizeof *circuit);
  circuit->bound = tfhe_bound(facts);
  if (facts->min_count > TFHE_MIN_CAP)
    return TFHE_REFUSE_MIN;
  if ((uint64_t)facts->min_kept != facts->min_count)
    return TFHE_FAULT_SHAPE;
  for (j = 0; j < facts->k; j++)
    circuit->inputs += facts->n[j];
  b.gates = NULL;
  b.inputs = circuit->inputs;
  b.count = 0;
  b.over = 0;
  if (!tfhe_pass(arena, facts, &b, &out))
    return TFHE_OOM;
  if (b.over)
    return TFHE_REFUSE_GATES;
  circuit->gates = arena_alloc(arena, ((size_t)b.count + 1u) * sizeof *circuit->gates);
  if (circuit->gates == NULL)
    return TFHE_OOM;
  b.gates = circuit->gates;
  b.count = 0;
  if (!tfhe_pass(arena, facts, &b, &out))
    return TFHE_OOM;
  circuit->gate_count = b.count;
  if (tfhe_is_const(out))
    return TFHE_FAULT_SHAPE;
  circuit->out = out;
  if (!tfhe_depth(arena, circuit, &circuit->depth, &circuit->nots))
    return TFHE_OOM;
  return circuit->depth > circuit->bound ? TFHE_FAULT_DEPTH : TFHE_OK;
}
