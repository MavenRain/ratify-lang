#define _POSIX_C_SOURCE 200809L
/* The MCP stdio server of `langc mcp` (mcp.h, S16 A4): the framing and the
   error codes (Q-S16-7 (a)), the lifecycle (Q-S16-1 (a)) and the tools
   check, fmt, explain, list-base, show-law and build (Q-S16-2 to Q-S16-4
   (a)). A copy of accrue-lang src/mcp.c, the copy of the cairn-lang 60768e8
   server (the cairn-lang S13 port of evolve-lang src/mcp.c, 8c5d8e1). The
   ratify-lang changes: the 6 tools and McpArgs, and an argument failure of
   a known tool is a tool result with isError true, not -32602 (Q-S16-7 (a),
   Q-U2-1 (a)). open_memstream is POSIX.1-2008, not C99 (docs/MCP.md). */
#include "mcp.h"

#include <stdlib.h>
#include <string.h>

#include "front/base.h"
#include "jsonin.h"
#include "jsonout.h"
/* The arena of one request line (the limit of main.c). */
#define MCP_ARENA_LIMIT_BYTES ((size_t)1 << 30)

/* JSON-RPC 2.0 error codes. */
enum {
  RPC_PARSE = -32700,
  RPC_BAD_REQUEST = -32600,
  RPC_UNKNOWN_METHOD = -32601,
  RPC_BAD_PARAMS = -32602,
  RPC_INTERNAL = -32603
};

/* The tools/list result (Q-S16-2 (a), Q-S16-3 (a)). */
static const char MCP_TOOLS[] =
  "{\"tools\":["
  "{\"name\":\"check\",\"description\":\"Checks the ratify-lang program at path. The text is the langc check --json line.\","
  "\"inputSchema\":{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\"}},\"required\":[\"path\"]}},"
  "{\"name\":\"fmt\",\"description\":\"Gives the canonical text of the ratify-lang program at path, as langc fmt.\","
  "\"inputSchema\":{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\"}},\"required\":[\"path\"]}},"
  "{\"name\":\"explain\",\"description\":\"Gives the fix text of a diagnostic code, as langc explain.\","
  "\"inputSchema\":{\"type\":\"object\",\"properties\":{\"code\":{\"type\":\"string\"}},\"required\":[\"code\"]}},"
  "{\"name\":\"list-base\",\"description\":\"Lists the guest builtins and the domain definitions, as langc list-base. Under langc --kit mcp it lists the kit signature.\","
  "\"inputSchema\":{\"type\":\"object\",\"additionalProperties\":false}},"
  "{\"name\":\"show-law\",\"description\":\"Shows each law row of docs/PLAN.md section 6, or only the row with the name row, as langc show-law.\","
  "\"inputSchema\":{\"type\":\"object\",\"properties\":{\"row\":{\"type\":\"string\"}}}},"
  "{\"name\":\"build\",\"description\":\"Builds the ratify-lang program at path and writes the file out, as langc build PATH -o OUT. The target runtime adds --runtime.\","
  "\"inputSchema\":{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\"},"
  "\"target\":{\"type\":\"string\",\"enum\":[\"main\",\"runtime\"]},\"out\":{\"type\":\"string\"}},"
  "\"required\":[\"path\",\"target\",\"out\"]}}"
  "]}";

/* The build targets by their `target` word (Q-S16-3 (a)). */
static const struct {
  const char *word;
  McpTarget target;
} MCP_TARGETS[] = {
  {"main", MCP_TARGET_MAIN},
  {"runtime", MCP_TARGET_RUNTIME},
};

/* The shared writer replaces invalid UTF-8 bytes in captured CLI output
   (including truncated diagnostics) so every reply remains valid JSON. */
static void put_string(const char *text, size_t len, FILE *out) {
  json_string_n(text, len, out);
}

/* Writes the id of a request: a string, an integer (its source text) or
   null. */
