# TFHE tally circuit (T2)

This file gives the format of the T2 target (PLAN section 8.2, S12,
Q-S12-1 to Q-S12-8 (a)). `langc [--kit] tfhe PROG` checks PROG and
writes JSON Lines to stdout: one TFHE Boolean circuit for each decided
rule definition of PROG. The circuit computes accept . count on one
encrypted bit for each member. ratify-lang does not run TFHE: the lines
give the gates, and `build/tfhetool` runs them in the clear (no TFHE
library, Q-S12-6 (a)).

## Rules for each line

- A line is one JSON object in its RFC 8785 (JCS) form, then `\n`, as in
  T4 (`docs/REPORT-JSON.md`). The writer (`src/front/report.c`,
  `src/front/tfhe.c` and `src/jsonout.c`) makes the JSON text, then the
  parse round trip makes the bytes (Q-S12-5 (a)).
- Each line has `"ratify":1` and `"kind"`: `"program"` or `"circuit"`.
- Each number is a JSON integer below 2^53.
- A line has no time field (PLAN section 5).

## Line order

1. One `program` line.
2. One `circuit` line for each decided rule definition, in definition
   order. These are the definitions of the T4 `rule` lines: the type
   `Rule`, `Honest` or `ForkFree` at a closed committee and a closed f.
   An `Honest` or `ForkFree` line has the circuit of its rule.

`langc tfhe` builds each circuit before it writes the first line. Thus a
refused program writes no line.

## The program line

| Key | Value |
|---|---|
| `circuits` | the count of `circuit` lines |
| `gate_max` | `65536` (`TFHE_GATE_MAX`) |
| `kind` | `"program"` |
| `min_cap` | `4096` (`TFHE_MIN_CAP`) |
| `ratify` | `1` |

## The circuit line

| Key | Value |
|---|---|
| `bound` | B, the static depth bound (below) |
| `classes` | the class sizes n_j, in class order |
| `depth` | D, the longest bootstrap chain from an input to `out` |
| `gate_count` | the count of rows in `gates` |
| `gates` | the gate rows `[op, a, b]`, in wire order |
| `inputs` | I, the sum of the class sizes |
| `kind` | `"circuit"` |
| `min` | each minimal accepting vector, in index order |
| `min_count` | the count of minimal accepting vectors |
| `name` | the definition name |
| `nots` | the count of `NOT` rows |
| `out` | the accept wire |
| `ratify` | `1` |
| `type` | `"Rule"`, `"Honest"` or `"ForkFree"` |

Wires (Q-S12-1 (a)). The input wires are 0 to I - 1, one for each
member, in class order and then member order. Gate row g writes wire
I + g and reads only lower wires. `op` is one of the strings `"AND"`,
`"OR"`, `"XOR"`, `"NAND"`, `"NOR"`, `"XNOR"` and `"NOT"`; a `NOT` row has
b = a. `out` is a wire below I + `gate_count`.

The `min` list (Q-S12-2 (a)). The circuit line lists each minimal vector,
at most `TFHE_MIN_CAP` 4096. The T4 `rule` line lists the first 64
(Q-S11-4 (a)). Thus for `wide8` the circuit line has 70 vectors and the
T4 line has 64.

## Gate set (Q-S12-3 (a))

`AND`, `OR`, `XOR`, `NAND`, `NOR` and `XNOR` read two wires and cost one
bootstrap each. `NOT` reads one wire and costs no bootstrap. The builder
folds constants, so no row reads a constant. The S12 builder writes only
`XOR`, `AND` and `OR` rows (`nots` is 0 on each row). The format and
`build/tfhetool` accept all 7 ops.

## The form of the circuit (Q-S12-2 (a))

O3 makes each decided rule upward closed. Thus accept(c) = OR over the
minimal vectors m of AND over the classes j of (c_j >= m_j). The stages:

1. A popcount for each class: an adder tree gives c_j in
   b(n_j) = ceil(log2(n_j + 1)) bits.
2. A constant comparator chain for each class and threshold, LSB up:
   g = 1, then g = c_i AND g when m_i = 1, else g = c_i OR g. A
   threshold m_j = 0 is always true and gives no class term.
