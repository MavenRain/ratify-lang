/* The fix table of `langc check --json` (S15 A2, Q-S15-3 (a)): one fix
   hint for each diagnostic code string of src/front and src/main.c, and IO
   (main.c) and REFUSE_FORMER (PLAN sec 7). The 11 sec 7 codes start with the
   sec 7 text (docs/PLAN.md sec 7). docs/CHECK-JSON.md has the rule. */
#ifndef RATIFY_FRONT_FIX_H
#define RATIFY_FRONT_FIX_H

#include <stddef.h>

/* The fix hint of CODE, or NULL when CODE has no row. */
const char *fix_of(const char *code);

/* The number of rows. */
size_t fix_count(void);

/* The code of row I, or NULL when I is not below fix_count(). */
const char *fix_code(size_t i);

#endif
