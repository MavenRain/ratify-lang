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

#endif
