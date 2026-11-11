/* The langc command line. SHARED by the tcc kits.
   Exit 0: ok. Exit 1: the program is refused, or explain or show-law does
   not know the name. Exit 2: usage or IO. */
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "front/check.h"
#include "front/fix.h"
#include "front/fmt.h"
#include "front/front.h"
#include "front/lower.h"
#include "front/noir.h"
#include "front/report.h"
#include "mcp.h"
#include "target.h"

#define ARENA_LIMIT_BYTES ((size_t)1 << 30)

typedef enum {
  CMD_CHECK,
  CMD_EVAL,
  CMD_IR,
  CMD_ABI,
  CMD_BUILD,
  CMD_REPORT,
  CMD_TFHE,
  CMD_NOIR,
  CMD_FMT,
  CMD_EXPLAIN,   /* S16 U1 */
  CMD_LIST_BASE, /* S16 U1 */
  CMD_SHOW_LAW,  /* S16 U1 */
  CMD_MCP,       /* S16 U2 */
} Command;

typedef struct {
  Command command;
  const char *prog_path;
  const char *entry;
  char **args;
  int arg_count;
  const char *out_path;
  int runtime;
  int json;
  CheckMode mode; /* orbit-lang S3: --kit selects the kit mode */
  const char *host_path; /* S9 step d: langc eval ... --host FILE */
  int check; /* S14 A1: langc fmt PROG --check */
  const char *name; /* S16 U1: the CODE of explain or the ROW of show-law, or NULL */
  FILE *out; /* S16 U1: each verb writes to these two streams (D-U1-1) */
  FILE *err;
} Options;

/* S9 step d: read the host table at PATH into M. */
static int read_host_table(Machine *m, const char *path, Diag *diag) {
  FILE *f = fopen(path, "r");
  int ok;
  if (f == NULL)
    return diag_fail(diag, "USAGE", NULL, "the host table %s cannot be opened", path);
  ok = host_table_read(m, f);
  fclose(f);
  return ok;
}

static const struct {
  const char *word;
  Command command;
} COMMANDS[] = {
  {"check", CMD_CHECK},
  {"eval", CMD_EVAL},
  {"ir", CMD_IR},
  {"abi", CMD_ABI},
  {"build", CMD_BUILD},
  {"report", CMD_REPORT},
  {"tfhe", CMD_TFHE},
  {"noir", CMD_NOIR},
  {"fmt", CMD_FMT},
  {"explain", CMD_EXPLAIN},
  {"list-base", CMD_LIST_BASE},
  {"show-law", CMD_SHOW_LAW},
  {"mcp", CMD_MCP},
};

static int usage(FILE *err) {
  fputs("usage: langc [--kit] check PROG [--json]\n"
        "       langc [--kit] eval PROG NAME [ARGS...]\n"
        "       langc [--kit] ir PROG\n"
        "       langc [--kit] abi PROG [--text]\n"
        "       langc [--kit] build PROG -o OUT [--runtime]\n"
        "       langc [--kit] report PROG\n"
        "       langc [--kit] tfhe PROG\n"
        "       langc [--kit] noir PROG\n"
        "       langc fmt PROG [--check]\n"
        "       langc explain CODE\n"
        "       langc [--kit] list-base\n"
        "       langc show-law [ROW]\n"
        "       langc [--kit] mcp\n"
        "--kit: the kit mode, the trusted test scope of the kit gate\n", err);
  return 2;
}

static int find_command(const char *word, Command *out) {
  for (size_t i = 0; i < sizeof COMMANDS / sizeof COMMANDS[0]; i++) {
    if (strcmp(COMMANDS[i].word, word) == 0) {
      *out = COMMANDS[i].command;
      return 1;
    }
  }
  return 0;
}

