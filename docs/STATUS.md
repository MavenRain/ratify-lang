# ratify-lang status

The build implements slice S3 of `docs/PLAN.md`: the Cubical TT kernel,
ported from permit-lang 3d47120, and R0 in the kernel and the Wasm
writer, ported from permit-lang 18deea2, without the affine layer and the
permit domain (Q-S0-10 (a), Q-S2-1 (a)). It does not complete M0. The
next slice is S4: `Party`, `Committee`, `Approvals`, `Counts`, `count`.

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
- The gate: 30 examples, 17 parse refusals, 158 check refusals, 10
  kit-only refusals, 1 time hint, 79 evals, 1 build, 1 ir lowered, 5 ir
  refusals, 2 ir lowerings, 4 notrap modules, 7 notrap bad rows, 10 spine
  evals, 33 Wasm modules valid, 998 differential cases with 0 mismatches;
  the review, domain and path regressions. The C runner also checks the
  restored unused-field and proof-transport fixtures; each differential
  module passes `build/notraptool`.

## Remaining M0 work

1. S4: `Party`, `Committee`, `Approvals`, `Counts`, `count`; G_P
   derived; TCM-A; `REFUSE_ORBIT`, `REFUSE_BOUND`. It needs the S3 commit
   first.
2. S5 to S8: the rest of the policy layer of `docs/PLAN.md` section 12
   (`Rule`, `Policy`, the crypto layer, `Certificate`). Each slice needs
   the slice before it.
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
