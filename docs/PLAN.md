# ratify-lang plan

ratify-lang is a Cubical TT language for one job: threshold authorities that
amend themselves. A policy is a committee in classes, an authorize rule, an
amend rule and a fault bound. A rule reads only the count of approvals in
each class. Each decision is one bit. An amendment changes the policy one
step at a time, and the old amend rule judges the new policy. The checker
proves the policy facts before any vote: the rule is monotone, each
accepting set holds an honest party, and two accepting sets of the amend
rule overlap in more than f parties. A verifier host replays the log and
checks the certificates. No program can have a runtime error.

The design is `design/DESIGN.md`: the request, verbatim, the design source
note, the S0 rulings and the overlap. The design itself is
`design/self-amending-threshold-authority.html`, verbatim. This file gives
the readings of the request, the route, the slices and the open questions.
"design sec N" is section N of the design. Section 14 lists the questions
for the USER. A reading that is not ruled stays a recommendation.

Status: S9 (the Wasm module and the crypto import ABI): the Cubical TT kernel of permit-lang
3d47120 and R0 from permit-lang 18deea2 are in, without the affine layer
and the permit domain (Q-S0-10 (a), Q-S2-1 (a)). The Wasm writer emits no
trap form. The domain has `Committee`, `Party`, `Approvals`, `Counts`,
`count`, `Rule`, `accept`, `Honest`, `ForkFree`, `Policy`, `Decision`,
`authorize`, `Amendment`, `Log`, `System`, `policyAt`, `Base`, `El`,
`Enc`, `eval`, `Share`, `Proof`, `Protocol`, `Reason`, `Verdict`,
`Evidence`, `EvList`, `equivocates`, `Certificate`, `verify` and `Bytes`, and the
kernel gives `REFUSE_ORBIT`, `REFUSE_BOUND`, `REFUSE_RULE`,
`REFUSE_HONEST`, `REFUSE_FORK`, `REFUSE_FIXPOINT`, `REFUSE_ARROW`,
`REFUSE_CLOCK` and `REFUSE_APPROX` (Q-S4-1 to Q-S4-4 (a), Q-S5-1 to
Q-S5-3 (a), Q-S6-1 to Q-S6-4 (a), Q-S7-1 to Q-S7-6 (a), Q-S8-1 to Q-S8-5
(a), Q-S8-6 (b), Q-S9-1 to Q-S9-6 and Q-S9-8 (a), Q-S9-7 not needed,
Q-S10-1 to Q-S10-7, Q-S10-U2-1 to Q-S10-U2-4, Q-S10-U3-1 to Q-S10-U3-4
and Q-S10-U3-6 (a), Q-S10-U3-L (c), Q-S11-1 to Q-S11-10 (a), Q-S12-1 to
Q-S12-8 (a)).
`src/crypto/` has the crypto C of quorum-lang and
Schnorr on BN254 G1, and `build/cryptotool` runs the crypto rows and the
certificate rows, and writes the host table. The T1 module calls six
host imports of module `ratify` (section 8.2). Section 1 has the rulings.
S10 adds `ratifyd`, the Wasm engine of `host/` and the demo of section
8.4 (`build/demotool`). S11 adds the T4 report `langc report`
(`docs/REPORT-JSON.md`). S12 adds the T2 TFHE tally circuit `langc
tfhe` (`docs/TFHE-CIRCUIT.md`). Next: S13 (section 12).

## 1. Rulings

