# ratify-lang specification

Status: S19 completes M0, and the plan is CLOSED (section 10).
Section 9 gives the open items. `ratify-lang` is a working name.

`docs/PLAN.md` holds the rulings and the slices. `docs/VALIDATION.md`
holds the evidence.

## 1. Purpose

ratify-lang is a Cubical TT language for threshold authorities that
amend themselves (`design/DESIGN.md`). A policy is a committee in
classes, an authorize rule, an amend rule and a fault bound. A rule
reads only the count of approvals in each class. Thus each decision
is one bit, and it does not depend on the names of the parties. The
checker proves the policy facts before any vote. The old amend rule
judges each new policy, one step at a time. The time is the height
of the log, and no program reads a clock (PLAN section 5, :413). A
verifier host replays the log and checks the certificates. The
design is a denotational design over Unified Aggregation Theory.

No program can have a runtime error (R0, PLAN section 2, :253): the
Nat operations are total, `REFUSE_PARTIAL` refuses a partial kit
word, and the Wasm writer emits no trap form (`test/notraptool.c`
checks each built module). PLAN section 6 (:422) gives the laws and
the obligations that the checker discharges. The compiler `langc`
checks the program, its types and its laws, and then writes the
targets.

The host is TinyCC (`PIN`). PLAN section 8.2 (:485) gives four
targets:

- T1, the verifier host: one Wasm module for each program
  (`langc build`) and the C host `ratifyd` with its Wasm engine
  (`host/`). The demo is in `test/demo/` (PLAN section 8.4, :582).
- T2, the TFHE tally circuit (`langc tfhe PROG`,
  `docs/TFHE-CIRCUIT.md`).
- T3, the ZK relations R_ballot and R_dec as Noir (`langc noir PROG`,
  `docs/NOIR-RELATIONS.md`) and the Nargo packages in `noir/`.
- T4, the JSON report (`langc report PROG`, `docs/REPORT-JSON.md`).

`src/main.c` :68 to :80 gives the 13 commands, and :83 gives the
usage. There is no `run` command and no `prove` command.

Its type formers are F1 to F15 of `formers/FORMERS.md`. Its core data
types and core operations are only the types and operations of
`design/DESIGN.md` (the design). PLAN section 4 (:355) gives the
minimal base signature (R2) of the design as names, so sections 5
and 6 cite PLAN section 4.

The tools for agents are in PLAN section 9 (A1 to A6, :648):

- `langc fmt PROG [--check]` prints the one canonical form of a
  program (S14, `docs/FMT.md`).
- `langc check PROG --json` gives the JSON report of the checker
  (S15, `docs/CHECK-JSON.md`).
- `langc mcp` is the MCP server (S16, `docs/MCP.md`). The verbs
  `langc explain CODE`, `langc list-base` and `langc show-law [ROW]`
  also serve agents.
- `skills/ratify-lang/SKILL.md` has the graded examples G1 to G5
  (`examples/skill/`). `AGENTS.md` and the README table say when to
  use the language (S17).
- `docs/TOKEN-BASELINE.md` compares the tokens of G1 to G5 in
  ratify-lang, Python and Rust (A6, S18). By the offline estimate,
  the Python sum is 0.40 of the ratify-lang sum, and the Rust sum
  is 0.58. S18 did not run `make tokens`.

`langc mcp` serves the author of a program. `ratifyd --call` runs a
T1 entry with a byte image and explicit word arguments
(`host/ratifyd.c` :208 to :242). The demo runner `demotool` combines
the Wasm checks with evaluator verification (section 8).

## 2. Programs

A program is a sequence of definitions:

```
def NAME : TYPE := TERM
```

The domain file `domain/domain.lang` declares the base words of PLAN
section 4 (:355). A kit program (`langc --kit`, `src/main.c` :438)
can also use the host imports of the verifier host, for example
`sig_verify` (G5).

