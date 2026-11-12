# ratify-lang status

The build implements the slices S0 to S19 of `docs/PLAN.md`: the Cubical
TT kernel, ported from permit-lang 3d47120, R0 in the kernel and the
Wasm writer, ported from permit-lang 18deea2, the committee words
`Committee`, `Party`, `Approvals`, `Counts` and `count` (Q-S4-1 to
Q-S4-4 (a)), the rule words `Rule`, `accept`, `Honest` and `ForkFree`
with O3, O4 and O5 (Q-S5-1 to Q-S5-3 (a)), the policy words `Policy`,
`Decision`, `authorize`, `Amendment`, `Log`, `System` and `policyAt`
with TCM-E, O2 and O10 (Q-S6-1 to Q-S6-4 (a)), the crypto words `Enc`,
`eval`, `Share`, `Proof` and `Protocol` with the crypto rows and
`REFUSE_APPROX` (Q-S7-1 to Q-S7-6 (a)), and the certificate words
`Reason`, `Verdict`, `Evidence`, `EvList`, `equivocates`, `Certificate`
and `verify` with the certificate rows and a faster O5 pass (Q-S8-1 to
Q-S8-5 (a), Q-S8-6 (b)). S9 and S10 add the target T1: the Wasm module,
the crypto import ABI, the verifier host `ratifyd` with its Wasm engine,
and the demo D1 to D8. S11 adds the JSON report `langc report` (T4), S12
adds the TFHE tally circuit `langc tfhe` (T2), and S13 adds the Noir
relations `langc noir` (T3). S14 adds `langc fmt`, S15 adds
`langc check --json`, and S16 adds `langc mcp` with the verbs
`langc explain`, `langc list-base` and `langc show-law`. S17 adds the
skill `skills/ratify-lang/SKILL.md` with the graded examples G1 to G5,
`test/skilltool.c`, the final `AGENTS.md` and the final README table.
S18 adds the token comparison with Python and Rust (A6):
`docs/TOKEN-BASELINE.md`, `test/tokcount.c` and the baselines in
`test/baseline/`. By the offline estimate, the token sum of the 5 Python
files is 0.40 of the ratify-lang sum, and the Rust sum is 0.58. S18 did
not run `make tokens` (the count_tokens route). The gate has 65 count
lines (S17: 63). The affine layer and the permit domain are not in the
build (Q-S0-10 (a), Q-S2-1 (a)). S19 is the close: `SPEC.md`,
`probe/CAPABILITY.md`, `docs/VALIDATION.md` and the final gate. S19
completes M0, and the plan is CLOSED. `docs/PLAN.md` section 13 gives
the open items.

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
- S8: the certificate words in `domain/domain.lang`. `Reason` has the 8
  reasons of Q-S8-3 (a), and `Verdict` is `valid` or `invalid r`.
  `Certificate P` is `certOf` with the height, the action, the signer
  set, b, P' and one `Decision` for each host byte check (Q-S8-1 (a)).
  `verify P Q l c evs` checks c against the tip Q of `l : Log P Q`, and
  the first failure wins (Q-S8-2 (a), Q-S8-3 (a)). `Evidence c` is
  `evOf`, `equivocates` holds for two signed bodies at one height with
  two hashes, and `EvList c` is the input list (Q-S8-4 (a)).
  `examples/certificate.lang` has 18 closed `refl` rows.
- S8: `test/cryptotool.c` has the cert section, 12 rows (Q-S8-5 (a)):
  the body bytes (height 8, prev hash 32, action 8, big endian), Schnorr
  over keccak256 of the body, the check order height, prev hash,
  signature, and the evidence rows.
- S8: the O4 and O5 margins (`rule_margins` in `src/front/check.c`) come
  from one upward pass over the count space, O(space k), with no list of
  minimal accepting vectors (Q-S8-6 (b), OI-14 closed). 20 classes of 1
  (a count space of 2^20) check in about 12 s
  (`examples/fork-margin.lang`).
- The gate: 37 examples, 17 parse refusals, 180 check refusals, 10
  kit-only refusals, 1 time hint, 112 evals, 14 crypto rep lines, 1
  build, 1 ir lowered, 5 ir refusals, 2 ir lowerings, 4 notrap modules,
  7 notrap bad rows, 10 spine evals, 40 Wasm modules valid, 998
  differential cases with 0 mismatches, 109 crypto rows; the review,
  domain, path and crypto regressions. The C runner also checks the
  restored unused-field and proof-transport fixtures; each differential
  module passes `build/notraptool`.
- `langc check PROG --json` (S15, `docs/CHECK-JSON.md`): one JSON object
  in the RFC 8785 form with `diagnostics` (0 or 1 object: `code`,
  `expected`, `fix`, `found`, `message`, `span`) and `progress` (the
  `parsed` and `typed` flags and the law counts). `src/front/fix.c` has
  one fix text for each code (71 rows; S15: 69). In `make check`,
  `build/checkjsontool` checks the 21 rows of `test/json/` and the fix
  table.
- `langc mcp` (S16, `docs/MCP.md`): an MCP server on stdin and stdout
  (MCP `2025-11-25`) with the 6 tools `check`, `fmt`, `explain`,
  `list-base`, `show-law` and `build`. Each tool is a CLI verb; S16 adds
  the verbs `langc explain CODE`, `langc list-base` and `langc show-law
  [ROW]`. In `make check`, `build/mcptool` checks the 10 rows of
  `test/mcp/`.

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

1. None: the plan is CLOSED. `docs/PLAN.md` section 13 gives the open
   items.

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
- The guest has no hash type: a hash in `Certificate` and `Evidence` is
  a `Size` stand-in compared with `szEq`, and the action is a `Size`
  code. The host byte checks (the cert rows of `build/cryptotool`) use
  the real keccak-256 hash and the real Schnorr signature.
- `REFUSE_APPROX` reads names only. A name that is not in its list of
  five is not refused.
- `REFUSE_FORMER` has a fix row in `src/front/fix.c` and a row in
  section 7 of `docs/PLAN.md`, but no site of the checker gives it.
  `REFUSE_CELL` is the code of that refusal today.
- In the kit mode, `langc check --json` decides no rule definition.
  Thus a kit program has the L trigger only, and the kit demo row has
  the law counts 2, 1, 0 and 1 (`docs/CHECK-JSON.md`).
- `langc mcp` supports MCP `2025-11-25` only: it does not support
  2026-07-28. The tools do not include `tfhe`, `noir`, `report`, `ir`,
  `abi` or `fmt --check`: these are CLI only (`docs/MCP.md`).
- The `main` and `runtime` targets of the `build` tool give the same
  bytes today. Thus the gate line `mcp builds` cannot see a swap of the
  2 targets (Q-U3-1 (a)).

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
