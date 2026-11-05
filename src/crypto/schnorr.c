/* Schnorr signatures over BN254 G1 with Keccak-256 (slice S7, Q-S7-3 (a)).
   See crypto/schnorr.h. */
#include <stdlib.h>
#include <string.h>

#include "crypto/schnorr.h"

#include "crypto/bn254.h"
#include "crypto/elgamal.h"
#include "crypto/keccak.h"

/* 1 when the SIZE bytes at P are all 0. The byte form of the point at
   infinity is 64 zero bytes (EIP-196), and g1_from_be takes only
   coordinates below p, so this test is exact. */
static int all_zero(const unsigned char *p, size_t size) {
  size_t i;
  unsigned char acc = 0;
  for (i = 0; i < size; i++)
    acc |= p[i];
  return acc == 0;
}

/* keccak256(PREFIX || M) mod r at OUT. Returns 1, or 0 when there is no
   memory. */
static int hash_scalar(U256 *out, const unsigned char *prefix, size_t nprefix, const unsigned char *m, size_t size) {
  unsigned char digest[32];
  unsigned char *buf = malloc(nprefix + size + 1u);
  if (buf == NULL)
    return 0;
  memcpy(buf, prefix, nprefix);
  if (size > 0u)
    memcpy(buf + nprefix, m, size);
  keccak256(buf, nprefix + size, digest);
  free(buf);
  u256_from_be(out, digest);
  scalar_reduce(out, out);
  return 1;
}

static int is_zero(const U256 *x) {
  U256 zero;
  u256_from_u64(&zero, 0);
  return u256_cmp(x, &zero) == 0;
}

int schnorr_public(unsigned char pk[64], const U256 *sk) {
  U256 x;
  G1 p;
  scalar_reduce(&x, sk);
  if (is_zero(&x))
    return 0;
  elg_public(&p, &x);
  g1_to_be(pk, &p);
  return 1;
}

int schnorr_sign(unsigned char sig[96], const U256 *sk, const unsigned char *m, size_t size) {
  U256 x, k, e, s;
  unsigned char skbe[32];
  unsigned char rp[128];
  G1 g, r, p;
  Fe fk, fe, fx, fex, fs;
  scalar_reduce(&x, sk);
  if (is_zero(&x))
    return 0;
  u256_to_be(skbe, &x);
  if (!hash_scalar(&k, skbe, 32u, m, size))
    return 0;
  if (is_zero(&k))
    return 0;
  elg_generator(&g);
  g1_mul(&r, &g, &k);
  g1_mul(&p, &g, &x);
  g1_to_be(rp, &r);
  g1_to_be(rp + 64, &p);
  if (!hash_scalar(&e, rp, 128u, m, size))
    return 0;
  /* Each operand is below r, so fe_from_u256 takes it. */
  if (!fe_from_u256(&fk, &k, &BN254_FR) || !fe_from_u256(&fe, &e, &BN254_FR) || !fe_from_u256(&fx, &x, &BN254_FR))
    return 0;
  fe_mul(&fex, &fe, &fx, &BN254_FR);
  fe_add(&fs, &fk, &fex, &BN254_FR);
  fe_to_u256(&s, &fs);
  memcpy(sig, rp, 64u);
  u256_to_be(sig + 64, &s);
  return 1;
}

int schnorr_verify(const unsigned char pk[64], const unsigned char *m, size_t size, const unsigned char sig[96]) {
  U256 s, sr, e;
  unsigned char rp[128];
  unsigned char lhs[64];
  unsigned char rhs[64];
  G1 g, p, r, sg, ep, rep;
  if (all_zero(pk, 64u) || all_zero(sig, 64u))
    return 0;
  if (!g1_from_be(&p, pk) || !g1_from_be(&r, sig))
    return 0;
  u256_from_be(&s, sig + 64);
  scalar_reduce(&sr, &s);
  if (u256_cmp(&sr, &s) != 0)
    return 0;
  memcpy(rp, sig, 64u);
  memcpy(rp + 64, pk, 64u);
  if (!hash_scalar(&e, rp, 128u, m, size))
    return 0;
  elg_generator(&g);
  g1_mul(&sg, &g, &s);
  g1_mul(&ep, &p, &e);
  g1_add(&rep, &r, &ep);
  g1_to_be(lhs, &sg);
  g1_to_be(rhs, &rep);
  return memcmp(lhs, rhs, 64u) == 0;
}