The compiler refuses these host forms in a program
(`formers/tcc-wasm.md`): a data declaration
(`test/check/data.lang`), an axiom (`test/check/axiom.lang`),
general recursion (`test/check/rec.lang`) and a definition that uses
a core name or a prelude name (`test/check/core-name.lang`).
`test/check/expect.txt` gives the code of each mutant. Thus a program
cannot add a data type, an unproved fact or general recursion.
Recursion comes only from `fold` and `unfold` (F6, F7). The compiler
writes the targets; a program cannot.

The lowering has two HOST-LIMIT refusals (`formers/tcc-wasm.md`):
`REFUSE_RUNTIME_TREE` (a fold over a tree that exists only at run
time, F6) and `REFUSE_RUNTIME_CLOSURE` (a function value at run time,
F9).

PLAN section 7 (:454) adds eleven check codes for the domain
(Q-S0-6 (a)). Each code has a fix hint (A2). The last two rows are
not check codes: only `langc tfhe` gives `REFUSE_DEPTH` (S12,
Q-S12-4 (a)), and only `langc noir` gives `REFUSE_NOIR` (S13).

| Code | When | Design |
|---|---|---|
| `REFUSE_ARROW` | A ranked choice over 3 or more options. | sec 7 |
| `REFUSE_ORBIT` | A rule names a party inside its class. Fix: a class with only that party. | sec 4 box |
| `REFUSE_APPROX` | An approximate scheme (CKKS). | TCM-C |
| `REFUSE_CLOCK` | A wall clock or a host tick. | sec 10 |
| `REFUSE_FIXPOINT` | A policy that ratifies itself, or general recursion. | sec 8 |
| `REFUSE_RULE` | A rule that is not monotone, accepts 0, or does not accept n (O3). | sec 11 |
| `REFUSE_HONEST` | The O4 margin is 0 or less. | sec 11 |
| `REFUSE_FORK` | The O5 margin of the amend rule is 0 or less. | sec 11, TCM-B |
| `REFUSE_BOUND` | The product of (n_j + 1) is above 2^20 (Q-S0-4 (a)). | R0 |
| `REFUSE_PARTIAL` | A partial kit word. | R0 |
| `REFUSE_FORMER` | A type constructor that is not CM or CC. | R1 |
| `REFUSE_DEPTH` | At `langc tfhe` only: more than `TFHE_GATE_MAX` 65536 gates, or more than `TFHE_MIN_CAP` 4096 minimal vectors. `langc check` and T1 do not change. | O6 |
| `REFUSE_NOIR` | At `langc noir` only: two decided rule definitions with one K name, or a member count above the u32 range. Exit 1, no text. `langc check` and T1 do not change. | O8 |

## 3. Type formers

The type formers are F1 to F15 of `formers/FORMERS.md`. This language
uses the tcc-wasm column of the realization matrix.
`formers/tcc-wasm.md` gives the host form of each former. F1 to F4 (F4
is `List`), F11 and F13 are DONE with no limit. The table gives only
the other formers.

| ID | Status on tcc-wasm | Effect on this language | Open item |
|---|---|---|---|
| F5 | DONE over Option, Sum E and List | The domain monads are guest families (section 4): `Log` (`domain/domain.lang` :404), `Enc` (:441) and `Share` (:454) | none |
| F6 | DONE over Nat, List and domain families. HOST-LIMIT: a run-time tree | `REFUSE_RUNTIME_TREE` when a tree exists only at run time (`test/ir/tree.lang`). A fold over the block `Hop` and `Echo` (`domain/domain.lang` :276) gets `REFUSE_LOWER` (`test/ir/hop.lang`) | none |
| F7 | DONE into List with a Nat step limit | none | none |
| F8 | DONE over Option and List | none | none |
| F9 | DONE. HOST-LIMIT: a function value at run time | `REFUSE_RUNTIME_CLOSURE` (`test/ir/closure.lang`); a lambda stays only as the direct argument of an eliminator | O16 |
| F10 | DONE over `Type 0` and over Nat | none | none |
| F12 | DONE. `trans` is a guest def by `hcomp` (`domain/domain.lang` :22) | A path erases on Wasm | none |
| F14 | DONE for Type 0 and Type 1, without cumulativity | none | none |
| F15 | PARTIAL: only the domain file declares an indexed family | A program cannot declare an indexed family. The indexed domain families include `Party`, `Rule`, `Amendment`, `Log`, `Enc`, `Share` and `Certificate` (`domain/domain.lang` :284 to :522) | none |

