# Host capability probe: tcc-wasm

Date: 2026-11-11. Host tree: `/Users/oobi/Documents/ratify-lang` at
22caa70 (tree 0c0ed010), with the S19 edit of `SPEC.md` not
committed (u2-probe.log:3-4). The probe read the tree and did not
change it. Driver: `build/langc` (file time 2026-11-11 19:11,
u2-probe.log:9). `make -q` exits 0 for `build/langc`,
`build/ratifyd`, `build/demotool`, `build/cryptotool`,
`build/jsontool` and `build/enginetool`, thus the binaries agree with
their sources (u2-probe.log:10). The tools are tcc 0.9.28rc
2026-09-04 mob@0fb54300 (AArch64 Darwin), node v23.10.0,
wasm-validate 1.0.39 and wasm-objdump 1.0.39 (u2-probe.log:5-8), the
versions of `PIN` lines 1, 3, 4 and 5. Other work ran on the machine
(load averages at u2-probe.log:2, :366). Thus each wall time is an
upper bound. A cite `u2-probe.log:N` is a line of the run log
`ratify-lang-work/s19/u2-probe.log`, which is outside the repo.

The probe ran `zsh -f u2-probe.sh` in `ratify-lang-work/s19/` (run 3:
76 steps, EXIT 0). Each step is one `step` line of the script. To run
some steps, keep the setup lines and those `step` lines. The script
writes the generated inputs below `u2/in/`, the outputs below
`u2/out/` and the log `u2-probe.log`. Its output is the summary
`u2-probe.sum`, one line for each step and an `EXIT` line at the end.
Each step runs below `probe/guard.py --rss-mb 4096 --timeout 120`,
which stops a command above 4096 MB resident memory or above 120 s.
The guard prints one JSON line for each step, with the wall time and
two resident peaks (u2-probe.log:14). The largest process in the
probe used 172.8 MB (u2-probe.log:268, the check of
`examples/circle.lang`). The guard rows at the end of this file give
each step. In run 1, the two `--json` steps put the flag before PROG,
and each gave USAGE `too many arguments` and exit 2. Runs 2 and 3 put
the flag after PROG. Run 3 is run 2 with the summary kept in a file.
The script, the generated inputs and the logs are local artifacts,
not files of this repository. They are provenance for this report,
not a repository-local replay command.

To recheck the repository fixtures from the repository root, run:

```sh
make check
python3 probe/guard.py --rss-mb 4096 --timeout 120 -- build/langc --kit check test/demo/demo.lang
python3 probe/guard.py --rss-mb 4096 --timeout 120 -- build/langc --kit check examples/sigma.lang
python3 probe/guard.py --rss-mb 4096 --timeout 120 -- build/langc --kit check test/check/self.lang
python3 probe/guard.py --rss-mb 4096 --timeout 120 -- build/langc --kit check test/check/rec.lang
python3 probe/guard.py --rss-mb 4096 --timeout 120 -- build/langc --kit check test/check/axiom.lang
```

`make check` runs the gate (`Makefile:147`, `test/gate.sh`). The
first two guard rows print `ok` and exit 0 (u2-probe.log:33, :49).
The `self.lang` and `rec.lang` rows exit 1 with REFUSE_FIXPOINT
(u2-probe.log:53, :61). The last row exits 1 with REFUSE_AXIOM
(u2-probe.log:280). These commands cover the checked-in fixtures, not
the generated probe inputs or the cost measurements in section 6.

The base span: `langc check` with no `--kit` refuses a program with a
kit item. For `test/demo/demo.lang` the line starts REFUSE_KIT
`flagDec: Flag is a kit item outside the base span`
(u2-probe.log:69), and for `test/check/self.lang` it starts `spin:
Nat is a kit item outside the base span` (u2-probe.log:65). `langc
explain REFUSE_KIT` gives the text of the code (u2-probe.log:115).
The gate runs a program with `--kit` when `examples/kit.txt` or
`test/check/kit.txt` lists it, and so do the kit steps of the probe.
The gate builds the demo with `--kit` (`test/gate.sh:235`, :242).
With `--kit`, `test/demo/demo.lang` checks `ok` (u2-probe.log:33),
and so do the generated cost files (u2-probe.log:311, :329). `langc
list-base` prints the names of the base span, first `Unit builtin -`
(u2-probe.log:131).

## Questions

Each host must answer these questions. Each answer is one numbered
section below. An answer sets the status of a former in
`formers/tcc-wasm.md`.

