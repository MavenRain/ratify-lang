//! G3: an amendment that changes the amend rule (g3-amend-rule.lang).
//!
//! The policy p0 has the amend bar t = 3 of 4. The policy p1 has the
//! amend bar t = 4 of 4. amend_ok gives the decision of the amend rule:
//! 3 approvals pass the amend rule of p0 and do not pass that of p1.
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

fn show_decision(d: bool) -> &'static str {
    if d { "inr unit" } else { "inl unit" }
}

fn main() {
    let p0 = Policy { amend: Rule(vec![3]) };
    let p1 = Policy { amend: Rule(vec![4]) };
    let s3 = vec![vec![false, true, true, true]];
    let s4 = vec![vec![true; 4]];
    println!("{}", show_decision(amend_ok(&p0, &s3)));
    println!("{}", show_decision(amend_ok(&p1, &s3)));
    println!("{}", show_decision(amend_ok(&p1, &s4)));
}
