# Validation

Date: 2026-11-11. tcc: 0.9.28rc 2026-09-04 mob@0fb54300 (AArch64
Darwin). Host executable: `build/langc` at 22caa70.

`make check` passes: it builds `build/langc`, the C host
`build/ratifyd` and the test tools, and it checks the examples, the
refusals, the evaluator, the IR, the Wasm modules, the no-trap checks,
the differential cases, the crypto rows, the host table rows, the
engine, the demo D1 to D8, the JSON report, the TFHE circuit, the Noir
text, `fmt`, `check --json`, `mcp`, `explain`, the skill and the token
report. It prints 65 count lines with zero failures (`gate: 0
failures`), and the review, domain, path and crypto regressions pass.
34 compile lines use `-Werror`. The final gate run of `make -B check`
took 179 seconds of wall time (`make check` prints no time).

Each paragraph below gives the gate of one slice, from its row in
`docs/PLAN.md` section 12, and the mutants of that slice.
`probe/CAPABILITY.md` gives the probe runs and the cost.

The S0 gate (the seed from tcc-wasm, `DESIGN.md`, the design HTML,
`docs/PLAN.md`, the README table and `AGENTS.md`) exits 0.

S1 has no code change: the USER rulings on section 14 and Q-S1-1 to
Q-S1-3 are in section 1 of `docs/PLAN.md`.

The S2 gate (the Cubical TT kernel, ported from permit-lang 3d47120,
with no affine layer and no R0) passes with 29 examples, 17 parse
refusals, 154 check refusals, 10 kit-only refusals, 1 time hint, 78
evals and 1 build refusal. The review, domain and path regressions
pass. The mutants M1 (path ends), M2 (glue face) and M3 (universe
level) are killed.

The S3 gate (R0 in the kernel and the Wasm writer, ported from
permit-lang 3d47120..18deea2) passes with 30 examples, 17 parse
refusals, 158 check refusals, 10 kit-only refusals, 1 time hint, 79
evals, 1 build, 1 ir lowered, 5 ir refusals, 2 ir lowerings, 4 notrap
modules, 7 notrap bad rows, 10 spine evals, 33 Wasm modules valid and
998 differential cases with 0 mismatches. The mutants M1, M2
(`REFUSE_PARTIAL`), M3 (the div guard of the no-trap tool) and M4
(`flag_norm`) are killed. M5 (the `br_table` default) and M6 (the IF
condition) are equivalent.

The S4 gate (`Party`, `Committee`, `Approvals`, `Counts`, `count`,
`REFUSE_ORBIT` and `REFUSE_BOUND`) passes with 32 examples, 162 check
refusals, 81 evals, 35 Wasm modules valid and the S3 counts of the
other rows. The mutants M1 (the bound compare), M2 (the factor
n_j + 1), M3 (the elim case of `orbit_uses`) and M4 (the approval arm
of `countBits`) are killed.

The S5 gate (`Rule`, the minimal accepting vectors, O3 to O5,
`REFUSE_RULE`, `REFUSE_HONEST` and `REFUSE_FORK`) passes with 33
examples, 168 check refusals, 85 evals, 36 Wasm modules valid and the
S4 counts of the other rows. The mutants M1 to M5 are killed.

The S6 gate (`Policy`, `Decision`, `authorize`, `Amendment`, `Log`,
`System` and `policyAt`, with `REFUSE_FIXPOINT`, `REFUSE_ARROW` and
`REFUSE_CLOCK`) passes with 34 examples, 17 parse refusals, 177 check
refusals, 87 evals, 37 Wasm modules valid and 998 differential cases
with 0 mismatches. The mutants M1 to M5 are killed.

The S7 gate (the crypto layer: `Enc`, `Share`, `Proof`, `Protocol`,
Schnorr on BN254 G1 and `REFUSE_APPROX`) passes with 35 examples, 17
parse refusals, 179 check refusals, 94 evals, 14 crypto rep lines, 38
Wasm modules valid and 998 differential cases with 0 mismatches.
`build/cryptotool` has 97 rows: 23 known-answer, 6 invalid-signature,
43 threshold, 17 reshare and 8 invalid-proof. The mutants M1 to M5
are killed. M1a (the R on-curve check of Schnorr verify) and M4a (the
body of `reshare`) are equivalent.

