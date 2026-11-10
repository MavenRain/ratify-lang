// R_ballot for the decided rule def bar of test/noir/rule.lang: k = K_BAR
// (S13 U1). The public inputs are pk, c1 and c2 (each point as its two
// BN254_Fq coordinates), ctx and the slot j. b and r are private.
mod kat;

use bignum::{BigNum, BN254_Fq};
use ratify::{K_BAR, point, r_ballot};

fn main(
    pk_x: pub BN254_Fq,
    pk_y: pub BN254_Fq,
    c1_x: pub BN254_Fq,
    c1_y: pub BN254_Fq,
    c2_x: pub BN254_Fq,
    c2_y: pub BN254_Fq,
    ctx: pub [u8; 32],
    j: pub u32,
    b: Field,
    r: Field,
) {
    r_ballot(K_BAR, point(pk_x, pk_y), point(c1_x, c1_y), point(c2_x, c2_y), ctx, j, b, r);
}

// The KAT rows (S13 U2). src/kat.nr is the text of `cryptotool noir DIR`:
// T1 values with fixed scalars. Each should_fail row breaks one constraint
// and keeps the other constraints true. Thus the drop of that constraint
// (an emitter mutant, S13 U3) makes the row pass, and nargo test fails.
fn fq(limbs: [u128; 3]) -> BN254_Fq {
    BN254_Fq::from_limbs(limbs)
}

// M4 (G and pk swapped in the c2 constraint) fails the two honest rows.
fn ballot(c1: [[u128; 3]; 2], c2: [[u128; 3]; 2], j: u32, b: Field, r: Field) {
    main(fq(kat::PK[0]), fq(kat::PK[1]), fq(c1[0]), fq(c1[1]), fq(c2[0]), fq(c2[1]), kat::CTX, j, b, r);
}

#[test]
fn honest_bit_0() {
    ballot(kat::C1_B0, kat::C2_B0, 0, 0, kat::R0);
}

// The last slot j = K_BAR - 1 is in range.
#[test]
fn honest_bit_1() {
    ballot(kat::C1_B1, kat::C2_B1, K_BAR - 1, 1, kat::R1);
}

// M1: b = 2 with c2 = 2 G + r pk (the T1 ElGamal of 2). Only the bit
// constraint fails.
#[test(should_fail)]
fn bit_2() {
    ballot(kat::C1_B1, kat::C2_B2, 0, 2, kat::R1);
}

// M2: c1 = r G + G. Only the c1 constraint fails.
#[test(should_fail)]
fn wrong_c1() {
    ballot(kat::C1_BAD, kat::C2_B1, 0, 1, kat::R1);
}

// M3: c2 = G + r pk + pk. Only the c2 constraint fails.
#[test(should_fail)]
fn wrong_c2() {
    ballot(kat::C1_B1, kat::C2_BAD, 0, 1, kat::R1);
}

// M7: the slot j = K_BAR. Only the slot constraint fails.
#[test(should_fail)]
fn slot_k() {
    ballot(kat::C1_B1, kat::C2_B1, K_BAR, 1, kat::R1);
}

// T1 requires canonical field coordinates, not just the curve equation
// modulo p. These encodings of G have x = p + 1 or y = p + 2.
#[test(should_fail)]
fn noncanonical_x() {
    let mut limbs = BN254_Fq::modulus().get_limbs();
    limbs[0] += 1;
    let _p = point(fq(limbs), fq([2, 0, 0]));
}

#[test(should_fail)]
fn noncanonical_y() {
    let mut limbs = BN254_Fq::modulus().get_limbs();
    limbs[0] += 2;
    let _p = point(fq([1, 0, 0]), fq(limbs));
}

// T1's (0, 0) encodes the identity. Zero randomness gives c1 = identity
// and c2 = b G for either bit.
#[test]
fn zero_randomness_bit_0() {
    let zero = fq([0, 0, 0]);
    main(fq(kat::PK[0]), fq(kat::PK[1]), zero, zero, zero, zero, kat::CTX, 0, 0, 0);
}

#[test]
fn zero_randomness_bit_1() {
    let zero = fq([0, 0, 0]);
    main(fq(kat::PK[0]), fq(kat::PK[1]), zero, zero, fq([1, 0, 0]), fq([2, 0, 0]), kat::CTX, 0, 1, 0);
}
