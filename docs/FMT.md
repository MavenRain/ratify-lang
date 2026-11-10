# langc fmt: the canonical form

S14 (A1 in section 9 of `docs/PLAN.md`) adds `langc fmt`, the canonical
printer. The USER rulings are Q-S14-1 to Q-S14-8 (a) (`docs/PLAN.md`
section 1). Section 6 lists the decisions that are not rulings. The USER
can overturn each one.

## 1. The command

```
langc fmt PROG [--check]
```

- Without `--check`, fmt writes the canonical text of PROG to stdout and
  exits 0.
- With `--check`, fmt writes nothing to stdout. It exits 0 when the
  bytes of PROG are the canonical text. It exits 1 with `FMT_DIFF` when
  the bytes are different.
- When PROG does not lex or parse, fmt exits 1 with the diagnostic of
  the front end (a `LEX_` or `PARSE_` code).
- fmt reads the program only: `lex_source` and `parse_source` with
  `ORIGIN_PROGRAM`, as `load_one` in `src/front/front.c`. It does not
  load the domain and it does not check the program. Thus `sealed`,
  `law` and `history` print wherever they parse (N-U1-6).

| Code | When | Result |
|---|---|---|
| `FMT_TRAILING_COMMENT` | A line has code and then `--` | Exit 1, no stdout, `source:line:col` of the `--` |
| `FMT_DIFF` | `--check`, and the bytes are not the canonical text | Exit 1, no stdout |

## 2. The canonical form

Declarations (Q-S14-4 (a)):
- One declaration is on one line. One space is between two tokens.
- When the first line is above 100 columns, fmt breaks it once, after
  `:=`, and indents the body by 2. There is no other break.
- The width is in bytes, without the newline. "Above 100" is 101 or
  more (N-3).
- A family prints its first constructor on the header line, because
  the parser refuses a `|` before the first constructor. Each later
  constructor is on its own line `  | c ...`. The width rule looks at
  the first line only. Above 100 bytes, the break after `:=` moves the
  first constructor to the next line. A later constructor line above
  100 bytes does not break (N-U1-4).
- An axiom has no `:=`, thus it never breaks (N-U1-4). A family with no
  constructor has no space after `:=`, thus it never breaks (N-U1-8).

Binders (Q-S14-3 (a)):
- Sigma prints in the keyword form `Sigma (x : A) B`, not in the star
  form `(x : A) * B`.
- One name is in each group: `(x y : A)` prints as `(x : A) (y : A)`.
- Consecutive fun binders merge into one fun:
  `fun (x : A) (y : B) => b`. A bare binder stays bare:
  `fun x y => b`.
- Pi and bare `fun x` print as written, because their AST is different.

Spaces and parentheses (N-U1-2, N-U1-3):
- No space is inside `( )`, after `~` or before `,`.
- A system prints as `[ k -> t, k -> t ]`. The empty system is `[ ]`.
- A term gets the minimum parentheses for the grammar levels of
  `src/front/parser.c`. `Type n` and `Sigma (x : A) B` are atoms of the
  grammar, but fmt gives them the level of an application. Thus they
  get parentheses as an application argument, under `~` and as a Sigma
  body: `Path (Type 0) A B`.

Comments and blank lines (Q-S14-1 (a), N-1, N-U1-5):
- The lexer drops the comments. fmt scans the text a second time and
  keeps each full-line comment. `--` starts a comment at each position,
  because no token holds `--`.
- A comment prints at column 0, from the `--`, without trailing white
  space. It prints above the declaration that follows it, or at the end
  of the file.
- A comment inside a declaration on more than one line prints after
  that declaration.
- fmt refuses a comment after code on the same line:
  `FMT_TRAILING_COMMENT`.
- A run of blank lines between two items prints as one blank line. No
  blank line is at the start or at the end of the file. fmt drops a
  blank line inside the token span of a declaration.

## 3. The laws

Q-S14-5 (a). For each `.lang` file x of the corpus that parses:
- idempotent: fmt(fmt x) = fmt x, as bytes;
- ast: dump(parse(fmt x)) = dump(parse x).

dump is the AST as an S-expression, without positions. The test tool
`test/asttool.c` (`build/asttool PROG`) prints it, one line for each
declaration (N-U2-1). It prints all the Decl fields for each kind and
the history mark of each field (N-U2-2). The ast law also wants a dump
of x that is not empty. Thus a file with no declaration cannot pass by
accident (N-U2-5).

The corpus is `rg --files -g '*.lang' domain examples test` (N-U2-4):
284 files. fmt accepts 267. It refuses 17: 16 rows of `test/parse/`
with a `LEX_` or `PARSE_` code (N-U2-3), and
`test/fmt/refuse/trailing.lang` with `FMT_TRAILING_COMMENT`.

Q-S14-2 (a): the parser keeps the two forms of each syntax, and fmt
prints one. No refusal changes. `langc fmt PROG --check` on each
`examples/*.lang` file keeps the examples canonical (37 files, N-U2-6).
S14 rewrote 34 of the 37 examples once with `langc fmt`. The line
references into them in `formers/FORMERS.md` and `formers/tcc-wasm.md`
moved to the new line of the same declaration (14 references, N-U2-7).
No golden file changed.

## 4. The rows

