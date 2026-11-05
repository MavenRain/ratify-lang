# ratify-lang status

The build implements slice S5 of `docs/PLAN.md`: the Cubical TT kernel,
ported from permit-lang 3d47120, R0 in the kernel and the Wasm writer,
ported from permit-lang 18deea2, the committee words `Committee`,
`Party`, `Approvals`, `Counts` and `count` (Q-S4-1 to Q-S4-4 (a)), and
the rule words `Rule`, `accept`, `Honest` and `ForkFree` with O3, O4 and
O5 (Q-S5-1 to Q-S5-3 (a)). The affine layer and the permit domain are not
in the build (Q-S0-10 (a), Q-S2-1 (a)). It does not complete M0. The next
slice is S6: `Policy`, `Decision`, `authorize`, `Amendment`, `Log`.

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
- The gate: 33 examples, 17 parse refusals, 168 check refusals, 10
  kit-only refusals, 1 time hint, 85 evals, 1 build, 1 ir lowered, 5 ir
  refusals, 2 ir lowerings, 4 notrap modules, 7 notrap bad rows, 10 spine
  evals, 36 Wasm modules valid, 998 differential cases with 0 mismatches;
  the review, domain and path regressions. The C runner also checks the
  restored unused-field and proof-transport fixtures; each differential
  module passes `build/notraptool`.

## Remaining M0 work

1. S6: `Policy`, `Decision`, `authorize`, `Amendment`, `Log` (CM),
   `System` (CC), `policyAt`; TCM-E, O2, O10; `REFUSE_FIXPOINT`,
   `REFUSE_ARROW`, `REFUSE_CLOCK`. `Policy c f` holds one `Honest` and one
   `ForkFree` (Q-S5-2 (a)). It needs the S5 commit first.
2. S7 and S8: the rest of the policy layer of `docs/PLAN.md` section 12
   (the crypto layer, `Certificate`). Each slice needs the slice before
   it.
3. S9 to S19: the targets T1 to T4, A1 to A6 and the close.

## Known limits

- The affine syntax (`-o`, `@0`, `@1`, `unpack`, `Budget`) is not in the
  language. Four gate rows give a parse or check refusal for it
  (`LEX_CHAR`, `NAME_UNKNOWN`, `TYPE_MISMATCH`), not an affine code.
- `REFUSE_CELL` holds the CM and CC shape check inline in the kernel.
  The R1 slice renames it or adds `REFUSE_FORMER`.

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
