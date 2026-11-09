/* Checks the string escapes of src/jsonout.c (S11 U4, Q-S11-10 (a)). Each
 * row writes its bytes through out_bytes and out_line to a temp file and
 * compares the file with the RFC 8785 (JCS) form of the line. No report
 * string of langc has '"', '\\' or a control byte (def names match
 * [A-Za-z_][A-Za-z0-9_']*), so only this tool sees these escapes. */
#include <stdio.h>
#include <string.h>

#include "jsonout.h"

/* The LEN bytes at TEXT go in the string of the line {"s":...}. WANT is the
 * JCS form of that line and its '\n'. */
typedef struct {
  const char *name;
  const char *text;
  size_t len;
  const char *want;
} Row;

static const Row ROWS[] = {
    {"quote", "a\"b", 3u, "{\"s\":\"a\\\"b\"}\n"},
    {"backslash", "a\\b", 3u, "{\"s\":\"a\\\\b\"}\n"},
    {"newline", "a\nb", 3u, "{\"s\":\"a\\nb\"}\n"},
    {"control", "a\001b", 3u, "{\"s\":\"a\\u0001b\"}\n"},
    {"bad utf-8", "a\377b", 3u, "{\"s\":\"a\357\277\275b\"}\n"},
};

/* Runs ROW. Returns 1, or 0 after a FAIL line. */
static int run(const Row *row) {
  Arena arena;
  Diag diag;
  JsonOut out = {&arena, &diag, row->name, NULL, 0, 0};
  FILE *file = tmpfile();
  char got[64];
  size_t want = strlen(row->want);
  size_t n = 0;
  int ok;
  arena_init(&arena, 1u << 20);
  diag_init(&diag);
  ok = file != NULL && out_text(&out, "{\"s\":\"") && out_bytes(&out, row->text, row->len) && out_text(&out, "\"}") &&
       out_line(&out, file) && fseek(file, 0L, SEEK_SET) == 0;
  if (ok)
    n = fread(got, 1u, sizeof got, file);
  ok = ok && n == want && memcmp(got, row->want, want) == 0;
  if (!ok)
    fprintf(stderr, "FAIL json %s: the line is not the JCS form\n", row->name);
  if (file != NULL)
    fclose(file);
  arena_release(&arena);
  return ok;
}

int main(void) {
  size_t count = sizeof ROWS / sizeof ROWS[0];
  size_t passed = 0;
  size_t i;
  for (i = 0; i < count; i++)
    passed += (size_t)run(&ROWS[i]);
  if (passed != count)
    return 1;
  printf("json escapes: %u checked\n", (unsigned)count);
  return 0;
}