static void put_id(const JsonValue *id, FILE *out) {
  if (id != NULL && id->kind == JSON_STRING) {
    put_string(id->text, id->len, out);
    return;
  }
  fputs(id != NULL && id->kind == JSON_NUMBER ? id->text : "null", out);
}

static void rpc_head(const JsonValue *id, FILE *out) {
  fputs("{\"jsonrpc\":\"2.0\",\"id\":", out);
  put_id(id, out);
}

static void rpc_error(const JsonValue *id, int code, const char *message, FILE *out) {
  rpc_head(id, out);
  fprintf(out, ",\"error\":{\"code\":%d,\"message\":", code);
  put_string(message, strlen(message), out);
  fputs("}}\n", out);
}

/* `initialize` always gives MCP_PROTOCOL_VERSION, whatever revision the
   client asks for (Q-S16-1 (a)). */
static void mcp_initialize(Arena *arena, const JsonValue *id, const JsonValue *params, McpRunTool run_tool, FILE *out) {
  (void)arena;
  (void)run_tool;
  if (json_get_string(json_member(params, "protocolVersion")) == NULL) {
    rpc_error(id, RPC_BAD_PARAMS, "Invalid params", out);
    return;
  }
  rpc_head(id, out);
  fputs(",\"result\":{\"protocolVersion\":\"" MCP_PROTOCOL_VERSION "\",\"capabilities\":{\"tools\":{}},"
        "\"serverInfo\":{\"name\":\"ratify-lang\",\"version\":\"S16\"}}}\n",
        out);
}

static void mcp_ping(Arena *arena, const JsonValue *id, const JsonValue *params, McpRunTool run_tool, FILE *out) {
  (void)arena;
  (void)params;
  (void)run_tool;
  rpc_head(id, out);
  fputs(",\"result\":{}}\n", out);
}

static void mcp_tools_list(Arena *arena, const JsonValue *id, const JsonValue *params, McpRunTool run_tool, FILE *out) {
  (void)arena;
  (void)params;
  (void)run_tool;
  rpc_head(id, out);
  fputs(",\"result\":", out);
  fputs(MCP_TOOLS, out);
  fputs("}\n", out);
}

/* Sets *TARGET from WORD. Returns 0 when WORD is NULL or not a target. */
static int find_target(const char *word, McpTarget *target) {
  for (size_t i = 0; word != NULL && i < sizeof MCP_TARGETS / sizeof MCP_TARGETS[0]; i++) {
    if (strcmp(MCP_TARGETS[i].word, word) == 0) {
      *target = MCP_TARGETS[i].target;
      return 1;
    }
  }
  return 0;
}

/* The exit code of a tools/call with an argument failure: the usage exit
   code of the CLI (Q-U2-1 (a)). */
#define MCP_EXIT_USAGE 2

/* Sets *VALUE to the string argument KEY of the tool TOOL. An absent KEY
   that is not REQUIRED gives NULL. Returns 1, or 0 after one line on ERR
   that names the argument (Q-S16-7 (a)): KEY is absent and REQUIRED, or KEY
   is not a string, or the string has a NUL byte. */
static int string_arg(const JsonValue *args, const char *tool, const char *key, int required, const char **value,
                      FILE *err) {
  const JsonValue *member = json_member(args, key);
  *value = json_get_string(member);
  if (*value != NULL || (member == NULL && !required)) return 1;
  if (member == NULL) {
    fprintf(err, "langc mcp: the tool '%s' needs the argument '%s'\n", tool, key);
  } else {
    fprintf(err, "langc mcp: the argument '%s' of the tool '%s' must be a string with no NUL byte\n", key, tool);
  }
  return 0;
}

/* Sets *TARGET from the `target` argument of build. Returns 1, or 0 after
   one line on ERR. */
static int target_arg(const JsonValue *args, McpTarget *target, FILE *err) {
  const char *word = NULL;
  if (!string_arg(args, "build", "target", 1, &word, err)) return 0;
  if (find_target(word, target)) return 1;
  fputs("langc mcp: the argument 'target' of the tool 'build' must be \"main\" or \"runtime\"\n", err);
  return 0;
}