S19 corrected four kit cites of `formers/tcc-wasm.md` (Q-S19-5 (c)).
The F12 row cites `trans` at :22 and the F13 row cites `transport` at
:9 of the domain file. The F15 row (:28) cites :140, the first family
with a value index. The F9 note (:40) names `docs/host/CAPABILITY.md`,
which gives the HOST-LIMIT reasons.

## 4. Structures

- **Monad**: `pure`, `map`, `bind` on Option, Sum E and List (F5). The
  log `Log P Q` (`domain/domain.lang` :404) is the indexed cell monad
  (CM) of PLAN section 3 (:339): the free category on accepted
  amendments. `lgNil P` is the unit, `lgCons` adds one amendment, and
  `append` (:406) is the bind. `appendNilL`, `appendNilR` and
  `appendAssoc` (:411, :413, :417) give the category laws as paths.
  `Enc` and `Share` (:441, :454) are CMs with no cells (PLAN section 3,
  :341). `enc` (:445) and `share` (:455) are the unit. `encMeaning`
  (:447) gives the value that an `Enc` holds.
- **Algebra**: `fold` on Nat, List and the domain families (F6).
  `fmFold` (:116) folds a `FreeMon` (:99) into a set. `lgLen` (:421)
  gives the length of a log as a `Size` (:48). `unfold` makes a List
  with a Nat step limit (F7). The cofamily `Stream` (:54) has
  `sExtend` (:57) and `sDuplicate` (:59). `System` (:434) is
  `Stream Policy`, the cocell comonad (CC) of PLAN section 3 (:340),
  the dual of `Log`. `systemOf` (:435) makes a `System` from a log.
- **Filterable**: `filter` on Option and List (F8).

The host does not give these carriers: `FreeMon`, `Stream`, `Size`,
`Log`, `System`, `Enc` and `Share`. The domain file declares each of
them.

## 5. Core types

The PLAN section 4.1 table (:362 to :377) is the source. The domain
file `domain/domain.lang` declares each type in the host forms of
section 2. No definition here is probe-forced: the consequences
section of `probe/CAPABILITY.md` (the S19 U2 probe) changes no
definition.

