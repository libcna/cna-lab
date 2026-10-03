# Formal XNA 4.0 to Swift mapping

The public API authority is the pinned Microsoft XNA Framework 4.0 Windows
runtime metadata contract, not CNA and not a sibling binding. The selected
reference contains 257 public types and 2,964 declared members. The retained
contract SHA-256 is
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
The snapshot was extracted from the Microsoft XNA 4.0 Windows runtime metadata
by mature-binding tooling and copied byte-for-byte; `tools/api_compat` verifies
the retained snapshot hash before projecting it.

## Registered reference assemblies

A hash-matched assembly is the **behaviour** authority for the types it
declares. All seven assemblies below are version 4.0.0.0 of the same Microsoft
XNA Framework redistributable and are registered as authoritative reference
inputs. No Microsoft binary or extracted proprietary source is in the
repository or the release archive; only the hashes are retained.

| Contract types | Assembly | SHA-256 | Registered |
|---:|---|---|---|
| 120 | `Microsoft.Xna.Framework.dll` | `38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130` | Foundation 1 |
| 99 | `Microsoft.Xna.Framework.Graphics.dll` | `560080fc39021c611ca9d076dcebed312faf6d7d1413c2dc523683ea635e9f55` | Foundation 1 |
| 17 | `Microsoft.Xna.Framework.Game.dll` | `b5dffdd8125abef2a4507ba4e1d2f11062143f0a63d48fe4f298b95ad746a1f0` | Foundation 17 |
| 8 | `Microsoft.Xna.Framework.Input.Touch.dll` | `b0585224c18022c3661057ae79544644c10f33f1dc529678364f3d6b25151c25` | Foundation 17 |
| 7 | `Microsoft.Xna.Framework.Xact.dll` | `a14d5364dca7cf49fb90639e87ba04d52b59a700dc9198efa5707ce8eae28f0a` | Foundation 17 |
| 3 | `Microsoft.Xna.Framework.Video.dll` | `17538b1ca9d48a993e2cd88c96b436df08e7abb4aec5d4758eb21feb580d6e06` | Foundation 17 |
| 3 | `Microsoft.Xna.Framework.Storage.dll` | `798f678e9ae3d9afc3bed66c30123bc9634fb923b6d200188344b618e608cbb8` | Foundation 17 |

Registration is a deliberate provenance step, never a side effect of selecting
a type. An assembly is registered only after
`tools/api_compat/pinned_assembly_audit.py` machine-compares its public
metadata against every retained contract entry it owns. That comparison
reproduces the contract's **257 types and all 2,964 members exactly**, with
zero mismatches, from these seven files and no others.

The audit's own correctness is not asserted, it is calibrated: the two
assemblies registered before the tool existed must reproduce their entries
exactly, and `--require-exact` makes that a hard gate. Sixty mutation
self-tests additionally prove the comparison is not vacuous — a dropped,
renamed, retyped, restaticed or invented member, a changed constant, a flipped
`sealed` bit, a changed base type and a dropped declared interface must each
be detected.

The contract records a *reduced* direct-interface set: an interface already
implied as a base of another listed entry is omitted, as `IEnumerable` is
behind `IEnumerable<T>`. The audit therefore requires the sound relation —
every interface the contract records is actually declared by the assembly —
and reports the 43 reductions separately rather than treating them as
discrepancies.

## Names and kinds

- Namespace marker enums produce `Microsoft.Xna.Framework`, `.Graphics`,
  `.Graphics.PackedVector`, `.Input`, `.Content`, and `.Audio`. Markers are
  mapping infrastructure, not XNA types, and the verifier excludes the eight
  marker symbols. A namespace gains its marker when its first type is
  implemented; `.Audio` was added in Foundation 14 for `SoundState` and
  `AudioChannels`.
- CLR class -> Swift class; an externally subclassable CLR class -> `open`
  Swift class where required.
- A public CLR class whose declared constructors are all non-public is not
  externally constructible or derivable. It maps to a plain Swift `public class`
  that is neither `open` — because no accessible constructor makes it externally
  subclassable — nor `final` unless the CLR type is itself sealed, because
  `sealed=false` must not be strengthened. Openness is never chosen
  mechanically from the sealed bit alone; constructor accessibility is inspected.
