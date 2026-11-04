# ratify-lang plan

ratify-lang is a Cubical TT language for one job: threshold authorities that
amend themselves. A policy is a committee in classes, an authorize rule, an
amend rule and a fault bound. A rule reads only the count of approvals in
each class. Each decision is one bit. An amendment changes the policy one
step at a time, and the old amend rule judges the new policy. The checker
proves the policy facts before any vote: the rule is monotone, each
accepting set holds an honest party, and two accepting sets of the amend
rule overlap in more than f parties. A verifier host replays the log and
checks the certificates. No program can have a runtime error.

The design is `design/DESIGN.md`: the request, verbatim, the design source
note, the S0 rulings and the overlap. The design itself is
`design/self-amending-threshold-authority.html`, verbatim. This file gives
the readings of the request, the route, the slices and the open questions.
"design sec N" is section N of the design. Section 14 lists the questions
for the USER. A reading that is not ruled stays a recommendation.

Status: S4 (the committee words): the Cubical TT kernel of permit-lang
3d47120 and R0 from permit-lang 18deea2 are in, without the affine layer
and the permit domain (Q-S0-10 (a), Q-S2-1 (a)). The Wasm writer emits no
trap form. The domain has `Committee`, `Party`, `Approvals`, `Counts` and
`count`, and the kernel gives `REFUSE_ORBIT` and `REFUSE_BOUND` (Q-S4-1 to
Q-S4-4 (a)). Section 1 has the rulings. Next: S5 (`Rule`, `and`, `or`,
the minimal accepting vectors; section 12).

## 1. Rulings

| Date | Ruling | Source |
|---|---|---|
| 2026-11-03 | Name: ratify-lang. Repo MavenRain/ratify-lang, private at creation. The USER makes it public. | DESIGN.md 3 |
| 2026-11-03 | T1 (primary): a Wasm module and a TinyCC C host, from the lang-template `hosts/tcc-wasm` kit. The policy, the tally rule, the log replay and the certificate check compile to one Wasm module. A C host runs it. Differential tests under node and wasmtime. | DESIGN.md 3 |
| 2026-11-03 | Extra targets (later slices): T2 FHE tally circuit (TFHE or BFV, a static depth bound, CKKS refused), T3 ZK relation export (R_ballot, R_dec), T4 JSON report. | DESIGN.md 3 |
| 2026-11-03 | Not chosen: native C library, JS module, Boolean circuit IR as primary target, Lean 4 export. | DESIGN.md 3 |
| 2026-11-03 | Host: TinyCC. `langc`, `langc mcp` and the T1 host are C99 that TinyCC builds. | DESIGN.md 1, 3 |
| 2026-11-03 | Q-S0-1 (a) "Verdict + Reason (Recommended)": `verify` gives `Verdict = valid \| invalid Reason` with the eight reasons of section 2. | USER, S1 |
| 2026-11-03 | Q-S0-2 (a) "Quorum t in type (Recommended)": `reconstruct` takes a `Quorum t` value; the type carries \|T\| >= t (section 2). | USER, S1 |
| 2026-11-03 | Q-S0-3 (a) "Fin (length + 1) (Recommended)": the height is `Fin (length + 1)` of a given log, so `policyAt` is total (sections 2, 4.1). | USER, S1 |
| 2026-11-03 | Q-S0-4 (a) "REFUSE_BOUND > 2^20 (Recommended)": `REFUSE_BOUND` when the product of (n_j + 1) is above 2^20 (sections 2, 7). | USER, S1 |
| 2026-11-03 | Q-S0-5 (a) "Log CM, System CC (Recommended)": the former table of section 3; TCM-D is ASSUMED. | USER, S1 |
| 2026-11-03 | Q-S0-6 (a) "Eleven codes (Recommended)": the eleven codes of section 7. | USER, S1 |
| 2026-11-03 | Q-S0-7 (a) "Thresholds, and, or, tables (Recommended)": a threshold for each class, `and`, `or`, and a table of count vectors; the checker computes the minimal accepting vectors (section 4.1). | USER, S1 |
| 2026-11-03 | Q-S0-8 (a) "Four buckets (Recommended)": the four counts of section 6; `laws_discharged` counts DISCHARGED rows only. | USER, S1 |
| 2026-11-03 | Q-S0-9 (b) "Real crypto in v1": real crypto in the v1 module, the quorum-lang exponential ElGamal route (quorum-lang be2c329 docs/PLAN.md sec 4.2, line 392). Not the recommendation. Edits: section 2 (the crypto imports are total), 3 (the `Enc` and `Share` representation), 4.1 (the crypto sorts, `Certificate`), 6 (TCM-C, View, O7, O8), 8.2 (T1), 10 (crypto rows), 11 (quorum-lang row), 12 (S7, S9), 13 (OI-10). Q-S1-1 to Q-S1-3 come from this ruling. | USER, S1 |
| 2026-11-03 | Q-S0-10 (a) "permit-lang kernel (Recommended)": the permit-lang kernel without the affine layer (sections 3, 8.1, 11, 12 S2). | USER, S1 |
| 2026-11-03 | Q-S0-11 (a) "Own C99 interpreter (Recommended)": the own C99 Wasm interpreter of the relay-lang ruling, a copy of the permit-lang engine; node and wasmtime are oracles in the gate (section 8.2). | USER, S1 |
| 2026-11-03 | Q-S0-12 (a) "TFHE (Recommended)": T2 is TFHE; the depth bound is per gate (sections 8.2, 12 S12). | USER, S1 |
| 2026-11-03 | Q-S0-13 (b) "Noir": T3 exports R_ballot and R_dec as Noir. Not the recommendation. Edits: section 6 (O8), 8.2 (T3), 10 (golden files), 12 (S13), 13 (OI-11). | USER, S1 |
| 2026-11-03 | Q-S0-14 (a) "permit-lang ruling (Recommended)": A6 follows the permit-lang ruling (count_tokens, equal outputs) (section 9). | USER, S1 |
| 2026-11-03 | Q-S0-15 (a) "Open items, not v1 (Recommended)": the items of design sec 13 stay open items OI-1 to OI-5, not v1 features (section 13). | USER, S1 |
| 2026-11-03 | Q-S1-1 (a) "Host-import ABI (Recommended)": the crypto C code (a port of quorum-lang src/crypto) lives in `ratifyd`; the module calls it through a fixed host-import ABI on real values, not opaque handles (the quorum-lang Q9 (a) ruling). Edits: section 2, 8.2, 11, 12 (S7, S9), 13 (OI-10). | USER, S1 |
| 2026-11-03 | Q-S1-2 (a) "Counts c at T1 (Recommended)": T1 sums the ballots of each class, decrypts the count vector c and applies the rule in the clear. The T1 output leaks c, not only b. T2 gives the one-bit output of design sec 9 stage 3. Edits: section 6 (View rows), 8.2 (T1, T4), 13 (OI-5). | USER, S1 |
| 2026-11-03 | Q-S1-3 (a) "Schnorr BN254 + keccak (Recommended)": Schnorr signatures on BN254 G1 with a keccak-256 challenge; keccak-256 for the entry hash. Edits: section 4.1 (`Certificate`), 8.2, 12 (S7). | USER, S1 |
| 2026-11-03 | Q-S2-1 (a) "Accept until S3 (Recommended)": S2 ports permit-lang 3d47120, the kernel without R0, the affine layer and the domain words. lower.c, wasm.c and ir.h stay out of the build until S3. `langc build` gives `REFUSE_LOWER` "target pending S3". The IR rows, the Wasm modules and the differential cases come back at S3 (the permit-lang Q-S2-3 (a) ruling). Edits: sections 11, 12, 13. | USER, S2 |
| 2026-11-04 | Q-S4-1 (a) "Domain families": `Committee`, `Party`, `Approvals` and `Counts` are families in `domain/domain.lang`, not kernel types. `posWeaken`, `countBits` and `count` are domain definitions. G_P is derived: `Perm` gives one word of adjacent swaps for each class, and `actP` applies the words to an approval set. Edits: section 12. | USER, S4 |
| 2026-11-04 | Q-S4-2 (a) "Seal Approvals": a program definition that is not a law and reads `Approvals` (a fold, an elim or a projection in the syntax type, the quoted type or the body) gives `REFUSE_ORBIT`. A program uses the domain definitions `count` and `actP`. Edits: section 12. | USER, S4 |
| 2026-11-04 | Q-S4-3 (a) "Def types + Committee defs": `REFUSE_BOUND` checks the type of each program definition and each definition of type `Committee`. A type that names `Party`, `Approvals` or `Counts` at a closed committee whose count space (the product of n_j + 1) is above 2^20 is refused, and so is a `Committee` value with such a space. An open committee is not refused. Edits: section 12. | USER, S4 |
| 2026-11-04 | Q-S4-4 (a) "By construction + refl rows": TCM-A holds by construction: a `Perm` has no word across two classes, and `count` counts each class. `examples/committee.lang` has the closed `refl` rows `tcmSwap`, `tcmLift` and `tcmVeto` (`count c (actP c w S) = count c S`), and `figThree` and `vetoTwo` pin the counts 3 of 4 and (2, 0). Edits: section 12. | USER, S4 |

