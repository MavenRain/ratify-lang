/* The JSON report of `langc report` (S11, T4; Q-S11-1 (a), Q-S11-2 (a)).
   See docs/REPORT-JSON.md. */
#ifndef RATIFY_FRONT_REPORT_H
#define RATIFY_FRONT_REPORT_H

#include <stdio.h>

#include "front/core.h"

/* Writes the report of the checked program on M to OUT: JSON Lines, each
   line one object in its RFC 8785 (JCS) form, then '\n'. Each line has
   "ratify":1 and "kind". The lines are the program line, then one rule
   line for each decided rule def in def order. M must come from
   check_program_facts. Returns 1, or 0 after a diagnostic. */
int report_program(Machine *m, FILE *out);
/* S12 (Q-S12-5 (a)): writes the T2 TFHE circuits of the checked program
   on M to OUT: JSON Lines in the RFC 8785 (JCS) form, the program line,
   then one circuit line for each decided rule def in def order (see
   front/tfhe.h). All circuits are built before the first line: a
   circuit above a cap refuses the program with REFUSE_DEPTH and writes
   no line; D > B is an INTERNAL fault. M must come from
   check_program_facts. Returns 1, or 0 after a diagnostic. */
int report_tfhe(Machine *m, FILE *out);

/* S15 A2 (Q-S15-1 (a)): writes the one line of `langc check --json`
   (docs/CHECK-JSON.md) to OUT. M is the machine of check_program_facts, or
   NULL when the run stopped before the check (IO, LEX, PARSE). DIAG is the
   run diagnostic (set = the run failed). PARSED is check_program_count of
   the parse (0 when M is NULL). OWN takes a failure of the writer itself.
   Returns 1, or 0 after a diagnostic on OWN. */
int report_check(Arena *arena, Machine *m, const Diag *diag, uint32_t parsed, Diag *own, FILE *out);

#endif
