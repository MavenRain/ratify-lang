# AGENTS.md

Instructions for agents that work in this repository.

## Routing rule

Use ratify-lang when the task is in a "yes" row of the README table
"When to reach for ratify-lang": a threshold authority that amends
itself. A group approves an action by a rule that reads only the count
of approvals in each class of a committee. The checker proves the policy
facts before any vote: each decision is one bit (O2), each accepting set
holds an honest party (O4), and two amendments cannot both pass at one
height (O5). The old amend rule judges each new policy, one step at a
time. The yes rows are a plain t-of-n policy (G1), two classes and a
veto (G2), an amendment that changes the amend rule (G3), the replay of
a log of amendments (G4) and the demo of the verifier host (G5). No
program reads a clock: the time is the height of the log, and a wall
clock gives `REFUSE_CLOCK`. Do not use it when one party decides or a
plain access list is enough, when the policy never changes, when a rule
must treat named parties in one class differently (`REFUSE_ORBIT`), for
a ranked choice over 3 or more options (`REFUSE_ARROW`), for approximate
arithmetic (`REFUSE_APPROX`) or for general-purpose code. The skill
`skills/ratify-lang/SKILL.md` gives the routing rule, the loop and 5
graded examples (G1 to G5, the files of `examples/skill/`).

## The loop

1. Write FILE.lang: one `def NAME : TYPE := TERM` for each value. For
   the names of the base, run `build/langc list-base`. A def of a `Path`
   type with the term `refl` is a law: the checker proves it. A kit item
   (`Nat`, its operations and the host imports such as `sig_verify`)
   needs the kit mode, `build/langc --kit`. G1 to G4 are guest files. G5
   is a kit file (`examples/skill/kit.txt`).
2. Run `build/langc fmt FILE.lang --check`. No output and exit 0: the
   text is canonical. Exit 1 with `FMT_DIFF`: the text is not canonical.
   Then write the output of `build/langc fmt FILE.lang` to a new file,
   and replace FILE.lang with it. fmt does not check the program
   (`docs/FMT.md`).
3. Run `build/langc check FILE.lang --json`
   (`build/langc --kit check FILE.lang --json` for a kit file). The flag
   `--json` comes after the file. The command writes one JSON line on
   stdout, with the `diagnostics` array and the `progress` object
   (`docs/CHECK-JSON.md`). Exit 0: the checker accepts the file, and the
   array is empty. Exit 1: the checker refuses the file, and the array
   has the first failure. Exit 2: langc cannot read the file (`IO`), or
   the arguments are wrong (`USAGE`).
4. Read the `code`, the `message`, the `fix` and the `span` (the file,
   the line and the column) of the diagnostic.
   `build/langc explain CODE` gives one line: the code and its fix text.
5. Repair: change the file, then do the steps 2 and 3 again. When the
   checker accepts the file, `build/langc eval FILE.lang NAME`
   (`build/langc --kit eval FILE.lang NAME` for a kit file) gives the
   normal form of the def NAME, in one line.

The command `build/langc mcp` gives the verbs to an agent as an MCP
server on stdin and stdout (`docs/MCP.md`). Its 6 tools are `check`,
`fmt`, `explain`, `list-base`, `show-law` and `build`. There is no
`eval` tool and no `fmt --check` tool: use the command line for those.
With `--kit`, each tool runs in the kit mode.

## Status (S19)

The slices S0 to S16 give the Cubical TT kernel with R0 (S2 and S3), the
domain words (S4 to S8: the committee, the rule, the policy, the crypto
layer and the certificate), the four targets (the Wasm module and the
verifier host `ratifyd`, T1, S9 and S10; the TFHE tally circuit, T2,
S12; the Noir relations, T3, S13; the JSON report, T4, S11), `langc fmt`
(S14), `langc check --json` (S15), and `langc mcp`, `langc explain`,
`langc list-base` and `langc show-law` (S16). S17 adds
`skills/ratify-lang/SKILL.md`, the graded examples of `examples/skill/`,
`test/skilltool.c`, the fixtures of `test/skill/`, this file and the
README table. S18 adds the token comparison with Python and Rust (A6):
`docs/TOKEN-BASELINE.md`, `test/tokcount.c` and `test/baseline/`. S19
closes the plan: `SPEC.md`, `probe/CAPABILITY.md`, `docs/VALIDATION.md`
and the final gate. `docs/PLAN.md` section 12 gives each slice. The plan
is CLOSED, and section 13 gives the open items.

## Commands that work now

