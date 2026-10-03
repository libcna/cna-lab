# Foundation 27–28 — a CLR BCL base class projects through real Swift inheritance

Sixteen still-missing XNA types share one question: what happens to a CLR class
used as the direct **base** of an XNA class. Foundation 26 admitted the
Microsoft `mscorlib` that can answer it for the collection half. This milestone
decides the rule, builds the support classes, and proves them with the type the
question was first asked about.

## The rule

> A CLR BCL base class with a registered Swift projection becomes a **real
> Swift superclass**, and the CLR generic argument is preserved in the
> specialization.

```text
System.Collections.ObjectModel.Collection`1          ->  CNACollection<Element>
System.Collections.ObjectModel.ReadOnlyCollection`1  ->  CNAReadOnlyCollection<Element>
System.Collections.Generic.List`1                    ->  CNAList<Element>
```

```swift
public final class GameComponentCollection:
    CNACollection<any Microsoft.Xna.Framework.IGameComponent>
```

The base is **not** dropped to `Object`, the inherited state is **not**
flattened into the subclass, no Swift value type is substituted, and
composition is **not** used merely because the base is non-XNA. Each of those
is a measured negative control below.

The three support classes live outside `Microsoft.Xna.Framework`. They are
language/BCL-support API, not XNA types and not XNA identities, and they are
counted in no XNA scoreboard. No `::System` namespace is fabricated for them —
a test asserts their reflected names carry neither `Microsoft.Xna.Framework`
nor `System.`.

### Why not an Array, and why not composition

`Collection<T>..ctor(IList<T> list)` stores its argument by reference and never
copies it, so the collection is a **live view** of the caller's list; a Swift
`Array` would hand back a snapshot and lose both that and the CLR reference
identity. Composition would compile, but `GameComponentCollection` declares
only a constructor, four overrides and two events — everything usable is
inherited — so composition would mean re-declaring fifteen members by hand and
the compiler would stop checking that they match. It is also, by itself,
undetectable in the Symbol Graph as anything other than a missing base, which
is exactly how the verifier now reports it.

## The BCL behaviour, from the admitted binary

Everything below was read from the CIL of the `mscorlib` admitted in
Foundation 26 (SHA-256 `5634668d…acc63`).

### Construction

```text
Collection`1::.ctor()            items = new List<T>()        // its own store
Collection`1::.ctor(IList<T>)    list == null -> ArgumentNullException
                                 items = list                 // stored, live
ReadOnlyCollection`1::.ctor(IList<T>)
                                 list == null -> ArgumentNullException
                                 list = list                  // stored, live
```

`get_Items` is `ldarg.0; ldfld items; ret` — the field itself, so the protected
view is the same object every time and never a copy.

Both null checks are unreachable through the Swift signatures: a non-Optional
class parameter cannot be nil, so neither initializer throws. That is a
narrowing of the input domain, not a change of behaviour for any value the
signature admits.

### Every public mutator routes through a hook

```text
Add(item)            IsReadOnly -> NotSupportedException
                     InsertItem(items.Count, item)     // count read BEFORE
Clear()              IsReadOnly -> NotSupportedException
                     ClearItems()
Insert(index, item)  IsReadOnly -> NotSupportedException
                     index < 0 || index >  Count -> ArgumentOutOfRangeException
                     InsertItem(index, item)
Remove(item)         IsReadOnly -> NotSupportedException
                     i = items.IndexOf(item); i < 0 -> return false, NO hook
                     RemoveItem(i); return true
RemoveAt(index)      IsReadOnly -> NotSupportedException
                     index < 0 || index >= Count -> ArgumentOutOfRangeException
                     RemoveItem(index)
Item setter          IsReadOnly -> NotSupportedException
                     index < 0 || index >= Count -> ArgumentOutOfRangeException
                     SetItem(index, value)
```

None of them touches `items` on any other path, so a subclass override is
always reached. Three details a plausible reimplementation gets wrong, each
with a test:

- **`Insert` uses `ble`; the setter and `RemoveAt` use `blt`.** Inserting *at*
  `Count` is a legal append; assigning or removing one past the end is not;
