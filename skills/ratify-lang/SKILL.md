---
name: ratify-lang
description: Write, check and repair ratify-lang programs (a Cubical TT language for threshold authorities that amend themselves) with the langc checker. Use it when the task has a committee in classes, a t-of-n approval rule, a veto class, an amend rule that judges each new policy, or the replay of a log of amendments.
---

# ratify-lang

## Routing rule

Use ratify-lang when the task is a threshold authority that amends
itself: a group approves an action by a rule that reads only the count
of approvals in each class of a committee. The checker proves the policy
facts before any vote: each decision is one bit (O2), each accepting set
holds an honest party (O4), and two amendments cannot both pass at one
height (O5). The old amend rule judges each new policy, one step at a
time. The examples are a plain t-of-n policy (G1), two classes and a
veto (G2), an amendment that changes the amend rule (G3), the replay of
a log of amendments (G4) and the demo of the verifier host (G5). No
program reads a clock: the time is the height of the log, and a wall
clock gives `REFUSE_CLOCK`. Use something else when one party decides or
a plain access list is enough, when the policy never changes, when a
rule must treat named parties in one class differently (`REFUSE_ORBIT`),
when the choice is a ranked choice over 3 or more options
(`REFUSE_ARROW`), when the arithmetic is approximate (`REFUSE_APPROX`),
and for general-purpose code. The README table (README.md, "When to
reach for ratify-lang") gives the yes and no rows.

## The loop

1. Write FILE.lang: one `def NAME : TYPE := TERM` for each value. For
   the names of the base, run `build/langc list-base`. A def of a `Path`
   type with the term `refl` is a law: the checker proves it. A kit item
   (`Nat`, its operations and the host imports such as `sig_verify`)
   needs the kit mode, `langc --kit`. G1 to G4 are guest files. G5 is a
   kit file (examples/skill/kit.txt).
2. Run `build/langc fmt FILE.lang --check`. No output and exit 0: the
   text is canonical. Exit 1 with `FMT_DIFF`: the text is not canonical.
   Then write the output of `build/langc fmt FILE.lang` to a new file,
   and replace FILE.lang with it. A line that has code and then a `--`
   comment gives `FMT_TRAILING_COMMENT`: put the comment on a line of
   its own. fmt does not check the program (docs/FMT.md).
3. Run `build/langc check FILE.lang --json` (`build/langc --kit check
   FILE.lang --json` for a kit file). The flag `--json` comes after the
   file. The command writes one JSON line on stdout, with the
   `diagnostics` array and the `progress` object (docs/CHECK-JSON.md).
   Exit 0: the checker accepts the file, and the array is empty. Exit 1:
   the checker refuses the file, and the array has the first failure.
   Exit 2: langc cannot read the file (`IO`), or the arguments are wrong
   (`USAGE`).
4. Read the `code`, the `message`, the `fix` and the `span` (the file,
   the line and the column) of the diagnostic. `build/langc explain
   CODE` gives one line: the code and its fix text. Change the file,
   then do the steps 2 and 3 again.
5. Run `build/langc eval FILE.lang NAME` (`build/langc --kit eval
   FILE.lang NAME` for a kit file) for the normal form of the def NAME,
   in one line. A `Decision` is a `Sum Unit Unit`: yes prints as `inr
   unit` and no prints as `inl unit`. `eval FILE.lang NAME ARGS...`
   gives the def NAME applied to the arguments (G5).

The command `build/langc mcp` gives the verbs to an agent as a Model
Context Protocol server on stdin and stdout (docs/MCP.md). Its 6 tools
are `check`, `fmt`, `explain`, `list-base`, `show-law` and `build`. The
`check` tool is `check PROG --json`. There is no `eval` tool and no `fmt
--check` tool: use the command line for those. With `--kit`, each tool
runs in the kit mode.

## Verbs

With no argument, `build/langc` writes a USAGE line and the usage text
on stderr, and exits with code 2:

```text
langc: USAGE: -: expected a command and a program path
usage: langc [--kit] check PROG [--json]
       langc [--kit] eval PROG NAME [ARGS...]
       langc [--kit] ir PROG
       langc [--kit] abi PROG [--text]
       langc [--kit] build PROG -o OUT [--runtime]
       langc [--kit] report PROG
       langc [--kit] tfhe PROG
       langc [--kit] noir PROG
       langc fmt PROG [--check]
       langc explain CODE
       langc [--kit] list-base
       langc show-law [ROW]
       langc [--kit] mcp
--kit: the kit mode, the trusted test scope of the kit gate
```

- `check PROG [--json]`: check PROG. The text mode gives `ok` on stdout,
  or one diagnostic line on stderr. With `--json`, it gives one JSON
  line on stdout (docs/CHECK-JSON.md).
- `eval PROG NAME [ARGS...]`: the normal form of the def NAME, in one
  line.
- `fmt PROG [--check]`: the canonical text of PROG on stdout
  (docs/FMT.md). With `--check`, no output, and exit 0 only when PROG is
  canonical.
- `explain CODE`: one line `CODE: FIX` with the fix text of the
  diagnostic code CODE. An unknown code gives exit 1.
- `list-base`: one line for each name of the base: the name, its kind
  and its place, for example `NAME def domain/domain.lang:LINE`.
- `show-law [ROW]`: one line for each law row, in the form `ROW:
  STATEMENT (STATUS, trigger T)`. The STATUS is `DISCHARGED`, `ASSUMED`,
  `AT TARGET` or `EXTERNAL`. With ROW (for example `O5`), it gives that
  row only.
- `mcp`: the MCP server (docs/MCP.md).
- `ir`, `abi`, `build`, `report`, `tfhe` and `noir`: the back end and
  the targets (docs/host/README.md). This skill does not use them.

## Examples

The examples go from easy to hard. Run each command from the repository
root. The block below each command is its stdout and its stderr. The
gate (test/skilltool.c) runs each command and compares the output and
the exit code with the text.

### G1

A plain t-of-n policy. The committee `fig` has one class of 4 parties,
and the rule `maj` is the bar of 3 approvals. `plOf` makes the policy
`p0` from the committee, the fault bound f = 1, the `Honest` value of
the authorize rule (O4) and the `ForkFree` value of the amend rule (O5).
`hoRule` and `ffRule` give the two values, and the checker computes the
margin of each one. `authorize` gives the decision of a policy on a set
of approvals: `okAuth` has 4 approvals, so it is yes (`inr unit`), and
`noAuth` has no approval, so it is no (`inl unit`). The defs `okIsYes`
and `noIsNo` are laws: the check proves each `Path` by `refl`. It is a
guest file, so the commands do not use `--kit`.

File: examples/skill/g1-threshold.lang

```lang
-- G1: a plain t-of-n policy. One class of 4 parties, the fault bound
-- f = 1, and the bar t = 3 for the authorize rule and the amend rule.
def one : Size := szS sz0
def two : Size := szS one
def three : Size := szS two
def four : Size := szS three
def yes : Decision := inr unit
def no : Decision := inl unit
-- The bar: the count 3 in a class of 4.
def k3 : Pos (szS four) := posNext four (posNext three (posNext two (posFirst one)))
def fig : Committee := cmClass four cmNil
def maj : Rule fig := rlMin fig (ctClass four cmNil k3 ctNil)
-- The policy: the committee, f = 1, the authorize rule, the amend rule.
def majHonest : Honest fig one := hoRule fig one maj
def majFork : ForkFree fig one := ffRule fig one maj
def p0 : Policy := plOf fig one majHonest majFork
-- Approval sets: all 4 parties, and no party.
def s4 : Approvals fig := apClass four cmNil (fun (p : Pos four) => yes) apNil
def s0 : Approvals fig := apClass four cmNil (fun (p : Pos four) => no) apNil
-- Each decision is yes or no: 4 approvals give yes, 0 give no.
def okAuth : Decision := authorize p0 Unit unit s4
def noAuth : Decision := authorize p0 Unit unit s0
def okIsYes : Path Decision okAuth yes := refl
def noIsNo : Path Decision noAuth no := refl
```

`build/langc check examples/skill/g1-threshold.lang` gives:

```text
ok
```

The exit code is 0.

`build/langc eval examples/skill/g1-threshold.lang okAuth` gives:

```text
inr unit
```

The exit code is 0.

`build/langc eval examples/skill/g1-threshold.lang noAuth` gives:

```text
inl unit
```

The exit code is 0.

### G2

Two classes and a veto. The committee `veto` has a class of 3 parties
and a veto class of 1 party. A `Counts` value has one count for each
class. `rlAnd` makes the rule `vetoRule` from two bars: 2 of the 3
parties, and the veto party. `accept` gives the decision of a rule on a
`Counts` value: `vetoNo` has the counts (2, 0), so it is no, and
`vetoYes` has the counts (2, 1), so it is yes. In the policy `pVeto`,
`okAuth` has the approval of all 4 parties, and `noAuth` does not have
the approval of the veto party. It is a guest file, so the commands do
not use `--kit`.

File: examples/skill/g2-veto.lang

```lang
-- G2: two classes and a veto. The committee has a class of 3 parties
-- and a veto class of 1 party. The rule is an and of 2 bars: 2 of the
-- 3 parties, and the veto party.
def one : Size := szS sz0
def two : Size := szS one
def three : Size := szS two
def yes : Decision := inr unit
def no : Decision := inl unit
def veto : Committee := cmClass three (cmClass one cmNil)
-- A Counts value has one count for each class: need2 = (2, 0),
-- needVeto = (0, 1), both = (2, 1).
def need2 : Counts veto :=
  ctClass three (cmClass one cmNil) (posNext three (posNext two (posFirst one))) (ctClass one cmNil (posFirst one) ctNil)
def needVeto : Counts veto :=
  ctClass three (cmClass one cmNil) (posFirst three) (ctClass one cmNil (posNext one (posFirst sz0)) ctNil)
def both : Counts veto :=
  ctClass three (cmClass one cmNil) (posNext three (posNext two (posFirst one))) (ctClass one cmNil (posNext one (posFirst sz0)) ctNil)
def vetoRule : Rule veto := rlAnd veto (rlMin veto need2) (rlMin veto needVeto)
-- The counts (2, 0) are not accepted: the veto party did not approve.
-- The counts (2, 1) are accepted.
def vetoNo : Decision := accept veto vetoRule need2
def vetoYes : Decision := accept veto vetoRule both
def vetoNoIsNo : Path Decision vetoNo no := refl
def vetoYesIsYes : Path Decision vetoYes yes := refl
-- The policy: f = 1, and vetoRule is the authorize rule and the amend
-- rule.
def vetoHonest : Honest veto one := hoRule veto one vetoRule
def vetoFork : ForkFree veto one := ffRule veto one vetoRule
def pVeto : Policy := plOf veto one vetoHonest vetoFork
-- Approval sets: all 4 parties, and the 3 parties without the veto
-- party.
def sAll : Approvals veto :=
  apClass three (cmClass one cmNil) (fun (p : Pos three) => yes) (apClass one cmNil (fun (p : Pos one) => yes) apNil)
def sNoVeto : Approvals veto :=
  apClass three (cmClass one cmNil) (fun (p : Pos three) => yes) (apClass one cmNil (fun (p : Pos one) => no) apNil)
def okAuth : Decision := authorize pVeto Unit unit sAll
def noAuth : Decision := authorize pVeto Unit unit sNoVeto
def okIsYes : Path Decision okAuth yes := refl
def noIsNo : Path Decision noAuth no := refl
```

`build/langc check examples/skill/g2-veto.lang` gives:

```text
ok
```

The exit code is 0.

`build/langc eval examples/skill/g2-veto.lang vetoNo` gives:

```text
inl unit
```

The exit code is 0.

`build/langc eval examples/skill/g2-veto.lang vetoYes` gives:

```text
inr unit
```

The exit code is 0.

`build/langc eval examples/skill/g2-veto.lang okAuth` gives:

```text
inr unit
```

The exit code is 0.

`build/langc eval examples/skill/g2-veto.lang noAuth` gives:

```text
inl unit
```

The exit code is 0.

### G3

An amendment that changes the amend rule. The policy `p0` has an amend
bar of 3 approvals in the class of 4, and the policy `p1` has an amend
bar of all 4. `amOf P Q S refl` is an amendment from `P` to `Q`: the
`refl` is the proof that the approvals `S` pass the amend rule of `P`,
the policy that the amendment replaces. `amendOk` gives that decision as
a value. The 3 approvals of `s3` pass the amend rule of `p0` (`by3P0`)
and do not pass the amend rule of `p1` (`by3P1`), and the 4 approvals of
`s4` pass it (`by4P1`). The checker refuses an amend rule that is too
weak. The last two rows check the file test/check/fork-even.lang and
explain its code: an amend bar of 2 approvals in a class of 4 with f = 0
has the O5 margin 0, because two accepting sets can have no shared
party. It is a guest file, so the commands do not use `--kit`.

File: examples/skill/g3-amend-rule.lang

```lang
-- G3: an amendment that changes the amend rule. The policy p0 has the
-- amend bar t = 3. The policy p1 has the amend bar t = 4. The
-- authorize bar stays t = 3.
def one : Size := szS sz0
def two : Size := szS one
def three : Size := szS two
def four : Size := szS three
def yes : Decision := inr unit
def no : Decision := inl unit
def k3 : Pos (szS four) := posNext four (posNext three (posNext two (posFirst one)))
def k4 : Pos (szS four) := posNext four (posNext three (posNext two (posNext one (posFirst sz0))))
def fig : Committee := cmClass four cmNil
def maj : Rule fig := rlMin fig (ctClass four cmNil k3 ctNil)
def all4 : Rule fig := rlMin fig (ctClass four cmNil k4 ctNil)
def majHonest : Honest fig one := hoRule fig one maj
def majFork : ForkFree fig one := ffRule fig one maj
def allFork : ForkFree fig one := ffRule fig one all4
def p0 : Policy := plOf fig one majHonest majFork
def p1 : Policy := plOf fig one majHonest allFork
-- Approval sets: 3 of 4 (no approval at the first position), and all 4.
def s3 : Approvals fig :=
  apClass four cmNil (fun (p : Pos four) => elim (fun (k : Size) (q : Pos k) => Sum Unit Unit) (fun (m : Size) => no) (fun (m : Size) (q : Pos m) (ih : Sum Unit Unit) => yes) p) apNil
def s4 : Approvals fig := apClass four cmNil (fun (p : Pos four) => yes) apNil
-- An amendment must pass the amend rule of the policy that it
-- replaces. p0 -> p1 passes with 3 approvals. After that step the
-- amend bar is t = 4, so p1 -> p0 needs all 4 approvals.
def am01 : Amendment p0 p1 := amOf p0 p1 s3 refl
def am10 : Amendment p1 p0 := amOf p1 p0 s4 refl
-- amendOk gives the decision of the amend rule. 3 approvals pass the
-- amend rule of p0. They do not pass the amend rule of p1.
def by3P0 : Decision := amendOk p0 s3
def by3P1 : Decision := amendOk p1 s3
def by4P1 : Decision := amendOk p1 s4
```

`build/langc check examples/skill/g3-amend-rule.lang` gives:

```text
ok
```

The exit code is 0.

`build/langc eval examples/skill/g3-amend-rule.lang by3P0` gives:

```text
inr unit
```

The exit code is 0.

`build/langc eval examples/skill/g3-amend-rule.lang by3P1` gives:

```text
inl unit
```

The exit code is 0.

`build/langc eval examples/skill/g3-amend-rule.lang by4P1` gives:

```text
inr unit
```

The exit code is 0.

`build/langc check test/check/fork-even.lang --json` gives:

```text
{"diagnostics":[{"code":"REFUSE_FORK","expected":null,"fix":"The O5 margin of the amend rule is 0 or less. Raise the amend thresholds until two minimal accepting vectors overlap in more than f members. If the message names ffRule, make ffRule the whole body of a closed def of type ForkFree c f.","found":null,"message":"half has an O5 margin of 0 or less (the least overlap of two minimal accepting vectors, minus f); rule O5","span":{"col":1,"file":"test/check/fork-even.lang","line":7}}],"progress":{"laws_assumed":0,"laws_at_target":0,"laws_discharged":0,"laws_external":0,"laws_total":0,"parsed":6,"typed":5}}
```

The exit code is 1.

`build/langc explain REFUSE_FORK` gives:

```text
REFUSE_FORK: The O5 margin of the amend rule is 0 or less. Raise the amend thresholds until two minimal accepting vectors overlap in more than f members. If the message names ffRule, make ffRule the whole body of a closed def of type ForkFree c f.
```

The exit code is 0.

### G4

The replay of a log with `policyAt`. The log `log012` has two
amendments, from `p0` to `p1` and from `p1` to `p2`, so it has the
heights 0, 1 and 2. `lgCons` puts one amendment before a log, and
`lgNil` is the empty log. `policyAt` gives the policy at a height: the
laws `at0`, `at1` and `at2` show the 3 policies, and the law `past`
shows that the policy stays `p2` after the end of the log. The policy
`p2` has a new committee of 2 classes. The amend rule at a height is the
amend rule of the policy at that height: the 3 approvals of `s3` pass at
the height 0 (`amend3At0`) and do not pass at the height 1
(`amend3At1`). It is a guest file, so the commands do not use `--kit`.

File: examples/skill/g4-replay.lang

```lang
-- G4: a log replay with policyAt. The log has 2 amendments,
-- p0 -> p1 -> p2, so it has the heights 0, 1 and 2. Step 1 has 3 of 4
-- approvals (the amend bar of p0 is t = 3). Step 2 has 4 of 4
-- approvals (the amend bar of p1 is t = 4).
def one : Size := szS sz0
def two : Size := szS one
def three : Size := szS two
def four : Size := szS three
def yes : Decision := inr unit
def no : Decision := inl unit
def k3 : Pos (szS four) := posNext four (posNext three (posNext two (posFirst one)))
def k4 : Pos (szS four) := posNext four (posNext three (posNext two (posNext one (posFirst sz0))))
def fig : Committee := cmClass four cmNil
def maj : Rule fig := rlMin fig (ctClass four cmNil k3 ctNil)
def all4 : Rule fig := rlMin fig (ctClass four cmNil k4 ctNil)
def majHonest : Honest fig one := hoRule fig one maj
def majFork : ForkFree fig one := ffRule fig one maj
def allFork : ForkFree fig one := ffRule fig one all4
def p0 : Policy := plOf fig one majHonest majFork
def p1 : Policy := plOf fig one majHonest allFork
-- The policy p2 has a committee of 2 classes: 2 of the class of 2 and
-- 2 of the class of 3.
def c2of2 : Pos (szS two) := posNext two (posNext one (posFirst sz0))
def c2of3 : Pos (szS three) := posNext three (posNext two (posFirst one))
def duoCom : Committee := cmClass two (cmClass three cmNil)
def duoBar : Rule duoCom :=
  rlMin duoCom (ctClass two (cmClass three cmNil) c2of2 (ctClass three cmNil c2of3 ctNil))
def duoHonest : Honest duoCom one := hoRule duoCom one duoBar
def duoFork : ForkFree duoCom one := ffRule duoCom one duoBar
def p2 : Policy := plOf duoCom one duoHonest duoFork
-- Approval sets: 3 of 4 (no approval at the first position), and all 4.
def s3 : Approvals fig :=
  apClass four cmNil (fun (p : Pos four) => elim (fun (k : Size) (q : Pos k) => Sum Unit Unit) (fun (m : Size) => no) (fun (m : Size) (q : Pos m) (ih : Sum Unit Unit) => yes) p) apNil
def s4 : Approvals fig := apClass four cmNil (fun (p : Pos four) => yes) apNil
-- The log: one entry for each amendment.
def am01 : Amendment p0 p1 := amOf p0 p1 s3 refl
def am12 : Amendment p1 p2 := amOf p1 p2 s4 refl
def log012 : Log p0 p2 := lgCons p0 p1 p2 am01 (lgCons p1 p2 p2 am12 (lgNil p2))
-- The replay: policyAt gives the policy at each height of the log.
def h0 : Pos three := posFirst two
def h1 : Pos three := posNext two (posFirst one)
def h2 : Pos three := posNext two (posNext one (posFirst sz0))
def at0 : Path Policy (policyAt p0 p2 log012 h0) p0 := refl
def at1 : Path Policy (policyAt p0 p2 log012 h1) p1 := refl
def at2 : Path Policy (policyAt p0 p2 log012 h2) p2 := refl
-- Past the end of the log the policy stays p2.
def past : Path Policy (sHead Policy (sTail Policy (sTail Policy (sTail Policy (systemOf p0 p2 log012))))) p2 :=
  refl
-- The amend rule at a height: the 3 approvals of s3 pass at height 0
-- (p0) and do not pass at height 1 (p1).
def amend3At0 : Decision := amendOk (policyAt p0 p2 log012 h0) s3
def amend3At1 : Decision := amendOk (policyAt p0 p2 log012 h1) s3
```

`build/langc check examples/skill/g4-replay.lang` gives:

```text
ok
```

The exit code is 0.

`build/langc eval examples/skill/g4-replay.lang amend3At0` gives:

```text
inr unit
```

The exit code is 0.

`build/langc eval examples/skill/g4-replay.lang amend3At1` gives:

```text
inl unit
```

The exit code is 0.

### G5

The demo of the verifier host. The file is the `langc fmt` text of
test/demo/demo.lang, and the gate holds that law. The committee has 3
operators and 1 auditor in a class of its own (a veto), and the log
`lDemo` has two amendments. The entry `demo` runs 13 host checks
(signatures, hashes and one share proof) and gives the flag word W: the
bit k is 1 when the check k holds, so a good set gives 8191. The entry
`demoV` decodes W and runs `verify`. Its arguments are W, the height,
the action, the amend flag, the target and the signers. Its result is
the verdict word: 0 is `valid`, and a number from 1 to 8 is the place of
the reason in the family `Reason` (5 is `badSignature` and 8 is
`equivocation`). In each row the height is 2, the action is 7, there is
no amendment and all 5 parties sign. The rows give the good set, the set
without the bit 0 (the first signature is bad) and the set without the
bit 7 (the evidence shows two different signed messages of one party).
It is a kit file (examples/skill/kit.txt), so each command uses `--kit`.

File: examples/skill/g5-demo.lang

```lang
-- The S10 demo (PLAN 8.4, Q-S10-U3-1 (a)), kit mode (Nat and the imports are kit items). Two classes:
-- three operators, and one auditor in a class of its own (a veto). f = 1.
-- pBase: authorize 2 of 3 operators and the auditor (O4 margin 3 - 1 = 2), amend
-- 3 of 3 and the auditor (O5 margin (3 + 3 - 3) + (1 + 1 - 1) - 1 = 3). pAmd1 sets
-- the authorize bar to 3 of 3. pAdd adds a fourth operator: authorize 2 of 4, amend
-- 3 of 4 (O5 margin (3 + 3 - 4) + 1 - 1 = 2). lDemo: pBase -> pAmd1 -> pAdd.
def one : Size := szS sz0
def two : Size := szS one
def three : Size := szS two
def four : Size := szS three
def yes : Decision := inr unit
def no : Decision := inl unit
-- The bars: a value of Pos (szS n) is a count 0 to n.
def c1of1 : Pos (szS one) := posNext one (posFirst sz0)
def c2of3 : Pos (szS three) := posNext three (posNext two (posFirst one))
def c3of3 : Pos (szS three) := posNext three (posNext two (posNext one (posFirst sz0)))
def c2of4 : Pos (szS four) := posNext four (posNext three (posFirst two))
def c3of4 : Pos (szS four) := posNext four (posNext three (posNext two (posFirst one)))
def aud : Committee := cmClass one cmNil
def dc : Committee := cmClass three aud
def dc4 : Committee := cmClass four aud
def auth2 : Rule dc := rlMin dc (ctClass three aud c2of3 (ctClass one cmNil c1of1 ctNil))
def all3 : Rule dc := rlMin dc (ctClass three aud c3of3 (ctClass one cmNil c1of1 ctNil))
def auth24 : Rule dc4 := rlMin dc4 (ctClass four aud c2of4 (ctClass one cmNil c1of1 ctNil))
def amend34 : Rule dc4 := rlMin dc4 (ctClass four aud c3of4 (ctClass one cmNil c1of1 ctNil))
def auth2Honest : Honest dc one := hoRule dc one auth2
def all3Honest : Honest dc one := hoRule dc one all3
def all3Fork : ForkFree dc one := ffRule dc one all3
def auth24Honest : Honest dc4 one := hoRule dc4 one auth24
def amend34Fork : ForkFree dc4 one := ffRule dc4 one amend34
def pBase : Policy := plOf dc one auth2Honest all3Fork
def pAmd1 : Policy := plOf dc one all3Honest all3Fork
def pAdd : Policy := plOf dc4 one auth24Honest amend34Fork
-- The log of two amendments. Each amendment has the approval of all parties of dc.
def allDc : Approvals dc :=
  apClass three aud (fun (p : Pos three) => yes) (apClass one cmNil (fun (p : Pos one) => yes) apNil)
def lDemo : Log pBase pAdd :=
  lgCons pBase pAmd1 pAdd (amOf pBase pAmd1 allDc refl) (lgCons pAmd1 pAdd pAdd (amOf pAmd1 pAdd allDc refl) (lgNil pAdd))
def rLen : Path Size (lgLen pBase pAdd lDemo) two := refl
-- A2: the entry demo (kit mode). The slot table is in the header of the entry.
-- flagDec: a Flag as a Decision. bitLow m: the low bit of m.
def flagDec : Flag -> Decision := fun (f : Flag) => flagIf f yes no
def bitLow : Nat -> Decision := fun (m : Nat) => flagDec (natEq (natMod m 2) 1)
def szOf : Nat -> Size :=
  fun (n : Nat) => fold (fun (acc : Size) => szS acc) sz0 (flagIf (natLe n 64) n 65)
def sigOk : Nat -> Nat -> Nat -> Decision :=
  fun (pk : Nat) (msg : Nat) (sg : Nat) => sig_verify (pair pk 64) (pair msg 48) (pair sg 96)
-- signedBy a d: when the party approves (a), its signature check d must hold.
def signedBy : Decision -> Decision -> Decision :=
  fun (a : Decision) (d : Decision) => sOr (sNot a) d
def party0 : Party dc4 := ptHere four aud (posFirst three)
def apDemo : Decision -> Decision -> Decision -> Decision -> Decision -> Approvals dc4 :=
  fun (a0 : Decision) (a1 : Decision) (a2 : Decision) (a3 : Decision) (a4 : Decision) => apClass four aud (consP three a0 (consP two a1 (consP one a2 (consP sz0 a3 (fun (q : Pos sz0) => no))))) (apClass one cmNil (consP sz0 a4 (fun (q : Pos sz0) => no)) apNil)
def toNone : Sum Unit Policy := inl unit
def toBase : Sum Unit Policy := inr pBase
def toAmd1 : Sum Unit Policy := inr pAmd1
def toAdd : Sum Unit Policy := inr pAdd
def targetOf : Nat -> Sum Unit Policy :=
  fun (t : Nat) => flagIf (natEq t 1) toBase (flagIf (natEq t 2) toAmd1 (flagIf (natEq t 3) toAdd toNone))
-- One evidence entry against party 0 at the height h. same: evB has the hash evH of evA.
def evDemo : Size -> Decision -> Decision -> Decision -> EvList dc4 :=
  fun (h : Size) (same : Decision) (sa : Decision) (sb : Decision) => evCons dc4 (evOf dc4 party0 h sz0 (either (fun (u : Unit) => one) (fun (u : Unit) => sz0) same) sa sb) (evNil dc4)
-- vdWord: valid 0, invalid r = 1 + the index of r (Reason order, domain.lang).
def rsWord : Reason -> Nat := fun (r : Reason) => elim (fun (q : Reason) => Nat) 1 2 3 4 5 6 7 8 r
def vdWord : Verdict -> Nat :=
  fun (v : Verdict) => elim (fun (w : Verdict) => Nat) 0 (fun (r : Reason) => rsWord r) v
-- A2' (Q-S10-U3-L (c), split runtimes): the Wasm entry demo runs the 13 host checks
-- only and gives the flag word W (bit k = check k). The entry demoV runs in the
-- evaluator only: it decodes W and runs verify. demoV gives Option Nat, so the
-- build does not lower it (a build entry has a Nat result).
-- addBit d p acc: acc + p when d is yes, else acc (no natAdd overflow: W < 8192).
-- acc occurs once: the kit lowering inlines acc, so a second copy doubles the body at
-- each of the 13 levels of demo (80229 locals, over the Wasm cap of 50000; now 182).
def addBit : Decision -> Nat -> Nat -> Nat :=
  fun (d : Decision) (p : Nat) (acc : Nat) => option 0 (fun (n : Nat) => n) (natAdd (either (fun (u : Unit) => 0) (fun (u : Unit) => p) d) acc)
-- bitOf w p: the bit of w at the power p, as a Decision.
def bitOf : Nat -> NonZero Nat -> Decision := fun (w : Nat) (p : NonZero Nat) => bitLow (natDiv w p)
-- The entry demo (Q-S10-U3-2 (a), A0 "OFF only"): 18 OFF words. Fixed slots (OFF LEN):
-- pk0..pk4 0 64 128 192 256 (64), body 320 (48, prev field 328 32), prev 368 (48),
-- sig0..sig4 416 512 608 704 800 (96), proof 896 (232 + 64 + 128: 896, 1128, 1192),
-- evA 1320 (48, prev field 1328), evB 1368 (48, prev field 1376), evH 1416 (32),
-- sigA 1448, sigB 1544 (96). The sub-offsets are literals of the table; each host call
-- is open on an OFF word. The bits of W: k 0..4 sig_i on body, k 5 prev hash (328),
-- k 6 share_verify, k 7 evB hash = evH, k 8 sigA on evA, k 9 evA prev (1328),
-- k 10 evA hash = evH, k 11 sigB on evB, k 12 evB prev (1376). A good set gives 8191.
def demo : Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Nat :=
  fun (pk0 : Nat) (pk1 : Nat) (pk2 : Nat) (pk3 : Nat) (pk4 : Nat) (body : Nat) (prev : Nat) (s0 : Nat) (s1 : Nat) (s2 : Nat) (s3 : Nat) (s4 : Nat) (proof : Nat) (evA : Nat) (evB : Nat) (evH : Nat) (sigA : Nat) (sigB : Nat) => addBit (sigOk pk0 body s0) 1 (addBit (sigOk pk1 body s1) 2 (addBit (sigOk pk2 body s2) 4 (addBit (sigOk pk3 body s3) 8 (addBit (sigOk pk4 body s4) 16 (addBit (hash_eq (pair prev 48) (pair 328 32)) 32 (addBit (share_verify (pair proof 232) (pair 1128 64) (pair 1192 128)) 64 (addBit (hash_eq (pair evB 48) (pair evH 32)) 128 (addBit (sigOk pk0 evA sigA) 256 (addBit (hash_eq (pair prev 48) (pair 1328 32)) 512 (addBit (hash_eq (pair evA 48) (pair evH 32)) 1024 (addBit (sigOk pk0 evB sigB) 2048 (addBit (hash_eq (pair prev 48) (pair 1376 32)) 4096 0))))))))))))
-- The entry demoV (evaluator only): W, then height, action, amend, target, signers.
-- Each import call of the RESUME12 demo body is now its bit of W.
def demoV : Nat -> Nat -> Nat -> Nat -> Nat -> Nat -> Option Nat :=
  fun (w : Nat) (height : Nat) (action : Nat) (amend : Nat) (target : Nat) (signers : Nat) => some (vdWord (verify pBase pAdd lDemo (certOf pAdd (szOf height) (szOf action) (apDemo (bitLow signers) (bitLow (natDiv signers 2)) (bitLow (natDiv signers 4)) (bitLow (natDiv signers 8)) (bitLow (natDiv signers 16))) (flagDec (natLe 1 amend)) (targetOf target) (flagDec (natLe amend 1)) (flagDec (natLe target 3)) (bitOf w 32) (sAnd (signedBy (bitLow signers) (bitOf w 1)) (sAnd (signedBy (bitLow (natDiv signers 2)) (bitOf w 2)) (sAnd (signedBy (bitLow (natDiv signers 4)) (bitOf w 4)) (sAnd (signedBy (bitLow (natDiv signers 8)) (bitOf w 8)) (signedBy (bitLow (natDiv signers 16)) (bitOf w 16)))))) (bitOf w 64)) (evDemo (szOf height) (bitOf w 128) (sAnd (bitOf w 256) (sAnd (bitOf w 512) (bitOf w 1024))) (sAnd (bitOf w 2048) (bitOf w 4096)))))
```

`build/langc --kit check examples/skill/g5-demo.lang` gives:

```text
ok
```

The exit code is 0.

`build/langc --kit eval examples/skill/g5-demo.lang demoV 8191 2 7 0 0 31` gives:

```text
0
```

The exit code is 0.

`build/langc --kit eval examples/skill/g5-demo.lang demoV 8190 2 7 0 0 31` gives:

```text
5
```

The exit code is 0.

`build/langc --kit eval examples/skill/g5-demo.lang demoV 8063 2 7 0 0 31` gives:

```text
8
```

The exit code is 0.
