# ratify-lang status

The build implements slice S7 of `docs/PLAN.md`: the Cubical TT kernel,
ported from permit-lang 3d47120, R0 in the kernel and the Wasm writer,
ported from permit-lang 18deea2, the committee words `Committee`,
`Party`, `Approvals`, `Counts` and `count` (Q-S4-1 to Q-S4-4 (a)), the
rule words `Rule`, `accept`, `Honest` and `ForkFree` with O3, O4 and O5
(Q-S5-1 to Q-S5-3 (a)), the policy words `Policy`, `Decision`,
`authorize`, `Amendment`, `Log`, `System` and `policyAt` with TCM-E, O2
and O10 (Q-S6-1 to Q-S6-4 (a)), and the crypto words `Enc`, `eval`,
`Share`, `Proof` and `Protocol` with the crypto rows and `REFUSE_APPROX`
(Q-S7-1 to Q-S7-6 (a)). The affine layer and the permit domain are not
in the build (Q-S0-10 (a), Q-S2-1 (a)). It does not complete M0. The
next slice is S8: the certificate words.

## Implemented

- The front end in `src/front/`: lexer, parser, elaboration and type
  check, evaluation.
- Pi and Sigma types, and the universes up to the largest level. A
  universe above it gives `TYPE_UNIVERSE`.
- The interval, path types and path application `@`. The end check of a
  path gives `PATH_END` (or `PATH_UIP` when the ends are paths).
- Transport, hcomp, Glue and `ua`. The face check of glue gives
  `GLUE_BOUNDARY`.
- Families from `domain/domain.lang`, with indices (former F15), path
  constructors and `fold`.
- The kernel prelude in `domain/domain.lang` (`domain/README.md` lists
  the names).
- R0 (`docs/PLAN.md` section 2): the Nat operations are total, and a
  partial kit word gives `REFUSE_PARTIAL`.
- The IR lowering (`src/front/lower.c`, `src/ir.h`) and the Wasm writer
  `src/wasm.c`. The writer emits no trap form; `test/notraptool.c` checks
  each built module. The entry sets a Flag argument above 1 to 1.
- S4: the committee words in `domain/domain.lang`. `Committee` is a list
  of classes (`cmNil`, `cmClass`). `Party`, `Approvals` and `Counts` are
  families on a committee. `count` gives the count of each class of an
  approval set. `Perm` (one word of adjacent swaps for each class) and
  `actP` give G_P. `examples/committee.lang` has the TCM-A rows `tcmSwap`,
  `tcmLift` and `tcmVeto` as closed `refl` paths.
- S4: two refusals in `src/front/check.c`. `REFUSE_ORBIT`: a program
  definition that is not a law reads `Approvals`. `REFUSE_BOUND`: a
  definition type names `Party`, `Approvals` or `Counts` at a closed
  committee whose count space (the product of n_j + 1) is above 2^20, or
  a definition of type `Committee` has such a value.
- S4: the domain definition `count` is a core name (rule R4). Three guest
  names changed: `count` in `examples/formers.lang` is now `counted`, and
  the state field `count` in `examples/state-no-history.lang` and
  `test/check/state-boundary.lang` is now `stateCount`.
- S5: the rule words in `domain/domain.lang`. `Rule c` is `rlMin` (a bar
  for each class), `rlAnd`, `rlOr` or `rlTable` (an exact table
  `CtTable c`), and `accept` applies a rule to a count vector. `Honest c
  f` and `ForkFree c f` hold one rule for the fault bound `f : Size`
  (Q-S5-1 (a), Q-S5-2 (a)).
- S5: three refusals in `src/front/check.c`. For a program definition
  that is not a law, with the type `Rule`, `Honest` or `ForkFree` at a
  closed committee, the kernel fills the accept table of the count space
  and computes the minimal accepting vectors. `REFUSE_RULE`: the rule is
  not monotone, accepts 0 or does not accept n (O3). `REFUSE_HONEST`: the
  O4 margin is 0 or less. `REFUSE_FORK`: the O5 margin is 0 or less.
  `examples/rule.lang` has the TCM-B rows `tcmAnd` and `tcmOr` as closed
  `refl` paths. The quoted-type walk of `REFUSE_ORBIT` stays (Q-S5-3
  (a)).
- S6: the policy words in `domain/domain.lang`. `Decision` is `Sum Unit
  Unit`. `Policy` is `plOf` with the committee `plCom`, the fault bound
  `plF`, the rule `plAuth : Honest plCom plF` and the amend rule `plAmend
  : ForkFree plCom plF` (Q-S6-1 (a)). `authorize` and `amendOk` give a
  `Decision` from the approvals of the policy committee. `Amendment P Q`
  is `amOf`. `Log P Q` has `lgNil` and `lgCons`, `append` is the bind, and
  `appendNilL`, `appendNilR` and `appendAssoc` are the TCM-E paths.
  `policyAt` takes a height `Pos (szS (lgLen P Q l))`. `System` is
  `Stream Policy`, and `systemOf` builds it by `sCorec` (Q-S6-4 (a)).
  `examples/policy.lang` has the O10 rows `o10At0` and `o10Past`.
- S6: refusals in `src/front/check.c`. `REFUSE_ARROW`: the type of a
  guest definition takes `Approvals` or `Counts` and gives a closed
  choice of three or more values (Q-S6-3 (a)). `REFUSE_CLOCK` and
  `REFUSE_FIXPOINT` are the kit codes `REFUSE_TIME` and `REFUSE_REC` with
  new names, and `deadline`, `timeout` and `expires` are clock words too
  (Q-S6-2 (a)). The seal: `hoRule` and `ffRule` can only be the whole
  body of a closed decided definition, else `REFUSE_HONEST` or
  `REFUSE_FORK`. The seal is a walk over the written term (the type and
  the body of the definition), not over the core term (Q-S6-1 (a)).