static int parse_build_options(int argc, char **argv, Options *opt, Diag *diag) {
  for (int i = 3; i < argc; i++) {
    int is_out = strcmp(argv[i], "-o") == 0 && i + 1 < argc;
    int is_runtime = strcmp(argv[i], "--runtime") == 0;
    if (!is_out && !is_runtime) return diag_fail(diag, "USAGE", NULL, "unknown build argument '%s'", argv[i]);
    opt->out_path = is_out ? argv[i + 1] : opt->out_path;
    opt->runtime |= is_runtime;
    i += is_out;
  }
  return opt->out_path != NULL ? 1 : diag_fail(diag, "USAGE", NULL, "build needs -o OUT");
}

/* S16 U1: the verbs with no program path: `explain CODE`, `list-base` and
   `show-law [ROW]`. S16 U2: `mcp` also. Returns 1, or 0 after a USAGE
   diagnostic. */
static int parse_plain_options(int argc, char **argv, Options *opt, Diag *diag) {
  opt->name = argc >= 3 ? argv[2] : NULL;
  switch (opt->command) {
    case CMD_EXPLAIN:
      if (argc == 2) return diag_fail(diag, "USAGE", NULL, "explain needs a code");
      return argc == 3 ? 1 : diag_fail(diag, "USAGE", NULL, "explain takes only a code");
    case CMD_LIST_BASE:
      return argc == 2 ? 1 : diag_fail(diag, "USAGE", NULL, "list-base takes no argument");
    case CMD_SHOW_LAW:
      return argc <= 3 ? 1 : diag_fail(diag, "USAGE", NULL, "show-law takes only a row name");
    case CMD_MCP:
      return argc == 2 ? 1 : diag_fail(diag, "USAGE", NULL, "mcp takes no argument");
    case CMD_FMT:
    case CMD_CHECK:
    case CMD_EVAL:
    case CMD_IR:
    case CMD_ABI:
    case CMD_BUILD:
    case CMD_REPORT:
    case CMD_TFHE:
    case CMD_NOIR:
      break; /* parse_options has these */
  }
  return 0;
}

static int parse_options(int argc, char **argv, Options *opt, Diag *diag) {
  memset(opt, 0, sizeof *opt);
  if (argc < 2 || !find_command(argv[1], &opt->command)) {
    return diag_fail(diag, "USAGE", NULL, "expected a command and a program path");
  }
  /* S16 U1: explain, list-base and show-law have no program path. S16 U2:
     mcp also. */
  if (opt->command == CMD_EXPLAIN || opt->command == CMD_LIST_BASE || opt->command == CMD_SHOW_LAW ||
      opt->command == CMD_MCP)
    return parse_plain_options(argc, argv, opt, diag);
  if (argc < 3) return diag_fail(diag, "USAGE", NULL, "expected a command and a program path");
  opt->prog_path = argv[2];
  switch (opt->command) {
    case CMD_CHECK:
      /* S15 A2: --json gives the one line of docs/CHECK-JSON.md. */
      opt->json = argc == 4;
      return argc == 3 || (argc == 4 && strcmp(argv[3], "--json") == 0) ? 1
                                                                          : diag_fail(diag, "USAGE", NULL, "too many arguments");
    case CMD_IR:
    case CMD_REPORT:
    case CMD_TFHE:
    case CMD_NOIR:
      return argc == 3 ? 1 : diag_fail(diag, "USAGE", NULL, "too many arguments");
    case CMD_ABI:
      /* The JSON ABI is the default; --text gives the C-K4-16 lines (C-c7-1). */
      opt->json = argc == 3;
      return argc == 3 || (argc == 4 && strcmp(argv[3], "--text") == 0) ? 1
        : diag_fail(diag, "USAGE", NULL, "abi takes only --text after the program");
    case CMD_FMT:
      /* S14 A1 (N-2): --check compares the text with its canonical text. */
      opt->check = argc == 4;
      return argc == 3 || (argc == 4 && strcmp(argv[3], "--check") == 0) ? 1
        : diag_fail(diag, "USAGE", NULL, "fmt takes only --check after the program");
    case CMD_EVAL:
      if (argc < 4) return diag_fail(diag, "USAGE", NULL, "eval needs a definition name");
      opt->entry = argv[3];
      opt->args = argv + 4;
      opt->arg_count = argc - 4;
      /* S9 step d: a trailing `--host FILE` gives the host table. */
      opt->host_path = NULL;
      if (opt->arg_count >= 2 && strcmp(argv[argc - 2], "--host") == 0) {
        opt->host_path = argv[argc - 1];
        opt->arg_count -= 2;
      }
      return 1;
    case CMD_BUILD:
      return parse_build_options(argc, argv, opt, diag);
    case CMD_EXPLAIN:
    case CMD_LIST_BASE:
    case CMD_SHOW_LAW:
    case CMD_MCP:
      break; /* S16 U1: parse_plain_options has these */
  }
  return 0;
}

