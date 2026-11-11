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
def yes : Decision := inl unit
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
