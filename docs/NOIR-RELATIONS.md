# T3: R_ballot and R_dec as Noir

S13 (`docs/PLAN.md` section 8.2, T3; Q-S0-13 (b), Q-S13-1 to Q-S13-8).
`langc [--kit] noir PROG` checks PROG and writes one Noir library text:
a K global for each decided rule definition and the two relations
R_ballot and R_dec. The relations use the T1 bytes (ElGamal and DLEQ on
BN254 G1, `src/crypto/`), so T3 exports O8 for T1 (`docs/PLAN.md`
section 6).

## 1. The command

- `langc [--kit] noir PROG`: exit 0, the text on stdout. A check error
  gives the same output as `langc check`.
- `REFUSE_NOIR` (D-U1-1): two decided rule definitions with one K name,
  or a member count above the u32 range (not reachable with unary Size).
  Exit 1, no stdout. Each refusal comes before the first byte of text.
  `langc check` and T1 do not change.
- A PROG with no decided rule definition gives the text with no K global
  (D-U1-2). No golden row has this case.

## 2. The relations

The group is BN254 G1. The generator G is `BN254::one()` = (1, 2), the
T1 generator. A scalar is a Noir `Field` (the BN254 scalar field, the
native field of Noir); `BN254Scalar::from` converts it. No Fiat-Shamir
step is in the circuit (Q-S13-2 (a)).

R_ballot (one bit ciphertext, as the T1 import with J = 3):

- Public: PK, C1, C2, CTX, the slot j. k is the constant `K_<DEF>`.
- Witness: the bit b and the randomness r.
- Constraints (`r_ballot` in `noir/ratify/src/lib.nr`):
  1. b (1 - b) = 0, so b is 0 or 1.
  2. r G = C1.
  3. b G + r PK = C2.
  4. j < k.
- CTX binds the proof to its context. No constraint reads it (`_ctx`).

R_dec (one DLEQ share, as the T1 import with J = 2):

- Public: KEY, C1, D. Witness: the secret s.
- Constraints (`r_dec` in `lib.nr`):
  1. s G = KEY.
  2. s C1 = D.

## 3. The K globals

`pub global K_<DEF>: u32` is the member count of the decided rule
definition DEF: the sum of its class sizes n_j (Q-S13-8 (a)). The text
has one K global for each decided rule definition, in program order.
The K name is `K_`, then the definition name in upper case, with `_` for
each character not in [A-Za-z0-9_] (D-U1-1). The ballot slot j of a
member is below K.

## 4. The public inputs and the limb form

- A point input of a bin is its two coordinates (x, y), each a
  `BN254_Fq`. `point(x, y)` first calls `validate_in_field` on each
  coordinate, including its limb ranges and the strict bound below the
  base modulus. `from_coordinates` alone checks the curve equation
  modulo the base modulus and does not enforce that bound.
- The pair (0, 0) encodes the identity, as in T1 `g1_from_words`.
  `point` passes `x.is_zero() & y.is_zero()` as the infinity flag to
  `BN254::from_coordinates`; every other pair must be on the curve.
  This includes ciphertexts with zero randomness and shares with a
  zero secret (S13 review correction to D-U1-5).
- The coordinates are in the base field of BN254, not in the native
  field of Noir. A `BN254_Fq` has 3 limbs of 120 bits, little endian. In
  Prover.toml, a point input is two tables, for example `[pk_x]` and
  `[pk_y]`, each with `limbs = ["0x..", "0x..", "0x.."]` (D-U2-4).
- `noir/r_ballot` main: `pk_x`, `pk_y`, `c1_x`, `c1_y`, `c2_x`, `c2_y`
  (pub `BN254_Fq`), `ctx` (pub `[u8; 32]`), `j` (pub `u32`), `b` and `r`
  (`Field`). k = `K_BAR`, the first decided rule definition of
  `test/noir/rule.lang` (D-U1-3).
- `noir/r_dec` main: `key_x`, `key_y`, `c1_x`, `c1_y`, `d_x`, `d_y` (pub
  `BN254_Fq`), `s` (`Field`).