| Question | Section | Formers |
|---|---|---|
| Which commands check a file, print a normal form, build and run? | 1 | all |
| Does the host take several files, or must the driver join them? | 2 | all |
| Which recursion does the host accept, and how does it refuse the rest? | 3 | F6, F7 |
| Which `Nat` primitives exist? Does `Nat` have an eliminator? | 4 | F6, F7 |
| How does a result leave the host? | 5 | target |
| What do check and build cost (time and resident memory) at N = 100 and N = 1000? | 6 | all |
| Can a family take a parameter? Can a constructor of such a family appear in a term? | 7 | F4, F12, F15 |
| Can a program project a Sigma? Does the kernel accept Sigma eta? | 8 | F11 |
| How does a program use equality: transport, cong, an index type? | 9 | F12, F13 |
| How does the host report an axiom? | 10 | R2 |

## 1. Commands

| Job | Command | Result |
| --- | --- | --- |
| Check | `build/langc [--kit] check PROG [--json]` | `ok` and exit 0 for `test/demo/demo.lang` with `--kit` (u2-probe.log:33), or one line `langc: CODE: NAME: TEXT` and exit 1 (u2-probe.log:53). `check PROG --json` prints one JSON object (section 10) |
| Print normal form | `build/langc [--kit] eval PROG NAME [ARGS...]` | The normal form of NAME. For `pBase` in `test/demo/demo.lang`: a term that starts `plOf (cmClass (szS (szS (szS sz0))) (cmClass (szS sz0) cmNil))` (u2-probe.log:37). Exit 0 |
| Build | `build/langc [--kit] build PROG -o OUT [--runtime]` | One wasm module. For `test/demo/demo.lang`: 5281 bytes, and `wasm-validate` accepts it (u2-probe.log:83, :86, :348). With `--runtime` the module is also 5281 bytes (u2-probe.log:89, :349) |
| Run | `build/demotool MODULE PROG IMAGE` | `demotool` runs the built demo module on one image and prints one verdict. For `dimg/D1.txt`: `8191 valid` and exit 0, the same as line 1 of `test/demo/want.txt` (u2-probe.log:303, :361-362) |

- `langc` with no command gives USAGE `expected a command and a
  program path` and exit 2 (u2-probe.log:15). It prints the usage of
  13 commands: `check [--json]`, `eval`, `ir`, `abi [--text]`, `build`
  (with `-o` and `--runtime`), `report`, `tfhe`, `noir`, `fmt
  [--check]`, `explain CODE`, `list-base`, `show-law [ROW]` and `mcp`,
  and one line for `--kit` (u2-probe.log:15-31; `src/main.c:83`).
- One file for each command: `check` with two files gives USAGE `too
  many arguments` and exit 2 (u2-probe.log:146, section 2).
- `ir` prints the lowered function, `func demo (nat, nat, ...`
  (u2-probe.log:73). `abi` prints one entry, `demo`
  (u2-probe.log:79).
- `report` prints a JSON report of 1047 bytes. Its first line is
  `{"kind":"program","logs":1,"policies":3,"ratify":1,"rules":0}`
  (u2-probe.log:92, :352). `tfhe` prints one JSON object of 75 bytes,
  with `"circuits":0` for the demo (u2-probe.log:98, :353). `noir`
  prints Noir source of 1641 bytes, which starts `// The T3 relations
  of ratify-lang as Noir` (u2-probe.log:102, :354).
- `fmt PROG --check` prints nothing and exits 0 for
  `examples/sigma.lang` (u2-probe.log:108). For `test/demo/demo.lang`
  it gives FMT_DIFF and exit 1 (u2-probe.log:111): the demo file is
  not in the canonical form of `fmt`.
- `explain CODE` prints the fix text of a code (u2-probe.log:115,
  :123, :127). A name that is not a code of the fix table gives
  EXPLAIN and exit 1 (u2-probe.log:119). `show-law TCM-A` prints
  `TCM-A: authorize is invariant under G_P (DISCHARGED, trigger R)`
  (u2-probe.log:137). `list-base` prints the base span
  (u2-probe.log:131).
- `mcp` reads JSON-RPC lines on its input and exits 0 at the end of
  the input. An `initialize` with empty params gives error -32602
  `Invalid params` (u2-probe.log:141). `tools/list` gives 6 tools:
  `check`, `fmt`, `explain`, `list-base`, `show-law` and `build`
  (u2-probe.log:144).
