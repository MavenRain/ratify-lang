/* Arena allocation and the diagnostic sink. SHARED by the tcc kits. */
#ifndef LANG_FRONT_BASE_H
#define LANG_FRONT_BASE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct ArenaChunk ArenaChunk;

/* Every compiler object lives in one arena. The arena frees all at once. */
typedef struct {
  ArenaChunk *head;
  size_t used_bytes;
  size_t limit_bytes;
} Arena;

void arena_init(Arena *arena, size_t limit_bytes);
/* Returns zeroed memory, or NULL when the limit is reached. */
void *arena_alloc(Arena *arena, size_t size);
char *arena_strndup(Arena *arena, const char *text, size_t len);
void arena_release(Arena *arena);

/* The first failure wins. Later failures do not overwrite it. The winning
   failure writes every field, so no field of an older failure stays after a
   reset of `set`. */
typedef struct {
  int set;
  const char *code;
  char def[64];
  char msg[512];
  /* The start of the span, FILE:LINE:COL (S15). LINE 0 = no span. */
  const char *file;
  int line;
  int col;
  /* The expected / found pair (S15). An empty string = no pair. */
  char expected[256];
  char found[256];
} Diag;

void diag_init(Diag *diag);
/* Records the failure and returns 0, so a caller can write
   `return diag_fail(...)`. DEF is NULL when no definition applies. */
int diag_fail(Diag *diag, const char *code, const char *def, const char *fmt, ...);
/* As diag_fail, with the span start FILE:LINE:COL (LINE 0 = no span) and the
   expected / found pair (NULL = no pair). */
int diag_fail_at(Diag *diag, const char *code, const char *def, const char *file, int line, int col, const char *expected, const char *found, const char *fmt, ...);
/* Gives the winning failure the span FILE:LINE:COL when it has no span yet.
   Returns 0, as diag_fail. */
int diag_span(Diag *diag, const char *file, int line, int col);
/* Prints `langc: CODE: DEF: message`. */
void diag_print(const Diag *diag, FILE *out);

#endif
