/* ratify-lang S10 U1 (Q-S10-3 (a), Q-S10-6 (a)): runs entries of a langc
 * Wasm module in the own engine (host/engine.c) and prints each result as
 * test/run-wasm.mjs does. A port of permit-lang test/enginetool.c (main
 * :210-253), with the host table of run-wasm.mjs in place of the O5 stub.
 *
 *   build/enginetool [--host TABLE] FILE NAME [ARGS...]
 *   build/enginetool [--host TABLE] FILE -   (one `NAME [ARGS...]` line on stdin per call)
 *   build/enginetool --bytes OFF LEN FILE
 *
 * TABLE has `NAME W1 .. Wp RESULT` rows (decimal words); `#` lines and empty
 * lines are skipped. Each of the six imports of module "ratify" gives the
 * RESULT of the first row of its name and words, as `langc eval --host`
 * does. The tool prints one number per call. --bytes prints in hex the LEN
 * bytes at OFF of the exported memory (engine_bytes) of a module with no
 * imports. Exit 2: usage or a bad row. Exit 1: a load error, a call with no
 * row, a module with imports and no TABLE, a bad call, or bytes outside the
 * memory. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "wasmread.h"

enum { LINE_MAX = 4096, ROW_MAX = 4096, WORD_MAX = 9, IMPORT_COUNT = 6 };

/* A row of the table: the import (an index of table_funcs), its PARAMS
 * words and the result. */
typedef struct {
  unsigned import;
  unsigned params;
  uint64_t words[WORD_MAX];
  uint64_t result;
} Row;

typedef struct {
  Row rows[ROW_MAX];
  unsigned count;
} Table;

/* The context of the table imports: the table, and the key of a call with
 * no row (MISS 1). */
typedef struct {
  const Table *table;
  unsigned miss;
  unsigned import;
  unsigned params;
  uint64_t words[WORD_MAX];
} Host;

/* 1 and the value when S is a run of decimal digits that fits in 64 bits
 * (the number of host/ratifyd.c). */
static int number(const char *s, uint64_t *v) {
  uint64_t n = 0;
  if (*s == '\0')
    return 0;
  for (; *s != '\0'; s++) {
    uint64_t d = (uint64_t)(*s - '0');
    if (*s < '0' || *s > '9' || n > (UINT64_MAX - d) / 10u)
      return 0;
    n = n * 10u + d;
  }
  *v = n;
  return 1;
}

/* Splits LINE at each space into at most MAX fields (an empty field stays,
 * as in String.split). Gives the count, or MAX + 1 for more. */
static unsigned split(char *line, char **f, unsigned max) {
  unsigned n = 1;
  char *c;
  f[0] = line;
  for (c = strchr(line, ' '); c; c = strchr(c + 1, ' ')) {
    if (n == max)
      return max + 1;
    *c = '\0';
    f[n++] = c + 1;
  }
  return n;
}

/* The first row of IMPORT with the words ARGS (the first row wins, as in
 * langc eval). Gives 1 and *RESULT, or 0 and the key of the miss in CTX. */
static int lookup(void *ctx, unsigned import, unsigned params, const uint64_t *args, uint64_t *result) {
  Host *h = ctx;
  unsigned i;
  for (i = 0; i < h->table->count; i++) {
    const Row *r = &h->table->rows[i];
    if (r->import == import && memcmp(r->words, args, params * sizeof *args) == 0) {
      *result = r->result;
      return 1;
    }
  }
  h->miss = 1;
  h->import = import;
  h->params = params;
  memcpy(h->words, args, params * sizeof *args);
  return 0;
}

static int sig_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return lookup(ctx, 0, 6, args, result);
}

static int hash_eq(void *ctx, const uint64_t *args, uint64_t *result) {
  return lookup(ctx, 1, 4, args, result);
}

static int share_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return lookup(ctx, 2, 6, args, result);
}

static int ballot_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return lookup(ctx, 3, 9, args, result);
}

static int tally_add(void *ctx, const uint64_t *args, uint64_t *result) {
  return lookup(ctx, 4, 6, args, result);
}

static int combine(void *ctx, const uint64_t *args, uint64_t *result) {
  return lookup(ctx, 5, 8, args, result);
}

/* The six imports of module "ratify", in the order of src/front/eval.c and
 * test/run-wasm.mjs. */
static const EngineHostFunc table_funcs[IMPORT_COUNT] = {
    {"ratify", "sig_verify", 6, sig_verify}, {"ratify", "hash_eq", 4, hash_eq},
    {"ratify", "share_verify", 6, share_verify}, {"ratify", "ballot_verify", 9, ballot_verify},
    {"ratify", "tally_add", 6, tally_add}, {"ratify", "combine", 8, combine}};

/* Reads the rows of F (the file NAME) into T. Gives 1, or 0 and a message
 * for a bad row. */
