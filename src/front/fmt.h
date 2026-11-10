/* `langc fmt` (S14 A1): the canonical text of a program. */
#ifndef LANG_FRONT_FMT_H
#define LANG_FRONT_FMT_H

#include <stdio.h>

#include "front/parser.h"

/* Lexes and parses the program TEXT as `langc check` does, then writes its
   canonical text to OUT. With CHECK, writes nothing and compares the
   canonical text with TEXT. Returns the exit status: 0, or 1 after a
   diagnostic (a parse refusal, FMT_TRAILING_COMMENT, FMT_DIFF). */
int fmt_program(Arena *arena, const char *source, const char *text, size_t len, int check, FILE *out, Diag *diag);

#endif
