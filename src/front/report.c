/* The lines of `langc report` (S11, T4). See report.h. The pattern is
   permit-lang src/report.c, rewritten for the ratify rule facts. */
#include <stdio.h>
#include <string.h>

#include "front/check.h"
#include "front/fix.h"
#include "front/report.h"
#include "front/tfhe.h"
#include "jsonout.h"

static const char *const RULE_TYPE[] = {"Rule", "Honest", "ForkFree"};

/* The facts of one policy: the decided hoBody (Honest, AUTH) and ffBody
   (ForkFree, AMEND). At a log height h >= 1, SET holds the SET_K counts of
   the approval set of the h-th amendment; NULL otherwise. */
typedef struct {
  RuleFacts auth;
  RuleFacts amend;
  uint32_t *set;
  uint32_t set_k;
} Height;

/* A closed Policy def (LOG 0, one height) or a closed Log def (LOG 1,
   LENGTH + 1 heights). */
typedef struct {
  const char *name;
  int log;
  uint32_t length;
  Height *at;
} Item;

/* Adds the integer V as a JSON number (Q-S11-2 (a): numbers are JSON
   integers). */
static int out_int(JsonOut *out, int64_t v) {
  char buf[24];
  int n = snprintf(buf, sizeof buf, "%lld", (long long)v);
  return n > 0 && (size_t)n < sizeof buf && out_put(out, buf, (size_t)n);
}

/* Adds ,"KEY": and the integer V. */
static int out_key_int(JsonOut *out, const char *key, int64_t v) {
  return out_text(out, ",\"") && out_text(out, key) && out_text(out, "\":") && out_int(out, v);
}

/* Adds ,"KEY": and true or false. */
static int out_key_bool(JsonOut *out, const char *key, int b) {
  return out_text(out, ",\"") && out_text(out, key) && out_text(out, "\":") && out_text(out, b ? "true" : "false");
}

/* Adds the K counts at V as a JSON array. */
static int out_counts(JsonOut *out, const uint32_t *v, uint32_t k) {
  uint32_t j;
  int ok = out_text(out, "[");
  for (j = 0; ok && j < k; j++)
    ok = out_comma(out, j) && out_int(out, (int64_t)v[j]);
  return ok && out_text(out, "]");
}

/* n, the sum of the classes of the record R. */
static int64_t class_sum(const RuleFacts *r) {
  int64_t n = 0;
  uint32_t i;
  for (i = 0; i < r->k; i++)
    n += (int64_t)r->n[i];
  return n;
}

/* The rule line of the record R. The margins are least - f (O4) and
   overlap - f (O5). The read set is n - least + f + 1, n the sum of the
   classes (ForkFree, Q-S11-5 (a), design section 10). */
static int out_rule(const RuleFacts *r, JsonOut *out) {
  uint32_t i;
  int ok = out_text(out, "{\"kind\":\"rule\",\"ratify\":1,\"name\":") && out_string(out, r->name) &&
           out_text(out, ",\"type\":") && out_string(out, RULE_TYPE[r->kind]) && out_text(out, ",\"classes\":") &&
           out_counts(out, r->n, r->k);
  if (ok && r->kind != 0)
    ok = out_key_int(out, "f", r->f) && out_key_int(out, "o4_margin", r->least - r->f);
  if (ok && r->kind == 2)
    ok = out_key_int(out, "o5_margin", r->overlap - r->f) &&
         out_key_int(out, "read_set", class_sum(r) - r->least + r->f + 1);
  ok = ok && out_key_int(out, "min_count", (int64_t)r->min_count) &&
       out_key_int(out, "min_listed", (int64_t)r->min_listed) && out_text(out, ",\"min\":[");
  for (i = 0; ok && i < r->min_listed; i++)
    ok = out_comma(out, i) && out_counts(out, r->min + (size_t)i * r->k, r->k);
  return ok && out_text(out, "]") && out_key_int(out, "accept_vectors", (int64_t)r->accept) &&
         out_key_int(out, "reject_vectors", (int64_t)(r->space - r->accept)) &&
         out_text(out, ",\"leak_t1\":\"counts\"") && out_key_bool(out, "accept_reveals_all", r->accept == 1u) &&
         out_key_bool(out, "reject_reveals_all", r->space - r->accept == 1u) && out_text(out, "}");
}

