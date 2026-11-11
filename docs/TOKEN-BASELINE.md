# Token baseline (S18)

This file compares the tokens of the worked examples G1 to G5 in
ratify-lang, Python and Rust (PLAN 9, row A6). The counts in this file
are an offline estimate: ceil(bytes / 4) of each file (Q-S18-2 (a), the
permit-lang route). They are not tokenizer counts. The API route (`make
tokens`: the Anthropic `count_tokens` endpoint with the newest Claude
model, Q-S0-14 (a), Q-S18-2 (a)) is not run, because no API Console key
is available. `make check` checks the file hashes and the estimate block
in this file, with no key and no network (Q-S18-3 (a)).

Status: the estimate is from 2026-11-11 (`build/tokcount --estimate`).
The API route is not run.

## The task

Each G is one worked example of `skills/ratify-lang/SKILL.md` (S17). The
3 programs of a G do the same task and print the same lines (Q-S18-1
(a)):

| G | task | eval rows |
| :--- | :--- | :--- |
| G1 | a plain t-of-n policy (guest mode) | `okAuth`, `noAuth` |
| G2 | two classes and a veto (guest mode) | `vetoNo`, `vetoYes`, `okAuth`, `noAuth` |
| G3 | an amendment that changes the amend rule (guest mode) | `by3P0`, `by3P1`, `by4P1` |
| G4 | the replay of a log of amendments (guest mode) | `amend3At0`, `amend3At1` |
| G5 | the demo of the verifier host (kit mode) | `demoV` with 3 argument rows |

The 5 G have 14 eval rows. G5 is the demo: `g5-demo.lang` is the `langc
fmt` text of `test/demo/demo.lang` (Q-S17-1 (a)). Thus the 5 programs
are "G1 to G5 and the demo" of row A6.

## The files

- `examples/skill/g1-threshold.lang` to `g5-demo.lang`: the ratify-lang
  programs (S17). The count is of the `.lang` file only.
  `examples/skill/kit.txt` names the 1 kit file, `g5-demo.lang`.
- `test/baseline/g1-threshold.py` to `g5-demo.py`: the same tasks in
  Python, with the standard library only (Q-S18-3 (a)).
- `test/baseline/rust/src/bin/g1.rs` to `g5.rs`: the same tasks in Rust,
  1 crate (`ratify-baseline`) with 5 bins (Q-S18-3 (a)). The crate has
  no dependency. `Cargo.lock` pins the build.

Each Python and each Rust program prints the eval lines of its G in the
SKILL.md order. The lines must be equal, byte for byte, to the output of
`langc eval` (G1 to G4) or `langc --kit eval` (G5). `make tokens` checks
this before it counts.

## The method

- Estimate: `build/tokcount --estimate` reads the full text of each
  file, comments included. The estimate of a file is ceil(bytes / 4),
  that is the integer quotient of bytes plus 3 by 4 (Q-S18-2 (a)). It
  needs no key and no network. An estimate is not a tokenizer count: it
  has no model and no message frame.
- Request (the API route): `POST /v1/messages/count_tokens` with 1 user
  message. The message is the full text of 1 file, comments included.
  The reply is `{"input_tokens": N}`.
- Model (the API route): the model with the newest `created_at` in the
  reply of `GET /v1/models` on the run date (Q-S18-2 (a)).
- Frame (the API route): N includes the tokens of the message frame.
  Thus `make tokens` also counts a fixed 1-byte message, `x`. The net
  count of a file is N minus the count of that message (Q-S18-2 (a)).
- Ratios: each ratio divides 2 estimates now (2 net counts on the API
  route), half up, with 2 decimals. One line gives each Python file and
  each Rust file over the ratify-lang file of the same G. The last 2
  lines give the sum of each language over the sum of the 5 ratify-lang
  files. Each file is a full alternative, thus the table has no total
  row.
- Hashes: the SHA-256 of each counted file. `make check` runs
  `build/tokcount --check docs/TOKEN-BASELINE.md`. Each hash in the
  table must be the hash of the file now. `make check` also compares the
  lines between the markers with the output of `build/tokcount
  --estimate`. Thus a changed program fails the gate until a new
  estimate block replaces the old one.

## The run

The estimate: from the repo root, offline, with no key:

```
build/tokcount --estimate
```

The output is the block between the markers below, byte for byte.

The API route (not run; it needs an API Console key): from the repo
root, outside the sandbox, with `ANTHROPIC_API_KEY` set:

```
make tokens
```

The rule does these steps:

1. It builds the Rust crate (`RUST_BUILD`, by default `cargo build -j
   2`).
2. It runs each program and compares its output with `langc eval`.
3. It gets the model list and selects the model (`build/tokcount
   --model`).
4. It sends 1 request for the frame message and 1 request for each of
   the 15 files (`build/tokcount --request` makes each body). It keeps
   each reply in `build/tokens/`.
5. It writes the table with `build/tokcount --table build/tokens`.

Now the lines between the markers below come from `build/tokcount
--estimate`. On the API route, step 5 replaces them.

## The table