- CLR struct -> Swift struct.
- CLR interface -> Swift protocol.
- CLR enum -> Swift enum with the reviewed fixed-width raw type.
- CLR `[Flags]` enum -> Swift `OptionSet` when raw flag behavior is preserved.
- CLR delegate -> a typed Swift closure at the native boundary; public delegate
  types will receive deterministic named closure projections when selected.
- Nested CLR `+` identity -> nested Swift `.` identity.
- Public XNA spelling remains PascalCase whenever Swift syntax permits it.
  There are no lowercase strict-surface compatibility aliases.

## Members and overloads

- CLR `.ctor` -> overloaded Swift `init`.
- Value-type constructor external labels are `_`; class constructor labels
  preserve metadata parameter names.
- A normal method's first external label is `_`; later labels preserve XNA
  metadata names. Operator parameters all use `_`.
- Fields and properties preserve static/instance identity, type, and
  mutability. XNA static value properties remain Swift static properties.
- CLR operators map to Swift operator functions. The verifier measures each
  overload separately.
- A reference-typed operator whose null behavior must remain observable may
  require Optional operands. Swift cannot declare an operator as a type member
  when both operands are Optional, so the configured global compiler symbol is
  assigned back to its XNA owner and measured as that same operator identity.
- CLR `ref`/`out` -> Swift `inout`; direction, type, label, and mutability are
  measured.
- A CLR caller-owned destination array named `destinationArray` or `corners`
  maps to Swift `inout Array`. Source arrays remain value snapshots. This
  preserves writes through Swift value semantics and gives overlapping
  source/destination calls an explicit copy-on-write snapshot rule. The verifier
  measures every selected destination-array mapping independently.
- The `array` parameter of `ICollection<T>.CopyTo` is identified from the
  pinned direct interface plus member semantics and likewise maps to `inout
  Array`. This is a general caller-owned mutation rule, not a diagnostic
  allowlist.
- CLR protected virtual lifecycle members -> `open` methods. Their additional
  Swift visibility is `LANGUAGE_MAPPING`, not an unexpected XNA member.
- A non-public CLR constructor maps to an `internal` Swift initializer with the
  same arity, parameter order and types. It is implementation infrastructure:
  the compiler-emitted public Symbol Graph must expose zero public `init`
  identities for such a type, and no public static factory may substitute for
  it. `NONPUBLIC_CONSTRUCTION_PROJECTIONS` measures every implemented reference
  class in this category and records its observed public Swift initializer
  count, so the rule is enforced generally rather than per named type.
- A selected inherited public member may temporarily be declared on a partial
  subtype until its XNA base exists. It must exactly match a selected ancestor.
  `System.IDisposable` maps to public `Dispose()`. These are counted mapping
  entries, not silently ignored members.

## Generics and nullability

Swift cannot place `Foo` and `Foo<T>` in one scope. The non-generic type keeps
its canonical name. The two collisions in this profile map deterministically:

- `ContentTypeReader<T>` -> `ContentTypeReaderOfT<T>`
- `IPackedVector<TPacked>` -> `IPackedVectorOfT<TPacked>`

No aliases collapse the identities. Reference nullability maps to Swift
Optional only where the selected XNA signature permits null; for return
positions "permits null" is a measured verdict, set out in full under *The
general reference-return nullability projection* below. Native create failure
is an Error, not an Optional result. Value types remain non-optional unless the
signature itself is nullable.

`System.Nullable<T>` maps independently to Swift `T?` when `T` is a faithfully
represented value type. Thus XNA geometry intersection distances return
`Float?`, and `out Nullable<Single>` maps to `inout Float?`; `nil` means XNA
`HasValue=false` and is not an error or sentinel value. Selected
`IEnumerable<Vector3>` point inputs map to `[Vector3]`, the established finite
Swift collection projection, without introducing a synthetic Microsoft
collection type.

Reference parameters become Optional only where null is an observable selected
operation rather than merely an immediate invalid argument. CurveKey therefore
maps typed `Equals`, `CompareTo`, and both operator operands to Optional;
CurveKeyCollection maps `IndexOf`, `Contains`, and `Remove` items to Optional.
`CompareTo(nil)` throws the mapped null-reference error. `Add(nil)` and an
indexed setter value of nil are immediate XNA argument failures, so those
strict parameters remain non-Optional.