| Type | Meaning (PLAN section) | Definition |
|---|---|---|
| `Party` | An element of the finite committee (4.1, :364) | `domain/domain.lang` :284: `Party c`, a class and a `Pos` in that class |
| `Committee` | Classes 1 to k with sizes n_1 to n_k (4.1, :365) | `domain/domain.lang` :282: `cmNil`, and `cmClass` with a `Size` for each class |
| `Approvals` | A subset of the committee. A missing ballot is not an approval (4.1, :366) | `domain/domain.lang` :290: `Approvals c` |
| `Counts` | A vector c with 0 <= c_j <= n_j (4.1, :367) | `domain/domain.lang` :294: `Counts c` |
| `Rule` | A monotone accept on counts, false at 0 and true at n (4.1, :368) | `domain/domain.lang` :369: `rlMin`, `rlAnd`, `rlOr`, `rlTable` (Q-S0-7 (a)); `accept` (:373) |
| `Policy` | Committee, authorize rule, amend rule, fault bound f (4.1, :369) | `domain/domain.lang` :387: `plOf`. O4 is `Honest` (:381) and O5 is `ForkFree` (:382), as fields |
| `Decision` | `Bool` (4.1, :370) | `domain/domain.lang` :386: `Sum Unit Unit` |
| `Amendment P P'` | An approval set S that the amend rule of P accepts, and a policy P' (4.1, :371) | `domain/domain.lang` :402: `amOf` |
| `Log P Q` | The CM of section 3 (4.1, :372) | `domain/domain.lang` :404: `lgNil`, and `lgCons` (:405) |
| `System` | The CC of section 3 (4.1, :373) | `domain/domain.lang` :434: `Stream Policy` |
| `Height` | `Fin (length + 1)` of a given log (4.1, :374; Q-S0-3 (a)) | No name of its own: `Pos (szS (lgLen P Q l))` in `policyAt` (:431) |
| `Enc a` | An exponential ElGamal ciphertext on BN254 G1 at T1 (4.1, :375) | `domain/domain.lang` :441: `encOf` over a `Base` (:438) |
| `Share a` | A Shamir share at T1 (4.1, :375) | `domain/domain.lang` :454: `shareOf` |
| `Proof R x` | A sigma proof at T1. A witness exists, and the proof does not show it (4.1, :375) | `domain/domain.lang` :462: a `Trunc1` |
| `Protocol a b` | The ideal function of a protocol (4.1, :375) | `domain/domain.lang` :469: the function type. A family with a function field is not a cell signature (R1) |
| `Certificate` | (h, a, ballots, proofs, b), and for an amendment P' and the hash of entry h (4.1, :376) | `domain/domain.lang` :522: `certOf` with 11 fields. Each check result is a `Decision` field |
| `Verdict`, `Reason` | `valid`, or `invalid` with a reason (4.1, :377; 2) | `domain/domain.lang` :479, :478: 8 reasons, `malformed` to `equivocation` |
| `Evidence` | The equivocation evidence of S8 (12, :770) | `domain/domain.lang` :501, and the list `EvList` (:502) |
| `Size` | The step index and the size of a class | `domain/domain.lang` :48: `sz0`, `szS` |

G_P (the class-preserving group, design sec 4) is not a user type. The
checker gets it from the classes (PLAN :379 to :385). A rule reads only
`Counts`, so each rule is orbit-constant. A rule that names one party
inside its class is refused (`REFUSE_ORBIT`).

## 6. Core operations

The PLAN section 4.2 table (:389 to :402) is the source. Each type is
the type in `domain/domain.lang`. The meaning is the denotation of the
operation.

| Operation | Type | Meaning (PLAN section) |
|---|---|---|
| `count P S` | `count : (c : Committee) -> Approvals c -> Counts c` (:310) | The vector of the sizes of S intersect class_j (4.2, :391) |
| `authorize P a S` | `authorize : (P : Policy) -> (A : Type 0) -> A -> Approvals (polCom P) -> Decision` (:395) | accept_P(count_P S) (4.2, :392) |
| `and r r'`, `or r r'` | `rlAnd`, `rlOr` : `(c : Committee) -> Rule c -> Rule c -> Rule c` (:370, :371) | Pointwise on counts. `or` on an amend rule checks O5 again (4.2, :393) |
| `nil P`, `cons e log`, `append l1 l2` | `lgNil`, `lgCons` (:404, :405); `append : Log P Q -> Log Q R -> Log P R`, with the policies as arguments (:406) | The free category (4.2, :394) |
| `policyAt log h` | `policyAt : (P : Policy) -> (Q : Policy) -> (l : Log P Q) -> Pos (szS (lgLen P Q l)) -> Policy` (:431) | P_0 at 0; the target of entry h at h + 1 (4.2, :395) |
| `enc pk a` | `enc : (b : Base) -> Nat -> El b -> Enc b` (:445) | a (4.2, :396) |
| `eval g c_1 ... c_m` | `eval : (El a -> El b -> El c) -> Enc a -> Enc b -> Enc c`, with the bases as arguments (:450) | g of the meanings (4.2, :397) |
| `share t n s` | `share : (b : Base) -> Size -> Size -> El b -> Share b` (:455) | s (4.2, :398) |
| `reshare C C' t' xs` | `reshare : (b : Base) -> Committee -> Committee -> Size -> Share b -> Share b` (:459) | The same secret, new holders (4.2, :398) |
| `reconstruct T xs` | `reconstruct : (b : Base) -> Size -> Share b -> El b` (:457) | s, total by Q-S0-2 (a) (4.2, :399) |
| `prove R x w` | `prove : (x : X) -> (w : W) -> R x w -> Proof X W R x`, with X, W and R as arguments (:464) | R(x, w) (4.2, :400) |
| `run pi xs` | `run : (A : Type 0) -> (B : Type 0) -> Protocol A B -> A -> B` (:473) | f_pi(xs) (4.2, :401) |
| `verify cert` | `verify : (P : Policy) -> (Q : Policy) -> Log P Q -> Certificate Q -> EvList (polCom Q) -> Verdict` (:527) | `valid` exactly when the certificate denotes a valid step (4.2, :402) |