The (a) rulings keep the readings of sections 2 to 11; those sections now
cite the ruled option. The rows for Q-S0-9 (b), Q-S0-13 (b) and Q-S1-1 to
Q-S1-3 list the sections that changed.

The seed is `bin/new-lang.sh ratify-lang tcc-wasm` from lang-template
5265a2a. The kit gives the parser, the checker, the evaluator, the lowering
and the Wasm writer. No kit emits a C verifier host, an FHE circuit, a ZK
constraint system or a JSON report, so these are new code.

Carried default, not ruled for ratify-lang: proofs are guest-language terms
only (quorum-lang ruling, 2026-10-25). Lean export is not a target. The Lean
obligations of the design (O1, O11, Lean T1 to T4) stay on the UAT side
(section 6).

## 2. The reading of R0: no runtime error

R0: no program can trap or surface an exception, on any target.

The reading is the permit-lang reading (permit-lang f9d8a15 docs/PLAN.md
lines 147 to 212), which takes the accrue-lang reading and the relay-lang
host rule N6. Each operation is total on its declared type. Each run of
each target gives a value of the result type. A rejection is a value, never
an error:

- A decision is `Bool` (design sec 4, row Decision). A rule that does not
  accept gives `false`.
- `verify` gives `Verdict = valid | invalid Reason` (design sec 9, stage 6).
  Each reason has a name: `badProof`, `badSignature`, `wrongHeight`,
  `badPrevHash`, `illTypedPolicy`, `notAccepted`, `equivocation`,
  `malformed` (Q-S0-1 (a)).
- A host failure (a socket error, a missing file, bytes that do not parse)
  never reaches the module. The T1 host `ratifyd` maps it to `invalid
  malformed` first (relay-lang rule N6).
- Each crypto import of `ratifyd` (Q-S1-1 (a)) is total: it gives a value
  for each input. A ballot proof or a decryption proof that does not
  verify gives `invalid badProof`. A signature that does not verify gives
  `invalid badSignature`. Bytes that are not a point of BN254 G1 give
  `invalid malformed`. The discrete log of a decrypted count is a bounded
  search over 0 to n_j (quorum-lang be2c329 docs/PLAN.md line 424), and
  the bound of Q-S0-4 keeps it small. S7 sets the reason when no count in
  the range matches.
