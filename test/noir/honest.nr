// The T3 relations of ratify-lang as Noir (S13, Q-S0-13 (b)): R_ballot and
// R_dec over BN254 with noir_bigcurve (Q-S13-1 (a)). `langc noir PROG`
// writes this text. Do not edit it: the gate compares it byte for byte.
use bigcurve::{BigCurve, BN254, BN254Scalar};
use bignum::{BigNum, BN254_Fq};

// K_<DEF> is the member count of the decided rule def DEF: the sum of its
// class sizes (Q-S13-8 (a)). The ballot slot j of a member is below K.
pub global K_H3: u32 = 5;
pub global K_H4: u32 = 5;

// A public point from its canonical coordinates. T1 encodes the identity
// as (0, 0); all other points must satisfy the curve equation.
pub fn point(x: BN254_Fq, y: BN254_Fq) -> BN254 {
    x.validate_in_field();
    y.validate_in_field();
    BN254::from_coordinates(x, y, x.is_zero() & y.is_zero())
}

// R_ballot: (c1, c2) is the ElGamal encryption of the bit b under the
// public key pk with the randomness r, and the slot j is below k. The
// generator G is BN254::one() = (1, 2), the T1 generator. ctx binds the
// proof to its context; no constraint reads it.
pub fn r_ballot(k: u32, pk: BN254, c1: BN254, c2: BN254, _ctx: [u8; 32], j: u32, b: Field, r: Field) {
    let g = BN254::one();
    assert(b * (1 - b) == 0);
    assert(g.mul(BN254Scalar::from(r)) == c1);
    assert(g.mul(BN254Scalar::from(b)) + pk.mul(BN254Scalar::from(r)) == c2);
    assert(j < k);
}

// R_dec: key = s G is the key share of the secret s, and d = s c1 is its
// decryption share of the ciphertext with the first part c1.
pub fn r_dec(key: BN254, c1: BN254, d: BN254, s: Field) {
    let g = BN254::one();
    assert(g.mul(BN254Scalar::from(s)) == key);
    assert(c1.mul(BN254Scalar::from(s)) == d);
}
