# `langc mcp`

`langc mcp` (S16, PLAN section 9 row A4) is a Model Context Protocol
(MCP) server on stdin and stdout. It gives the verbs `check --json`,
`fmt`, `explain`, `list-base`, `show-law` and `build` of `langc` to an
agent as 6 tools. The server is in `src/mcp.c`. It calls no function of
`src/main.c`: `src/main.c` gives it the verbs as a callback (Q-S16-5
(a)).
The server is a copy of the accrue-lang server (a copy of the cairn-lang
60768e8 server), with the ratify-lang tools. The rulings of S16 are
Q-S16-1 to Q-S16-10, Q-U2-1 and Q-U3-1 (`docs/PLAN.md` section 1).

## Usage

```sh
build/langc mcp
build/langc --kit mcp
```

With `--kit`, each tool runs in the kit mode, as `langc --kit VERB`.
`mcp` takes no argument: `langc mcp X` gives the diagnostic `USAGE`
(`mcp takes no argument`), the usage text and exit 2. At EOF the server
exits 0, also after error replies. It exits 2 when stdin cannot be read
or stdout cannot be written.

The server has the rights of the user who starts it, the same as `langc`
on the command line (Q-S16-6 (a)). The tools `check`, `fmt` and `build`
read the file `path`, and the tool `build` writes the file `out`. Thus
an agent that uses the server can read each file that the user can read
as a program, and can write a module to each path that the user can
write.

## Framing

- The server reads one JSON-RPC 2.0 message on each line of stdin until
  EOF, and writes each reply on one line of stdout.
- The longest line is 1048576 bytes (1 MiB, `MCP_LINE_MAX_BYTES` in
  `src/mcp.h`), without the newline. A longer line gives -32700 with id
  null. The server drops the rest of that line and reads on (Q-S16-7
  (a)).
- A line that is not JSON gives -32700 with id null.
- An id is absent, a string or an integer. A message that is not a valid
  request gives -32600, with its id when the id is valid, else null.
- A notification (a valid request without an id) gets no reply, also
  when its method is not known.

## Version

`initialize` always gives the protocol revision `2025-11-25`, only the
capability `tools`, and the server info
`{"name":"ratify-lang","version":"S16"}` (Q-S16-1 (a)). The request must
have a `protocolVersion` string in its params, else it gives -32602. The
server does not compare that string with its own revision. The server
does not support the revision `2026-07-28`.

## Methods

| Method | Result |
|---|---|
| `initialize` | the revision, the capabilities and the server info (Version) |
| `ping` | `{}` |
| `tools/list` | the 6 tools, each with its `name`, `description` and `inputSchema` |
| `tools/call` | the tool result (Tools) |

Each other method gives -32601. The server does not need `initialize`
before another method.

## Tools

A tool that reads a program takes its path in `path`, not the source
text (Q-S16-6 (a)).

| Tool | Arguments (`inputSchema`) | The CLI verb |
|---|---|---|
| `check` | `path` | `langc check PATH --json` (`docs/CHECK-JSON.md`) |
| `fmt` | `path` | `langc fmt PATH` (`docs/FMT.md`) |
| `explain` | `code` | `langc explain CODE` |
| `list-base` | none | `langc list-base` |
| `show-law` | `row` (optional) | `langc show-law [ROW]` |
| `build` | `path`, `target`, `out` | `langc build PATH -o OUT`, with `--runtime` for the target `runtime` |

Each argument is a string. Each argument is required, except `row`. The
`target` of `build` is one of 2 strings (Q-S16-3 (a)): `main` gives
`-o OUT` and `runtime` gives `--runtime -o OUT`. The `build` tool writes
the file `out`. The server ignores a member of `arguments` that is not
in the table. A `tools/call` with no `arguments` member is a call with
no arguments.

The other verbs of `langc` are not tools. There is no `eval` tool
(Q-S16-2 (a)). T2 to T4 (`tfhe`, `noir`, `report`) stay CLI verbs
(Q-S16-3 (a)). `ir`, `abi` and `fmt --check` are CLI only also.

