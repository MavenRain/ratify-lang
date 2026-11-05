# Domain slot

In S4 the domain is the kernel prelude, ported from permit-lang 3d47120,
the IR test families of permit-lang 18deea2, and the committee words of
ratify-lang S4.
`domain/domain.lang` holds the domain. At build time, `tcc -run
gen/embed.c` copies it into `build/domain.c`, so the domain is part of
`build/langc`. `langc` loads the domain first, then the program.

## What the file holds

- `family` declarations. Only this file can declare a family. In a
  program, `family` is refused with `REFUSE_DATA` (rule R1).
- `def` operations, written in the language.
- Line comments that start with `--`.

Each family, constructor, field and definition name in this file becomes
a core name. A program cannot use one of them as a new definition name
(rule R4, `REFUSE_NAME`).

## Family syntax

```
family NAME (param : T)* := ctor (field : T)* | ctor (field : T)* ...
```

- A family with only nullary constructors is an enum.
- A family with one constructor is a record. It gets one projection for
  each field name.
- A field can have the type of its own family. Such a field is recursive.
- A parameter is an index (former F15). A constructor takes the indices
  first.
- A `fold` over a family takes one argument for each constructor, in
  declaration order. A constant constructor takes a value. A constructor
  with fields takes a function. A recursive field arrives folded.

In the IR, a constructor value is a heap block. Word 0 is the constructor
index, in declaration order. Each op is total; a value is one word.
Field K is word 1 + K.
A `fold` over a family with one recursive field lowers to a loop.
A family with two or more recursive fields cannot be folded at run time
(`REFUSE_RUNTIME_TREE`, see `docs/CAPABILITY.md`).

## The prelude

The prelude comes from permit-lang 3d47120 (S2). The kit
sample (`Color`, `Item`, `Stack`, `Tree`, `Lot`) is not in the domain: it
uses `natMul`, and the kernel has no `natMul`.

| Kind | Names |
|---|---|
| Families | `S1`, `Trunc1`, `Trunc0`, `Size`, `Parity`, `Empty`, `Bag`, `FreeMon`, `Pos`, `Word`, `ChoiceBag`, `OrbitN`, `Charter` |
| Path operations | `transport`, `subst`, `J`, `JRefl`, `trans`, `isProp`, `isSet` |
| Size and word operations | `sExtend`, `sDuplicate`, `sizeFlip`, `sizeEven`, `sizePred`, `OpPos`, `consP`, `swapFront`, `actW` |
| Free monoid operations | `fmCons`, `fmIsSet`, `fmFold`, `fmBind`, `fmMap` |
| Equivalences | `isContr`, `fiber`, `isEquiv`, `Equiv`, `idEquiv`, `isoCap`, `isoPath`, `isoToEquiv`, `ua`, `uaArrow` |

Many kernel rows in `examples/`, `test/check/` and `test/eval/expect.txt`
use these names.

## The committee words

S4 adds the committee words of `docs/PLAN.md` section 4.1 (Q-S4-1 (a)).
Each name in this file is a core name, so the fields of each family have
a prefix.

| Name | Kind | Prefix | Meaning |
|---|---|---|---|
| `Committee` | family | `cm` | `cmNil`, or `cmClass` with the class size `cmSize : Size` and the rest `cmRest` |
| `Party` | family on `Committee` | `pt` | `ptHere`, a `Pos n` in the first class, or `ptThere`, a party of the rest |
| `Approvals` | family on `Committee` | `ap` | `apNil`, or `apClass` with `apBits : Pos n -> Sum Unit Unit` for the class; `inr unit` is an approval, `inl unit` is a missing ballot |
| `Counts` | family on `Committee` | `ct` | `ctNil`, or `ctClass` with `ctValue : Pos (szS n)`, so 0 <= c_j <= n_j by type |
| `posWeaken` | definition | | `(n : Size) -> Pos n -> Pos (szS n)` |
| `countBits` | definition | | the count of one class, an elim on `Size` over the bit map |
| `count` | definition | | `(c : Committee) -> Approvals c -> Counts c` |
| `Perm` | definition | | G_P: a large elim on `Committee`, one `Word` of adjacent swaps for each class |
| `actP` | definition | | `(c : Committee) -> Perm c -> Approvals c -> Approvals c`, `actW` on each class |

A program definition that is not a law and reads `Approvals` (a fold, an
elim or a projection) gives `REFUSE_ORBIT`. A program uses `count` and
`actP` instead. A definition type that names `Party`, `Approvals` or
`Counts` at a closed committee whose count space is above 2^20 gives
`REFUSE_BOUND`. See `examples/committee.lang` and
`examples/bound-edge.lang`.

## The rule words

S5 adds the rule words of `docs/PLAN.md` section 4.1 (Q-S5-1 (a), Q-S5-2
(a)). The fields of each family have a prefix (rule R4).