- `ratifyd` with no arguments prints `usage: ratifyd --call IMAGE FILE
  NAME [ARGS...]` and exits 2 (u2-probe.log:307;
  `host/ratifyd.c:217`). `demotool MODULE PROG IMAGE` is the run step
  of the gate for the demo module (`test/gate.sh:272`).
- The host tools: `build/cryptotool` with no argument gives `crypto
  known-answer: 23 of 23 rows passed` in 4.344 s (u2-probe.log:284),
  and `build/jsontool` gives `json escapes: 5 checked`
  (u2-probe.log:290). `cryptotool demo DIR` writes the 8 demo images
  (u2-probe.log:294, :365), and `cryptotool image` prints 4893 bytes
  (u2-probe.log:297, :355).
- `PIN` gives the tool versions of the last green host gate, and the
  probe tools are the same (u2-probe.log:5-8). `PIN` line 7 names
  `make -C hosts/tcc-wasm check`. This repo has no `hosts/`
  directory, thus the probe has no step for it.
- Not probed: a run of a built module in node, the `ARGS` of `eval`,
  the tool calls of `mcp`, a `ratifyd --call` (14 `Nat` arguments by
  hand), and `make tokens` (`build/tokcount`, `Makefile:179`).

## 2. Several files

No. A program is one file, with no import form. A file that starts
with `import domain` gives PARSE_EXPECT and exit 1 (u2-probe.log:164).
`langc check` with two files gives USAGE `too many arguments` and exit
2 (u2-probe.log:146). A program uses the domain names with no import:
`def d0 : Size := sz0` checks `ok` (u2-probe.log:168). Thus a driver
must join the parts of a program into one file. The cost rows do
this: the probe joins `test/demo/demo.lang` and a `Size` chain into
one file, and the check and the build of that file exit 0
(u2-probe.log:311, :315, :329, :333; section 6).

## 3. Recursion and totality

No recursive definition checks. `def rec` (`test/check/rec.lang`)
gives REFUSE_FIXPOINT `loop: def rec is refused (rule R3)` and exit 1
(u2-probe.log:61). With `--kit`, a call of the definition itself
(`test/check/self.lang`) or of a later name
(`test/check/later.lang`) gives REFUSE_FIXPOINT and exit 1. The lines
start `spin: spin is this definition` (u2-probe.log:53) and `early:
late is this definition` (u2-probe.log:57). In the base span
`test/check/self.lang` stops first at REFUSE_KIT (u2-probe.log:65;
the base-span paragraph above). `langc explain REFUSE_REC` gives
EXPLAIN `'REFUSE_REC' is not a code of the fix table` and exit 1
(u2-probe.log:119). Recursion is `fold` and `unfold`. F6 (algebra
fold) is DONE over Nat, List and the domain families, with a
HOST-LIMIT note, and F7 (algebra unfold) is DONE into List with a Nat
step limit (`formers/tcc-wasm.md:19-20`). The probe ran no `fold` or
`unfold` of its own.

## 4. Nat

The primitives are `natAdd`, `natSub`, `natDiv` and `natMod`, with
the comparisons `natEq` and `natLe` (`src/front/check.c:185-188`,
:194-195). `natAdd` gives an `Option Nat` (:185): `natAdd 2 3` is
`5`, and `natAdd 18446744073709551615 1` is `none` (u2-probe.log:180,
:184). `natSub` gives 0 when the second argument is the larger
(u2-probe.log:172). The divisor of `natDiv` and `natMod` is a
`NonZero Nat`: `natDiv 7 2` is 3 (u2-probe.log:188), and the literal
0 gives REFUSE_PARTIAL `divz: the literal 0 is not a value of NonZero
Nat` and exit 1 (u2-probe.log:192), and the same for `natMod`
(u2-probe.log:196). `natMul` and `natRec` give NAME_UNKNOWN and exit
1 (u2-probe.log:200, :204): `Nat` has no eliminator primitive, and
the eliminator is the algebra `fold` (F6, section 3). The base span
refuses `Nat` (u2-probe.log:65), thus the steps above run with
`--kit`. The largest value is 18446744073709551615, that is 2^64 - 1
(u2-probe.log:176). A literal above it gives LEX_NAT_RANGE and exit 1
(u2-probe.log:208). No step of the probe trapped. A definition cannot
recurse on a `Nat` (section 3). The base span counts with the domain
family `Size` (`domain/domain.lang:48`).

## 5. How a result leaves the host

