// R_dec (S13 U1). The public inputs are key, c1 and d (each point as its
// two BN254_Fq coordinates). s is private.
mod kat;

use bignum::{BigNum, BN254_Fq};
use ratify::{point, r_dec};

fn main(
    key_x: pub BN254_Fq,
    key_y: pub BN254_Fq,
    c1_x: pub BN254_Fq,
    c1_y: pub BN254_Fq,
    d_x: pub BN254_Fq,
    d_y: pub BN254_Fq,
    s: Field,
) {
    r_dec(point(key_x, key_y), point(c1_x, c1_y), point(d_x, d_y), s);
}

// The KAT rows (S13 U2). src/kat.nr is the text of `cryptotool noir DIR`:
// T1 values with fixed scalars. Each should_fail row breaks one constraint
// and keeps the other constraints true. Thus the drop of that constraint
// (an emitter mutant, S13 U3) makes the row pass, and nargo test fails.
fn fq(limbs: [u128; 3]) -> BN254_Fq {
    BN254_Fq::from_limbs(limbs)
}

fn dec(key: [[u128; 3]; 2], d: [[u128; 3]; 2]) {
    main(fq(key[0]), fq(key[1]), fq(kat::C1[0]), fq(kat::C1[1]), fq(d[0]), fq(d[1]), kat::S);
}

#[test]
fn honest_share() {
    dec(kat::KEY, kat::D);
}

// M5: key = s G + G. Only the key constraint fails.
#[test(should_fail)]
fn wrong_key() {
    dec(kat::KEY_BAD, kat::D);
}

// M6: d = s c1 + G. Only the share constraint fails.
#[test(should_fail)]
fn wrong_d() {
    dec(kat::KEY, kat::D_BAD);
}

// With a zero secret both key and d are the identity, encoded by (0, 0)
// in T1. With c1 = identity, d is identity for every secret.
#[test]
fn zero_secret() {
    let zero = fq([0, 0, 0]);
    main(zero, zero, fq(kat::C1[0]), fq(kat::C1[1]), zero, zero, 0);
}

#[test]
fn identity_ciphertext() {
    let zero = fq([0, 0, 0]);
    main(fq(kat::KEY[0]), fq(kat::KEY[1]), zero, zero, zero, zero, kat::S);
}