- **`Add` reads `items.Count` before calling the hook**, so an override
  receives the index the item had on entry;
- **`Remove` calls no hook at all when the item is absent** — it asks the
  backing store for the index first and returns `false`.

The four hooks each delegate once: `ClearItems -> items.Clear()`,
`InsertItem -> items.Insert(index, item)`, `RemoveItem -> items.RemoveAt(index)`,
`SetItem -> items[index] = item`.

### Overridability is exactly the four hooks

Every public method of `Collection<T>` is `virtual final` in the metadata — a
*sealed* interface implementation. The four protected hooks are `virtual` and
not final. The Swift projection reproduces that split: the public surface is
`final` and the four hooks are `open`. Making `Add` or `Insert` overridable
would invent an extension point the CLR does not have.

### Iteration

`Collection<T>.GetEnumerator` returns the **backing store's** enumerator, and
`List<T>.Enumerator.MoveNext` throws `InvalidOperationException` once the
`_version` it captured no longer matches. That is the contract `CNAEnumerator`
already projects as `CNAError.collectionModified`, so no second enumerator
model was invented. A consequence worth stating: a mutation made directly
through a *wrapped* list, never touching the collection, invalidates the
collection's enumerator too. There is a test for exactly that.

The support classes deliberately do **not** conform to Swift `Sequence`.
`IteratorProtocol.next()` cannot throw, so adopting it would supply an
iteration path that cannot express the invalidation contract. A test asserts
the absence of the conformance rather than leaving it implicit.

### Element equality

`List<T>.IndexOf` compares with `EqualityComparer<T>.Default`, which the CLR
resolves at runtime from the element type. `cnaDefaultEquals` resolves it the
same way: a dynamic `Equatable` conformance is preferred, exactly as the CLR
prefers `IEquatable<T>` and an overridden `Object.Equals`; otherwise two class
instances compare by identity, which is what `Object.Equals` reduces to.

One row diverges and is documented rather than hidden: a Swift value type with
no `Equatable` conformance has no equality to use, where the CLR would fall
back on reflective field-wise comparison. Every specialization the pinned XNA
contract actually declares — `IGameComponent`, `Effect`, `ModelBone`,
`ModelMesh`, `ModelMeshPart` — is a reference type, so no projected XNA surface
reaches it.

### `CNAList` is deliberately smaller than `List<T>`

Only the `IList<T>`/`ICollection<T>` surface the two collection families
actually call is projected, plus the parameterless constructor and `Add`
needed to build a list to wrap. `Sort`, `Reverse`, `BinarySearch`,
`ConvertAll`, `GetRange`, `Capacity` and the rest are absent, and the verifier
forbids them: **admitting BCL behaviour is not the same as approving a public
projection of it.**

## The XNA proof — `GameComponentCollection`

Re-derived from the registered `Microsoft.Xna.Framework.Game.dll`
(SHA-256 `b5dffdd8…a1f0`), whose declared base is exactly
`Collection`1<Microsoft.Xna.Framework.IGameComponent>`. Both exception messages
are the assembly's **own** resource strings, decoded out of its
`Microsoft.Xna.Framework.Resources.resources` blob rather than invented:

```text
CannotAddSameComponentMultipleTimes
    "Cannot add the same game component to a game component collection
     multiple times."
CannotSetItemsIntoGameComponentCollection
    "Cannot set a value using operator[] on GameComponentCollection.
     Use Add/Remove instead."
```

### The four overrides, in IL order

```text
InsertItem(index, item)
    base.IndexOf(item) != -1 -> ArgumentException      // BEFORE the insert
    base.InsertItem(index, item)
    if (item != null) OnComponentAdded(new …EventArgs(item))

RemoveItem(index)
    item = base.get_Item(index)                        // read FIRST
    base.RemoveItem(index)
    if (item != null) OnComponentRemoved(new …EventArgs(item))

SetItem(index, item)
    throw NotSupportedException                        // and nothing else