As printed text, as one wasm module, or as the verdict of `demotool`.
`eval` prints the normal form (u2-probe.log:37). `ir` and `abi` print
the lowered function and the interface (u2-probe.log:73, :79).
`build` writes one wasm module with `-o` (u2-probe.log:83, :348), and
`wasm-validate` accepts each module of the probe (u2-probe.log:86,
:318, :336). `report` prints the JSON report (u2-probe.log:92),
`tfhe` prints a JSON object with the count of FHE circuits
(u2-probe.log:98), and `noir` prints the T3 relations as Noir source
(u2-probe.log:102). `check PROG --json` prints the diagnostics as one
JSON object (u2-probe.log:272). `demotool` runs a built module on one
image file and prints one verdict line (u2-probe.log:303).

## 6. Cost

Each file is `test/demo/demo.lang` with N + 1 `Size` definitions `k0`
to `kN` at the end. `demo.lang` has no `def start`, and no definition
uses the chain. Check runs `langc --kit check` on the file. Build
runs `langc --kit build FILE -o OUT`. Run is `demotool` on the
module, the file and the image `dimg/D1.txt`, with one verdict. Each
cell gives the wall time and the peak resident memory, the larger of
the two `probe/guard.py` peaks (u2-probe.log:311, :315, :321, :329,
:333, :339).

| Step | N = 100 | N = 1000 |
| --- | --- | --- |
| Check (`langc --kit check FILE`) | 0.024 s, 14.7 MB | 0.051 s, 16.9 MB |
| Build (`langc --kit build FILE -o OUT`) | 0.049 s, 15.0 MB | 0.052 s, 18.5 MB |
| Run (`demotool MODULE FILE D1.txt`, 1 verdict) | 0.149 s, 14.2 MB | 0.198 s, 17.5 MB |

- Each run prints `8191 valid`, the same as line 1 of
  `test/demo/want.txt` (u2-probe.log:321, :339, :361, :363-364).
- The modules for N = 100 and N = 1000 are 5281 bytes, the size of
  the module of `test/demo/demo.lang` alone (u2-probe.log:348-351).
  No definition uses the chain, thus the build adds no bytes for it.
- The cost files are 11030 and 37131 bytes (u2-probe.log:358-359).
- A chain of N `Size` definitions alone checks in 0.027 s at 11.5 MB
  for N = 100 and in 0.053 s at 16.3 MB for N = 1000
  (u2-probe.log:325, :343).

## 7. Parametric families

A program cannot declare a family: a program with a family `Box`
(u2-probe.sh:31) gives REFUSE_DATA `Box: a program cannot declare a
family` (rule R1) and exit 1, with `--kit` (u2-probe.log:212) and in
the base span (u2-probe.log:216). In `domain/domain.lang`, `Trunc1`
(:38) and `Trunc0` (:42) take a type parameter. `Pos` (:140) and
`Word` (:146) take a `Size` index, with the type `Size -> Type 0`.
The constructor `trunc1` takes its type as an explicit argument: with
the type left out it gives TYPE_ARITY `trunc1 needs 2 arguments,
found 1` (u2-probe.log:220), and with the type it checks `ok`
(u2-probe.log:224). A constructor of an index family appears in a
term: `def p : Pos (szS sz0) := posFirst sz0` (u2-probe.sh:34) checks
`ok` (u2-probe.log:228). `examples/quot.lang` and
`examples/trunc.lang` check `ok` with `--kit` (u2-probe.log:232,
:236). F15 stays PARTIAL: only the domain file declares an indexed
family (`formers/tcc-wasm.md:28`).

## 8. Sigma projection and eta

The projections check: the projection step (u2-probe.sh:35, the
projections of a `pack`) checks `ok` (u2-probe.log:240), and
`examples/sigma.lang` checks `ok` with `--kit` (u2-probe.log:49). A
Sigma pattern does not parse: a `fun` with the pattern `(n, p)`
(u2-probe.sh:36) gives PARSE_EXPECT and exit 1 (u2-probe.log:244).
The kernel accepts Sigma eta: the eta step (u2-probe.sh:37) proves
the path from `pack (witness s) (payload s)` to `s` and checks `ok`
(u2-probe.log:248). F11 (Sigma) is DONE (`formers/tcc-wasm.md:24`).

## 9. Equality

