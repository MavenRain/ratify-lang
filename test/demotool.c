/* ratify-lang S10 U3 Part C (Q-S10-U3-L (c), Form (ii)): the demo runner. It
 * reads the records of a demo image (test/cryptotool.c mode `demo DIR`),
 * puts the bytes at the slots of the host region of HOST_MEM bytes
 * (src/hostmem.h), and runs the Wasm entry `demo` of MODULE in the own
 * engine (host/engine.c) with the six `ratify` imports bound to the bodies of
 * host/imports.c, as host/ratifyd.c does. That gives the flag word W. Then it
 * evaluates the entry `demoV` of DEMO.lang (kit mode) on W and the words of
 * the image in the same process, and prints one line `W VERDICT`.
 *
 *   build/demotool MODULE DEMO.lang DK.txt
 *
 * A record line is `KIND HEX` (a slot of demo_slots, HEX = 2 * LEN lowercase
 * hex digits) or `KIND N` (amend, target, signers: decimal). Each KIND occurs
 * once. A bad record prints `- invalid malformed`, exit 0, and the module
 * does not run. Exit 0: a verdict. Exit 1: no file, a load or engine error,
 * W over 8191, an eval error, or a payload word that is not a verdict word.
 * Exit 2: usage. The W bit order is test code here and in demo.lang. */
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "front/check.h"
#include "front/front.h"
#include "imports.h"

enum {
  DEMO_SLOTS = 18,
  DEMO_WORDS = 3,
  DEMO_ARGS = 6,
  DEMO_IMPORTS = 6,
  DEMO_TALLY_ADD = 4,
  DEMO_TALLY_BYTES = 128,
  DEMO_W_MAX = 8191,
  DEMO_VERDICT_MAX = 8,
  DEMO_BODY = 5,          /* the slot of the cert body */
  DEMO_ACTION_AT = 40,    /* the action word in the body (cert_body order) */
  DEMO_LINE_CAP = 1024,   /* over `proof ` and 2 * 424 hex digits */
  DEMO_MODULE_MAX = 1 << 20, /* WASM_MODULE_MAX of host/wasmread.h */
  DEMO_SOURCE_MAX = 1 << 20,
  DEMO_WORD_TEXT = 24
};

#define DEMO_ARENA_BYTES ((size_t)1 << 30)

/* A slot of the region: the KIND of its record, its OFF and its LEN. */
typedef struct {
  const char *kind;
  unsigned off;
  unsigned len;
} DemoSlot;

/* The slot table of demo.lang (the header of the entry demo), in the order
 * of the 18 OFF words of demo. */
static const DemoSlot demo_slots[DEMO_SLOTS] = {
    {"pk0", 0, 64},     {"pk1", 64, 64},    {"pk2", 128, 64},   {"pk3", 192, 64},
    {"pk4", 256, 64},   {"body", 320, 48},  {"prev", 368, 48},  {"sig0", 416, 96},
    {"sig1", 512, 96},  {"sig2", 608, 96},  {"sig3", 704, 96},  {"sig4", 800, 96},
    {"proof", 896, 424}, {"evA", 1320, 48}, {"evB", 1368, 48},  {"evH", 1416, 32},
    {"sigA", 1448, 96}, {"sigB", 1544, 96}};

/* The word records, in the order of the last three words of demoV. */
static const char *const demo_word_kinds[DEMO_WORDS] = {"amend", "target", "signers"};

/* The Reason names of domain.lang: the payload word N of demoV is
 * `invalid` and the name N - 1 (vdWord order). */
static const char *const demo_reasons[DEMO_VERDICT_MAX] = {
    "malformed", "illTypedPolicy", "wrongHeight", "badPrevHash",
    "badSignature", "badProof", "notAccepted", "equivocation"};

/* The image: the region, the words, and the KINDs that occur. */
typedef struct {
  unsigned char region[HOST_MEM];
  uint64_t words[DEMO_WORDS];
  int seen[DEMO_SLOTS + DEMO_WORDS];
} DemoImage;