/* Runs the tool NAME with ARGS. Returns the exit code of the verb, or
   MCP_EXIT_USAGE after an argument failure (its line is on ERR), or -1 for
   an unknown tool. */
static int call_tool(const char *name, const JsonValue *args, McpRunTool run_tool, FILE *out, FILE *err) {
  McpArgs in;
  memset(&in, 0, sizeof in);
  if (strcmp(name, "check") == 0)
    return string_arg(args, name, "path", 1, &in.path, err) ? run_tool(MCP_TOOL_CHECK, &in, out, err) : MCP_EXIT_USAGE;
  if (strcmp(name, "fmt") == 0)
    return string_arg(args, name, "path", 1, &in.path, err) ? run_tool(MCP_TOOL_FMT, &in, out, err) : MCP_EXIT_USAGE;
  if (strcmp(name, "explain") == 0)
    return string_arg(args, name, "code", 1, &in.code, err) ? run_tool(MCP_TOOL_EXPLAIN, &in, out, err) : MCP_EXIT_USAGE;
  if (strcmp(name, "list-base") == 0) return run_tool(MCP_TOOL_LIST_BASE, &in, out, err);
  if (strcmp(name, "show-law") == 0)
    return string_arg(args, name, "row", 0, &in.row, err) ? run_tool(MCP_TOOL_SHOW_LAW, &in, out, err) : MCP_EXIT_USAGE;
  if (strcmp(name, "build") == 0) {
    int ok = string_arg(args, name, "path", 1, &in.path, err) && target_arg(args, &in.target, err) &&
             string_arg(args, name, "out", 1, &in.out_path, err);
    return ok ? run_tool(MCP_TOOL_BUILD, &in, out, err) : MCP_EXIT_USAGE;
  }
  return -1;
}

/* `tools/call`: item 1 is the CLI stdout of the verb, item 2 is the CLI
   stderr when it is not empty, and isError is exit != 0 (Q-S16-4 (a)). An
   argument failure of a known tool has this form also (Q-S16-7 (a)). Only
   an unknown tool, no `name` string or `arguments` that is not an object
   gives -32602. */
static void mcp_call(Arena *arena, const JsonValue *id, const JsonValue *params, McpRunTool run_tool, FILE *out) {
  (void)arena;
  const char *name = json_get_string(json_member(params, "name"));
  const JsonValue *args = json_member(params, "arguments");
  if (name == NULL || (args != NULL && args->kind != JSON_OBJECT)) {
    rpc_error(id, RPC_BAD_PARAMS, "Invalid params", out);
    return;
  }
  char *text = NULL;
  size_t len = 0;
  char *err_text = NULL;
  size_t err_len = 0;
  FILE *capture = open_memstream(&text, &len);
  FILE *err_capture = open_memstream(&err_text, &err_len);
  int opened = capture != NULL && err_capture != NULL;
  int status = opened ? call_tool(name, args, run_tool, capture, err_capture) : 0;
  /* Close both captures, also when the first close fails. */
  int closed = capture == NULL || fclose(capture) == 0;
  closed = (err_capture == NULL || fclose(err_capture) == 0) && closed;
  if (!opened || !closed) {
    rpc_error(id, RPC_INTERNAL, "Internal error", out);
  } else if (status < 0) {
    rpc_error(id, RPC_BAD_PARAMS, "Invalid params", out);
  } else {
    rpc_head(id, out);
    fputs(",\"result\":{\"content\":[{\"type\":\"text\",\"text\":", out);
    put_string(text != NULL ? text : "", text != NULL ? len : 0, out);
    if (err_len > 0) {
      fputs("},{\"type\":\"text\",\"text\":", out);
      put_string(err_text, err_len, out);
    }
    fprintf(out, "}],\"isError\":%s}}\n", status != 0 ? "true" : "false");
  }
  free(text);
  free(err_text);
}