`IPackedVectorOfT<TPacked>` uses Swift's primary-associated-type protocol
syntax and explicitly declares `associatedtype TPacked`. The compiler Symbol
Graph must expose that exact name and the verifier measures it, not merely the
number of generic parameters. A concrete conformance supplies a same-named
typealias; Color and all 17 concrete PackedVector structs expose the exact
fixed-width compiler witness, producing the corresponding mapped
`IPackedVectorOfT<TPacked>` interface.

Interface mutation maps to a `mutating` Swift protocol requirement when the CLR
operation changes packed struct storage. Thus `IPackedVector.PackFromVector4`
is mutating, while `ToVector4` is not. XNA's private explicit-interface methods
do not count among a concrete struct's public declared CLR members, but Swift
requires public conformance witnesses. The verifier excludes a witness only
when the concrete CLR type has the direct generic packed interface, the
non-generic interface declares the requirement, the concrete public contract
lacks it, the compiler emits a matching `sourceOrigin`, and the full witness
signature/mutating identity matches. It records all such deterministic
language projections separately rather than using a diagnostic allowlist.
Unrelated public members and malformed witnesses remain errors.

## Errors

Swift `throws` is the language projection for runtime/XNA failure paths because
Swift has no CLR unchecked exceptions. It does not add a reference member and
is ignored only as a measured `LANGUAGE_MAPPING` signature detail. Ordinary
failures never use `fatalError`, process exit, silent defaults, or no-ops.
`CNAError` is support API outside the strict XNA namespace. It carries the
projections of the CLR exceptions the pinned surface actually throws, including
`System.NotSupportedException` as `notSupported`.

## The general CLR property accessor projection

Swift has **no throwing property setter**, and the compiler separately refuses
any `set` beside a getter that is `throws`. One rule covers every CLR property,
indexed or not, and it is decided per *accessor*:

- the **getter** keeps Swift accessor syntax — `var P` for a plain property,
  `subscript` for an indexed one — and gains the `get throws` effect exactly
  when the CLR getter is fallible;
- the **setter** keeps Swift `set` syntax only when Swift can express it, which
  means only when both accessors are infallible. Otherwise it becomes the
  method `Set<PropertyName>`, preserving XNA capitalisation and carrying the
  setter's own `throws`.

| CLR accessors | Swift projection |
|---|---|
| infallible get, no set | `var P: T { get }` |
| infallible get, infallible set | `var P: T { get set }` |
| infallible get, **throwing set** | `var P: T { get }` + `func SetP(_ value: T) throws` |
| **throwing get**, no set | `var P: T { get throws }` |
| **throwing get**, throwing set | `var P: T { get throws }` + `func SetP(_ value: T) throws` |
| throwing get, infallible set | `var P: T { get throws }` + `func SetP(_ value: T)` |
| set only | `func SetP(_ value: T)`, `throws` per the setter |

Getter fallibility and setter fallibility are decided independently and never
infect one another. The single exception is a compiler constraint, not a
preference: Swift 6.0.3 rejects a `set` accessor beside a throwing getter with
*"'set' accessor is not allowed on property with 'get' accessor that is 'async'
or 'throws'"*, so a throwing getter forces the writer out of `set` syntax even
when the setter itself cannot fail.

The writer method is the projection of the CLR **setter accessor**. It is not a
new XNA member: a CLR property remains one CLR member, and the verifier
recombines the reader and writer compiler symbols into that single source
property identity before any scoreboard counts it. `EXPECTED_SWIFT_MEMBERS` is
therefore unchanged by this rule.

`Set<PropertyName>` is the only writer spelling. `setPropertyName`,
`propertyNameSet` and `TrySetPropertyName` are rejected, and a static CLR
setter projects to a static writer.

The **indexed** case is the rule this generalises, unchanged in shape. A
read-only indexed property stays a Swift `subscript`, gaining `get throws` when
its getter is fallible. When its writer becomes a method the reader is spelled
`Item(...)` beside `SetItem(...)` rather than as a subscript, so the two halves
of one CLR member stay one named pair.

Of the twelve reader/writer combinations the rule defines, the pinned contract
exercises eight; the other four — including every write-only property — are
measured as absent rather than assumed away.

### What makes an accessor fallible

`tools/api_compat/reference/xna40-accessor-fallibility.json` is the pinned
per-accessor verdict for all 840 public properties, regenerated by
`tools/api_compat/accessor_fallibility.py` from the CIL of the seven registered
assemblies and hash-checked by the verifier exactly as the contract is. An
accessor is fallible when, and only when:

