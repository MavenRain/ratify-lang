/* `langc fmt` (S14 A1, Q-S14-1 to 4 (a); the notes N-1 to N-3 and N-U1-x
   are in s14/U1-RECORD.md). One declaration on one line, each family
   constructor after the first on its own line `  | c ...`, and above 100
   columns one break after `:=`. lex_source drops the comments, so a side
   scan of the text keeps the full-line comments: each one prints above the
   declaration that follows it, or at the end. */
#include <string.h>

#include "front/fmt.h"

#define FMT_WIDTH 100

/* The canonical text, in arena blocks that double. */
typedef struct {
  Arena *arena;
  char *buf;
  size_t len;
  size_t cap;
  int oom;
} Out;

static void put_n(Out *o, const char *text, size_t n) {
  if (o->oom || n == 0) return;
  if (o->len + n > o->cap) {
    size_t next = o->cap == 0 ? 256u : o->cap;
    while (next < o->len + n) next *= 2u;
    char *fresh = arena_alloc(o->arena, next);
    if (fresh == NULL) {
      o->oom = 1;
      return;
    }
    if (o->len > 0) memcpy(fresh, o->buf, o->len);
    o->buf = fresh;
    o->cap = next;
  }
  memcpy(o->buf + o->len, text, n);
  o->len += n;
}

static void put(Out *o, const char *text) {
  put_n(o, text, strlen(text));
}

static void put_nat(Out *o, uint64_t n) {
  char digits[24];
  size_t at = sizeof digits;
  do {
    at--;
    digits[at] = (char)('0' + n % 10u);
    n /= 10u;
  } while (n > 0);
  put_n(o, digits + at, sizeof digits - at);
}

/* The levels of the grammar in parser.c, loosest first. */
typedef enum {
  LV_TERM,
  LV_JOIN,
  LV_MEET,
  LV_PAPP,
  LV_APP,
  LV_ATOM
} Level;

/* `Type n` and `Sigma (x : A) B` are atoms of the grammar, but fmt gives
   them the level of an application (N-U1-3): `Path (Type 0) A B`. */
static Level level_of(const Term *t) {
  switch (t->kind) {
    case TERM_VAR:
    case TERM_NAT:
    case TERM_INEG:
      return LV_ATOM;
    case TERM_UNIV:
    case TERM_SIGMA:
    case TERM_APP:
      return LV_APP;
    case TERM_PAPP:
      return LV_PAPP;
    case TERM_IMEET:
      return LV_MEET;
    case TERM_IJOIN:
      return LV_JOIN;
    case TERM_LAM:
    case TERM_PI:
    case TERM_SYSTEM:
    case TERM_BRANCH:
      return LV_TERM;
  }
  return LV_TERM;
}

static void term(Out *o, const Term *t, Level need);

/* `(x : A)`: one name per group (Q-S14-3). */
static void binder(Out *o, const char *name, const Term *type) {
  put(o, "(");
  put(o, name);
  put(o, " : ");
  term(o, type, LV_TERM);
  put(o, ")");
}

/* `[ k1 -> t1, ... ]`, or `[ ]`. Only the first cell has no branch. */
static void system_cells(Out *o, const Term *cell) {
  put(o, "[");
  for (const Term *c = cell; c != NULL && c->left != NULL; c = c->right) {
    put(o, c == cell ? " " : ", ");
    term(o, c->left->left, LV_JOIN);
    put(o, " -> ");
    term(o, c->left->right, LV_TERM);
  }
  put(o, " ]");
}

/* Consecutive fun binders merge into one `fun` (Q-S14-3); a bare binder
   stays bare. */
static void lambda(Out *o, const Term *t) {
  const Term *body = t;
  put(o, "fun");
  for (; body->kind == TERM_LAM; body = body->right) {
    put(o, " ");
    if (body->left == NULL) put(o, body->name);
    else binder(o, body->name, body->left);
  }
  put(o, " => ");
  if (body->kind == TERM_SYSTEM) system_cells(o, body);
  else term(o, body, LV_TERM);
}

