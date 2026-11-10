/* The T3 Noir text of `langc noir PROG` (S13, Q-S0-13 (b), Q-S13-3 (c)).
   See noir.h. The relations are fixed text (the API facts are in the S13
   noir-pins.txt); only the K globals come from the rule facts. */
#include <stdint.h>
#include <stdio.h>

#include "front/base.h"
#include "front/noir.h"

/* The text before the K globals: the header and the imports. */
static const char NOIR_HEAD[] =
    "// The T3 relations of ratify-lang as Noir (S13, Q-S0-13 (b)): R_ballot and\n"
    "// R_dec over BN254 with noir_bigcurve (Q-S13-1 (a)). `langc noir PROG`\n"
    "// writes this text. Do not edit it: the gate compares it byte for byte.\n"
    "use bigcurve::{BigCurve, BN254, BN254Scalar};\n"
    "use bignum::{BigNum, BN254_Fq};\n"
    "\n"
    "// K_<DEF> is the member count of the decided rule def DEF: the sum of its\n"
    "// class sizes (Q-S13-8 (a)). The ballot slot j of a member is below K.\n";

/* The text after the K globals: point, r_ballot (M1 to M4, M7) and r_dec
   (M5, M6). Each constraint is on its own line: one mutant site each. */
static const char NOIR_TAIL[] =
    "\n"
    "// A public point from its canonical coordinates. T1 encodes the identity\n"
    "// as (0, 0); all other points must satisfy the curve equation.\n"
    "pub fn point(x: BN254_Fq, y: BN254_Fq) -> BN254 {\n"
    "    x.validate_in_field();\n"
    "    y.validate_in_field();\n"
    "    BN254::from_coordinates(x, y, x.is_zero() & y.is_zero())\n"
    "}\n"
    "\n"
    "// R_ballot: (c1, c2) is the ElGamal encryption of the bit b under the\n"
    "// public key pk with the randomness r, and the slot j is below k. The\n"
    "// generator G is BN254::one() = (1, 2), the T1 generator. ctx binds the\n"
    "// proof to its context; no constraint reads it.\n"
    "pub fn r_ballot(k: u32, pk: BN254, c1: BN254, c2: BN254, _ctx: [u8; 32], j: u32, b: Field, r: Field) {\n"
    "    let g = BN254::one();\n"
    "    assert(b * (1 - b) == 0);\n"
    "    assert(g.mul(BN254Scalar::from(r)) == c1);\n"
    "    assert(g.mul(BN254Scalar::from(b)) + pk.mul(BN254Scalar::from(r)) == c2);\n"
    "    assert(j < k);\n"
    "}\n"
    "\n"
    "// R_dec: key = s G is the key share of the secret s, and d = s c1 is its\n"
    "// decryption share of the ciphertext with the first part c1.\n"
    "pub fn r_dec(key: BN254, c1: BN254, d: BN254, s: Field) {\n"
    "    let g = BN254::one();\n"
    "    assert(g.mul(BN254Scalar::from(s)) == key);\n"
    "    assert(c1.mul(BN254Scalar::from(s)) == d);\n"
    "}\n";

/* The char of the K name for the char C of a def name: upper case, and '_'
   for a char not in [A-Za-z0-9_]. */
static char k_char(char c) {
  int lower = c >= 'a' && c <= 'z';
  int keep = (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
  return lower ? (char)(c - 'a' + 'A') : keep ? c : '_';
}

/* 1 when the def names A and B give one K name. */
static int k_same(const char *a, const char *b) {
  size_t i = 0;
  while (a[i] != '\0' && b[i] != '\0' && k_char(a[i]) == k_char(b[i])) i++;
  return a[i] == '\0' && b[i] == '\0';
}

/* The member count of R: the sum of its K class sizes. */
static uint64_t k_total(const RuleFacts *r) {
  uint64_t total = 0;
  uint32_t j;
  for (j = 0; j < r->k; j++) total += r->n[j];
  return total;
}

/* Writes the line `pub global K_<DEF>: u32 = COUNT;` of R. */
static int out_k(const RuleFacts *r, FILE *file) {
  size_t i;
  int ok = fputs("pub global K_", file) >= 0;
  for (i = 0; ok && r->name[i] != '\0'; i++) ok = fputc(k_char(r->name[i]), file) != EOF;
  return ok && fprintf(file, ": u32 = %llu;\n", (unsigned long long)k_total(r)) > 0;
}

int report_noir(Machine *m, FILE *file) {
  uint32_t i;
  uint32_t a;
  int ok = 1;
  int written;
  /* All refusals come before the first byte of text. */
  for (i = 0; ok && i < m->rule_fact_count; i++) {
    for (a = 0; ok && a < i; a++)
      ok = !k_same(m->rule_facts[a].name, m->rule_facts[i].name) ||
           diag_fail(m->diag, "REFUSE_NOIR", m->rule_facts[i].name, "its K name is the K name of the rule def '%s'",
                     m->rule_facts[a].name);
    ok = ok && (k_total(&m->rule_facts[i]) <= UINT32_MAX ||
                diag_fail(m->diag, "REFUSE_NOIR", m->rule_facts[i].name, "its member count is above the u32 range"));
  }
  if (!ok) return 0;
  written = fputs(NOIR_HEAD, file) >= 0;
  for (i = 0; written && i < m->rule_fact_count; i++) written = out_k(&m->rule_facts[i], file);
  written = written && fputs(NOIR_TAIL, file) >= 0 && fflush(file) == 0;
  return written || diag_fail(m->diag, "IO", NULL, "cannot write the Noir text");
}