- a `throw` is reachable from it through calls that stay inside the registered
  assemblies — the shortest chain and the constructed exception types are
  recorded per entry; or
- an unguarded caller-supplied index reaches a CLR index operation, which
  validates it. `CurveKeyCollection.Item` hands its index straight to
  `List<CurveKey>.get_Item`; `EffectParameterCollection.Item` range-checks
  first and returns null, so it is *not* fallible; or
- for an abstract or interface accessor, a registered implementor is. Swift
  forces this: a non-throwing protocol requirement cannot be witnessed by a
  throwing getter at all.

Allocation, calls that leave the registered set, `ldelem` on an internally
computed index, and universal CLR failures such as `OutOfMemoryException` are
**not** XNA contract behaviour and are never fallibility. `BoundingFrustum.Left`
indexes an array and stays infallible because its index is the constant 2.

The one approximation in the analysis — merging same-named overloads by arity —
is bounded and proved immaterial: the *any-overload* and *all-overload*
closures give identical verdicts for every contract accessor, so a
signature-precise analysis is sandwiched between two answers that agree.

Measured over the contract: **114 fallible getters and 113 fallible setters**,
of which 113 writers become methods and none is an infallible writer method.

## The general reference-return nullability projection

A CLR reference return and a CLR failure are **independent facts**. `null` is a
normal, successful result; a thrown exception is not a value at all. Swift
spells the two with orthogonal features, so the projection decides them
separately and all four combinations are legitimate:

| XNA return | XNA getter/method | Swift |
|---|---|---|
| proven non-null | infallible | `T` / `var P: T { get }` |
| proven nullable | infallible | `T?` / `var P: T? { get }` |
| proven non-null | fallible | `throws -> T` / `var P: T { get throws }` |
| proven nullable | fallible | `throws -> T?` / `var P: T? { get throws }` |

Two rules follow, and both are measured rather than trusted:

- **`throws` never stands in for a normal null.** If the selected XNA member
  can normally return null, the Swift return is Optional; if that member has no
  contract-relevant throw path, the Swift member does not throw. No placeholder
  object, sentinel instance, `fatalError`, force unwrap or `CNAError` is
  invented to avoid Optional.
- **Optional never absorbs a real failure.** A member that can *both* return
  null normally and throw on another path is `T?` **and** `throws`. `try?` and
  `catch { return nil }` are not a mapping.

`System.Object` already maps to `Any?` and `System.Nullable<T>` to `T?`; a
proven-nullable return of either is not double-wrapped. Selective reference
*parameter* nullability is unchanged and is not reopened by this rule.

### What proves a reference return nullable

`tools/api_compat/reference/xna40-reference-return-nullability.json` is the
pinned verdict for all **369** public reference-typed return positions — 177
property getters, 170 methods and 22 field reads — regenerated by
`tools/api_compat/return_nullability.py` from the CIL of the seven registered
assemblies and hash-checked by the verifier exactly as the contract is. Each
position is exactly one of three:

- `PROVEN_NULLABLE_SUCCESS` — a normal return of null is demonstrated: an
  `ldnull` that reaches a `ret`, a field whose lifecycle leaves it null at a
  reachable public read, or a callee whose own return is proven nullable.
- `PROVEN_NONNULL_SUCCESS` — every reachable `ret` yields a value whose
  non-nullness is demonstrated (`newobj`, `newarr`, `ldstr`, `box`, a reference
  the member's own null guard refined non-null, a field every constructor
  initialises that nothing ever nulls, or a callee proven non-null) and the set
  of outcomes is exact.
- `UNKNOWN_REFERENCE_NULLABILITY` — neither. A call that leaves the registered
  assemblies, an `isinst`, an array element, a native or mixed-mode boundary,
  and an abstract member with no registered implementor all land here.

An unproven return **keeps the non-Optional projection and is named
individually** in the strict report's `unknownReturnNullabilityProjections`.
That is a recorded deferral, not a decision: guessing such a return into
Optional would invent a state XNA cannot produce, and guessing it out of
Optional would hide one it can.

A field read is decided by the whole observable lifecycle, not by one site: the
value every declared constructor of the declaring type leaves behind, joined
with every store anywhere in the registered set. "This reference field could
theoretically be null" is not evidence; "no constructor assigns it" and
"`Dispose` stores `ldnull` into it" are.