static int read_source(Arena *arena, const char *path, char **out, size_t *len, Diag *diag) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) return diag_fail(diag, "IO", NULL, "cannot open %s", path);
  char *buf = arena_alloc(arena, SOURCE_MAX_BYTES + 1);
  size_t count = buf == NULL ? 0 : fread(buf, 1, SOURCE_MAX_BYTES + 1, file);
  int read_error = ferror(file);
  fclose(file);
  if (buf == NULL) return diag_fail(diag, "OOM", NULL, "no memory for %s", path);
  if (read_error) return diag_fail(diag, "IO", NULL, "cannot read %s", path);
  if (count > SOURCE_MAX_BYTES) return diag_fail(diag, "IO_SIZE", NULL, "%s is larger than %zu bytes", path, SOURCE_MAX_BYTES);
  *out = buf;
  *len = count;
  return 1;
}

/* Lowers the entries, then runs the ir, abi or build command. */
static int run_target(const Options *opt, Machine *machine, Diag *diag) {
  IrProgram prog;
  FILE *out;
  size_t i;
  int ok = 1;
  if (!lower_program(machine, &prog)) return 1;
  switch (opt->command) {
    case CMD_IR:
      ir_print(&prog, opt->out);
      return 0;
    case CMD_ABI:
      for (i = 0; i < prog.func_count && ok; i++) ok = target_abi_line(&prog.funcs[i], opt->out, opt->err);
      return ok ? 0 : 1;
    case CMD_CHECK:
    case CMD_EVAL:
    case CMD_BUILD:
    case CMD_REPORT:
    case CMD_TFHE:
    case CMD_NOIR:
    case CMD_FMT:
    case CMD_EXPLAIN:
    case CMD_LIST_BASE:
    case CMD_SHOW_LAW:
    case CMD_MCP:
      break;
  }
  out = fopen(opt->out_path, "wb");
  if (out == NULL) return diag_fail(diag, "IO", NULL, "cannot write '%s'", opt->out_path) + 1;
  ok = target_write(&prog, opt->runtime ? TARGET_PART_RUNTIME : TARGET_PART_MAIN, out, opt->err);
  return fclose(out) == 0 && ok ? 0 : 1;
}

/* S15 A2 (Q-S15-1 (a)): `langc [--kit] check PROG --json`. The exit code is
   the one of `langc check`; the diagnostic goes into the JSON line on
   stdout, and no text goes to stderr. Only a failure of the writer itself
   gives text on stderr (exit 2). */
static int run_check_json(const Options *opt, Arena *arena, Diag *diag) {
  char *text = NULL;
  size_t len = 0;
  DeclList decls;
  Machine machine;
  Diag own;
  int read = read_source(arena, opt->prog_path, &text, &len, diag);
  int loaded = read && front_load(arena, opt->prog_path, text, len, &decls, diag);
  int checked = loaded && check_program_facts(arena, &decls, &machine, diag, opt->mode);
  diag_init(&own);
  if (!report_check(arena, loaded ? &machine : NULL, diag, loaded ? check_program_count(&decls, decls.count) : 0u, &own,
                    opt->out)) {
    diag_print(&own, opt->err);
    return 2;
  }
  return read ? !checked : 2;
}

