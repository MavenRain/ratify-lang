//! G4: a log replay with policy_at (g4-replay.lang).
//!
//! The log has 2 amendments, p0 -> p1 -> p2, so it has the heights 0,
//! 1 and 2. Each step must pass the amend rule of the policy that it
//! replaces. Past the end of the log the policy stays p2. 3 approvals
//! pass the amend rule at height 0 (p0) and do not pass it at height 1
//! (p1).
type Approvals = Vec<Vec<bool>>;

struct Rule(Vec<usize>);

impl Rule {
    fn accept(&self, counts: &[usize]) -> bool {
        self.0.iter().zip(counts).all(|(b, c)| b <= c)
    }
}

struct Policy {
    amend: Rule,
}

fn count(approvals: &[Vec<bool>]) -> Vec<usize> {
    approvals.iter().map(|cls| cls.iter().filter(|a| **a).count()).collect()
}

fn amend_ok(p: &Policy, approvals: &[Vec<bool>]) -> bool {
    p.amend.accept(&count(approvals))
}

/// The policies of the log from p0; none when a step fails.
fn replay(p0: Policy, steps: Vec<(Policy, Approvals)>) -> Option<Vec<Policy>> {
    steps.into_iter().try_fold(vec![p0], |log, (next, s)| {
        let ok = log.last().is_some_and(|p| amend_ok(p, &s));
        ok.then(|| log.into_iter().chain(std::iter::once(next)).collect::<Vec<_>>())
    })
}

fn policy_at(log: &[Policy], h: usize) -> Option<&Policy> {
    log.get(h.min(log.len().saturating_sub(1)))
}

fn show_decision(d: bool) -> &'static str {
    if d { "inr unit" } else { "inl unit" }
}

fn main() {
    let p0 = Policy { amend: Rule(vec![3]) };
    let p1 = Policy { amend: Rule(vec![4]) };
    let p2 = Policy { amend: Rule(vec![2, 2]) };
    let s3 = vec![vec![false, true, true, true]];
    let s4 = vec![vec![true; 4]];
    let log012 = replay(p0, vec![(p1, s3.clone()), (p2, s4)]);
    let amend_at = |h: usize| {
        log012.as_deref().and_then(|log| policy_at(log, h)).is_some_and(|p| amend_ok(p, &s3))
    };
    println!("{}", show_decision(amend_at(0)));
    println!("{}", show_decision(amend_at(1)));
}