/* Adds the facts of the policy P: the classes, f, the O4 margin of the
   authorize rule, and the O4 and O5 margins and the read set of the amend
   rule (as out_rule). */
static int out_policy(JsonOut *out, const Height *p) {
  const RuleFacts *a = &p->amend;
  return out_text(out, ",\"classes\":") && out_counts(out, a->n, a->k) && out_key_int(out, "f", a->f) &&
         out_key_int(out, "auth_o4_margin", p->auth.least - p->auth.f) &&
         out_key_int(out, "amend_o4_margin", a->least - a->f) && out_key_int(out, "amend_o5_margin", a->overlap - a->f) &&
         out_key_int(out, "read_set", class_sum(a) - a->least + a->f + 1);
}

/* 1 when the type T is the family NAME (at any indices). */
static int family_is(const Machine *m, const Value *t, const char *name) {
  return t != NULL && val_is(t, OP_FAMILY) && t->inst < m->family_count && strcmp(m->families[t->inst].name, name) == 0;
}

/* 1 when V is the constructor NAME. */
static int ctor_is(const Machine *m, const Value *v, const char *name) {
  return v != NULL && val_is(v, OP_CTOR) && v->inst < m->ctor_count && m->ctors[v->inst].name != NULL &&
         strcmp(m->ctors[v->inst].name, name) == 0;
}

/* Field I of the constructor V with N fields (the last N arguments), or
   NULL. */
static const Value *field(const Value *v, uint32_t n, uint32_t i) {
  return v != NULL && val_is(v, OP_CTOR) && v->argc >= n && i < n ? v->args[v->argc - n + i] : NULL;
}

/* The number of SUCC layers of V down to ZERO (Size: szS, sz0; Pos:
   posNext, posFirst), or -1 when V is open. */
static int64_t layers(const Machine *m, const Value *v, const char *succ, const char *zero) {
  int64_t k = 0;
  while (ctor_is(m, v, succ) && v->argc > 0u) {
    k++;
    v = v->args[v->argc - 1u];
  }
  return ctor_is(m, v, zero) ? k : -1;
}

/* The facts of the closed policy P (plOf com f auth amend): hoBody at kind
   1 and ffBody at kind 2, each at com and f. 0 when P is open. */
static int policy_facts(Machine *m, const Value *p, Height *out) {
  const Value *auth = field(p, 4u, 2u);
  const Value *amend = field(p, 4u, 3u);
  int64_t f = layers(m, field(p, 4u, 1u), "szS", "sz0");
  out->set = NULL;
  out->set_k = 0;
  return ctor_is(m, p, "plOf") && f >= 0 && ctor_is(m, auth, "hoRule") && ctor_is(m, amend, "ffRule") &&
         check_rule_facts(m, field(p, 4u, 0u), field(auth, 3u, 2u), 1, f, &out->auth) &&
         check_rule_facts(m, field(p, 4u, 0u), field(amend, 3u, 2u), 2, f, &out->amend);
}

/* The K counts of the approval set S at the committee COM into OUT: the
   domain def count (domain.lang:310) at COM and S, a Counts value of K
   ctClass nodes, each count a Pos (posNext layers). */
static int set_counts(Machine *m, const Value *com, const Value *s, uint32_t k, uint32_t *out) {
  const Value *v = NULL;
  uint32_t i;
  int ok;
  for (i = 0; v == NULL && i < m->def_count; i++) {
    if (strcmp(m->defs[i].name, "count") == 0)
      v = def_value(m, i);
  }
  v = v != NULL ? apply_value(m, v, com) : NULL;
  v = v != NULL ? apply_value(m, v, s) : NULL;
  ok = v != NULL;
  for (i = 0; ok && i < k; i++) {
    int64_t c = ctor_is(m, v, "ctClass") ? layers(m, field(v, 4u, 2u), "posNext", "posFirst") : -1;
    ok = c >= 0;
    out[i] = ok ? (uint32_t)c : 0u;
    v = field(v, 4u, 3u);
  }
  return ok && ctor_is(m, v, "ctNil");
}

/* The heights of the closed Log value L into ITEM. The policy at height 0
   is the first policy; at h >= 1 it is the amTo of the h-th amendment
   (policyAtSize, domain.lang:427), with the counts of its approval set
   amSet at the committee of amFrom. 0 when L is open. */