/* The result of demo_read. */
typedef enum { DEMO_READ_OK, DEMO_READ_MALFORMED, DEMO_READ_MISSING } DemoRead;

/* The context of the imports: the engine of the call. */
typedef struct {
  Engine *engine;
} DemoHost;

/* 1 and the value when S is a run of decimal digits that fits in 64 bits. */
static int demo_number(const char *s, uint64_t *v) {
  uint64_t n = 0;
  if (*s == '\0')
    return 0;
  for (; *s != '\0'; s++) {
    if (*s < '0' || *s > '9' || n > (UINT64_MAX - (uint64_t)(*s - '0')) / 10u)
      return 0;
    n = n * 10u + (uint64_t)(*s - '0');
  }
  *v = n;
  return 1;
}

/* The value of the lowercase hex digit C, or -1. */
static int demo_hex(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

/* The index of KIND: a slot, then DEMO_SLOTS + a word, or -1. */
static int demo_kind(const char *kind) {
  int i;
  for (i = 0; i < DEMO_SLOTS; i++)
    if (strcmp(kind, demo_slots[i].kind) == 0)
      return i;
  for (i = 0; i < DEMO_WORDS; i++)
    if (strcmp(kind, demo_word_kinds[i]) == 0)
      return DEMO_SLOTS + i;
  return -1;
}

/* Puts the record LINE (no newline) into IMAGE. Gives 0 for an unknown KIND,
 * a KIND that occurred before, a bad word, or a hex field that is not 2 * LEN
 * hex digits (a short record is bad too: each import reads a fixed size). */
static int demo_record(char *line, DemoImage *image) {
  char *field = strchr(line, ' ');
  int k;
  size_t i;
  if (field == NULL)
    return 0;
  *field++ = '\0';
  k = demo_kind(line);
  if (k < 0 || image->seen[k])
    return 0;
  image->seen[k] = 1;
  if (k >= DEMO_SLOTS)
    return demo_number(field, &image->words[k - DEMO_SLOTS]);
  if (strlen(field) != 2u * demo_slots[k].len)
    return 0;
  for (i = 0; i < demo_slots[k].len; i++) {
    int hi = demo_hex(field[2 * i]), lo = demo_hex(field[2 * i + 1]);
    if (hi < 0 || lo < 0)
      return 0;
    image->region[demo_slots[k].off + i] = (unsigned char)(hi * 16 + lo);
  }
  return 1;
}

/* Reads the records of the file NAME into IMAGE. Each KIND must occur once,
 * and a line must fit in DEMO_LINE_CAP. */
static DemoRead demo_read(const char *name, DemoImage *image) {
  static char line[DEMO_LINE_CAP + 2];
  FILE *f = fopen(name, "r");
  int ok = 1, i;
  if (!f) {
    fprintf(stderr, "demotool: cannot open %s\n", name);
    return DEMO_READ_MISSING;
  }
  memset(image, 0, sizeof *image);
  while (ok && fgets(line, sizeof line, f)) {
    size_t n = strlen(line);
    int full = n > 0 && line[n - 1] == '\n';
    if (full)
      line[n - 1] = '\0';
    ok = (full || feof(f)) && demo_record(line, image);
  }
  fclose(f);
  for (i = 0; i < DEMO_SLOTS + DEMO_WORDS; i++)
    ok = ok && image->seen[i];
  return ok ? DEMO_READ_OK : DEMO_READ_MALFORMED;
}

/* The import J on ARGS, as host/ratifyd.c: copies the region out of the
 * module memory, calls host_import, and for tally_add with result 1 writes
 * the 128 bytes at ARGS[0] back. Gives 0, a host error, when the memory has
 * no region. */
static int demo_import(void *ctx, uint32_t j, const uint64_t *args, uint64_t *result) {
  static unsigned char copy[HOST_MEM];
  DemoHost *h = ctx;
  const unsigned char *mem;
  if (!engine_bytes(h->engine, 0, HOST_MEM, &mem))
    return 0;
  memcpy(copy, mem, HOST_MEM);
  *result = host_import(j, args, copy);
  if (j == DEMO_TALLY_ADD && *result == 1u)
    return engine_put_bytes(h->engine, args[0], DEMO_TALLY_BYTES, copy + args[0]);
  return 1;
}

static int demo_sig_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return demo_import(ctx, 0u, args, result);
}

