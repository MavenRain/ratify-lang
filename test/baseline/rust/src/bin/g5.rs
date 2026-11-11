//! G5: the S10 demo, the verdict of a certificate (g5-demo.lang).
//!
//! Two classes: 3 operators, and 1 auditor in a class of its own (a
//! veto). The log p_base -> p_amd1 -> p_add has 2 amendments; p_add has
//! 4 operators. demo_v decodes the flag word W of the 13 host checks
//! (bit k is check k) and runs verify on a certificate of p_add. The
//! first check that fails gives the reason. The code of valid is 0,
//! else 1 + the index of the reason: 1 malformed, 2 ill typed policy, 3
//! wrong height, 4 bad prev hash, 5 bad signature, 6 bad proof, 7 not
//! accepted, 8 equivocation.
type Approvals = Vec<Vec<bool>>;

struct Rule(Vec<usize>);

impl Rule {
    fn accept(&self, counts: &[usize]) -> bool {
        self.0.iter().zip(counts).all(|(b, c)| b <= c)
    }
}

struct Policy {
    auth: Rule,
    amend: Rule,
}

struct Cert {
    height: usize,
    signers: Approvals,
    amend: bool,
    wf: bool,
    wt: bool,
    prev: bool,
    sig: bool,
    proof: bool,
}

struct Evidence {
    party: (usize, usize),
    height: usize,
    hash_a: u64,
    hash_b: u64,
    sig_a: bool,
    sig_b: bool,
}

fn count(approvals: &[Vec<bool>]) -> Vec<usize> {
    approvals.iter().map(|cls| cls.iter().filter(|a| **a).count()).collect()
}

/// The policies of the log from p0; none when a step fails.
fn replay(p0: Policy, steps: Vec<(Policy, Approvals)>) -> Option<Vec<Policy>> {
    steps.into_iter().try_fold(vec![p0], |log, (next, s)| {
        let ok = log.last().is_some_and(|p| p.amend.accept(&count(&s)));
        ok.then(|| log.into_iter().chain(std::iter::once(next)).collect::<Vec<_>>())
    })
}

fn equivocates(e: &Evidence) -> bool {
    e.sig_a && e.sig_b && e.hash_a != e.hash_b
}

fn hits(signers: &[Vec<bool>], height: usize, e: &Evidence) -> bool {
    let (cls, pos) = e.party;
    let signed = signers.get(cls).and_then(|c| c.get(pos)).is_some_and(|a| *a);
    equivocates(e) && e.height == height && signed
}

fn rule_of(q: &Policy, amend: bool) -> &Rule {
    if amend { &q.amend } else { &q.auth }
}

fn verify(log: &[Policy], c: &Cert, evs: &[Evidence]) -> usize {
    let accepted = log.last().is_some_and(|q| rule_of(q, c.amend).accept(&count(&c.signers)));
    let caught = evs.iter().any(|e| hits(&c.signers, c.height, e));
    let checks =
        [c.wf, c.wt, c.height + 1 == log.len(), c.prev, c.sig, c.proof, accepted, !caught];
    checks.iter().position(|ok| !ok).map_or(0, |i| i + 1)
}

fn bit(w: u64, k: u32) -> bool {
    (w >> k) & 1 == 1
}

fn demo_v(log: &[Policy], w: u64, height: usize, _action: u64, amend: u64, target: u64,
          signers: u64) -> usize {
    let a: Vec<bool> = (0..5).map(|i| bit(signers, i)).collect();
    let h = height.min(65);
    let sig = a.iter().zip(0..).all(|(ai, i)| !ai || bit(w, i));
    let cert = Cert {
        height: h,
        signers: vec![a.iter().take(4).copied().collect(), a.iter().skip(4).copied().collect()],
        amend: 1 <= amend,
        wf: amend <= 1,
        wt: target <= 3,
        prev: bit(w, 5),
        sig,
        proof: bit(w, 6),
    };
    let ev = Evidence {
        party: (0, 0),
        height: h,
        hash_a: 0,
        hash_b: if bit(w, 7) { 0 } else { 1 },
        sig_a: bit(w, 8) && bit(w, 9) && bit(w, 10),
        sig_b: bit(w, 11) && bit(w, 12),
    };
    verify(log, &cert, &[ev])
}

fn main() {
    let p_base = Policy { auth: Rule(vec![2, 1]), amend: Rule(vec![3, 1]) };
    let p_amd1 = Policy { auth: Rule(vec![3, 1]), amend: Rule(vec![3, 1]) };
    let p_add = Policy { auth: Rule(vec![2, 1]), amend: Rule(vec![3, 1]) };
    let all_dc = vec![vec![true; 3], vec![true]];
    let l_demo = replay(p_base, vec![(p_amd1, all_dc.clone()), (p_add, all_dc)]);
    l_demo.iter().for_each(|log| {
        println!("{}", demo_v(log, 8191, 2, 7, 0, 0, 31));
        println!("{}", demo_v(log, 8190, 2, 7, 0, 0, 31));
        println!("{}", demo_v(log, 8063, 2, 7, 0, 0, 31));
    });
}
