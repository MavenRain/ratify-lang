# JSON check (A2)

This file gives the schema of `langc check --json` (PLAN section 9 row
A2, S15, Q-S15-1 to Q-S15-8 (a)). The command checks PROG as
`langc check` does, then writes one JSON line to stdout: the first
diagnostic, its fix hint, and the progress of the check.

## The command

```
langc [--kit] check PROG --json
```

- Without `--json`, `langc check` is the text mode. The text mode does
  not change: the 181 check rows keep their bytes.
- `--json` is the only fourth argument. Another fourth argument gives
  `USAGE` (`too many arguments`) as text on stderr, exit 2.
- The exit codes are the same as in the text mode: 0 when PROG checks,
  1 when langc refuses PROG (a lexer, parser or check code), 2 when
  langc cannot read PROG (`IO`).
- In `--json` mode, langc writes no text to stderr. The one exception
  is a failure of the JSON writer (for example `OOM`): langc writes the
  text diagnostic of that failure to stderr and exits with 2.

## The line

- The line is one JSON object in its RFC 8785 (JCS) form, then `\n`
  (Q-S15-1 (a)). The writer (`report_check` in `src/front/report.c`)
  makes the JSON text through `JsonOut`, then `json_parse` and
  `jcs_write` make the bytes, as for T4 (`docs/REPORT-JSON.md`). Thus
  the keys are in JCS order and the line has no white space.
- The object has two keys: `diagnostics` and `progress`.
- `diagnostics` is an array of 0 or 1 object. The first failure gives
  the object. When PROG checks, the array is empty.
- Each number is a JSON integer.

Example (a refusal):

```
{"diagnostics":[{"code":"TYPE_MISMATCH","expected":"Type 0","fix":"Change the term until its type is the expected type, or change the declared type.","found":"Type 1","message":"expected a term of type Type 0, found a term of type Type 1","span":{"col":1,"file":"test/check/universe.lang","line":1}}],"progress":{"laws_assumed":0,"laws_at_target":0,"laws_discharged":0,"laws_external":0,"laws_total":0,"parsed":1,"typed":0}}
```

Example (`langc --kit check test/demo/demo.lang --json`, exit 0):

```
{"diagnostics":[],"progress":{"laws_assumed":1,"laws_at_target":0,"laws_discharged":2,"laws_external":1,"laws_total":2,"parsed":48,"typed":48}}
```

## The diagnostic object

A diagnostic object has 6 keys (Q-S15-1 (a)).

| Key | Value |
|---|---|
| `code` | The diagnostic code, as in the text mode. |
| `expected` | A string or `null`. See the pair rule. |
| `fix` | The fix hint of `code` from the fix table, or `null` when the table has no row for `code`. |
| `found` | A string or `null`. See the pair rule. |
| `message` | The message of the text line. The text line is `langc: CODE: WHERE: MESSAGE`; `message` is the MESSAGE part. |
| `span` | `{"col":C,"file":PATH,"line":L}` or `null`. See the span rule. |

### The span rule

- `span` gives the start position only (Q-S15-2 (a)). `file` is the
  PROG path as the command line gives it. `line` and `col` start at 1.
- A lexer or parser code gives the position of the token in its
  message.
- A check code gives the line and column of the declaration under
  check (`Decl.line`, `Decl.col`).
- `span` is `null` for `IO` and for each code that has no position.
- Known gap (D-U1-2): `front_load_domain` lexes the kit domain text
  under the PROG source name. A failure in a kit domain declaration
  thus gives the PROG path with a line and column of the domain text.
  No gate row gets to this case: the kit domain texts are fixed and
  check.

### The pair rule

- `expected` and `found` are strings only where the message has the
  form `expected X, found Y` (Q-U2-1 (a), D-U1-3): `PARSE_EXPECT`, and
  the two `TYPE_MISMATCH` sites of that form in `src/front/check.c`.
- The messages of the form `NAME needs X, found Y` give `null` for
  both keys.
- All other codes give `null` for both keys.

### The fix table

- `src/front/fix.c` has the fix table: 71 rows, sorted by code
  (Q-S15-3 (a), D-U2-1). `fix_of(code)` gives the hint, `fix_count()`
  and `fix_code(i)` give the rows.
- The rule: each code string at a `diag_fail`, `diag_fail_at`, `FAIL`,
  `FAIL_PAIR` or `kit_refused` site of `src/front` and `src/main.c` has
  one row. `IO` (`src/main.c`) and `REFUSE_FORMER` (PLAN section 7)
  also have a row. The table thus also has rows for codes that
  `langc check` does not give (the `fmt`, `tfhe`, `noir` and lower
  codes).
- The 11 PLAN section 7 codes: the row is the section 7 text, then one
  fix sentence. The `REFUSE_ORBIT` row is the section 7 text only,
  because that text has its own fix.
- No site in `src` gives `REFUSE_FORMER` today. `REFUSE_CELL` does the
  CM and CC shape check (`docs/STATUS.md`, known limits).
- One code has one fix text, and a code can have 2 or more sites in the
  checker. Thus each fix text is true at each site of its code
  (D-CLOSE-1, a USER ruling): the text gives each cause, or it tells
  the reader to use the message. The row of a section 7 code starts
  with the section 7 text.

## The progress object

The progress object has 7 keys (Q-S15-4 (a)).