static int demo_hash_eq(void *ctx, const uint64_t *args, uint64_t *result) {
  return demo_import(ctx, 1u, args, result);
}

static int demo_share_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return demo_import(ctx, 2u, args, result);
}

static int demo_ballot_verify(void *ctx, const uint64_t *args, uint64_t *result) {
  return demo_import(ctx, 3u, args, result);
}

static int demo_tally_add(void *ctx, const uint64_t *args, uint64_t *result) {
  return demo_import(ctx, 4u, args, result);
}

static int demo_combine(void *ctx, const uint64_t *args, uint64_t *result) {
  return demo_import(ctx, 5u, args, result);
}

/* The six imports of module "ratify", in the order of host/ratifyd.c. */
static const EngineHostFunc demo_funcs[DEMO_IMPORTS] = {
    {"ratify", "sig_verify", 6, demo_sig_verify}, {"ratify", "hash_eq", 4, demo_hash_eq},
    {"ratify", "share_verify", 6, demo_share_verify}, {"ratify", "ballot_verify", 9, demo_ballot_verify},
    {"ratify", "tally_add", 6, demo_tally_add}, {"ratify", "combine", 8, demo_combine}};

/* Loads the module FILE into E with BINDING. Gives 1, or 0 and a message. */
static int demo_load(Engine *e, const char *file, const EngineBinding *binding) {
  static unsigned char bytes[DEMO_MODULE_MAX + 1];
  FILE *m = fopen(file, "rb");
  size_t size;
  if (!m) {
    fprintf(stderr, "demotool: cannot open %s\n", file);
    return 0;
  }
  size = fread(bytes, 1, sizeof bytes, m);
  fclose(m);
  if (engine_load(e, bytes, size, binding) != ENGINE_OK) {
    fprintf(stderr, "demotool: load: %s\n", engine_message(e));
    return 0;
  }
  return 1;
}

/* Runs the entry demo of the module FILE on the region of IMAGE with the 18
 * slot OFFs (C2). Gives 1 and W, or 0 and a message. */
static int demo_run(const char *file, const DemoImage *image, uint64_t *w) {
  static DemoHost host;
  static uint64_t args[DEMO_SLOTS];
  EngineBinding binding = {demo_funcs, DEMO_IMPORTS, &host};
  Engine e;
  unsigned func = 0, i;
  int ok;
  host.engine = &e;
  if (!demo_load(&e, file, &binding))
    return 0;
  for (i = 0; i < DEMO_SLOTS; i++)
    args[i] = demo_slots[i].off;
  ok = engine_put_bytes(&e, 0, HOST_MEM, image->region) && engine_find(&e, "demo", &func) &&
       engine_call(&e, func, args, DEMO_SLOTS, w) == ENGINE_OK;
  if (!ok)
    fprintf(stderr, "demotool: demo of %s: no region, no entry demo, or a bad call: %s\n", file,
            engine_message(&e));
  engine_free(&e);
  return ok;
}

/* The big-endian word of the 8 bytes at P. */
static uint64_t demo_be_word(const unsigned char *p) {
  uint64_t v = 0;
  int i;
  for (i = 0; i < 8; i++)
    v = (v << 8) | p[i];
  return v;
}

/* Reads the output of eval in OUT: one decimal payload word and a newline
 * (eval prints the payload of `some N` as N). */
