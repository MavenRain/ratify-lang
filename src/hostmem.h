/* The host region of a module with imports (S10, Q-S10-1 and Q-S10-2 (a)):
   the bytes [0, HOST_MEM) are below the heap, and the heap starts at
   HOST_MEM. The host writes the operand bytes in the region, and an operand
   must lie in it. One constant for src/wasm.c, test/cryptotool.c and
   host/ratifyd.c (Q-S10-U2-1 (a)). */
#ifndef LANG_HOSTMEM_H
#define LANG_HOSTMEM_H

#define HOST_MEM 65536u

#endif