| Key | Value |
|---|---|
| `parsed` | The program declarations after the parse. 0 when the read, the lexer or the parser fails. |
| `typed` | The program declarations that check before the first failure. A block that fails counts none of its declarations. `typed` = `parsed` when PROG checks. |
| `laws_total` | The DISCHARGED rows of the law table that apply. |
| `laws_discharged` | `laws_total` when PROG checks, 0 on a refusal. |
| `laws_assumed` | The ASSUMED rows that apply. |
| `laws_at_target` | The AT TARGET rows that apply. |
| `laws_external` | The EXTERNAL rows that apply. |

A program declaration is a declaration of PROG. The declarations of
the kit domain text are not program declarations.

### The triggers

A row of the law table applies when its trigger applies (Q-S15-5 (a)).

| Trigger | Applies when PROG has |
|---|---|
| R | A decided rule definition: a guest program definition, not a law, of the type `Rule c`, `Honest c f` or `ForkFree c f` at a closed c and a closed f (S5). |
| H | A decided rule definition of the type `Honest c f` or `ForkFree c f`. |
| F | A decided rule definition of the type `ForkFree c f`. |
| L | A closed `Log` definition. |

- In the kit mode, langc decides no rule definition (D-U2-6), thus R,
  H and F do not apply. The T4 `program` line has `"rules":0` in the
  kit mode for the same reason. The demo in the kit mode thus gives
  the L rows only (the example above).
- On a refusal, the triggers come from the definitions that check
  before the failure (D-U2-2). On `IO`, a lexer code or a parser code,
  no definition checks, thus all law counts are 0.
- A program with all four triggers gets the PLAN section 6 totals:
  `laws_total` 8 (and `laws_discharged` 8 when PROG checks),
  `laws_assumed` 8, `laws_at_target` 1, `laws_external` 2.

### The law table

`src/front/report.c` `LAWS[]` has the 19 rows.

| Law | Kind | Trigger |
|---|---|---|
| TCM-A | DISCHARGED | R |
| TCM-B | DISCHARGED | R |
| O2 | DISCHARGED | R |
| O3 | DISCHARGED | R |
| O4 | DISCHARGED | H |
| O5 | DISCHARGED | F |
| TCM-E | DISCHARGED | L |
| O10 | DISCHARGED | L |
| TCM-C | ASSUMED | R |
| TCM-D | ASSUMED | R |
| View 1 | ASSUMED | R |
| View 2 | ASSUMED | R |
| View 3 | ASSUMED | R |
| O7 | ASSUMED | R |
| O8 | ASSUMED | R |
| O9 | ASSUMED | L |
| O6 | AT TARGET | R |
| O1 | EXTERNAL | R |
| O11 | EXTERNAL | L |

## The gate rows

`build/checkjsontool` (`test/checkjsontool.c`, S15 U3, Q-S15-6 (a))
runs in `make check` and writes 3 count lines. A failed check writes a
`FAIL` line, and the tool exits 1.

- `json rows: 21 checked`. Each line of `test/json/rows.txt` is a row:
  `MODE RC PROG NAME`. The tool runs `build/langc check PROG --json`
  from the repo root (with `--kit` when MODE is `kit`). The exit code
  must be RC, stderr must be empty, and stdout must be
  `test/json/NAME.json` byte for byte. Each golden must also be its own
  RFC 8785 form: `json_parse`, then `jcs_write`, then `\n` give the
  same bytes.
- `json fix rows: 11 checked`. The tool reads the section 7 table of
  `docs/PLAN.md`, less the `REFUSE_DEPTH` and `REFUSE_NOIR` rows. The
  fix row of each code must start with the section 7 text.
- `json fix table: 71 codes`. The count is `fix_count()`. The rows must
  be sorted by code. At each `diag_fail`, `diag_fail_at`, `FAIL`,
  `FAIL_PAIR` and `kit_refused` site of `src/front` and `src/main.c`,
  the first string literal before the next `;` is the code when it has
  the form `[A-Z][A-Z0-9_]+`. Each such code must have a row.

The 21 rows:

- 11 PLAN section 7 rows: the first `test/check` program of each code
  in `test/check/expect.txt`. No program gives `REFUSE_FORMER` (see the
  fix table). Thus the 11th row is `test/check/guest-list.lang`, which
  gives `REFUSE_CELL`, the code of that refusal today (D-U3-2).
  `json fix rows` still checks the `REFUSE_FORMER` fix row.
- 6 ok rows. The law counts (discharged, assumed, at target, external)
  are: `norule` (no trigger) 0, 0, 0, 0; `rule` (R) 4, 7, 1, 1;
  `honest` (R, H) 5, 7, 1, 1; `forkfree` (R, H, F) 6, 7, 1, 1; `log`
  (R, H, F, L) 8, 8, 1, 2; `demo` (kit mode, L) 2, 1, 0, 1. A
  `ForkFree` definition alone sets H. A closed guest `Log` definition
  needs a `Policy` value, and a `Policy` needs an `Honest` and a
  `ForkFree` value. Thus the `log` row has all four triggers and the
  section 6 totals (D-U3-1).
- `LEX_CHAR` (`test/json/lexchar.lang`), `PARSE_EXPECT`
  (`test/json/parseexpect.lang`), `TYPE_MISMATCH`
  (`test/check/universe.lang`, example 1) and `IO` (a missing path).
  In the `LEX_CHAR` and `PARSE_EXPECT` rows, `message` starts with the
  `FILE:LINE:COL` position: the position of a lexer or parser code is
  in its message (Q-S15-2 (a)). The `IO` path has a `"` byte, thus
  `message` has the `\"` escape.