Checker facts are not user operations: the minimal accepting vectors
of each rule, the O4 margin, the O5 margin and the read-set size (PLAN
:404 to :411). `langc report` gives them (`docs/REPORT-JSON.md`).

## 7. Target and instance encoding

The compiler has four targets (PLAN section 8.2, :485 to :571). T1 is
the primary target.

| Target | Command | Output | Format |
|---|---|---|---|
| T1 | `langc [--kit] build PROG -o OUT` | One Wasm module for `authorize`, the rule facts, `policyAt` and the log replay (:487). `verify` does not lower: the evaluator runs it (O16) | The import ABI below |
| T2 | `langc [--kit] tfhe PROG` | The TFHE tally circuit for accept . count, with a static depth bound for each gate; `REFUSE_APPROX` for CKKS (:555) | JSON Lines in RFC 8785 form, `docs/TFHE-CIRCUIT.md` |
| T3 | `langc [--kit] noir PROG` | One Noir library text: a K global for each decided rule definition and the relations R_ballot and R_dec over BN254 G1 (:559) | `docs/NOIR-RELATIONS.md`; 3 Noir packages below noir/ |
| T4 | `langc [--kit] report PROG` | The policy facts, the minimal accepting vectors, the O4 and O5 margins, the read-set size, the log verdict at each height and the leak of each rule (:566) | JSON Lines in RFC 8785 form, `docs/REPORT-JSON.md` |

An instance is a top-level definition whose type is a data type (F14).

The T1 import ABI (S9; Q-S9-1, Q-S9-2, Q-S9-6 and Q-S9-8 (a), :501):
the module imports six functions from the import module `ratify`, and
import J is function J. Each parameter and each result is an i64. A
byte operand is two parameters, OFF and then LEN, in memory 0 (the
export `memory`). A word operand (j, t, n_j) is one parameter. The
host reads and writes each operand in the big-endian `_be` form of the
C code, and it reads all the operands before it writes an output. In
the guest, a byte operand is one argument of type `Bytes`, with
`def Bytes : Type 0 := Prod Nat Nat`, and a word operand is one `Nat`.
The five checks give `Decision` (the result is the tag: 0 refuses, 1
accepts), and `combine` gives `Nat`.

| J | Import | Operands (bytes) | Result | C body |
|---|---|---|---|---|
| 0 | `sig_verify` | pk 64, msg any, sig 96 | 1 good, else 0 | `schnorr_verify` |
| 1 | `hash_eq` | data any, want 32 | 1 equal, else 0 | `keccak256` of data, then compare with want |
| 2 | `share_verify` | part 232, key 64, ct 128 | 1 good, else 0 | `thr_verify_be` |
| 3 | `ballot_verify` | proof 384, pk 64, ct 128, ctx 32, word j | 1 good, else 0 | `sig_or_verify_be` |
| 4 | `tally_add` | out 128, a 128, b 128 | 1 written, else 0 | `tally_add_be` |
| 5 | `combine` | parts 232 x K, keys 64 x N, ct 128, word t, word n_j | count + 1, else 0 | `thr_open_be`, then the bound n_j |

K = LEN(parts) / 232 and N = LEN(keys) / 64. A LEN that is not a
multiple gives 0 (:521).