## 5. The packages

- `noir/ratify` (type lib): `src/lib.nr` is the text of `langc noir
  test/noir/rule.lang` (D-U1-3). The gate compares it byte for byte.
- `noir/r_ballot` and `noir/r_dec` (type bin): `src/main.nr`,
  `src/kat.nr` and `Prover.toml`. Each bin depends on noir-bignum
  directly (main names `BN254_Fq`) and on ratify by the path
  `../ratify` (D-U1-5).
- `.gitignore` has `noir/*/target/` (the nargo output).

## 6. The pins

noir_bigcurve v0.15.0, noir-bignum v0.11.0 and nargo 1.0.0-rc.0; each
Nargo.toml has `compiler_version = ">=1.0.0"` (Q-S13-7 (a)). The two dep
edges to noir-bignum (ratify to bignum, ratify to bigcurve to bignum)
give one crate: `nargo check` gives rc 0 in the 3 packages.

## 7. The KAT rows

- `build/cryptotool noir DIR` writes 4 files in the layout of `noir/`:
  `r_ballot/src/kat.nr`, `r_ballot/Prover.toml`, `r_dec/src/kat.nr` and
  `r_dec/Prover.toml` (D-U2-1). In kat.nr a point is `[[u128; 3]; 2]`
  (the x limbs, the y limbs) and a scalar is a `Field`.
- The scalars sk, r0, r1 and s are sha256("ratify-lang S13 U2 " + name)
  mod r (D-U2-2). The points come from the T1 functions: PK =
  `elg_public(sk)`; (C1, C2) = `elg_encrypt` of m = 0 (r0), 1 (r1) and
  2 (r1); KEY = `elg_public(s)`; D = `g1_mul(C1_B1, s)` (D-U2-3).
- The bad points are a good point plus G or plus PK: C1_BAD = C1_B1 + G,
  C2_BAD = C2_B1 + PK, KEY_BAD = KEY + G, D_BAD = D + G. Each is on the
  curve, so `from_coordinates` accepts it and the row fails on its own
  constraint only (D-U2-3).
- The test rows call `main` (D-U2-5). r_ballot: `honest_bit_0`,
  `honest_bit_1` (j = K_BAR - 1, the last slot in range), and the
  `should_fail` rows `bit_2`, `wrong_c1`, `wrong_c2` and `slot_k`. r_dec:
  `honest_share`, and the `should_fail` rows `wrong_key` and `wrong_d`.
- The review regressions add `noncanonical_x` and `noncanonical_y`
  (`should_fail`, coordinates of G with p added),
  `zero_randomness_bit_0`, `zero_randomness_bit_1`, `zero_secret` and
  `identity_ciphertext`. There are 10 r_ballot and 5 r_dec tests.
- Prover.toml: r_ballot has the b = 1 row in the slot j = 0; r_dec has
  the honest share (D-U2-4).

## 8. The tests

The gate compares bytes only (Q-S13-4 (a)):

- `noir rows: 7 checked`: `test/noir/NAME.lang` and `NAME.nr` for the 7
  rows of `test/report/`.
- `noir refusals: 1 checked`: `test/noir/refuse/clash.lang` (`def
  barhonest` and `barHonest` have one K name, `K_BARHONEST`) (D-U1-4).
- `noir package: 1 checked`: `noir/ratify/src/lib.nr`.
- `noir kat: 4 checked`: the 4 files of `cryptotool noir` (D-U2-1).

The original USER runs used nargo 1.0.0-rc.0 outside the gate
(2026-11-10), before the review regressions:

- `nargo check`, `test`, `execute` and `info` in each package: rc 0, 0
  warnings. 6 r_ballot and 3 r_dec test rows pass. `execute` solves both
  witnesses from the Prover.toml files.
- Emitter mutants (Q-S13-5 (a)): each mutant is the `langc noir` text
  with one change in `src/front/noir.c`, so its text is not the golden
  text. Both packages run `nargo test` on each mutant (D-U3-1, D-U3-2):