A program writes equality as `Path A x y`. F12 and F13 are DONE
(`formers/tcc-wasm.md:25-26`): `refl` and `sym` are builtins, and
`transport` is a guest definition by `transp`. `transport`, `J` and
`trans` are definitions in `domain/domain.lang:9`, :14 and :22. A
program that uses `transport` checks `ok` (u2-probe.sh:38,
u2-probe.log:252), and so does a program that uses `trans`
(u2-probe.sh:39, u2-probe.log:256). `S1` (`domain/domain.lang:34`)
is a higher inductive family with `loop (i : I)`.
`examples/paths.lang`, `examples/equality.lang` and
`examples/circle.lang` check `ok` with `--kit` (u2-probe.log:260,
:264, :268). The circle check takes 0.185 s at 172.8 MB
(u2-probe.log:268). `trunc1` takes its type as an argument (section
7), thus one family serves each type.

## 10. Axiom detection

No command lists the axioms of a file (the usage,
u2-probe.log:15-31). `langc check FILE` prints only `ok` for a file
with no axiom (u2-probe.log:168). `langc check FILE --json` prints
one JSON object. For a one-definition probe input with no axiom, it
prints `"diagnostics":[]` and a `progress` object, and exits 0
(u2-probe.log:272). For `test/check/axiom.lang` with `--kit`, it
prints one diagnostic with the code REFUSE_AXIOM and exits 1
(u2-probe.log:276). A `check` of that file with no `--json` gives
REFUSE_AXIOM `magic: an axiom has no proof (rule R2)` and exit 1
(u2-probe.log:280). `langc explain REFUSE_AXIOM` prints the text of
the code (u2-probe.log:123). Thus a program with an axiom does not
check, and R2 holds at the check. The flag `--json` comes after PROG:
`check --json PROG` gives USAGE `too many arguments` and exit 2 (run
1).

## Consequences for M0

1. A program is one file. The domain names need no import (section
   2).
2. No `def rec`, and no call of the definition itself or of a later
   one: REFUSE_FIXPOINT. Recursion is `fold` and `unfold` (section 3,
   R3).
3. A program with a kit item (`Nat`, `Flag`) runs with `langc --kit`,
   as the gate does for the demo. The base span refuses it with
   REFUSE_KIT (the base-span paragraph, section 4).
4. A `Nat` is at most 2^64 - 1. `natAdd` gives `none` above that
   value, `natSub` stops at 0, and a literal 0 divisor does not check.
   Thus a driver reads the `Option` of `natAdd`, not a trap (section
   4).
5. One build writes one wasm module, which exports `demo`. `report`,
   `tfhe` and `noir` print the other outputs: the JSON report, the
   count of FHE circuits and the T3 relations as Noir source
   (sections 1 and 5).
6. A program cannot declare a family, thus F15 stays PARTIAL (section
   7).
7. A check or a build at N = 1000 costs below 0.06 s and 19 MB. A
   chain that no definition uses adds no bytes to the module (section
   6).
8. An axiom stops the check with REFUSE_AXIOM and exit 1 (section 10,
   R2).
9. `demotool` is the run step of the demo module: one image in, one
   verdict out. `ratifyd --call` takes the arguments of a name by
   hand, and the probe did not measure it (sections 1 and 6).

## Guard rows

One row for each step of run 3, from `u2-probe.sum`. The log line is
the `## NAME: CMD` line of the step. MB is the larger of the two guard
peaks.

