# JSON report (T4)

This file gives the schema of the T4 target (PLAN section 8.2, S11,
Q-S11-1 to Q-S11-10 (a)). `langc [--kit] report PROG` checks PROG and
writes JSON Lines to stdout: the static facts of the rules, the policies
and the logs of PROG. It does not lower PROG.

## Rules for each line

- A line is one JSON object in its RFC 8785 (JCS) form, then `\n`
  (Q-S11-3 (a)). The writer (`src/front/report.c` and `src/jsonout.c`)
  makes the JSON text, then `json_parse` (`host/jsonin.c`) and
  `jcs_write` (`host/jcs.c`) make the bytes. Thus the keys are in JCS
  order and the line has no white space. `src/jsonout.c`,
  `host/jsonin.c` and `host/jcs.c` are copies of permit-lang.
- Each line has `"ratify":1` (the schema version) and `"kind"`:
  `"program"`, `"rule"`, `"policy"`, `"log"` or `"height"` (Q-S11-2 (a)).
- Each number is a JSON integer. Each value is below 2^53, because a
  count space has at most 2^20 vectors (`REFUSE_BOUND`).
- A report string cannot have `"`, `\`, a control byte or a bad UTF-8
  byte: a definition name matches `[A-Za-z_][A-Za-z0-9_']*`, and the other
  strings are constants. `build/jsontool` checks the escapes of the
  writer (Q-S11-10 (a)).
- A line has no time field (PLAN section 5).

## Line order

1. One `program` line.
2. One `rule` line for each decided rule definition, in definition order.
   A decided rule definition is a program definition that is not a law,
   with the type `Rule`, `Honest` or `ForkFree` at a closed committee and
   a closed f (S5). An open committee or an open f gives no decision and
   no line.
3. One `policy` line for each closed `Policy` definition, in definition
   order.
4. For each closed `Log` definition, in definition order: one `log` line,
   then its `height` lines for h = 0 to the length.

## The program line

| Key | Value |
|---|---|
| `kind` | `"program"` |
| `logs` | the count of `log` lines |
| `policies` | the count of `policy` lines |
| `ratify` | `1` |
| `rules` | the count of `rule` lines |

## The rule line

The check pass keeps one record for each decided rule definition when
the command is `report`, so the report does not decide a rule again.
Let k be the class count, n_j the size of class j, n the sum of the n_j,
`least` the smallest accepting total and `overlap` the smallest
sum_j max(0, a_j + b_j - n_j) over two minimal accepting vectors a and b
(PLAN section 4.2).

| Key | Value |
|---|---|
| `accept_reveals_all` | `true` when `accept_vectors` is 1: only the full vector accepts (unanimity, see Leakage) |
| `accept_vectors` | the count of accepting vectors of the count space |
| `classes` | the class sizes `[n_1, ..., n_k]` |
| `f` | the fault bound (`Honest` and `ForkFree` only) |
| `kind` | `"rule"` |
| `leak_t1` | `"counts"`: at T1 the output is the count vector c (Q-S1-2 (a)) |
| `min` | the minimal accepting vectors in index order, at most 64 (Q-S11-4 (a)) |
| `min_count` | the count of all minimal accepting vectors |
| `min_listed` | the count of vectors in `min`: `min_count`, but at most 64 |
| `name` | the definition name |
| `o4_margin` | `least - f` (`Honest` and `ForkFree` only) |
| `o5_margin` | `overlap - f` (`ForkFree` only) |
| `ratify` | `1` |
| `read_set` | `n - least + f + 1` (`ForkFree` only, Q-S11-5 (a)) |
| `reject_reveals_all` | `true` when `reject_vectors` is 1: only the zero vector rejects |
| `reject_vectors` | the count of vectors of the count space minus `accept_vectors` |
| `type` | `"Rule"`, `"Honest"` or `"ForkFree"` |

The read set: a set W with |W| >= `least` and a set R with |R| =
`read_set` have at least `least + read_set - n = f + 1` parties in
common. Thus the value is sound for each monotone rule. For a plain
t-of-n rule, `least` is t and the value is n - t + f + 1 (design sec 10).

The index order: the index of the vector v is v_1 + (n_1 + 1) (v_2 +
(n_2 + 1) (v_3 + ...)), so class 1 is the low digit. `rule_next`
(`src/front/check.c`) increments v_1 first. For `anyPair` of
`test/report/oneofn.lang` (classes `[2,2]`), `min` is `[[1,0],[0,1]]`.

The cap: `min` keeps the first 64 vectors and `min_count` counts all of
them. For `atLeast4` of `test/report/wide8.lang` (8 classes of 1),
`min_count` is 70 and `min_listed` is 64.

## The policy line

For each closed `Policy` definition (`plOf` with `plCom`, `plF`,
`plAuth` and `plAmend`), the report decides the authorize rule (kind 1,
`Honest`) and the amend rule (kind 2, `ForkFree`) again with
`rule_decide`. The line does not name these rules.

| Key | Value |
|---|---|
| `amend_o4_margin` | the O4 margin of the amend rule |
| `amend_o5_margin` | the O5 margin of the amend rule |
| `auth_o4_margin` | the O4 margin of the authorize rule |
| `classes` | the class sizes of `plCom` |
| `f` | `plF` |
| `kind` | `"policy"` |
| `name` | the definition name |
| `ratify` | `1` |
| `read_set` | the read set of the amend rule |

## The log line and the height lines

The `log` line has `kind` `"log"`, `length` (the `lgLen` of the log),
`name` and `ratify`.

Then one `height` line for each h from 0 to `length` (Q-S11-7 (a)). It
has `kind` `"height"`, `log` (the log name), `height` (h), `ratify` and
the keys of the policy line of policyAt h, without `name`. The report
walks the closed `Log` value: the policy at 0 is the source policy, and
the policy at h >= 1 is the `amTo` of amendment h, which is policyAt h by
definition (`domain/domain.lang:404-432`). For h >= 1 the line also has:

| Key | Value |
|---|---|
| `approvals` | the count vector of the approval set of amendment h |
| `decision` | `"accept"` |

A checked `Log` has an accepting amendment at each height (`amOk`,
`domain/domain.lang:402`). The verdict `invalid r` comes only from
`verify` on a `Certificate`, in `ratifyd`.

The log lines of `test/report/log2.lang`:

```json
{"kind":"log","length":2,"name":"log012","ratify":1}
{"amend_o4_margin":2,"amend_o5_margin":1,"auth_o4_margin":2,"classes":[4],"f":1,"height":0,"kind":"height","log":"log012","ratify":1,"read_set":3}
{"amend_o4_margin":3,"amend_o5_margin":3,"approvals":[3],"auth_o4_margin":2,"classes":[4],"decision":"accept","f":1,"height":1,"kind":"height","log":"log012","ratify":1,"read_set":2}
{"amend_o4_margin":3,"amend_o5_margin":2,"approvals":[4],"auth_o4_margin":3,"classes":[2,3],"decision":"accept","f":1,"height":2,"kind":"height","log":"log012","ratify":1,"read_set":3}
```

## Leakage (OI-5)

Design sec 13 (D:624), first item: the output leaks what the rule
implies. T4 states this leak for each rule (Q-S11-6 (a)). When
`accept_reveals_all` is `true`, an accept tells that each party approved.
When `reject_reveals_all` is `true`, a reject tells that no party
approved. At T1 the output also leaks the count vector c (`leak_t1`).
T2 (S12) gives the one-bit output (`docs/TFHE-CIRCUIT.md`).

## Exit status

The exit status is 0 for the lines, 1 for a refused program (the check
diagnostics go to stderr) or a failed write, and 2 for a usage error.
Each line is written and flushed; a failed write, `fputc` or `fflush`
gives 1.

## Rows

- `test/report/*.lang` and `test/report/*.json`: `test/gate.sh` runs
  `langc report` on each `.lang` file, wants exit 0, and compares the
  bytes with the `.json` file. The 7 rows are `honest` (Honest rules
  only), `log2` (8 rules, 3 policies, a log of 2 heights), `minlast`
  (`rlMin (2,2)` over classes `[3,2]`), `oneofn` (1-of-n rules:
  `reject_reveals_all`), `rule` (a `Rule`, an `Honest` and a `ForkFree`
  definition at a plain t-of-n), `unanimity` (`accept_reveals_all`) and
  `wide8` (the cap). Gate line: `report rows: 7 checked`.
- `build/reporttool` (`test/reporttool.c`, a port of permit-lang): the
  report of `test/report/rule.lang` must exit 0 with a writable stdout
  and 1 with a read-only stdout. Gate line: `report write errors: 2
  checked`.
- `build/jsontool` (`test/jsontool.c`, Q-S11-10 (a)): 5 rows write a
  string with `"`, `\`, a newline, U+0001 or the byte 0xff through
  `out_bytes` and `out_line`, and compare the line with its RFC 8785
  form. Gate line: `json escapes: 5 checked`.

## Mutants (S11)

| Mutant | Change | Result |
|---|---|---|
| M1 | `o4_margin` + 1 | KILLED (`honest`, `log2`, `rule`, `unanimity`) |
| M2a | the minimal test reads the first class only | KILLED (`log2`) |
| M2b | the minimal test reads the last class only | KILLED (`minlast`) |
| M3 | `read_set` without the + 1 | KILLED (`log2`, `rule`, `unanimity`) |
| M4 | `out_bytes` does not escape `"` | KILLED (`build/jsontool`) |
| M5 | `out_line` writes the text without the JCS round trip | KILLED (all 7 rows) |
| M6 | the cap is 63 | KILLED (`wide8`) |
| M7a | the height policy takes `set_k` from `amTo` | KILLED (`log2`) |
| M7b | the height committee comes from `amTo` | EQUIVALENT |
| M8 | the log walk starts at height 1 | KILLED (`log2`) |

M7b is equivalent: the count vector counts the approval set (field 2 of
the amendment), and the committee is only the motive index of that
elimination. Thus the count vector does not change, and no row can kill
the mutant.