An abstract or interface member declares no body and is decided by joining its
registered implementors, exactly as accessor fallibility is — and for the same
reason: a Swift `override` cannot widen a return to Optional, so the
declaration has to accommodate every implementation.

Measured over the contract: **115 proven nullable, 128 proven non-null, 126
unproven**, and all four nullable/fallible combinations are exercised — 70
nullable-infallible, 45 nullable-fallible, 62 non-null-infallible, 66
non-null-fallible.

### The canonical case

`GraphicsDeviceManager.get_GraphicsDevice` is

```text
IL_0000:  ldarg.0
IL_0001:  ldfld  GraphicsDevice GraphicsDeviceManager::device
IL_0006:  ret
```

a bare field read with no failure path, and `device` is never assigned by the
constructor and is explicitly `ldnull`-stored by both `Dispose` (IL_00b0) and
`CreateDevice` (IL_0014). XNA's own `BeginDraw`/`EndDraw` guard it with
`brfalse` rather than treating null as an error. The XNA-faithful Swift shape
is therefore

```swift
public var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? { get }
```

— Optional and **not** throwing. `null != error` is the whole content of the
rule, and this member is where it is decided.

## Comparison and collection interfaces

`System.IComparable<T>` maps to the exact concrete `CompareTo(T)` member. It
does not imply Swift `Comparable` conformance or synthesize ordering operators.
The pre-existing `System.IEquatable<T>` mapping remains the XNA `Equals(T)` and
operator members without Swift `Equatable` conformance.

`System.Collections.Generic.ICollection<T>` maps to the concrete XNA public
members that implement it: `Add`, `Clear`, `Contains`, `CopyTo`, `Remove`,
`Count`, and `IsReadOnly`. The verifier records the pinned direct-interface
identity and requires those compiler symbols, but there is no fake Microsoft
protocol and no automatic Swift `Collection`, `MutableCollection`, `Sequence`,
or `RandomAccessCollection` conformance.

`System.Collections.Generic.IEnumerator<T>` return values map to the public
support type `CNAEnumerator<T>` outside the XNA namespace. Its throwing
`Next() -> T?` preserves a live cursor and mutation invalidation. It does not
conform to nonthrowing `IteratorProtocol`. The related generic and non-generic
`IEnumerable` identities add no automatic Swift conformance.

`System.Collections.Generic.IList<T>` maps to the concrete XNA members that
implement it: the seven inherited `ICollection<T>` members plus `IndexOf`,
`Insert`, `RemoveAt` and the indexed `Item`. As with `ICollection<T>` there is
no automatic Swift `Collection`, `MutableCollection` or `RandomAccessCollection`
conformance.

A `CopyTo` array parameter on a type whose pinned direct interface is
`ICollection<T>` **or** `IList<T>` is caller-owned mutable destination storage
and maps to Swift `inout`. `IList<T>` inherits `ICollection<T>`, so the
destination is the same; projecting it as a value copy would silently discard
every written element.

The indexed cases of the general accessor projection are set out above. All
four read/write CLR indexers in the contract have a fallible setter and a
fallible getter, so all four are the `Item`/`SetItem` throwing pair; ten
read-only indexers are infallible and stay plain Swift subscripts, and ten are
fallible and are `Item(...) throws`.

## BCL mappings started in Foundation 1

| CLR type | Swift projection | Rule |
|---|---|---|
| `System.TimeSpan` | `Duration` | CNA supplies exact signed 100-nanosecond ticks; Swift constructs seconds/attoseconds without a floating-point interval. |
| `System.IO.Stream` | `Foundation.InputStream` | The strict `Texture2D.FromStream` projection reads the stream to contiguous bytes internally before CNA decode. |
| `System.IntPtr` | `Int` | Pointer-width signed integer; see the general rule below. |
| `System.Object` | `Any?` | Optional preserves CLR null. |
| `System.EventArgs` | `CNAEventArgs` | `open class` outside the XNA namespace; a **measured** base, and `Empty` is one shared instance. See below. |
| `System.EventHandler<TArgs>` | `CNAEvent<TArgs>` | One get-only property per CLR event; no `add_`/`remove_`/`raise_` identity. See below. |
| `System.Collections.Generic.IEnumerator<T>` | `CNAEnumerator<T>` | Throwing live enumeration preserves CLR mutation invalidation without a fake Microsoft type. |

