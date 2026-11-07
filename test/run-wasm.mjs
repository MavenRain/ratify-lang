// Runs entries of a langc Wasm module and prints each result as `langc eval`
// does: a number, or `trap` when the module traps.
// Usage: node test/run-wasm.mjs [--host TABLE] FILE NAME [ARGS...]
//        node test/run-wasm.mjs [--host TABLE] FILE -   (one `NAME [ARGS...]` line on stdin per call)
// TABLE is the host table of `langc eval --host` (S9, Q-S9-4 (a)): `NAME W1 ..
// Wp RESULT` rows and `#` lines. Each of the six imports of module "ratify"
// gives the RESULT of the row of its name and words. As in eval, the key is
// the words (OFF LEN pairs and Nats), not the bytes at them. A call with no
// row, or a module with imports and no TABLE, gives exit 1; a bad row gives
// exit 2.
import { readFileSync, writeSync } from "node:fs";

// The import names and word counts, in the order of src/front/eval.c.
const IMPORTS = new Map([["sig_verify", 6], ["hash_eq", 4], ["share_verify", 6], ["ballot_verify", 9], ["tally_add", 6], ["combine", 8]]);

const fail = (status, text) => {
  writeSync(2, `run-wasm: ${text}\n`);
  process.exit(status);
};

const argv = process.argv.slice(2);
const [table, [file, name, ...args]] = argv[0] === "--host" ? [argv[1], argv.slice(2)] : [undefined, argv];
if (file === undefined || name === undefined) {
  process.stderr.write("usage: node test/run-wasm.mjs [--host TABLE] FILE NAME [ARGS...] | [--host TABLE] FILE -\n");
  process.exit(2);
}

// One row: the key `NAME W1 .. Wp` (decimal words) and the result.
const row = (line, index) => {
  const [key, ...words] = line.split(" ");
  const count = IMPORTS.get(key);
  if (count === undefined) fail(2, `${table}: line ${index + 1}: no host import ${key}`);
  if (words.length !== count + 1 || !words.every((w) => /^[0-9]+$/.test(w)))
    fail(2, `${table}: line ${index + 1}: ${key} takes ${count} words and one result`);
  const values = words.map(BigInt);
  return [[key, ...values.slice(0, count)].join(" "), values[count]];
};
const rows = new Map(
  table === undefined
    ? []
    : readFileSync(table, "utf8")
        .split("\n")
        .map((line, index) => [line, index])
        .filter(([line]) => line !== "" && !line.startsWith("#"))
        .map(([line, index]) => row(line, index))
        // A Map keeps the last row of a key; eval takes the first.
        .reverse(),
);

class HostMiss extends Error {}

const host = (key) => (...words) => {
  const call = [key, ...words.map((w) => BigInt.asUintN(64, w))].join(" ");
  const got = rows.get(call);
  if (got === undefined) throw new HostMiss(call);
  return got;
};

const wasm = new WebAssembly.Module(readFileSync(file));
const needs = WebAssembly.Module.imports(wasm);
if (table === undefined && needs.length > 0)
  fail(1, `${file}: the module imports ${needs[0].module}.${needs[0].name} and there is no host table`);
const imports = table === undefined ? {} : { ratify: Object.fromEntries([...IMPORTS.keys()].map((key) => [key, host(key)])) };
const instance = new WebAssembly.Instance(wasm, imports);

const call = ([entry, ...words]) => {
  const fn = instance.exports[entry];
  if (typeof fn !== "function") throw new Error(`no exported entry '${entry}'`);
  try {
    return BigInt.asUintN(64, fn(...words.map(BigInt))).toString();
  } catch (e) {
    if (e instanceof WebAssembly.RuntimeError) return "trap";
    if (e instanceof HostMiss) fail(1, `no host row for ${e.message}`);
    throw e;
  }
};

const calls = name === "-"
  ? readFileSync(0, "utf8").split("\n").filter((line) => line !== "").map((line) => line.split(" "))
  : [[name, ...args]];
process.stdout.write(calls.map((c) => call(c) + "\n").join(""));
