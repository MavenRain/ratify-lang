//! G1: a plain t-of-n policy (g1-threshold.lang).
//!
//! One class of 4 parties and the bar t = 3 of the authorize rule. A
//! rule accepts when the count of approvals in each class is at its bar
//! or more. 4 approvals give yes and 0 approvals give no.
struct Rule(Vec<usize>);

impl Rule {
    fn accept(&self, counts: &[usize]) -> bool {
        self.0.iter().zip(counts).all(|(b, c)| b <= c)
    }
}

struct Policy {
    auth: Rule,
}

fn count(approvals: &[Vec<bool>]) -> Vec<usize> {
    approvals.iter().map(|cls| cls.iter().filter(|a| **a).count()).collect()
}

fn authorize(p: &Policy, approvals: &[Vec<bool>]) -> bool {
    p.auth.accept(&count(approvals))
}

fn show_decision(d: bool) -> &'static str {
    if d { "inr unit" } else { "inl unit" }
}

fn main() {
    let p0 = Policy { auth: Rule(vec![3]) };
    let s4 = vec![vec![true; 4]];
    let s0 = vec![vec![false; 4]];
    println!("{}", show_decision(authorize(&p0, &s4)));
    println!("{}", show_decision(authorize(&p0, &s0)));
}