| Name | Kind | Prefix | Meaning |
|---|---|---|---|
| `CtTable` | family on `Committee` | `tb` | `tbNil`, or `tbCons` with a count vector `tbRow` and the rest `tbRest` |
| `Rule` | family on `Committee` | `rl` | `rlMin` (accept when the count is at or above the bar `rlBar` in each class), `rlAnd`, `rlOr`, or `rlTable` (accept exactly the vectors in `rlRows`) |
| `accept` | definition | | `(c : Committee) -> Rule c -> Counts c -> Sum Unit Unit`, an elim on `Rule`; `inr unit` accepts |
| `Honest` | family on `Committee` and `Size` | `ho` | `hoRule`, with the fault bound `hoF` and the rule `hoBody` |
| `ForkFree` | family on `Committee` and `Size` | `ff` | `ffRule`, with the fault bound `ffF` and the amend rule `ffBody` |
| `sAnd`, `sOr` | definition | | `and` and `or` on `Sum Unit Unit` |
| `posVal`, `szLe`, `szEq`, `CtK`, `ctAll`, `tbHas` | definition | | the compare helpers of `accept` |

A program definition that is not a law, with the type `Rule c`, `Honest
c f` or `ForkFree c f` at a closed committee, is checked on the accept
table of the count space. The kernel computes the minimal accepting
vectors. A rule that is not monotone, accepts the zero vector, or does
not accept n gives `REFUSE_RULE` (O3). `Honest` and `ForkFree` check the
O4 margin (`REFUSE_HONEST`), and `ForkFree` also checks the O5 margin
(`REFUSE_FORK`). An open committee or an open f gives no decision, and a
count space above 2^20 gives `REFUSE_BOUND`. See `examples/rule.lang`.

## The policy words

S6 adds the policy words of `docs/PLAN.md` section 12 row S6 (Q-S6-1 to
Q-S6-4 (a)). The fields of each family have a prefix (rule R4).

| Name | Kind | Prefix | Meaning |
|---|---|---|---|
| `Decision` | definition | | `Sum Unit Unit`; `inr unit` accepts |
| `Policy` | family | `pl` | `plOf`, with the committee `plCom`, the fault bound `plF`, the rule `plAuth : Honest plCom plF` and the amend rule `plAmend : ForkFree plCom plF` |
| `polCom`, `hoBodyOf`, `ffBodyOf` | definition | | the accessors of `Policy`, `Honest` and `ForkFree` |
| `authorize` | definition | | `(P : Policy) -> (A : Type 0) -> A -> Approvals (polCom P) -> Decision` |
| `amendOk` | definition | | `(P : Policy) -> Approvals (polCom P) -> Decision` |
| `Amendment` | family on `Policy` and `Policy` | `am` | `amOf`, with `amFrom`, `amTo`, the approvals `amSet` and one `Path` field |
| `Log` | family on `Policy` and `Policy` | `lg` | `lgNil`, or `lgCons` with the amendment `lgHead` and the rest `lgTail` |
| `append`, `appendNilL`, `appendNilR`, `appendAssoc` | definition | | the bind of `Log` and its TCM-E paths |
| `lgLen`, `policyAtSize`, `policyAt` | definition | | the height of a log, and the policy at a height `Pos (szS (lgLen P Q l))` |
| `System`, `systemOf` | definition | | `Stream Policy`, built by `sCorec` at a seed `Size` |

`hoRule` and `ffRule` are sealed: each can only be the whole body of a
closed decided definition, else `REFUSE_HONEST` or `REFUSE_FORK`. The
check reads the written term, so a binder with the same name is refused
too. See `examples/policy.lang`.

## The IR test families

S3 adds `Tree`, `Signal`, `Window`, `Hop` and `Echo` from permit-lang
18deea2 for the IR rows. `Size` is in the prelude.

| Family | Kind | Use |
|---|---|---|
| `Tree` | two recursive fields, `forkLeft` and `forkRight` | the tree refusal `test/ir/tree.lang` |
| `Size` | one recursive field, `szPred` | `sizeSum` and `sizeTrap`; `test/ir/spine.lang` (the heap refusal and the spine evals) |
| `Signal`, `Window` | a block with no recursive field | the block case split `test/ir/block.lang` |
| `Hop`, `Echo` | a block; each has one recursive field of the other family | the block refusal `test/ir/hop.lang` |

`family Tree` exists only for the tree refusal. The gate row checks that
the lowering refuses a fold over a run-time tree with
`REFUSE_RUNTIME_TREE`. Rule R1 lets only this file declare a family, so
the family must be here. The blocks `Signal` / `Window` and `Hop` /
`Echo` exist only for the block rows.

## How to replace the domain

1. Write your families and definitions in `domain/domain.lang`. Keep the
   names apart from the core names in `README.md`.
2. Change the examples and the tests that use the prelude names and the
   IR test families: the kernel rows in `examples/`, `test/check/` and
   `test/eval/`, their lines in the `expect.txt` files, and
   `test/ir/spine.lang`, `test/ir/tree.lang`, `test/ir/block.lang` and
   `test/ir/hop.lang`.
3. Keep a family with two recursive fields for `test/ir/tree.lang`, or
   remove that file and its block in `test/gate.sh`.
4. Run `make check` in the kit directory. The build makes `build/domain.c`
   again when `domain/domain.lang` changes.