| Mutant | Change in lib.nr | The failed tests |
|---|---|---|
| M1 | no b (1 - b) = 0 | `bit_2` |
| M2 | no r G = C1 | `wrong_c1` |
| M3 | no b G + r PK = C2 | `wrong_c2` |
| M4 | G and PK swap in the C2 row | `honest_bit_0`, `honest_bit_1` |
| M5 | no s G = KEY | `wrong_key` |
| M6 | no s C1 = D | `wrong_d` |
| M7 | no j < k | `slot_k` |

  7 of 7 are KILLED. No other test fails.

The review reruns `nargo check` for all packages and `execute` and `info`
for both bins after the point-decoding fixes: rc 0, and Prover.toml is
unchanged. The expanded suites pass 10 r_ballot and 5 r_dec tests with
2 threads. Focused mutation runs repeat M1 to M7 on their named tests
above. Three more mutants drop x validation, drop y validation or force
the infinity flag to false; they fail `noncanonical_x`,
`noncanonical_y` or `zero_randomness_bit_0`, respectively. All 10
mutants are killed, and each changed library differs from the emitted
package text checked by the gate.

## 9. The cost (OI-11)

`nargo info`, function main, nargo 1.0.0-rc.0, after the review fixes:

| Package | ACIR opcodes | Brillig opcodes |
|---|---|---|
| r_ballot | 68506 | 145210 |
| r_dec | 45398 | 115084 |

The cost comes from the non-native base field arithmetic of noir-bignum.
The original `nargo test` peak memory footprint of r_ballot was 4.04e9
bytes with its 6 tests in parallel, and 1.93e9 bytes or less with 2 threads
(D-U3-3).

## 10. Open

- N-U2-1: `nargo execute` of r_ballot prints one note, `bug: Brillig
  function call isn't properly covered by a manual constraint`, at
  noir_bigcurve v0.15.0 `src/bigcurve.nr:371` (`unsafe { lhsJ.add(rhsJ)
  }` in the `impl Add`). The call comes from `r_ballot` in `lib.nr`, the `+`
  of the C2 row. The upstream comment says that `add_with_hint` checks
  the result. Evidence, not proof: `wrong_c2` fails, and M3 and M4 are
  KILLED. This repo does not prove the soundness of that `+`.
- No proof backend runs: T3 gives the relations and their tests, not a
  proof or a verifier.

## 11. Decisions (S13)

These are not USER rulings. The USER can overturn each.

- D-U1-1 The K name, and `REFUSE_NOIR` (section 1, section 3).
- D-U1-2 A PROG with no decided rule definition gives no K global.
- D-U1-3 The package source is `test/noir/rule.lang`, not
  `test/demo/demo.lang` (it has no decided rule definition).
  `noir/r_ballot` uses `K_BAR`.
- D-U1-4 One refusal row, `test/noir/refuse/clash.lang`; the gate lines
  `noir refusals` and `noir package`.
- D-U1-5 `point` uses `from_coordinates`; the bins depend on
  noir-bignum and on ratify by path. The review adds explicit canonical
  coordinate validation and T1 identity decoding (section 4); the lib
  imports both `BigNum` and `BN254_Fq` from noir-bignum.
- D-U2-1 `cryptotool noir DIR` writes the 4 KAT files; the gate line
  `noir kat`.
- D-U2-2 The fixed scalars from sha256.
- D-U2-3 The KAT points from the T1 functions; the bad points on the
  curve.
- D-U2-4 The Prover.toml rows and the limb tables.
- D-U2-5 The test rows call `main` through the helpers `fq`, `ballot` and
  `dec`.
- D-U2-6 The USER script gets the noirup script into its own NARGO_HOME
  and does not run the noirup installer (the installer edits the shell
  profile).
- D-U3-1 A mutant is KILLED when each wanted test fails in its package.
- D-U3-2 The clean text runs first and must pass 6 + 3 tests.
- D-U3-3 `nargo test --test-threads 2`, and a limit of 4.0e9 bytes on
  the peak memory.