### The general CLR event projection

Every public event in the pinned contract has one shape,
`System.EventHandler<TArgs>` — all 49 of them. Each maps to exactly **one**
get-only Swift property keeping its XNA name:

```swift
var EnabledChanged: CNAEvent<CNAEventArgs> { get }
```

The CLR `add_`/`remove_`/`raise_` accessors are how IL encodes an event, not XNA
identities, and never appear in the Swift surface. Zero members in the pinned
contract carry those prefixes, so any such name is a leaked accessor and is
reported as `EVENT_MAPPING_MISMATCH`.

An event is **not** projected as a bare closure, an array of closures, a
writable closure property, or a callback pointer. Each of those is a measured
mismatch with its own self-test.

Three support types outside the XNA namespace carry the projection. They are
language-support API, not XNA types, so they are counted in no XNA scoreboard —
but they are measured against a pinned shape, not assumed.

| Support type | Role |
|---|---|
| `CNAEvent<TArgs>` | Consumer view. Exactly `Add` and `Remove`. Cannot raise. |
| `CNAEventSource<TArgs>` | Declaring side. Owns the storage, exposes `Event` and `Raise`. |
| `CNAEventSubscription` | Opaque registration token returned by `Add`. |

#### Why a token, and where it differs from CLR

CLR removes a handler by **delegate identity**: target object plus method, so
two references to the same method on the same instance are equal and
`Delegate.Remove` matches them. Swift closures have no identity — two closures
spelled identically are simply two values — so that rule cannot be reproduced.
`Add` therefore returns an opaque token and `Remove` matches on it.

This is recorded as a deliberate `LANGUAGE_PROJECTION` rather than presented as
equivalence. The observable difference: adding the same closure twice creates
**two** registrations with **two** tokens, each removed independently, where
CLR's `Delegate.Remove` would remove the last matching entry of an equal
delegate. The token exposes no handler and no implementation state, cannot be
constructed outside the module, and deliberately does **not** unsubscribe on
`deinit` — a CLR event retains its delegates until they are explicitly removed.

#### Why two types instead of one

CLR reserves raising an event to the declaring type. Swift has no member that is
public to read and private to invoke, so the capability split is modelled as two
objects over one private storage. **`CNAEventSource` is composed with
`CNAEvent`, never derived from it** — both are `final` with no superclass — so a
consumer handed the view has no downcast that recovers `Raise`. The compiler
rejects the attempt outright.

`CNAEventSource` is public because an external package must be able to conform
to `IUpdateable` or `IDrawable` and raise its own events:

```swift
private let enabledChangedSource = CNAEventSource<CNAEventArgs>()

public var EnabledChanged: CNAEvent<CNAEventArgs> {
    enabledChangedSource.Event
}

// inside the declaring type only:
try enabledChangedSource.Raise(self, args: CNAEventArgs.Empty)
```

That publicness is language-support machinery, not an XNA identity. The same
shape is what a future native-backed event will use: a protected runtime type
owns the source privately and a native callback calls `Raise`, while consumers
still see only the view. No such event is implemented, and this milestone adds
no CNA ABI.

#### Subscription and dispatch semantics

- registration order is the invocation order;
- duplicate registrations are permitted and independently removable;
- `Remove` on an already-removed token, or on a token belonging to another
  event, is harmless rather than corrupting;
- storage retains handlers strongly, as a CLR event retains delegates;
- dispatch walks a **snapshot**, so adding or removing inside a handler affects
  later raises, never the raise in progress — this mirrors the CLR raise, which
  loads the delegate field into a local before invoking;
- handlers may throw. The first error propagates to the raiser, no later handler
  runs, and the registration list is left intact. No handler error is swallowed;
- a non-throwing Swift closure is usable wherever the throwing handler type is
  expected.

### `System.EventArgs` as a measured base

`CNAEventArgs` was an empty support **struct**, which made the CLR
event-argument hierarchy inexpressible. It is now an `open class`, and the base
is measured:

| Requirement | Diagnostic when violated |
|---|---|
| Swift superclass is `CNAEventArgs` | `BASE_MAPPING_MISMATCH` |
| not `AnyObject`, `Object`, or another support base | `BASE_MAPPING_MISMATCH` |
| not a `struct` where the CLR declares a class | `TYPE_KIND_MISMATCH` + `BASE_MAPPING_MISMATCH` |

