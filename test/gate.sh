#!/bin/sh
# The kit gate. `make check` runs it from the kit directory after the build.
set -eu
fail=0
count=0
# kit LIST FILE prints --kit when FILE is a line of LIST. A row whose text uses a
# kit name or a Nat or word literal is a kit row (orbit-lang S3); the others run
# in guest mode.
kit() {
  if [ -f "$1" ] && rg -q -x -F "$2" "$1"; then echo --kit; fi
}
for prog in examples/*.lang; do
  out=$(build/langc $(kit examples/kit.txt "${prog#examples/}") check "$prog" 2>&1) || true
  count=$((count + 1))
  if [ "$out" != "ok" ]; then
    echo "FAIL $prog: $out"
    fail=$((fail + 1))
  fi
done
echo "examples: $count checked, $fail failed"

tmp=${TMPDIR:-/tmp}/langc-gate.$$
mkdir -p "$tmp"
awk 'BEGIN { printf "def deep : Nat := " } { } END {
  for (i = 0; i < 1100; i++) printf "("; printf "1"; for (i = 0; i < 1100; i++) printf ")"; print "" }' </dev/null >"$tmp/deep.lang"

# refusals DIR LABEL: each line of DIR/expect.txt is `FILE CODE`. `langc check`
# must exit 1 and print `langc: CODE: ...` on stderr.
refusals() {
  checked=0
  while read -r file code; do
    path="$1/$file"
    [ "$file" = "deep.lang" ] && path="$tmp/deep.lang"
    status=0
    build/langc $(kit "$1/kit.txt" "$file") check "$path" 2>"$tmp/err" >/dev/null || status=$?
    first=$(head -n 1 "$tmp/err")
    checked=$((checked + 1))
    case "$status:$first" in
      "1:langc: $code: "*) ;;
      *) echo "FAIL $file: want exit 1 and $code, got exit $status: $first"; fail=$((fail + 1)) ;;
    esac
  done <"$1/expect.txt"
  echo "$2 refusals: $checked checked"
}
refusals test/parse parse
refusals test/check check

# A REFUSE_KIT row refuses a kit item in guest mode only: `langc --kit check`
# must print ok.
kit_only=0
while read -r file code; do
  [ "$code" = "REFUSE_KIT" ] || continue
  status=0
  out=$(build/langc --kit check "test/check/$file" 2>&1) || status=$?
  kit_only=$((kit_only + 1))
  if [ "$status:$out" != "0:ok" ]; then
    echo "FAIL $file: want ok under --kit, got exit $status: $out"
    fail=$((fail + 1))
  fi
done <test/check/expect.txt
echo "kit-only refusals: $kit_only checked"

# A refl refusal shows both normal forms.
build/langc $(kit test/check/kit.txt bad-refl.lang) check test/check/bad-refl.lang 2>"$tmp/err" >/dev/null || true
if ! rg -q -F 'the path at i1 is some 1201, but the type needs some 1200' "$tmp/err"; then
  echo "FAIL bad-refl.lang: the message does not show both normal forms"
  fail=$((fail + 1))
fi

# A REFUSE_CLOCK refusal names the op-count form (S3, O3).
time_hint=0
build/langc $(kit test/check/kit.txt time-block.lang) check test/check/time-block.lang 2>"$tmp/err" >/dev/null || true
time_hint=$((time_hint + 1))
if ! rg -q -F 'count operations: a position is `len r` (an OpPos) and a deadline is a count of later operations' "$tmp/err"; then
  echo "FAIL time-block.lang: the message does not name the op-count form"
  fail=$((fail + 1))
fi
echo "time hint: $time_hint checked"

# Each line of test/eval/expect.txt is `FILE NAME [ARGS] => OUTPUT`. `langc eval`
# must exit 0 and print OUTPUT.
evals=0
while IFS= read -r line; do
  want=${line#* => }
  set -- ${line%% => *}
  file=$1
  name=$2
  shift 2
  status=0
  got=$(build/langc $(kit examples/kit.txt "$file.lang") eval "examples/$file.lang" "$name" "$@" 2>"$tmp/err") || status=$?
  first=$(head -n 1 "$tmp/err")
  evals=$((evals + 1))
  case "$status|$got|$first" in
    "0|$want|"*) ;;
    *) echo "FAIL eval $file $name: want $want, got exit $status: $got $first"; fail=$((fail + 1)) ;;
  esac
done <test/eval/expect.txt
echo "evals: $evals checked"

# Each line of `build/cryptotool rep` is `crypto NAME BITS => COUNT` (S7, TCM-C):
# COUNT is the threshold open of the sum of the casts of the bits. `langc eval`
# must exit 0 and print COUNT.
reps=0
status=0
build/cryptotool rep >"$tmp/rep.txt" 2>"$tmp/err" || status=$?
if [ "$status" -ne 0 ]; then
  echo "FAIL cryptotool rep: exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
while IFS= read -r line; do
  want=${line#* => }
  set -- ${line%% => *}
  file=$1
  name=$2
  shift 2
  status=0
  got=$(build/langc $(kit examples/kit.txt "$file.lang") eval "examples/$file.lang" "$name" "$@" 2>"$tmp/err") || status=$?
  first=$(head -n 1 "$tmp/err")
  reps=$((reps + 1))
  case "$status|$got|$first" in
    "0|$want|"*) ;;
    *) echo "FAIL crypto rep $name $*: want $want, got exit $status: $got $first"; fail=$((fail + 1)) ;;
  esac
done <"$tmp/rep.txt"
echo "crypto rep: $reps checked"

# eval_refused STATUS CODE NAME [ARGS]: `langc --kit eval examples/entries.lang
# NAME ARGS` must exit STATUS and print `langc: CODE: NAME: ...` on stderr.
eval_refused() {
  want_status=$1
  code=$2
  shift 2
  status=0
  build/langc --kit eval examples/entries.lang "$@" 2>"$tmp/err" >/dev/null || status=$?
  case "$status:$(head -n 1 "$tmp/err")" in
    "$want_status:langc: $code: $1: "*) ;;
    *) echo "FAIL eval $*: want exit $want_status and $code, got exit $status"; fail=$((fail + 1)) ;;
  esac
}
eval_refused 2 EVAL_ARGS addPrice 1
eval_refused 2 EVAL_ARGS addPrice x 1
eval_refused 2 EVAL_ARGS addPrice 18446744073709551616 1
eval_refused 2 EVAL_ARGS pick 2 4 9
eval_refused 1 EVAL_ENTRY nope

# host_eval STATUS WANT TABLE NAME [ARGS] (S9 step d1b): `langc --kit eval
# test/eval/host.lang NAME ARGS --host test/eval/TABLE` (no --host for TABLE
# -) must exit STATUS. At STATUS 0 it prints WANT, else stderr starts with
# `langc: WANT`. A TABLE that starts with / is used as is (S9 step d2a).
host_evals=0
host_eval() {
  want_status=$1
  want=$2
  table=$3
  shift 3
  status=0
  case "$table" in
    -) build/langc --kit eval test/eval/host.lang "$@" >"$tmp/out" 2>"$tmp/err" || status=$? ;;
    /*) build/langc --kit eval test/eval/host.lang "$@" --host "$table" >"$tmp/out" 2>"$tmp/err" || status=$? ;;
    *) build/langc --kit eval test/eval/host.lang "$@" --host "test/eval/$table" >"$tmp/out" 2>"$tmp/err" || status=$? ;;
  esac
  host_evals=$((host_evals + 1))
  case "$want_status:$status:$(cat "$tmp/out"):$(head -n 1 "$tmp/err")" in
    "0:0:$want:") ;;
    "$want_status:$want_status::langc: $want"*) ;;
    *) echo "FAIL host eval $table $*: want exit $want_status and $want, got exit $status"; fail=$((fail + 1)) ;;
  esac
}
host_eval 0 "inr unit" host-table.txt sigYes
host_eval 0 "inl unit" host-table.txt sigNo
host_eval 0 42 host-table.txt joinShares 1 2 3 4 5 6 7 8
host_eval 1 "HOST: sigMiss: no host row for sig_verify" host-table.txt sigMiss
host_eval 1 "HOST: hashMiss: no host row for hash_eq" host-table.txt hashMiss
host_eval 1 "HOST: sigYes: no host row for sig_verify" host-empty.txt sigYes
host_eval 1 "HOST: sigYes: the host import sig_verify has no host table" - sigYes
host_eval 2 "HOST_TABLE: -: line 2: sig_verify takes 6 words and one result" host-bad.txt sigYes

# `build/cryptotool host` (S9 step d2a) writes the host table of the six
# crypto imports over its fixture image. The rows below eval host.lang on it.
status=0
build/cryptotool host >"$tmp/host-crypto.txt" 2>"$tmp/err" || status=$?
if [ "$status" -ne 0 ]; then
  echo "FAIL cryptotool host: exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
echo "host table rows: $(awk '!/^#/ {n++} END {print n + 0}' "$tmp/host-crypto.txt") written"
# `build/cryptotool image` (S10 U2) writes the same fixture as `OFF HEX`
# lines: the image that build/ratifyd puts in the host region.
status=0
build/cryptotool image >"$tmp/host-image.txt" 2>"$tmp/err" || status=$?
if [ "$status" -ne 0 ]; then
  echo "FAIL cryptotool image: exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
host_eval 0 "inr unit" "$tmp/host-crypto.txt" sigGood
host_eval 0 "inl unit" "$tmp/host-crypto.txt" sigBad
host_eval 0 "inl unit" "$tmp/host-crypto.txt" sigOff
host_eval 0 "inr unit" "$tmp/host-crypto.txt" hashGood
host_eval 0 "inl unit" "$tmp/host-crypto.txt" hashBad
host_eval 0 "inr unit" "$tmp/host-crypto.txt" tallyZero
host_eval 0 "inr unit" "$tmp/host-crypto.txt" shareGood
host_eval 0 "inr unit" "$tmp/host-crypto.txt" shareGood2
host_eval 0 "inl unit" "$tmp/host-crypto.txt" shareBad
host_eval 0 "inr unit" "$tmp/host-crypto.txt" ballotGood
host_eval 0 "inl unit" "$tmp/host-crypto.txt" ballotBad
host_eval 0 "inr unit" "$tmp/host-crypto.txt" tallyReal
host_eval 0 2 "$tmp/host-crypto.txt" combineOne
host_eval 0 3 "$tmp/host-crypto.txt" combineSum
host_eval 0 0 "$tmp/host-crypto.txt" combineBad
echo "host evals: $host_evals checked"

# The target (S3). The Wasm writer is in the build: `langc build
# examples/functions.lang` must exit 0 and write a module that wasm-validate
# accepts.
builds=0
status=0
build/langc $(kit examples/kit.txt functions.lang) build examples/functions.lang -o "$tmp/functions.wasm" 2>"$tmp/err" || status=$?
builds=$((builds + 1))
if [ "$status" -ne 0 ] || ! wasm-validate "$tmp/functions.wasm" 2>>"$tmp/err"; then
  echo "FAIL build functions: want exit 0 and a valid module, got exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
echo "builds: $builds checked"

# The demo (S10 U3 Part A, test/demo). demo.lang uses Nat, so it is a kit row:
# `langc --kit check` must print ok, `langc --kit build` must write a valid
# module, and `langc --kit abi --text` must give one entry, `demo` (demoV has
# an Option Nat result, so the build does not lower it).
demos=0
status=0
out=$(build/langc --kit check test/demo/demo.lang 2>&1) || status=$?
demos=$((demos + 1))
[ "$status:$out" = "0:ok" ] || { echo "FAIL demo check: want ok, got exit $status: $out"; fail=$((fail + 1)); }
status=0
build/langc --kit build test/demo/demo.lang -o "$tmp/demo.wasm" 2>"$tmp/err" || status=$?
demos=$((demos + 1))
if [ "$status" -ne 0 ] || ! wasm-validate "$tmp/demo.wasm" 2>>"$tmp/err"; then
  echo "FAIL build demo: want exit 0 and a valid module, got exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
status=0
out=$(build/langc --kit abi test/demo/demo.lang --text 2>&1) || status=$?
demos=$((demos + 1))
[ "$status:$out" = "0:demo" ] || { echo "FAIL demo abi: want one entry demo, got exit $status: $out"; fail=$((fail + 1)); }
# policies.lang is the policy defs of demo.lang with no Nat (Q-S10-U3-6 (a)).
# Guest `langc check` must print ok, so rule_def decides O3, O4 and O5 for pBase,
# pAmd1 and pAdd. The sync row: with the `--` lines removed, policies.lang must
# be the same as demo.lang up to the `-- A2:` line.
status=0
out=$(build/langc check test/demo/policies.lang 2>&1) || status=$?
demos=$((demos + 1))
[ "$status:$out" = "0:ok" ] || { echo "FAIL demo policies check: want ok, got exit $status: $out"; fail=$((fail + 1)); }
awk '!/^--/' test/demo/policies.lang >"$tmp/pol"
awk '/^-- A2:/ { exit } !/^--/' test/demo/demo.lang >"$tmp/dpol"
demos=$((demos + 1))
cmp -s "$tmp/pol" "$tmp/dpol" || { echo "FAIL demo policies sync: the policy defs of policies.lang and demo.lang are not the same"; fail=$((fail + 1)); }
echo "demo rows: $demos checked"

# The demo verdict rows (S10 U3 Part D1). `build/cryptotool demo DIR` writes the
# images D1..D8 (the RESUME18 table); D9 is D1 with the word record target 4 (no
# image gives illTypedPolicy). build/demotool runs the module and then demoV on
# each image: its `W VERDICT` line must be line k of test/demo/want.txt.
verdicts=0
mkdir "$tmp/dimg"
status=0
build/cryptotool demo "$tmp/dimg" >/dev/null 2>"$tmp/err" || status=$?
[ "$status" -eq 0 ] || { echo "FAIL demo images: want exit 0, got exit $status: $(head -n 1 "$tmp/err")"; fail=$((fail + 1)); }
awk '$1 == "target" { $2 = 4 } 1' "$tmp/dimg/D1.txt" >"$tmp/dimg/D9.txt"
for k in 1 2 3 4 5 6 7 8 9; do
  want=$(awk -v k="$k" 'NR == k' test/demo/want.txt)
  status=0
  got=$(build/demotool "$tmp/demo.wasm" test/demo/demo.lang "$tmp/dimg/D$k.txt" 2>&1) || status=$?
  verdicts=$((verdicts + 1))
  [ "$status:$got" = "0:$want" ] || { echo "FAIL demo verdict D$k: want $want, got exit $status: $got"; fail=$((fail + 1)); }
done
echo "demo verdict rows: $verdicts checked"
# The malformed rows (Part D2): D1 with an unknown KIND, a missing KIND, and the
# body record one byte longer and one byte shorter than its slot. demotool must
# refuse each before the module runs.
malformed=0
awk 'NR == 1 { $1 = "pkX" } 1' "$tmp/dimg/D1.txt" >"$tmp/dimg/m-unknown"
awk 'NR != 2' "$tmp/dimg/D1.txt" >"$tmp/dimg/m-missing"
awk '$1 == "body" { $2 = $2 "00" } 1' "$tmp/dimg/D1.txt" >"$tmp/dimg/m-longer"
awk '$1 == "body" { $2 = substr($2, 1, length($2) - 2) } 1' "$tmp/dimg/D1.txt" >"$tmp/dimg/m-shorter"
for m in unknown missing longer shorter; do
  status=0
  got=$(build/demotool "$tmp/demo.wasm" test/demo/demo.lang "$tmp/dimg/m-$m" 2>&1) || status=$?
  malformed=$((malformed + 1))
  [ "$status:$got" = "0:- invalid malformed" ] || { echo "FAIL demo malformed $m: want - invalid malformed, got exit $status: $got"; fail=$((fail + 1)); }
done
echo "demo malformed rows: $malformed checked"
# The bit-order rows (Part D3): demoV on W with the bit k clear and the D1 words
# (height 2, action 7, amend 0, target 0, signers 31) gives the Reason word of
# check k: 5 badSignature (k 0..4), 4 badPrevHash (5), 6 badProof (6), 8
# equivocation (7). The bits 8..12 count only when bit 7 is clear (W 8063 gives
# 8): then each one clear gives 0 (the equivocation is not proved).
bits=0
for row in 8190:5 8189:5 8187:5 8183:5 8175:5 8159:4 8127:6 8063:8 7807:0 7551:0 7039:0 6015:0 3967:0; do
  status=0
  got=$(build/langc --kit eval test/demo/demo.lang demoV "${row%%:*}" 2 7 0 0 31 2>&1) || status=$?
  bits=$((bits + 1))
  [ "$status:$got" = "0:${row#*:}" ] || { echo "FAIL demo bit row W ${row%%:*}: want ${row#*:}, got exit $status: $got"; fail=$((fail + 1)); }
done
echo "demo bit rows: $bits checked"
# The cross rows (Part D3): on test/demo/host-table.txt (the 13 host calls of demo
# at the D1 OFFs) the evaluator and the Wasm module (enginetool) must both give
# W 4709. This pins the W bit order of the guest demo in the two runtimes.
cross=0
offs="0 64 128 192 256 320 368 416 512 608 704 800 896 1320 1368 1416 1448 1544"
status=0
got=$(build/langc --kit eval test/demo/demo.lang demo $offs --host test/demo/host-table.txt 2>&1) || status=$?
cross=$((cross + 1))
[ "$status:$got" = "0:4709" ] || { echo "FAIL demo cross eval: want 4709, got exit $status: $got"; fail=$((fail + 1)); }
status=0
got=$(build/enginetool --host test/demo/host-table.txt "$tmp/demo.wasm" demo $offs 2>&1) || status=$?
cross=$((cross + 1))
[ "$status:$got" = "0:4709" ] || { echo "FAIL demo cross wasm: want 4709, got exit $status: $got"; fail=$((fail + 1)); }
echo "demo cross rows: $cross checked"
# bad-o5.lang is the P' of the D7 amendment (Q-S10-U3-4 (a)). Its amend rule
# fails O5, so guest `langc check` refuses it with REFUSE_FORK. Kit mode does
# not decide O3, O4 or O5 (rule_def runs in guest mode only).
refusals test/demo demo

# The IR (S3): `langc ir examples/functions.lang` must exit 0 and give one
# func line for each line of `langc abi`.
irs=0
status=0
build/langc $(kit examples/kit.txt functions.lang) ir examples/functions.lang >"$tmp/ir.txt" 2>"$tmp/ir.err" || status=$?
irs=$((irs + 1))
funcs=$(rg -c '^func ' "$tmp/ir.txt" || echo 0)
entries=$(build/langc $(kit examples/kit.txt functions.lang) abi examples/functions.lang | rg -c '.' || echo 0)
[ "$status" -eq 0 ] && [ "$funcs" -gt 0 ] && [ "$funcs" -eq "$entries" ] || { echo "FAIL ir functions: exit $status, $funcs funcs for $entries entries: $(head -n 1 "$tmp/ir.err")"; fail=$((fail + 1)); }
echo "ir: $irs lowered"

# The IR refusals (S3): `langc --kit check` must accept each file, and
# `langc --kit ir` must exit 1 and print `langc: ` and the given text first.
# closure.lang gives a function value at run time (REFUSE_RUNTIME_CLOSURE).
# kmap.lang uses a KMap operation at run time (a refuse() arm of lower.c).
# tree.lang folds a tree that exists only at run time (REFUSE_RUNTIME_TREE).
# spine.lang walks lists that exist only at run time, so its heap use has no
# static bound (S3).
irrefusals=0
ir_refused() {
  status=0
  build/langc --kit check "test/ir/$1" > /dev/null 2>&1 || status=$?
  [ "$status" -eq 0 ] || { echo "FAIL $1: check exit $status"; fail=$((fail + 1)); }
  status=0
  first=$(build/langc --kit ir "test/ir/$1" 2>&1 > /dev/null) || status=$?
  case "$status:$first" in
    "1:langc: $2"*) ;;
    *) echo "FAIL $1: want exit 1 and $2, got exit $status: $first"; fail=$((fail + 1)) ;;
  esac
  irrefusals=$((irrefusals + 1))
}
ir_refused closure.lang "REFUSE_RUNTIME_CLOSURE: bad: "
ir_refused kmap.lang "REFUSE_LOWER: got: the Wasm host does not lower an elim, KMap"
ir_refused tree.lang "REFUSE_RUNTIME_TREE: tips: "
ir_refused spine.lang "REFUSE_LOWER: trapHead: heap use has no static bound"
ir_refused hop.lang "REFUSE_LOWER: hopDepth: a recursive fold over a block of families"
echo "ir refusals: $irrefusals checked"

# The IR lowerings (S3): `langc
# --kit check` must accept each file and `langc --kit ir` must exit 0. The
# IR text of natops.lang and of block.lang (S3) must be
# test/ir/natops.ir and test/ir/block.ir (golden files: a change of the
# lowering changes the text).
irlowered=0
ir_lowered() {
  status=0
  build/langc --kit check "test/ir/$1" > /dev/null 2>&1 || status=$?
  [ "$status" -eq 0 ] || { echo "FAIL $1: check exit $status"; fail=$((fail + 1)); }
  status=0
  build/langc --kit ir "test/ir/$1" >"$tmp/$1.ir" 2>"$tmp/ir.err" || status=$?
  [ "$status" -eq 0 ] || { echo "FAIL $1: want ir exit 0, got exit $status: $(head -n 1 "$tmp/ir.err")"; fail=$((fail + 1)); }
  irlowered=$((irlowered + 1))
}
ir_lowered natops.lang
cmp -s "$tmp/natops.lang.ir" test/ir/natops.ir || { echo "FAIL natops.lang: the IR text is not test/ir/natops.ir"; fail=$((fail + 1)); }
ir_lowered block.lang
cmp -s "$tmp/block.lang.ir" test/ir/block.ir || { echo "FAIL block.lang: the IR text is not test/ir/block.ir"; fail=$((fail + 1)); }
echo "ir lowerings: $irlowered lowered"

# The writer (S3): `langc --kit build
# test/ir/natops.lang` must exit 0 and write a module that wasm-validate
# accepts. In `wasm-objdump -x`, memory 0 must have 1 page (minimum =
# maximum = the pages of the largest heap bound, at least 1). The module of
# examples/functions.lang (the builds rows) has 0 heap words: its memory 0
# must also have 1 page. The Wasm runs and the differential lines run node:
# they come back at S8 (Q-S3-2 (b)).

status=0
build/langc --kit build test/ir/natops.lang -o "$tmp/natops.wasm" 2>"$tmp/err" || status=$?
if [ "$status" -ne 0 ] || ! wasm-validate "$tmp/natops.wasm" 2>>"$tmp/err"; then
  echo "FAIL build natops: want exit 0 and a valid module, got exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
# The module of test/ir/block.lang (S3): the same build
# check. The notrap row below uses it.
status=0
build/langc --kit build test/ir/block.lang -o "$tmp/block.wasm" 2>"$tmp/err" || status=$?
if [ "$status" -ne 0 ] || ! wasm-validate "$tmp/block.wasm" 2>>"$tmp/err"; then
  echo "FAIL build block: want exit 0 and a valid module, got exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
# The module of examples/state-no-history.lang (Q-S3-3 (b)): build lowers a
# state family as a plain family. The same build check; the notrap row
# below uses it. Its differential row comes at S8 (Q-S3-2 (b)).
status=0
build/langc $(kit examples/kit.txt state-no-history.lang) build examples/state-no-history.lang -o "$tmp/state-no-history.wasm" 2>"$tmp/err" || status=$?
if [ "$status" -ne 0 ] || ! wasm-validate "$tmp/state-no-history.wasm" 2>>"$tmp/err"; then
  echo "FAIL build state-no-history: want exit 0 and a valid module, got exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
# The module of test/ir/host.lang (S9 step e): its host calls are open, so
# the module imports the six ratify functions. The same build check; the
# notrap row and the host rows below use it.
status=0
build/langc --kit build test/ir/host.lang -o "$tmp/host.wasm" 2>"$tmp/err" || status=$?
if [ "$status" -ne 0 ] || ! wasm-validate "$tmp/host.wasm" 2>>"$tmp/err"; then
  echo "FAIL build host: want exit 0 and a valid module, got exit $status: $(head -n 1 "$tmp/err")"
  fail=$((fail + 1))
fi
for mod in natops functions; do
  wasm-objdump -x "$tmp/$mod.wasm" 2>/dev/null | rg -q -x -F ' - memory[0] pages: initial=1 max=1' ||
    { echo "FAIL $mod.wasm: memory 0 is not initial=1 max=1"; fail=$((fail + 1)); }
done
# build/notraptool (test/notraptool.c, S3) must accept the
# modules of natops.lang, block.lang, functions.lang,
# state-no-history.lang and host.lang: exit 0 and no output. It fails a module with unreachable, memory.grow, an opcode that
# is not in src/wasmop.h, an i64.div_u or i64.rem_u with no divisor guard,
# or a memory with no maximum or a maximum that is not the minimum.
notrapmods=0
for mod in natops block functions state-no-history host; do
  notrapmods=$((notrapmods + 1))
  status=0
  got=$(build/notraptool "$tmp/$mod.wasm" 2>&1) || status=$?
  [ "$status" -eq 0 ] && [ -z "$got" ] ||
    { echo "FAIL notrap $mod.wasm: want exit 0 and no output, got exit $status: $got"; fail=$((fail + 1)); }
done
echo "notrap modules: $notrapmods checked"
# The bad rows (S3): each line of test/notrap/bad/expect.txt is
# `NAME MESSAGE`. wat2wasm makes $tmp/NAME.wasm from test/notrap/bad/NAME.wat.
# build/notraptool must exit 1 and print the one line
# `notraptool: $tmp/NAME.wasm: MESSAGE`.
notrapbad=0
while read -r name msg; do
  notrapbad=$((notrapbad + 1))
  status=0
  got=$(wat2wasm "test/notrap/bad/$name.wat" -o "$tmp/$name.wasm" 2>&1 && build/notraptool "$tmp/$name.wasm" 2>&1) || status=$?
  [ "$status" -eq 1 ] && [ "$got" = "notraptool: $tmp/$name.wasm: $msg" ] ||
    { echo "FAIL notrap bad $name: want exit 1 and $msg, got exit $status: $got"; fail=$((fail + 1)); }
done <test/notrap/bad/expect.txt
echo "notrap bad rows: $notrapbad checked"
# The host rows (S9 step f, Q-S9-4 (a)): `node test/run-wasm.mjs --host
# TABLE` gives each ratify import the RESULT of the row of its name and words,
# as `langc eval --host` does. On the cryptotool table (written above) the
# module of test/ir/host.lang gives the combine counts and the sig_verify
# Decision of the host eval rows. A missing row and a module with imports and
# no table (`-`) exit 1.
hostwasms=0
hostengines=0
ratifyds=0
host_wasm() {
  want_status=$1
  want=$2
  table=$3
  shift 3
  status=0
  case "$table" in
    -) got=$(node test/run-wasm.mjs "$tmp/host.wasm" "$@" 2>&1) || status=$? ;;
    *) got=$(node test/run-wasm.mjs --host "$table" "$tmp/host.wasm" "$@" 2>&1) || status=$? ;;
  esac
  hostwasms=$((hostwasms + 1))
  [ "$status" -eq "$want_status" ] && [ "$got" = "$want" ] ||
    { echo "FAIL host wasm $table $*: want exit $want_status and $want, got exit $status: $(printf '%s\n' "$got" | head -n 1)"; fail=$((fail + 1)); }
  # The engine (S10 U1, Q-S10-3 (a)): build/enginetool on the same table and
  # words gives the same exit status, and for exit 0 the same output.
  status=0
  case "$table" in
    -) got=$(build/enginetool "$tmp/host.wasm" "$@" 2>&1) || status=$? ;;
    *) got=$(build/enginetool --host "$table" "$tmp/host.wasm" "$@" 2>&1) || status=$? ;;
  esac
  hostengines=$((hostengines + 1))
  [ "$status" -eq "$want_status" ] && { [ "$status" -ne 0 ] || [ "$got" = "$want" ]; } ||
    { echo "FAIL host engine $table $*: want exit $want_status and $want, got exit $status: $(printf '%s\n' "$got" | head -n 1)"; fail=$((fail + 1)); }
  # ratifyd (S10 U2, Q-S10-U2-4 (a)): on a row of the cryptotool table,
  # build/ratifyd runs the same entry and words with the six real import
  # bodies over the fixture image. It gives the same exit status and output.
  case "$table" in
    "$tmp/host-crypto.txt")
      status=0
      got=$(build/ratifyd --call "$tmp/host-image.txt" "$tmp/host.wasm" "$@" 2>&1) || status=$?
      ratifyds=$((ratifyds + 1))
      [ "$status" -eq "$want_status" ] && [ "$got" = "$want" ] ||
        { echo "FAIL ratifyd $*: want exit $want_status and $want, got exit $status: $(printf '%s\n' "$got" | head -n 1)"; fail=$((fail + 1)); }
      ;;
    *) ;;
  esac
}
host_wasm 0 2 "$tmp/host-crypto.txt" joinShares 1152 464 1616 192 1024 128 2 1
host_wasm 0 3 "$tmp/host-crypto.txt" joinShares 2544 464 1616 192 3008 128 2 2
host_wasm 0 0 "$tmp/host-crypto.txt" joinShares 1152 464 1616 192 2288 128 2 1
host_wasm 0 1 "$tmp/host-crypto.txt" sigPick 0 64 64 6 70 96
host_wasm 0 0 "$tmp/host-crypto.txt" sigPick 0 64 64 6 166 96
host_wasm 0 0 "$tmp/host-crypto.txt" sigPick 0 64 65536 1 70 96
host_wasm 0 1 "$tmp/host-crypto.txt" sigPick 0 64 65536 0 166 96
host_wasm 0 42 test/eval/host-table.txt joinShares 1 2 3 4 5 6 7 8
host_wasm 1 "run-wasm: no host row for combine 1 2 3 4 5 6 7 9" test/eval/host-table.txt joinShares 1 2 3 4 5 6 7 9
host_wasm 1 "run-wasm: $tmp/host.wasm: the module imports ratify.sig_verify and there is no host table" - joinShares 1 2 3 4 5 6 7 8
# Two rows of one name and words (test/engine/dup-table.txt): the first row
# wins, 42 and not 7, on both sides.
host_wasm 0 42 test/engine/dup-table.txt joinShares 1 2 3 4 5 6 7 8
# The image rows (S10 U2, Q-S10-U2-4 (a)): the fixture image and one more
# line. A line that does not parse (odd hex, a bad hex digit, bytes past
# HOST_MEM, a bad offset) gives `invalid malformed`, exit 0, and the module
# does not run. A last byte at HOST_MEM - 1 fits, and the entry runs.
ratifydimages=0
ratifyd_image() {
  want=$1
  extra=$2
  { cat "$tmp/host-image.txt"; printf '%s\n' "$extra"; } >"$tmp/image-row.txt"
  status=0
  got=$(build/ratifyd --call "$tmp/image-row.txt" "$tmp/host.wasm" sigPick 0 64 64 6 70 96 2>&1) || status=$?
  ratifydimages=$((ratifydimages + 1))
  [ "$status" -eq 0 ] && [ "$got" = "$want" ] ||
    { echo "FAIL ratifyd image $extra: want exit 0 and $want, got exit $status: $(printf '%s\n' "$got" | head -n 1)"; fail=$((fail + 1)); }
}
ratifyd_image "invalid malformed" "0 abc"
ratifyd_image "invalid malformed" "0 0g"
ratifyd_image "invalid malformed" "65535 0000"
ratifyd_image "invalid malformed" "1x 00"
ratifyd_image 1 "65535 00"
echo "host wasm rows: $hostwasms checked"
echo "host engine rows: $hostengines checked"
echo "ratifyd rows: $ratifyds checked"
echo "ratifyd image rows: $ratifydimages checked"
# The engine load refusals (S10 U1, the form of the permit-lang S9 U1 rows):
# each line below is `NAME MESSAGE`. wat2wasm makes $tmp/engine-NAME.wasm
# from test/engine/NAME.wat. build/enginetool must exit 1 and print the one
# line `enginetool: load: MESSAGE` (an import that the engine does not bind,
# an import with the wrong type).
enginerefusals=0
while read -r name msg; do
  enginerefusals=$((enginerefusals + 1))
  status=0
  got=$(wat2wasm "test/engine/$name.wat" -o "$tmp/engine-$name.wasm" 2>&1 && build/enginetool --host test/eval/host-table.txt "$tmp/engine-$name.wasm" x 2>&1) || status=$?
  [ "$status" -eq 1 ] && [ "$got" = "enginetool: load: $msg" ] ||
    { echo "FAIL engine refusal $name: want exit 1 and $msg, got exit $status: $got"; fail=$((fail + 1)); }
done <<'EOF'
unbound unbound import
badtype import type
EOF
echo "engine refusals: $enginerefusals checked"
# The guest memory bytes (S10 U1, engine_bytes): each line below is
# `OFF LEN STATUS WANT`. test/engine/bytes.wat has 1 page (65536 bytes) and
# the bytes 01 to 08 at 65528. A range past the end of the memory exits 1.
# A word past 2^64 - 1 is a usage error, exit 2.
wat2wasm test/engine/bytes.wat -o "$tmp/engine-bytes.wasm" 2>/dev/null ||
  { echo "FAIL wat2wasm test/engine/bytes.wat"; fail=$((fail + 1)); }
enginebytes=0
while read -r off len want_status want; do
  enginebytes=$((enginebytes + 1))
  status=0
  got=$(build/enginetool --bytes "$off" "$len" "$tmp/engine-bytes.wasm" 2>&1) || status=$?
  [ "$status" -eq "$want_status" ] && [ "$got" = "$want" ] ||
    { echo "FAIL engine bytes $off $len: want exit $want_status and $want, got exit $status: $got"; fail=$((fail + 1)); }
done <<'EOF'
65528 8 0 0102030405060708
65536 0 0
0 8 0 0000000000000000
65529 8 1 enginetool: bytes 65529 8: not in the memory
65537 0 1 enginetool: bytes 65537 0: not in the memory
8 18446744073709551615 1 enginetool: bytes 8 18446744073709551615: not in the memory
0 18446744073709551616 2 usage: enginetool --bytes OFF LEN FILE
EOF
echo "engine bytes rows: $enginebytes checked"
# The kit evals of test/ir/spine.lang at 4. Each line is `NAME VALUE`.
spineevals=0
while read -r name want; do
  got=$(build/langc --kit eval test/ir/spine.lang "$name" 4 2>&1) || got="exit $?: $got"
  spineevals=$((spineevals + 1))
  [ "$got" = "$want" ] || { echo "FAIL spine $name 4: want $want, got $got"; fail=$((fail + 1)); }
done <<'EOF'
foldOnes 8
mapUp 20
bindUp 50
filterUp 3
trapHead 1
trapSum 3
filterTrap 7
bindTrap 1
sizeSum 20
sizeTrap 4
EOF
echo "spine evals: $spineevals checked"

# The Wasm target (S3, OI-12): each module builds and is valid. Then the differential: on
# the grid of test/grid.awk, node and `langc eval` give the same result for
# each entry. R0 (docs/PLAN.md section 2, OI-6): no target traps, so a
# `trap` line on either side is a FAIL. The stderr of eval is not merged
# into stdout. A file in test/ir/ takes --kit.
wasms=0
cases=0
mismatches=0
enginemods=0
enginemismatches=0
wasmtimemods=0
wasmtimemismatches=0
# i64 words (S10 U2 Part D): wasmtime takes and prints an i64 signed; node
# and eval use a u64. sub64(x) is 2^64 - x on decimal strings, because awk
# numbers are doubles and lose the low digits past 2^53. big(x) is 1 when x
# is 2^63 or more.
i64awk='function sub64(x,  m, r, i, d, b) {
  m = "18446744073709551616"
  while (length(x) < length(m)) x = "0" x
  r = ""
  b = 0
  for (i = length(m); i > 0; i--) {
    d = substr(m, i, 1) - substr(x, i, 1) - b
    b = (d < 0)
    r = (d + 10 * b) r
  }
  sub(/^0+/, "", r)
  return r == "" ? "0" : r
}
function big(x) { return length(x) > 19 || (length(x) == 19 && x >= "9223372036854775808") }'
difkit() {
  case "$1" in
    test/ir/*) echo --kit ;;
    *) kit examples/kit.txt "$(basename "$1")" ;;
  esac
}
for f in examples/*.lang test/ir/natops.lang test/ir/block.lang test/ir/proof-trap.lang; do
  w="$tmp/$(basename "$f" .lang).wasm"
  wasms=$((wasms + 1))
  if ! build/langc $(difkit "$f") build "$f" -o "$w" 2>"$tmp/w.err" || ! wasm-validate "$w" 2>>"$tmp/w.err" ||
    ! build/notraptool "$w" 2>>"$tmp/w.err" ||
    ! wasm-objdump -x "$w" >/dev/null 2>>"$tmp/w.err"; then
    echo "FAIL wasm $f: $(head -n 1 "$tmp/w.err")"
    fail=$((fail + 1))
    continue
  fi
  build/langc $(difkit "$f") ir "$f" | awk -f test/grid.awk >"$tmp/cases.txt"
  : >"$tmp/want.txt"
  while read -r call; do
    status=0
    want=$(build/langc $(difkit "$f") eval "$f" $call 2>/dev/null) || status=$?
    [ "$status" -eq 0 ] || want="eval-exit-$status"
    echo "$want" >>"$tmp/want.txt"
  done <"$tmp/cases.txt"
  if ! node test/run-wasm.mjs "$w" - <"$tmp/cases.txt" >"$tmp/got.txt" 2>"$tmp/w.err"; then
    echo "FAIL node $f: $(head -n 1 "$tmp/w.err")"
    fail=$((fail + 1))
    continue
  fi
  cases=$((cases + $(rg -c '.' "$tmp/cases.txt" || echo 0)))
  paste -d '|' "$tmp/cases.txt" "$tmp/want.txt" "$tmp/got.txt" | awk -F '|' '$2 != $3' >"$tmp/bad.txt"
  bad=$(rg -c '.' "$tmp/bad.txt" || echo 0)
  mismatches=$((mismatches + bad))
  [ "$bad" -eq 0 ] || { echo "FAIL differential $f: $bad mismatches, first $(head -n 1 "$tmp/bad.txt")"; fail=$((fail + 1)); }
  traps=$(cat "$tmp/want.txt" "$tmp/got.txt" | rg -c -x -F trap || echo 0)
  [ "$traps" -eq 0 ] || { echo "FAIL trap $f: $traps trap lines (R0)"; fail=$((fail + 1)); }
  # The engine (S10 U1): build/enginetool on the same calls gives the lines
  # of node.
  enginemods=$((enginemods + 1))
  if ! build/enginetool "$w" - <"$tmp/cases.txt" >"$tmp/engine.txt" 2>"$tmp/w.err"; then
    echo "FAIL engine $f: $(head -n 1 "$tmp/w.err")"
    fail=$((fail + 1))
    continue
  fi
  bad=$(paste -d '|' "$tmp/cases.txt" "$tmp/got.txt" "$tmp/engine.txt" | awk -F '|' '($2 "") != ($3 "")' | rg -c '.' || echo 0)
  enginemismatches=$((enginemismatches + bad))
  [ "$bad" -eq 0 ] || { echo "FAIL engine differential $f: $bad mismatches"; fail=$((fail + 1)); }
  # wasmtime (S10 U2 Part D, Q-S10-6 (a)): `wasmtime run --invoke` on the
  # same calls gives the lines of node, and each call exits 0. i64awk maps
  # the words before the call and the results after it.
  wasmtimemods=$((wasmtimemods + 1))
  awk "$i64awk"' { for (i = 2; i <= NF; i++) if (big($i)) $i = "-" sub64($i); print }' "$tmp/cases.txt" >"$tmp/i64.txt"
  : >"$tmp/wasmtime.txt"
  while read -r entry words; do
    status=0
    out=$(wasmtime run --invoke "$entry" "$w" $words 2>"$tmp/w.err" </dev/null) || status=$?
    [ "$status" -eq 0 ] || out="wasmtime-exit-$status"
    echo "$out" >>"$tmp/wasmtime.txt"
  done <"$tmp/i64.txt"
  awk "$i64awk"' /^-/ { print sub64(substr($0, 2)); next } { print }' "$tmp/wasmtime.txt" >"$tmp/wasmtime-u.txt"
  paste -d '|' "$tmp/cases.txt" "$tmp/got.txt" "$tmp/wasmtime-u.txt" | awk -F '|' '($2 "") != ($3 "")' >"$tmp/bad.txt"
  bad=$(rg -c '.' "$tmp/bad.txt" || echo 0)
  wasmtimemismatches=$((wasmtimemismatches + bad))
  [ "$bad" -eq 0 ] || { echo "FAIL wasmtime differential $f: $bad mismatches, first $(head -n 1 "$tmp/bad.txt")"; fail=$((fail + 1)); }
done
echo "wasm: $wasms modules valid"
echo "differential: $cases cases, $mismatches mismatches"
echo "engine modules: $enginemods checked, $enginemismatches mismatches"
echo "wasmtime modules: $wasmtimemods checked, $wasmtimemismatches mismatches"
# A Flag argument larger than 1 (langc eval refuses it with EVAL_ARGS). R0
# (docs/PLAN.md section 2, OI-6): the entry sets a Flag argument of 2 or
# more to 1 (flag_norm in src/wasm.c), so the module gives 4 and does not
# trap.
got=$(node test/run-wasm.mjs "$tmp/entries.wasm" pick 2 4 9 2>&1) || true
[ "$got" = "4" ] || { echo "FAIL wasm pick 2 4 9: want 4, got $got"; fail=$((fail + 1)); }

status=0
build/langc 2>/dev/null || status=$?
[ "$status" -eq 2 ] || { echo "FAIL usage: want exit 2, got $status"; fail=$((fail + 1)); }
status=0
build/langc --kit 2>/dev/null || status=$?
[ "$status" -eq 2 ] || { echo "FAIL usage --kit: want exit 2, got $status"; fail=$((fail + 1)); }
status=0
build/langc check "$tmp/missing.lang" 2>/dev/null || status=$?
[ "$status" -eq 2 ] || { echo "FAIL missing file: want exit 2, got $status"; fail=$((fail + 1)); }
rm -rf "$tmp"

if rg -n '\x{2013}|\x{2014}' . >/dev/null; then
  echo "FAIL: an em-dash or en-dash is in the kit"
  fail=$((fail + 1))
fi
echo "gate: $fail failures"
[ "$fail" -eq 0 ]