- A missing ballot is the trivial public encryption of 0 (design sec 9,
  stage 3). It can only lower a count, so it is not an error.

The design has two partial operations. ratify-lang makes them total:

- `reconstruct T xs` is defined only when |T| >= t (design sec 5,
  cryptographic layer). The type carries the bound (Q-S0-2 (a)):
  `reconstruct` takes a `Quorum t` value, and the only way to make one is
  a check of |T| >= t that gives `Bool`.
- `policyAt h` past the end of the log. The height is `Fin (length + 1)` of
  a given `Log`, so `policyAt` is total (Q-S0-3 (a)).

Other sources of a trap and their reading:

- Counts are bounded: 0 <= c_j <= n_j (design sec 4, row Counts). The
  checker refuses a committee whose count space (the product of (n_j +
  1)) is above 2^20, with `REFUSE_BOUND` (Q-S0-4 (a)).
- No division. The O4 and O5 margins use only addition, subtraction that
  stops at 0, and comparison (design sec 11, O5).
- A partial kit word (`head`, `tail`, `pred`, `div`, `rem`) gives
  `REFUSE_PARTIAL` (permit-lang f9d8a15 docs/PLAN.md line 182).
- The Wasm writer emits no `unreachable`, puts a guard before each `div_u`
  and `rem_u`, and gives each `br_table` a default that runs the last arm.
  The entry sets a Flag argument above 1 to 1 (`flag_norm` in
  `src/wasm.c`, permit-lang 18deea2). A C tool `test/notraptool.c` checks each module (permit-lang f9d8a15
  docs/PLAN.md lines 186 and 210; port from permit-lang).
- The kit gate row "a trap is equal to a trap" changes: R0 makes a trap a
  failure on each target (permit-lang f9d8a15 docs/PLAN.md lines 188 to
  201). Open item OI-6.

## 3. The reading of R1

R1: the type constructors are the cell monads and their duals, as
represented computationally in Cubical TT.

ratify-lang takes the permit-lang reading (permit-lang f9d8a15 docs/PLAN.md
section 3, from accrue-lang). The ambient structure is not limited: Pi,
Sigma, `Path`, `PathP`, the interval `I`, `transp`, `hcomp`, the universe
`U`, `Glue` and `ua`. A declared type constructor must be one of two
classes:

- CM, cell monad: the free algebra monad of a cell signature. The parameter
  occurs only in the generator `eta`. Unit is `eta`, bind is the recursor,
  and the laws are paths by induction.
- CC, cocell comonad: the cofree coalgebra comonad of a cocell signature.
  One observation `eps`, each other field `D -> D` or `D -> E -> D`. Counit
  is `eps`, extend is the corecursor.

The formers that the design needs (Q-S0-5 (a)):

| Former | Class | Reading | Design |
|---|---|---|---|
| `Log P Q` | CM (indexed) | The free category on accepted amendments. `nil P` is the unit, `cons e log` adds one cell. `append` is the bind. The category laws (unit, associativity) are paths by induction. | sec 4, sec 5, TCM-E |
| `System` | CC (dual of `Log`) | The cofree comonad of `Amendment -> _`: a Moore machine. `eps` is the policy at the current height, `step : System -> Amendment -> System` moves one height. `policyAt h` of a log is `eps` after h steps. | sec 4 (Elliott behavior), sec 8 |
| `Enc a`, `Share a` | CM, no cells | The identity monad at a base type: `enc` and `share` are the unit, and the denotation is the value (design sec 5: the meaning of `enc pk a` is a). The parameter is a base type only (`Bool`, `Counts`). At T1 the representation is real (Q-S0-9 (b)): `Enc` is an exponential ElGamal ciphertext on BN254 G1 and `Share` is a Shamir share of the DKG key (section 8.2). The gate checks the representation map: the decrypt of the sum of casts equals the counts (quorum-lang S6). | sec 4, sec 5 |
| `Proof R x` | none (a base sort) | A proposition-indexed sort, not a former: the meaning is the claim that a witness exists. | sec 4 |
| `Protocol a b` | none (a base sort) | The meaning is the ideal functionality, a function. Composition holds only up to computational indistinguishability (TCM-D), so it is an ASSUMED law (section 6), not a CM law. | sec 4, TCM-D |

REFUSED formers: general recursion and fixpoints (`REFUSE_FIXPOINT`, design
sec 8), exceptions, effects, a ranked ballot over 3 or more options
(`REFUSE_ARROW`), a clock (`REFUSE_CLOCK`). Section 7 lists the codes.

The kernel source is the permit-lang kernel without the affine layer
(Q-S0-10 (a)). The nearest sibling for ballots under R1 is
quorum-lang (quorum-lang be2c329 docs/PLAN.md section 2, lines 73 to 177;
`FMSet` as CM at line 139). ratify-lang needs no `FMSet`: an approval set is
a subset of a finite committee, a base type.

## 4. The minimal base signature (R2)

Each name cites the design. Nothing outside this list is core (design
DESIGN.md head).

### 4.1 Types

