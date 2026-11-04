# ratify-lang

ratify-lang is a Cubical TT language for threshold authorities that amend
themselves. A policy is a committee in classes, an authorize rule, an amend
rule and a fault bound. A rule reads only the count of approvals in each
class, so each decision is one bit and does not depend on the names of the
parties. The checker proves the policy facts before any vote. The old amend
rule judges each new policy, one step at a time. A verifier host replays the
log and checks the certificates. No program can have a runtime error. The
design is a denotational design over Unified Aggregation Theory
(`design/DESIGN.md`).

ratify-lang comes from lang-template with the tcc-wasm host. Status: S4
(the committee words): the Cubical TT kernel of permit-lang 3d47120 and
R0 from permit-lang 18deea2: total Nat operations, `REFUSE_PARTIAL`, and
a Wasm writer that emits no trap form (`test/notraptool.c` checks each
built module). S4 adds the domain words `Committee`, `Party`,
`Approvals`, `Counts` and `count`, and the refusals `REFUSE_ORBIT` and
`REFUSE_BOUND`. The other ratify-lang types, refusals and targets do not
exist yet.
`docs/PLAN.md` gives the plan, and its section 14 gives the open
questions.

## When to reach for ratify-lang

DRAFT at S0. Slice S17 makes this table final.

| Use ratify-lang when | Use something else when |
|---|---|
| A group must approve an action by a threshold, and you must show before any vote that the rule is monotone, that each accepting set holds an honest party, and that two amendments cannot both pass at one height. | One party decides, or a plain access list is enough. |
| The rule for changing the rules is part of the policy, and the old rule must judge the new one. | The policy never changes, or an operator edits a config file. |
| Parties fall into classes (operators, auditors) and only the count in each class matters. A veto is a class with one party. | A rule must treat named parties in one group differently (`REFUSE_ORBIT`). |
| A verifier must replay a log of amendments and check certificates with no shared clock. | A decision depends on a wall-clock deadline (`REFUSE_CLOCK`). |
| Ballots are private (FHE tally, threshold decryption, ZK ballot proofs) and the tally must equal the meaning exactly. | You need a ranked choice over 3 or more options (`REFUSE_ARROW`) or approximate arithmetic such as CKKS (`REFUSE_APPROX`). |

## Files

- `design/DESIGN.md`: the request, the design source note and the S0
  rulings. `design/self-amending-threshold-authority.html` is the design,
  verbatim. It is the only source of the core types and the core operations.
- `docs/PLAN.md`: the readings, the route, the slices and the questions.
- `AGENTS.md`: the gate command, the layout and the rules for agents.
- `SPEC.md`: the specification. Slice S19 fills it.
- `formers/FORMERS.md`: the type formers. `formers/tcc-wasm.md` gives their
  status on the tcc-wasm host.
- `probe/CAPABILITY.md`: the host probe.
- `docs/STATUS.md` and `docs/VALIDATION.md`: the status and the gate results.
- `docs/host/README.md`: how to build and test the host kit, and how to
  replace the sample domain in `domain/`.
