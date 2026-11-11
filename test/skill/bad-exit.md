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

The exit code is 1.

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