The S8 gate (`Certificate`, `Verdict`, `Reason`, `verify`, the
equivocation evidence and a faster O5 walk) passes with 37 examples,
17 parse refusals, 180 check refusals, 112 evals, 14 crypto rep lines,
40 Wasm modules valid, 998 differential cases with 0 mismatches and
109 crypto rows. On 18752 cases, the O5 pass gives the same least
total and least overlap as the S5 walk. The mutants M1a, M1b and M2 to
M9 are killed (M1a by the USER row `rBothBad`).

The S9 gate (T1 part 1: the Wasm module and the six imports of module
`ratify`) passes with 37 examples, 17 parse refusals, 180 check
refusals, 112 evals, 14 crypto rep lines, 26 host table rows, 23 host
evals, 5 notrap modules, 11 notrap bad rows, 9 host wasm rows, 40 Wasm
modules valid, 998 differential cases with 0 mismatches and 109 crypto
rows. The mutants M-h1 (the import name), M-h2 (the signature answer)
and M-h3 (the import section) are killed.

The S10 gate (T1 part 2: `ratifyd`, the Wasm engine and the demo D1
to D8) adds 5 demo rows, 1 demo refusal, 9 verdict rows, 4 malformed
rows, 13 bit-order rows and 2 cross rows. The mutants u6 to u10 are
killed. u11 (the length check `>` for `!=`) is equivalent.

The S11 gate (T4, the JSON report) has 7 report rows, 2 report write
errors and 5 json escapes; the 33 count lines of S10 do not change.
The mutants M1, M2a, M2b, M3, M4, M5, M6, M7a and M8 are killed. M7b
(the height committee from `amTo`) is equivalent.

The S12 gate (T2, the TFHE tally circuit) has 7 tfhe rows, 1 tfhe
refusal and 19 tfhe sims; the 36 count lines of S11 do not change.
The mutants M1, M2, M3, M5b, M6, M6s and M8 are killed. A scratch
driver, not the gate, kills M7a and M7b (the caps off by one). M4 (NOT
counted in D) and M5 (no D <= B check) are equivalent.

The S13 gate (T3, R_ballot and R_dec as Noir) checks bytes only: 7
noir rows, 1 noir refusal, 1 noir package and 4 noir kat files; the 39
count lines of S12 do not change. With nargo 1.0.0-rc.0, check, test,
execute and info exit 0 in each package, and 10 + 5 test rows pass.
The Noir tests kill the emitter mutants M1 to M7, 7 of 7.

The S14 gate (A1, `langc fmt`) has 10 fmt rows, 1 fmt refusal,
`fmt idempotent: 267 checked, 17 refused`, `fmt ast: 267 checked` and
`fmt examples: 37 checked`; 49 count lines. `langc fmt` rewrote 34 of
the 37 examples once. The gate kills the printer mutants M1 to M6, 6
of 6.

The S15 gate (A2, `langc check --json`) has `json rows: 21 checked`,
`json fix rows: 11 checked` and `json fix table: 69 codes`; 52 count
lines. With the 7 `.lang` files of `test/json/`, the fmt laws give
`fmt idempotent: 272 checked, 19 refused` and `fmt ast: 272 checked`.
The gate kills the mutants M1 to M7, 7 of 7.

The S16 gate (A4, `langc mcp`) has `mcp rows: 10 passed`,
`mcp negatives: 3 of 3`, `mcp tools: 6 of 6`, `mcp builds: 2 of 2`,
`mcp long line: 1 of 1`, `explain codes: 71 of 71` and
`law rows: 19 of 19`; 59 count lines. The fmt law counts do not
change. The gate kills the mutants M1 to M8, 8 of 8.

The S17 skill gate (`build/skilltool`) checks the 5 graded examples
of `skills/ratify-lang/SKILL.md`: `skill checks: 5 of 5`,
`skill quotes: 5 of 5`, `skill outputs: 21 of 21` and
`skill negatives: 6 of 6`; 63 count lines. On the skill block, the
mutants M1, M2, M3, M4 and M8 are killed. M5, M6 and M7 are live on a
clean tree: section 13 of `docs/PLAN.md` holds them (OI-19).

The S18 token rows of `make check` (`build/tokcount`) read the 15
counted files of `docs/TOKEN-BASELINE.md`: G1 to G5 in ratify-lang
(`examples/skill/`), Python and Rust (`test/baseline/`).
`token report hashes: 15 of 15` and `token estimate: passed`. By the
offline estimate ceil(bytes / 4), the token sum of the 5 Python files
is 0.40 of the ratify-lang sum, and the Rust sum is 0.58. The gate
has 65 count lines. `make tokens` (the count_tokens route) is not
run: the USER runs it outside the sandbox with `ANTHROPIC_API_KEY`
(section 13, OI-18).