- S7: the crypto words in `domain/domain.lang`. `Base` is the code sort
  (`baseBool`, `baseCounts c`), and `El` gives the type of a code
  (Q-S7-1 (a)). `Enc b` is `encOf`; `enc`, `encMeaning` and `eval` (two
  inputs) give the homomorphic step. `Share b` is `shareOf`; `share`,
  `reconstruct` and `reshare` keep the secret. `Proof X W R x` is
  `Trunc1 (Sigma (w : W) (R x w))`, and `prove` gives it (Q-S7-6 (a)).
  `Protocol A B` is the function type `A -> B`, with `protoOf` and
  `run`. The denotation of each word is the value, so eval needs no
  crypto. `examples/crypto.lang` has the rows.
- S7: `src/crypto/` has the 20 files of quorum-lang src/crypto, byte
  equal, and `schnorr.c` and `schnorr.h`: Schnorr on BN254 G1, with a
  signature (R, s) of 96 bytes (Q-S7-2 (a), Q-S7-3 (a)).
  `build/cryptotool` (`test/cryptotool.c`) runs 97 rows: 23
  known-answer, 6 invalid-signature, 43 threshold, 17 reshare and 8
  invalid-proof. `build/cryptotool rep` prints 14 eval lines, and the
  gate runs `langc eval` on each (TCM-C at T1).
- S7: `REFUSE_APPROX` in `src/front/check.c`. The names `ckks`,
  `approx`, `approximate`, `float` and `double` are refused at the four
  sites of the clock check, with the hint "an exact scheme only"
  (Q-S7-5 (a)).
- The gate: 35 examples, 17 parse refusals, 179 check refusals, 10
  kit-only refusals, 1 time hint, 94 evals, 14 crypto rep lines, 1
  build, 1 ir lowered, 5 ir refusals, 2 ir lowerings, 4 notrap modules,
  7 notrap bad rows, 10 spine evals, 38 Wasm modules valid, 998
  differential cases with 0 mismatches, 97 crypto rows; the review,
  domain, path and crypto regressions. The C runner also checks the
  restored unused-field and proof-transport fixtures; each differential
  module passes `build/notraptool`.

## Laws

The kernel does not prove an ASSUMED row of `docs/PLAN.md` section 6
(Q-S7-4 (a)). The table gives the rows that test each one.

| Law | Kind | Rows |
|---|---|---|
| TCM-C | ASSUMED (T1); AT TARGET (T2) | the 14 crypto rep lines: the gate runs `langc eval` on each and compares the count of `build/cryptotool` |
| TCM-D | ASSUMED | none: the UC theorem is outside the kernel |
| View 1 to 3 | ASSUMED | none: named hardness and proof-system assumptions |
| O7 | ASSUMED | the 43 threshold rows and the 8 invalid-proof rows |
| O8 | ASSUMED (T1); exported at T3 | the 8 invalid-proof rows |
| O9 | ASSUMED | the 17 reshare rows |

The 23 known-answer rows and the 6 invalid-signature rows test the
hashes, the curve code and Schnorr on BN254 G1.

## Remaining M0 work

1. S8: the certificate words of `docs/PLAN.md` section 12: `Certificate`,
   `Verdict`, `Reason`, `verify` and the equivocation evidence. It needs
   the S7 commit first.
2. S9 to S19: the targets T1 to T4 (S9: the Wasm module, the crypto
   import ABI and the notrap rows), A1 to A6 and the close. Each slice
   needs the slice before it.

## Known limits

- The affine syntax (`-o`, `@0`, `@1`, `unpack`, `Budget`) is not in the
  language. Four gate rows give a parse or check refusal for it
  (`LEX_CHAR`, `NAME_UNKNOWN`, `TYPE_MISMATCH`), not an affine code.
- `REFUSE_CELL` holds the CM and CC shape check inline in the kernel.
  The R1 slice renames it or adds `REFUSE_FORMER`.
- `Fin` is internal (O23), so `REFUSE_ARROW` checks only `Sum` trees and
  `Pos n`.
- The crypto words are a denotation: `enc` keeps the value, and `eval`
  applies the function to the values. The real scheme runs only in
  `build/cryptotool` until the crypto import ABI (S9).
- `REFUSE_APPROX` reads names only. A name that is not in its list of
  five is not refused.

## Internal boundaries

`src/front/lexer.c` and `src/front/parser.c` read text into terms
(`src/front/ast.h`). `src/front/check.c` elaborates terms to core terms
(`src/front/core.h`) and gives the refusal codes. `src/front/eval.c`
evaluates core terms. `src/front/front.c` loads the domain, then the
program. `src/front/lower.c` lowers a checked
definition to the IR (`src/ir.h`), and `src/wasm.c` writes a Wasm
module from the IR. `src/main.c` holds the commands.

`gen/embed.c` copies `domain/domain.lang` into `build/domain.c` at build
time, so the domain is part of `build/langc`.

`test/gate.sh` runs the examples and the rows in `test/parse/`,
`test/check/` and `test/eval/` against their `expect.txt` files, and the
IR, Wasm and differential rows. `build/notraptool` checks each built Wasm
module for a trap form.
`test/review.sh` runs the review regressions. `build/fronttool` runs the
domain regressions and `build/pathtool` the path regressions.
`make check` also compiles all sources with clang and `-Werror`.
`build/cryptotool` (`test/cryptotool.c` and `src/crypto/`) runs the
crypto rows and prints the rep lines for `test/gate.sh`.