static int log_heights(Machine *m, const Value *l, Item *item) {
  const Value *v;
  uint32_t h = 0;
  int ok;
  item->length = 0;
  for (v = l; ctor_is(m, v, "lgCons"); v = field(v, 5u, 4u))
    item->length++;
  if (!ctor_is(m, v, "lgNil"))
    return 0;
  item->at = arena_alloc(m->arena, ((size_t)item->length + 1u) * sizeof *item->at);
  ok = item->at != NULL && policy_facts(m, item->length == 0u ? field(l, 1u, 0u) : field(l, 5u, 0u), &item->at[0]);
  for (v = l; ok && ctor_is(m, v, "lgCons"); v = field(v, 5u, 4u)) {
    const Value *am = field(v, 5u, 3u);
    Height *at = &item->at[h + 1u];
    ok = ctor_is(m, am, "amOf") && policy_facts(m, field(am, 4u, 1u), at);
    at->set_k = item->at[h].auth.k;
    at->set = ok ? arena_alloc(m->arena, ((size_t)at->set_k + 1u) * sizeof *at->set) : NULL;
    ok = ok && at->set != NULL && set_counts(m, field(field(am, 4u, 0u), 4u, 0u), field(am, 4u, 2u), at->set_k, at->set);
    h++;
  }
  return ok;
}

/* The closed Policy and Log defs of the program into ITEMS in def order: a
   program def of type Policy or Log whose value walks to closed
   constructors with decided rules. Returns the item count. */
static uint32_t collect(Machine *m, Item *items) {
  uint32_t i;
  uint32_t count = 0;
  for (i = 0; i < m->def_count; i++) {
    const DefInfo *d = &m->defs[i];
    int policy = d->origin == ORIGIN_PROGRAM && family_is(m, d->type, "Policy");
    int log = d->origin == ORIGIN_PROGRAM && family_is(m, d->type, "Log");
    Item *item = &items[count];
    item->name = d->name;
    item->log = log;
    item->length = 0;
    item->at = policy ? arena_alloc(m->arena, sizeof *item->at) : NULL;
    if (policy && item->at != NULL && policy_facts(m, def_value(m, i), item->at))
      count++;
    if (log && log_heights(m, def_value(m, i), item))
      count++;
  }
  return count;
}

/* The policy line of the closed Policy def ITEM. */
static int out_policy_line(Machine *m, const Item *item, FILE *file) {
  JsonOut out = {m->arena, m->diag, item->name, NULL, 0, 0};
  return out_text(&out, "{\"kind\":\"policy\",\"ratify\":1,\"name\":") && out_string(&out, item->name) &&
         out_policy(&out, &item->at[0]) && out_text(&out, "}") && out_line(&out, file);
}

/* The log line of the closed Log def ITEM, then its height lines h = 0 to
   the length. h >= 1 adds the approvals of the h-th amendment and the
   decision, "accept" by amOk (domain.lang:402). */
static int out_log_lines(Machine *m, const Item *item, FILE *file) {
  JsonOut out = {m->arena, m->diag, item->name, NULL, 0, 0};
  uint32_t h;
  int ok = out_text(&out, "{\"kind\":\"log\",\"ratify\":1,\"name\":") && out_string(&out, item->name) &&
           out_key_int(&out, "length", (int64_t)item->length) && out_text(&out, "}") && out_line(&out, file);
  for (h = 0; ok && h <= item->length; h++) {
    JsonOut line = {m->arena, m->diag, item->name, NULL, 0, 0};
    const Height *at = &item->at[h];
    ok = out_text(&line, "{\"kind\":\"height\",\"ratify\":1,\"log\":") && out_string(&line, item->name) &&
         out_key_int(&line, "height", (int64_t)h) && out_policy(&line, at);
    if (ok && h > 0u)
      ok = out_text(&line, ",\"approvals\":") && out_counts(&line, at->set, at->set_k) &&
           out_text(&line, ",\"decision\":\"accept\"");
    ok = ok && out_text(&line, "}") && out_line(&line, file);
  }
  return ok;
}