static void term_body(Out *o, const Term *t) {
  switch (t->kind) {
    case TERM_VAR:
      put(o, t->name);
      return;
    case TERM_NAT:
      put_nat(o, t->nat);
      return;
    case TERM_UNIV:
      put(o, "Type ");
      put_nat(o, t->nat);
      return;
    case TERM_APP:
      term(o, t->left, LV_APP);
      put(o, " ");
      term(o, t->right, LV_ATOM);
      return;
    case TERM_LAM:
      lambda(o, t);
      return;
    case TERM_PI:
      if (t->name != NULL) binder(o, t->name, t->left);
      else term(o, t->left, LV_JOIN);
      put(o, " -> ");
      term(o, t->right, LV_TERM);
      return;
    case TERM_SIGMA:
      /* The keyword form, not `(x : A) * B` (Q-S14-3). */
      put(o, "Sigma ");
      binder(o, t->name, t->left);
      put(o, " ");
      term(o, t->right, LV_ATOM);
      return;
    case TERM_PAPP:
      term(o, t->left, LV_PAPP);
      put(o, " @ ");
      term(o, t->right, LV_APP);
      return;
    case TERM_INEG:
      put(o, "~");
      term(o, t->left, LV_ATOM);
      return;
    case TERM_IMEET:
      term(o, t->left, LV_MEET);
      put(o, " /\\ ");
      term(o, t->right, LV_PAPP);
      return;
    case TERM_IJOIN:
      term(o, t->left, LV_JOIN);
      put(o, " \\/ ");
      term(o, t->right, LV_MEET);
      return;
    case TERM_SYSTEM:
      system_cells(o, t);
      return;
    case TERM_BRANCH:
      term(o, t->left, LV_JOIN);
      put(o, " -> ");
      term(o, t->right, LV_TERM);
      return;
  }
}

/* T, in parentheses only when its level is looser than NEED. */
static void term(Out *o, const Term *t, Level need) {
  int paren = level_of(t) < need;
  if (paren) put(o, "(");
  term_body(o, t);
  if (paren) put(o, ")");
}

/* ` (x : A)` per field, with the history marker of the state form. */
static void fields(Out *o, const Field *items, size_t count) {
  for (size_t i = 0; i < count; i++) {
    put(o, items[i].history ? " history " : " ");
    binder(o, items[i].name, items[i].type);
  }
}

static void ctor(Out *o, const Ctor *c) {
  put(o, c->name);
  fields(o, c->fields, c->field_count);
  if (c->result != NULL) {
    put(o, " : ");
    term(o, c->result, LV_TERM);
  }
  if (c->boundary != NULL) {
    put(o, " ");
    system_cells(o, c->boundary);
  }
}

/* A family, a state form or a cofamily; `and` for a later sort of a block.
   *DEFINE is the offset of the space after `:=`. */
static void family(Out *o, const DeclList *list, size_t at, size_t *define) {
  const Decl *d = &list->items[at];
  put(o, at > d->block_first ? "and "
       : d->is_state ? "state "
       : d->is_sealed ? "sealed family "
       : d->kind == DECL_COFAMILY ? "cofamily " : "family ");
  put(o, d->name);
  fields(o, d->params, d->param_count);
  if (d->index != NULL) {
    put(o, " : ");
    term(o, d->index, LV_TERM);
  }
  put(o, " :=");
  /* No constructor: no space after `:=`, so no break. */
  *define = d->ctor_count > 0 ? o->len : 0;
  for (size_t i = 0; i < d->ctor_count; i++) {
    put(o, i == 0 ? " " : "\n  | ");
    ctor(o, &d->ctors[i]);
  }
}

/* Turns the space at AT into a line break and an indent of 2. */
static void break_at(Out *o, size_t at) {
  put_n(o, "  ", 2u);
  if (o->oom) return;
  memmove(o->buf + at + 3u, o->buf + at + 1u, o->len - at - 3u);
  memcpy(o->buf + at, "\n  ", 3u);
}

/* One declaration and its newline. Above FMT_WIDTH bytes, its first line
   breaks once after `:=` (Q-S14-4, N-3). */
static void decl(Out *o, const DeclList *list, size_t at) {
  const Decl *d = &list->items[at];
  size_t start = o->len;
  size_t define = 0;
  switch (d->kind) {
    case DECL_DEF:
      put(o, d->is_law ? "law def " : "def ");
      if (d->is_rec) put(o, "rec ");
      put(o, d->name);
      put(o, " : ");
      term(o, d->type, LV_TERM);
      put(o, " :=");
      define = o->len;
      put(o, " ");
      term(o, d->body, LV_TERM);
      break;
    case DECL_AXIOM:
      put(o, "axiom ");
      put(o, d->name);
      put(o, " : ");
      term(o, d->type, LV_TERM);
      break;
    case DECL_FAMILY:
    case DECL_COFAMILY:
      family(o, list, at, &define);
      break;
  }
  if (o->oom) return;
  const char *nl = memchr(o->buf + start, '\n', o->len - start);
  size_t width = (nl == NULL ? o->buf + o->len : nl) - (o->buf + start);
  if (define > 0 && width > FMT_WIDTH) break_at(o, define);
  put(o, "\n");
}