The host table (S9; Q-S9-3 and Q-S9-4 (a), :523): the `cryptotool`
test tool (`test/cryptotool.c`) writes `NAME ARGS RESULT` rows from
the real crypto. `langc eval --host FILE` and `test/run-wasm.mjs`
with `--host FILE` take the answer of each import from that table.
The key of a row is the guest words, not the bytes.

A module with imports has a host region [0, HOST_MEM) below its heap,
and HOST_MEM = 65536 (`src/hostmem.h`; S10, Q-S10-1 and Q-S10-2 (a)).
The host writes the operand bytes in the region before the call, and
an operand must lie in the region (:536 to :542).

ElGamal can only add (Q-S1-2 (a)). T1 sums the ballots of each class,
decrypts the count vector c and applies the rule in the clear, so the
T1 output leaks c (O5). The engine of `ratifyd` is an own C99
interpreter, a copy of the permit-lang engine (Q-S0-11 (a), :547).

Proofs at T1: the module checks the real ballot proofs, share proofs
and signatures through `ballot_verify`, `share_verify` and
`sig_verify` on real values. At T3 the relations R_ballot and R_dec
are Noir programs (Q-S0-13 (b), O11).

## 8. Host and target

The host is TinyCC. The repo has a `PIN` file (7 lines). It records
`tcc -v` 0.9.28rc (mob@0fb54300, AArch64 Darwin) and its path, `node`
v23.10.0, and `wasm-validate` and `wasm-objdump` 1.0.39. The gate uses
node and the wabt tools from /opt/homebrew/bin. `PIN` also records the
kit host check (make -C hosts/tcc-wasm check) GREEN on 2026-10-08.

The kernel is the Cubical TT kernel of permit-lang 3d47120 (S2, PLAN
section 8.1, :479). The crypto C in `src/crypto/` (S7) is a port of
quorum-lang be2c329 src/crypto, from intent-lang 79e1898 (PLAN :493).
It gives exponential ElGamal on BN254 G1, a Pedersen DKG, threshold
decryption with DLEQ proofs, disjunctive Chaum-Pedersen proofs and
keccak-256 hashes. The Schnorr signatures on BN254 G1 are new C
(Q-S7-3 (a), `src/crypto/schnorr.h`). `langc` does not link the
crypto C. `ratifyd`, `cryptotool` and `demotool` link it (`Makefile`
:26 to :37).

`host/ratifyd.c` is the C host. `ratifyd --call IMAGE FILE NAME
[ARGS...]` loads an operand image and a Wasm module, calls the named
entry with the word arguments and prints its numeric result (:208
to :242). It has no log-file or certificate-verdict command. For the
demo, `demotool MODULE DEMO.lang DK.txt` runs the Wasm entry `demo`
to get W, evaluates `demoV` in kit mode, and prints `W VERDICT`
(`test/demotool.c` :334 to :373). `verify` does not lower (O16).
`host/engine.c` and `host/wasmread.c` are a copy of the permit-lang
engine, plus `engine_bytes`, the bound check of a byte operand (PLAN
:547 to :554). `ratifyd`, `cryptotool` and `demotool` also link
`host/imports.c` (`Makefile` :28, :34, :37).

`docs/host/CAPABILITY.md` holds the host facts and the HOST-LIMIT
reasons. `probe/CAPABILITY.md` gives the S19 U2 probe runs and their
cost (its section 6). The host facts of the design come from the
former rows F1 to F15 of `formers/tcc-wasm.md` (section 3).

The checks on the output are in `make check` (`Makefile` :147). It
runs `test/gate.sh`, `test/review.sh` and the test tools, and it
checks the token counts of `docs/TOKEN-BASELINE.md` (:155 to :157).
`test/notraptool.c` (S3) gives the R0 check: each Wasm module of the
gate must give exit 0 and no output, and each row of
`test/notrap/bad/expect.txt` must give exit 1 and its message
(`test/gate.sh` :427 to :453). The golden rows compare the output
with a file byte for byte: the IR text of `test/ir/natops.ir` and
`test/ir/block.ir` (:362 to :366) and the T3 Noir text of
`test/noir/` (:784 to :800). The T2 rows check each circuit line
against the T4 rule line of the report (:776 to :783).
`make check-clang` (`Makefile` :128 to :145) compiles the C programs
again with clang, in 17 rows.