A tool result is

```json
{"content":[{"type":"text","text":"STDOUT"},{"type":"text","text":"STDERR"}],"isError":false}
```

The first item is the stdout of the CLI verb. The second item is its
stderr, and it is there only when the stderr is not empty. Each byte of
either stream that does not start a UTF-8 scalar becomes the JSON
escape of U+FFFD (a backslash, then `ufffd`).
`isError` is `true` when the exit code of the verb is not 0 (Q-S16-4
(a)). Thus a refused program, an unknown code of `explain` or an unknown
row of `show-law` is a tool result with `isError` true, not a JSON-RPC
error. The result has no `structuredContent`, and no tool has an
`outputSchema`.

## The text of each tool

A text diagnostic of `langc` on stderr has the form
`langc: CODE: NAME: MESSAGE`, where NAME is `-` when the diagnostic has
no name.

### `check`

Item 1 is the one JSON line of `docs/CHECK-JSON.md`. An accepted
program gives exit 0. A refused program gives exit 1, and a file that
`langc` cannot read gives exit 2. For these two, `isError` is true, the
JSON line has the diagnostic and there is no second item.

### `fmt`

Item 1 is the canonical text of the program (`docs/FMT.md`). A file that
`langc` cannot read gives an empty item 1, the `IO` diagnostic in item 2
and `isError` true.

### `explain`

Item 1 is one line, `CODE: FIX`, where FIX is the fix text of CODE in
the fix table (`src/front/fix.c`). Example:

```
TYPE_MISMATCH: Change the term until its type is the expected type, or change the declared type.
```

A CODE that is not in the fix table gives exit 1: item 1 is empty and
item 2 is `langc: EXPLAIN: -: 'CODE' is not a code of the fix table`.
On the command line, `langc explain` with no code (`explain needs a
code`) or with a second argument (`explain takes only a code`) gives the
diagnostic `USAGE`, the usage text and exit 2.

### `list-base`

Without `--kit`, item 1 has one line for each guest builtin, in the
order of the table `BUILTINS`, then one line for each definition of the
embedded domain file, in the order of that file:

```
NAME builtin -
NAME KIND SOURCE:LINE
```

KIND is `def`, `family` or `cofamily`. Examples: `Unit builtin -` and
`authorize def domain/domain.lang:395`. At S16 there are 38 builtin
lines and 120 definition lines.

With `--kit` (`langc --kit list-base`, or this tool under
`langc --kit mcp`), item 1 has only the kit signature, one line for each
row of the table `SIGNATURE`:

```
NAME SORT KIND
```

SORT is `type` or `op`. KIND is `slot`, `defined` or `primitive`.
Example: `Member type slot`. At S16 there are 16 lines.

On the command line, `langc list-base` with an argument gives the
diagnostic `USAGE` (`list-base takes no argument`), the usage text and
exit 2.

### `show-law`

Item 1 has one line for each law row of `docs/PLAN.md` section 6, in the
order of the table `LAWS` (19 lines at S16). With `row`, it has only the
line of that row. The line is

```
NAME: STATEMENT (KIND, trigger T)
```

KIND is `DISCHARGED`, `ASSUMED`, `AT TARGET` or `EXTERNAL`. T is the
trigger letter of the row: `R`, `H`, `F` or `L`. Example:
`O4: each accepting set holds an honest party (DISCHARGED, trigger H)`.

A `row` that is not a law row gives exit 1: item 1 is empty and item 2
is `langc: LAW: -: 'ROW' is not a law row of docs/PLAN.md section 6`.
On the command line, `langc show-law` with a second argument gives the
diagnostic `USAGE` (`show-law takes only a row name`), the usage text
and exit 2.

### `build`

Item 1 is empty. An accepted program gives exit 0, and the file `out`
has the module, the same bytes as the file of the CLI verb. A refused
program gives exit 1 and its diagnostic in item 2, and the server does
not write `out`. A file `out` that `langc` cannot write gives the `IO`
diagnostic in item 2 and exit 1.

### An argument failure

