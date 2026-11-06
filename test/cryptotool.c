/* Host crypto regressions (S7, S8): the known answers of the src/crypto port
   (keccak, G1, ElGamal, Schnorr), the invalid-signature rows, the threshold
   rows (DKG, open with DLEQ, the algebra row), the reshare rows, the
   invalid-proof rows and the certificate rows (S8: the body bytes, the
   height, prev hash and signature checks in the Q-S8-3 order, and the
   equivocation evidence). Each group prints one summary line. With the argument
   `rep`, the eval lines of the representation map (TCM-C).
   A port of quorum-lang test/tallytool.c at be2c329, trimmed. Each constant
   has its source and a second source (Python integers and `cast keccak`)
   in a comment. */
#include <stdio.h>
#include <string.h>

#include "crypto/dkg.h"
#include "crypto/elgamal.h"
#include "crypto/keccak.h"
#include "crypto/schnorr.h"
#include "crypto/sigma.h"
#include "crypto/threshold.h"

#define ZERO "0000000000000000000000000000000000000000000000000000000000000000"
#define ONE "0000000000000000000000000000000000000000000000000000000000000001"
#define TWO "0000000000000000000000000000000000000000000000000000000000000002"
/* The group order r of G1 (src/crypto/bn254.c BN254_FR) and r - 1. */
#define ORDER "30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001"
#define ORDER_LESS_ONE "30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000000"
/* p - 2 for the field prime p (src/crypto/bn254.c BN254_FP). */
#define PRIME_LESS_TWO "30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd45"
/* 2G and 3G. The sources are at the rows in known_rows. */
#define TWO_GX "030644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd3"
#define TWO_GY "15ed738c0e0a7c92e7845f96b2ae9c0a68a6a449e3538fc7ff3ebf7a5a18a2c4"
#define THREE_GX "0769bf9ac56bea3ff40232bcb1b6bd159315d84715b8e679f2d355961915abf0"
#define THREE_GY "2ab799bee0489429554fdb7c8d086475319e63b40b9c5b57cdf1ff3dd9fe2261"
/* The Schnorr outputs for sk 7 and m "ratify" (6 bytes): pk = sk G (x, y),
   sig = R (x, y) and s, and sig0 = the sig of m of size 0. Source: the S7
   smoke driver (a second build of src/crypto/schnorr.c). The verify rows
   are the second source: the good rows give 1. */
#define SCHNORR_PK                                                                                                     \
  "17072b2ed3bb8d759a5325f477629386cb6fc6ecb801bd76983a6b86abffe078"                                                   \
  "168ada6cd130dd52017bb54bfa19377aadfe3bf05d18f41b77809f7f60d4af9e"
#define SCHNORR_SIG                                                                                                    \
  "13491c983dbd8cdf588d147f4a37ac6b2d178683ea8ba417637f8f8488f7136c"                                                   \
  "014c802bede35b52d0b6206ee21707fa928ac154000e7d0c7c75759c1e5278d8"                                                   \
  "027e47119a84c163c9fe558c651843f4223a5c5ad8c6294a6afdff1436944d05"
#define SCHNORR_SIG0                                                                                                   \
  "0d5e40a992311d18c28832c7486d4dc167db50b91058ecd861b03a7672a370e3"                                                   \
  "13e2d703d6594799f859db21be2b9517c75418c762b5ab32ea262f96915d8a00"                                                   \
  "2dd07fcccaf21dcd09c8623bd2e8f3dcf9517da65f5b8cb1b82a0ecd28bbdc82"

/* The rows of a group: ROWS run, PASS passed. */
typedef struct {
  uint32_t rows;
  uint32_t pass;
} Count;

/* Count the row result OK in C. */
static void add(Count *c, int ok) {
  c->rows++;
  c->pass += (uint32_t)(ok != 0);
}

/* Print the summary line of the group NAME. 1 when each row passed. */
static int summary(const char *name, const Count *c) {
  printf("crypto %s: %u of %u rows passed\n", name, (unsigned)c->pass, (unsigned)c->rows);
  return c->pass == c->rows;
}

/* The word of the 64 hex digits HEX, the most significant digit first. */
static void word_hex(U256 *out, const char *hex) {
  uint32_t i;
  for (i = 0; i < 4u; i++)
    out->limb[i] = 0;
  for (i = 0; i < 64u; i++) {
    char c = hex[i];
    uint64_t d = c >= 'a' ? (uint64_t)(c - 'a' + 10) : (uint64_t)(c - '0');
    out->limb[3u - i / 16u] |= d << (60u - 4u * (i % 16u));
  }
}

static void word_print(const U256 *x) {
  fprintf(stderr, "%016llx%016llx%016llx%016llx", (unsigned long long)x->limb[3],
          (unsigned long long)x->limb[2], (unsigned long long)x->limb[1], (unsigned long long)x->limb[0]);
}

