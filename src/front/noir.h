/* The T3 Noir text of `langc noir PROG` (S13, Q-S0-13 (b), Q-S13-3 (c)). */
#ifndef RATIFY_FRONT_NOIR_H
#define RATIFY_FRONT_NOIR_H

#include <stdio.h>

#include "front/core.h"

/* Writes the T3 Noir library module of the checked program on M to OUT:
   the relations r_ballot and r_dec over BN254 (noir_bigcurve, Q-S13-1
   (a)), and for each decided rule def DEF in def order the global K_<DEF>,
   its member count (the sum of its class sizes, Q-S13-8 (a)). The K name
   is "K_", then DEF in upper case with '_' for each char not in
   [A-Za-z0-9_]. Two decided rule defs with one K name, or a member count
   above the u32 range, refuse the program with REFUSE_NOIR and write no
   text. M must come from check_program_facts. Returns 1, or 0 after a
   diagnostic. */
int report_noir(Machine *m, FILE *out);

#endif
