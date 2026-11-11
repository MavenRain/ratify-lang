/* The rows of `langc check --json` and its fix table (S15 U3, Q-S15-6 (a),
 * D-U2-7). Writes 3 count lines. A failed check writes a FAIL line, and the
 * tool exits 1.
 *
 * json rows: N checked. Each row of test/json/rows.txt (MODE RC PROG NAME)
 * runs `build/langc [--kit] check PROG --json` from the repo root. The exit
 * code is RC, stderr is empty, and stdout is test/json/NAME.json byte for
 * byte. The golden is its own RFC 8785 form: json_parse, then jcs_write,
 * then a newline give the same bytes.
 *
 * json fix rows: N checked. Each row of the PLAN section 7 table
 * (docs/PLAN.md), less the rows that only `langc tfhe` and `langc noir`
 * give, has a fix row that starts with the section 7 text (Q-S15-3 (a)).
 *
 * json fix table: N codes. N = fix_count(). The rows are sorted by code.
 * The code string at each diag_fail, diag_fail_at, FAIL, FAIL_PAIR and
 * kit_refused site of src/front and src/main.c has a row (D-U2-1). */
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "front/base.h"
#include "front/fix.h"
#include "jcs.h"

extern char **environ;

#define OUT_PATH "build/checkjson.out"
#define ERR_PATH "build/checkjson.err"

static const char *const SITES[] = {"diag_fail(", "diag_fail_at(", "FAIL(", "FAIL_PAIR(", "kit_refused("};

/* Writes the FAIL line of CHECK for WHAT. Returns 0. */
static int fail(const char *check, const char *what, const char *why) {
  printf("FAIL %s %s: %s\n", check, what, why);
  return 0;
}

/* Reads PATH into ARENA; a NUL follows the LEN bytes. Returns the bytes, or
 * NULL when PATH cannot be read or the arena is full. */
static char *slurp(Arena *arena, const char *path, size_t *len) {
  FILE *file = fopen(path, "rb");
  char *text = NULL;
  long size;
  if (file == NULL)
    return NULL;
  if (fseek(file, 0, SEEK_END) == 0 && (size = ftell(file)) >= 0 && fseek(file, 0, SEEK_SET) == 0) {
    text = arena_alloc(arena, (size_t)size + 1);
    if (text != NULL && fread(text, 1, (size_t)size, file) == (size_t)size)
      *len = (size_t)size;
    else
      text = NULL;
  }
  fclose(file);
  return text;
}

/* Runs ARGS with stdout at OUT_PATH and stderr at ERR_PATH. Returns the exit
 * code, or -1 when the spawn fails or the child does not exit. */