static int demo_payload(FILE *out, uint64_t *verdict) {
  static char line[DEMO_WORD_TEXT + 2];
  size_t n;
  rewind(out);
  if (!fgets(line, sizeof line, out)) {
    fprintf(stderr, "demotool: demoV printed no line\n");
    return 0;
  }
  n = strlen(line);
  if (n > 0 && line[n - 1] == '\n')
    line[n - 1] = '\0';
  if (!demo_number(line, verdict)) {
    fprintf(stderr, "demotool: demoV printed '%s', not a payload word\n", line);
    return 0;
  }
  return 1;
}

/* Evaluates `demoV WORDS` of the program SOURCE in kit mode in this process
 * (C3). Gives 1 and the payload word, or 0 and a message. */
static int demo_eval(const char *source, const uint64_t *words, uint64_t *verdict) {
  static char text[DEMO_SOURCE_MAX + 1];
  static char word_text[DEMO_ARGS][DEMO_WORD_TEXT];
  char *args[DEMO_ARGS];
  Arena arena;
  Diag diag;
  DeclList decls;
  Machine machine;
  FILE *f = fopen(source, "rb");
  FILE *out;
  size_t len;
  int ok, i;
  if (!f) {
    fprintf(stderr, "demotool: cannot open %s\n", source);
    return 0;
  }
  len = fread(text, 1, sizeof text, f);
  fclose(f);
  if (len > DEMO_SOURCE_MAX) {
    fprintf(stderr, "demotool: %s is larger than %d bytes\n", source, DEMO_SOURCE_MAX);
    return 0;
  }
  out = tmpfile();
  if (!out) {
    fprintf(stderr, "demotool: no temporary file for the eval output\n");
    return 0;
  }
  for (i = 0; i < DEMO_ARGS; i++) {
    snprintf(word_text[i], DEMO_WORD_TEXT, "%llu", (unsigned long long)words[i]);
    args[i] = word_text[i];
  }
  diag_init(&diag);
  arena_init(&arena, DEMO_ARENA_BYTES);
  ok = front_load(&arena, source, text, len, &decls, &diag) &&
       check_program(&arena, &decls, &machine, &diag, CHECK_KIT) &&
       eval_command(&machine, "demoV", args, DEMO_ARGS, out, stderr) == 0;
  if (!ok)
    diag_print(&diag, stderr);
  ok = ok && demo_payload(out, verdict);
  arena_release(&arena);
  fclose(out);
  return ok;
}

int main(int argc, char **argv) {
  static DemoImage image;
  uint64_t w = 0, verdict = 0, words[DEMO_ARGS];
  if (argc != 4) {
    fprintf(stderr, "usage: demotool MODULE DEMO.lang DK.txt\n");
    return 2;
  }
  switch (demo_read(argv[3], &image)) {
  case DEMO_READ_MISSING:
    return 1;
  case DEMO_READ_MALFORMED:
    printf("- invalid malformed\n");
    return 0;
  case DEMO_READ_OK:
    break;
  }
  if (!demo_run(argv[1], &image, &w))
    return 1;
  if (w > DEMO_W_MAX) {
    fprintf(stderr, "demotool: W %llu is over %d\n", (unsigned long long)w, DEMO_W_MAX);
    return 1;
  }
  words[0] = w;
  words[1] = demo_be_word(image.region + demo_slots[DEMO_BODY].off);
  words[2] = demo_be_word(image.region + demo_slots[DEMO_BODY].off + DEMO_ACTION_AT);
  words[3] = image.words[0];
  words[4] = image.words[1];
  words[5] = image.words[2];
  if (!demo_eval(argv[2], words, &verdict))
    return 1;
  if (verdict > DEMO_VERDICT_MAX) {
    fprintf(stderr, "demotool: demoV gave %llu, past the verdict words\n", (unsigned long long)verdict);
    return 1;
  }
  if (verdict == 0u)
    printf("%llu valid\n", (unsigned long long)w);
  else
    printf("%llu invalid %s\n", (unsigned long long)w, demo_reasons[verdict - 1u]);
  return 0;
}
