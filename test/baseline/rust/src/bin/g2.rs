//! G2: two classes and a veto (g2-veto.lang).
//!
//! The committee has a class of 3 parties and a veto class of 1 party.
//! The rule is an and of 2 bars: 2 of the 3 parties, and the veto
//! party. The counts (2, 0) are not accepted; the counts (2, 1) are
//! accepted.
enum Rule {
    Min(Vec<usize>),
    And(Box<Rule>, Box<Rule>),
}

impl Rule {
    fn accept(&self, counts: &[usize]) -> bool {
        match self {
            Self::Min(bars) => bars.iter().zip(counts).all(|(b, c)| b <= c),
            Self::And(x, y) => x.accept(counts) && y.accept(counts),
        }
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
    let need2 = vec![2, 0];
    let need_veto = vec![0, 1];
    let both = vec![2, 1];
    let veto_rule = Rule::And(Box::new(Rule::Min(need2.clone())), Box::new(Rule::Min(need_veto)));
    println!("{}", show_decision(veto_rule.accept(&need2)));
    println!("{}", show_decision(veto_rule.accept(&both)));
    let p_veto = Policy { auth: veto_rule };
    let s_all = vec![vec![true; 3], vec![true]];
    let s_no_veto = vec![vec![true; 3], vec![false]];
    println!("{}", show_decision(authorize(&p_veto, &s_all)));
    println!("{}", show_decision(authorize(&p_veto, &s_no_veto)));
}