| Date | Ruling | Source |
|---|---|---|
| 2026-11-03 | Name: ratify-lang. Repo MavenRain/ratify-lang, private at creation. The USER makes it public. | DESIGN.md 3 |
| 2026-11-03 | T1 (primary): a Wasm module and a TinyCC C host, from the lang-template `hosts/tcc-wasm` kit. The policy, the tally rule, the log replay and the certificate check compile to one Wasm module. A C host runs it. Differential tests under node and wasmtime. | DESIGN.md 3 |
| 2026-11-03 | Extra targets (later slices): T2 FHE tally circuit (TFHE or BFV, a static depth bound, CKKS refused), T3 ZK relation export (R_ballot, R_dec), T4 JSON report. | DESIGN.md 3 |
| 2026-11-03 | Not chosen: native C library, JS module, Boolean circuit IR as primary target, Lean 4 export. | DESIGN.md 3 |
| 2026-11-03 | Host: TinyCC. `langc`, `langc mcp` and the T1 host are C99 that TinyCC builds. | DESIGN.md 1, 3 |
| 2026-11-03 | Q-S0-1 (a) "Verdict + Reason (Recommended)": `verify` gives `Verdict = valid \| invalid Reason` with the eight reasons of section 2. | USER, S1 |
| 2026-11-03 | Q-S0-2 (a) "Quorum t in type (Recommended)": `reconstruct` takes a `Quorum t` value; the type carries \|T\| >= t (section 2). | USER, S1 |
| 2026-11-03 | Q-S0-3 (a) "Fin (length + 1) (Recommended)": the height is `Fin (length + 1)` of a given log, so `policyAt` is total (sections 2, 4.1). | USER, S1 |
| 2026-11-03 | Q-S0-4 (a) "REFUSE_BOUND > 2^20 (Recommended)": `REFUSE_BOUND` when the product of (n_j + 1) is above 2^20 (sections 2, 7). | USER, S1 |
| 2026-11-03 | Q-S0-5 (a) "Log CM, System CC (Recommended)": the former table of section 3; TCM-D is ASSUMED. | USER, S1 |
| 2026-11-03 | Q-S0-6 (a) "Eleven codes (Recommended)": the eleven codes of section 7. | USER, S1 |
| 2026-11-03 | Q-S0-7 (a) "Thresholds, and, or, tables (Recommended)": a threshold for each class, `and`, `or`, and a table of count vectors; the checker computes the minimal accepting vectors (section 4.1). | USER, S1 |
| 2026-11-03 | Q-S0-8 (a) "Four buckets (Recommended)": the four counts of section 6; `laws_discharged` counts DISCHARGED rows only. | USER, S1 |
| 2026-11-03 | Q-S0-9 (b) "Real crypto in v1": real crypto in the v1 module, the quorum-lang exponential ElGamal route (quorum-lang be2c329 docs/PLAN.md sec 4.2, line 392). Not the recommendation. Edits: section 2 (the crypto imports are total), 3 (the `Enc` and `Share` representation), 4.1 (the crypto sorts, `Certificate`), 6 (TCM-C, View, O7, O8), 8.2 (T1), 10 (crypto rows), 11 (quorum-lang row), 12 (S7, S9), 13 (OI-10). Q-S1-1 to Q-S1-3 come from this ruling. | USER, S1 |
| 2026-11-03 | Q-S0-10 (a) "permit-lang kernel (Recommended)": the permit-lang kernel without the affine layer (sections 3, 8.1, 11, 12 S2). | USER, S1 |
| 2026-11-03 | Q-S0-11 (a) "Own C99 interpreter (Recommended)": the own C99 Wasm interpreter of the relay-lang ruling, a copy of the permit-lang engine; node and wasmtime are oracles in the gate (section 8.2). | USER, S1 |
| 2026-11-03 | Q-S0-12 (a) "TFHE (Recommended)": T2 is TFHE; the depth bound is per gate (sections 8.2, 12 S12). | USER, S1 |
| 2026-11-03 | Q-S0-13 (b) "Noir": T3 exports R_ballot and R_dec as Noir. Not the recommendation. Edits: section 6 (O8), 8.2 (T3), 10 (golden files), 12 (S13), 13 (OI-11). | USER, S1 |
| 2026-11-03 | Q-S0-14 (a) "permit-lang ruling (Recommended)": A6 follows the permit-lang ruling (count_tokens, equal outputs) (section 9). | USER, S1 |
| 2026-11-03 | Q-S0-15 (a) "Open items, not v1 (Recommended)": the items of design sec 13 stay open items OI-1 to OI-5, not v1 features (section 13). | USER, S1 |
| 2026-11-03 | Q-S1-1 (a) "Host-import ABI (Recommended)": the crypto C code (a port of quorum-lang src/crypto) lives in `ratifyd`; the module calls it through a fixed host-import ABI on real values, not opaque handles (the quorum-lang Q9 (a) ruling). Edits: section 2, 8.2, 11, 12 (S7, S9), 13 (OI-10). | USER, S1 |
| 2026-11-03 | Q-S1-2 (a) "Counts c at T1 (Recommended)": T1 sums the ballots of each class, decrypts the count vector c and applies the rule in the clear. The T1 output leaks c, not only b. T2 gives the one-bit output of design sec 9 stage 3. Edits: section 6 (View rows), 8.2 (T1, T4), 13 (OI-5). | USER, S1 |
| 2026-11-03 | Q-S1-3 (a) "Schnorr BN254 + keccak (Recommended)": Schnorr signatures on BN254 G1 with a keccak-256 challenge; keccak-256 for the entry hash. Edits: section 4.1 (`Certificate`), 8.2, 12 (S7). | USER, S1 |
| 2026-11-03 | Q-S2-1 (a) "Accept until S3 (Recommended)": S2 ports permit-lang 3d47120, the kernel without R0, the affine layer and the domain words. lower.c, wasm.c and ir.h stay out of the build until S3. `langc build` gives `REFUSE_LOWER` "target pending S3". The IR rows, the Wasm modules and the differential cases come back at S3 (the permit-lang Q-S2-3 (a) ruling). Edits: sections 11, 12, 13. | USER, S2 |
| 2026-11-04 | Q-S4-1 (a) "Domain families": `Committee`, `Party`, `Approvals` and `Counts` are families in `domain/domain.lang`, not kernel types. `posWeaken`, `countBits` and `count` are domain definitions. G_P is derived: `Perm` gives one word of adjacent swaps for each class, and `actP` applies the words to an approval set. Edits: section 12. | USER, S4 |
| 2026-11-04 | Q-S4-2 (a) "Seal Approvals": a program definition that is not a law and reads `Approvals` (a fold, an elim or a projection in the syntax type, the quoted type or the body) gives `REFUSE_ORBIT`. A program uses the domain definitions `count` and `actP`. Edits: section 12. | USER, S4 |
| 2026-11-04 | Q-S4-3 (a) "Def types + Committee defs": `REFUSE_BOUND` checks the type of each program definition and each definition of type `Committee`. A type that names `Party`, `Approvals` or `Counts` at a closed committee whose count space (the product of n_j + 1) is above 2^20 is refused, and so is a `Committee` value with such a space. An open committee is not refused. Edits: section 12. | USER, S4 |
| 2026-11-04 | Q-S4-4 (a) "By construction + refl rows": TCM-A holds by construction: a `Perm` has no word across two classes, and `count` counts each class. `examples/committee.lang` has the closed `refl` rows `tcmSwap`, `tcmLift` and `tcmVeto` (`count c (actP c w S) = count c S`), and `figThree` and `vetoTwo` pin the counts 3 of 4 and (2, 0). Edits: section 12. | USER, S4 |
| 2026-11-04 | Q-S5-1 (a) "Domain family, exact table": `Rule c` is a family in `domain/domain.lang`: `rlMin` (accept when the count is at or above the bar `rlBar : Counts c` in each class), `rlAnd`, `rlOr`, and `rlTable` (accept exactly the count vectors in `rlRows : CtTable c`). `accept` is a domain definition. Edits: section 12. | USER, S5 |
| 2026-11-04 | Q-S5-2 (a) "Families indexed by f": `Honest c f` and `ForkFree c f` are families on a committee and the fault bound `f : Size`, each with one rule. S6 `Policy c f` holds one `Honest` and one `ForkFree`. Edits: section 12. | USER, S5 |
| 2026-11-04 | Q-S5-3 (a) "Keep the walk": `REFUSE_ORBIT` keeps the walk of the quoted type (OI-13 (a)). No S5 rule puts `count c S` in the type of a definition that is not a law. Edits: sections 12, 13. | USER, S5 |
| 2026-11-04 | Q-S6-1 (a) "Fields + seal": `Policy` is one family `plOf` with the fields `plCom`, `plF`, `plAuth : Honest plCom plF` and `plAmend : ForkFree plCom plF`. `hoRule` and `ffRule` can only be the whole body of a closed decided definition, else `REFUSE_HONEST` or `REFUSE_FORK`. Edits: sections 12, 13. | USER, S6 |
| 2026-11-04 | Q-S6-2 (a) "Rename": the kit codes `REFUSE_TIME` and `REFUSE_REC` become `REFUSE_CLOCK` and `REFUSE_FIXPOINT`, and `deadline`, `timeout` and `expires` are clock words too. Edits: section 12. | USER, S6 |
| 2026-11-04 | Q-S6-3 (a) "Result type": `REFUSE_ARROW` refuses a guest definition whose type takes `Approvals` or `Counts` and gives a closed choice of three or more values. Edits: section 12. | USER, S6 |
| 2026-11-04 | Q-S6-4 (a) "Stream Policy": `System` is `Stream Policy`, and `systemOf` builds it by `sCorec` at a seed `Size`. Edits: sections 3, 12. | USER, S6 |
| 2026-11-05 | Q-S7-1 (a) "Code sort Base + El": `Enc` and `Share` are families on the code sort `Base` (`baseBool`, `baseCounts c`), and `El` gives the type of a code. The denotation of each crypto word is the value, so eval needs no crypto. Edits: section 12. | USER, S7 |
| 2026-11-05 | Q-S7-2 (a) "20-file closure, byte equal": `src/crypto/` has the 20 files of quorum-lang src/crypto (keccak, u256, bn254, elgamal, threshold, sigma, dkg, secp256k1, xmd, sha256), byte equal, and adds `schnorr.c` and `schnorr.h`. Edits: section 12. | USER, S7 |
| 2026-11-05 | Q-S7-3 (a) "(R, s), 96 bytes": a Schnorr signature on BN254 G1 is the point R (64 bytes) and the scalar s (32 bytes). Verify refuses s at or above the group order r. Edits: section 12. | USER, S7 |
| 2026-11-05 | Q-S7-4 (a) "Tool rows + docs": the ASSUMED laws stay ASSUMED. `build/cryptotool` tests the scheme with rows, and `docs/STATUS.md` gives the rows of each ASSUMED law. Edits: section 12. | USER, S7 |
| 2026-11-05 | Q-S7-5 (a) "APPROX_WORDS name list": `REFUSE_APPROX` refuses the names `ckks`, `approx`, `approximate`, `float` and `double` at the four sites of the clock check (after `REFUSE_CLOCK`), with the hint "an exact scheme only". Edits: section 12. | USER, S7 |
| 2026-11-05 | Q-S7-6 (a) "Trunc1, witness exists": `Proof X W R x` is `Trunc1 (Sigma (w : W) (R x w))`: a witness exists, and the proof does not show it. `prove` gives `trunc1` of the pair. Edits: section 12. | USER, S7 |
| 2026-11-05 | Q-S8-1 (a) "checked form": `Certificate P` holds the height, the action, the signer set, b, P' and one `Decision` for each host byte check (well formed, well typed, prev hash, signatures, proofs). Edits: section 12. | USER, S8 |
| 2026-11-05 | Q-S8-2 (a) "verify P Q l c": `verify` takes `l : Log P Q` and `c : Certificate Q`, and gives `wrongHeight` when the height of c is not `lgLen l`. Edits: section 12. | USER, S8 |
| 2026-11-05 | Q-S8-3 (a) "bytes, link, crypto, rule": the first failure wins, in the order `malformed`, `illTypedPolicy`, `wrongHeight`, `badPrevHash`, `badSignature`, `badProof`, `notAccepted`, `equivocation`. Edits: section 12. | USER, S8 |
| 2026-11-05 | Q-S8-4 (a) "Evidence family": `Evidence C`, the decision `equivocates` and an input list of evidence (`EvList`); O11 stays EXTERNAL. Edits: section 12. | USER, S8 |
| 2026-11-05 | Q-S8-5 (a) "cert section in cryptotool.c": the host certificate rows are a section of `test/cryptotool.c`, with no new tool. Edits: section 12. | USER, S8 |
| 2026-11-05 | Q-S8-6 (b) "S8 makes the walk faster" (not the recommendation): one upward pass over the count space gives the O4 and O5 margins in O(space k), with no list of minimal accepting vectors. Edits: section 6 (O4, O5), 12, 13 (OI-14). | USER, S8 |
| 2026-11-06 | Q-S9-1 (a) "memory operands": one import module `ratify`; each import reads its byte operands from linear memory as (offset, length) pairs that the host writes, each parameter an i64, and gives one i64. An operand outside memory gives 0, never a trap (the quorum-lang model, quorum-lang be2c329 src/wasm.c line 278). Edits: section 2, 8.2. | USER, S9 |
| 2026-11-06 | Q-S9-2 (a) "six public steps": the import set is fixed at S9: the signature check (`sig_verify`), the entry hash compare (`hash_eq`), the decryption share check (`share_verify`), the ballot proof check (`ballot_verify`), the tally add (`tally_add`) and the Lagrange combine with the bounded discrete log (`combine`). Edits: section 8.2. | USER, S9 |
| 2026-11-06 | Q-S9-3 (a) "host words, one table": each import is a host word of the kit table, and the writer lowers a host word to a call of its import. `langc eval --host FILE` takes the answer from a table of `NAME ARGS RESULT` rows that `build/cryptotool` writes from the real crypto. The count is a `Nat` (Q-S9-8 (a)). Edits: section 8.2, 10. | USER, S9 |
| 2026-11-06 | Q-S9-4 (a) "node takes the table": `test/run-wasm.mjs` takes the same `--host FILE` table and builds the import object from it. The wasmtime host rows wait for S10, where the engine of `ratifyd` gives the real import bodies. Edits: section 8.2, 10, 13 (OI-10). | USER, S9 |
| 2026-11-06 | Q-S9-5 (a) "port the import check": `test/notraptool.c` has the permit-lang f9d8a15 import check and the four bad rows (import-kind, import-module, import-name, import-type) with module `ratify` and the Q-S9-2 names and types. Edits: section 10. | USER, S9 |
| 2026-11-06 | Q-S9-6 (a) "count + 1": `combine` gives count + 1 (1 to n_j + 1) for a count that fits, and 0 for no fit or an operand outside memory. The guest checks that the result is not 0, then count = result - 1. So 0 is the refused value of all six imports. Edits: section 2, 8.2. | USER, S9 |
| 2026-11-06 | Q-S9-8 (a) "internal builtins, `Bytes`": the six host words are internal builtin rows (the quorum-lang RULE_HOST form, part = the import number J). A byte operand is one argument of the domain def `def Bytes : Type 0 := Prod Nat Nat` (OFF, LEN), and the lowering gives the two fields as the two parameters. A word operand (j, t, n_j) is one `Nat`. The five checks give `Decision` (0 refuses, 1 accepts) and `combine` gives `Nat`. Edits: section 8.2. | USER, S9 |
| 2026-11-06 | Q-S10-1 (a) "region below the heap": a module with imports has a host region [0, HOST_MEM) below its heap, and its heap starts at HOST_MEM. A module without imports does not change. The OFF words are fixed numbers, so one host table key works for eval, node, the engine and wasmtime. Edits: section 8.2, 13 (OI-10). | USER, S10 |
| 2026-11-06 | Q-S10-2 (a) "HOST_MEM 65536, the bound is the region": HOST_MEM = 65536 (one page). An operand must lie in [0, HOST_MEM): an operand that reaches the heap gives 0, so the guest cannot give its own pair blocks as bytes. `test/cryptotool.c` takes the same constant (4096 before). Edits: section 2, 8.2. | USER, S10 |
| 2026-11-06 | Q-S10-3 (a) "copy the engine": `host/engine.c`, `engine.h`, `wasmread.c` and `wasmread.h` are a copy of permit-lang `host/` (a port of relay-lang 4781ca7), plus `engine_bytes` for the operand bytes. `test/notraptool.c` stays. Edits: section 8.2. | USER, S10 |
| 2026-11-06 | Q-S10-4 (a) "hex text records": the input of `ratifyd` is one text file for each case, one line for each record, the byte fields as hex. `cryptotool demo DIR` writes D1 to D8 from the real crypto. `ratifyd` prints one verdict line; bytes that do not parse give `invalid malformed`, and the module does not run. Edits: section 8.4. | USER, S10 |
| 2026-11-06 | Q-S10-5 (a) "the guest fills the flags": a demo program calls the imports to fill the Decision flags of a Certificate, then runs `verify`. Its entry gives the Verdict as one word (0 valid, 1 + the Reason index). Edits: section 8.4. | USER, S10 |
| 2026-11-06 | Q-S10-6 (a) "engine and node, wasmtime without imports": differential rows of the engine against node on each module and on the host rows (table mode, both sides), and wasmtime CLI rows for each module without imports. wasmtime for a module with imports stays in OI-10. Edits: section 8.2, 13 (OI-10). | USER, S10 |
| 2026-11-06 | Q-S10-7 (a) "three units": U1 the engine; U2 the host region and `ratifyd` with the real import bodies; U3 the demo D1 to D8, the docs and the close. Each unit has its own serial gate. Edits: section 12. | USER, S10 |
| 2026-11-07 | Q-S10-U2-1 (a) "src/hostmem.h": one header `src/hostmem.h` has `#define HOST_MEM 65536u`. `src/wasm.c`, `test/cryptotool.c` and `host/ratifyd.c` include it (the host build adds `-Isrc`). `src/` does not depend on `host/`. Edits: section 8.2. | USER, S10 |
| 2026-11-07 | Q-S10-U2-2 (a) "engine_put_bytes": `host/engine.c` and `engine.h` get `engine_put_bytes(e, off, len, src)`, with the same bound check as `engine_bytes`. The reads stay const. Edits: none. | USER, S10 |
| 2026-11-07 | Q-S10-U2-3 (a) "share host/imports.c": the six bodies of the cryptotool `host_import` move to `host/imports.c` and `imports.h`, over `unsigned char mem[HOST_MEM]`. `cryptotool` and `ratifyd` link it. `ratifyd` copies the region out, calls the body, and writes the region back for `tally_add`. Edits: none. | USER, S10 |
| 2026-11-07 | Q-S10-U2-4 (a) "--call + OFF HEX": `ratifyd --call IMAGE FILE NAME ARGS`. Each IMAGE line is `OFF HEX` (a decimal region offset, then the bytes as lowercase hex). The bytes go into the region, then entry NAME runs, and the i64 prints as `enginetool` prints it. A bad line prints `invalid malformed`, rc 0, and the module does not run. The verdict mode `ratifyd FILE CASE` waits for U3. Edits: section 8.4. | USER, S10 |
| 2026-11-07 | Q-S10-U3-1 (a) "test/demo": the demo program is `test/demo/demo.lang`, with its want file `test/demo/want.txt`. The examples loop counts stay the same. Edits: section 8.4. | USER, S10 |
| 2026-11-07 | Q-S10-U3-2 (a) "fixed slots": each record KIND has a fixed OFF and LEN in [0, HOST_MEM), and the entry `demo` takes the OFF words. Edits: section 8.4. | USER, S10 |
| 2026-11-07 | Q-S10-U3-3 (a) "share_verify": import 2 (`share_verify`) checks `certProof`. Edits: section 8.4. | USER, S10 |
| 2026-11-07 | Q-S10-U3-4 (a) "certWt = no": the guest sets `certWt` = no for a target index that has no checked Policy. Edits: section 8.4. | USER, S10 |
| 2026-11-08 | Q-S10-U3-6 (a) "policies.lang": `test/demo/policies.lang` has the policy definitions of `demo.lang` with no Nat. A guest-mode check row proves the O3, O4 and O5 decisions, and a sync row keeps the two files the same. Edits: section 10. | USER, S10 |
| 2026-11-08 | Q-S10-U3-L (c) "split runtimes": `verify` does not lower (OI-16). The Wasm entry `demo` runs the 13 host checks and gives the flag word W. The entry `demoV` runs `verify` in the evaluator only. `build/demotool` runs the two in one process. Edits: section 8.4, OI-16. | USER, S10 |
| 2026-11-08 | Q-S11-1 (a) "langc report": `langc [--kit] report PROG` checks PROG and writes JSON Lines to stdout, with the exit status 0, 1 or 2 of permit-lang. Edits: section 8.2. | USER, S11 |
| 2026-11-08 | Q-S11-2 (a) "JSON Lines": each line has `"ratify":1` and `"kind"` (`program`, `rule`, `policy`, `log`, `height`). Numbers are JSON integers; each value is below 2^53. Edits: section 8.2. | USER, S11 |
| 2026-11-08 | Q-S11-3 (a) "copy the JCS files": `src/jsonout.c`, `host/jsonin.c` and `host/jcs.c` are copies of permit-lang. Each line is the RFC 8785 form after a parse round trip. S15 uses the files again. Edits: section 8.2. | USER, S11 |
| 2026-11-08 | Q-S11-4 (a) "cap 64": `min` lists the minimal accepting vectors in index order, at most 64. `min_count` counts all of them and `min_listed` counts the list. Edits: section 4.2. | USER, S11 |
| 2026-11-08 | Q-S11-5 (a) "read set": each `ForkFree` definition gives `read_set` = n - least + f + 1 (n the sum of the class sizes, least the O4 least total). For a plain t-of-n rule this is n - t + f + 1. Edits: section 4.2. | USER, S11 |
| 2026-11-08 | Q-S11-6 (a) "leakage fields": each rule line has `leak_t1` `"counts"`, `accept_vectors`, `reject_vectors`, `accept_reveals_all` and `reject_reveals_all` (OI-5). Edits: section 13. | USER, S11 |
| 2026-11-08 | Q-S11-7 (a) "height lines": each closed `Log` definition gives one `height` line for h = 0 to its length, with the facts of policyAt h; for h >= 1 also the approvals and the decision `accept`. Edits: section 8.2. | USER, S11 |
| 2026-11-08 | Q-S11-8 (a) "S11 and S15": S11 owns `langc report`, the RuleFacts record and the writer. S15 `check --json` shows the margins through them. Edits: section 12. | USER, S11 |
| 2026-11-08 | Q-S11-9 (a) "golden rows": `test/report/NAME.lang` and `NAME.json`, the gate line `report rows: N checked`, and `test/reporttool.c` for the write errors. Edits: section 10. | USER, S11 |
| 2026-11-09 | Q-S11-10 (a) "jsontool": `test/jsontool.c` writes strings with `"`, `\`, a newline, U+0001 and a bad UTF-8 byte through the writer, and compares each line with its RFC 8785 form. No report string has these bytes, so only this tool kills the escape mutant M4. Edits: section 10. | USER, S11 |
| 2026-11-09 | Q-S12-1 (a) "Member bits": one TFHE bit for each member (the sum of n_j inputs, in class order and then member order). The circuit computes the count (a popcount for each class) and accept; only the accept bit leaves it (OI-5). Edits: sections 8.2, 13. | USER, S12 |
| 2026-11-09 | Q-S12-2 (a) "Min-vector OR-AND": accept(c) = OR over the minimal vectors m of AND over j of (c_j >= m_j). This is exact, because O3 makes each decided rule upward closed. T2 keeps each minimal vector up to `TFHE_MIN_CAP` 4096; the T4 `min` list stays at 64. Edits: section 8.2. | USER, S12 |
| 2026-11-09 | Q-S12-3 (a) "TFHE gate set": AND, OR, XOR, NAND, NOR and XNOR (two inputs, one bootstrap each), NOT with no bootstrap, and constants folded at build time. D = the longest bootstrap chain from an input to the output (NOT adds 0). Edits: section 8.2. | USER, S12 |
| 2026-11-09 | Q-S12-4 (a) "Per gate + B + REFUSE_DEPTH": the noise depth of each gate is 1; each circuit has a static bound B from n_j, k and M before the build; the line gives D and B; D > B is a writer fault (exit 1). `REFUSE_DEPTH` at `langc tfhe` only (not at check; T1 does not change) for more than `TFHE_GATE_MAX` 65536 gates or more than `TFHE_MIN_CAP` 4096 minimal vectors. Edits: sections 6 (O6), 7. | USER, S12 |
| 2026-11-09 | Q-S12-5 (a) "JSON Lines via jsonout": `langc [--kit] tfhe PROG` writes RFC 8785 JSON Lines through `src/jsonout.c`: a `program` line, then one `circuit` line for each decided rule definition (the name, the type, the classes, the inputs, the gates as `[op, a, b]` rows, the output wire, the gate count, D and B). Edits: section 8.2. | USER, S12 |
| 2026-11-09 | Q-S12-6 (a) "C simulator vs T4 min list": `test/tfhetool.c` is a Boolean simulator that parses with `host/jsonin.c`. It runs each input assignment, compares the out bit with an oracle from the `min` list (checked against the T4 report), and computes D again; gate line `tfhe sims: N checked`. No TFHE library. Edits: section 10. | USER, S12 |
| 2026-11-09 | Q-S12-7 (a) "The 7 report rules": `test/tfhe/NAME.lang` and `NAME.jsonl` for the 7 rows of `test/report/` (`tfhe rows: 7 checked`), the `REFUSE_DEPTH` rows (`tfhe refusals: N checked`), and the mutants M1 to M8. Edits: section 10. | USER, S12 |
| 2026-11-09 | Q-S12-8 (a) "U1-U4": S12 has four units: the builder and the depth pass, the command and the golden rows, the simulator and the mutants, and the docs. Edits: section 12. | USER, S12 |

The (a) rulings keep the readings of sections 2 to 11; those sections now
cite the ruled option. The rows for Q-S0-9 (b), Q-S0-13 (b) and Q-S1-1 to
Q-S1-3 list the sections that changed.

The seed is `bin/new-lang.sh ratify-lang tcc-wasm` from lang-template
5265a2a. The kit gives the parser, the checker, the evaluator, the lowering
and the Wasm writer. No kit emits a C verifier host, an FHE circuit, a ZK
constraint system or a JSON report, so these are new code.

Carried default, not ruled for ratify-lang: proofs are guest-language terms
only (quorum-lang ruling, 2026-10-25). Lean export is not a target. The Lean
obligations of the design (O1, O11, Lean T1 to T4) stay on the UAT side
(section 6).

## 2. The reading of R0: no runtime error

R0: no program can trap or surface an exception, on any target.

The reading is the permit-lang reading (permit-lang f9d8a15 docs/PLAN.md
lines 147 to 212), which takes the accrue-lang reading and the relay-lang
host rule N6. Each operation is total on its declared type. Each run of
each target gives a value of the result type. A rejection is a value, never
an error:

- A decision is `Bool` (design sec 4, row Decision). A rule that does not
  accept gives `false`.
- `verify` gives `Verdict = valid | invalid Reason` (design sec 9, stage 6).
  Each reason has a name: `badProof`, `badSignature`, `wrongHeight`,
  `badPrevHash`, `illTypedPolicy`, `notAccepted`, `equivocation`,
  `malformed` (Q-S0-1 (a)).
- A host failure (a socket error, a missing file, bytes that do not parse)
  never reaches the module. The T1 host `ratifyd` maps it to `invalid
  malformed` first (relay-lang rule N6).
- Each crypto import of `ratifyd` (Q-S1-1 (a)) is total: it gives a value
  for each input. A ballot proof or a decryption proof that does not
  verify gives `invalid badProof`. A signature that does not verify gives
  `invalid badSignature`. Bytes that are not a point of BN254 G1 give
  `invalid malformed`. The discrete log of a decrypted count is a bounded
  search over 0 to n_j (quorum-lang be2c329 docs/PLAN.md line 424), and
  the bound of Q-S0-4 keeps it small. S7 sets the reason when no count in
  the range matches.
- The T1 imports of module `ratify` are total (Q-S9-1 (a), Q-S9-6 (a)).
  An operand outside the host region (OFF + LEN above HOST_MEM, computed
  without wrap; Q-S10-2 (a), section 8.2), or a fixed-size operand with a
  different LEN, gives 0,
  and the import reads and writes nothing. It never traps. 0 is the
  refused value of each import: a check gives 1 or 0, and `combine` gives
  count + 1 (1 to n_j + 1), or 0 when no count in 0 to n_j fits.
- A missing ballot is the trivial public encryption of 0 (design sec 9,
  stage 3). It can only lower a count, so it is not an error.

The design has two partial operations. ratify-lang makes them total:

- `reconstruct T xs` is defined only when |T| >= t (design sec 5,
  cryptographic layer). The type carries the bound (Q-S0-2 (a)):
  `reconstruct` takes a `Quorum t` value, and the only way to make one is
  a check of |T| >= t that gives `Bool`.
- `policyAt h` past the end of the log. The height is `Fin (length + 1)` of
  a given `Log`, so `policyAt` is total (Q-S0-3 (a)).

Other sources of a trap and their reading:

- Counts are bounded: 0 <= c_j <= n_j (design sec 4, row Counts). The
  checker refuses a committee whose count space (the product of (n_j +
  1)) is above 2^20, with `REFUSE_BOUND` (Q-S0-4 (a)).
- No division. The O4 and O5 margins use only addition, subtraction that
  stops at 0, and comparison (design sec 11, O5).
- A partial kit word (`head`, `tail`, `pred`, `div`, `rem`) gives
  `REFUSE_PARTIAL` (permit-lang f9d8a15 docs/PLAN.md line 182).
- The Wasm writer emits no `unreachable`, puts a guard before each `div_u`
  and `rem_u`, and gives each `br_table` a default that runs the last arm.
  The entry sets a Flag argument above 1 to 1 (`flag_norm` in
  `src/wasm.c`, permit-lang 18deea2). A C tool `test/notraptool.c` checks each module (permit-lang f9d8a15
  docs/PLAN.md lines 186 and 210; port from permit-lang).
- The kit gate row "a trap is equal to a trap" changes: R0 makes a trap a
  failure on each target (permit-lang f9d8a15 docs/PLAN.md lines 188 to
  201). Open item OI-6.

## 3. The reading of R1

R1: the type constructors are the cell monads and their duals, as
represented computationally in Cubical TT.

ratify-lang takes the permit-lang reading (permit-lang f9d8a15 docs/PLAN.md
section 3, from accrue-lang). The ambient structure is not limited: Pi,
Sigma, `Path`, `PathP`, the interval `I`, `transp`, `hcomp`, the universe
`U`, `Glue` and `ua`. A declared type constructor must be one of two
classes:

- CM, cell monad: the free algebra monad of a cell signature. The parameter
  occurs only in the generator `eta`. Unit is `eta`, bind is the recursor,
  and the laws are paths by induction.
- CC, cocell comonad: the cofree coalgebra comonad of a cocell signature.
  One observation `eps`, each other field `D -> D` or `D -> E -> D`. Counit
  is `eps`, extend is the corecursor.

The formers that the design needs (Q-S0-5 (a)):

| Former | Class | Reading | Design |
|---|---|---|---|
| `Log P Q` | CM (indexed) | The free category on accepted amendments. `nil P` is the unit, `cons e log` adds one cell. `append` is the bind. The category laws (unit, associativity) are paths by induction. | sec 4, sec 5, TCM-E |
| `System` | CC (dual of `Log`) | The cofree comonad of `Amendment -> _`: a Moore machine. In S6 `System` is `Stream Policy` (Q-S6-4 (a)): `systemOf` builds it by `sCorec` at a seed `Size`, the head is the policy at the current height, and the tail moves one height. `policyAt h` of a log is the policy after h steps. | sec 4 (Elliott behavior), sec 8 |
| `Enc a`, `Share a` | CM, no cells | The identity monad at a base type: `enc` and `share` are the unit, and the denotation is the value (design sec 5: the meaning of `enc pk a` is a). The parameter is a base type only (`Bool`, `Counts`). At T1 the representation is real (Q-S0-9 (b)): `Enc` is an exponential ElGamal ciphertext on BN254 G1 and `Share` is a Shamir share of the DKG key (section 8.2). The gate checks the representation map: the decrypt of the sum of casts equals the counts (quorum-lang S6). | sec 4, sec 5 |
| `Proof R x` | none (a base sort) | A proposition-indexed sort, not a former: the meaning is the claim that a witness exists. | sec 4 |
| `Protocol a b` | none (a base sort) | The meaning is the ideal functionality, a function. Composition holds only up to computational indistinguishability (TCM-D), so it is an ASSUMED law (section 6), not a CM law. | sec 4, TCM-D |

REFUSED formers: general recursion and fixpoints (`REFUSE_FIXPOINT`, design
sec 8), exceptions, effects, a ranked ballot over 3 or more options
(`REFUSE_ARROW`), a clock (`REFUSE_CLOCK`). Section 7 lists the codes.

The kernel source is the permit-lang kernel without the affine layer
(Q-S0-10 (a)). The nearest sibling for ballots under R1 is
quorum-lang (quorum-lang be2c329 docs/PLAN.md section 2, lines 73 to 177;
`FMSet` as CM at line 139). ratify-lang needs no `FMSet`: an approval set is
a subset of a finite committee, a base type.

## 4. The minimal base signature (R2)

Each name cites the design. Nothing outside this list is core (design
DESIGN.md head).

### 4.1 Types

| Type | Meaning | Design |
|---|---|---|
| `Party` | An element of the finite committee. | sec 4 |
| `Committee` | Classes 1 to k with sizes n_1 to n_k. | sec 4 |
| `Approvals` | A subset of the committee. A missing ballot is not an approval. | sec 4 |
| `Counts` | A vector c with 0 <= c_j <= n_j. | sec 4 |
| `Rule` | A monotone `accept : Counts -> Bool` with accept(0) = false and accept(n) = true (O3 as fields). Syntax (Q-S0-7 (a)): a threshold for each class, `and`, `or`, and a table of count vectors. The checker computes the minimal accepting vectors. | sec 4, sec 11 |
| `Policy` | Committee, authorize rule, amend rule, fault bound f, with O4 for both rules and O5 for the amend rule as fields. | sec 4, sec 8 |
| `Decision` | `Bool`. | sec 4 |
| `Amendment P P'` | An approval set S with amend_P(count_P S) = true, and P', a well-typed `Policy`. | sec 5 |
| `Log P Q` | CM (section 3). | sec 4 |
| `System` | CC (section 3). | sec 4 |
| `Height` | `Fin (length + 1)` of a given log (Q-S0-3 (a)). | sec 10 |
| `Enc a`, `Share a`, `Proof R x`, `Protocol a b` | Sorts with laws. Real crypto at T1 (Q-S0-9 (b), section 8.2): `Enc` is an exponential ElGamal ciphertext on BN254 G1, `Share` a Shamir share, `Proof` a sigma proof (disjunctive Chaum-Pedersen for R_ballot, DLEQ for R_dec), `Protocol` the ideal function. | sec 4 |
| `Certificate` | (h, a, ballots, proofs, b), and for an amendment P' and the hash of entry h. The signatures are Schnorr on BN254 G1 with a keccak-256 challenge; the entry hash is keccak-256 (Q-S1-3 (a)). | sec 9, stage 5 |
| `Verdict`, `Reason` | Section 2. | sec 9, stage 6 |