Model: none (an offline estimate). Date: 2026-11-11.

<!-- tokcount begin -->
| file | bytes | estimate | sha256 |
| :--- | ---: | ---: | :--- |
| examples/skill/g1-threshold.lang | 1232 | 308 | d0fa09986e85a774ba9275a51b41ed53ac542da9dec73da6e50ef508dfc74be5 |
| test/baseline/g1-threshold.py | 752 | 188 | 2b84c2df74f3e6358417729b85c8e4f57d2f3a6748000536b7edf7b0be3e47e0 |
| test/baseline/rust/src/bin/g1.rs | 1004 | 251 | 07983b4f51e823aa514112a4e4dca75b070525a0372713abcdeea5e2ce3e94c7 |
| examples/skill/g2-veto.lang | 2155 | 539 | 9576b1b64279d84cf6e0f560aeba517cf1ff03c3ca1dbc95d066f87742c83f36 |
| test/baseline/g2-veto.py | 1058 | 265 | faed292beb9ff834b7d9fddab8d92e37ef20b6ce368ac0a43c42bc74d5e63c20 |
| test/baseline/rust/src/bin/g2.rs | 1542 | 386 | 5da17da8166e288f66cc9327176d4310f0e11420e2a34819ddd0ae8d800c934e |
| examples/skill/g3-amend-rule.lang | 1837 | 460 | 1bdd1732ac2d6ac7c4adb81d00401c17d71c3aadd229b9031c8f7b5adf90efca |
| test/baseline/g3-amend-rule.py | 860 | 215 | 05ffb80e922efa57bb08d4db990d559f5a21546f74a7d1c8481cf0580ecc58fd |
| test/baseline/rust/src/bin/g3.rs | 1151 | 288 | 1f2b85fdd5c7b2518a2d13cf7985682a0d51e5d79405e362337e161269f375b3 |
| examples/skill/g4-replay.lang | 3006 | 752 | 97fe8e0ff7899fb723fc1c31be2b8121eedc055121dacb58b529dfc9da19c42e |
| test/baseline/g4-replay.py | 1290 | 323 | e5694a8c9ad69fd2399a8462cb2673eca9938c2682e86a88181805823414d265 |
| test/baseline/rust/src/bin/g4.rs | 1894 | 474 | f7a2e770bb9ff613ef88c709210ad57b0550b7739f73e008bd8c391161c71ee2 |
| examples/skill/g5-demo.lang | 8148 | 2037 | 5d544b3793f89f19561847bdacfccecda480c3765e93179360a476d18bd1707d |
| test/baseline/g5-demo.py | 2641 | 661 | 436602eaa2bf99f4f476bd457845693f8aa31b0e33abf55f0f541540f6fc39c8 |
| test/baseline/rust/src/bin/g5.rs | 3921 | 981 | 4eca28e751d63e544280a33f2f42cee68bcbe1b9963107e8a65d20bf637d5502 |

estimate: ceil(bytes / 4) of each file; this is not a tokenizer count.

ratio of the estimates (test/baseline/g1-threshold.py over examples/skill/g1-threshold.lang): 0.61
ratio of the estimates (test/baseline/rust/src/bin/g1.rs over examples/skill/g1-threshold.lang): 0.81
ratio of the estimates (test/baseline/g2-veto.py over examples/skill/g2-veto.lang): 0.49
ratio of the estimates (test/baseline/rust/src/bin/g2.rs over examples/skill/g2-veto.lang): 0.72
ratio of the estimates (test/baseline/g3-amend-rule.py over examples/skill/g3-amend-rule.lang): 0.47
ratio of the estimates (test/baseline/rust/src/bin/g3.rs over examples/skill/g3-amend-rule.lang): 0.63
ratio of the estimates (test/baseline/g4-replay.py over examples/skill/g4-replay.lang): 0.43
ratio of the estimates (test/baseline/rust/src/bin/g4.rs over examples/skill/g4-replay.lang): 0.63
ratio of the estimates (test/baseline/g5-demo.py over examples/skill/g5-demo.lang): 0.32
ratio of the estimates (test/baseline/rust/src/bin/g5.rs over examples/skill/g5-demo.lang): 0.48
ratio of the estimates (the 5 Python files over the 5 ratify-lang files): 0.40
ratio of the estimates (the 5 Rust files over the 5 ratify-lang files): 0.58
<!-- tokcount end -->

Result: for the 5 G together, the estimate of the Python files is 0.40
times the estimate of the ratify-lang files, and the estimate of the
Rust files is 0.58 times the estimate of the ratify-lang files.

## Notes

- Versions: the estimate uses no tool version; it reads only the bytes
  of the files. On the API route, the `python3`, `rustc` and `cargo`
  versions come from `build/tokens/versions.txt`.
- A `make tokens` run that replaces the estimate block must also remove
  the estimate compare of `make check` (the 2 Makefile lines after
  `build/tokcount --check`).
- The crate has no dependency. Thus G1 to G5 use only the Rust standard
  library.
- Run `make tokens` outside the sandbox. In the sandbox, the Rust build
  stops at `sccache rustc -vV`.
