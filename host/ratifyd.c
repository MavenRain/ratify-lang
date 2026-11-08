/* ratify-lang S10 U2 (Q-S10-U2-4 (a)): the T1 host. It writes an image into
 * the host region of HOST_MEM bytes (src/hostmem.h), runs an entry of a
 * langc Wasm module in the own engine (host/engine.c) with the six `ratify`
 * imports bound to the bodies of host/imports.c, and prints the i64 result
 * as test/enginetool.c does.
 *
 *   build/ratifyd --call IMAGE FILE NAME [ARGS...]
 *
 * IMAGE has `OFF HEX` lines: a decimal region offset, one space, then one or
 * more bytes as lowercase hex, with OFF + bytes <= HOST_MEM. A bad line
 * prints `invalid malformed` on stdout, exit 0, and the module does not run.
 * Each import copies the region out of the module memory, calls host_import,
 * and for tally_add with result 1 writes the 128 bytes at W[0] back. The
 * verdict mode `ratifyd FILE CASE` waits for U3. Exit 2: usage. Exit 1: no
 * IMAGE or FILE, a load error, no region, no entry, a bad word or a bad
 * call. */
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "imports.h"
#include "wasmread.h"

enum { IMPORT_COUNT = 6, TALLY_ADD = 4, TALLY_BYTES = 128, LINE_CAP = 2 * HOST_MEM + 24 };

/* The result of read_image. */
typedef enum { IMAGE_OK, IMAGE_MALFORMED, IMAGE_MISSING } ImageResult;

/* The context of the imports: the engine of the call. */
typedef struct {
  Engine *engine;
} Host;

/* 1 and the value when S is a run of decimal digits that fits in 64 bits. */
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

/* The value of the lowercase hex digit C, or -1. */
static int hex_digit(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

/* One line of F without its '\n' in LINE (CAP bytes), as permit-lang
 * host/permitd.c get_line: gives the length, -1 at the end of F, or -2 for
 * a line over CAP - 1 bytes. */
static long get_line(FILE *f, char *line, size_t cap) {
  size_t n = 0;
  int c = getc(f);
  if (c == EOF)
    return -1;
  for (; c != EOF && c != '\n'; c = getc(f)) {
    if (n + 1 >= cap)
      return -2;
    line[n++] = (char)c;
  }
  line[n] = '\0';
  return (long)n;
}

/* Writes the bytes of the image line LINE (LEN chars) into MEM. Gives 1, or
 * 0 for a line that is not `OFF HEX` or for bytes outside the region. */
static int put_line(const char *line, size_t len, unsigned char mem[HOST_MEM]) {
  size_t i = 0, k, count;
  uint64_t off = 0;
  if (len == 0 || line[0] < '0' || line[0] > '9')
    return 0;
  for (; i < len && line[i] >= '0' && line[i] <= '9'; i++) {
    if (off > HOST_MEM)
      return 0;
    off = off * 10u + (uint64_t)(line[i] - '0');
  }
  if (i + 1 >= len || line[i] != ' ' || (len - i - 1) % 2 != 0)
    return 0;
  count = (len - i - 1) / 2;
  if (off > HOST_MEM || count > HOST_MEM - off)
    return 0;
  for (k = 0; k < count; k++) {
    int hi = hex_digit(line[i + 1 + 2 * k]), lo = hex_digit(line[i + 2 + 2 * k]);
    if (hi < 0 || lo < 0)
      return 0;
    mem[off + k] = (unsigned char)(hi * 16 + lo);
  }
  return 1;
}

/* Reads the image file NAME into MEM. */
static ImageResult read_image(const char *name, unsigned char mem[HOST_MEM]) {
  static char line[LINE_CAP];
  ImageResult r = IMAGE_OK;
  FILE *f = fopen(name, "r");
  long n;
  if (!f) {
    fprintf(stderr, "ratifyd: cannot open %s\n", name);
    return IMAGE_MISSING;
  }
  for (n = get_line(f, line, sizeof line); r == IMAGE_OK && n != -1; n = get_line(f, line, sizeof line))
    if (n < 0 || !put_line(line, (size_t)n, mem))
      r = IMAGE_MALFORMED;
  fclose(f);
  return r;
}

/* The import J on ARGS: copies the region out of the module memory, calls
 * host_import, and for tally_add with result 1 writes the 128 bytes at
 * ARGS[0] back (host_import gives 1 only when they are in the region).
 * Gives 0, a host error, when the memory has no region. */
static int run_import(void *ctx, uint32_t j, const uint64_t *args, uint64_t *result) {
  static unsigned char copy[HOST_MEM];
  Host *h = ctx;
  const unsigned char *mem;
  if (!engine_bytes(h->engine, 0, HOST_MEM, &mem))
    return 0;
  memcpy(copy, mem, HOST_MEM);
  *result = host_import(j, args, copy);
  if (j == TALLY_ADD && *result == 1u)
    return engine_put_bytes(h->engine, args[0], TALLY_BYTES, copy + args[0]);
  return 1;
}

static int sig_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return run_import(ctx, 0u, args, result);
}