ClearItems()
    for (i = 0; i < base.Count; i++)                   // FORWARD
        OnComponentRemoved(new …EventArgs(base.get_Item(i)))
    base.ClearItems()                                  // AFTER the loop
```

Four orderings are observable and each is tested:

- a **refused duplicate stores nothing and raises nothing** — the throw
  precedes both the insert and the raise;
- `ComponentAdded` fires with the item **already in** the collection;
- `ComponentRemoved` fires with the item **already gone**;
- **`Clear` announces every component while the collection is still full.**
  A handler that reads `Count` during `Clear` sees the original count, and one
  that reads `Item(0)` sees the first component rather than the one being
  reported. The obvious implementation — remove each item, then announce it —
  is observably different, and this ordering is XNA's, taken from this
  assembly's IL and from no other binding.

`SetItem` is refused outright with no bounds check of its own. Because
`Collection<T>.set_Item` validates *before* reaching the hook, an out-of-range
index still reports the range rather than the refusal — the inherited order is
preserved, not short-circuited. There is a test for that too.

The duplicate check is by identity, so two distinct components are two entries
even when otherwise indistinguishable.

### Events

The Foundation 19 architecture is reused unchanged: `CNAEvent`,
`CNAEventSource`, `CNAEventSubscription`, `CNAEventArgs`. No second event
system was invented. The collection owns two private `CNAEventSource`s and
publishes only the consumer views, which is what the CLR's `private` fields
plus `add_`/`remove_` accessors mean; XNA's own `OnComponentAdded` and
`OnComponentRemoved` are `private` in the IL and are therefore not projected.
Sender is the collection, and each raise constructs a fresh
`GameComponentCollectionEventArgs` — every `newobj` in the IL — so two
notifications never share an argument object.

## Verification

### The base is measured in two halves

The Swift Symbol Graph's `inheritsFrom` relationship names only the **generic
symbol** and carries no generic argument:

```json
{"kind": "inheritsFrom",
 "source": "…GameComponentCollectionC", "target": "s:3CNA13CNACollectionC"}
```

So the superclass **identity** is proved from the compiler relationship, and
the **generic argument** is supplemented from the compiled Swift source
declaration — the same supplementation the enum raw types already use, and
reported as `UNMEASURED_STRUCTURAL_CATEGORY` when it cannot be read rather than
assumed either way. Both halves must match, and neither is allowlisted.

### Negative controls run against the real tree

Each of these was applied to the actual source, rebuilt, and re-measured:

| Break | Caught by |
|---|---|
| `CNACollection<Any>` erasure | **compiler** (overrides stop matching) *and* verifier `BASE_MAPPING_MISMATCH` |
| wrong element type (`IUpdateable`) | **compiler** (17 errors) *and* verifier |
| composition instead of inheritance | **verifier only** — it compiles cleanly, and reports `expected base CNACollection, found None` |
| the four hooks made `public` instead of `open` | verifier, naming all four |

The composition row is the one that matters most: it is the failure a
compile-only check would have missed entirely.

Beyond those, the self-test suite adds negative controls for a missing base,
`Any?`/`AnyObject` in its place, the wrong support class, `CNAEventArgs` in its
place, an `[Element]` Array substitution, the backing store as the base, a
struct where the CLR declares a class (caught twice), six wrong
specializations including an unspecialized base, unreadable source evidence,
and a support class smuggled into the XNA namespace. The support classes
themselves are measured against a pinned shape: kind, `open`/`final`, arity,
base, required members, hooks that must be `open`, and forbidden members.

```text
API_SELF_TESTS          2197 -> 2288
SYMBOL_GRAPH_SELF_TESTS 17 (unchanged)
```

## Scoreboard

```text
                              before   after