| Type | Meaning | Design |
|---|---|---|
| `Party` | An element of the finite committee. | sec 4 |
| `Committee` | Classes 1 to k with sizes n_1 to n_k. | sec 4 |
| `Approvals` | A subset of the committee. A missing ballot is not an approval. | sec 4 |
| `Counts` | A vector c with 0 <= c_j <= n_j. | sec 4 |
| `Rule` | A monotone `accept : Counts -> Bool` with accept(0) = false and accept(n) = true (O3 as fields). Syntax (Q-S0-7 (a)): a threshold for each class, `and`, `or`, and a table of count vectors. The checker computes the minimal accepting vectors. | sec 4, sec 11 |
| `Policy` | Committee, authorize rule, amend rule, fault bound f, with O4 for both rules and O5 for the amend rule as fields. | sec 4, sec 8 |
| `Decision` | `Bool`. | sec 4 |
| `Amendment P P'` | An approval set S with amend_P(count_P S) = true, and P', a well-typed `Policy`. | sec 5 |
| `Log P Q` | CM (section 3). | sec 4 |
| `System` | CC (section 3). | sec 4 |
| `Height` | `Fin (length + 1)` of a given log (Q-S0-3 (a)). | sec 10 |
| `Enc a`, `Share a`, `Proof R x`, `Protocol a b` | Sorts with laws. Real crypto at T1 (Q-S0-9 (b), section 8.2): `Enc` is an exponential ElGamal ciphertext on BN254 G1, `Share` a Shamir share, `Proof` a sigma proof (disjunctive Chaum-Pedersen for R_ballot, DLEQ for R_dec), `Protocol` the ideal function. | sec 4 |
| `Certificate` | (h, a, ballots, proofs, b), and for an amendment P' and the hash of entry h. The signatures are Schnorr on BN254 G1 with a keccak-256 challenge; the entry hash is keccak-256 (Q-S1-3 (a)). | sec 9, stage 5 |
| `Verdict`, `Reason` | Section 2. | sec 9, stage 6 |