3. A balanced AND tree over the class terms of each minimal vector.
4. A balanced OR tree over the minimal vectors.

## Depth D and bound B (O6, Q-S12-4 (a))

The noise depth of each gate is 1: each gate reads inputs or bootstrap
outputs. D is the longest bootstrap chain from an input to `out` (a
`NOT` adds 0). The static bound B comes from n_j, k and M (the
`min_count`) before the build:

```
B    = max_j T(n_j) + clog2(k) + clog2(M)
T(n) = P(n) + b(n) - 1 for n >= 1, T(0) = 0
P(n) = P(ceil(n / 2)) + 2 (b(n) - 1) for n >= 2, P(0) = P(1) = 0
b(n) = ceil(log2(n + 1))
clog2(x) = ceil(log2 x) for x >= 1, clog2(0) = 0
```

P bounds the popcount, T adds the comparator chain, and the AND and OR
trees add clog2(k) and clog2(M). The proof is the comment in
`src/front/tfhe.h`. Folding only removes gates, so D <= B. The writer
checks D <= B for each circuit: D > B is a writer fault (`INTERNAL`,
exit 1), not a refusal.

The 19 circuits of the 7 rows:

| Row | Definitions | k | I | min_count | gates | D | B |
|---|---|---|---|---|---|---|---|
| `rule` | bar, barHonest, barFork | 1 | 5 | 1 | 14 | 6 | 10 |
| `honest` | h3 | 1 | 5 | 1 | 16 | 7 | 10 |
| `honest` | h4 | 1 | 5 | 1 | 14 | 6 | 10 |
| `unanimity` | all3, all3Fork | 1 | 3 | 1 | 6 | 4 | 5 |
| `minlast` | minLast | 2 | 5 | 1 | 8 | 4 | 6 |
| `oneofn` | any1 | 1 | 4 | 1 | 13 | 5 | 8 |
| `oneofn` | anyPair | 2 | 4 | 2 | 7 | 3 | 5 |
| `log2` | maj, majHonest, majFork | 1 | 4 | 1 | 13 | 5 | 8 |
| `log2` | all4, allFork | 1 | 4 | 1 | 11 | 4 | 8 |
| `log2` | duoBar, duoHonest, duoFork | 2 | 5 | 1 | 8 | 4 | 6 |
| `wide8` | atLeast4 | 8 | 8 | 70 | 279 | 9 | 10 |

## Caps and `REFUSE_DEPTH` (Q-S12-4 (a))

- A circuit with more than `TFHE_GATE_MAX` 65536 gates (65536 is
  accepted), or a rule with a `min_count` above `TFHE_MIN_CAP` 4096,
  gives `REFUSE_DEPTH`. `langc tfhe` writes no line, writes one line
  `langc: REFUSE_DEPTH: NAME: ...` to stderr, and exits 1.
- Only `langc tfhe` gives `REFUSE_DEPTH`. `langc check` accepts the
  program, and T1 does not change.
- Row B, `test/tfhe/refuse/mins.lang`: 16 classes of 1 party. The rule
  `halves` is an `rlAnd` of two `rlOr` chains of 70 `rlMin` terms each,
  so it has 4900 minimal vectors. `langc check` accepts it, and
  `langc tfhe` gives `langc: REFUSE_DEPTH: halves: the rule has 4900
  minimal vectors, more than 4096`.
- Row A (more than 65536 gates) is not reachable from a guest program. A
  unary `Size` above about 2000 does not evaluate (`EVAL_DEPTH`, limit
  2000), and `REFUSE_BOUND` keeps the count space at 2^20 or less. Thus
  the sum of n_j stays near 2600 or less, and the popcount stays near
  16000 gates. A wide `min` list (up to 4096 vectors over up to 20
  classes) can in principle go above 65536 gates, but S12 has no guest
  program of that shape. A scratch driver outside the repo checks the
  gate cap on synthetic facts (mutant M7b, below).

## Leakage (OI-5)