int report_program(Machine *m, FILE *file) {
  JsonOut out = {m->arena, m->diag, "report", NULL, 0, 0};
  Item *items = arena_alloc(m->arena, ((size_t)m->def_count + 1u) * sizeof *items);
  uint32_t count = items != NULL ? collect(m, items) : 0u;
  uint32_t logs = 0;
  uint32_t i;
  int ok = items != NULL;
  for (i = 0; i < count; i++)
    logs += items[i].log ? 1u : 0u;
  ok = ok && out_text(&out, "{\"kind\":\"program\",\"ratify\":1") &&
       out_key_int(&out, "rules", (int64_t)m->rule_fact_count) && out_key_int(&out, "policies", (int64_t)(count - logs)) &&
       out_key_int(&out, "logs", (int64_t)logs) && out_text(&out, "}") && out_line(&out, file);
  for (i = 0; ok && i < m->rule_fact_count; i++) {
    JsonOut rule = {m->arena, m->diag, m->rule_facts[i].name, NULL, 0, 0};
    ok = out_rule(&m->rule_facts[i], &rule) && out_line(&rule, file);
  }
  for (i = 0; ok && i < count; i++)
    ok = items[i].log || out_policy_line(m, &items[i], file);
  for (i = 0; ok && i < count; i++)
    ok = !items[i].log || out_log_lines(m, &items[i], file);
  return ok;
}

/* The circuit line of the record R and its circuit C (S12, Q-S12-5 (a)):
   the classes, the inputs, min_count and all MIN_KEPT minimal vectors (the
   T2 list, not the first 64 of the rule line), the gate rows as [op, a, b],
   the output wire, the gate and NOT counts, D and B. */
static int out_circuit(const RuleFacts *r, const TfheCircuit *c, JsonOut *out) {
  uint32_t i;
  int ok = out_text(out, "{\"kind\":\"circuit\",\"ratify\":1,\"name\":") && out_string(out, r->name) &&
           out_text(out, ",\"type\":") && out_string(out, RULE_TYPE[r->kind]) && out_text(out, ",\"classes\":") &&
           out_counts(out, r->n, r->k) && out_key_int(out, "inputs", (int64_t)c->inputs) &&
           out_key_int(out, "min_count", (int64_t)r->min_count) && out_text(out, ",\"min\":[");
  for (i = 0; ok && i < r->min_kept; i++)
    ok = out_comma(out, i) && out_counts(out, r->min + (size_t)i * r->k, r->k);
  ok = ok && out_text(out, "],\"gates\":[");
  for (i = 0; ok && i < c->gate_count; i++)
    ok = out_comma(out, i) && out_text(out, "[") && out_string(out, tfhe_op_name(c->gates[i].op)) &&
         out_text(out, ",") && out_int(out, (int64_t)c->gates[i].a) && out_text(out, ",") &&
         out_int(out, (int64_t)c->gates[i].b) && out_text(out, "]");
  return ok && out_text(out, "]") && out_key_int(out, "out", (int64_t)c->out) &&
         out_key_int(out, "gate_count", (int64_t)c->gate_count) && out_key_int(out, "nots", (int64_t)c->nots) &&
         out_key_int(out, "depth", (int64_t)c->depth) && out_key_int(out, "bound", (int64_t)c->bound) &&
         out_text(out, "}");
}

/* Builds the circuit of the record R into C. The caps refuse with
   REFUSE_DEPTH (Q-S12-4 (a)); the faults are INTERNAL. */
static int tfhe_circuit(Machine *m, const RuleFacts *r, TfheCircuit *c) {
  switch (tfhe_build(m->arena, r, c)) {
    case TFHE_OK:
      return 1;
    case TFHE_REFUSE_GATES:
      return diag_fail(m->diag, "REFUSE_DEPTH", r->name, "the circuit needs more than %u gates", TFHE_GATE_MAX);
    case TFHE_REFUSE_MIN:
      return diag_fail(m->diag, "REFUSE_DEPTH", r->name, "the rule has %llu minimal vectors, more than %u",
                       (unsigned long long)r->min_count, TFHE_MIN_CAP);
    case TFHE_FAULT_DEPTH:
      return diag_fail(m->diag, "INTERNAL", r->name, "the circuit depth D is above its bound B");
    case TFHE_FAULT_SHAPE:
      return diag_fail(m->diag, "INTERNAL", r->name, "the circuit output is a constant, or the minimal list is short");
    case TFHE_OOM:
      return diag_fail(m->diag, "OOM", r->name, "the circuit does not fit");
  }
  return 0;
}

