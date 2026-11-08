/*
 * The bodies of the six `ratify` imports (S10 U2, Q-S10-U2-3 (a)). They
 * moved from the host_import of test/cryptotool.c (S9 step d2, the ABI of
 * s9/abi.txt and PLAN 8.2).
 */
#include <string.h>

#include "crypto/elgamal.h"
#include "crypto/keccak.h"
#include "crypto/schnorr.h"
#include "crypto/sigma.h"
#include "crypto/threshold.h"
#include "imports.h"

/* 1 when the LEN bytes at OFF are in the region (no wrap), else 0. */
static int host_fits(uint64_t off, uint64_t len) {
  return off <= HOST_MEM && len <= HOST_MEM - off;
}

/* 1 when the operand pair P (OFF, LEN) fits and LEN is SIZE (SIZE 0 = any). */
static int host_operand(const uint64_t *p, uint64_t size) {
  return host_fits(p[0], p[1]) && (size == 0u || p[1] == size);
}

/* The bit length of N: the smallest b with 2^b above N (at most 64). */
static uint64_t host_bits(uint64_t n) {
  uint64_t b = 0;
  while (b < 64u && (UINT64_C(1) << b) <= n)
    b++;
  return b;
}

/* The result of the import J on the words W over MEM, by the bodies of
   s9/abi.txt. An operand that does not fit, or a size other than the fixed
   one, gives 0. */
uint64_t host_import(uint32_t j, const uint64_t *w, unsigned char mem[HOST_MEM]) {
  unsigned char digest[32];
  unsigned char a[128];
  unsigned char b[128];
  uint64_t m = 0;
  uint64_t k;
  uint64_t n;
  switch (j) {
  case 0u:
    return (host_operand(w, 64u) && host_operand(w + 2, 0u) && host_operand(w + 4, 96u) &&
            schnorr_verify(mem + w[0], mem + w[2], (size_t)w[3], mem + w[4]) == 1)
               ? 1u
               : 0u;
  case 1u:
    if (!host_operand(w, 0u) || !host_operand(w + 2, 32u))
      return 0u;
    keccak256(mem + w[0], (size_t)w[1], digest);
    return memcmp(digest, mem + w[2], 32u) == 0 ? 1u : 0u;
  case 2u:
    return (host_operand(w, 232u) && host_operand(w + 2, 64u) && host_operand(w + 4, 128u) &&
            thr_verify_be(mem + w[0], mem + w[2], mem + w[4]) == 0)
               ? 1u
               : 0u;
  case 3u:
    return (host_operand(w, 384u) && host_operand(w + 2, 64u) && host_operand(w + 4, 128u) &&
            host_operand(w + 6, 32u) &&
            sig_or_verify_be(mem + w[0], mem + w[2], mem + w[4], mem + w[6], w[8]) == SIG_OK)
               ? 1u
               : 0u;
  case 4u:
    if (!host_operand(w, 128u) || !host_operand(w + 2, 128u) || !host_operand(w + 4, 128u))
      return 0u;
    memcpy(a, mem + w[2], 128u);
    memcpy(b, mem + w[4], 128u);
    tally_add_be(mem + w[0], a, b, 1u);
    return 1u;
  case 5u:
    if (!host_operand(w, 0u) || !host_operand(w + 2, 0u) || !host_operand(w + 4, 128u) || w[1] % 232u != 0u ||
        w[3] % 64u != 0u)
      return 0u;
    k = w[1] / 232u;
    n = w[3] / 64u;
    return (thr_open_be(&m, mem + w[0], (uint32_t)k, mem + w[2], (uint32_t)n, mem + w[4], k, w[6], n,
                        host_bits(w[7])) == THR_FOUND &&
            m <= w[7])
               ? m + 1u
               : 0u;
  default:
    return 0u;
  }
}
