# ratify-lang status

The build implements slice S2 of `docs/PLAN.md`: the Cubical TT kernel,
ported from permit-lang 3d47120 without R0, the affine layer and the
permit domain (Q-S0-10 (a), Q-S2-1 (a)). It does not complete M0. The
next slice is S3: R0 in the kernel and the Wasm writer.

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
- The gate: 29 examples, 17 parse refusals, 154 check refusals, 10
  kit-only refusals, 1 time hint, 78 evals, 1 build refusal; the review,
  domain and path regressions.

## Remaining M0 work

1. S3: R0 in the kernel and the Wasm writer (`REFUSE_PARTIAL`, the
   no-trap rows, the trap row OI-6). It brings `src/front/lower.c`,
   `src/wasm.c` and `src/ir.h` back into the build, with the IR rows, the
   Wasm modules and the differential cases. It needs the S2 commit first.
2. S4 to S8: the policy layer of `docs/PLAN.md` section 12 (`Party`,
   `Committee`, `Rule`, `Policy`, the crypto layer, `Certificate`). Each
   slice needs the slice before it.
3. S9 to S19: the targets T1 to T4, A1 to A6 and the close.

## Known limits

- `langc build` gives `REFUSE_LOWER` "target pending S3". Cause:
  `src/front/lower.c`, `src/wasm.c` and `src/ir.h` are out of the build
  until S3 (Q-S2-1 (a)). Until S3 the gate has no IR rows, no Wasm
  modules and no differential cases (OI-12).
- The affine syntax (`-o`, `@0`, `@1`, `unpack`, `Budget`) is not in the
  language. Four gate rows give a parse or check refusal for it
  (`LEX_CHAR`, `NAME_UNKNOWN`, `TYPE_MISMATCH`), not an affine code.
- `REFUSE_CELL` holds the CM and CC shape check inline in the kernel.
  The R1 slice renames it or adds `REFUSE_FORMER`.
- The kit row "a trap is equal to a trap" stays until S3 (OI-6).

## Internal boundaries

`src/front/lexer.c` and `src/front/parser.c` read text into terms
(`src/front/ast.h`). `src/front/check.c` elaborates terms to core terms
(`src/front/core.h`) and gives the refusal codes. `src/front/eval.c`
evaluates core terms. `src/front/front.c` loads the domain, then the
program. `src/main.c` holds the commands.

`gen/embed.c` copies `domain/domain.lang` into `build/domain.c` at build
time, so the domain is part of `build/langc`.

`test/gate.sh` runs the examples and the rows in `test/parse/`,
`test/check/` and `test/eval/` against their `expect.txt` files.
`test/review.sh` runs the review regressions. `build/fronttool` runs the
domain regressions and `build/pathtool` the path regressions.
`make check` also compiles all sources with clang and `-Werror`.
