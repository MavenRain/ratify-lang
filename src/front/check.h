/* The type checker, the refusals R1-R4 and the eval command. SHARED by the
   tcc kits. */
#ifndef LANG_FRONT_CHECK_H
#define LANG_FRONT_CHECK_H

#include "front/core.h"

#define ENTRY_PARAMS_MAX 32u /* 16 before sealed-lang S2 (R23): slashUsr takes 19 words */

/* An entry: a program definition of type Nat, Flag, or a function of Nat and
   Flag arguments to a Nat or Flag result. */
typedef struct {
  uint32_t param_count;
  int param_flag[ENTRY_PARAMS_MAX];
  int result_flag;
  int result_option;
} Entry;

/* Checks the domain declarations, then the program, into M. Returns 1, or 0
   after a diagnostic. */
/* The check mode (orbit-lang S3). In guest mode a program declaration
   cannot use a kit item (REFUSE_KIT). Kit mode is the trusted test scope of
   the kit gate (`langc --kit`). */
typedef enum {
  CHECK_GUEST,
  CHECK_KIT
} CheckMode;

int check_program(Arena *arena, const DeclList *decls, Machine *m, Diag *diag, CheckMode mode);
/* S11 (Q-S11-4 (a)): as check_program, and keeps a RuleFacts record on M
   for each decided rule def (Machine.rule_facts) for `langc report`. */
int check_program_facts(Arena *arena, const DeclList *decls, Machine *m, Diag *diag, CheckMode mode);
/* S15 (Q-S15-4 (a)): the program declarations (ORIGIN_PROGRAM) among the
   first N of DECLS. `langc check --json` gives it as `parsed` (N = all). */
uint32_t check_program_count(const DeclList *decls, size_t n);
/* S11: decides the rule value RULE at the closed committee COMMITTEE as
   KIND (0 Rule, 1 Honest, 2 ForkFree) at the fault bound F, and fills OUT
   from the arena of M (name NULL). Returns 1 when the rule passes O3 (and
   O4, O5 by KIND); 0 for an open committee, a refusal or out of memory. */
int check_rule_facts(Machine *m, const Value *committee, const Value *rule, int kind, int64_t f, RuleFacts *out);
/* Returns 1 when TYPE is the type of an entry. */
int entry_of(Machine *m, const Value *type, Entry *out);
/* Evaluates the definition NAME on ARGS and prints the result. Returns 0, 1
   after a diagnostic, or 2 after an EVAL_ARGS diagnostic. */
int eval_command(Machine *m, const char *name, char *const *args, int arg_count, FILE *out, FILE *err);
/* S9 step d: read the `langc eval --host FILE` table into M (eval.c
   host_call uses it). Gives 1, or 0 after a diagnostic. */
int host_table_read(Machine *m, FILE *f);

#endif
