# AGENTS.md

Instructions for agents that work in this repository. DRAFT at S0. Slice
S17 makes this file final.

## What ratify-lang is

A Cubical TT language for threshold authorities that amend themselves. See
the README table "When to reach for ratify-lang" before you pick it for a
task.

## Gate

    make -C /path/to/ratify-lang check

The gate must exit 0. For a log that you keep, run it serial (no `-j`) and
append `EXIT $?` to the log.

## Layout

- `design/`: the request and the design, verbatim. Do not edit
  `design/self-amending-threshold-authority.html`.
- `docs/PLAN.md`: the plan. Section 1 holds the rulings. Section 14 holds
  the open questions.
- `src/`: `langc` (parser, checker, evaluator, lowering, Wasm writer).
- `domain/`: the domain sample of the kit. Later slices replace it.
- `examples/`, `test/`: the gate rows.
- `formers/`, `probe/`: the type formers and the host probe.

## Rules

- R0: no program can trap. A rejection is a value (`false`, or
  `invalid Reason`). Do not add a partial operation.
- R1: a type constructor is a cell monad or its dual. Any other former is
  refused.
- The core types and operations come only from the design. Do not add one
  that `docs/PLAN.md` section 4 does not list.
- Time is the log height. Do not add a clock.
- `langc mcp` (the authoring server) and `ratifyd` (the verifier host) are
  two programs. Do not merge them.
- No em-dash or en-dash in any file. The gate fails on one.
- Sign each commit (`git commit -s`).