typedef void (*McpMethod)(Arena *arena, const JsonValue *id, const JsonValue *params, McpRunTool run_tool, FILE *out);

static const struct {
  const char *name;
  McpMethod run;
} MCP_METHODS[] = {
  {"initialize", mcp_initialize},
  {"ping", mcp_ping},
  {"tools/list", mcp_tools_list},
  {"tools/call", mcp_call},
};

/* An id is absent, a string or an integer (Q-S16-1 (a), as the siblings). */
static int id_ok(const JsonValue *id) {
  return id == NULL || id->kind == JSON_STRING || (id->kind == JSON_NUMBER && strpbrk(id->text, ".eE") == NULL);
}

/* Answers the message in the LEN bytes of LINE on OUT. A notification (a
   valid request without an id) gets no reply. Any other method gives
   -32601 (Q-S16-7 (a)). */
static void mcp_message(Arena *arena, const char *line, size_t len, McpRunTool run_tool, FILE *out) {
  JsonValue msg;
  Diag diag;
  diag_init(&diag);
  if (!json_parse(arena, line, len, &msg, &diag)) {
    rpc_error(NULL, RPC_PARSE, "Parse error", out);
    return;
  }
  const JsonValue *id = json_member(&msg, "id");
  const JsonValue *params = json_member(&msg, "params");
  const char *version = json_get_string(json_member(&msg, "jsonrpc"));
  const JsonValue *method = json_member(&msg, "method");
  int valid = version != NULL && strcmp(version, "2.0") == 0 && method != NULL && method->kind == JSON_STRING &&
              id_ok(id) && (params == NULL || params->kind == JSON_OBJECT || params->kind == JSON_ARRAY);
  if (!valid) {
    rpc_error(id_ok(id) ? id : NULL, RPC_BAD_REQUEST, "Invalid Request", out);
    return;
  }
  if (id == NULL) return;
  for (size_t i = 0; i < sizeof MCP_METHODS / sizeof MCP_METHODS[0]; i++) {
    if (strlen(MCP_METHODS[i].name) == method->len && memcmp(MCP_METHODS[i].name, method->text, method->len) == 0) {
      MCP_METHODS[i].run(arena, id, params, run_tool, out);
      return;
    }
  }
  rpc_error(id, RPC_UNKNOWN_METHOD, "Method not found", out);
}

/* Reads one line of IN without its newline into LINE (MCP_LINE_MAX_BYTES
   bytes) and sets *LEN. Returns 0 at EOF before the first byte, 1 for a
   line, or -1 for a longer line: the rest of it up to the newline is read
   and dropped. */
static int read_line(FILE *in, char *line, size_t *len) {
  int c = getc(in);
  if (c == EOF) return 0;
  size_t n = 0;
  int over = 0;
  for (; c != EOF && c != '\n'; c = getc(in)) {
    over |= n == MCP_LINE_MAX_BYTES;
    if (!over) line[n++] = (char)c;
  }
  *len = n;
  return over ? -1 : 1;
}

int mcp_serve(FILE *in, FILE *out, McpRunTool run_tool) {
  char *line = malloc(MCP_LINE_MAX_BYTES);
  if (line == NULL) return 2;
  size_t len = 0;
  int got;
  int ok = 1;
  while (ok && (got = read_line(in, line, &len)) != 0) {
    len -= got > 0 && len > 0 && line[len - 1] == '\r';
    Arena arena;
    arena_init(&arena, MCP_ARENA_LIMIT_BYTES);
    if (got < 0) {
      rpc_error(NULL, RPC_PARSE, "Parse error", out);
    } else if (len > 0) {
      mcp_message(&arena, line, len, run_tool, out);
    }
    arena_release(&arena);
    ok = fflush(out) == 0 && !ferror(out);
  }
  free(line);
  return ok && !ferror(in) ? 0 : 2;
}