int report_tfhe(Machine *m, FILE *file) {
  JsonOut out = {m->arena, m->diag, "tfhe", NULL, 0, 0};
  TfheCircuit *c = arena_alloc(m->arena, ((size_t)m->rule_fact_count + 1u) * sizeof *c);
  uint32_t i;
  int ok = c != NULL || diag_fail(m->diag, "OOM", NULL, "the circuit table does not fit");
  for (i = 0; ok && i < m->rule_fact_count; i++)
    ok = tfhe_circuit(m, &m->rule_facts[i], &c[i]);
  ok = ok && out_text(&out, "{\"kind\":\"program\",\"ratify\":1") &&
       out_key_int(&out, "circuits", (int64_t)m->rule_fact_count) && out_key_int(&out, "gate_max", TFHE_GATE_MAX) &&
       out_key_int(&out, "min_cap", TFHE_MIN_CAP) && out_text(&out, "}") && out_line(&out, file);
  for (i = 0; ok && i < m->rule_fact_count; i++) {
    JsonOut line = {m->arena, m->diag, m->rule_facts[i].name, NULL, 0, 0};
    ok = out_circuit(&m->rule_facts[i], &c[i], &line) && out_line(&line, file);
  }
  return ok;
}

/* The sec 6 law table of `langc check --json` (S15, Q-S15-5 (a)). A row
   applies when the program has its trigger: R = a decided rule def, H = an
   Honest or ForkFree rule def, F = a ForkFree rule def, L = a closed Log
   def. An ASSUMED row is never counted as discharged. docs/CHECK-JSON.md
   has the same table. */
typedef enum { LAW_DISCHARGED, LAW_ASSUMED, LAW_AT_TARGET, LAW_EXTERNAL, LAW_KINDS } LawKind;
typedef enum { ON_R, ON_H, ON_F, ON_L, LAW_TRIGGERS } LawTrigger;

typedef struct {
  const char *name;
  LawKind kind;
  LawTrigger on;
  const char *statement; /* S16 U1: the Statement column of docs/PLAN.md section 6 */
} Law;

static const Law LAWS[] = {
    {"TCM-A", LAW_DISCHARGED, ON_R, "authorize is invariant under G_P"},
    {"TCM-B", LAW_DISCHARGED, ON_R, "and, or are pointwise"},
    {"O2", LAW_DISCHARGED, ON_R, "each decision is binary"},
    {"O3", LAW_DISCHARGED, ON_R, "monotone, accept(0) = false, accept(n) = true"},
    {"O4", LAW_DISCHARGED, ON_H, "each accepting set holds an honest party"},
    {"O5", LAW_DISCHARGED, ON_F, "fork freedom of the amend rule"},
    {"TCM-E", LAW_DISCHARGED, ON_L, "nil is the unit, append is associative"},
    {"O10", LAW_DISCHARGED, ON_L, "causality of the log"},
    {"TCM-C", LAW_ASSUMED, ON_R, "dec(eval g cs) = g(dec cs)"},
    {"TCM-D", LAW_ASSUMED, ON_R, "composition of protocols, up to indistinguishability"},
    {"View 1", LAW_ASSUMED, ON_R, "the three view equations of design sec 5"},
    {"View 2", LAW_ASSUMED, ON_R, "the three view equations of design sec 5"},
    {"View 3", LAW_ASSUMED, ON_R, "the three view equations of design sec 5"},
    {"O7", LAW_ASSUMED, ON_R, "threshold decryption matches the ideal"},
    {"O8", LAW_ASSUMED, ON_R, "ZK for R_ballot and R_dec"},
    {"O9", LAW_ASSUMED, ON_L, "resharing on a committee change"},
    {"O6", LAW_AT_TARGET, ON_R, "FHE exactness for accept . count"},
    {"O1", LAW_EXTERNAL, ON_R, "each rule is orbit-constant under G_P"},
    {"O11", LAW_EXTERNAL, ON_L, "at most one entry per height"},
};

/* The triggers of M into SEEN: its rule facts (KIND 0 Rule, 1 Honest, 2
   ForkFree) and the closed Log defs that collect finds. On a refusal, M
   holds the definitions checked before the failure. Returns 1, or 0 after
   a diagnostic on OWN. */