- `make -C /path/to/ratify-lang check`: build `build/langc` and the test
  tools (for example `build/skilltool`), then run the gate
  `test/gate.sh`. The gate prints count lines (for example
  `skill checks: 5 of 5`) and `gate: 0 failures` at its end. A failure
  prints a `FAIL` line. The gate must exit 0. For a log that you keep,
  run it serial (no `-j`) and append `EXIT $?` to the log.
- `build/langc fmt PROG [--check]`: the canonical text of PROG on stdout
  (`docs/FMT.md`). With `--check`: no output, and exit 0 only when PROG
  is canonical.
- `build/langc [--kit] check PROG [--json]`: check PROG. The text mode
  gives `ok` on stdout, or one diagnostic line on stderr. With `--json`:
  one JSON line on stdout (`docs/CHECK-JSON.md`). The flag comes after
  PROG.
- `build/langc [--kit] mcp`: the MCP server (`docs/MCP.md`). Its 6 tools
  are `check`, `fmt`, `explain`, `list-base`, `show-law` and `build`.
- `build/langc explain CODE`: one line `CODE: FIX` with the fix text of
  the diagnostic code CODE. An unknown code gives exit 1.
- `build/langc [--kit] list-base`: one line for each name of the base:
  the name, its kind and its place.
- `build/langc show-law [ROW]`: one line for each law row, in the form
  `ROW: STATEMENT (STATUS, trigger T)`. With ROW (for example `O5`):
  that row only.
- `build/langc [--kit] eval PROG NAME [ARGS...]`: the normal form of the
  def NAME, in one line.
- `build/langc [--kit] build PROG -o OUT [--runtime]`: the Wasm module
  of PROG, the target T1 (`docs/host/README.md`).
- `build/langc [--kit] report PROG`: the JSON report, the target T4
  (`docs/REPORT-JSON.md`).
- `build/langc [--kit] tfhe PROG`: the TFHE tally circuit, the target T2
  (`docs/TFHE-CIRCUIT.md`).
- `build/langc [--kit] noir PROG`: the relations R_ballot and R_dec as
  Noir, the target T3 (`docs/NOIR-RELATIONS.md`).

With no argument, `build/langc` writes a USAGE line and the usage text
on stderr, and exits with code 2. The usage text has all the verbs.

## Rules for a change

- R0: no program can trap. A rejection is a value (`false`, or
  `invalid Reason`). Do not add a partial operation.
- R1: a type constructor is a cell monad or its dual. Any other former is
  refused.
- The core types and operations come only from the design. Do not add one
  that `docs/PLAN.md` section 4 does not list.
- Time is the log height. Do not add a clock.
- `langc mcp` (the authoring server) and `ratifyd` (the verifier host) are
  two programs. Do not merge them.
- No em-dash or en-dash in any file. The gate fails on one.
- Sign each commit (`git commit -s`).
- Each program block of SKILL.md is byte equal to its file in
  `examples/skill/`, and each output block is the output of its command.
  The gate checks both (`skill quotes`, `skill outputs`). Change the
  file and the block together. `examples/skill/g5-demo.lang` is the
  `langc fmt` text of `test/demo/demo.lang`: change those two files
  together.

## Files

- `design/`: the request and the design, verbatim. Do not edit
  `design/self-amending-threshold-authority.html`.
- `docs/PLAN.md`: the plan. Section 1 holds the rulings. Section 12
  gives the slices. Section 14 holds the open questions.
- `skills/ratify-lang/SKILL.md`: the skill: the routing rule, the loop,
  the verbs and the graded examples G1 to G5.
- `examples/skill/`: the 5 example files of the skill, and `kit.txt`,
  the list of the kit files.
- `src/`: `langc` (parser, checker, evaluator, lowering, Wasm writer).
- `domain/`: the domain files of the kit. `domain/domain.lang` has the
  defs of the base.
- `host/`: the Wasm engine of `build/ratifyd`.
- `examples/`, `test/`: the gate rows. `test/gate.sh` is the gate.
  `test/skill/` has the 6 negative fixtures of `build/skilltool`
  (`test/skilltool.c`).
- `test/baseline/`, `test/tokcount.c` and `docs/TOKEN-BASELINE.md`: the
  token comparison (A6). `test/baseline/` has G1 to G5 in Python and in
  Rust. `make check` runs the offline check of `build/tokcount`;
  `make tokens` runs the count_tokens route outside the sandbox, with
  `ANTHROPIC_API_KEY`.
- `noir/`: the Nargo packages of the Noir relations.
- `formers/`, `probe/`: the type formers and the host probe.
- `docs/`: `STATUS.md`, `FMT.md`, `CHECK-JSON.md`, `MCP.md`,
  `REPORT-JSON.md`, `TFHE-CIRCUIT.md`, `NOIR-RELATIONS.md` and
  `host/README.md`.