| Log line | Step | rc | s | MB |
| --- | --- | --- | --- | --- |
| 11 | `make` | 0 | 0.159 | 6.0 |
| 15 | `usage` | 2 | 0.028 | 1.5 |
| 33 | `check-deploy` | 0 | 0.062 | 14.3 |
| 37 | `eval-deploy` | 0 | 0.025 | 14.5 |
| 41 | `check-cert` | 0 | 0.026 | 13.3 |
| 45 | `check-policy` | 0 | 0.028 | 13.0 |
| 49 | `check-sigma` | 0 | 0.028 | 11.0 |
| 53 | `check-self` | 1 | 0.028 | 11.0 |
| 57 | `check-later` | 1 | 0.026 | 11.0 |
| 61 | `check-rec` | 1 | 0.028 | 10.9 |
| 65 | `self-base` | 1 | 0.027 | 12.4 |
| 69 | `natT-base` | 1 | 0.027 | 13.4 |
| 73 | `ir-deploy` | 0 | 0.026 | 14.5 |
| 79 | `abi-deploy` | 0 | 0.027 | 14.5 |
| 83 | `build-deploy` | 0 | 0.028 | 14.5 |
| 86 | `valid-deploy` | 0 | 0.029 | 2.4 |
| 89 | `build-deploy-rt` | 0 | 0.028 | 14.3 |
| 92 | `report-deploy` | 0 | 0.027 | 14.4 |
| 98 | `tfhe-deploy` | 0 | 0.025 | 14.4 |
| 102 | `noir-deploy` | 0 | 0.026 | 14.3 |
| 108 | `fmt-sigma` | 0 | 0.024 | 1.7 |
| 111 | `fmt-deploy` | 1 | 0.026 | 2.1 |
| 115 | `expl-kit` | 0 | 0.027 | 1.4 |
| 119 | `expl-rec` | 1 | 0.028 | 1.4 |
| 123 | `expl-axiom` | 0 | 0.027 | 1.4 |
| 127 | `expl-former` | 0 | 0.027 | 1.4 |
| 131 | `base` | 0 | 0.028 | 2.8 |
| 137 | `law1` | 0 | 0.028 | 1.7 |
| 141 | `mcp` | 0 | 0.033 | 1.7 |
| 146 | `two-files` | 2 | 0.029 | 1.5 |
| 164 | `import` | 1 | 0.028 | 4.2 |
| 168 | `one` | 0 | 0.025 | 12.3 |
| 172 | `nat-sub` | 0 | 0.024 | 12.3 |
| 176 | `nat-big` | 0 | 0.027 | 12.3 |
| 180 | `nat-sum` | 0 | 0.049 | 12.3 |
| 184 | `nat-over` | 0 | 0.028 | 12.2 |
| 188 | `nat-div` | 0 | 0.028 | 12.3 |
| 192 | `nat-divz` | 1 | 0.03 | 11.2 |
| 196 | `nat-modz` | 1 | 0.027 | 12.3 |
| 200 | `nat-mul` | 1 | 0.028 | 12.2 |
| 204 | `nat-rec` | 1 | 0.03 | 12.4 |
| 208 | `nat-range` | 1 | 0.028 | 4.0 |
| 212 | `data` | 1 | 0.023 | 12.2 |
| 216 | `box` | 1 | 0.027 | 12.4 |
| 220 | `trunc-noarg` | 1 | 0.028 | 12.2 |
| 224 | `trunc-arg` | 0 | 0.025 | 12.3 |
| 228 | `pos` | 0 | 0.028 | 12.2 |
| 232 | `quot` | 0 | 0.028 | 13.7 |
| 236 | `trunc-ex` | 0 | 0.027 | 21.9 |
| 240 | `proj` | 0 | 0.027 | 12.3 |
| 244 | `pat` | 1 | 0.023 | 4.2 |
| 248 | `eta` | 0 | 0.028 | 12.4 |
| 252 | `transport` | 0 | 0.027 | 12.3 |
| 256 | `trans` | 0 | 0.024 | 12.3 |
| 260 | `paths` | 0 | 0.027 | 11.8 |
| 264 | `equality` | 0 | 0.024 | 11.2 |
| 268 | `circle` | 0 | 0.185 | 172.8 |
| 272 | `json-one` | 0 | 0.025 | 12.4 |
| 276 | `json-axiom` | 1 | 0.028 | 11.2 |
| 280 | `check-axiom` | 1 | 0.027 | 12.2 |
| 284 | `crypto` | 0 | 4.344 | 1.6 |
| 290 | `json` | 0 | 0.027 | 1.8 |
| 294 | `dimg` | 0 | 0.821 | 1.9 |
| 297 | `image` | 0 | 0.455 | 1.6 |
| 303 | `run-demo` | 0 | 0.171 | 13.7 |
| 307 | `ratifyd-usage` | 2 | 0.026 | 1.5 |
| 311 | `check100` | 0 | 0.024 | 14.7 |
| 315 | `build100` | 0 | 0.049 | 15.0 |
| 318 | `valid100` | 0 | 0.027 | 2.3 |
| 321 | `run100` | 0 | 0.149 | 14.2 |
| 325 | `chain100` | 0 | 0.027 | 11.5 |
| 329 | `check1000` | 0 | 0.051 | 16.9 |
| 333 | `build1000` | 0 | 0.052 | 18.5 |
| 336 | `valid1000` | 0 | 0.028 | 2.3 |
| 339 | `run1000` | 0 | 0.198 | 17.5 |
| 343 | `chain1000` | 0 | 0.053 | 16.3 |