/* 1 when keccak256 of TEXT is the 64 hex digits WANT. */
static int hash_row(const char *name, const char *text, const char *want) {
  unsigned char digest[32];
  char got[65];
  uint32_t i;
  keccak256((const unsigned char *)text, strlen(text), digest);
  for (i = 0; i < 32u; i++)
    snprintf(got + 2u * i, 3, "%02x", digest[i]);
  if (strcmp(got, want) == 0)
    return 1;
  fprintf(stderr, "FAIL crypto %s: keccak256 %s\n", name, got);
  return 0;
}

/* 1 when the N bytes GOT (N at most 96) are the 2N hex digits WANT. */
static int bytes_row(const char *name, const unsigned char *got, size_t n, const char *want) {
  char hex[193];
  size_t i;
  hex[0] = '\0';
  for (i = 0; i < n && i < 96u; i++)
    snprintf(hex + 2u * i, 3, "%02x", got[i]);
  if (strcmp(hex, want) == 0)
    return 1;
  fprintf(stderr, "FAIL crypto %s: bytes %s\n", name, hex);
  return 0;
}

/* 1 when GOT is the word WANT. */
static int word_row(const char *name, const U256 *got, const char *want) {
  U256 w;
  word_hex(&w, want);
  if (u256_cmp(got, &w) == 0)
    return 1;
  fprintf(stderr, "FAIL crypto %s: word ", name);
  word_print(got);
  fputc('\n', stderr);
  return 0;
}

/* 1 when P is on the curve and is the point (X, Y). */
static int point_row(const char *name, const G1 *p, const char *x, const char *y) {
  U256 px;
  U256 py;
  U256 wx;
  U256 wy;
  g1_to_words(&px, &py, p);
  word_hex(&wx, x);
  word_hex(&wy, y);
  if (g1_on_curve(p) && u256_cmp(&px, &wx) == 0 && u256_cmp(&py, &wy) == 0)
    return 1;
  fprintf(stderr, "FAIL crypto %s: point (", name);
  word_print(&px);
  fputs(", ", stderr);
  word_print(&py);
  fputs(")\n", stderr);
  return 0;
}

/* K G for a small K. */
static void times_g(G1 *out, const G1 *g, uint64_t k) {
  U256 w;
  u256_from_u64(&w, k);
  g1_mul(out, g, &w);
}

/* The DKG of the rows: t 2 of n 3. Each open uses 8 bits. */
#define DKG_T 2u
#define DKG_N 3u
#define OPEN_BITS 8u

/* The keys of a DKG: the member shares, KEYS[j - 1] = PK_j, and PK. */
typedef struct {
  ThrShare share[DKG_N];
  G1 keys[DKG_N];
  G1 pk;
} Keys;

/* 1 when the code GOT is WANT. */
static int code_row(const char *name, int got, int want) {
  if (got == want)
    return 1;
  fprintf(stderr, "FAIL crypto %s: code %d, want %d\n", name, got, want);
  return 0;
}

/* 1 when the points A and B are equal. */
static int same_row(const char *name, const G1 *a, const G1 *b) {
  if (g1_eq(a, b))
    return 1;
  fprintf(stderr, "FAIL crypto %s: the points differ\n", name);
  return 0;
}

/* X + 1 mod r, for X below r. */
static void plus_one(U256 *x) {
  Fe a;
  Fe one;
  Fe sum;
  fe_from_u256(&a, x, &BN254_FR);
  fe_from_u64(&one, 1u);
  fe_add(&sum, &a, &one, &BN254_FR);
  fe_to_u256(x, &sum);
}

/* The cast of M under PK with the nonce elg_nonce(NONCE). */
static void cast(ElgCipher *out, const G1 *pk, uint64_t m, uint64_t nonce) {
  U256 wm;
  U256 r;
  u256_from_u64(&wm, m);
  elg_nonce(&r, nonce);
  elg_encrypt(out, pk, &wm, &r);
}

/* The threshold open of CT by the members A and B into M. */
static ThrResult open_pair(uint64_t *m, const Keys *keys, uint32_t a, uint32_t b, const ElgCipher *ct) {
  ThrPart parts[2];
  thr_partial(&parts[0], &keys->share[a - 1u], ct);
  thr_partial(&parts[1], &keys->share[b - 1u], ct);
  return thr_open(m, parts, 2u, keys->keys, DKG_T, DKG_N, ct, OPEN_BITS);
}

/* 1 when the threshold open of CT by the members A and B gives WANT. */
static int open_row(const char *name, const Keys *keys, uint32_t a, uint32_t b, const ElgCipher *ct, uint64_t want) {
  uint64_t m = 0;
  ThrResult res = open_pair(&m, keys, a, b, ct);
  if (res == THR_FOUND && m == want)
    return 1;
  fprintf(stderr, "FAIL crypto %s: code %d, m %llu, want %llu\n", name, (int)res, (unsigned long long)m,
          (unsigned long long)want);
  return 0;
}