static int read_table(FILE *f, const char *name, Table *t) {
  char line[LINE_MAX];
  char *field[WORD_MAX + 2];
  unsigned index = 0;
  while (fgets(line, sizeof line, f)) {
    unsigned n, k, i;
    Row *r = &t->rows[t->count];
    index++;
    line[strcspn(line, "\n")] = '\0';
    if (line[0] == '\0' || line[0] == '#')
      continue;
    n = split(line, field, WORD_MAX + 2);
    for (k = 0; k < IMPORT_COUNT && strcmp(field[0], table_funcs[k].name) != 0; k++)
      ;
    if (k == IMPORT_COUNT) {
      fprintf(stderr, "enginetool: %s: line %u: no host import %s\n", name, index, field[0]);
      return 0;
    }
    if (t->count == ROW_MAX) {
      fprintf(stderr, "enginetool: %s: more than %d rows\n", name, ROW_MAX);
      return 0;
    }
    r->import = k;
    r->params = table_funcs[k].params;
    for (i = 0; i < r->params && i + 1 < n && number(field[i + 1], &r->words[i]); i++)
      ;
    if (n != r->params + 2 || i != r->params || !number(field[n - 1], &r->result)) {
      fprintf(stderr, "enginetool: %s: line %u: %s takes %u words and one result\n", name, index, field[0], r->params);
      return 0;
    }
    t->count++;
  }
  return 1;
}

/* Loads the module FILE into E with BINDING (NULL: no imports). */
static int load(Engine *e, const char *file, const EngineBinding *binding) {
  static unsigned char bytes[WASM_MODULE_MAX + 1];
  FILE *m = fopen(file, "rb");
  size_t size;
  if (!m) {
    fprintf(stderr, "enginetool: cannot open %s\n", file);
    return 0;
  }
  size = fread(bytes, 1, sizeof bytes, m);
  fclose(m);
  if (engine_load(e, bytes, size, binding) != ENGINE_OK) {
    fprintf(stderr, "enginetool: load: %s\n", engine_message(e));
    return 0;
  }
  return 1;
}

/* Calls the entry F[0] with the N - 1 words of F[1..] and prints the
 * result. */
static int call(Engine *e, const Host *h, char **f, unsigned n) {
  static uint64_t args[ENGINE_PARAM_MAX];
  unsigned func = 0, i;
  uint64_t result = 0;
  if (!engine_find(e, f[0], &func)) {
    fprintf(stderr, "enginetool: no exported entry '%s'\n", f[0]);
    return 0;
  }
  if (n > ENGINE_PARAM_MAX + 1) {
    fprintf(stderr, "enginetool: %s: more than %d words\n", f[0], ENGINE_PARAM_MAX);
    return 0;
  }
  for (i = 1; i < n; i++)
    if (!number(f[i], &args[i - 1])) {
      fprintf(stderr, "enginetool: %s: bad word '%s'\n", f[0], f[i]);
      return 0;
    }
  if (engine_call(e, func, args, n - 1, &result) == ENGINE_OK) {
    printf("%llu\n", (unsigned long long)result);
    return 1;
  }
  if (!h->miss) {
    fprintf(stderr, "enginetool: call %s: %s\n", f[0], engine_message(e));
    return 0;
  }
  fprintf(stderr, "enginetool: no host row for %s", table_funcs[h->import].name);
  for (i = 0; i < h->params; i++)
    fprintf(stderr, " %llu", (unsigned long long)h->words[i]);
  fprintf(stderr, "\n");
  return 0;
}

/* Makes the call of each line of S on E. Gives 1 when each call is good. */
static int run_lines(Engine *e, const Host *h, FILE *s) {
  char line[LINE_MAX];
  char *f[ENGINE_PARAM_MAX + 1];
  while (fgets(line, sizeof line, s)) {
    line[strcspn(line, "\n")] = '\0';
    if (line[0] != '\0' && !call(e, h, f, split(line, f, ENGINE_PARAM_MAX + 1)))
      return 0;
  }
  return 1;
}

/* --bytes: prints in hex the LEN bytes at OFF of the memory of FILE. */
static int dump_bytes(const char *off, const char *len, const char *file) {
  Engine e;
  uint64_t o = 0, n = 0, i;
  const unsigned char *p = NULL;
  if (!number(off, &o) || !number(len, &n)) {
    fprintf(stderr, "usage: enginetool --bytes OFF LEN FILE\n");
    return 2;
  }
  if (!load(&e, file, NULL))
    return 1;
  if (!engine_bytes(&e, o, n, &p)) {
    fprintf(stderr, "enginetool: bytes %s %s: not in the memory\n", off, len);
    engine_free(&e);
    return 1;
  }
  for (i = 0; i < n; i++)
    printf("%02x", p[i]);
  printf("\n");
  engine_free(&e);
  return 0;
}

int main(int argc, char **argv) {
  static Table table;
  static Host host;
  EngineBinding binding = {table_funcs, IMPORT_COUNT, &host};
  Engine e;
  int hosted, first, ok;
  if (argc == 5 && strcmp(argv[1], "--bytes") == 0)
    return dump_bytes(argv[2], argv[3], argv[4]);
  hosted = argc > 1 && strcmp(argv[1], "--host") == 0;
  first = hosted ? 3 : 1;
  if (argc < first + 2) {
    fprintf(stderr, "usage: enginetool [--host TABLE] FILE NAME [ARGS...] | [--host TABLE] FILE - | --bytes OFF LEN FILE\n");
    return 2;
  }
  host.table = &table;
  if (hosted) {
    FILE *t = fopen(argv[2], "r");
    if (!t) {
      fprintf(stderr, "enginetool: cannot open %s\n", argv[2]);
      return 1;
    }
    ok = read_table(t, argv[2], &table);
    fclose(t);
    if (!ok)
      return 2;
  }
  if (!load(&e, argv[first], hosted ? &binding : NULL))
    return 1;
  ok = argc == first + 2 && strcmp(argv[first + 1], "-") == 0
           ? run_lines(&e, &host, stdin)
           : call(&e, &host, argv + first + 1, (unsigned)(argc - first - 1));
  engine_free(&e);
  return ok ? 0 : 1;
}
