/*
 * The bodies of the six `ratify` imports (S10 U2, Q-S10-U2-3 (a)), over the
 * host region of HOST_MEM bytes (src/hostmem.h). test/cryptotool.c and
 * host/ratifyd.c link host/imports.c.
 */
#ifndef RATIFY_IMPORTS_H
#define RATIFY_IMPORTS_H

#include <stdint.h>

#include "hostmem.h"

/* The result of the import J (0 to 5, PLAN 8.2) on the words W over the
   region MEM. An operand that does not lie in [0, HOST_MEM), or a size other
   than the fixed one, gives 0, and no byte of MEM is read before the check.
   tally_add (J 4) reads its operands before it writes the 128 bytes at W[0].
   A J above 5 gives 0. */
uint64_t host_import(uint32_t j, const uint64_t *w, unsigned char mem[HOST_MEM]);

#endif