Only the accept bit leaves the circuit (Q-S12-1 (a)). The count vector c
stays inside the circuit; at T1 the output also leaks c (Q-S1-2 (a)).
The gates come only from public facts (the class sizes and the minimal
vectors), not from the ballots. The accept bit still leaks what the rule
implies: see `accept_reveals_all` and `reject_reveals_all` on the T4
`rule` line (`docs/REPORT-JSON.md`, "Leakage (OI-5)").

## Exit status

The exit status is 0 for the lines, 1 for a refused program (the check
diagnostics or `REFUSE_DEPTH` go to stderr) or a writer fault, and 2 for
a usage error.

## Rows

- `test/tfhe/NAME.lang` (copies of the 7 `test/report/` rows) and
  `test/tfhe/NAME.jsonl`: `test/gate.sh` runs `langc tfhe` on each
  `.lang` file, wants exit 0, and compares the bytes with the `.jsonl`
  file (Q-S12-7 (a)). Gate line: `tfhe rows: 7 checked`.
- `test/tfhe/refuse/*.lang`: `test/gate.sh` wants exit 1, an empty
  stdout and one stderr line `langc: REFUSE_DEPTH: ...`. Gate line:
  `tfhe refusals: 1 checked`.
- `build/tfhetool` (`test/tfhetool.c`, Q-S12-6 (a)): a Boolean simulator.
  For each circuit line it checks the T4 `rule` line of the same name
  (the classes, `min_count`, and the first `min_listed` vectors of
  `min`), the row shape (`op`, wire order, `gate_count`, `nots`, `out`),
  D again from the rows, and D <= B. Then it runs all 2^I input
  assignments (I <= 24) and compares the out bit with the OR-AND oracle
  on the full `min` list. For `wide8` the out bit must also equal "at
  least 4 of 8 bits". Gate line: `tfhe sims: 19 checked`.

## Mutants (S12)

| Mutant | Change | Result |
|---|---|---|
| M1 | the comparator chain swaps AND and OR | KILLED (rows, `build/tfhetool`) |
| M2 | the OR tree reads only the first 64 minimal vectors | KILLED (`wide8`, `build/tfhetool`) |
| M3 | the popcount carry drops the pass term | KILLED (rows, `build/tfhetool`) |
| M4 | D counts a `NOT` row as 1 | EQUIVALENT |
| M5 | no D <= B check at the write | EQUIVALENT |
| M5b | M5 and B = 0 | KILLED (rows, `build/tfhetool`: D above B) |
| M6 | a threshold m_j = 0 is read as 1 | KILLED (`oneofn`, `wide8`, `build/tfhetool`) |
| M6s | the m_j = 0 skip is removed | KILLED (`wide8` bytes only) |
| M7a | the min cap test is `>=` | KILLED (scratch cap driver only) |
| M7b | the gate cap test is `>` | KILLED (scratch cap driver only) |
| M8 | `out_line` writes the text without the JCS round trip | KILLED (the 7 tfhe rows and the 7 report rows) |

- M4 is equivalent. The builder writes only `XOR`, `AND` and `OR` rows.
  The popcount bits and carries are wires or the constant 0, never the
  constant 1, and the comparator chain starts at the constant 1 with
  `AND` or `OR`. Thus no row is a `NOT` (`nots` is 0 on each row), and D
  does not change. `build/tfhetool` gives a `NOT` row depth 0 and checks
  `nots`, so a later builder that writes `NOT` rows is checked.
- M5 is equivalent. D <= B holds by the bound proof in
  `src/front/tfhe.h`, so the check does not fire on the S12 builder.
  `build/tfhetool` checks D <= B on each line: M5b (B = 0) shows that
  this check kills a bound that is too small.
- M6s gives the same function. A threshold m_j = 0 folds to the constant
  1, and the AND tree drops it. Thus `build/tfhetool` passes, and only
  the row order of `wide8` changes. The golden bytes kill it.
- M7a and M7b are not killed by the gate, because no guest row is at a
  cap (row A above). A scratch driver (S12 work files, not in the repo)
  builds synthetic facts at 4096 and 4097 minimal vectors and at 65536
  and 65537 gates, and wants accept, `REFUSE_DEPTH`, accept and
  `REFUSE_DEPTH`. It kills both mutants.