static int law_triggers(Machine *m, Diag *own, int *seen) {
  Item *items = arena_alloc(m->arena, ((size_t)m->def_count + 1u) * sizeof *items);
  uint32_t count = items != NULL ? collect(m, items) : 0u;
  uint32_t i;
  for (i = 0; i < m->rule_fact_count; i++) {
    seen[ON_R] = 1;
    seen[ON_H] = seen[ON_H] || m->rule_facts[i].kind != 0;
    seen[ON_F] = seen[ON_F] || m->rule_facts[i].kind == 2;
  }
  for (i = 0; i < count; i++)
    seen[ON_L] = seen[ON_L] || items[i].log;
  return items != NULL ? 1 : diag_fail(own, "OOM", "check", "out of memory");
}

/* Adds ,"KEY": and TEXT as a JSON string, or null when TEXT is NULL or
   empty. */
static int out_key_text(JsonOut *out, const char *key, const char *text) {
  return out_text(out, ",\"") && out_text(out, key) && out_text(out, "\":") &&
         out_string(out, text != NULL && text[0] != '\0' ? text : NULL);
}

/* The diagnostic object of DIAG (Q-S15-1 (a), Q-S15-2 (a)): span = the
   start FILE:LINE:COL, or null when DIAG has no span (LINE 0). */
static int out_diagnostic(JsonOut *out, const Diag *diag) {
  int span = diag->line != 0 && diag->file != NULL;
  int ok = out_text(out, "{\"code\":") && out_string(out, diag->code) &&
           out_key_text(out, "expected", diag->expected) && out_key_text(out, "fix", fix_of(diag->code)) &&
           out_key_text(out, "found", diag->found) && out_text(out, ",\"message\":") && out_string(out, diag->msg) &&
           out_text(out, ",\"span\":");
  return ok &&
         (span ? out_text(out, "{\"col\":") && out_int(out, diag->col) && out_text(out, ",\"file\":") &&
                     out_string(out, diag->file) && out_key_int(out, "line", diag->line) && out_text(out, "}")
               : out_text(out, "null")) &&
         out_text(out, "}");
}

int report_check(Arena *arena, Machine *m, const Diag *diag, uint32_t parsed, Diag *own, FILE *file) {
  JsonOut out = {arena, own, "check", NULL, 0, 0};
  int seen[LAW_TRIGGERS] = {0, 0, 0, 0};
  int64_t counts[LAW_KINDS] = {0, 0, 0, 0};
  size_t i;
  int ok = m == NULL || law_triggers(m, own, seen);
  for (i = 0; i < sizeof LAWS / sizeof LAWS[0]; i++)
    counts[LAWS[i].kind] += seen[LAWS[i].on] ? 1 : 0;
  return ok && out_text(&out, "{\"diagnostics\":[") && (!diag->set || out_diagnostic(&out, diag)) &&
         out_text(&out, "],\"progress\":{\"laws_assumed\":") && out_int(&out, counts[LAW_ASSUMED]) &&
         out_key_int(&out, "laws_at_target", counts[LAW_AT_TARGET]) &&
         out_key_int(&out, "laws_discharged", diag->set ? 0 : counts[LAW_DISCHARGED]) &&
         out_key_int(&out, "laws_external", counts[LAW_EXTERNAL]) &&
         out_key_int(&out, "laws_total", counts[LAW_DISCHARGED]) && out_key_int(&out, "parsed", (int64_t)parsed) &&
         out_key_int(&out, "typed", m != NULL ? (int64_t)m->typed : 0) && out_text(&out, "}}") && out_line(&out, file);
}

/* S16 U1: `langc show-law [ROW]` (src/main.c). One line `NAME: STATEMENT
   (KIND, trigger T)` for the row with the name ROW, or for each row when ROW
   is NULL. Returns the number of lines. */
int report_laws(const char *row, FILE *out) {
  static const char *const kind_words[LAW_KINDS] = {"DISCHARGED", "ASSUMED", "AT TARGET", "EXTERNAL"};
  static const char *const trigger_words[LAW_TRIGGERS] = {"R", "H", "F", "L"};
  size_t i;
  int count = 0;
  for (i = 0; i < sizeof LAWS / sizeof LAWS[0]; i++) {
    int shown = row == NULL || strcmp(LAWS[i].name, row) == 0;
    if (shown)
      fprintf(out, "%s: %s (%s, trigger %s)\n", LAWS[i].name, LAWS[i].statement, kind_words[LAWS[i].kind],
              trigger_words[LAWS[i].on]);
    count += shown;
  }
  return count;
}