static int hash_eq(void *ctx, const uint64_t *args, uint64_t *result) {
  return run_import(ctx, 1u, args, result);
}

static int share_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return run_import(ctx, 2u, args, result);
}

static int ballot_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return run_import(ctx, 3u, args, result);
}

static int tally_add(void *ctx, const uint64_t *args, uint64_t *result) {
  return run_import(ctx, 4u, args, result);
}

static int combine(void *ctx, const uint64_t *args, uint64_t *result) {
  return run_import(ctx, 5u, args, result);
}

/* The six imports of module "ratify", in the order of test/enginetool.c. */
static const EngineHostFunc ratify_funcs[IMPORT_COUNT] = {
    {"ratify", "sig_verify", 6, sig_verify}, {"ratify", "hash_eq", 4, hash_eq},
    {"ratify", "share_verify", 6, share_verify}, {"ratify", "ballot_verify", 9, ballot_verify},
    {"ratify", "tally_add", 6, tally_add}, {"ratify", "combine", 8, combine}};

/* Loads the module FILE into E with BINDING. Gives 1, or 0 and a message. */
static int load(Engine *e, const char *file, const EngineBinding *binding) {
  static unsigned char bytes[WASM_MODULE_MAX + 1];
  FILE *m = fopen(file, "rb");
  size_t size;
  if (!m) {
    fprintf(stderr, "ratifyd: cannot open %s\n", file);
    return 0;
  }
  size = fread(bytes, 1, sizeof bytes, m);
  fclose(m);
  if (engine_load(e, bytes, size, binding) != ENGINE_OK) {
    fprintf(stderr, "ratifyd: load: %s\n", engine_message(e));
    return 0;
  }
  return 1;
}

/* Calls the entry F[0] with the N - 1 words of F[1..] and prints the
 * result. */
static int call(Engine *e, char **f, unsigned n) {
  static uint64_t args[ENGINE_PARAM_MAX];
  unsigned func = 0, i;
  uint64_t result = 0;
  if (!engine_find(e, f[0], &func)) {
    fprintf(stderr, "ratifyd: no exported entry '%s'\n", f[0]);
    return 0;
  }
  if (n > ENGINE_PARAM_MAX + 1) {
    fprintf(stderr, "ratifyd: %s: more than %d words\n", f[0], ENGINE_PARAM_MAX);
    return 0;
  }
  for (i = 1; i < n; i++)
    if (!number(f[i], &args[i - 1])) {
      fprintf(stderr, "ratifyd: %s: bad word '%s'\n", f[0], f[i]);
      return 0;
    }
  if (engine_call(e, func, args, n - 1, &result) != ENGINE_OK) {
    fprintf(stderr, "ratifyd: call %s: %s\n", f[0], engine_message(e));
    return 0;
  }
  printf("%llu\n", (unsigned long long)result);
  return 1;
}

int main(int argc, char **argv) {
  static unsigned char image[HOST_MEM];
  static Host host;
  EngineBinding binding = {ratify_funcs, IMPORT_COUNT, &host};
  Engine e;
  int ok;
  if (argc < 5 || strcmp(argv[1], "--call") != 0) {
    fprintf(stderr, "usage: ratifyd --call IMAGE FILE NAME [ARGS...]\n");
    return 2;
  }
  switch (read_image(argv[2], image)) {
  case IMAGE_MISSING:
    return 1;
  case IMAGE_MALFORMED:
    printf("invalid malformed\n");
    return 0;
  case IMAGE_OK:
    break;
  }
  host.engine = &e;
  if (!load(&e, argv[3], &binding))
    return 1;
  if (!engine_put_bytes(&e, 0, HOST_MEM, image)) {
    fprintf(stderr, "ratifyd: the memory of %s has no host region\n", argv[3]);
    engine_free(&e);
    return 1;
  }
  ok = call(&e, argv + 4, (unsigned)(argc - 4));
  engine_free(&e);
  return ok ? 0 : 1;
}