/* The known-answer rows of keccak, G1 and ElGamal (the quorum O82 vectors). */
static void known_rows(Count *c) {
  G1 g;
  G1 two;
  G1 p;
  U256 k;
  elg_generator(&g);
  /* Ethereum Keccak (not SHA3). Second source: cast keccak '' and 'abc'. */
  add(c, hash_row("keccak empty", "", "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"));
  add(c, hash_row("keccak abc", "abc", "4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45"));
  /* G = (1, 2) (EIP-196). */
  add(c, point_row("G", &g, ONE, TWO));
  /* 2G: intent-lang test/eval/expect.txt:53 (decimal) and test/precompile.sh:51-52
     (hex). Second source: Python affine double of (1, 2). */
  g1_add(&two, &g, &g);
  add(c, point_row("G + G", &two, TWO_GX, TWO_GY));
  g1_double(&p, &g);
  add(c, point_row("double G", &p, TWO_GX, TWO_GY));
  times_g(&p, &g, 2u);
  add(c, point_row("2 G", &p, TWO_GX, TWO_GY));
  /* 3G: intent-lang test/eval/expect.txt:60 (decimal). Second source: Python. */
  g1_add(&p, &two, &g);
  add(c, point_row("2G + G", &p, THREE_GX, THREE_GY));
  times_g(&p, &g, 3u);
  add(c, point_row("3 G", &p, THREE_GX, THREE_GY));
  /* r G = (0, 0), the point at infinity, and (r - 1) G = -G = (1, p - 2):
     exact by the group order. Second source: Python. */
  word_hex(&k, ORDER);
  g1_mul(&p, &g, &k);
  add(c, point_row("r G", &p, ZERO, ZERO));
  word_hex(&k, ORDER_LESS_ONE);
  g1_mul(&p, &g, &k);
  add(c, point_row("(r - 1) G", &p, ONE, PRIME_LESS_TWO));
  /* scalar_reduce: r gives 0, and 2^256 - 1 gives (2^256 - 1) mod r
     (Python). */
  word_hex(&k, ORDER);
  scalar_reduce(&k, &k);
  add(c, word_row("reduce r", &k, ZERO));
  word_hex(&k, "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
  scalar_reduce(&k, &k);
  add(c, word_row("reduce 2^256 - 1", &k, "0e0a77c19a07df2f666ea36f7879462e36fc76959f60cd29ac96341c4ffffffa"));
  /* elg_nonce(1) = keccak256(ELG_TAG word, word 1) mod r (Python with cast
     keccak), and C1 = elg_nonce(1) G: intent-lang test/precompile.sh:110-111.
     Second source: Python. */
  elg_nonce(&k, 1u);
  add(c, word_row("nonce 1", &k, "2c624b94b15b5d6b3f60a38a41d472fb1e3a20b411c167aa289cf048b9e4253a"));
  g1_mul(&p, &g, &k);
  add(c, point_row("C1", &p, "06107e6b6511db3eb78edf5b81e309c1ce2d43ffc1f99878add70ec94232f5bd",
                   "0c417387621eb1e2f78895cafc1ba63b3141ad838c1718913898a716e2513dcc"));
}

/* The Schnorr rows: the pinned outputs and the good verifies go to KAT,
   the invalid-signature rows go to BAD (one bit of R, of s, the key P of
   sk 8, the message "ratifz", the message of size 0, and s plus r). */
static void schnorr_rows(Count *kat, Count *bad) {
  static const unsigned char text[6] = {'r', 'a', 't', 'i', 'f', 'y'};
  static const unsigned char other[6] = {'r', 'a', 't', 'i', 'f', 'z'};
  unsigned char pk[64];
  unsigned char pk8[64];
  unsigned char sig[96];
  unsigned char sig0[96];
  unsigned char flip[96];
  unsigned char top[32];
  U256 sk;
  U256 sk8;
  U256 big;
  U256 red;
  U256 order;
  U256 s;
  U256 splus;
  u256_from_u64(&sk, 7u);
  u256_from_u64(&sk8, 8u);
  add(kat, code_row("schnorr public sk 7", schnorr_public(pk, &sk), 1));
  add(kat, bytes_row("schnorr pk of sk 7", pk, 64u, SCHNORR_PK));
  add(kat, code_row("schnorr public sk 8", schnorr_public(pk8, &sk8), 1));
  add(kat, code_row("schnorr sign", schnorr_sign(sig, &sk, text, 6u), 1));
  add(kat, bytes_row("schnorr sig of ratify", sig, 96u, SCHNORR_SIG));
  add(kat, code_row("schnorr verify good", schnorr_verify(pk, text, 6u, sig), 1));
  add(kat, code_row("schnorr sign size 0", schnorr_sign(sig0, &sk, text, 0u), 1));
  add(kat, bytes_row("schnorr sig of size 0", sig0, 96u, SCHNORR_SIG0));
  add(kat, code_row("schnorr verify size 0", schnorr_verify(pk, text, 0u, sig0), 1));
  memcpy(flip, sig, sizeof flip);
  flip[63] ^= 1u;
  add(bad, code_row("schnorr R bit", schnorr_verify(pk, text, 6u, flip), 0));
  memcpy(flip, sig, sizeof flip);
  flip[95] ^= 1u;
  add(bad, code_row("schnorr s bit", schnorr_verify(pk, text, 6u, flip), 0));
  add(bad, code_row("schnorr P of sk 8", schnorr_verify(pk8, text, 6u, sig), 0));
  add(bad, code_row("schnorr m ratifz", schnorr_verify(pk, other, 6u, sig), 0));
  add(bad, code_row("schnorr m size 0", schnorr_verify(pk, text, 0u, sig), 0));
  /* s + r gives the same point s.G, but verify takes only an s below r.
     Here r < 2^254 < 2r, so r = 2^254 - (2^254 mod r). */
  memset(top, 0, sizeof top);
  top[0] = 0x40u;
  u256_from_be(&big, top);
  scalar_reduce(&red, &big);
  u256_sub(&order, &big, &red);
  u256_from_be(&s, sig + 64);
  u256_add(&splus, &s, &order);
  memcpy(flip, sig, sizeof flip);
  u256_to_be(flip + 64, &splus);
  add(bad, code_row("schnorr s plus r", schnorr_verify(pk, text, 6u, flip), 0));
}

/* S8 (Q-S8-5 (a)): the host byte checks of a certificate. The body is the
   height (8 bytes), the prev hash (32 bytes) and the action code (8 bytes),
   big endian. The signer signs the keccak-256 hash of the body. The reasons
   are in the Q-S8-3 order: the first failure wins. */
typedef enum { CERT_VALID, CERT_WRONG_HEIGHT, CERT_BAD_PREV_HASH, CERT_BAD_SIGNATURE } CertReason;

static void cert_body(unsigned char body[48], uint64_t height, const unsigned char prev[32], uint64_t action) {
  uint32_t i;
  for (i = 0; i < 8u; i++) {
    body[7u - i] = (unsigned char)(height >> (8u * i));
    body[47u - i] = (unsigned char)(action >> (8u * i));
  }
  memcpy(body + 8, prev, 32u);
}

static int cert_sign(unsigned char sig[96], const U256 *sk, const unsigned char body[48]) {
  unsigned char digest[32];
  keccak256(body, 48u, digest);
  return schnorr_sign(sig, sk, digest, 32u);
}

static int cert_signed(const unsigned char pk[64], const unsigned char body[48], const unsigned char sig[96]) {
  unsigned char digest[32];
  keccak256(body, 48u, digest);
  return schnorr_verify(pk, digest, 32u, sig);
}

/* The reason of BODY and SIG under PK on a chain at the height HEIGHT with
   the tip hash PREV. */
static CertReason cert_check(const unsigned char pk[64], const unsigned char body[48], const unsigned char sig[96],
                             uint64_t height, const unsigned char prev[32]) {
  unsigned char want[48];
  cert_body(want, height, prev, 0u);
  if (memcmp(body, want, 8u) != 0)
    return CERT_WRONG_HEIGHT;
  if (memcmp(body + 8, prev, 32u) != 0)
    return CERT_BAD_PREV_HASH;
  if (!cert_signed(pk, body, sig))
    return CERT_BAD_SIGNATURE;
  return CERT_VALID;
}

/* 1 when the bodies A and B are equivocation evidence (Q-S8-4 (a)): one
   party PK, one height, both signatures check, and the hashes differ. */
static int cert_equivocates(const unsigned char pk[64], const unsigned char a[48], const unsigned char sa[96],
                            const unsigned char b[48], const unsigned char sb[96]) {
  unsigned char ha[32];
  unsigned char hb[32];
  keccak256(a, 48u, ha);
  keccak256(b, 48u, hb);
  return memcmp(a, b, 8u) == 0 && cert_signed(pk, a, sa) && cert_signed(pk, b, sb) && memcmp(ha, hb, 32u) != 0;
}

/* The certificate rows: sk 7, the tip hash keccak256("ratify"), height 5. */
static void cert_rows(Count *c) {
  static const unsigned char text[6] = {'r', 'a', 't', 'i', 'f', 'y'};
  unsigned char pk[64];
  unsigned char prev[32];
  unsigned char other[32];
  unsigned char body[48];
  unsigned char high[48];
  unsigned char fork[48];
  unsigned char later[48];
  unsigned char sig[96];
  unsigned char sig_high[96];
  unsigned char sig_fork[96];
  unsigned char sig_later[96];
  unsigned char flip[96];
  U256 sk;
  u256_from_u64(&sk, 7u);
  keccak256(text, 6u, prev);
  memcpy(other, prev, sizeof other);
  other[31] ^= 1u;
  cert_body(body, 5u, prev, 1u);
  cert_body(high, 6u, prev, 1u);
  cert_body(fork, 5u, prev, 2u);
  cert_body(later, 6u, prev, 2u);
  add(c, code_row("cert public sk 7", schnorr_public(pk, &sk), 1));
  add(c, code_row("cert sign", cert_sign(sig, &sk, body) && cert_sign(sig_high, &sk, high) && cert_sign(sig_fork, &sk, fork) &&
                                   cert_sign(sig_later, &sk, later), 1));
  add(c, code_row("cert valid", (int)cert_check(pk, body, sig, 5u, prev), (int)CERT_VALID));
  memcpy(flip, sig, sizeof flip);
  flip[95] ^= 1u;
  add(c, code_row("cert bad signature (s bit)", (int)cert_check(pk, body, flip, 5u, prev), (int)CERT_BAD_SIGNATURE));
  memcpy(flip, sig, sizeof flip);
  flip[63] ^= 1u;
  add(c, code_row("cert R not a point (R.y bit)", (int)cert_check(pk, body, flip, 5u, prev), (int)CERT_BAD_SIGNATURE));
  add(c, code_row("cert wrong prev hash", (int)cert_check(pk, body, sig, 5u, other), (int)CERT_BAD_PREV_HASH));
  add(c, code_row("cert wrong height in the body", (int)cert_check(pk, high, sig_high, 5u, prev), (int)CERT_WRONG_HEIGHT));
  add(c, code_row("cert height before signature", (int)cert_check(pk, high, sig, 5u, prev), (int)CERT_WRONG_HEIGHT));
  add(c, code_row("cert evidence: one height, two bodies", cert_equivocates(pk, body, sig, fork, sig_fork), 1));
  add(c, code_row("cert no evidence: one body twice", cert_equivocates(pk, body, sig, body, sig), 0));
  add(c, code_row("cert no evidence: two heights", cert_equivocates(pk, body, sig, later, sig_later), 0));
  add(c, code_row("cert no evidence: a bad signature", cert_equivocates(pk, body, sig, fork, sig), 0));
}

/* The keys of the members for QUAL: PK, sk_j from the share table, and
   PK_j from the commitments only. Each row checks PK_j = sk_j G. */
static void keys_of(Count *c, const char *name, Keys *out, const DkgDealing *dealings, const U256 *table,
                    uint32_t qual) {
  uint32_t j;
  dkg_public(&DKG_BN254, &out->pk, qual, dealings, DKG_N);
  for (j = 1u; j <= DKG_N; j++) {
    add(c, code_row(name, dkg_share(&DKG_BN254, &out->share[j - 1u], qual, table, j, DKG_N), DKG_OK));
    add(c, code_row(name, dkg_key(&DKG_BN254, &out->keys[j - 1u], qual, dealings, DKG_T, j, DKG_N), DKG_OK));
    add(c, same_row(name, &out->keys[j - 1u], &out->share[j - 1u].key));
  }
}

/* The good dealing: t 2 of n 3 under the context 7, the dealer i with the
   seed 100 + i. QUAL = {1, 2, 3}, and SUM is the sum of the a_{i,0}. */
static void deal_good(Count *c, Keys *keys, DkgDealing *dealings, U256 *table, U256 *ctx, Fe *sum) {
  U256 seed;
  U256 w;
  Fe a0;
  Fe tmp;
  uint32_t qual = 0;
  uint32_t i;
  u256_from_u64(ctx, 7u);
  fe_from_u64(sum, 0u);
  for (i = 1u; i <= DKG_N; i++) {
    u256_from_u64(&seed, 100u + i);
    add(c, code_row("dkg deal", dkg_deal(&DKG_BN254, &dealings[i - 1u], &table[(i - 1u) * DKG_N], &seed, i, DKG_T, DKG_N, ctx), DKG_OK));
    add(c, code_row("dkg proof", dkg_check_proof(&DKG_BN254, &dealings[i - 1u], i, DKG_T, ctx), 1));
    thr_nonce(&w, &seed, 0u);
    fe_from_u256(&a0, &w, &BN254_FR);
    fe_add(&tmp, sum, &a0, &BN254_FR);
    *sum = tmp;
  }
  add(c, code_row("dkg good", dkg_qual(&DKG_BN254, &qual, dealings, table, DKG_T, DKG_N, ctx), DKG_OK));
  add(c, code_row("dkg good QUAL", (int)qual, 14));
  keys_of(c, "dkg good keys", keys, dealings, table, qual);
}

/* The threshold rows (O7): the DKG, each pair of members opens a cast, the
   algebra row, and the parts that the DLEQ check refuses. KEYS gets the
   keys of the good dealing. */
static void threshold_rows(Count *c, Keys *keys) {
  DkgDealing dealings[DKG_N];
  DkgDealing bad[DKG_N];
  U256 table[DKG_N * DKG_N];
  U256 forged[DKG_N * DKG_N];
  Keys quorum;
  ThrShare share;
  ThrPart parts[2];
  ElgCipher ct;
  ElgCipher ca;
  ElgCipher cb;
  U256 ctx;
  U256 w;
  Fe sum;
  G1 g;
  G1 two;
  G1 pk;
  uint64_t m = 0;
  uint32_t qual = 0;
  /* (1) The good dealing: QUAL = {1, 2, 3}, PK = (the sum of a_{i,0}) G,
     and each pair of members opens a cast. */
  deal_good(c, keys, dealings, table, &ctx, &sum);
  fe_to_u256(&w, &sum);
  elg_generator(&g);
  g1_mul(&pk, &g, &w);
  add(c, same_row("dkg good PK", &keys->pk, &pk));
  cast(&ct, &keys->pk, 5u, 1u);
  add(c, open_row("dkg open {1, 2}", keys, 1u, 2u, &ct, 5u));
  add(c, open_row("dkg open {1, 3}", keys, 1u, 3u, &ct, 5u));
  add(c, open_row("dkg open {2, 3}", keys, 2u, 3u, &ct, 5u));
  /* The algebra row: Enc 2 + Enc 3 opens to 5 (eval g of the meanings). */
  cast(&ca, &keys->pk, 2u, 3u);
  cast(&cb, &keys->pk, 3u, 4u);
  elg_add(&ct, &ca, &cb);
  add(c, open_row("algebra Enc 2 + Enc 3", keys, 1u, 3u, &ct, 5u));
  /* (2) The dealer 2 sends f_2(3) + 1 to the member 3: QUAL = {1, 3}, and
     the key of QUAL decrypts. */
  memcpy(forged, table, sizeof forged);
  plus_one(&forged[DKG_N + 2u]);
  add(c, code_row("dkg bad share", dkg_qual(&DKG_BN254, &qual, dealings, forged, DKG_T, DKG_N, &ctx), DKG_BAD_SHARE));
  add(c, code_row("dkg bad share QUAL", (int)qual, 10));
  keys_of(c, "dkg bad share keys", &quorum, dealings, forged, qual);
  cast(&ct, &quorum.pk, 6u, 2u);
  add(c, open_row("dkg bad share open {1, 3}", &quorum, 1u, 3u, &ct, 6u));
  /* (3) The dealer 1 with z + 1: QUAL = {2, 3}. */
  memcpy(bad, dealings, sizeof bad);
  plus_one(&bad[0].z);
  add(c, code_row("dkg bad proof", dkg_qual(&DKG_BN254, &qual, bad, table, DKG_T, DKG_N, &ctx), DKG_BAD_PROOF));
  add(c, code_row("dkg bad proof QUAL", (int)qual, 12));
  /* The member 0 and the member n + 1. */
  add(c, code_row("dkg member 0", dkg_share(&DKG_BN254, &share, 14u, table, 0u, DKG_N), DKG_BAD_INDEX));
  add(c, code_row("dkg member n + 1", dkg_share(&DKG_BN254, &share, 14u, table, DKG_N + 1u, DKG_N), DKG_BAD_INDEX));
  /* Each dealing is bad: QUAL is empty. */
  plus_one(&bad[1].z);
  plus_one(&bad[2].z);
  add(c, code_row("dkg too few", dkg_qual(&DKG_BN254, &qual, bad, table, DKG_T, DKG_N, &ctx), DKG_TOO_FEW));
  add(c, code_row("dkg too few QUAL", (int)qual, 0));
  /* (4) A part with D + G and (5) a part with z + 1: the DLEQ check
     refuses each. */
  cast(&ct, &keys->pk, 3u, 31u);
  thr_partial(&parts[0], &keys->share[0], &ct);
  thr_partial(&parts[1], &keys->share[1], &ct);
  g1_add(&parts[0].d, &parts[0].d, &g);
  add(c, code_row("part D + G", thr_open(&m, parts, 2u, keys->keys, DKG_T, DKG_N, &ct, OPEN_BITS), THR_BAD_PROOF));
  thr_partial(&parts[0], &keys->share[0], &ct);
  plus_one(&parts[1].z);
  add(c, code_row("part z + 1", thr_open(&m, parts, 2u, keys->keys, DKG_T, DKG_N, &ct, OPEN_BITS), THR_BAD_PROOF));
  /* (6) An off-curve C1: (1, the y of 2G). */
  g1_add(&two, &g, &g);
  ct.c1 = g;
  ct.c1.y = two.y;
  add(c, code_row("off-curve C1", thr_open(&m, parts, 2u, keys->keys, DKG_T, DKG_N, &ct, OPEN_BITS), THR_REFUSED));
}

/* The reshare rows (O9, the quorum O97 row): Q = {1, 3} of the good dealing
   deals the key again to the new t 3 of n 4 under the context 8 (the dealer
   i with the seed 200 + i), and the new members {2, 3, 4} open the cast CT
   under the old PK. KEYS has the keys of the good dealing. */
static void reshare_rows(Count *c, const Keys *keys, const ElgCipher *ct) {
  static const uint32_t quorum[2] = {1u, 3u};
  DkgDealing dealings[2];
  U256 table[2u * 4u];
  G1 old[2];
  ThrShare fresh[4];
  G1 fresh_keys[4];
  ThrPart parts[3];
  U256 ctx;
  U256 seed;
  uint64_t m = 0;
  uint32_t qual = 0;
  uint32_t k;
  u256_from_u64(&ctx, 8u);
  for (k = 0; k < 2u; k++) {
    old[k] = keys->keys[quorum[k] - 1u];
    u256_from_u64(&seed, 200u + quorum[k]);
    add(c, code_row("reshare deal",
                    dkg_reshare_deal(&DKG_BN254, &dealings[k], &table[k * 4u], &seed, &keys->share[quorum[k] - 1u],
                                     quorum, 2u, 3u, 4u, &ctx),
                    DKG_OK));
  }
  add(c, code_row("reshare good",
                  dkg_reshare_qual(&DKG_BN254, &qual, dealings, table, old, quorum, 2u, DKG_T, 3u, 4u, &ctx), DKG_OK));
  for (k = 1u; k <= 4u; k++) {
    add(c, code_row("reshare share", dkg_reshare_share(&DKG_BN254, &fresh[k - 1u], qual, table, quorum, 2u, k, 4u),
                    DKG_OK));
    add(c, code_row("reshare key",
                    dkg_reshare_key(&DKG_BN254, &fresh_keys[k - 1u], qual, dealings, quorum, 2u, 3u, k, 4u), DKG_OK));
    add(c, same_row("reshare PK'_j = sk'_j G", &fresh_keys[k - 1u], &fresh[k - 1u].key));
  }
  for (k = 0; k < 3u; k++)
    thr_partial(&parts[k], &fresh[k + 1u], ct);
  add(c, code_row("reshare open {2, 3, 4}", thr_open(&m, parts, 3u, fresh_keys, 3u, 4u, ct, OPEN_BITS), THR_FOUND));
  add(c, code_row("reshare open m", (int)m, 5));
}

/* The result of sig_onehot_verify on a ballot of K slots with the bits
   BITS and the nonces elg_nonce(20 + j): each slot has its valid bit
   proof, and the sum has a proof of randomness for m = 1 (a forged ballot
   when the bits do not sum to 1). */
static SigResult forged_ballot(const G1 *pk, const uint32_t *bits, uint32_t k, const U256 *ctx) {
  ElgCipher cts[SIG_K_MAX];
  SigOr slots[SIG_K_MAX];
  SigDleq sum;
  ElgCipher total;
  U256 r;
  U256 m;
  Fe rsum;
  Fe fr;
  Fe tmp;
  uint32_t j;
  fe_from_u64(&rsum, 0u);
  for (j = 0; j < k; j++) {
    elg_nonce(&r, 20u + j);
    u256_from_u64(&m, bits[j]);
    elg_encrypt(&cts[j], pk, &m, &r);
    sig_or_prove(&slots[j], pk, &cts[j], bits[j], &r, ctx, j);
    fe_from_u256(&fr, &r, &BN254_FR);
    fe_add(&tmp, &rsum, &fr, &BN254_FR);
    rsum = tmp;
  }
  total = cts[0];
  for (j = 1u; j < k; j++)
    elg_add(&total, &total, &cts[j]);
  fe_to_u256(&r, &rsum);
  u256_from_u64(&m, 1u);
  sig_rand_prove(&sum, pk, &total, &m, &r, ctx);
  return sig_onehot_verify(cts, slots, &sum, pk, k, ctx);
}

/* The invalid-proof rows (O8, the quorum O83 rows) and the good rows, under
   the keys of the good dealing. */
static void proof_rows(Count *c, const Keys *keys) {
  static const uint32_t two_hot[2] = {1u, 1u};
  static const uint32_t all_zero[2] = {0u, 0u};
  ElgCipher cts[SIG_K_MAX];
  ElgCipher moved[SIG_K_MAX];
  SigOr slots[SIG_K_MAX];
  SigOr forged[SIG_K_MAX];
  SigOr sim;
  SigDleq sum;
  ElgCipher ct;
  U256 ctx;
  U256 other;
  U256 seed;
  U256 m0;
  U256 m1;
  G1 y0;
  G1 y1;
  u256_from_u64(&ctx, 7u);
  u256_from_u64(&other, 8u);
  /* (1) A slot with plaintext 2 and a simulated OR proof: no branch is
     real, so the split fails. */
  cast(&ct, &keys->pk, 2u, 30u);
  u256_from_u64(&m0, 0u);
  u256_from_u64(&m1, 1u);
  sig_plain(&y0, &ct, &m0);
  sig_plain(&y1, &ct, &m1);
  u256_from_u64(&sim.e0, 3u);
  u256_from_u64(&sim.z0, 4u);
  u256_from_u64(&sim.e1, 5u);
  u256_from_u64(&sim.z1, 6u);
  sig_simulate(&sim.a0, &sim.b0, &keys->pk, &ct.c1, &y0, &sim.e0, &sim.z0);
  sig_simulate(&sim.a1, &sim.b1, &keys->pk, &ct.c1, &y1, &sim.e1, &sim.z1);
  add(c, code_row("plaintext 2", sig_or_verify(&sim, &keys->pk, &ct, &ctx, 0u), SIG_BAD_SLOT));
  /* (2) A two-hot ballot and (3) an all-zero ballot: each bit proof is
     valid, and the sum proof fails. */
  add(c, code_row("two-hot", forged_ballot(&keys->pk, two_hot, 2u, &ctx), SIG_BAD_SUM));
  add(c, code_row("all-zero", forged_ballot(&keys->pk, all_zero, 2u, &ctx), SIG_BAD_SUM));
  /* The good row k 2: the ballot for the choice 1 verifies. */
  u256_from_u64(&seed, 40u);
  add(c, code_row("one-hot k 2 prove", sig_onehot_prove(cts, slots, &sum, &keys->pk, 2u, 1u, &seed, &ctx), SIG_OK));
  add(c, code_row("one-hot k 2", sig_onehot_verify(cts, slots, &sum, &keys->pk, 2u, &ctx), SIG_OK));
  /* (4) A forged split: e0 + e1 != e. */
  memcpy(forged, slots, sizeof forged);
  forged[0].e0.limb[0] ^= 1u;
  add(c, code_row("forged split", sig_onehot_verify(cts, forged, &sum, &keys->pk, 2u, &ctx), SIG_BAD_SLOT));
  /* (5) Each slot (with its bit proof) at the other slot index j. */
  moved[0] = cts[1];
  moved[1] = cts[0];
  forged[0] = slots[1];
  forged[1] = slots[0];
  add(c, code_row("slot moved", sig_onehot_verify(moved, forged, &sum, &keys->pk, 2u, &ctx), SIG_BAD_SLOT));
  /* (6) The valid ballot under another ctx: the points are on the curve,
     so the first bit proof fails (CTX is in its challenge). */
  add(c, code_row("other ctx", sig_onehot_verify(cts, slots, &sum, &keys->pk, 2u, &other), SIG_BAD_SLOT));
}

/* The bit I of the case V of length LEN: the bit LEN - 1 - I of V. */
static uint32_t bit_of(uint32_t v, uint32_t len, uint32_t i) {
  return (v >> (len - 1u - i)) & 1u;
}

/* The rep mode (TCM-C): the good dealing, then for each approval list over
   Bool of length 1 to 3 (14 cases), the cast of each approval bit, the sum
   (elg_add), and the open of the sum by the members 1 and 2. Each line is
   an eval line of the entry repCountLEN of examples/crypto.lang:
   `crypto repCountLEN B1 .. BLEN => COUNT` (14 lines). */
static int rep_lines(void) {
  DkgDealing dealings[DKG_N];
  U256 table[DKG_N * DKG_N];
  Keys keys;
  ElgCipher sum;
  ElgCipher bit;
  U256 ctx;
  Fe a0;
  Count c = {0u, 0u};
  uint64_t nonce = 1000u;
  uint64_t count;
  uint32_t len;
  uint32_t v;
  uint32_t i;
  ThrResult res;
  deal_good(&c, &keys, dealings, table, &ctx, &a0);
  for (len = 1u; len <= 3u; len++) {
    for (v = 0; v < (1u << len); v++) {
      cast(&sum, &keys.pk, bit_of(v, len, 0u), nonce++);
      for (i = 1u; i < len; i++) {
        cast(&bit, &keys.pk, bit_of(v, len, i), nonce++);
        elg_add(&sum, &sum, &bit);
      }
      count = 0;
      res = open_pair(&count, &keys, 1u, 2u, &sum);
      printf("crypto repCount%u", (unsigned)len);
      for (i = 0; i < len; i++)
        printf(" %u", (unsigned)bit_of(v, len, i));
      if (res == THR_FOUND)
        printf(" => %llu\n", (unsigned long long)count);
      else
        printf(" => open-%d\n", (int)res);
      add(&c, code_row("rep open", (int)res, THR_FOUND));
    }
  }
  return c.pass == c.rows ? 0 : 1;
}

int main(int argc, char **argv) {
  Count kat = {0u, 0u};
  Count badsig = {0u, 0u};
  Count thr = {0u, 0u};
  Count re = {0u, 0u};
  Count badproof = {0u, 0u};
  Count cert = {0u, 0u};
  ElgCipher ct;
  Keys keys;
  int ok;
  if (argc == 2 && strcmp(argv[1], "rep") == 0)
    return rep_lines();
  if (argc != 1) {
    fputs("usage: cryptotool [rep]\n", stderr);
    return 2;
  }
  known_rows(&kat);
  schnorr_rows(&kat, &badsig);
  threshold_rows(&thr, &keys);
  cast(&ct, &keys.pk, 5u, 1u);
  reshare_rows(&re, &keys, &ct);
  proof_rows(&badproof, &keys);
  cert_rows(&cert);
  ok = summary("known-answer", &kat);
  ok &= summary("invalid-signature", &badsig);
  ok &= summary("threshold", &thr);
  ok &= summary("reshare", &re);
  ok &= summary("invalid-proof", &badproof);
  ok &= summary("cert", &cert);
  if (ok)
    puts("crypto regressions: passed");
  return ok ? 0 : 1;
}