static int before(const Token *tok, const Decl *d) {
  return tok->line < d->line || (tok->line == d->line && tok->col < d->col);
}

/* The line of the last token of the declaration AT. *CURSOR moves forward
   over the tokens, so the scan is linear. */
static int last_line(const TokenList *toks, const DeclList *list, size_t at, size_t *cursor) {
  const Decl *next = at + 1 < list->count ? &list->items[at + 1] : NULL;
  size_t k = *cursor;
  while (k + 1 < toks->count && (next == NULL || before(&toks->items[k], next))) k++;
  *cursor = k;
  return k > 0 ? toks->items[k - 1].line : 0;
}

typedef enum {
  LINE_BLANK,
  LINE_COMMENT,
  LINE_CODE
} LineKind;

/* One line of the text. A `--` starts a comment wherever it is: no token
   has `--` in it. */
typedef struct {
  LineKind kind;
  int trailing; /* code, then a comment */
  size_t dash;  /* the offset of `--` */
  size_t comment_len;
  size_t next;  /* the offset of the next line */
} Line;

static int is_space(char c) {
  return c == ' ' || c == '\t' || c == '\r';
}

static Line scan_line(const char *text, size_t len, size_t pos) {
  Line line;
  size_t end = pos;
  while (end < len && text[end] != '\n') end++;
  size_t dash = pos;
  while (dash + 1 < end && (text[dash] != '-' || text[dash + 1] != '-')) dash++;
  int comment = dash + 1 < end;
  size_t code_end = comment ? dash : end;
  int code = 0;
  for (size_t i = pos; i < code_end; i++) code |= !is_space(text[i]);
  size_t stop = end;
  while (comment && stop > dash && is_space(text[stop - 1])) stop--;
  line.kind = code ? LINE_CODE : comment ? LINE_COMMENT : LINE_BLANK;
  line.trailing = code && comment;
  line.dash = dash;
  line.comment_len = comment ? stop - dash : 0;
  line.next = end < len ? end + 1 : len;
  return line;
}

int fmt_program(Arena *arena, const char *source, const char *text, size_t len, int check, FILE *out, Diag *diag) {
  TokenList toks;
  DeclList list = {NULL, 0, 0};
  if (!lex_source(arena, source, text, len, &toks, diag) || !parse_source(arena, source, ORIGIN_PROGRAM, &toks, &list, diag))
    return 1;
  Out o = {arena, NULL, 0, 0, 0};
  size_t next_decl = 0;
  size_t cursor = 0;
  int span_end = 0; /* the last line of the last declaration printed */
  int blank = 0;    /* N-1: one blank line is due before the next item */
  int line_no = 1;
  for (size_t pos = 0; pos < len; line_no++) {
    Line line = scan_line(text, len, pos);
    if (line.trailing)
      return diag_fail(diag, "FMT_TRAILING_COMMENT", NULL, "%s:%d:%zu: a comment after code on the same line; fmt keeps full-line comments only",
                       source, line_no, line.dash - pos + 1u) + 1;
    switch (line.kind) {
      case LINE_CODE:
        for (; next_decl < list.count && list.items[next_decl].line == line_no; next_decl++) {
          if (blank) put(&o, "\n");
          blank = 0;
          decl(&o, &list, next_decl);
          span_end = last_line(&toks, &list, next_decl, &cursor);
        }
        break;
      case LINE_COMMENT:
        if (blank) put(&o, "\n");
        blank = 0;
        put_n(&o, text + line.dash, line.comment_len);
        put(&o, "\n");
        break;
      case LINE_BLANK:
        /* N-1: a blank line inside a declaration is not between items. */
        blank |= line_no > span_end && o.len > 0;
        break;
    }
    pos = line.next;
  }
  if (o.oom) return diag_fail(diag, "OOM", NULL, "%s: no memory for the canonical text", source) + 1;
  if (!check) return (o.len == 0 || fwrite(o.buf, 1, o.len, out) == o.len) && fflush(out) == 0 ? 0 : 1;
  if (o.len == len && (len == 0 || memcmp(o.buf, text, len) == 0)) return 0;
  return diag_fail(diag, "FMT_DIFF", NULL, "%s: the text is not the canonical text of `langc fmt`", source) + 1;
}