static int run(char *const args[]) {
  posix_spawn_file_actions_t actions;
  pid_t pid, waited;
  int status, ok;
  if (posix_spawn_file_actions_init(&actions) != 0)
    return -1;
  ok = posix_spawn_file_actions_addopen(&actions, 1, OUT_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644) == 0 &&
       posix_spawn_file_actions_addopen(&actions, 2, ERR_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644) == 0 &&
       posix_spawn(&pid, args[0], &actions, NULL, args, environ) == 0;
  posix_spawn_file_actions_destroy(&actions);
  if (!ok)
    return -1;
  do {
    waited = waitpid(pid, &status, 0);
  } while (waited < 0 && errno == EINTR);
  return waited == pid && WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* 1 when the LEN bytes of TEXT are one JSON value in its RFC 8785 form and
 * a newline: json_parse, then jcs_write give the bytes before the newline. */
static int jcs_same(const char *text, size_t len) {
  static unsigned char bytes[1u << 16];
  Arena arena;
  Diag diag;
  JsonValue value;
  JcsBuf buf;
  int same = 0;
  memset(&diag, 0, sizeof diag);
  buf.bytes = bytes;
  buf.cap = sizeof bytes;
  buf.len = 0;
  arena_init(&arena, 1u << 22);
  if (len > 0 && text[len - 1] == '\n' && json_parse(&arena, text, len - 1, &value, &diag) &&
      jcs_write(&value, &buf) == JCS_OK)
    same = buf.len == len - 1 && memcmp(buf.bytes, text, len - 1) == 0;
  arena_release(&arena);
  return same;
}

/* Checks one row of test/json/rows.txt. Returns 1, or 0 after a FAIL line. */
static int check_row(Arena *arena, char *mode, char *rc, char *prog, char *name) {
  char golden[256];
  char *args[6];
  char *want, *out, *err;
  size_t want_len = 0, out_len = 0, err_len = 0, n = 0;
  int code;
  args[n++] = "build/langc";
  if (strcmp(mode, "kit") == 0)
    args[n++] = "--kit";
  else if (strcmp(mode, "guest") != 0)
    return fail("json row", name, "the mode is not guest or kit");
  args[n++] = "check";
  args[n++] = prog;
  args[n++] = "--json";
  args[n] = NULL;
  if (strlen(rc) != 1 || !isdigit((unsigned char)rc[0]))
    return fail("json row", name, "the exit code is not one digit");
  snprintf(golden, sizeof golden, "test/json/%s.json", name);
  want = slurp(arena, golden, &want_len);
  if (want == NULL)
    return fail("json row", name, "cannot read the golden");
  if (!jcs_same(want, want_len))
    return fail("json row", name, "the golden is not its RFC 8785 form and a newline");
  code = run(args);
  if (code != rc[0] - '0') {
    printf("FAIL json row %s: want exit %c, got %d\n", name, rc[0], code);
    return 0;
  }
  out = slurp(arena, OUT_PATH, &out_len);
  err = slurp(arena, ERR_PATH, &err_len);
  if (out == NULL || err == NULL)
    return fail("json row", name, "cannot read the output");
  if (err_len != 0)
    return fail("json row", name, "stderr is not empty");
  if (out_len != want_len || memcmp(out, want, want_len) != 0)
    return fail("json row", name, "stdout is not the golden");
  return 1;
}

/* Checks each row of test/json/rows.txt; COUNT gets the row count. Empty
 * lines and `#` lines are not rows. Returns 1, or 0 after a FAIL line. */
static int check_rows(Arena *arena, size_t *count) {
  size_t len = 0;
  char *text = slurp(arena, "test/json/rows.txt", &len);
  char *line, *next;
  if (text == NULL)
    return fail("json rows", "test/json/rows.txt", "cannot read");
  for (line = text; *line != '\0'; line = next) {
    char *field[4];
    char *at = line;
    size_t n = 0;
    next = strchr(line, '\n');
    if (next == NULL)
      next = line + strlen(line);
    else
      *next++ = '\0';
    if (*line == '\0' || *line == '#')
      continue;
    while (n < 4 && at != NULL) {
      field[n++] = at;
      at = strchr(at, ' ');
      if (at != NULL)
        *at++ = '\0';
    }
    if (n != 4 || at != NULL)
      return fail("json rows", line, "a row is MODE RC PROG NAME");
    if (!check_row(arena, field[0], field[1], field[2], field[3]))
      return 0;
    *count += 1;
  }
  return *count > 0 || fail("json rows", "test/json/rows.txt", "no row");
}

/* Checks the fix row of each PLAN section 7 code; COUNT gets the code
 * count. A table row is `| `CODE` | TEXT | DESIGN |`. The rows whose TEXT
 * starts with "At `langc " are not check codes. Returns 1, or 0 after a
 * FAIL line. */
static int check_fix_rows(Arena *arena, size_t *count) {
  size_t len = 0;
  char *plan = slurp(arena, "docs/PLAN.md", &len);
  char *at, *end;
  if (plan == NULL)
    return fail("json fix rows", "docs/PLAN.md", "cannot read");
  at = strstr(plan, "\n## 7. ");
  end = at == NULL ? NULL : strstr(at + 1, "\n## 8. ");
  if (at == NULL || end == NULL)
    return fail("json fix rows", "docs/PLAN.md", "no section 7");
  *end = '\0';
  for (at = strstr(at, "\n| `"); at != NULL; at = strstr(at, "\n| `")) {
    char *code = at + 4;
    char *code_end = strchr(code, '`');
    char *text, *text_end;
    const char *fix;
    if (code_end == NULL || strncmp(code_end, "` | ", 4) != 0)
      return fail("json fix rows", "docs/PLAN.md", "a section 7 row without a code cell");
    *code_end = '\0';
    text = code_end + 4;
    text_end = strstr(text, " | ");
    if (text_end == NULL)
      return fail("json fix rows", code, "the section 7 row has no text cell");
    *text_end = '\0';
    at = text_end + 1;
    if (strncmp(text, "At `langc ", 10) == 0)
      continue;
    fix = fix_of(code);
    if (fix == NULL)
      return fail("json fix rows", code, "no fix row");
    if (strncmp(fix, text, strlen(text)) != 0)
      return fail("json fix rows", code, "the fix row does not start with the section 7 text");
    *count += 1;
  }
  return *count > 0 || fail("json fix rows", "docs/PLAN.md", "no section 7 row");
}

/* Copies the first string literal between AT and the next ';' to CODE when
 * it is a code: [A-Z][A-Z0-9_]+ in fewer than CAP bytes. Returns 1 then,
 * else 0. */
static int literal_code(const char *at, char *code, size_t cap) {
  const char *end = strchr(at, ';');
  const char *quote = strchr(at, '"');
  size_t n = 0;
  if (quote == NULL || (end != NULL && quote > end))
    return 0;
  for (quote++; *quote != '"'; quote++) {
    if (n + 1 >= cap || !(isupper((unsigned char)*quote) || (n > 0 && (isdigit((unsigned char)*quote) || *quote == '_'))))
      return 0;
    code[n++] = *quote;
  }
  code[n] = '\0';
  return n >= 2;
}

/* Checks the code at each site of PATH; SITE_COUNT counts the sites with a
 * code. Returns 1, or 0 after a FAIL line. */
static int scan_file(Arena *arena, const char *path, size_t *site_count) {
  size_t len = 0, s;
  char *text = slurp(arena, path, &len);
  if (text == NULL)
    return fail("json fix table", path, "cannot read");
  for (s = 0; s < sizeof SITES / sizeof SITES[0]; s++) {
    const char *at;
    for (at = strstr(text, SITES[s]); at != NULL; at = strstr(at + 1, SITES[s])) {
      char code[64];
      if (at > text && (isalnum((unsigned char)at[-1]) || at[-1] == '_'))
        continue;
      if (!literal_code(at + strlen(SITES[s]), code, sizeof code))
        continue;
      *site_count += 1;
      if (fix_of(code) == NULL) {
        printf("FAIL json fix table %s: %s has no fix row\n", path, code);
        return 0;
      }
    }
  }
  return 1;
}

/* Scans each .c and .h file of DIR. Returns 1, or 0 after a FAIL line. */
static int scan_dir(Arena *arena, const char *dir, size_t *site_count) {
  DIR *handle = opendir(dir);
  struct dirent *entry;
  char path[512];
  int ok = 1;
  if (handle == NULL)
    return fail("json fix table", dir, "cannot open");
  while (ok && (entry = readdir(handle)) != NULL) {
    size_t n = strlen(entry->d_name);
    if (n > 2 && entry->d_name[n - 2] == '.' && (entry->d_name[n - 1] == 'c' || entry->d_name[n - 1] == 'h')) {
      snprintf(path, sizeof path, "%s/%s", dir, entry->d_name);
      ok = scan_file(arena, path, site_count);
    }
  }
  closedir(handle);
  return ok;
}

/* Checks the order of the fix table and the code at each diag site.
 * Returns 1, or 0 after a FAIL line. */
static int check_fix_table(Arena *arena) {
  size_t count = fix_count(), site_count = 0, i;
  if (count == 0 || fix_code(count) != NULL)
    return fail("json fix table", "src/front/fix.c", "fix_count and fix_code disagree");
  for (i = 0; i < count; i++) {
    if (fix_code(i) == NULL || fix_of(fix_code(i)) == NULL)
      return fail("json fix table", "src/front/fix.c", "a row has no code or no hint");
    if (i > 0 && strcmp(fix_code(i - 1), fix_code(i)) >= 0)
      return fail("json fix table", fix_code(i), "the rows are not sorted by code");
  }
  if (!scan_file(arena, "src/main.c", &site_count) || !scan_dir(arena, "src/front", &site_count))
    return 0;
  return site_count > 0 || fail("json fix table", "src/front", "no diag site");
}

int main(void) {
  Arena arena;
  size_t rows = 0, fix_rows = 0;
  int ok = 1;
  arena_init(&arena, 1u << 26);
  if (check_rows(&arena, &rows))
    printf("json rows: %lu checked\n", (unsigned long)rows);
  else
    ok = 0;
  if (check_fix_rows(&arena, &fix_rows))
    printf("json fix rows: %lu checked\n", (unsigned long)fix_rows);
  else
    ok = 0;
  if (check_fix_table(&arena))
    printf("json fix table: %lu codes\n", (unsigned long)fix_count());
  else
    ok = 0;
  arena_release(&arena);
  return ok ? 0 : 1;
}