`test/fmt/NAME.lang` is the input and `test/fmt/NAME.fmt` is its
canonical text (Q-S14-6 (a)). Each input is not canonical on purpose.
For each row, the gate wants (N-U1-7):
- fmt of NAME.lang gives the bytes of NAME.fmt;
- `--check` on NAME.fmt exits 0 with no output;
- `--check` on NAME.lang exits 1 with `FMT_DIFF`.

| Row | The rewrites it tests |
|---|---|
| sigma | star form to `Sigma`; `Sigma (x y : A)` split; nested Sigma body in parentheses; extra body parentheses dropped; Sigma as an application argument in parentheses |
| groups | Pi group, family parameters, constructor fields, fun group: one name in each group |
| fun | chained bare fun merge `fun x y`; typed merge; bare and typed mixed; `(fun z => z) y` keeps its parentheses |
| comments | blank lines at the start and the end dropped; trailing spaces cut; 3 blank lines to 1; indented comment to column 0; comment and blank line inside a declaration; comment at the end of the file |
| width | 100 bytes not broken (input on 2 lines); 101 broken; long family header broken; later constructor line of 112 bytes not broken; axiom of 168 bytes not broken |
| family | index, constructor results, path constructor boundary, all written on many lines |
| forms | state with history, sealed family, law def, def rec, an `and` block, cofamily, axiom |
| interval | `~(i /\ j)`, `(i \/ j) /\ k`, `p @ i /\ j`, `p @ (f x)` to `p @ f x`, `~~i`, minimum parentheses in join and meet, left-associative join |
| systems | spaces of `[~i -> a,i -> b]`, `[  ]` to `[ ]`, a branch body that is a fun with a system |
| parens | `f (g x)`, `f (Type 0)`, `(A -> B) -> C`, `((x : A) -> B) -> C`, `f (x -> y)` kept; `((f) (x))`, `A -> (B -> C)`, `(f x) y` dropped |
| refuse/trailing | `def a : Nat := zero -- note`: exit 1, `FMT_TRAILING_COMMENT` at 1:21, no stdout |

The gate lines of S14:

```
fmt rows: 10 checked
fmt refusals: 1 checked
fmt idempotent: 267 checked, 17 refused
fmt ast: 267 checked
fmt examples: 37 checked
```

The C write-error runner also checks fmt with writable and read-only
stdout: `fmt write errors: 2 checked`. With these 6 lines, the gate has
49 count lines (S13: 43).

## 5. The mutants

Q-S14-7 (a). Each mutant is one edit in `src/front/fmt.c`. For each
mutant, `make -B build/langc build/asttool` and then `sh test/gate.sh`.
The gate kills the mutant when it exits with a value that is not 0 and
prints at least one FAIL line.

| Mutant | Edit in `src/front/fmt.c` | Killed by |
|---|---|---|
| M1 | Sigma prints as `(x : A) * B` | row `fmt sigma` (text, `--check`); `fmt examples` `--check` on 7 files (bagorbit, cocell, kan, oneuse, orbit, regimes, sigma). 9 FAIL lines |
| M2 | no group split: two fields that share one type print as one group `(x y : A)` | row `fmt groups` (text). 1 FAIL line |
| M3 | fmt drops the comments after the last declaration | row `fmt comments` (text, `--check`). 2 FAIL lines. The ast law cannot see comments |
| M4 | an application argument prints at the application level, not the atom level: a needed parenthesis is dropped | rows `fmt family`, `fmt parens`, `fmt sigma` (text, `--check`); `fmt ast` on 158 files; `fmt examples` `--check` on 36 files. 200 FAIL lines |
| M5 | `--check` exits 0 always | `--check` on the `.lang` file of each of the 10 rows: want exit 1 and `FMT_DIFF`. 10 FAIL lines |
| M6 | the break comes above 99 columns, not above 100 | row `fmt width` (text, `--check`); `fmt examples` `--check` on algebra, cocell, crypto. 5 FAIL lines |

Result: the gate killed 6 of 6 mutants. For each mutant the gate exit
was 1. After each mutant, the restored `src/front/fmt.c` was the same as
the copy (`cmp` 0). The final rebuild gave exit 0.

## 6. Decisions that are not rulings

The USER can overturn each one.

- N-1: a run of blank lines between two items prints as one blank line;
  none at the start or the end of the file.
- N-2: the command is `langc fmt PROG [--check]`, as
  `langc abi PROG [--text]`.
- N-3: the width is in bytes, without the newline; "above 100" is 101
  or more.
- N-U1-1: the codes `FMT_TRAILING_COMMENT` and `FMT_DIFF` (section 1).
- N-U1-2: the spaces of section 2.
- N-U1-3: `Type n` and `Sigma` get the level of an application.
- N-U1-4: the family and axiom width rules of section 2.
- N-U1-5: the comment rules of section 2.
- N-U1-6: fmt parses the program only, with no domain and no check.
- N-U1-7: the gate also runs `--check` on each row.
- N-U1-8: a family with no constructor has no break position.
- N-U2-1: the AST dump is the test tool `test/asttool.c`, so the
  `langc` command set does not grow.
- N-U2-2: the dump prints all the Decl fields for each kind.
- N-U2-3: "a parse code" in the law is a `LEX_` or `PARSE_` code.
- N-U2-4: the gate lists the corpus with `rg --files`, which honors
  `.gitignore`.
- N-U2-5: the ast law wants a dump of x that is not empty.
- N-U2-6: `fmt examples` covers `examples/*.lang` (37 files).
- N-U2-7: a line reference that the examples rewrite moved points at
  the new header line of the same declaration.
