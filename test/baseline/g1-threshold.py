"""G1: a plain t-of-n policy (g1-threshold.lang).

One class of 4 parties and the bar t = 3 of the authorize rule. A rule
accepts when the count of approvals in each class is at its bar or
more. 4 approvals give yes and 0 approvals give no.
"""
from collections import namedtuple

Policy = namedtuple("Policy", "auth")


def rl_min(bars):
    return lambda counts: all(b <= c for b, c in zip(bars, counts))


def count(approvals):
    return [sum(cls) for cls in approvals]


def authorize(p, approvals):
    return p.auth(count(approvals))


def show_decision(d):
    return "inr unit" if d else "inl unit"


p0 = Policy(rl_min([3]))
s4 = [[True] * 4]
s0 = [[False] * 4]
print(show_decision(authorize(p0, s4)))
print(show_decision(authorize(p0, s0)))