/* S16 U1: `langc explain CODE` writes `CODE: FIX`, the row of CODE in the
   fix table (src/front/fix.c). An unknown code: the code EXPLAIN, exit 1. */
static int run_explain(const Options *opt, Diag *diag) {
  const char *fix = fix_of(opt->name);
  if (fix == NULL) return diag_fail(diag, "EXPLAIN", NULL, "'%s' is not a code of the fix table", opt->name) + 1;
  fprintf(opt->out, "%s: %s\n", opt->name, fix);
  return 0;
}

/* S16 U1: `langc show-law [ROW]` writes one line for the row ROW of the sec
   6 law table, or each row with no ROW. An unknown row: the code LAW, exit
   1. */
static int run_show_law(const Options *opt, Diag *diag) {
  if (report_laws(opt->name, opt->out) > 0) return 0;
  return diag_fail(diag, "LAW", NULL, "'%s' is not a law row of docs/PLAN.md section 6", opt->name) + 1;
}

static const char *list_base_kind(DeclKind kind) {
  switch (kind) {
    case DECL_DEF:
      return "def";
    case DECL_AXIOM:
      return "axiom";
    case DECL_FAMILY:
      return "family";
    case DECL_COFAMILY:
      return "cofamily";
  }
  return "?";
}

/* S16 U1: `langc list-base` writes the names that a guest program has
   before its first line: one line `NAME builtin -` for each guest builtin,
   then one line `NAME KIND SOURCE:LINE` for each declaration of the domain
   file. `langc --kit list-base` writes only the rows of the kit signature. */
static int run_list_base(const Options *opt, Arena *arena, Diag *diag) {
  DeclList decls;
  size_t i;
  if (opt->mode == CHECK_KIT) {
    check_list_signature(opt->out);
    return 0;
  }
  if (!front_load(arena, "list-base", "", 0, &decls, diag)) return 1;
  check_list_builtins(opt->out);
  for (i = 0; i < decls.count; i++) {
    const Decl *d = &decls.items[i];
    if (d->origin == ORIGIN_DOMAIN)
      fprintf(opt->out, "%s %s %s:%d\n", d->name, list_base_kind(d->kind), d->source, d->line);
  }
  return 0;
}

/* S16 U1: the end of a verb. A status other than 0 prints the diagnostic on
   the err stream of OPT; `check --json` has it in the JSON line. */
static int finish_status(const Options *opt, Diag *diag, int status) {
  if (status != 0 && !(opt->command == CMD_CHECK && opt->json)) diag_print(diag, opt->err);
  return status;
}

static int run(const Options *opt, Arena *arena, Diag *diag) {
  char *text = NULL;
  size_t len = 0;
  /* S16 U1: the three verbs with no program path. */
  if (opt->command == CMD_EXPLAIN) return run_explain(opt, diag);
  if (opt->command == CMD_SHOW_LAW) return run_show_law(opt, diag);
  if (opt->command == CMD_LIST_BASE) return run_list_base(opt, arena, diag);
  if (opt->command == CMD_CHECK && opt->json) return run_check_json(opt, arena, diag);
  if (!read_source(arena, opt->prog_path, &text, &len, diag)) return 2;
  /* S14 A1: fmt lexes and parses the program as check does, and checks nothing. */
  if (opt->command == CMD_FMT) return fmt_program(arena, opt->prog_path, text, len, opt->check, opt->out, diag);
  DeclList decls;
  if (!front_load(arena, opt->prog_path, text, len, &decls, diag)) return 1;
  Machine machine;
  /* S11: the report keeps the facts of each decided rule def (no second decide).
     S12: so does tfhe (the T2 circuits are built from the facts).
     S13: so does noir (the K globals are the member counts of the facts). */
  int facts = opt->command == CMD_REPORT || opt->command == CMD_TFHE || opt->command == CMD_NOIR;
  int checked = facts ? check_program_facts(arena, &decls, &machine, diag, opt->mode)
                      : check_program(arena, &decls, &machine, diag, opt->mode);
  if (!checked) return 1;
  switch (opt->command) {
    case CMD_CHECK:
      fputs("ok\n", opt->out);
      return 0;
    case CMD_EVAL:
      if (opt->host_path != NULL && !read_host_table(&machine, opt->host_path, diag))
        return 2;
      return eval_command(&machine, opt->entry, opt->args, opt->arg_count, opt->out, opt->err);
    case CMD_REPORT:
      return report_program(&machine, opt->out) ? 0 : 1;
    case CMD_TFHE:
      return report_tfhe(&machine, opt->out) ? 0 : 1;
    case CMD_NOIR:
      return report_noir(&machine, opt->out) ? 0 : 1;
    case CMD_IR:
    case CMD_ABI:
    case CMD_BUILD:
    case CMD_FMT:
    case CMD_EXPLAIN:
    case CMD_LIST_BASE:
    case CMD_SHOW_LAW:
    case CMD_MCP:
      break;
  }
  return run_target(opt, &machine, diag);
}

