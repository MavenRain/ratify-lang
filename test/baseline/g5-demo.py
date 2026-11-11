"""G5: the S10 demo, the verdict of a certificate (g5-demo.lang).

Two classes: 3 operators, and 1 auditor in a class of its own (a veto).
The log p_base -> p_amd1 -> p_add has 2 amendments; p_add has 4
operators. demo_v decodes the flag word W of the 13 host checks (bit k
is check k) and runs verify on a certificate of p_add. The first check
that fails gives the reason. The code of valid is 0, else 1 + the index
of the reason: 1 malformed, 2 ill typed policy, 3 wrong height, 4 bad
prev hash, 5 bad signature, 6 bad proof, 7 not accepted, 8
equivocation.
"""
from collections import namedtuple

Policy = namedtuple("Policy", "auth amend")
Cert = namedtuple("Cert", "height signers amend wf wt prev sig proof")
Evidence = namedtuple("Evidence", "party height hash_a hash_b sig_a sig_b")


def rl_min(bars):
    return lambda counts: all(b <= c for b, c in zip(bars, counts))


def count(approvals):
    return [sum(cls) for cls in approvals]


def replay(p, steps):
    """The policies of the log from p; None when a step fails."""
    log = [p]
    for new, approvals in steps:
        if not log[-1].amend(count(approvals)):
            return None
        log.append(new)
    return log


def equivocates(e):
    return e.sig_a and e.sig_b and e.hash_a != e.hash_b


def hits(signers, height, e):
    cls, pos = e.party
    return equivocates(e) and e.height == height and signers[cls][pos]


def verify(log, c, evs):
    q = log[-1]
    rule = q.amend if c.amend else q.auth
    checks = (c.wf, c.wt, c.height == len(log) - 1, c.prev, c.sig, c.proof,
              rule(count(c.signers)),
              not any(hits(c.signers, c.height, e) for e in evs))
    return next((i + 1 for i, ok in enumerate(checks) if not ok), 0)


def bit(w, k):
    return (w >> k) & 1 == 1


def demo_v(log, w, height, action, amend, target, signers):
    a = [bit(signers, i) for i in range(5)]
    h = min(height, 65)
    sig = all(bit(w, i) for i, ai in enumerate(a) if ai)
    cert = Cert(h, [a[:4], a[4:]], 1 <= amend, amend <= 1, target <= 3,
                bit(w, 5), sig, bit(w, 6))
    ev = Evidence((0, 0), h, 0, 0 if bit(w, 7) else 1,
                  bit(w, 8) and bit(w, 9) and bit(w, 10),
                  bit(w, 11) and bit(w, 12))
    return verify(log, cert, [ev])


p_base = Policy(rl_min([2, 1]), rl_min([3, 1]))
p_amd1 = Policy(rl_min([3, 1]), rl_min([3, 1]))
p_add = Policy(rl_min([2, 1]), rl_min([3, 1]))
all_dc = [[True] * 3, [True]]
l_demo = replay(p_base, [(p_amd1, all_dc), (p_add, all_dc)])
print(demo_v(l_demo, 8191, 2, 7, 0, 0, 31))
print(demo_v(l_demo, 8190, 2, 7, 0, 0, 31))
print(demo_v(l_demo, 8063, 2, 7, 0, 0, 31))
