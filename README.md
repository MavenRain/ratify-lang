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

ratify-lang comes from lang-template with the tcc-wasm host. Status: S17
(the skill and the agent docs): the Cubical TT kernel of permit-lang
3d47120 and R0 from permit-lang 18deea2: total Nat operations,
`REFUSE_PARTIAL`, and a Wasm writer that emits no trap form
(`test/notraptool.c` checks each built module). S4 adds the domain words
`Committee`, `Party`, `Approvals`, `Counts` and `count`, and the
refusals `REFUSE_ORBIT` and `REFUSE_BOUND`. S5 adds the rule words
`Rule`, `accept`, `Honest` and `ForkFree`, and the refusals
`REFUSE_RULE`, `REFUSE_HONEST` and `REFUSE_FORK`. S6 adds the policy
words `Policy`, `Decision`, `authorize`, `Amendment`, `Log`, `System`
and `policyAt`, and the refusals `REFUSE_ARROW`, `REFUSE_CLOCK` and
`REFUSE_FIXPOINT`. S7 adds the crypto words `Enc`, `eval`, `Share`,
`Proof` and `Protocol`, the refusal `REFUSE_APPROX`, and `src/crypto/`
(the crypto C of quorum-lang and Schnorr on BN254 G1) with the test tool
`build/cryptotool`. S8 adds the certificate words `Certificate`,
`Verdict`, `Reason`, `verify`, `Evidence` and `equivocates`, and a
faster O5 pass. S9 adds the first part of the target T1: the Wasm module
and the crypto import ABI. S10 adds `build/ratifyd`, its Wasm engine
(`host/`) and the demo (`test/demo/`): `build/cryptotool demo DIR`
writes the cases D1 to D8, and `build/demotool MODULE DEMO.lang DK.txt`
prints the flag word W and the verdict of one case (`docs/PLAN.md`
section 8.4). S11 adds `langc report PROG`, the T4 JSON report
(`docs/REPORT-JSON.md`). S12 adds `langc tfhe PROG`, the T2 TFHE tally
circuit (`docs/TFHE-CIRCUIT.md`). S13 adds `langc noir PROG`, the T3
relations R_ballot and R_dec as Noir (`docs/NOIR-RELATIONS.md`), and the
Nargo packages in `noir/`. S14 adds `langc fmt PROG [--check]`, the
canonical printer (`docs/FMT.md`). S15 adds `langc check PROG --json`,
the JSON report of the checker (`docs/CHECK-JSON.md`). S16 adds
`langc mcp`, the MCP server (`docs/MCP.md`), and the verbs
`langc explain CODE`, `langc list-base` and `langc show-law [ROW]`. S17
adds the skill `skills/ratify-lang/SKILL.md`, its 5 graded examples G1
to G5 (`examples/skill/`), the test tool `build/skilltool`, the table
below and the final `AGENTS.md`. The next slice is S18: the token
comparison with Python and Rust (A6). S19 closes the plan.
`docs/PLAN.md` gives the plan, and its section 14 gives the open
questions.

## When to reach for ratify-lang

The skill `skills/ratify-lang/SKILL.md` gives the routing rule, the loop
and the 5 graded examples (G1 to G5), and `AGENTS.md` gives the commands
and the rules for a change.

| Task | ratify-lang? | Use instead |
| --- | --- | --- |
| A plain t-of-n policy: a group approves an action by a rule that reads only the count of approvals, and the checker proves the policy facts before any vote (G1) | yes | |
| A committee with two classes and a veto: the rule reads the count of approvals in each class (G2) | yes | |
| An amendment that changes the amend rule: the old amend rule judges each new policy, one step at a time (G3) | yes | |
| The replay of a log of amendments: the time is the height of the log, and no program reads a clock (G4) | yes | |
| The demo of the verifier host `ratifyd`: a kit file that uses the host imports, for example `sig_verify` (G5) | yes | |
| One party decides, or a plain access list is enough | no | An access list or plain authorization rules, for example Cedar |
| The policy never changes, or an operator edits a config file | no | A fixed multi-signature rule, or the config file |
| A rule must treat named parties in one class differently (`REFUSE_ORBIT`) | no | A rule language with named principals, for example Cedar |
| A decision depends on a wall-clock deadline (`REFUSE_CLOCK`) | no | A system with a trusted clock. In ratify-lang the time is the height of the log |
| A ranked choice over 3 or more options (`REFUSE_ARROW`), or approximate arithmetic such as CKKS (`REFUSE_APPROX`) | no | A ranked-choice voting system, or an FHE library with an approximate scheme |

## Files

- `design/DESIGN.md`: the request, the design source note and the S0
  rulings. `design/self-amending-threshold-authority.html` is the design,
  verbatim. It is the only source of the core types and the core operations.
- `docs/PLAN.md`: the readings, the route, the slices and the questions.
- `AGENTS.md`: the routing rule, the loop, the commands and the rules
  for agents.
- `skills/ratify-lang/SKILL.md`: the skill for an agent: the routing
  rule, the loop, the verbs and the graded examples G1 to G5.
- `examples/skill/`: the 5 example files of the skill, and `kit.txt`,
  the list of the kit files.
- `test/skill/` and `test/skilltool.c`: the 6 negative fixtures and the
  test tool `build/skilltool`. The gate lines of the skill are
  `skill checks: 5 of 5`, `skill quotes: 5 of 5`,
  `skill outputs: 21 of 21` and `skill negatives: 6 of 6`.
- `SPEC.md`: the specification. Slice S19 fills it.
- `formers/FORMERS.md`: the type formers. `formers/tcc-wasm.md` gives their
  status on the tcc-wasm host.
- `probe/CAPABILITY.md`: the host probe.
- `docs/STATUS.md` and `docs/VALIDATION.md`: the status and the gate results.
- `docs/host/README.md`: how to build and test the host kit, and how to
  replace the sample domain in `domain/`.
