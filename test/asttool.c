/* The AST dump of S14 (Q-S14-5 (a)): asttool PROG lexes and parses PROG
   as `langc fmt` does (ORIGIN_PROGRAM, no domain, no check) and prints
   the declarations as S-expressions without positions, one line per
   declaration. The gate law: dump(fmt x) = dump(x). A NULL name or term
   prints as `()`; names never hold a paren. */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "front/lexer.h"
#include "front/parser.h"

static void name(FILE *out, const char *s) {
  fputs(s == NULL ? "()" : s, out);
}

static void term(FILE *out, const Term *t);

static void pair(FILE *out, const char *tag, const Term *t) {
  fprintf(out, "(%s ", tag);
  term(out, t->left);
  fputc(' ', out);
  term(out, t->right);
  fputc(')', out);
}

static void binder(FILE *out, const char *tag, const Term *t) {
  fprintf(out, "(%s ", tag);
  name(out, t->name);
  fputc(' ', out);
  term(out, t->left);
  fputc(' ', out);
  term(out, t->right);
  fputc(')', out);
}

static void term(FILE *out, const Term *t) {
  if (t == NULL) {
    fputs("()", out);
    return;
  }
  switch (t->kind) {
  case TERM_VAR:
    fputs("(var ", out);
    name(out, t->name);
    fputc(')', out);
    return;
  case TERM_NAT:
    fprintf(out, "(nat %" PRIu64 ")", t->nat);
    return;
  case TERM_APP:
    pair(out, "app", t);
    return;
  case TERM_LAM:
    binder(out, "lam", t);
    return;
  case TERM_PI:
    binder(out, "pi", t);
    return;
  case TERM_SIGMA:
    binder(out, "sigma", t);
    return;
  case TERM_UNIV:
    fprintf(out, "(univ %" PRIu64 ")", t->nat);
    return;
  case TERM_PAPP:
    pair(out, "papp", t);
    return;
  case TERM_INEG:
    fputs("(ineg ", out);
    term(out, t->left);
    fputc(')', out);
    return;
  case TERM_IMEET:
    pair(out, "imeet", t);
    return;
  case TERM_IJOIN:
    pair(out, "ijoin", t);
    return;
  case TERM_SYSTEM:
    pair(out, "system", t);
    return;
  case TERM_BRANCH:
    pair(out, "branch", t);
    return;
  }
}

static void fields(FILE *out, const Field *fs, size_t count) {
  fputc('(', out);
  for (size_t k = 0; k < count; k++) {
    fputs(k == 0 ? "(field " : " (field ", out);
    name(out, fs[k].name);
    fputc(' ', out);
    term(out, fs[k].type);
    fprintf(out, " %d)", fs[k].history);
  }
  fputc(')', out);
}

static void ctors(FILE *out, const Ctor *cs, size_t count) {
  fputc('(', out);
  for (size_t k = 0; k < count; k++) {
    fputs(k == 0 ? "(ctor " : " (ctor ", out);
    name(out, cs[k].name);
    fputc(' ', out);
    fields(out, cs[k].fields, cs[k].field_count);
    fputc(' ', out);
    term(out, cs[k].result);
    fputc(' ', out);
    term(out, cs[k].boundary);
    fputc(')', out);
  }
  fputc(')', out);
}

static const char *kind_tag(DeclKind kind) {
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

static const char *origin_tag(Origin origin) {
  switch (origin) {
  case ORIGIN_DOMAIN:
    return "domain";
  case ORIGIN_KIT:
    return "kit";
  case ORIGIN_PROGRAM:
    return "program";
  }
  return "?";
}

static void decl(FILE *out, const Decl *d) {
  fprintf(out, "(%s %s ", kind_tag(d->kind), origin_tag(d->origin));
  name(out, d->name);
  fprintf(out, " (rec %d) (state %d) (sealed %d) (law %d) (block %zu %zu) ",
          d->is_rec, d->is_state, d->is_sealed, d->is_law, d->block_first, d->block_count);
  term(out, d->type);
  fputc(' ', out);
  term(out, d->body);
  fputc(' ', out);
  fields(out, d->params, d->param_count);
  fputc(' ', out);
  term(out, d->index);
  fputc(' ', out);
  ctors(out, d->ctors, d->ctor_count);
  fputs(")\n", out);
}

/* slurp reads PATH into an arena buffer; 0 when it cannot. */
static int slurp(Arena *arena, const char *path, char **text, size_t *len) {
  FILE *in = fopen(path, "rb");
  if (in == NULL) return 0;
  int ok = fseek(in, 0, SEEK_END) == 0;
  long size = ok ? ftell(in) : -1;
  ok = ok && size >= 0 && fseek(in, 0, SEEK_SET) == 0;
  *len = ok ? (size_t)size : 0;
  *text = ok ? arena_alloc(arena, *len + 1) : NULL;
  ok = ok && *text != NULL && fread(*text, 1, *len, in) == *len;
  fclose(in);
  return ok;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fputs("usage: asttool PROG\n", stderr);
    return 2;
  }
  Arena arena;
  Diag diag;
  TokenList tokens = {0};
  DeclList decls = {0};
  char *text = NULL;
  size_t len = 0;
  arena_init(&arena, (size_t)1 << 26);
  diag_init(&diag);
  if (!slurp(&arena, argv[1], &text, &len)) {
    fprintf(stderr, "asttool: cannot read %s\n", argv[1]);
    arena_release(&arena);
    return 2;
  }
  int ok = lex_source(&arena, argv[1], text, len, &tokens, &diag)
    && parse_source(&arena, argv[1], ORIGIN_PROGRAM, &tokens, &decls, &diag);
  if (ok) {
    for (size_t k = 0; k < decls.count; k++) decl(stdout, &decls.items[k]);
  } else {
    diag_print(&diag, stderr);
  }
  arena_release(&arena);
  return ok ? 0 : 1;
}