/* S16 U2: the mode of `langc [--kit] mcp`. main sets it before mcp_serve. */
static CheckMode mcp_mode = CHECK_GUEST;

/* S16 U2 (Q-S16-5 (a)): runs the verb of TOOL as the CLI does, with fresh
   Options, a fresh Diag and a fresh Arena. The CLI stdout goes to OUT and
   the CLI stderr to ERR. Returns the CLI exit code. */
static int mcp_tool(McpTool tool, const McpArgs *args, FILE *out, FILE *err) {
  Options opt;
  memset(&opt, 0, sizeof opt);
  opt.mode = mcp_mode;
  opt.out = out;
  opt.err = err;
  switch (tool) {
    case MCP_TOOL_CHECK:
      opt.command = CMD_CHECK;
      opt.prog_path = args->path;
      opt.json = 1;
      break;
    case MCP_TOOL_FMT:
      opt.command = CMD_FMT;
      opt.prog_path = args->path;
      break;
    case MCP_TOOL_EXPLAIN:
      opt.command = CMD_EXPLAIN;
      opt.name = args->code;
      break;
    case MCP_TOOL_LIST_BASE:
      opt.command = CMD_LIST_BASE;
      break;
    case MCP_TOOL_SHOW_LAW:
      opt.command = CMD_SHOW_LAW;
      opt.name = args->row;
      break;
    case MCP_TOOL_BUILD:
      opt.command = CMD_BUILD;
      opt.prog_path = args->path;
      opt.out_path = args->out_path;
      opt.runtime = args->target == MCP_TARGET_RUNTIME;
      break;
  }
  Diag diag;
  diag_init(&diag);
  Arena arena;
  arena_init(&arena, ARENA_LIMIT_BYTES);
  int status = run(&opt, &arena, &diag);
  arena_release(&arena);
  return finish_status(&opt, &diag, status);
}

int main(int argc, char **argv) {
  Diag diag;
  diag_init(&diag);
  Options opt;
  int kit = argc > 1 && strcmp(argv[1], "--kit") == 0;
  if (!parse_options(argc - kit, argv + kit, &opt, &diag)) {
    diag_print(&diag, stderr);
    return usage(stderr);
  }
  opt.mode = kit ? CHECK_KIT : CHECK_GUEST;
  opt.out = stdout;
  opt.err = stderr;
  /* S16 U2: mcp serves the tools on stdin and stdout to the end of stdin;
     each call has its own Options, Diag and Arena (Q-S16-5 (a)). */
  if (opt.command == CMD_MCP) {
    mcp_mode = opt.mode;
    return mcp_serve(stdin, stdout, mcp_tool);
  }
  Arena arena;
  arena_init(&arena, ARENA_LIMIT_BYTES);
  int status = run(&opt, &arena, &diag);
  arena_release(&arena);
  return finish_status(&opt, &diag, status);
}