It deliberately does **not** conform to `Sendable`. The class is open, so a
subclass anywhere may add mutable stored state, and `@unchecked Sendable` would
assert exactly the guarantee that cannot be earned.

`CNAEventArgs.Empty` is one shared instance. This is derived, not assumed: all
46 `EventHandler<EventArgs>` raise sites across the registered
`Microsoft.Xna.Framework.Game.dll` and `Microsoft.Xna.Framework.Graphics.dll`
execute `ldsfld System.EventArgs::Empty`, and none executes
`newobj System.EventArgs::.ctor`, so every handler observes the same object.

### Undecided BCL bases are unmeasured, never dropped

A non-XNA base that is neither a CLR root (`System.Object`, `System.ValueType`,
`System.Enum`) nor a decided support projection has **no** silent fallback. If a
type carrying one were implemented, the verifier reports
`UNMEASURED_STRUCTURAL_CATEGORY` rather than accepting a base-less Swift class.

Twenty-one still-missing types are guarded this way, including
`GameComponentCollection` (`Collection<IGameComponent>`), the eight exception
types (`System.Exception`, `ExternalException`), the five
`ContentSerializer*Attribute` types (`System.Attribute`), the four
`ReadOnlyCollection<T>` model types, and `LaunchParameters`
(`Dictionary<String, String>`). Each remains a genuine BCL mapping decision, and
a future decision plugs into the same measured machinery `CNAEventArgs` uses.

### The general `System.IntPtr` language projection

`System.IntPtr` maps to Swift `Int`: the opaque pointer-width signed numeric
value of the CLR IntPtr. `IntPtr.Zero` maps to `0`. This is a **language
projection**, and it is deliberately not any of the following:

- a Swift pointer,
- a dereferenceable address,
- a CNA native handle,
- an SDL window,
- a `GraphicsDevice`,
- proof that the handle is valid,
- proof that CNA can consume it.

`Int` is the correct projection precisely because it follows the host pointer
width rather than fixing one: a `Int64` substitute is a fixed-width choice, and
a `UInt` substitute contradicts CLR IntPtr's signed semantics, so both remain
mapping mismatches. The public XNA projection never exposes
`UnsafeRawPointer`, `UnsafeMutableRawPointer`, or `OpaquePointer` merely
because the CLR source type is `IntPtr`, and holding an XNA `IntPtr` value must
never require unsafe pointer manipulation from a consumer of the binding.

Because the mapped value is an ordinary integer, the expected projection is
**not** counted as `RAW_HANDLE_LEAK`. That exemption is narrow: it applies only
to the mapped XNA IntPtr value and never to a CNA FFI or native implementation
handle. `RAW_HANDLE_LEAK` and `PUBLIC_NATIVE_FFI_LEAK` both remain required at
zero, and the verifier carries negative fixtures for the accidental
`UnsafeRawPointer`, `UnsafeMutableRawPointer`, `OpaquePointer`, `Int64`, `UInt`,
CNA-handle-wrapper, and platform-window-wrapper projections. Each is proved
twice — once against a synthetic owner so the rule is general, and once against
the real selected identity that carries it.

The first XNA public signature to use the rule is
`Graphics.PresentationParameters.DeviceWindowHandle`. A stored handle is pure
managed descriptor state: the projection never dereferences it, validates it
against a real window, resolves it through SDL, creates or resets a device, or
hands it to CNA.

The verifier derives `EXPECTED_SWIFT_TYPES=257`. It derives
`EXPECTED_SWIFT_MEMBERS=2887` by excluding exactly 49 enum backing fields named
`value__` and 28 CLR finalizers. Swift raw-value storage and non-public `deinit`
are language mappings. No missing functional API is hidden by those omissions.

The report reserves `ALLOWLIST_ENTRIES` for genuine manual diagnostic
suppressions; it is currently zero. Deterministic projection transformations
are reported separately as `LANGUAGE_PROJECTION_EXCLUSIONS` (49 enum storage
fields, 28 finalizer mappings, eight namespace markers, three inherited member
projections, and 26 explicit-interface protocol witnesses). Comparison,
collection, enumerator, indexed-property, optional-operator-placement,
caller-owned-array, and non-public-construction mappings have their own precise
counters. A verifier self-test proves that a real suppression counts as an
allowlist entry and that a missing geometry member cannot be reclassified as a
projection rule.