G_P (the class-preserving group, design sec 4) is NOT a user type. The
checker derives it from the classes. A rule reads only `Counts`, so each
rule is orbit-constant by construction (design sec 5: "This one fact
discharges UAT's orbit-constancy obligation for every rule"). A rule that
names one party inside its class cannot be written (`REFUSE_ORBIT`). To
give a party a veto, declare a class with only that party (design sec 4,
"Vetoes and named roles").

### 4.2 Operations

| Operation | Meaning | Design |
|---|---|---|
| `count P S` | The vector of \|S intersect class_j\|. | sec 5 |
| `authorize P a S` | accept_P(count_P S). | sec 5 |
| `and r r'`, `or r r'` | Pointwise on counts. `or` on an amend rule checks O5 again. | TCM-B |
| `nil P`, `cons e log`, `append l1 l2` | The free category. | sec 5, TCM-E |
| `policyAt log h` | P_0 at 0; the target of entry h at h + 1. | sec 5 |
| `enc pk a` | a. | sec 5 |
| `eval g c_1 ... c_m` | g of the meanings. | sec 5 |
| `share t n s`, `reshare C C' t' xs` | s; the same secret, new holders. | sec 5, sec 8 |
| `reconstruct T xs` | s, total by Q-S0-2 (a). | sec 5 |
| `prove R x w` | R(x, w). | sec 5 |
| `run pi xs` | f_pi(xs). | sec 5 |
| `verify cert` | `valid` exactly when the certificate denotes a valid step. | sec 9, stage 6 |

Checker facts (not user operations, shown by `langc check --json` and T4):
the minimal accepting vectors of each rule, the O4 margin (smallest
accepting total minus f), the O5 margin (the smallest
sum_j max(0, a_j + b_j - n_j) minus f over minimal accepting a, b), and the
read-set size n - t + f + 1 for a plain t-of-n amend rule (design sec 10).
S11: `langc report` gives these facts (`docs/REPORT-JSON.md`). The read
set of each `ForkFree` rule is n - least + f + 1 (Q-S11-5 (a)), and `min`
lists at most 64 vectors (Q-S11-4 (a)).

## 5. Time

Time is the log height, a natural number (design sec 10: "No shared
clock"). There is no wall clock and no host tick. A name such as `now`,
`deadline`, `timeout` or `expires` gives `REFUSE_CLOCK`. `System` (section
3) is the behavior of design sec 8: a function from height to policy.
`policyAt (h + 1)` reads only `policyAt h` and entry h, so the definition
is causal and total (O10).

## 6. Laws and obligations

Each law is a guest-language term with a `Path` type, or a decision of the
checker on a finite set. `langc check --json` reports four counts in
`progress` (Q-S0-8 (a)): `laws_discharged` and `laws_total` (the DISCHARGED
rows that apply to the program), `laws_assumed`, `laws_at_target` and
`laws_external`. An ASSUMED row is never counted as discharged.

| Row | Statement | Kind | How |
|---|---|---|---|
| TCM-A | authorize is invariant under G_P | DISCHARGED | count is G_P-invariant; a rule reads only counts |
| TCM-B | and, or are pointwise | DISCHARGED | definition; O5 checked again after `or` |
| TCM-C | dec(eval g cs) = g(dec cs) | ASSUMED (T1); AT TARGET (T2) | scheme correctness; at T1, g is the sum of ElGamal ciphertexts, exact while each count is below the group order r (Q-S0-9 (b)) |
| TCM-D | composition of protocols, up to indistinguishability | ASSUMED | UC theorem, outside the kernel |
| TCM-E | nil is the unit, append is associative | DISCHARGED | paths by induction (Lean T4 on the UAT side) |
| View 1 to 3 | the three view equations of design sec 5 | ASSUMED | named hardness and proof-system assumptions; the T1 leakage is the count vector c (Q-S1-2 (a)) |
| O1 | each rule is orbit-constant under G_P | EXTERNAL (Lean) | guest form is TCM-A, counted once |
| O2 | each decision is binary | DISCHARGED | the type of `accept` |
| O3 | monotone, accept(0) = false, accept(n) = true | DISCHARGED | decided on the finite count space |
| O4 | each accepting set holds an honest party | DISCHARGED | decided on the count space: the least total of an accepted vector (the S8 pass, Q-S8-6 (b)) |
| O5 | fork freedom of the amend rule | DISCHARGED | decided on the count space: one upward pass gives the least overlap of two accepted vectors (S8, Q-S8-6 (b), `rule_margins` in `src/front/check.c`) |
| O6 | FHE exactness for accept . count | AT TARGET (T2) | a static depth bound: per gate (noise depth 1), B per circuit (Q-S12-4 (a)); S12: `langc tfhe` gives D and B, and `build/tfhetool` checks D <= B |
| O7 | threshold decryption matches the ideal | ASSUMED | DDH and the DLEQ proofs (T1, Q-S0-9 (b)); noise flooding proof (T2) |
| O8 | ZK for R_ballot and R_dec | ASSUMED (T1); exported at T3 | Chaum-Pedersen and DLEQ in the random oracle model (T1); Noir at T3 (Q-S0-13 (b)) |
| O9 | resharing on a committee change | ASSUMED | proactive secret sharing |
| O10 | causality of the log | DISCHARGED | by construction (section 5) |
| O11 | at most one entry per height | EXTERNAL (Lean) | O5 and honest single signing (Lean T3) |

Count at S1 (the rulings change no kind): DISCHARGED 8 (TCM-A, TCM-B, TCM-E, O2, O3, O4, O5, O10),
ASSUMED 8 (TCM-C, TCM-D, three view equations, O7, O8, O9), AT TARGET 1
(O6), EXTERNAL 2 (O1, O11).

## 7. Refusals

The eleven new check codes (Q-S0-6 (a)). Each one gets a `fix` hint at A2.
The last row, `REFUSE_DEPTH` (S12, Q-S12-4 (a)), is not a check code: only
`langc tfhe` gives it.

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

## 8. Route

### 8.1 Kernel

Port the permit-lang Cubical TT kernel without the affine layer (Q-S0-10
(a)), then add R0 (section 2) and the classes check of R1 (section 3). Survey first: SAME, CHANGED,
SOURCE-ONLY.

### 8.2 The four targets

- T1 (primary): one Wasm module for `authorize`, the rule facts,
  `policyAt`, the log replay and `verify`. The C host `ratifyd` reads a
  log file and certificates, runs the module and prints the verdicts.
- T1 crypto (Q-S0-9 (b)): exponential ElGamal ballots on BN254 G1, a
  Pedersen DKG key, threshold decryption with DLEQ proofs, disjunctive
  Chaum-Pedersen ballot proofs, Schnorr signatures on BN254 G1 and
  keccak-256 entry hashes (Q-S1-3 (a)). The C code is a port of
  quorum-lang be2c329 src/crypto (from intent-lang 79e1898). It lives in
  `ratifyd`, and the module calls it through a fixed host-import ABI on
  real values (Q-S1-1 (a), the quorum-lang Q9 (a) ruling). `ratifyd` runs
  only the public steps: the proof and signature checks, the sum, the
  Lagrange combine and the bounded discrete log. A C test tool makes the
  fixtures (the DKG, the casts, the partial decryptions, the signatures),
  as quorum-lang `test/tallytool.c` does.
- T1 import ABI (S9; Q-S9-1, Q-S9-2, Q-S9-6 and Q-S9-8 (a)): one import
  module `ratify` with six function imports; import J is function J. Each
  parameter and each result is an i64. A byte operand is two parameters,
  OFF and then LEN, in memory 0 (exported as `memory`). A word operand (j,
  t, n_j) is one parameter. The host reads and writes each operand in the
  big-endian `_be` form of the C code, and reads all the operands before
  it writes an output. In the guest, a byte operand is one argument of
  `def Bytes : Type 0 := Prod Nat Nat`, and a word operand is one `Nat`.
  The five checks give `Decision` (the import result is the tag: 0
  refuses, 1 accepts), and `combine` gives `Nat`.

| J | Import | Operands (bytes) | i64 params | Guest args | Result | C body |
|---|---|---|---|---|---|---|
| 0 | `sig_verify` | pk 64, msg any, sig 96 | 6 | 3 | 1 good, else 0 | `schnorr_verify` |
| 1 | `hash_eq` | data any, want 32 | 4 | 2 | 1 equal, else 0 | `keccak256` of data, then compare with want |
| 2 | `share_verify` | part 232, key 64, ct 128 | 6 | 3 | 1 good, else 0 | `thr_verify_be` |
| 3 | `ballot_verify` | proof 384, pk 64, ct 128, ctx 32, word j | 9 | 5 | 1 good, else 0 | `sig_or_verify_be` |
| 4 | `tally_add` | out 128, a 128, b 128 | 6 | 3 | 1 written, else 0 | `tally_add_be` |
| 5 | `combine` | parts 232 x K, keys 64 x N, ct 128, word t, word n_j | 8 | 5 | count + 1, else 0 | `thr_open_be`, then the bound n_j |

  K = LEN(parts) / 232 and N = LEN(keys) / 64; a LEN that is not a
  multiple gives 0.
- T1 host table (S9; Q-S9-3 and Q-S9-4 (a)): `build/cryptotool` writes
  `NAME ARGS RESULT` rows from the real crypto. `langc eval --host FILE`
  and `test/run-wasm.mjs --host FILE` take the answer of each import from
  that table. The key of a row is the guest words (OFF, LEN and the word
  operands), not the bytes, so eval, node and the module see one table
  with one key. A call with no row, and a module with imports but no
  table, are usage errors of the tool (rc 1), not results of the module.
- No memory image in S9 (DECISION): each entry resets the heap pointer to
  0 (`src/wasm.c` line 14), so bytes written at offset 0 before the call
  are overwritten by the pairs of the entry before an import reads them.
  The table key is the words, so node needs no bytes. A host that reads
  the operand bytes (the engine of `ratifyd`, wasmtime) needs a fixture
  layout outside the heap: S10, OI-10.
- T1 host region (S10; Q-S10-1 and Q-S10-2 (a)): a module with imports
  has a host region [0, HOST_MEM) below its heap, HOST_MEM = 65536. Its
  heap starts at HOST_MEM, and its memory is ceil((HOST_MEM + 8 * the
  heap words) / 65536) pages. A module without imports does not change.
  The host writes the operand bytes in the region before the call, and an
  operand must lie in the region. `src/wasm.c`, `test/cryptotool.c` and
  `host/ratifyd.c` use one constant HOST_MEM, in `src/hostmem.h`.
- T1 tally (Q-S1-2 (a)): ElGamal can only add. T1 sums the ballots of each
  class, decrypts the count vector c and applies the rule in the clear. So
  the T1 output leaks c, not only b. The one-bit output of design sec 9
  stage 3 needs T2.
- T1 engine: the Wasm engine of `ratifyd` is an own C99 interpreter, a
  copy of the permit-lang engine (Q-S0-11 (a), the relay-lang ruling).
  Differential rows under node and wasmtime (OI-10). S10 (Q-S10-3 (a)):
  `host/engine.c`, `engine.h`, `wasmread.c` and `wasmread.h` are a copy of
  permit-lang `host/`, plus `engine_bytes`, the bound check of a byte
  operand. The rows (Q-S10-6 (a)): the engine against node on each module
  and on the host rows (table mode, both sides), and wasmtime CLI rows for
  each module without imports.
- T2: the FHE tally circuit for accept . count in TFHE (Q-S0-12 (a)), with
  a static depth bound per gate (O6). `REFUSE_APPROX` for CKKS. S12:
  `langc [--kit] tfhe PROG`, JSON Lines in RFC 8785 form (Q-S12-5 (a));
  the format is `docs/TFHE-CIRCUIT.md`.
- T3: R_ballot and R_dec as Noir programs (Q-S0-13 (b)), with golden files
  (O8). See OI-11.
- T4: the JSON report: the policy facts, the minimal accepting vectors, the
  O4 and O5 margins, the read-set size, the per-height log verdicts, and
  what the output leaks for each rule (design sec 13, first item; at T1
  the leakage is c, Q-S1-2 (a)). S11: `langc [--kit] report PROG`, JSON
  Lines in RFC 8785 form (Q-S11-1 to Q-S11-3 (a)); the schema is
  `docs/REPORT-JSON.md`.

### 8.3 Two C programs

`langc mcp` and `ratifyd` are different programs. Do not merge them.

- `langc mcp` is the AUTHORING server for agents that WRITE ratify-lang:
  `check`, `fmt`, `explain`, `list-base`, `show-law`, `build`.
- `ratifyd` is the VERIFIER host: it runs the T1 module on certificates and
  logs. It holds no secret key and casts no ballot.

### 8.4 The demo

A committee in two classes: three operators, and one auditor in a class of
its own (a veto, design sec 4 box). f = 1. Authorize rule: 2 of 3 operators
and the auditor. Amend rule: 3 of 3 operators and the auditor. The checker
shows O4 (margin 2) and O5 (margin 3). A log of two amendments, one of
which adds a fourth operator.

The runtimes are split (Q-S10-U3-L (c), OI-16). The Wasm entry `demo` of
`test/demo/demo.lang` (kit mode) runs the 13 host checks and gives the
flag word W: bit k is 1 when check k holds, so a good set gives 8191. The
entry `demoV` runs in the evaluator only. It reads W, the height, the
action and the words `amend`, `target` and `signers`, runs `verify`, and
gives the Verdict as one word: 0 is `valid`, 1 + the Reason index is
`invalid REASON`. The W bit order is test code in `demo` and `demoV`; the
gate rows of section 10 pin it.

The input (Q-S10-4 (a), Q-S10-U3-2 (a)): one text file for each case, one
line for each record. A record is `KIND HEX` for the 18 byte KINDs, or
`KIND N` for the words `amend`, `target` and `signers`. Each byte KIND has
a fixed slot (OFF, LEN) in the host region: `pk0` to `pk4` at 0, 64, 128,
192 and 256 (64 bytes each); `body` 320 (48); `prev` 368 (48); `sig0` to
`sig4` at 416, 512, 608, 704 and 800 (96 each); `proof` 896 (424: the
share 232, the key 64, the cipher 128); `evA` 1320 (48); `evB` 1368 (48);
`evH` 1416 (32); `sigA` 1448 and `sigB` 1544 (96 each). `demo` takes the
18 OFFs in this order. The height and the action are the body bytes 0 to
8 and 40 to 48, as big-endian words.

The bits of W: k 0 to 4, the signature of party k on `body` (party 4 is
the auditor); k 5, keccak256 of `prev` is the prev field of `body`; k 6,
`share_verify` on `proof` (Q-S10-U3-3 (a)); k 7, keccak256 of `evB` is
`evH`; k 8, `sigA` on `evA`; k 9 and k 12, the prev fields of `evA` and
`evB`; k 10, keccak256 of `evA` is `evH`; k 11, `sigB` on `evB`.

`build/cryptotool demo DIR` writes D1 to D8 from the real crypto (DIR must
exist). `build/demotool MODULE DEMO.lang DK.txt` writes the records in a
zero region, runs `demo` on the engine with the six imports of `ratifyd`,
then runs `demoV` in the same process, and prints one line `W VERDICT`.
Exit 0: a verdict. Exit 1: a load, engine or eval error. Exit 2: usage. A
record set that does not parse gives `- invalid malformed` with exit 0,
and the module does not run: an unknown KIND, a KIND two times, a missing
KIND, a bad hex field or word, or a hex field with a byte count other than
the slot LEN.

| Case | Change from the good image | W | Verdict |
|---|---|---|---|
| D1 | none | 8191 | `valid` |
| D2 | `sig1` bad | 8189 | `invalid badSignature` |
| D3 | the prev field of `body` bad, all signatures again | 8159 | `invalid badPrevHash` |
| D4 | `proof` with the key of party 1 | 8127 | `invalid badProof` |
| D5 | `evB` with action 10, signed by party 0 | 8063 | `invalid equivocation` |
| D6 | `body` height 3, all signatures again | 8191 | `invalid wrongHeight` |
| D7 | `sig3` bad, `signers` 1 (operator 0 only) | 8183 | `invalid notAccepted` |
| D8 | `evH` with 63 hex digits | - | `invalid malformed` |

An amendment whose P' fails O5 is refused before a vote: the refusal row
of `test/demo/bad-o5.lang` (`REFUSE_FORK`, rule O5). A large height word
(for example a 64-bit value) makes `demoV` reach the arena limit of the
evaluator (`langc: OOM`), so `demotool` gives exit 1, not `wrongHeight`
(OI-17).

The U2 mode (Q-S10-U2-4 (a)): `ratifyd --call IMAGE FILE NAME ARGS`
writes the IMAGE bytes in the host region, runs entry NAME of FILE on
ARGS and prints the i64 result. An IMAGE line is `OFF HEX`: a decimal
region offset, then the bytes as lowercase hex.

## 9. Agent adoption (A1 to A6)

The request asks for six items so that agents reach for ratify-lang when it
fits. Each item has one deliverable, one slice and one gate.

| Item | Pasted line | Deliverable | Slice | Gate |
|---|---|---|---|---|
| A1 | "one canonical form per operation and a langc fmt canonical printer" | No aliases, no second syntax. `langc fmt FILE` prints the canonical form. `langc fmt --check` exits 1 when a file is not canonical. | S14 | `fmt` is idempotent; `parse (fmt x)` has the same core term as `x`; each example is canonical. |
| A2 | "langc check --json, giving fix hints and progress counts (parsed, typed, laws discharged)" | One JSON object: `diagnostics` (code, span, expected, found, `fix`) and `progress` (`parsed`, `typed`, `laws_discharged`, `laws_total`, `laws_assumed`, `laws_at_target`, `laws_external`). Schema in `docs/CHECK-JSON.md`. | S15 | Rows in `test/json/`, byte equal. Each code of section 7 has a `fix` row. |
| A3 | "a SKILL.md with 3 to 5 graded examples" | `skills/ratify-lang/SKILL.md` with five graded examples: G1 a plain t-of-n policy, G2 two classes and a veto, G3 an amendment that changes the amend rule, G4 a log replay with `policyAt`, G5 the demo. | S17 | Each example checks; each SKILL.md block and its file are byte equal. |
| A4 | "an MCP server written in TinyCC (langc mcp)" | An MCP server over stdio (JSON-RPC 2.0, one message on each line), in C, in the same `langc`. Tools: section 8.3. The MCP spec version is pinned in `docs/MCP.md`. | S16 | `test/mcptool.c` sends `test/mcp/*.in` and compares the replies with `*.out`. |
| A5 | "AGENTS.md and a README table saying when to use the language" | `AGENTS.md` and the README table "When to reach for ratify-lang". Both exist at S0 as DRAFT; S17 makes them final. | S0, S17 | Review only. |
| A6 | "a token-count comparison against Python and Rust" | G1 to G5 and the demo in ratify-lang, Python and Rust, with equal outputs. Report `docs/TOKEN-BASELINE.md`: the tokenizer pin, each count, each file hash (Q-S0-14 (a): the permit-lang ruling, count_tokens and equal outputs). | S18 | `make tokens`, not in `make check`. |

## 10. Tests

- The kit gate rows, with the R0 change of the trap row (OI-6).
- One check row for each code of section 7, and one `fix` row at A2.
- Eval rows for each operation of section 4.2 on closed data.
- Crypto rows (Q-S0-9 (b)): known-answer rows, invalid-proof and
  invalid-signature rows, and representation rows (the decrypt of the sum
  of casts equals `langc eval` of the counts), as quorum-lang S6 (O80 to
  O84).
- Law rows: the design example (one class of 4, t = 3, f = 1: O5 margin
  2 * 3 - 4 - 1 = 1) and the demo of section 8.4.
- `test/notraptool.c` on each module, with the import check and the four
  import bad rows (Q-S9-5 (a)); differential rows under node and wasmtime
  (T1).
- Host rows (S9): `build/cryptotool` writes the host table, `langc eval
  --host` rows on `test/eval/host.lang`, and `test/run-wasm.mjs --host`
  rows on the module of `test/ir/host.lang`. Eval and node give the same
  answer from one table; a missing row and a missing table give rc 1.
- Demo rows (S10, section 8.4): the check, build and abi rows of
  `demo.lang`, the two `policies.lang` rows and the `bad-o5.lang` refusal;
  9 verdict rows against `test/demo/want.txt` (D1 to D8, and D9: D1 with
  `target 4` gives `invalid illTypedPolicy`); 4 malformed rows; 13
  bit-order rows (`demoV` on W with one bit clear gives the Reason of that
  check); 2 cross rows (`langc eval --host` and the engine on
  `test/demo/host-table.txt` both give W 4709).
- Golden files for T2, T3 (Noir) and T4. T4 (S11): the 7 rows of
  `test/report/` (`report rows: 7 checked`), `build/reporttool`
  (`report write errors: 2 checked`) and `build/jsontool` (`json
  escapes: 5 checked`, Q-S11-10 (a)). T2 (S12): the 7 rows of
  `test/tfhe/` (`tfhe rows: 7 checked`), 1 refusal row (`tfhe refusals:
  1 checked`) and `build/tfhetool`, a Boolean simulator checked against
  the T4 `min` list (`tfhe sims: 19 checked`, Q-S12-6 (a), Q-S12-7 (a)).

## 11. Port sources

| Source | What | Where |
|---|---|---|
| lang-template 5265a2a | the tcc-wasm kit | `hosts/tcc-wasm` |
| permit-lang 3d47120, 18deea2, f9d8a15 | 3d47120 (S2, Q-S0-10 (a), Q-S2-1 (a)): the kernel without R0, the affine layer and the domain words: `src/front/` (ast.h, check.c, check.h, core.h, eval.c, front.c, front.h, lexer.c, lexer.h, parser.c), `src/main.c`, `test/fronttool.c`, `test/pathtool.c`, `test/review.sh`, `test/gate.sh`, `domain/domain.lang` and the kernel rows. 18deea2 (S3): the R0 reading, `REFUSE_PARTIAL`, `src/wasmop.h`, the no-trap Wasm writer, `test/notraptool.c`, the trap row; taken at S3 by hand (57 paths take the 18deea2 file, 2 are deleted, 10 are merged; the Wasm and differential rows stay in S3, permit-lang moved them to its S8). f9d8a15: A1 to A6; the C99 Wasm engine (Q-S0-11 (a)) | docs/PLAN.md lines 147 to 212, 214, 628 to 641 (f9d8a15); S2 surveyed the kernel and S3 the R0 port; S10 surveys the rest |
| quorum-lang be2c329 | R1 former table, ballots, the FHE route, the Wasm replica; src/crypto and `test/tallytool.c` (Q-S0-9 (b)); the host-import ABI (Q-S1-1 (a)) | docs/PLAN.md lines 73 to 177, 352 to 523, 524 to 572, 574 |
| orbit-lang 2162fb2 | the orbit projection and the Lan facts (UAT `orbitProjection`, `Aggregation`) | docs/PLAN.md lines 48 to 62 |

## 12. Slices

One session for each slice: scope, gate, mutants, docs, close.

| Slice | Work | Status |
|---|---|---|
| S0 | Seed from tcc-wasm, DESIGN.md and the design HTML, this plan, README table, AGENTS.md; gate EXIT 0; seed commit; private repo | DONE |
| S1 | The USER rules section 14 and Q-S1-1 to Q-S1-3. Section 1 has the rulings. | DONE |
| S2 | Port the Cubical TT kernel (8.1, Q-S0-10 (a), Q-S2-1 (a)): permit-lang 3d47120, no affine layer, no R0. Gate: 29 examples, 17 parse refusals, 154 check refusals, 10 kit-only refusals, 1 time hint, 78 evals, 1 build refusal; the review, domain and path regressions pass. Mutants M1 (path ends), M2 (glue face), M3 (universe level): KILLED. | DONE |
| S3 | Port permit-lang 3d47120..18deea2: R0 in the kernel and the Wasm writer, `REFUSE_PARTIAL`, `src/wasmop.h`, `test/notraptool.c`, the trap row (OI-6). `src/front/lower.c`, `src/wasm.c` and `src/ir.h` come back into the build, with the IR rows, the Wasm modules and the differential cases (OI-12). Gate: 30 examples, 17 parse refusals, 158 check refusals, 10 kit-only refusals, 1 time hint, 79 evals, 1 build, 1 ir lowered, 5 ir refusals, 2 ir lowerings, 4 notrap modules, 7 notrap bad rows, 10 spine evals, 33 Wasm modules valid, 998 differential cases with 0 mismatches. Mutants M1, M2 (`REFUSE_PARTIAL`), M3 (the div guard of the no-trap tool), M4 (`flag_norm`): KILLED. M5 (the `br_table` default) and M6 (the IF condition): equivalent. | DONE |
| S4 | `Party`, `Committee`, `Approvals`, `Counts`, `count`; G_P derived; TCM-A; `REFUSE_ORBIT`, `REFUSE_BOUND`. The four words are families in `domain/domain.lang`, with the definitions `posWeaken`, `countBits` and `count`; G_P is `Perm` and `actP` (Q-S4-1 (a)). `REFUSE_ORBIT`: a program definition that is not a law reads `Approvals` (Q-S4-2 (a)). `REFUSE_BOUND`: a definition type, or a definition of type `Committee`, at a closed committee with a count space above 2^20 (Q-S4-3 (a)). TCM-A: by construction, and the closed `refl` rows `tcmSwap`, `tcmLift` and `tcmVeto` in `examples/committee.lang` (Q-S4-4 (a)). Rule R4: the domain name `count` clashed with three guest names, so `examples/formers.lang` has `counted`, and `examples/state-no-history.lang` and `test/check/state-boundary.lang` have the state field `stateCount`. Gate: 32 examples, 17 parse refusals, 162 check refusals, 10 kit-only refusals, 1 time hint, 81 evals, 1 build, 1 ir lowered, 5 ir refusals, 2 ir lowerings, 4 notrap modules, 7 notrap bad rows, 10 spine evals, 35 Wasm modules valid, 998 differential cases with 0 mismatches. Mutants M1 (the bound compare), M2 (the factor n_j + 1), M3 (the elim case of `orbit_uses`), M4 (the approval arm of `countBits`): KILLED. | DONE |
| S5 | `Rule`, `and`, `or`, the minimal accepting vectors; O3, O4, O5; `REFUSE_RULE`, `REFUSE_HONEST`, `REFUSE_FORK`; TCM-B. On the USER S4 commit c19574d. `Rule c` is a family in `domain/domain.lang` (`rlMin`, `rlAnd`, `rlOr`, `rlTable` with `CtTable c`), with the definition `accept` (Q-S5-1 (a)). `Honest c f` and `ForkFree c f` are families on a committee and the fault bound `f : Size` (Q-S5-2 (a)). For a program definition that is not a law, with the type `Rule`, `Honest` or `ForkFree` at a closed committee, the kernel fills the accept table of the count space, computes the minimal accepting vectors and decides O3 (`REFUSE_RULE`), O4 (`REFUSE_HONEST`, for `Honest` and `ForkFree`) and O5 (`REFUSE_FORK`, for `ForkFree`). An open committee or an open f gives no decision. The quoted-type walk of `REFUSE_ORBIT` stays (Q-S5-3 (a), OI-13 closed). TCM-B: the closed `refl` rows `tcmAnd` and `tcmOr` in `examples/rule.lang`, and the check rows `fork-even` and `fork-or`. Gate: 33 examples, 17 parse refusals, 168 check refusals, 10 kit-only refusals, 1 time hint, 85 evals, 1 build, 1 ir lowered, 5 ir refusals, 2 ir lowerings, 4 notrap modules, 7 notrap bad rows, 10 spine evals, 36 Wasm modules valid, 998 differential cases with 0 mismatches. Mutants M1 (the monotone verdict), M2 (the accept-0 check), M3 (the O4 margin compare), M4 (the clamp guard), M5 (the `rlAnd` and `rlOr` choice of the accept fill): KILLED. | DONE |
| S6 | `Policy`, `Decision`, `authorize`, `Amendment`, `Log` (CM), `System` (CC), `policyAt`; TCM-E, O2, O10; `REFUSE_FIXPOINT`, `REFUSE_ARROW`, `REFUSE_CLOCK`. On the USER S5 commit 1a38884. `Policy` is `plOf` with `plCom`, `plF`, `plAuth : Honest plCom plF` and `plAmend : ForkFree plCom plF`. `hoRule` and `ffRule` can only be the whole body of a closed decided definition, and the seal is a walk over the written term, not over the core term (Q-S6-1 (a)). `REFUSE_CLOCK` and `REFUSE_FIXPOINT` are the kit codes `REFUSE_TIME` and `REFUSE_REC` with new names (Q-S6-2 (a)). `REFUSE_ARROW` refuses a closed choice of three or more values from `Approvals` or `Counts` (Q-S6-3 (a)). `System` is `Stream Policy` (Q-S6-4 (a)). `Log` has the TCM-E paths `appendNilL`, `appendNilR` and `appendAssoc`, and `examples/policy.lang` has the O10 rows `o10At0` and `o10Past`. Gate: 34 examples, 17 parse refusals, 177 check refusals, 87 evals, 37 Wasm modules valid, 998 differential cases with 0 mismatches. Mutants M1 to M5 KILLED. The seal walk respects local binders (OI-15 closed); `Fin` is internal (O23), so `REFUSE_ARROW` checks only `Sum` trees and `Pos n`. | DONE |
| S7 | The crypto layer (Q-S0-9 (b), Q-S1-1 (a), Q-S1-3 (a)): `Enc`, `Share`, `Proof`, `Protocol` and their operations; port quorum-lang src/crypto and add Schnorr on BN254 G1; the test tool and the crypto rows (section 10); the ASSUMED rows; `REFUSE_APPROX`. On the USER S6 commit b44d1f6. `Enc` and `Share` are families on the code sort `Base`, and `El` gives the type of a code (Q-S7-1 (a)). `src/crypto/` has the 20 quorum-lang files, byte equal, and `schnorr.c` (Q-S7-2 (a)); a signature is (R, s), 96 bytes (Q-S7-3 (a)). The ASSUMED laws stay ASSUMED, and `docs/STATUS.md` gives their rows (Q-S7-4 (a)). `REFUSE_APPROX` refuses the 5 names `ckks`, `approx`, `approximate`, `float` and `double` at the 4 sites after `REFUSE_CLOCK`, with the hint "an exact scheme only" (Q-S7-5 (a)). `Proof` is `Trunc1` of the witness pair, and `trunc1` takes its type first (Q-S7-6 (a)). DECISION: a family with a function field gives `REFUSE_CELL` (R1), so `Protocol A B` is the function type `A -> B` and `protoOf` is a definition. The word is `protoOf`, not `ptOf`: `ptOf` is a field of `Party` (S4, rule R4). DECISION: `build/cryptotool rep` prints eval lines, and the gate runs `langc eval` on each. DECISION: `eval` has two inputs, so the sum is a chain of eval steps over `Enc (baseCounts (cmClass n cmNil))`: `zeroSum`, `addEnc`, then `openCount` (share 2 of 3, reconstruct, `ctTop`, `posVal`, `sizeNat`). Tool: `build/cryptotool` 97 rows (23 known-answer, 6 invalid-signature, 43 threshold, 17 reshare, 8 invalid-proof), with the row "schnorr s plus r"; 14 rep lines (TCM-C at T1). Gate: 35 examples, 17 parse refusals, 179 check refusals, 94 evals, 14 crypto rep lines, 38 Wasm modules valid, 998 differential cases with 0 mismatches. Mutants M1 to M5 KILLED. M1a (the R on-curve check of Schnorr verify) is EQUIVALENT: the challenge binds the R bytes, so R + e.P = s.G holds only for R = s.G - e.P, which is on the curve; the check stays. M4a (the body of `reshare`) is EQUIVALENT: the S7 domain bodies are parametric in b. | DONE |
| S8 | `Certificate`, `Verdict`, `Reason`, `verify`, equivocation evidence; a faster O5 walk (Q-S8-6 (b)). On the USER S7 commit 796f442. `Reason` has the 8 reasons of Q-S8-3 (a), and `Verdict` is `valid` or `invalid r`. `Certificate P` is `certOf` with the height, the action, the signer set, b, P' and one `Decision` for each host byte check (Q-S8-1 (a)). `verify P Q l c evs` checks c against the tip Q of `l : Log P Q`; the first failure wins (`vdThen`), and `wrongHeight` is a height that is not `lgLen l` (Q-S8-2 (a), Q-S8-3 (a)). `Evidence c` is `evOf` (a party, a height, two hashes, two signature decisions), `equivocates` holds for two signed bodies at one height with two hashes, and `EvList c` is the input list (Q-S8-4 (a)). DECISION: the guest has no hash type, so a hash is a `Size` stand-in compared with `szEq`; the action is a `Size` code, because `authorize` takes any A. `examples/certificate.lang` has 18 closed `refl` rows: one for each reason, the valid rows and the first-failure rows. `test/cryptotool.c` has the cert section, 12 rows (Q-S8-5 (a)): the body bytes, Schnorr over keccak256 of the body, the check order height, prev hash, signature, and the evidence. O5 (Q-S8-6 (b), OI-14 closed): `rule_margins` in `src/front/check.c` is one upward pass over the count space, O(space k), with no list of minimal accepting vectors; on 18752 cases it gives the same least total and least overlap as the S5 walk. `examples/fork-margin.lang` (20 classes of 1, f = 3) checks in 12.0 s, and `test/check/fork-margin.lang` (f = 4) gives `REFUSE_FORK`. Gate: 37 examples, 17 parse refusals, 180 check refusals, 112 evals, 14 crypto rep lines, 40 Wasm modules valid, 998 differential cases with 0 mismatches, 109 crypto rows. Mutants M1a and M1b (the reason order), M2 (the height), M3 (the accept choice), M4 (the evidence), M5 (the C signature check), M6 to M9 (the O5 pass): KILLED. M1a was LIVE first; the USER added the row `rBothBad`, which kills it. | DONE |
| S9 | T1 part 1: the Wasm module, the crypto import ABI (Q-S1-1 (a)), the notrap rows. On the USER S8 commit a2322ef. The module calls six function imports of module `ratify` (section 8.2; Q-S9-1, Q-S9-2, Q-S9-6 and Q-S9-8 (a)): `sig_verify`, `hash_eq`, `share_verify`, `ballot_verify`, `tally_add` and `combine`. Each is an internal builtin row whose part is the import number. `src/wasm.c` writes the import section, and the lowering gives each `Bytes` argument as its two fields and the `Decision` block from the import result. `langc eval --host FILE` and `test/run-wasm.mjs --host FILE` read one `NAME ARGS RESULT` table, which `build/cryptotool` writes from the real crypto (Q-S9-3 and Q-S9-4 (a)). `test/notraptool.c` has the import check and 4 new bad rows (Q-S9-5 (a)). DECISION: no memory image in S9; the key of the table is the guest words (section 8.2). Gate: 37 examples, 17 parse refusals, 180 check refusals, 112 evals, 14 crypto rep lines, 26 host table rows, 23 host evals, 5 notrap modules, 11 notrap bad rows, 9 host wasm rows, 40 Wasm modules valid, 998 differential cases with 0 mismatches, 109 crypto rows. Mutants M-h1 (the import name), M-h2 (the signature answer), M-h3 (the import section): KILLED. | DONE |
| S10 | T1 part 2: `ratifyd`, the Wasm engine (Q-S0-11 (a)), the demo D1 to D8. Three units (Q-S10-7 (a)), each with its own serial gate: U1 the engine (`host/`, `test/enginetool.c`, the load refusal rows, the engine against node rows); U2 the host region and `ratifyd` with the real import bodies; U3 the demo D1 to D8, the docs and the close. U3 result (Q-S10-U3-L (c)): `build/demotool` gives the W and the verdict of D1 to D8 as section 8.4 says. The gate adds 5 demo rows, 1 demo refusal, 9 verdict rows, 4 malformed rows, 13 bit-order rows and 2 cross rows. Mutants u6 (two bits of `demo` swapped), u7 (`demoV` reads a wrong bit), u8 (the height from the wrong body bytes), u9 (`demoV` drops a signature check) and u10 (two Reason names swapped): KILLED. u11 (the length check `>` for `!=`) is EQUIVALENT: a short hex field still fails at its NUL. | DONE |
| S11 | T4: the JSON report, `docs/REPORT-JSON.md`. On the USER commit ae31fa7. `langc [--kit] report PROG` writes JSON Lines in RFC 8785 form (Q-S11-1 to Q-S11-3 (a)): a `program` line, a `rule` line for each decided rule definition, a `policy` line for each closed `Policy` definition, and a `log` line and its `height` lines for each closed `Log` definition (Q-S11-7 (a)). The check pass keeps a RuleFacts record of each decided definition, so the report does not decide again: the class sizes, f, the least total, the overlap, the accept count and the minimal accepting vectors (at most 64 listed, Q-S11-4 (a)). The rule line gives the O4 and O5 margins, `read_set` = n - least + f + 1 (Q-S11-5 (a)) and the leakage fields (Q-S11-6 (a), OI-5). `src/jsonout.c`, `host/jsonin.c` and `host/jcs.c` are copies of permit-lang. Gate: 7 report rows, 2 report write errors, 5 json escapes (Q-S11-10 (a)); the 33 count lines of S10 do not change. Mutants M1 (the O4 margin), M2a and M2b (the minimal test), M3 (the read set), M4 (the `"` escape, killed by `build/jsontool`), M5 (no JCS round trip), M6 (the cap), M7a (the height policy) and M8 (height 0): KILLED. M7b (the height committee from `amTo`) is EQUIVALENT: the count vector counts the approval set, and the committee is only the motive index of that elimination. | DONE |
| S12 | T2: the TFHE tally circuit, `docs/TFHE-CIRCUIT.md`. On the USER commit 591a3f9. `langc [--kit] tfhe PROG` writes JSON Lines in RFC 8785 form (Q-S12-5 (a)): a `program` line and a `circuit` line for each decided rule definition. The circuit has one input bit for each member and gives only the accept bit (Q-S12-1 (a), OI-5): a popcount for each class, a constant comparator chain for each threshold, an AND tree for each minimal vector and an OR tree over the minimal vectors (Q-S12-2 (a)). RuleFacts keeps up to 4096 minimal vectors (`TFHE_MIN_CAP`); T4 still lists 64. The gates are AND, OR, XOR, NAND, NOR and XNOR (one bootstrap each) and NOT (no bootstrap) (Q-S12-3 (a)). Each line gives the depth D and the static bound B from n_j, k and M; D > B is a writer fault, and `REFUSE_DEPTH` at `langc tfhe` only refuses more than 65536 gates or more than 4096 minimal vectors (Q-S12-4 (a)). Gate: 7 tfhe rows, 1 tfhe refusal and 19 tfhe sims (`build/tfhetool`, Q-S12-6 (a)); the 36 count lines of S11 do not change. Mutants M1 (the comparator), M2 (the OR tree cut at 64), M3 (the popcount carry), M5b (B = 0), M6 (m_j = 0 read as 1), M6s (the m_j = 0 skip removed: the golden bytes only) and M8 (no JCS round trip): KILLED. M7a and M7b (the caps off by one): KILLED by a scratch driver, not by the gate, because no guest row is at a cap. M4 (NOT counted in D) and M5 (no D <= B check) are EQUIVALENT: the builder writes no NOT row, and D <= B holds by the bound proof in `src/front/tfhe.h`. | DONE |
| S13 | T3: R_ballot and R_dec as Noir (Q-S0-13 (b)), golden files. | |
| S14 | A1: canonical forms and `langc fmt`. | |
| S15 | A2: `langc check --json`. | |
| S16 | A4: `langc mcp`. | |
| S17 | A3 and A5: SKILL.md, G1 to G5, AGENTS.md and the README table final. | |
| S18 | A6: the token comparison. | |
| S19 | Close: SPEC.md, probe/CAPABILITY.md, VALIDATION.md, final gate. | |

## 13. Open items

The design obligations are O1 to O11. The open items of this plan are
OI-1 onward, so the two lists do not clash. OI-1 to OI-5 are the items of
design sec 13. They stay open items, not v1 features (Q-S0-15 (a)).

- OI-1 Committee loss can freeze the system (design sec 13). Recovery needs
  a clock. Not a v1 feature.
- OI-2 Stale certificates (design sec 13). `verify` takes the latest
  verified height as an input; the freshness rule is a later ruling.
- OI-3 Agenda control (design sec 13). The log records the order only.
- OI-4 Receipts (design sec 13). ZK and FHE give ballot secrecy, not
  receipt-freeness.
- OI-5 The output leaks what the rule implies (design sec 13). T4 states it
  for each rule. At T1 the output also leaks the count vector c (Q-S1-2
  (a)). T2 gives the one-bit output. S11: each `rule` line of T4 has
  `leak_t1`, `accept_vectors`, `reject_vectors`, `accept_reveals_all`
  and `reject_reveals_all` (Q-S11-6 (a), `docs/REPORT-JSON.md`). S12: the
  T2 circuit gives only the accept bit, and the count vector c stays
  inside the circuit (Q-S12-1 (a), `docs/TFHE-CIRCUIT.md`).
- OI-6 CLOSED at S3. The kit row "a trap is equal to a trap" (section 2)
  is gone: in the Wasm and differential rows a `trap` line is a FAIL. The
  row `pick 2 4 9` gives 4 and does not trap: the entry sets a Flag
  argument of 2 or more to 1 (`flag_norm`, mutant M4 kills it).
- OI-7 The view equations hold only up to indistinguishability and need a
  computational model (design sec 13). They stay ASSUMED.
- OI-8 The class-preserving group action is new Lean work on the UAT side
  (design sec 3, O1). Lean export is not a target.
- OI-9 Availability and the MPC tally route (design sec 9 and sec 10) are
  implementation choices. No target emits them in v1.
- OI-10 The node and wasmtime oracles of the differential rows need the
  crypto imports (Q-S1-1 (a)). S9 (Q-S9-4 (a)): node takes the answers
  from the host table (`test/run-wasm.mjs --host FILE`). S10 (Q-S10-1,
  Q-S10-2 and Q-S10-6 (a)): the operand bytes are in the host region below
  the heap (section 8.2), the engine of `ratifyd` gives the real import
  bodies, and wasmtime CLI rows cover each module without imports. Open:
  wasmtime for a module with imports.
- OI-11 The native field of Noir is the BN254 scalar field. The
  coordinates of a BN254 G1 point are in the base field, so the Noir
  R_ballot and R_dec of the ElGamal route need non-native field arithmetic
  (Q-S0-13 (b)). S13 measures the cost.
- OI-12 The gate gap from S2 to S3 (Q-S2-1 (a)). `src/front/lower.c`,
  `src/wasm.c` and `src/ir.h` are out of the build, and `langc build` gives
  `REFUSE_LOWER` "target pending S3". Until S3 the gate has no IR rows, no
  Wasm modules and no differential cases. CLOSED at S3: the gate has the
  IR rows, 33 Wasm modules and 998 differential cases with 0 mismatches.
  The kit had 13 modules and 813 cases. The list now holds the 30
  examples, `test/ir/natops.lang`, `test/ir/block.lang` and `test/ir/proof-trap.lang`;
  `natops.lang` gives 610 cases and `examples/entries.lang` 310.
- OI-13 `REFUSE_ORBIT` (S4) also walks the quoted type of a definition.
  The quote unfolds `count c S` on an open `S` to an elim on `Approvals`,
  so a definition that is not a law and has `count c S` in its type gives
  `REFUSE_ORBIT` (S4 probe). (a), recommended: S5 `Rule` does not put
  `count c S` in the type of a definition that is not a law. (b): S5
  drops the quoted-type walk of `REFUSE_ORBIT`. Closed by Q-S5-3 (a)
  (S5): the walk stays, and no S5 rule puts `count c S` in the type of a
  definition that is not a law.
- OI-14 The O5 walk (S5) is quadratic in the minimal accepting vectors:
  up to C(20, 10) = 184756 vectors at 2^20 for 20 classes of 1. CLOSED by
  Q-S8-6 (b) (S8): one upward pass, O(space k), `rule_margins` in
  `src/front/check.c`. 20 classes of 1 check in 12.0 s
  (`examples/fork-margin.lang`).
- OI-15 The seal of `hoRule` and `ffRule` (S6, Q-S6-1 (a)) walks the
  written term. CLOSED in the S6 review: the walk respects lambda, Pi and
  Sigma binders in their bodies while checking each binder's own type
  outside its scope. C regressions cover shadowing and constructor uses
  in binder annotations.
- OI-16 `verify` does not lower: the kit lowering of `verify` gives a
  stuck elim with a function motive (S10 survey L, option (a)). So the demo
  is split (Q-S10-U3-L (c)): the Wasm entry `demo` gives W, and `demoV`
  runs `verify` in the evaluator only (section 8.4). Open.
- OI-17 `demoV` on a large height word reaches the arena limit of the
  evaluator (`langc: OOM`, S10 mutant u8). `demotool` then gives exit 1,
  not `wrongHeight`. Open.

## 14. Questions for the USER (S0)

Each question has a recommendation (rec). S1 recorded the rulings
(2026-11-03, section 1).

- Q-S0-1 Rejections. RULED (a). (a) rec: `Verdict = valid | invalid Reason` with the
  eight reasons of section 2. (b) `Bool` only, and the host prints the
  reason.
- Q-S0-2 `reconstruct`. RULED (a). (a) rec: the type carries |T| >= t (a `Quorum t`
  value). (b) A result sum `reconstructed s | tooFew`.
- Q-S0-3 `policyAt` past the log end. RULED (a). (a) rec: the height is `Fin (length
  + 1)` of a given log. (b) The last policy for each h past the end.
- Q-S0-4 Bound of the count space. RULED (a). (a) rec: `REFUSE_BOUND` when the product
  of (n_j + 1) is above 2^20, so O3 to O5 stay finite and fast. (b) No
  bound; O4 and O5 in closed form for plain thresholds only.
- Q-S0-5 R1 classes. RULED (a). (a) rec: `Log` CM, `System` CC, `Enc` and `Share` the
  identity CM at base types, `Proof` and `Protocol` base sorts, TCM-D
  ASSUMED (section 3). (b) `Protocol` as a CM (the free category on
  stages), with TCM-D still ASSUMED.
- Q-S0-6 Refusal codes. RULED (a). (a) rec: the eleven codes of section 7. (b) Fewer
  codes: one `REFUSE_POLICY` for O3 to O5.
- Q-S0-7 Rule syntax. RULED (a). (a) rec: a threshold for each class, `and`, `or`, and
  a table of count vectors; the checker computes the minimal accepting
  vectors. (b) Any total function on `Counts`, with O3 decided by
  enumeration.
- Q-S0-8 Law counts. RULED (a). (a) rec: the four buckets of section 6;
  `laws_discharged` counts DISCHARGED rows only. (b) One `laws_total` for
  all rows, with ASSUMED rows listed as not discharged.
- Q-S0-9 Crypto layer at T1. RULED (b). (a) rec: abstract sorts with laws; the
  reference evaluator computes the denotation (design sec 5); the Wasm
  module takes opaque handles from `ratifyd` imports; real FHE and ZK
  arrive at T2 and T3. (b) Real crypto in the v1 module (the quorum-lang
  exponential ElGamal route, quorum-lang be2c329 docs/PLAN.md line 392).
- Q-S0-10 Kernel source. RULED (a). (a) rec: the permit-lang kernel without the affine
  layer (permit-lang took it from relay-lang). (b) The quorum-lang kernel.
  (c) Extend the kit checker in place.
- Q-S0-11 Wasm engine in `ratifyd`. RULED (a). (a) rec: the own C99 interpreter of the
  relay-lang ruling (copy the permit-lang engine); node and wasmtime are
  oracles in the gate. (b) Vendor wasm3. (c) The wasmtime C API.
- Q-S0-12 T2 scheme. RULED (a). (a) rec: TFHE (Boolean gates, bootstrapping, the depth
  bound is per gate). (b) BFV (a levelled circuit, the depth bound is the
  multiplicative depth).
- Q-S0-13 T3 format. RULED (b). (a) rec: Circom (R1CS). (b) Noir.
- Q-S0-14 Token comparison. RULED (a). (a) rec: the permit-lang ruling (count_tokens,
  equal outputs). (b) An offline tokenizer.
- Q-S0-15 Design sec 13 items. RULED (a). (a) rec: open items OI-1 to OI-5, not v1
  features. (b) Pick one for v1 (for example, the freshness rule of OI-2).

### Questions from Q-S0-9 (b) (S1)

- Q-S1-1 Where the T1 crypto runs. RULED (a). (a) rec: the port of
  quorum-lang src/crypto lives in `ratifyd`; the module calls it through a
  fixed host-import ABI on real values (the quorum-lang Q9 (a) ruling).
  (b) BN254 arithmetic as guest code, lowered into the Wasm module.
- Q-S1-2 The T1 tally output. RULED (a). (a) rec: T1 decrypts the count
  vector c and applies the rule in the clear; the T1 leakage is c; T2
  gives the one-bit output. (b) Only b at T1, by an MPC comparison (OI-9
  in v1).
- Q-S1-3 Signature and hash. RULED (a). (a) rec: Schnorr on BN254 G1 with
  a keccak-256 challenge; keccak-256 for the entry hash. (b) BIP-340
  Schnorr on secp256k1 with SHA-256.
