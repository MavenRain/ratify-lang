/* The fix table of `langc check --json` (S15 A2, Q-S15-3 (a)). See fix.h. */
#include <string.h>

#include "front/fix.h"

typedef struct {
  const char *code;
  const char *fix;
} FixRow;

/* One row for each code, in code order. */
static const FixRow FIXES[] = {
    {"BLOCK_ORDER",
     "Name only the earlier sorts of the block in a header."},
    {"CHECK_DEPTH",
     "Make the term nest less deeply: move inner terms into their own definitions."},
    {"DOMAIN_FAMILY",
     "Change the field or the boundary that the message names. A "
     "recursive field has the type of the family at its own "
     "parameters, and only a path constructor has a boundary."},
    {"EVAL_ARGS",
     "Give one argument for each parameter of the entry, each in "
     "the syntax of its parameter type. Give no arguments to a "
     "definition that is not an entry."},
    {"EVAL_DEPTH",
     "Make the evaluation nest less deeply: move inner terms into their own definitions."},
    {"EVAL_ENTRY",
     "Give an entry definition that computes a number."},
    {"EVAL_FUEL",
     "Make the evaluation shorter: use smaller literals or fewer steps."},
    {"EVAL_PRINT",
     "Give an entry whose normal form is a closed value."},
    {"FIN_CASE",
     "Make each case of the Fin elimination check against its case type."},
    {"FIN_RANGE",
     "Use a literal below the bound of the Fin type."},
    {"FMT_DIFF",
     "Run langc fmt PROG and use its output as the program text."},
    {"FMT_TRAILING_COMMENT",
     "Move the comment to its own line."},
    {"GLUE_BINDER",
     "Write _ as the tube binder; a branch cannot use the tube variable."},
    {"GLUE_BOUNDARY",
     "Make unglue of each branch equal to the base of glue where its key holds."},
    {"GLUE_EQUIV",
     "Write each branch as pack T e with T : Type 0 and e : Equiv T A."},
    {"HCOMP_CAP",
     "Make the base of hcomp equal to each branch at i0."},
    {"HCOMP_UNIVERSE",
     "Use hcomp at Type 0, or declare Glue for the higher universe."},
    {"HIT_BOUNDARY",
     "Change the path constructor or the fold that the message "
     "names. A path constructor has its point fields first, then 1 "
     "or 2 interval fields, then a boundary. A fold needs a case "
     "type for each field that a boundary term uses."},
    {"HIT_INDEX",
     "Give each constructor of an indexed family a result type "
     "after its fields: the family at its own parameters. Give each "
     "boundary term of a path constructor the family at its own "
     "index."},
    {"HOST",
     "Give a host table with --host for the host imports of the entry."},
    {"HOST_TABLE",
     "Correct the host table at the line that the message names. "
     "Each line names a host import, then gives its words and one "
     "result as decimal u64 fields. Keep each line and the number "
     "of rows below the limits, and make sure that langc can read "
     "the file."},
    {"INTERNAL",
     "This is a fault of langc. Report the program that causes it."},
    {"INTERVAL_TYPE",
     "Use I only as the binder type of a path abstraction or a "
     "PathP line. The line of a path must be a type, not I (rule "
     "R9)."},
    {"IO",
     "Make sure that the program file exists and that langc can read it."},
    {"IO_SIZE",
     "Make the program file smaller than the size limit."},
    {"LEX_CHAR",
     "Remove or change the byte at the span. If a letter follows a "
     "Nat literal, put a space or an operator between them."},
    {"LEX_NAT_RANGE",
     "Use a Nat literal of 2^64 - 1 or less."},
    {"NAME_UNKNOWN",
     "Declare the name before its use, or correct its spelling."},
    {"OOM",
     "Make the program smaller, or give langc more memory."},
    {"PARSE_DEPTH",
     "Make the terms nest less deeply: move inner terms into their own definitions."},
    {"PARSE_EXPECT",
     "Write the expected token at the position of the span."},
    {"PATH_END",
     "Give a path whose ends are the ends that the type needs."},
    {"PATH_UIP",
     "Give a path whose ends are the ends that the type needs; only a rule of uniqueness can equate other ends."},
    {"REFUSE_APPROX",
     "An approximate scheme (CKKS). Use an exact scheme: the counts are exact integers."},
    {"REFUSE_ARROW",
     "A ranked choice over 3 or more options. Give a choice of 2 values: decide each pair of options with its own rule."},
    {"REFUSE_AXIOM",
     "Replace the axiom with a definition that has a proof term (rule R2)."},
    {"REFUSE_BOUND",
     "The product of (n_j + 1) is above 2^20 (Q-S0-4 (a)). Make the classes smaller, or use fewer classes."},
    {"REFUSE_CELL",
     "Use FreeMon of domain/domain.lang as the cell monad. For a "
     "family declaration, give the family the cell or cocell "
     "signature shape that the message names (rule R1)."},
    {"REFUSE_CLOCK",
     "A wall clock or a host tick. Use the log height as the time."},
    {"REFUSE_DATA",
     "Use the families of domain/domain.lang: a program cannot "
     "declare a family (rule R1). The same holds for a cofamily "
     "(rule R1) and a law def (rule O7)."},
    {"REFUSE_DEPTH",
     "Make the rule smaller: fewer gates or fewer minimal vectors (langc tfhe only)."},
    {"REFUSE_FIXPOINT",
     "A policy that ratifies itself, or general recursion. Use only earlier definitions, and fold or unfold for recursion."},
    {"REFUSE_FORK",
     "The O5 margin of the amend rule is 0 or less. Raise the amend "
     "thresholds until two minimal accepting vectors overlap in "
     "more than f members. If the message names ffRule, make ffRule "
     "the whole body of a closed def of type ForkFree c f."},
    {"REFUSE_FORMER",
     "A type constructor that is not CM or CC. Use a type former of the base signature (domain/domain.lang)."},
    {"REFUSE_HISTORY_WRITE",
     "Change the definition as the message says: remove the "
     "reference from a definition with State in its type, give the "
     "name all of its arguments (rule D-9), or write the history "
     "field as the old field or a natAdd of the old field."},
    {"REFUSE_HONEST",
     "The O4 margin is 0 or less. Raise the thresholds until each "
     "minimal accepting vector has a total above f. If the message "
     "names hoRule, make hoRule the whole body of a closed def of "
     "type Honest c f."},
    {"REFUSE_KIT",
     "Use only the names of the base span, or run the trusted kit mode langc --kit."},
    {"REFUSE_LAW",
     "Remove the law-only name: use the public names of domain/domain.lang."},
    {"REFUSE_LOWER",
     "Use only terms that the Wasm host can lower."},
    {"REFUSE_MAP",
     "Use Nat as the key type of the Map. Use Nat or Flag as the "
     "value type."},
    {"REFUSE_NAME",
     "Rename the definition (rule R4)."},
    {"REFUSE_NOIR",
     "Give each decided rule def its own K name, and keep each member count in the u32 range (langc noir only)."},
    {"REFUSE_ORBIT",
     "A rule names a party inside its class. Fix: a class with only that party."},
    {"REFUSE_PARTIAL",
     "A partial kit word. Use the total form of the word."},
    {"REFUSE_RULE",
     "A rule that is not monotone, accepts 0, or does not accept n (O3). Build the rule from rlMin, rlAnd, rlOr and rlTable over closed counts."},
    {"REFUSE_RUNTIME_CLOSURE",
     "Compute the function value at check time: a run-time value cannot be a function."},
    {"REFUSE_RUNTIME_TREE",
     "Compute the fold at check time: a run-time value cannot be a fold over a tree."},
    {"REFUSE_SIG",
     "Give the slot the type that the signature needs. If the "
     "message says that the name is a name of the design signature, "
     "give the definition another name."},
    {"REFUSE_STATE",
     "Declare one state only. Give it the name State, no "
     "parameters, no index telescope and one constructor. Give each "
     "state field a first-order type and each history field the "
     "type Nat."},
    {"SYSTEM_COVER",
     "Add branches until the keys of the system cover the face."},
    {"SYSTEM_OVERLAP",
     "Make the branches equal where two keys hold."},
    {"TRANSP_LINE",
     "Make the line of transp constant where its key holds."},
    {"TYPE_ARITY",
     "Give the number of arguments that the name needs."},
    {"TYPE_INFER",
     "Add the declared type that the term needs."},
    {"TYPE_MISMATCH",
     "Change the term until its type is the expected type, or change the declared type."},
    {"TYPE_NOT_FUNCTION",
     "Apply only a term of a function type."},
    {"TYPE_UNIVERSE",
     "Use a universe level at or below the largest universe."},
    {"UNGLUE_TYPE",
     "Apply unglue only to a term of a Glue type."},
    {"USAGE",
     "Give a command and a program path: langc [--kit] check PROG [--json]."},
};

const char *fix_of(const char *code) {
  size_t i;
  for (i = 0; code != NULL && i < sizeof FIXES / sizeof FIXES[0]; i++)
    if (strcmp(FIXES[i].code, code) == 0) return FIXES[i].fix;
  return NULL;
}

size_t fix_count(void) { return sizeof FIXES / sizeof FIXES[0]; }

const char *fix_code(size_t i) { return i < fix_count() ? FIXES[i].code : NULL; }
