/* Schnorr signatures over BN254 G1 with Keccak-256 (slice S7, Q-S7-3 (a)).
   New C on the bn254, elgamal and keccak API; quorum-lang has no Schnorr
   on BN254 (its frost.c is FROST over secp256k1).

   A public key is 64 bytes, x then y, big endian (the g1_to_be form). A
   signature is 96 bytes: R.x || R.y || s, big endian. The challenge is
   e = keccak256(R || P || m) mod r. */
#ifndef LANG_CRYPTO_SCHNORR_H
#define LANG_CRYPTO_SCHNORR_H

#include <stddef.h>

#include "crypto/u256.h"

/* The public key (sk mod r) G at PK. Returns 1, or 0 when sk mod r is 0. */
int schnorr_public(unsigned char pk[64], const U256 *sk);
/* Signs the SIZE bytes at M with SK. The nonce is k = keccak256(sk || m)
   mod r, with sk the 32 bytes of sk mod r: a fixture nonce, so the
   known-answer rows can pin the output. s = k + e sk mod r. Returns 1, or
   0 when sk mod r or k is 0, or when there is no memory. */
int schnorr_sign(unsigned char sig[96], const U256 *sk, const unsigned char *m, size_t size);
/* 1 when SIG is a signature of the SIZE bytes at M under PK: P and R are
   on the curve and not the point at infinity, s < r, and s G = R + e P.
   Else 0. */
int schnorr_verify(const unsigned char pk[64], const unsigned char *m, size_t size, const unsigned char sig[96]);

#endif
