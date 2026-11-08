/* Regression checks for the engine's host import boundary. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"

enum { I32 = 0x7f, I64 = 0x7e, MODULE_CAP = 2 * ENGINE_STACK_MAX + 128 };

static unsigned calls;

static int word(void *ctx, const uint64_t *args, uint64_t *result) {
  (void)ctx;
  (void)args;
  calls++;
  *result = 7;
  return 1;
}

static void leb(unsigned char *bytes, size_t *at, size_t n) {
  do {
    unsigned char b = (unsigned char)(n & 127u);
    n >>= 7;
    bytes[(*at)++] = (unsigned char)(b | (n ? 128u : 0u));
  } while (n);
}

static void section(unsigned char *bytes, size_t *at, unsigned char id,
                    const unsigned char *data, size_t len) {
  bytes[(*at)++] = id;
  leb(bytes, at, len);
  memcpy(bytes + *at, data, len);
  *at += len;
}

/* A valid module with one import and one exported function x. The return
 * discards the unused operands below the import result. */
static size_t module(unsigned char *bytes, unsigned params, unsigned param_type,
                     unsigned result_type, unsigned pushes) {
  static const unsigned char header[] = {0, 'a', 's', 'm', 1, 0, 0, 0};
  static const unsigned char imports[] = {1, 4, 't', 'e', 's', 't', 4, 'w', 'o', 'r', 'd', 0, 0};
  static const unsigned char functions[] = {1, 1};
  static const unsigned char exports[] = {1, 1, 'x', 0, 1};
  static unsigned char body[MODULE_CAP], code[MODULE_CAP];
  unsigned char types[16];
  size_t at = sizeof header, n = 0, b = 0, c = 0;
  unsigned i;
  memcpy(bytes, header, sizeof header);
  types[n++] = 2;
  types[n++] = 0x60;
  types[n++] = (unsigned char)params;
  if (params)
    types[n++] = (unsigned char)param_type;
  types[n++] = 1;
  types[n++] = (unsigned char)result_type;
  types[n++] = 0x60;
  types[n++] = 0;
  types[n++] = 1;
  types[n++] = (unsigned char)result_type;
  section(bytes, &at, 1, types, n);
  section(bytes, &at, 2, imports, sizeof imports);
  section(bytes, &at, 3, functions, sizeof functions);
  section(bytes, &at, 7, exports, sizeof exports);
  body[b++] = 0;
  for (i = 0; i < pushes; i++) {
    body[b++] = (unsigned char)(param_type == I32 ? 0x41 : 0x42);
    body[b++] = 7;
  }
  body[b++] = 0x10;
  body[b++] = 0;
  body[b++] = 0x0f;
  body[b++] = 0x0b;
  code[c++] = 1;
  leb(code, &c, b);
  memcpy(code + c, body, b);
  c += b;
  section(bytes, &at, 10, code, c);
  return at;
}

static int signature(unsigned param_type, unsigned result_type, int accepts) {
  static unsigned char bytes[MODULE_CAP];
  EngineHostFunc fn = {"test", "word", 1, word};
  EngineBinding binding = {&fn, 1, NULL};
  Engine e;
  uint64_t result = 0;
  size_t size = module(bytes, 1, param_type, result_type, 1);
  EngineResult status = engine_load(&e, bytes, size, &binding);
  int ok;
  if (accepts)
    ok = status == ENGINE_OK && engine_call(&e, 1, NULL, 0, &result) == ENGINE_OK && result == 7;
  else
    ok = status == ENGINE_REFUSE && strcmp(engine_message(&e), "import type") == 0;
  if (!ok)
    fprintf(stderr, "engine api: signature %x -> %x: load %d, message '%s'\n",
            param_type, result_type, status, engine_message(&e));
  engine_free(&e);
  return ok;
}

static int full_stack(void) {
  static unsigned char bytes[MODULE_CAP];
  EngineHostFunc fn = {"test", "word", 0, word};
  EngineBinding binding = {&fn, 1, NULL};
  Engine e;
  uint64_t result = 0, *stack;
  size_t size = module(bytes, 0, I64, I64, ENGINE_STACK_MAX);
  int ok;
  if (engine_load(&e, bytes, size, &binding) != ENGINE_OK) {
    fprintf(stderr, "engine api: full stack load: %s\n", engine_message(&e));
    engine_free(&e);
    return 0;
  }
  /* A guard word observes an overflow without accessing outside the allocation. */
  stack = realloc(e.stack, (ENGINE_STACK_MAX + 1u) * sizeof *stack);
  if (!stack) {
    engine_free(&e);
    return 0;
  }
  e.stack = stack;
  stack[ENGINE_STACK_MAX] = UINT64_MAX;
  calls = 0;
  ok = engine_call(&e, 1, NULL, 0, &result) == ENGINE_HOST &&
       strcmp(engine_message(&e), "value stack bound") == 0 &&
       stack[ENGINE_STACK_MAX] == UINT64_MAX && calls == 0;
  if (!ok)
    fprintf(stderr, "engine api: full stack: guard %llu, calls %u, message '%s'\n",
            (unsigned long long)stack[ENGINE_STACK_MAX], calls, engine_message(&e));
  engine_free(&e);
  return ok;
}

int main(void) {
  int ok = signature(I64, I64, 1);
  ok = signature(I32, I64, 0) && ok;
  ok = signature(I64, I32, 0) && ok;
  ok = full_stack() && ok;
  if (ok)
    puts("engine api: 4 rows passed");
  return ok ? 0 : 1;
}