## 9. Open items

The open items are the OI rows of `docs/PLAN.md` section 13 (:783 to
:880). Item On is PLAN OI-n. The items O1 to O5 are the items of
design sec 13. Five items are closed: O6, O12, O13 (by Q-S5-3 (a),
PLAN :849), O14 and O15.

- O1. Committee loss can freeze the system (design sec 13). Recovery
  needs a clock. RULED 2026-11-03 (USER): Q-S0-15 (a), an open item,
  not a v1 feature. (PLAN OI-1.)
- O2. Stale certificates (design sec 13). `verify` takes the latest
  verified height as an input. The freshness rule is a later ruling.
  RULED 2026-11-03 (USER): Q-S0-15 (a). (PLAN OI-2.)
- O3. Agenda control (design sec 13). The log records the order only.
  RULED 2026-11-03 (USER): Q-S0-15 (a). (PLAN OI-3.)
- O4. Receipts (design sec 13). ZK and FHE give ballot secrecy, not
  receipt-freeness. RULED 2026-11-03 (USER): Q-S0-15 (a). (PLAN OI-4.)
- O5. The output leaks what the rule implies (design sec 13). At T1
  the output also leaks the count vector c (Q-S1-2 (a)). Each `rule`
  line of T4 gives the leak (Q-S11-6 (a)), and the T2 circuit gives
  only the accept bit (Q-S12-1 (a)). RULED 2026-11-03 (USER):
  Q-S0-15 (a). (PLAN OI-5.)
- O6. The kit row "a trap is equal to a trap" is gone: in the Wasm
  and differential rows a `trap` line is a FAIL. CLOSED at S3.
  (PLAN OI-6.)
- O7. The view equations hold only up to indistinguishability and need
  a computational model (design sec 13). They stay ASSUMED.
  (PLAN OI-7.)
- O8. The class-preserving group action is new Lean work on the UAT
  side (design sec 3). Lean export is not a target. (PLAN OI-8.)
- O9. Availability and the MPC tally route (design sec 9 and sec 10)
  are implementation choices. No target emits them in v1. (PLAN OI-9.)
- O10. The node and wasmtime oracles of the differential rows need
  the crypto imports (Q-S1-1 (a)). S9: node takes the answers from
  the host table (Q-S9-4 (a)). S10: the operand bytes are in the
  host region (section 7), and `ratifyd` gives the real import
  bodies. Open: wasmtime for a module with imports. (PLAN OI-10.)
- O11. The native field of Noir is the BN254 scalar field. A BN254 G1
  point has its coordinates in the base field, so the Noir R_ballot
  and R_dec need non-native field arithmetic (Q-S0-13 (b)). S13
  measured the cost. (PLAN OI-11.)
- O12. The gate gap from S2 to S3: `langc build` gave `REFUSE_LOWER`
  "target pending S3", and the gate had no IR rows, no Wasm modules
  and no differential cases. RULED 2026-11-03 (USER): Q-S2-1 (a),
  accept until S3. CLOSED at S3. (PLAN OI-12.)
- O13. `REFUSE_ORBIT` (S4) also walks the quoted type of a
  definition, so a definition that is not a law and has `count c S`
  in its type gives `REFUSE_ORBIT`. Probe-forced (S4 probe).
  RULED 2026-11-04 (USER): Q-S5-3 (a), keep the walk; no S5 rule puts
  `count c S` in such a type. CLOSED at S5. (PLAN OI-13.)