A call of a known tool with an argument failure is a tool result also
(Q-S16-7 (a), Q-U2-1 (a)). The server does not run the verb. Item 1 is
empty, item 2 is one line and `isError` is true. T is the tool and A is
the argument:

| Cause | Item 2 |
|---|---|
| A is required and absent | `langc mcp: the tool 'T' needs the argument 'A'` |
| A is not a string, or the string has a NUL byte | `langc mcp: the argument 'A' of the tool 'T' must be a string with no NUL byte` |
| `target` of `build` is a string that is not a target | `langc mcp: the argument 'target' of the tool 'build' must be "main" or "runtime"` |

The server examines the arguments in the order of the Tools table and
gives the first failure only. The six sibling servers (accrue, cairn,
lien, permit, relay, tranche) give -32602 in these cases.

## Errors

| Code | Cause |
|---|---|
| -32700 | The line is not JSON, or it is longer than 1 MiB. The id is null. |
| -32600 | The message is not a valid request (`Invalid Request`). |
| -32601 | The method is not one of the 4 methods. |
| -32602 | `initialize` has no `protocolVersion` string, or a `tools/call` has no `name` string, an `arguments` member that is not an object, or an unknown tool. |
| -32603 | The server cannot run the tool (`Internal error`). |

Only a failure of `open_memstream` or `fclose` gives -32603.

## Limits and state

- The server keeps no state between messages. Each message gets a fresh
  arena (1 GiB limit, `MCP_ARENA_LIMIT_BYTES` in `src/mcp.c`), and each
  `tools/call` runs its verb with fresh options, a fresh diagnostic and
  a second fresh arena for the verb (Q-S16-5 (a)).
- The server gives tools only: no `resources`, no `prompts`, no
  cancellation and no progress notices.
- Each tool reads its program from the file `path`. No tool takes the
  source text. A relative `path` or `out` starts at the working
  directory of the server (Q-S16-6 (a)).
- `src/mcp.c` gets the stdout and the stderr of a verb through
  `open_memstream`. This function is POSIX.1-2008, not C99.
  `docs/PLAN.md` (the host row of 2026-11-03) says that `langc mcp` is
  C99 that TinyCC builds: `open_memstream` is the one exception.
  `src/mcp.c` sets `_POSIX_C_SOURCE` to `200809L` for it, and TinyCC and
  clang build the file with `-std=c99`. A host with no `open_memstream`
  cannot build `langc`.

## The test rows

`test/mcp/` has 10 rows: `lifecycle`, `check`, `fmt`, `explain`,
`list-base`, `show-law`, `build`, `errors`, `kit-check` and
`kit-list-base`. Each `X.in` is a list of messages that starts with
`initialize` and `notifications/initialized`, and `X.out` has the exact
bytes of the replies. `build/mcptool` (`test/mcptool.c`) runs
`langc mcp` on each row (`langc --kit mcp` when `X` starts with `kit-`).
The stdout must give the bytes of `X.out`, the stderr must be empty and
the exit status must be 0. The gate also runs `build/mcptool` on 3 bad
inputs (a changed `.out`, a missing `.out` and no rows), and each must
give exit 1. The gate rows are `mcp rows: 10 passed`,
`mcp negatives: 3 of 3`, `mcp tools: 6 of 6`, `mcp builds: 2 of 2`,
`mcp long line: 1 of 1`, `explain codes: 71 of 71` and
`law rows: 19 of 19` (the mcp block of `test/gate.sh`). No row gets
-32603.

The row `build` writes one file for the target `main` and one file for
the target `runtime`. For `mcp builds: 2 of 2`, the gate makes the 2
files with the CLI (`langc build`, with `--runtime` for `runtime`) and
compares each file with the file of the row. On T1 the 2 targets give
the same bytes: `src/wasm.c` does not use the part (`(void)part;`), and
`src/target.h` says that the 2 parts are the same module bytes on Wasm.
Thus a swap of the 2 targets in the server has no effect today, and
this gate row sees a swap as soon as the 2 parts differ (Q-U3-1 (a)).
