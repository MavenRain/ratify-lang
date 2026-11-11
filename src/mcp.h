/* The MCP stdio server of `langc mcp` (S16 A4, Q-S16-1 to Q-S16-7). A copy
   of accrue-lang src/mcp.h (the copy of the cairn-lang 60768e8 file, the
   port of evolve-lang src/mcp.h, 8c5d8e1) with the ratify-lang tools. This
   file calls no function of main.c: main.c gives the verbs as a callback
   (Q-S16-5 (a)). */
#ifndef LANG_MCP_H
#define LANG_MCP_H

#include <stdio.h>

/* The longest request line that the server reads, without the newline. A
   longer line gives -32700 with id null; the server drops the rest of the
   line and reads on (Q-S16-7 (a)). */
#define MCP_LINE_MAX_BYTES ((size_t)1 << 20)

/* The protocol revision that `initialize` always gives (Q-S16-1 (a)). */
#define MCP_PROTOCOL_VERSION "2025-11-25"

typedef enum {
  MCP_TOOL_CHECK,
  MCP_TOOL_FMT,
  MCP_TOOL_EXPLAIN,
  MCP_TOOL_LIST_BASE,
  MCP_TOOL_SHOW_LAW,
  MCP_TOOL_BUILD
} McpTool;

/* The `target` of the build tool (Q-S16-3 (a)): main is `-o OUT` and
   runtime is `--runtime -o OUT`. */
typedef enum {
  MCP_TARGET_MAIN,
  MCP_TARGET_RUNTIME
} McpTarget;

/* The arguments of one tools/call (Q-S16-2 (a), Q-S16-6 (a)). The callback
   reads only the fields of its tool. */
typedef struct {
  const char *path;     /* check, fmt, build: the program file */
  McpTarget target;     /* build */
  const char *out_path; /* build: the file that the verb writes */
  const char *code;     /* explain: the code of the fix table */
  const char *row;      /* show-law: the row name, or NULL for each row */
} McpArgs;

/* Runs the verb of TOOL as the CLI does (`check PATH --json`, `fmt PATH`,
   `explain CODE`, `list-base`, `show-law [ROW]`, `build` with TARGET and
   OUT_PATH) with fresh Options, a fresh Diag and a fresh Arena (Q-S16-5
   (a)). Writes the CLI stdout to OUT and the CLI stderr to ERR. Returns
   the CLI exit code. */
typedef int (*McpRunTool)(McpTool tool, const McpArgs *args, FILE *out, FILE *err);

/* Reads one JSON-RPC 2.0 message on each line of IN until EOF and writes
   each reply on one line of OUT. Returns 0, or 2 when IN cannot be read or
   OUT cannot be written. */
int mcp_serve(FILE *in, FILE *out, McpRunTool run_tool);

#endif