REFERENCE_TYPES                  257     257     unchanged
REFERENCE_MEMBERS               2964    2964     unchanged
EXPECTED_SWIFT_TYPES             257     257     unchanged
EXPECTED_SWIFT_MEMBERS          2887    2887     unchanged
TARGET_TYPES                     124     125
TARGET_MEMBERS                  1696    1703
COMPLETE_TYPES                   119     120
MISSING_TYPE                     133     132
TOTAL_DIAGNOSTICS                286     285
MISSING_MEMBER                   130     130
PARTIAL_TYPES                      5       5
BASE_MAPPING_MISMATCH              2       2
INTERFACE_MAPPING_MISMATCH         1       1
PROPERTY_MAPPING_MISMATCH          4       4
OVERLOAD_MAPPING_MISMATCH         16      16
UNMEASURED_STRUCTURAL_CATEGORY     0       0
ALLOWLIST_ENTRIES                  0       0
```

`EXPECTED_SWIFT_MEMBERS` did **not** move, and that is a derivation rather than
a target. The seven new `TARGET_MEMBERS` are exactly
`GameComponentCollection`'s seven declared XNA identities — one constructor,
four overrides, two events — every one of which was already in the pinned
contract and already counted in the 2,887. The BCL surface the type inherits
is real, usable, and *not* an XNA identity, so it is counted on its own axis
and in no XNA total:

```text
BCL_BASE_PROJECTIONS               5    contract types with a decided BCL base
PROJECTED_BCL_BASE_TYPES           1    …implemented today
PENDING_BCL_BASE_TYPES             4    …still blocked on something else
BCL_INHERITED_MEMBER_PROJECTIONS  16    public members inherited, not declared
BCL_SUPPORT_TYPE_MEASUREMENTS      3    support classes held to a pinned shape
MEASURED_SUPPORT_BASE_PROJECTIONS  4 -> 9
```

No number is counted twice: 16 inherited BCL members appear in
`BCL_INHERITED_MEMBER_PROJECTIONS` and nowhere else, and the four pending types
are named rather than silently dropped.

The behaviour corpus keeps the same separation. `Foundation28ContractTests` is
XNA-derived and joins the XNA corpus; `Foundation27ContractTests` is derived
from `mscorlib` and is reported under a separate `PURE_BCL_DERIVED` authority,
because folding it into `OBSERVATIONS` would relabel BCL behaviour as XNA
behaviour.

```text
PURE_XNA_DERIVED   1768 -> 1835 observations, 0 failures
PURE_BCL_DERIVED   0    -> 77   observations, 0 failures
XCTEST             253  -> 309  tests, 0 failures
```

## Gates

```text
DEBUG_BUILD=PASS                RELEASE_BUILD=PASS
DEBUG_TESTS=309 PASS            RELEASE_TESTS=309 PASS
NATIVE_TESTS=309 PASS (0 skipped, CNA_NATIVE_LIBRARY set)
WARNINGS_AS_ERRORS=PASS_DEBUG_AND_RELEASE_INCLUDING_TESTS (forced full rebuild)
SYMBOL_GRAPH=PASS
API_SELF_TESTS=2288 PASS        SYMBOL_GRAPH_SELF_TESTS=17 PASS
NORMAL_STRICT=EXPECTED_RED_DEFERRED_PROFILE_ONLY
LEAK_ONLY=PASS
NATIVE_ABI=29/91/91/18/2/214 MISSING=0/0 MISMATCHES=0   unchanged
```

No CNA ABI symbol was added. This is pure Swift/BCL-support work and reaches no
native surface: a managed `Collection<T>` base needs no native API and got
none.

## What this milestone deliberately does not decide

The mechanism is general, but a decided base requires an admitted authority
behind it — a self-test enforces that every measured generic support base is a
projected admitted BCL family. These remain undecided and still report as
unmeasured if anything tries to implement them:

```text
System.Exception                                  8 XNA exception types
System.Runtime.InteropServices.ExternalException
System.Attribute                                  5 ContentSerializer* types
System.Collections.Generic.Dictionary`2           LaunchParameters
System.ComponentModel.ExpandableObjectConverter   MathTypeConverter
System.IO.BinaryReader                            ContentReader
```

`System.Exception` in particular is a cross-cutting public error-architecture
question — whether projected XNA exceptions conform to Swift `Error`, inherit
from a support class, or interact with `CNAError` — and admitting the binary
that declares it decides none of that.
