# ratify-lang domain design

> Paste the domain design below the line, verbatim. Do not edit it.

This file is the only source of the core types and the core operations of
ratify-lang. `SPEC.md` sections 5 and 6 cite its sections. Do not add a core
type or a core operation that this file does not give.

When the design changes, paste the new version. Then record the change as an
open item in `SPEC.md` section 9.

---

## 1. The request (USER, 2026-11-03, verbatim)

One change only: the paste had one trailing space after "criteria:".
That space is not here, because `git diff --check` refuses trailing
white space.

I want to write a new programming language with the following characteristics:

The language should not allow a single runtime error, period.

As far as type constructors/higher-kinded types, admit only cell monads and their duals as represented computationally in a cubical type theory.

For base types and operations, admit according to the following denotational design: https://claude.ai/artifact/Ge6sjsnVmXnwxY7Z5JqnDG

The host language shall be TinyCC.  Use your judgment to surface multiple-choice option(s) for the compilation target(s).

Finally, I want agents to reach for this language when it fits their use case(s).  To that end, adhere to the following additional criteria:

### 1.1 The pasted block (verbatim)

```
  - one canonical form per operation and a langc fmt canonical printer;
  - langc check --json, giving fix hints and progress counts (parsed, typed, laws
    discharged);
  - a SKILL.md with 3 to 5 graded examples;
  - an MCP server written in TinyCC (langc mcp);
  - AGENTS.md and a README table saying when to use the language;
  - a token-count comparison against Python and Rust.
```


## 2. The design (USER artifact, verbatim)

- File: `design/self-amending-threshold-authority.html`. It is the USER
  artifact, byte for byte. Do not edit it.
- Title: "Self-Amending Threshold Authority", a denotational design report
  dated 3 November 2026.
- Source: https://claude.ai/artifact/Ge6sjsnVmXnwxY7Z5JqnDG, version
  1793706339-dc0b, copied on 2026-11-03.
- sha256: 43f48f2f2fa2e28bd2c58c6cf053c382bbcfaa8aca4205e81ec6ee10bbf4b629
  (50702 bytes, 653 lines).
- The design uses UAT: https://github.com/MavenRain/unified-aggregation-theory
  at commit feb1089.
- Sections: 1 Summary, 2 Method, 3 What UAT supplies, 4 Semantic domains
  (the base types), 5 Operations (policy layer, cryptographic layer, view
  equations), 6 Type class morphisms TCM-A to TCM-E (the laws), 7 The three
  regimes, 8 Self-amendment, 9 Implementation (six stages, the ZK relations
  R_ballot and R_dec), 10 Off-chain consequences, 11 Proof obligations O1 to
  O11, 12 Lean sketch (Lean targets T1 to T4), 13 Open questions, 14 Sources.
- `docs/PLAN.md` cites the design as "design sec N". The Lean targets of
  design sec 12 are "Lean T1" to "Lean T4" there, because T1 to T4 are the
  compilation targets.

## 3. The S0 rulings (USER, 2026-11-03, AskUserQuestion)

- Name: ratify-lang (repo MavenRain/ratify-lang, private at creation; the USER flips it public).
- Primary target (single choice): Wasm module + TinyCC C host, from lang-template `hosts/tcc-wasm`. The policy, the tally rule, log replay and the certificate check compile to one Wasm module; a TinyCC C host runs it. Differential tests under node and wasmtime.
- Extra targets (later slices, multi choice): (a) FHE tally circuit: accept . count as a TFHE/BFV circuit file with a static depth bound (O6, TCM-C), CKKS refused; (b) ZK relation export: R_ballot and R_dec as constraint systems (Circom/R1CS or Noir, a Q item) (O8); (c) JSON report: policy facts, minimal accepting vectors, O4/O5 margins, read-set size, per-height log verdicts.
- NOT chosen: native C library, JS module, Boolean circuit IR as primary, Lean 4 export.
- Host: TinyCC (C compiled by tcc), for langc, the C host and `langc mcp`.

Options offered and not chosen are recorded here so no later slice re-asks them.

## 4. Overlap (checked 2026-11-03)

Sibling HEADs at S0: quorum-lang be2c329, orbit-lang 2162fb2, permit-lang
f9d8a15. The siblings give readings only.

- quorum-lang (USER ask 10-25): cubical TT DSL, R1 cell monads + duals, base = ThresholdVC Design.agda (threshold VC, FMSet ballots, FHE/MPC, ZK, IVC, CRDT), host TinyCC, kit tcc-wasm. Nearest sibling: ballots, FHE/MPC tally, ZK, same kit. Port its readings, cite SHA + file:line.
- orbit-lang (USER ask 10-16): cubical TT DSL for a ZK self-referential DAO, Lan along the voter-orbit quotient, target EVM bytecode, kit tcc-evm-contract. Nearest sibling for the orbit projection and self-reference. On-chain, so its target does not apply.
- permit-lang (USER ask 11-01): same R0 and R1 words, same agent-adoption block, kit tcc-wasm, an authoring MCP server and an enforcement host kept separate. Nearest sibling for the agent-adoption deliverables and the S0 shape.
- The USER asked for a NEW language, so ratify-lang is a new repo. Siblings give readings only.
