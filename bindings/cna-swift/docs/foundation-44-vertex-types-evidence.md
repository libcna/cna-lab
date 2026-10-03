# Foundation 44 — `IVertexType`, the four vertex structs, and a compiler bug

`VertexDeclaration` unblocked five types at once. All five are complete, and the
milestone found two things worth writing down: a projection rule that was too
strict, and a Swift 6.0.3 compiler crash that constrains how a consumer may use
the interface.

## 1. What was built

`IVertexType` is one get-only `VertexDeclaration`. Its recorded accessor verdict
is `IL_ABSTRACT_DECLARATION` with `fallible: false` — an interface declaration
has no body to fail in — so it is a plain non-throwing requirement.

Each of the four structs is `public sequential ansi serializable sealed
beforefieldinit`, extends `System.ValueType`, implements `IVertexType`
**explicitly**, and carries a `public static initonly VertexDeclaration
VertexDeclaration` built by its class constructor. That last word is why this
milestone had to come after Foundation 41: without `readonly` in the contract,
all four static fields would have been demanded as mutable `var`s.

The element tables come from the four `.cctor` bodies:

| Struct | Elements `(offset, format, usage, usageIndex)` | Stride |
| --- | --- | --- |
| `VertexPositionColor` | `(0, Vector3, Position, 0)`, `(12, Color, Color, 0)` | 16 |
| `VertexPositionColorTexture` | + `(16, Vector2, TextureCoordinate, 0)` | 24 |
| `VertexPositionNormalTexture` | `(0, Vector3, Position, 0)`, `(12, Vector3, Normal, 0)`, `(24, Vector2, TextureCoordinate, 0)` | 32 |
| `VertexPositionTexture` | `(0, Vector3, Position, 0)`, `(12, Vector2, TextureCoordinate, 0)` | 20 |

**The strides are not transcribed.** Each class constructor calls the
*one-argument* `VertexDeclaration` constructor, so the stride is what Foundation
43's maximum-end-offset rule computes, and the tests assert the four results.
That is the cheapest end-to-end check that the rule is right, because XNA's own
class constructors depend on it.

`GetHashCode` goes through `Helpers.SmartGetHashCode` over the sequential
layout — 4, 6, 8 and 5 `int32` words respectively — substituting `0x7FFFFFFF`
for a zero XOR, which an all-zero vertex reaches and a test pins. `op_Equality`
compares field by field through each field's own `op_Equality` and never
bitwise, so a NaN component makes a vertex unequal to itself while its hash
still matches; the CLR has exactly that disagreement and it is reproduced rather
than repaired.

The comparison **order** was read rather than assumed, and one of the four is
not in declaration order: `VertexPositionColor` compares `Color` first and
`Position` second, while the other three go left to right. That decides only
what short-circuits and is not observable, but writing all four from one
template without checking would have been guessing.

## 2. The witness rule was too strict, and is now measured on the right thing

The protocol-witness rule required the member to be **absent from the owner's
public declared CLR members**. Each vertex struct declares a
`public static initonly VertexDeclaration VertexDeclaration` field *and* an
explicit instance `IVertexType.get_VertexDeclaration`, so a name-only test saw
the field and refused the witness:

```text
UNMEASURED_STRUCTURAL_CATEGORY  Microsoft.Xna.Framework.Graphics.VertexPositionColor.VertexDeclaration
  protocol-witness rule lacks a unique concrete owner, exactly one direct CLR
  interface declaring the member, or absence from the owner's public declared
  CLR members
```

The CLR distinguishes the two by staticness and so does Swift, so the exclusion
is now about a same-named **instance** member: only that would mean the class
implemented the interface implicitly and needed no witness. The rule was written
for `IPackedVector<T>`, where the question never arose; generalizing it changed
no existing registration, and `PROTOCOL_WITNESS_MEMBER_PROJECTIONS` went from 29
to 33 by adding exactly the four new ones.

## 3. `any IVertexType` crashes swift-frontend 6.0.3

Three lines, against the built library:

```swift
let v = Microsoft.Xna.Framework.Graphics.VertexPositionColor(
    Microsoft.Xna.Framework.Vector3(1, 2, 3), Microsoft.Xna.Framework.Color.White)
let p: any Microsoft.Xna.Framework.Graphics.IVertexType = v
_ = p.VertexDeclaration
```

```text
swift-frontend: swift/include/swift/AST/Type.h:448:
  swift::CanType::CanType(TypeBase *):
  Assertion `isActuallyCanonicalOrNull() && "Forming a CanType out of a
  non-canonical type!"' failed.
While evaluating request IRGenRequest(...)
```

It is IR generation, not type checking, and the boundary was measured rather
than guessed:

| Form | Result |
| --- | --- |
| `G.VertexPositionColor.VertexDeclaration` (the static field) | compiles |
| `vertex.VertexDeclaration` (the witness, concretely) | compiles |
| `func f<T: G.IVertexType>(_ v: T) { v.VertexDeclaration }` | compiles |
| `any G.IVertexType` opened and read | **crashes** |
| `any G.IGraphicsDeviceService` opened and read | compiles |
| `any G.IPackedVector` opened and read | compiles |

So it is not existentials in general, and not this project's nested namespaces:
the two neighbouring protocols open fine. A standalone package reproducing the
shape in the abstract — nested enums, a struct conforming, a class-typed
requirement, a static field colliding with the instance witness — did **not**
crash, so the trigger needs more than that combination and was not chased
further.

What this changes for the projection: nothing. The conformance is what the
metadata says, the library builds, and both non-existential forms work. What it
changes for a **consumer** on this toolchain is that `any IVertexType` is
unusable and a generic constraint must be written instead, which is the
idiomatic Swift anyway. The tests use the constraint form and say why.

## 4. Falsifiability, and two tests the gate rejected first

Four new mutations: an element quadruple's offset moved, a static declaration
left unnamed, one word dropped from a folded hash, and one field left out of
`op_Equality`. **Two of the four survived the first run**, and both were real
weaknesses in the tests rather than in the projection.

*One field left out of `op_Equality`* survived because the equality test only
ever built a `VertexPositionColor`; the mutation dropped
`VertexPositionNormalTexture.TextureCoordinate`. The test now varies each field
of each of the four in turn.

*One word dropped from a folded hash* survived twice, and the second time is the
more interesting one. The first version changed one component per struct and the
mutation dropped a component it never changed. The second version varied every
component in turn and required the resulting hashes to be **distinct from each
other** — and that still passed, because dropping a word makes the case that
varies that word reproduce the *unchanged* vertex's hash, which is distinct from
all the others. Only adding the unchanged vertex to the comparison turns "these
components differ from each other" into "this component moves the hash". Both
repairs were verified against the planted defect directly before the gate was
re-run.

## 5. Measurement

```text
COMPLETE_TYPES=149  (was 144)      MISSING_TYPE=101  (was 106)
PURE_XNA_DERIVED=2239  (was 2193)
TOTAL_DIAGNOSTICS=224  (was 229)   UNMEASURED_STRUCTURAL_CATEGORY=0
PROTOCOL_WITNESS_MEMBER_PROJECTIONS=33  (was 29)
530 tests, 0 failures
PROJECTION_MUTATIONS=50 CAUGHT=50 SURVIVORS=0  (was 46)
```