- O14. The O5 walk (S5) was quadratic in the minimal accepting
  vectors. RULED 2026-11-05 (USER): Q-S8-6 (b), one upward pass
  (S8), `rule_margins` in `src/front/check.c`. CLOSED at S8.
  (PLAN OI-14.)
- O15. The seal of `hoRule` and `ffRule` (S6, Q-S6-1 (a)) walks the
  written term. The walk respects the lambda, Pi and Sigma binders.
  CLOSED at S6 (the S6 review). (PLAN OI-15.)
- O16. `verify` does not lower: the kit lowering of `verify` gives a
  stuck elim with a function motive. The demo is split
  (Q-S10-U3-L (c)): the Wasm entry `demo` gives W, and `demoV` runs
  `verify` in the evaluator only. (PLAN OI-16.)
- O17. `demoV` on a large height word reaches the arena limit of the
  evaluator (`langc: OOM`, S10 mutant u8). `demotool` then gives
  exit 1, not `wrongHeight`. (PLAN OI-17.)
- O18. `make tokens` (the count_tokens route of A6) has no run in
  the repo: the USER runs it outside the sandbox with
  `ANTHROPIC_API_KEY` (Q-S19-3 (a)). (PLAN OI-18.)
- O19. OPEN-U2-2 of S17: the skill mutant M5 (the block skips the
  fmt check) has no kill of its own, and M6 alone and M7 alone are
  equivalent on a clean tree (Q-S19-3 (a)). (PLAN OI-19.)
- O20. FMT_DIFF: `test/demo/demo.lang`, `test/demo/policies.lang` and
  `test/report/log2.lang` are not `langc fmt` canonical (OBS-S17-1 of
  S17, Q-S19-6 (a)). S19 does not change the test files.
  (PLAN OI-20.)

## 10. Milestones

This section maps the slices to the kit milestones by content. PLAN
section 12 (:756) gives each slice and its gate. The PLAN names no
milestone, and `docs/STATUS.md` uses M0 for all plan work (:30, :177).

| Milestone | Slices | Content | Status |
|---|---|---|---|
| M0 | S0 to S3 | Host probe (`probe/CAPABILITY.md`); the plan, the rulings and the seed (S0, S1); the Cubical TT kernel from permit-lang 3d47120 (S2); R0 in the kernel and the Wasm writer, `REFUSE_PARTIAL`, `test/notraptool.c` (S3) | DONE (the probe record: S19 U2) |
| M1 | S4 to S8 | Core types and operations: `Party` to `count`, `REFUSE_ORBIT` (S4); `Rule`, `and`, `or`, O3 to O5 (S5); `Policy`, `authorize`, `Amendment`, `Log`, `System`, `policyAt` (S6); `Enc`, `Share`, `Proof`, `Protocol` and the crypto C (S7); `Certificate`, `Verdict`, `verify` (S8) | DONE |
| M2 | S9 to S13 | Targets: T1 the Wasm module and the crypto import ABI (S9); `ratifyd`, the Wasm engine and the demo D1 to D8 (S10); T4 the JSON report (S11); T2 the TFHE tally circuit (S12); T3 R_ballot and R_dec as Noir (S13) | DONE |
| A | S14 to S18 | Agent work: `langc fmt` (A1, S14), `langc check --json` (A2, S15), `langc mcp` (A4, S16), `skills/ratify-lang/SKILL.md`, `AGENTS.md` and the README table (A3, A5, S17), the token comparison (A6, S18) | DONE |
| M3 | none | Hardening: speed, limits, a checked certificate for the output | PARTIAL: the refusal codes, the mutant rows and the R0 check of `test/notraptool.c` give the limits; the faster O5 walk (S8, Q-S8-6 (b)) is speed work. No slice has the checked certificate |
| Close | S19 | SPEC.md, `probe/CAPABILITY.md`, `docs/VALIDATION.md`, the close gate | DONE |

Status 2026-11-12 (S19 U3): the slices S0 to S19 are done, and the
plan is CLOSED. M0, M1, M2 and the agent work are DONE, and M3 is
PARTIAL. Section 9 gives the open items.
