"""G2: two classes and a veto (g2-veto.lang).

The committee has a class of 3 parties and a veto class of 1 party. The
rule is an and of 2 bars: 2 of the 3 parties, and the veto party. The
counts (2, 0) are not accepted; the counts (2, 1) are accepted.
"""
from collections import namedtuple

Policy = namedtuple("Policy", "auth")


def rl_min(bars):
    return lambda counts: all(b <= c for b, c in zip(bars, counts))


def rl_and(x, y):
    return lambda counts: x(counts) and y(counts)


def count(approvals):
    return [sum(cls) for cls in approvals]


def authorize(p, approvals):
    return p.auth(count(approvals))


def show_decision(d):
    return "inr unit" if d else "inl unit"


need2 = [2, 0]
need_veto = [0, 1]
both = [2, 1]
veto_rule = rl_and(rl_min(need2), rl_min(need_veto))
p_veto = Policy(veto_rule)
s_all = [[True] * 3, [True]]
s_no_veto = [[True] * 3, [False]]
print(show_decision(veto_rule(need2)))
print(show_decision(veto_rule(both)))
print(show_decision(authorize(p_veto, s_all)))
print(show_decision(authorize(p_veto, s_no_veto)))
