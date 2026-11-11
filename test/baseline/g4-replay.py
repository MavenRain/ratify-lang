"""G4: a log replay with policy_at (g4-replay.lang).

The log has 2 amendments, p0 -> p1 -> p2, so it has the heights 0, 1
and 2. Each step must pass the amend rule of the policy that it
replaces. Past the end of the log the policy stays p2. 3 approvals pass
the amend rule at height 0 (p0) and do not pass it at height 1 (p1).
"""
from collections import namedtuple

Policy = namedtuple("Policy", "amend")


def rl_min(bars):
    return lambda counts: all(b <= c for b, c in zip(bars, counts))


def count(approvals):
    return [sum(cls) for cls in approvals]


def amend_ok(p, approvals):
    return p.amend(count(approvals))


def replay(p, steps):
    """The policies of the log from p; None when a step fails."""
    log = [p]
    for new, approvals in steps:
        if not amend_ok(log[-1], approvals):
            return None
        log.append(new)
    return log


def policy_at(log, h):
    return log[min(h, len(log) - 1)]


def show_decision(d):
    return "inr unit" if d else "inl unit"


p0 = Policy(rl_min([3]))
p1 = Policy(rl_min([4]))
p2 = Policy(rl_min([2, 2]))
s3 = [[False, True, True, True]]
s4 = [[True] * 4]
log012 = replay(p0, [(p1, s3), (p2, s4)])
print(show_decision(amend_ok(policy_at(log012, 0), s3)))
print(show_decision(amend_ok(policy_at(log012, 1), s3)))
