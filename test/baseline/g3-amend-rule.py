"""G3: an amendment that changes the amend rule (g3-amend-rule.lang).

The policy p0 has the amend bar t = 3 of 4. The policy p1 has the amend
bar t = 4 of 4. amend_ok gives the decision of the amend rule: 3
approvals pass the amend rule of p0 and do not pass that of p1.
"""
from collections import namedtuple

Policy = namedtuple("Policy", "amend")


def rl_min(bars):
    return lambda counts: all(b <= c for b, c in zip(bars, counts))


def count(approvals):
    return [sum(cls) for cls in approvals]


def amend_ok(p, approvals):
    return p.amend(count(approvals))


def show_decision(d):
    return "inr unit" if d else "inl unit"


p0 = Policy(rl_min([3]))
p1 = Policy(rl_min([4]))
s3 = [[False, True, True, True]]
s4 = [[True] * 4]
print(show_decision(amend_ok(p0, s3)))
print(show_decision(amend_ok(p1, s3)))
print(show_decision(amend_ok(p1, s4)))