G_P (the class-preserving group, design sec 4) is NOT a user type. The
checker derives it from the classes. A rule reads only `Counts`, so each
rule is orbit-constant by construction (design sec 5: "This one fact
discharges UAT's orbit-constancy obligation for every rule"). A rule that
names one party inside its class cannot be written (`REFUSE_ORBIT`). To
give a party a veto, declare a class with only that party (design sec 4,
"Vetoes and named roles").

### 4.2 Operations

| Operation | Meaning | Design |
|---|---|---|
| `count P S` | The vector of \|S intersect class_j\|. | sec 5 |
| `authorize P a S` | accept_P(count_P S). | sec 5 |
| `and r r'`, `or r r'` | Pointwise on counts. `or` on an amend rule checks O5 again. | TCM-B |
| `nil P`, `cons e log`, `append l1 l2` | The free category. | sec 5, TCM-E |
| `policyAt log h` | P_0 at 0; the target of entry h at h + 1. | sec 5 |
| `enc pk a` | a. | sec 5 |
| `eval g c_1 ... c_m` | g of the meanings. | sec 5 |
| `share t n s`, `reshare C C' t' xs` | s; the same secret, new holders. | sec 5, sec 8 |
| `reconstruct T xs` | s, total by Q-S0-2 (a). | sec 5 |
| `prove R x w` | R(x, w). | sec 5 |
| `run pi xs` | f_pi(xs). | sec 5 |
| `verify cert` | `valid` exactly when the certificate denotes a valid step. | sec 9, stage 6 |

Checker facts (not user operations, shown by `langc check --json` and T4):
the minimal accepting vectors of each rule, the O4 margin (smallest
accepting total minus f), the O5 margin (the smallest
sum_j max(0, a_j + b_j - n_j) minus f over minimal accepting a, b), and the
read-set size n - t + f + 1 for a plain t-of-n amend rule (design sec 10).

## 5. Time

Time is the log height, a natural number (design sec 10: "No shared
clock"). There is no wall clock and no host tick. A name such as `now`,
`deadline`, `timeout` or `expires` gives `REFUSE_CLOCK`. `System` (section
3) is the behavior of design sec 8: a function from height to policy.
`policyAt (h + 1)` reads only `policyAt h` and entry h, so the definition
is causal and total (O10).

## 6. Laws and obligations

Each law is a guest-language term with a `Path` type, or a decision of the
checker on a finite set. `langc check --json` reports four counts in
`progress` (Q-S0-8 (a)): `laws_discharged` and `laws_total` (the DISCHARGED
rows that apply to the program), `laws_assumed`, `laws_at_target` and
`laws_external`. An ASSUMED row is never counted as discharged.

| Row | Statement | Kind | How |
|---|---|---|---|
| TCM-A | authorize is invariant under G_P | DISCHARGED | count is G_P-invariant; a rule reads only counts |
| TCM-B | and, or are pointwise | DISCHARGED | definition; O5 checked again after `or` |
| TCM-C | dec(eval g cs) = g(dec cs) | ASSUMED (T1); AT TARGET (T2) | scheme correctness; at T1, g is the sum of ElGamal ciphertexts, exact while each count is below the group order r (Q-S0-9 (b)) |
| TCM-D | composition of protocols, up to indistinguishability | ASSUMED | UC theorem, outside the kernel |
| TCM-E | nil is the unit, append is associative | DISCHARGED | paths by induction (Lean T4 on the UAT side) |
| View 1 to 3 | the three view equations of design sec 5 | ASSUMED | named hardness and proof-system assumptions; the T1 leakage is the count vector c (Q-S1-2 (a)) |
| O1 | each rule is orbit-constant under G_P | EXTERNAL (Lean) | guest form is TCM-A, counted once |
| O2 | each decision is binary | DISCHARGED | the type of `accept` |
| O3 | monotone, accept(0) = false, accept(n) = true | DISCHARGED | decided on the finite count space |
| O4 | each accepting set holds an honest party | DISCHARGED | decided on the minimal accepting vectors |
| O5 | fork freedom of the amend rule | DISCHARGED | decided on the minimal accepting vectors |
| O6 | FHE exactness for accept . count | AT TARGET (T2) | a static depth bound per circuit |
| O7 | threshold decryption matches the ideal | ASSUMED | DDH and the DLEQ proofs (T1, Q-S0-9 (b)); noise flooding proof (T2) |
| O8 | ZK for R_ballot and R_dec | ASSUMED (T1); exported at T3 | Chaum-Pedersen and DLEQ in the random oracle model (T1); Noir at T3 (Q-S0-13 (b)) |
| O9 | resharing on a committee change | ASSUMED | proactive secret sharing |
| O10 | causality of the log | DISCHARGED | by construction (section 5) |
| O11 | at most one entry per height | EXTERNAL (Lean) | O5 and honest single signing (Lean T3) |

Count at S1 (the rulings change no kind): DISCHARGED 8 (TCM-A, TCM-B, TCM-E, O2, O3, O4, O5, O10),
ASSUMED 8 (TCM-C, TCM-D, three view equations, O7, O8, O9), AT TARGET 1
(O6), EXTERNAL 2 (O1, O11).

## 7. Refusals

The eleven new check codes (Q-S0-6 (a)). Each one gets a `fix` hint at A2.

| Code | When | Design |
|---|---|---|
| `REFUSE_ARROW` | A ranked choice over 3 or more options. | sec 7 |
| `REFUSE_ORBIT` | A rule names a party inside its class. Fix: a class with only that party. | sec 4 box |
| `REFUSE_APPROX` | An approximate scheme (CKKS). | TCM-C |
| `REFUSE_CLOCK` | A wall clock or a host tick. | sec 10 |
| `REFUSE_FIXPOINT` | A policy that ratifies itself, or general recursion. | sec 8 |
| `REFUSE_RULE` | A rule that is not monotone, accepts 0, or does not accept n (O3). | sec 11 |
| `REFUSE_HONEST` | The O4 margin is 0 or less. | sec 11 |
| `REFUSE_FORK` | The O5 margin of the amend rule is 0 or less. | sec 11, TCM-B |
| `REFUSE_BOUND` | The product of (n_j + 1) is above 2^20 (Q-S0-4 (a)). | R0 |
| `REFUSE_PARTIAL` | A partial kit word. | R0 |
| `REFUSE_FORMER` | A type constructor that is not CM or CC. | R1 |

## 8. Route

### 8.1 Kernel

Port the permit-lang Cubical TT kernel without the affine layer (Q-S0-10
(a)), then add R0 (section 2) and the classes check of R1 (section 3). Survey first: SAME, CHANGED,
SOURCE-ONLY.

### 8.2 The four targets

- T1 (primary): one Wasm module for `authorize`, the rule facts,
  `policyAt`, the log replay and `verify`. The C host `ratifyd` reads a
  log file and certificates, runs the module and prints the verdicts.
- T1 crypto (Q-S0-9 (b)): exponential ElGamal ballots on BN254 G1, a
  Pedersen DKG key, threshold decryption with DLEQ proofs, disjunctive
  Chaum-Pedersen ballot proofs, Schnorr signatures on BN254 G1 and
  keccak-256 entry hashes (Q-S1-3 (a)). The C code is a port of
  quorum-lang be2c329 src/crypto (from intent-lang 79e1898). It lives in
  `ratifyd`, and the module calls it through a fixed host-import ABI on
  real values (Q-S1-1 (a), the quorum-lang Q9 (a) ruling). `ratifyd` runs
  only the public steps: the proof and signature checks, the sum, the
  Lagrange combine and the bounded discrete log. A C test tool makes the
  fixtures (the DKG, the casts, the partial decryptions, the signatures),
  as quorum-lang `test/tallytool.c` does.
- T1 tally (Q-S1-2 (a)): ElGamal can only add. T1 sums the ballots of each
  class, decrypts the count vector c and applies the rule in the clear. So
  the T1 output leaks c, not only b. The one-bit output of design sec 9
  stage 3 needs T2.
- T1 engine: the Wasm engine of `ratifyd` is an own C99 interpreter, a
  copy of the permit-lang engine (Q-S0-11 (a), the relay-lang ruling).
  Differential rows under node and wasmtime (OI-10).
- T2: the FHE tally circuit for accept . count in TFHE (Q-S0-12 (a)), with
  a static depth bound per gate (O6). `REFUSE_APPROX` for CKKS.
- T3: R_ballot and R_dec as Noir programs (Q-S0-13 (b)), with golden files
  (O8). See OI-11.
- T4: the JSON report: the policy facts, the minimal accepting vectors, the
  O4 and O5 margins, the read-set size, the per-height log verdicts, and
  what the output leaks for each rule (design sec 13, first item; at T1
  the leakage is c, Q-S1-2 (a)).

### 8.3 Two C programs

`langc mcp` and `ratifyd` are different programs. Do not merge them.

- `langc mcp` is the AUTHORING server for agents that WRITE ratify-lang:
  `check`, `fmt`, `explain`, `list-base`, `show-law`, `build`.
- `ratifyd` is the VERIFIER host: it runs the T1 module on certificates and
  logs. It holds no secret key and casts no ballot.

### 8.4 The demo

A committee in two classes: three operators, and one auditor in a class of
its own (a veto, design sec 4 box). f = 1. Authorize rule: 2 of 3 operators
and the auditor. Amend rule: 3 of 3 operators and the auditor. The checker
shows O4 (margin 2) and O5 (margin 3). A log of two amendments, one of
which adds a fourth operator. `ratifyd` gives a verdict for each of D1 to
D8: a valid authorization, a valid amendment, a bad proof, a bad
signature, a wrong height, an equivocation, an amendment whose P' fails O5
(refused before a vote), and bytes that do not parse (`malformed`).

## 9. Agent adoption (A1 to A6)

The request asks for six items so that agents reach for ratify-lang when it
fits. Each item has one deliverable, one slice and one gate.

| Item | Pasted line | Deliverable | Slice | Gate |
|---|---|---|---|---|
| A1 | "one canonical form per operation and a langc fmt canonical printer" | No aliases, no second syntax. `langc fmt FILE` prints the canonical form. `langc fmt --check` exits 1 when a file is not canonical. | S14 | `fmt` is idempotent; `parse (fmt x)` has the same core term as `x`; each example is canonical. |
| A2 | "langc check --json, giving fix hints and progress counts (parsed, typed, laws discharged)" | One JSON object: `diagnostics` (code, span, expected, found, `fix`) and `progress` (`parsed`, `typed`, `laws_discharged`, `laws_total`, `laws_assumed`, `laws_at_target`, `laws_external`). Schema in `docs/CHECK-JSON.md`. | S15 | Rows in `test/json/`, byte equal. Each code of section 7 has a `fix` row. |
| A3 | "a SKILL.md with 3 to 5 graded examples" | `skills/ratify-lang/SKILL.md` with five graded examples: G1 a plain t-of-n policy, G2 two classes and a veto, G3 an amendment that changes the amend rule, G4 a log replay with `policyAt`, G5 the demo. | S17 | Each example checks; each SKILL.md block and its file are byte equal. |
| A4 | "an MCP server written in TinyCC (langc mcp)" | An MCP server over stdio (JSON-RPC 2.0, one message on each line), in C, in the same `langc`. Tools: section 8.3. The MCP spec version is pinned in `docs/MCP.md`. | S16 | `test/mcptool.c` sends `test/mcp/*.in` and compares the replies with `*.out`. |
| A5 | "AGENTS.md and a README table saying when to use the language" | `AGENTS.md` and the README table "When to reach for ratify-lang". Both exist at S0 as DRAFT; S17 makes them final. | S0, S17 | Review only. |
| A6 | "a token-count comparison against Python and Rust" | G1 to G5 and the demo in ratify-lang, Python and Rust, with equal outputs. Report `docs/TOKEN-BASELINE.md`: the tokenizer pin, each count, each file hash (Q-S0-14 (a): the permit-lang ruling, count_tokens and equal outputs). | S18 | `make tokens`, not in `make check`. |

## 10. Tests

- The kit gate rows, with the R0 change of the trap row (OI-6).
- One check row for each code of section 7, and one `fix` row at A2.
- Eval rows for each operation of section 4.2 on closed data.
- Crypto rows (Q-S0-9 (b)): known-answer rows, invalid-proof and
  invalid-signature rows, and representation rows (the decrypt of the sum
  of casts equals `langc eval` of the counts), as quorum-lang S6 (O80 to
  O84).
- Law rows: the design example (one class of 4, t = 3, f = 1: O5 margin
  2 * 3 - 4 - 1 = 1) and the demo of section 8.4.
- `test/notraptool.c` on each module; differential rows under node and
  wasmtime (T1).
- Golden files for T2, T3 (Noir) and T4.

## 11. Port sources

| Source | What | Where |
|---|---|---|
| lang-template 5265a2a | the tcc-wasm kit | `hosts/tcc-wasm` |
| permit-lang 3d47120, 18deea2, f9d8a15 | 3d47120 (S2, Q-S0-10 (a), Q-S2-1 (a)): the kernel without R0, the affine layer and the domain words: `src/front/` (ast.h, check.c, check.h, core.h, eval.c, front.c, front.h, lexer.c, lexer.h, parser.c), `src/main.c`, `test/fronttool.c`, `test/pathtool.c`, `test/review.sh`, `test/gate.sh`, `domain/domain.lang` and the kernel rows. 18deea2 (S3): the R0 reading, `REFUSE_PARTIAL`, `src/wasmop.h`, the no-trap Wasm writer, `test/notraptool.c`, the trap row; taken at S3 by hand (57 paths take the 18deea2 file, 2 are deleted, 10 are merged; the Wasm and differential rows stay in S3, permit-lang moved them to its S8). f9d8a15: A1 to A6; the C99 Wasm engine (Q-S0-11 (a)) | docs/PLAN.md lines 147 to 212, 214, 628 to 641 (f9d8a15); S2 surveyed the kernel and S3 the R0 port; S10 surveys the rest |
| quorum-lang be2c329 | R1 former table, ballots, the FHE route, the Wasm replica; src/crypto and `test/tallytool.c` (Q-S0-9 (b)); the host-import ABI (Q-S1-1 (a)) | docs/PLAN.md lines 73 to 177, 352 to 523, 524 to 572, 574 |
| orbit-lang 2162fb2 | the orbit projection and the Lan facts (UAT `orbitProjection`, `Aggregation`) | docs/PLAN.md lines 48 to 62 |

## 12. Slices

One session for each slice: scope, gate, mutants, docs, close.

| Slice | Work | Status |
|---|---|---|
| S0 | Seed from tcc-wasm, DESIGN.md and the design HTML, this plan, README table, AGENTS.md; gate EXIT 0; seed commit; private repo | DONE |
| S1 | The USER rules section 14 and Q-S1-1 to Q-S1-3. Section 1 has the rulings. | DONE |
| S2 | Port the Cubical TT kernel (8.1, Q-S0-10 (a), Q-S2-1 (a)): permit-lang 3d47120, no affine layer, no R0. Gate: 29 examples, 17 parse refusals, 154 check refusals, 10 kit-only refusals, 1 time hint, 78 evals, 1 build refusal; the review, domain and path regressions pass. Mutants M1 (path ends), M2 (glue face), M3 (universe level): KILLED. | DONE |
| S3 | Port permit-lang 3d47120..18deea2: R0 in the kernel and the Wasm writer, `REFUSE_PARTIAL`, `src/wasmop.h`, `test/notraptool.c`, the trap row (OI-6). `src/front/lower.c`, `src/wasm.c` and `src/ir.h` come back into the build, with the IR rows, the Wasm modules and the differential cases (OI-12). Gate: 30 examples, 17 parse refusals, 158 check refusals, 10 kit-only refusals, 1 time hint, 79 evals, 1 build, 1 ir lowered, 5 ir refusals, 2 ir lowerings, 4 notrap modules, 7 notrap bad rows, 10 spine evals, 33 Wasm modules valid, 998 differential cases with 0 mismatches. Mutants M1, M2 (`REFUSE_PARTIAL`), M3 (the div guard of the no-trap tool), M4 (`flag_norm`): KILLED. M5 (the `br_table` default) and M6 (the IF condition): equivalent. | DONE |
| S4 | `Party`, `Committee`, `Approvals`, `Counts`, `count`; G_P derived; TCM-A; `REFUSE_ORBIT`, `REFUSE_BOUND`. The four words are families in `domain/domain.lang`, with the definitions `posWeaken`, `countBits` and `count`; G_P is `Perm` and `actP` (Q-S4-1 (a)). `REFUSE_ORBIT`: a program definition that is not a law reads `Approvals` (Q-S4-2 (a)). `REFUSE_BOUND`: a definition type, or a definition of type `Committee`, at a closed committee with a count space above 2^20 (Q-S4-3 (a)). TCM-A: by construction, and the closed `refl` rows `tcmSwap`, `tcmLift` and `tcmVeto` in `examples/committee.lang` (Q-S4-4 (a)). Rule R4: the domain name `count` clashed with three guest names, so `examples/formers.lang` has `counted`, and `examples/state-no-history.lang` and `test/check/state-boundary.lang` have the state field `stateCount`. Gate: 32 examples, 17 parse refusals, 162 check refusals, 10 kit-only refusals, 1 time hint, 81 evals, 1 build, 1 ir lowered, 5 ir refusals, 2 ir lowerings, 4 notrap modules, 7 notrap bad rows, 10 spine evals, 35 Wasm modules valid, 998 differential cases with 0 mismatches. Mutants M1 (the bound compare), M2 (the factor n_j + 1), M3 (the elim case of `orbit_uses`), M4 (the approval arm of `countBits`): KILLED. | DONE |
| S5 | `Rule`, `and`, `or`, the minimal accepting vectors; O3, O4, O5; `REFUSE_RULE`, `REFUSE_HONEST`, `REFUSE_FORK`; TCM-B. | NEXT |
| S6 | `Policy`, `Decision`, `authorize`, `Amendment`, `Log` (CM), `System` (CC), `policyAt`; TCM-E, O2, O10; `REFUSE_FIXPOINT`, `REFUSE_ARROW`, `REFUSE_CLOCK`. | |
| S7 | The crypto layer (Q-S0-9 (b), Q-S1-1 (a), Q-S1-3 (a)): `Enc`, `Share`, `Proof`, `Protocol` and their operations; port quorum-lang src/crypto and add Schnorr on BN254 G1; the test tool and the crypto rows (section 10); the ASSUMED rows; `REFUSE_APPROX`. | |
| S8 | `Certificate`, `Verdict`, `Reason`, `verify`, equivocation evidence. | |
| S9 | T1 part 1: the Wasm module, the crypto import ABI (Q-S1-1 (a)), the notrap rows. | |
| S10 | T1 part 2: `ratifyd`, the Wasm engine (Q-S0-11 (a)), the demo D1 to D8. | |
| S11 | T4: the JSON report, `docs/REPORT-JSON.md`. | |
| S12 | T2: the TFHE tally circuit (Q-S0-12 (a)), the depth bound (O6). | |
| S13 | T3: R_ballot and R_dec as Noir (Q-S0-13 (b)), golden files. | |
| S14 | A1: canonical forms and `langc fmt`. | |
| S15 | A2: `langc check --json`. | |
| S16 | A4: `langc mcp`. | |
| S17 | A3 and A5: SKILL.md, G1 to G5, AGENTS.md and the README table final. | |
| S18 | A6: the token comparison. | |
| S19 | Close: SPEC.md, probe/CAPABILITY.md, VALIDATION.md, final gate. | |

## 13. Open items

The design obligations are O1 to O11. The open items of this plan are
OI-1 onward, so the two lists do not clash. OI-1 to OI-5 are the items of
design sec 13. They stay open items, not v1 features (Q-S0-15 (a)).

- OI-1 Committee loss can freeze the system (design sec 13). Recovery needs
  a clock. Not a v1 feature.
- OI-2 Stale certificates (design sec 13). `verify` takes the latest
  verified height as an input; the freshness rule is a later ruling.
- OI-3 Agenda control (design sec 13). The log records the order only.
- OI-4 Receipts (design sec 13). ZK and FHE give ballot secrecy, not
  receipt-freeness.
- OI-5 The output leaks what the rule implies (design sec 13). T4 states it
  for each rule. At T1 the output also leaks the count vector c (Q-S1-2
  (a)). T2 gives the one-bit output.
- OI-6 CLOSED at S3. The kit row "a trap is equal to a trap" (section 2)
  is gone: in the Wasm and differential rows a `trap` line is a FAIL. The
  row `pick 2 4 9` gives 4 and does not trap: the entry sets a Flag
  argument of 2 or more to 1 (`flag_norm`, mutant M4 kills it).
- OI-7 The view equations hold only up to indistinguishability and need a
  computational model (design sec 13). They stay ASSUMED.
- OI-8 The class-preserving group action is new Lean work on the UAT side
  (design sec 3, O1). Lean export is not a target.
- OI-9 Availability and the MPC tally route (design sec 9 and sec 10) are
  implementation choices. No target emits them in v1.
- OI-10 The node and wasmtime oracles of the differential rows need the
  crypto imports (Q-S1-1 (a)). S9 surveys the quorum-lang S8 answer.
- OI-11 The native field of Noir is the BN254 scalar field. The
  coordinates of a BN254 G1 point are in the base field, so the Noir
  R_ballot and R_dec of the ElGamal route need non-native field arithmetic
  (Q-S0-13 (b)). S13 measures the cost.
- OI-12 The gate gap from S2 to S3 (Q-S2-1 (a)). `src/front/lower.c`,
  `src/wasm.c` and `src/ir.h` are out of the build, and `langc build` gives
  `REFUSE_LOWER` "target pending S3". Until S3 the gate has no IR rows, no
  Wasm modules and no differential cases. CLOSED at S3: the gate has the
  IR rows, 33 Wasm modules and 998 differential cases with 0 mismatches.
  The kit had 13 modules and 813 cases. The list now holds the 30
  examples, `test/ir/natops.lang`, `test/ir/block.lang` and `test/ir/proof-trap.lang`;
  `natops.lang` gives 610 cases and `examples/entries.lang` 310.
- OI-13 `REFUSE_ORBIT` (S4) also walks the quoted type of a definition.
  The quote unfolds `count c S` on an open `S` to an elim on `Approvals`,
  so a definition that is not a law and has `count c S` in its type gives
  `REFUSE_ORBIT` (S4 probe). (a), recommended: S5 `Rule` does not put
  `count c S` in the type of a definition that is not a law. (b): S5
  drops the quoted-type walk of `REFUSE_ORBIT`.

## 14. Questions for the USER (S0)

Each question has a recommendation (rec). S1 recorded the rulings
(2026-11-03, section 1).

- Q-S0-1 Rejections. RULED (a). (a) rec: `Verdict = valid | invalid Reason` with the
  eight reasons of section 2. (b) `Bool` only, and the host prints the
  reason.
- Q-S0-2 `reconstruct`. RULED (a). (a) rec: the type carries |T| >= t (a `Quorum t`
  value). (b) A result sum `reconstructed s | tooFew`.
- Q-S0-3 `policyAt` past the log end. RULED (a). (a) rec: the height is `Fin (length
  + 1)` of a given log. (b) The last policy for each h past the end.
- Q-S0-4 Bound of the count space. RULED (a). (a) rec: `REFUSE_BOUND` when the product
  of (n_j + 1) is above 2^20, so O3 to O5 stay finite and fast. (b) No
  bound; O4 and O5 in closed form for plain thresholds only.
- Q-S0-5 R1 classes. RULED (a). (a) rec: `Log` CM, `System` CC, `Enc` and `Share` the
  identity CM at base types, `Proof` and `Protocol` base sorts, TCM-D
  ASSUMED (section 3). (b) `Protocol` as a CM (the free category on
  stages), with TCM-D still ASSUMED.
- Q-S0-6 Refusal codes. RULED (a). (a) rec: the eleven codes of section 7. (b) Fewer
  codes: one `REFUSE_POLICY` for O3 to O5.
- Q-S0-7 Rule syntax. RULED (a). (a) rec: a threshold for each class, `and`, `or`, and
  a table of count vectors; the checker computes the minimal accepting
  vectors. (b) Any total function on `Counts`, with O3 decided by
  enumeration.
- Q-S0-8 Law counts. RULED (a). (a) rec: the four buckets of section 6;
  `laws_discharged` counts DISCHARGED rows only. (b) One `laws_total` for
  all rows, with ASSUMED rows listed as not discharged.
- Q-S0-9 Crypto layer at T1. RULED (b). (a) rec: abstract sorts with laws; the
  reference evaluator computes the denotation (design sec 5); the Wasm
  module takes opaque handles from `ratifyd` imports; real FHE and ZK
  arrive at T2 and T3. (b) Real crypto in the v1 module (the quorum-lang
  exponential ElGamal route, quorum-lang be2c329 docs/PLAN.md line 392).
- Q-S0-10 Kernel source. RULED (a). (a) rec: the permit-lang kernel without the affine
  layer (permit-lang took it from relay-lang). (b) The quorum-lang kernel.
  (c) Extend the kit checker in place.
- Q-S0-11 Wasm engine in `ratifyd`. RULED (a). (a) rec: the own C99 interpreter of the
  relay-lang ruling (copy the permit-lang engine); node and wasmtime are
  oracles in the gate. (b) Vendor wasm3. (c) The wasmtime C API.
- Q-S0-12 T2 scheme. RULED (a). (a) rec: TFHE (Boolean gates, bootstrapping, the depth
  bound is per gate). (b) BFV (a levelled circuit, the depth bound is the
  multiplicative depth).
- Q-S0-13 T3 format. RULED (b). (a) rec: Circom (R1CS). (b) Noir.
- Q-S0-14 Token comparison. RULED (a). (a) rec: the permit-lang ruling (count_tokens,
  equal outputs). (b) An offline tokenizer.
- Q-S0-15 Design sec 13 items. RULED (a). (a) rec: open items OI-1 to OI-5, not v1
  features. (b) Pick one for v1 (for example, the freshness rule of OI-2).

### Questions from Q-S0-9 (b) (S1)

- Q-S1-1 Where the T1 crypto runs. RULED (a). (a) rec: the port of
  quorum-lang src/crypto lives in `ratifyd`; the module calls it through a
  fixed host-import ABI on real values (the quorum-lang Q9 (a) ruling).
  (b) BN254 arithmetic as guest code, lowered into the Wasm module.
- Q-S1-2 The T1 tally output. RULED (a). (a) rec: T1 decrypts the count
  vector c and applies the rule in the clear; the T1 leakage is c; T2
  gives the one-bit output. (b) Only b at T1, by an MPC comparison (OI-9
  in v1).
- Q-S1-3 Signature and hash. RULED (a). (a) rec: Schnorr on BN254 G1 with
  a keccak-256 challenge; keccak-256 for the entry hash. (b) BIP-340
  Schnorr on secp256k1 with SHA-256.
