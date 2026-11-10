/* S12 (Q-S12-1 to Q-S12-4 (a)): the T2 TFHE tally circuit of one decided
   rule def, built from its RuleFacts.

   Inputs. One bit per member, class order then member order: wire w in
   [0, I) with I = sum n_j. Gate row g writes wire I + g and reads only
   lower wires. Only the accept bit leaves the circuit (OI-5).

   Gates. AND, OR, XOR, NAND, NOR, XNOR read two wires and cost one
   bootstrap each. NOT reads one wire (B = A in its row) and costs no
   bootstrap. Constants fold at build time, so no row reads a constant.

   Form (Q-S12-2 (a)). O3 makes each decided rule upward closed, so
   accept(c) = OR over the minimal vectors m of AND over j of (c_j >= m_j).
   The stages: a popcount per class, the constant comparator chain per
   (class, threshold) (LSB up: g = 1, then g = c_i AND g when m_i = 1, else
   c_i OR g; m_j = 0 drops the class term), a balanced AND tree per minimal
   vector, a balanced OR tree over the minimal vectors.

   Depth (O6). Per gate the noise depth is 1: each gate reads inputs or
   bootstrap outputs. D = the longest bootstrap chain from an input to the
   output (NOT adds 0). The static bound B comes from n_j, k and M (the
   min_count) before the build:

     B    = max_j T(n_j) + clog2(k) + clog2(M)
     T(n) = P(n) + b(n) - 1 for n >= 1, T(0) = 0
     P(n) = P(ceil(n / 2)) + 2 (b(n) - 1) for n >= 2, P(0) = P(1) = 0
     b(n) = ceil(log2(n + 1)), the bits of a count in [0, n]
     clog2(x) = ceil(log2 x) for x >= 1, clog2(0) = 0

   P bounds the popcount. A count over n bits adds the counts of the
   halves ceil(n / 2) and floor(n / 2) with a ripple-carry adder of b(n)
   bits (no carry out of the top bit). With inputs at depth d, carry i is
   at depth d + 1 + 2 i at most and sum bit i at d + 2 i (bit 0 at d + 1),
   so the top bit is at d + 2 (b(n) - 1). P is monotone (induction on n),
   so the larger half bounds both. The comparator chain adds one gate per
   count bit above bit 0, so T. The AND tree has at most k leaves and the
   OR tree M leaves. Folding only removes gates, so D <= B; D > B is a
   writer fault.

   Caps. More than TFHE_GATE_MAX gates, or a min_count above TFHE_MIN_CAP
   (core.h), refuses the build (REFUSE_DEPTH at `langc tfhe` only). */
#ifndef RATIFY_FRONT_TFHE_H
#define RATIFY_FRONT_TFHE_H

#include <stdint.h>

#include "front/core.h"

#define TFHE_GATE_MAX 65536u

typedef enum { TFHE_AND, TFHE_OR, TFHE_XOR, TFHE_NAND, TFHE_NOR, TFHE_XNOR, TFHE_NOT } TfheOp;

/* One gate row: OP reads wires A and B (B = A for NOT). */
typedef struct {
  TfheOp op;
  uint32_t a;
  uint32_t b;
} TfheGate;

/* INPUTS member bits, GATE_COUNT rows, the accept wire OUT, the NOTS rows
   with no bootstrap, D (DEPTH) and B (BOUND). */
typedef struct {
  uint32_t inputs;
  uint32_t gate_count;
  TfheGate *gates;
  uint32_t out;
  uint32_t nots;
  uint32_t depth;
  uint32_t bound;
} TfheCircuit;

/* REFUSE_GATES and REFUSE_MIN are the caps; FAULT_DEPTH (D > B) and
   FAULT_SHAPE (a constant output, or a kept list shorter than min_count)
   are writer faults; OOM is an arena failure. */
typedef enum {
  TFHE_OK,
  TFHE_REFUSE_GATES,
  TFHE_REFUSE_MIN,
  TFHE_FAULT_DEPTH,
  TFHE_FAULT_SHAPE,
  TFHE_OOM
} TfheResult;

/* OP on the bits A and B (B is not read for NOT). */
int tfhe_op_eval(TfheOp op, int a, int b);
const char *tfhe_op_name(TfheOp op);
/* B for FACTS, from n_j, k and min_count alone. */
uint32_t tfhe_bound(const RuleFacts *facts);
/* D and the NOT rows of CIRCUIT. Returns 0 on OOM. */
int tfhe_depth(Arena *arena, const TfheCircuit *circuit, uint32_t *depth, uint32_t *nots);
/* Build the circuit of FACTS into CIRCUIT (gates from ARENA). */
TfheResult tfhe_build(Arena *arena, const RuleFacts *facts, TfheCircuit *circuit);

#endif
