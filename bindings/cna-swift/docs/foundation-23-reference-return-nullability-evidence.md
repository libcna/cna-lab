# Foundation 23 — reference-return nullability

`null` is a value. An exception is not. XNA returns the first from members that
cannot fail, and raises the second from members that return nothing at all, and
until this milestone CNA-Swift had no way to say which was which: a nullable
CLR reference return was projected non-Optional, and where the binding had to
express "no value" it reached for `throws`. That substitution is wrong in both
directions — it turns a normal result into an error, and it makes a real error
indistinguishable from an ordinary absence — and it is now measured rather than
possible.

This milestone generalises the project's existing selective reference
nullability from parameters to **return positions**, derives the answer for
every one of them from the CIL of the seven registered assemblies, and makes it
a first-class dimension of the strict verifier.

## The decision

A CLR reference return and a CLR failure are independent axes and are projected
independently. All four combinations are legitimate:

| XNA return | XNA member | Swift |
|---|---|---|
| proven non-null | infallible | `T` / `var P: T { get }` |
| proven nullable | infallible | `T?` / `var P: T? { get }` |
| proven non-null | fallible | `throws -> T` / `var P: T { get throws }` |
| proven nullable | fallible | `throws -> T?` / `var P: T? { get throws }` |

Two rules follow:

- **`throws` never stands in for a normal null.** If the selected XNA member
  can normally return null, the Swift return is Optional; if that member has no
  contract-relevant throw path, the Swift member does not throw. No placeholder
  object, sentinel instance, `fatalError`, force unwrap or `CNAError` is
  invented to avoid Optional.
- **Optional never absorbs a real failure.** A member that can *both* return
  null normally and throw on another path is `T?` **and** `throws`; `try?` and
  `catch { return nil }` are not a mapping.

`System.Object` already maps to `Any?` and `System.Nullable<T>` to `T?`; a
proven-nullable return of either is not double-wrapped. The selective
`optionalReferenceParameters` policy is untouched, and the Foundation 22
per-accessor fallibility rule is untouched: fallibility is still decided per
accessor, and nullability is now decided per return position, and neither is
allowed to imply the other.

## How a verdict is derived

`tools/api_compat/return_nullability.py` reads the same seven hash-registered
assemblies as `accessor_fallibility.py`, through the same IL parser and the
same call graph, and abstractly interprets every method body.

The value lattice is three points, ordered

```text
NONNULL  <  UNKNOWN  <  NULLABLE            (join = max)
```

carried on a simulated evaluation stack. Each entry keeps a symbolic origin —
an argument, a local, a static field, or a field of a known object — so a
`brtrue`/`brfalse` (or a `beq`/`bne.un` against `ldnull`) refines exactly the
thing it tested on each outgoing edge, and every field refinement is discarded
at any call or store. `newobj`, `newarr`, `ldstr` and `box` are non-null;
`ldnull` is null; `isinst`, `ldelem.ref`, `ldind.ref`, `unbox.any` and any call
that leaves the registered assemblies are unknown. Method and field summaries
are computed together to a least fixpoint — eight rounds over 5,620 methods.

A **field** is decided by its whole observable lifecycle, not by one read: the
value every declared constructor of the declaring type leaves behind, joined
with every store anywhere in the registered set. A field no constructor assigns
holds the CLR's zero when the constructor returns, and is therefore nullable at
every later public read. That is what separates "this reference field could
theoretically be null" from "XNA normally leaves it null at a reachable public
read".

An **abstract or interface** member declares no body and is decided by joining
its registered implementors, exactly as accessor fallibility is, and for the
same reason: a Swift `override` cannot widen a return to Optional, so the
declaration has to accommodate every implementation. A virtual member with a
body is joined with its registered overrides on the same ground. That raised
exactly one verdict in the whole contract — `ContentTypeReader<T>.Read` from
`PROVEN_NONNULL_SUCCESS` to `UNKNOWN_REFERENCE_NULLABILITY` — and the report
names it.

### Evidence classes

| Verdict | Meaning |
|---|---|
| `PROVEN_NULLABLE_SUCCESS` | a normal return of null is demonstrated |
| `PROVEN_NONNULL_SUCCESS` | every reachable `ret` yields a demonstrably non-null value, and the outcome set is exact |
| `UNKNOWN_REFERENCE_NULLABILITY` | neither is demonstrated inside the registered set |

An unproven return **keeps the non-Optional projection and is named
individually** in the strict report's `unknownReturnNullabilityProjections`.
That is a recorded deferral, not a decision: guessing such a return into
Optional would invent a state XNA cannot produce, and guessing it out of
Optional would hide one it can. 126 positions are in that list today, and the
largest single group is `ToString()`, which reaches `System.String.Format` in
`mscorlib` and therefore proves nothing inside the registered set.

## The inventory

`tools/api_compat/reference/xna40-reference-return-nullability.json` is pinned
and hash-checked by the verifier against `returnNullabilitySha256` in
`mapping-rules.json`, exactly as the metadata contract and the accessor
inventory are. It records **only CLR facts** — no Swift spelling, no mapping
rule, no machine-local path — so it is a function of the pinned contract and
the seven registered binaries alone, and it regenerates byte-identically. The
human-readable form is `docs/generated/return-nullability-inventory.md`, which
adds the Swift column those facts imply.

```text
REFERENCE_RETURN_POSITIONS=369
  property getters 177
  methods          170
  field reads       22
  static            36     instance 333

PROVEN_NONNULL_RETURNS=128
PROVEN_NULLABLE_RETURNS=115
UNKNOWN_REFERENCE_RETURNS=126

NULLABLE_INFALLIBLE=70   NULLABLE_FALLIBLE=45
NONNULL_INFALLIBLE=62    NONNULL_FALLIBLE=66
UNKNOWN_INFALLIBLE=85    UNKNOWN_FALLIBLE=41
```

All four nullable/fallible combinations are exercised by the contract, so the
four-way matrix is measured rather than asserted.

Every one of the contract's 2,964 members is classified, and the classification
reconciles exactly:

| Category | Members |
|---|---:|
| `NOT_A_REFERENCE_RETURN` | 2326 |
| `NOT_A_REFERENCE_RETURN_CONSTRUCTOR` | 207 |
| `NOT_A_REFERENCE_RETURN_EVENT_PROJECTION` | 49 |
| `NOT_A_REFERENCE_RETURN_UNCONSTRAINED_GENERIC` | 13 |
| `REFERENCE_RETURN` | 369 |
| **total** | **2964** |

A CLR event is not a reference return position in the projected surface: it maps
to one get-only `CNAEvent<TArgs>` property, a CNA support object, and the CLR
delegate is never projected. An unconstrained generic return (`!!0`, `!0`) is
not a reference type by itself and is classified separately rather than guessed.

`UNRESOLVED_MEMBERS=0`. One body in the whole registered set could not be
decoded — `<global>::_initterm_m/2`, the C++ CRT static-initializer runner in
the three mixed-mode assemblies — and it is recorded as a hard error rather
than resolved by guessing; it returns `void` and no contract member reaches it.

## The canonical proof: `GraphicsDeviceManager.GraphicsDevice`

Re-derived from `Microsoft.Xna.Framework.Game.dll`
(`b5dffdd8125abef2a4507ba4e1d2f11062143f0a63d48fe4f298b95ad746a1f0`).

The getter is a bare field read:

```text
.method public hidebysig newslot specialname virtual final
        instance GraphicsDevice get_GraphicsDevice() cil managed
{
  IL_0000:  ldarg.0
  IL_0001:  ldfld  GraphicsDevice GraphicsDeviceManager::device
  IL_0006:  ret
}
```

No branch, no call, no `throw`: **infallible**.

The field's lifecycle proves it nullable three times over:

- `GraphicsDeviceManager..ctor(Game)` never assigns `device`. It stores
  `synchronizeWithVerticalRetrace`, `depthStencilFormat`, `backBufferWidth`,
  `backBufferHeight`, `game` and `graphicsProfile`, registers three
  `GameWindow` handlers, and returns. A manager that has just been constructed
  therefore returns **null**.
- `CreateDevice` disposes any existing device and stores `ldnull` into the
  field at IL_0014 before building the replacement.
- `Dispose` disposes the device and stores `ldnull` into the field at IL_00b0.

XNA's own code treats that null as an ordinary state rather than an error:
`IGraphicsDeviceManager.BeginDraw` and `.EndDraw` both guard the field with
`brfalse` and return quietly.

The XNA-faithful Swift shape is therefore

```swift
public var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? { get }
```

Optional and **not** throwing. `null != error`, and this member is where that is
decided.

`Game.GraphicsDevice` is the neighbouring case and lands in the fourth quadrant:
its getter resolves `IGraphicsDeviceService` from the service container,
`throw`s `InvalidOperationException` when there is none, and otherwise returns
`IGraphicsDeviceService.get_GraphicsDevice()`, whose only registered
implementor is the manager above. It is nullable **and** fallible:
`var GraphicsDevice: Graphics.GraphicsDevice? { get throws }`.

## What the runtime partials keep

Both members stay uncorrected in the implementation, deliberately.

`GraphicsDeviceManager.GraphicsDevice` is currently

```swift
public var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice {
    get throws {
        guard let game else { throw CNAError.disposedObject("GraphicsDeviceManager.Game") }
        return try game.GraphicsDevice
    }
}
```

and `Game.GraphicsDevice` borrows through `cna_game_get_graphics_device`. That
C entry point returns a `CNA_Result` and writes one `CNA_Handle`; the pinned
manifest records its result lifetime as "current callback only" and defines no
success-with-absent case. **CNA cannot presently tell "XNA has no device yet"
apart from "the native call failed."** Making either getter return `nil` would
mean catching every error and calling it absence, and making the manager's
getter infallible would mean discarding the generation and thread-ownership
checks that can genuinely fail — one fabricates XNA behaviour, the other
discards CNA's own.

So the expected shape is corrected and the implementation is not. Both members
are now measured `PROPERTY_MAPPING_MISMATCH` diagnostics on protected runtime
partials, carrying the exact sentence that says what is wrong:

```text
Microsoft.Xna.Framework.GraphicsDeviceManager.GraphicsDevice()
  XNA can normally return null from Microsoft.Xna.Framework.Graphics.GraphicsDevice
  here, so the Swift return must be Optional; expected …GraphicsDevice?, found
  …GraphicsDevice; Swift `throws` is present instead, and a normal null result is
  not a failure, so throws must not stand in for it
Microsoft.Xna.Framework.GraphicsDeviceManager.GraphicsDevice()
  CLR getter is infallible, so the Swift reader must not throw; found throws=True
  -- the CLR return is nullable and the getter is infallible, so the reader is
  `T? { get }`: a normal null must not arrive as an error
```

The architecture decision is complete; the native work it names is not, and is
not pretended to be. No CNA ABI symbol was added.

## Nullable returns on implemented types

Fifteen reference returns on the five partial types and the complete
`ResourceCreatedEventArgs` are proven nullable. Twelve of them are still
`MISSING_MEMBER` and gained only an expected shape; the two above are the
measured mismatches; the last needed no change at all.

| Type | Member | Throws | Evidence |
|---|---|---|---|
| `Game` | `GraphicsDevice` | yes | `IGraphicsDeviceService::get_GraphicsDevice` |
| `Game` | `Window` | no | `ldnull` reaches `ret` at IL_0015 |
| `Graphics.GraphicsDevice` | `Adapter` | no | the constructor leaves it null |
| `Graphics.GraphicsDevice` | `BlendState` | no | `ClearBlendState` IL_0002 |
| `Graphics.GraphicsDevice` | `DepthStencilState` | no | `ClearDepthStencilState` IL_0002 |
| `Graphics.GraphicsDevice` | `DisplayMode` | yes | no constructor assigns `_displayMode` |
| `Graphics.GraphicsDevice` | `Indices` | no | `DrawUserIndexedPrimitives` IL_018c, `set_Indices` IL_007e |
| `Graphics.GraphicsDevice` | `PresentationParameters` | no | the constructor leaves it null |
| `Graphics.GraphicsDevice` | `RasterizerState` | no | `ClearRasterizerState` IL_0002 |
| `Graphics.GraphicsDevice` | `SamplerStates` | no | no constructor assigns `pSamplerState` |
| `Graphics.GraphicsDevice` | `Textures` | no | no constructor assigns `pTextureCollection` |
| `Graphics.GraphicsDevice` | `VertexSamplerStates` | no | no constructor assigns `pVertexSamplerState` |
| `Graphics.GraphicsDevice` | `VertexTextures` | no | no constructor assigns `pVertexTextureCollection` |
| `Graphics.ResourceCreatedEventArgs` | `Resource` | no | `GraphicsDevice::FireCreatedEvent` IL_003d |
| `GraphicsDeviceManager` | `GraphicsDevice` | no | `Dispose` IL_00b0, `CreateDevice` IL_0014 |

### The one already-correct case, and why it is the good fixture

`ResourceCreatedEventArgs.Resource` was already `Any?`, because `System.Object`
maps to `Any?` — but the *reason* it is nullable is specific and worth
recording. `GraphicsDevice.FireCreatedEvent(object resource)` keeps one cached
args instance, writes the resource into `_resource` before raising the event,
and then executes

```text
IL_0036:  ldarg.0
IL_0037:  ldfld  ResourceCreatedEventArgs GraphicsDevice::createdEventArgs
IL_003c:  ldnull
IL_003d:  stfld  object ResourceCreatedEventArgs::_resource
```

once the handlers have returned, so the cached instance does not keep the
resource alive. A handler that retains the args and reads `Resource` afterwards
observes **null** through a getter that cannot fail.

The mirror case is what makes this a measured fact rather than a habit:
`FireDestroyedEvent` overwrites `_name` and `_tag` on the same cached-args path
and never nulls either afterwards, so neither `ResourceDestroyedEventArgs.Name`
nor `.Tag` is proven nullable, and neither is projected Optional on that ground.
The analysis found the asymmetry; nobody chose it.

## Audit of the complete types

All 56 reference return positions on the 118 complete types were checked
against the inventory. **No complete type's public signature was wrong**, and
none changed:

- 45 are `UNKNOWN_REFERENCE_NULLABILITY` and keep the non-Optional projection.
  40 of those are `ToString()`, which reaches `System.String.Format` outside
  the registered set.
- 10 are `PROVEN_NONNULL_SUCCESS` and were already non-Optional —
  `BoundingBox.GetCorners`, `Curve.Clone`, `Curve.Keys`, `CurveKey.Clone`,
  `CurveKeyCollection.Clone`, `CurveKeyCollection.GetEnumerator`,
  `DisplayModeCollection.GetEnumerator`, `DisplayModeCollection.Item`,
  `KeyboardState.GetPressedKeys`, `PresentationParameters.Clone`.
- 1 is `PROVEN_NULLABLE_SUCCESS` and was already Optional:
  `ResourceCreatedEventArgs.Resource`.

That the audit found nothing to correct is the result, not the absence of one:
the surface built before the rule existed turns out to agree with it everywhere
it was decidable.

## What the verifier now measures

Return nullability is a general mapping dimension, not a per-member note. The
expected Swift return type carries the Optional the pinned verdict requires,
and `compare()` names *which* of six failures a difference is rather than
reporting a bare type mismatch:

1. a nullable return projected non-Optional — and, if the Swift member throws,
   that `throws` is standing in for the null;
2. a non-null return projected Optional — and, if the CLR member is fallible
   and the Swift member does not throw, that Optional is swallowing the failure;
3. an unproven return projected Optional, reported as the deferral it breaks
   rather than as a false claim of non-nullness;
4. the Optional at the wrong generic level (`[T]?` against `[T?]`);
5. the Optional lost together with a type's qualification;
6. Optional applied to a CLR value type that is not `System.Nullable<T>`.

`T? { get throws }` collapsed to either half is reported by the accessor rule
and the nullability rule respectively, and the accessor diagnostic now says so
explicitly when the return is also nullable. Exactly one of the nullability
rule and the ordinary return comparison fires for any one member, so a single
defect is never counted twice.

New strict-report counters:

```text
REFERENCE_RETURN_PROJECTIONS=369
OPTIONAL_RETURN_PROJECTIONS=153      NONOPTIONAL_RETURN_PROJECTIONS=216
PROVEN_NULLABLE_RETURN_PROJECTIONS=115
PROVEN_NONNULL_RETURN_PROJECTIONS=128
UNKNOWN_RETURN_NULLABILITY_PROJECTIONS=126
NULLABLE_INFALLIBLE_RETURN_PROJECTIONS=70
NULLABLE_FALLIBLE_RETURN_PROJECTIONS=45
NONNULL_INFALLIBLE_RETURN_PROJECTIONS=62
NONNULL_FALLIBLE_RETURN_PROJECTIONS=66
MEASURED_RETURN_NULLABILITY_PROJECTIONS=59
PENDING_RETURN_NULLABILITY_PROJECTIONS=310
OPTIONAL_RETURN_PROJECTIONS_OBSERVED=4
```

`OPTIONAL_RETURN_PROJECTIONS` exceeds `PROVEN_NULLABLE_RETURN_PROJECTIONS` by
38 because `System.Object` returns arrive Optional from the type mapping
whatever their verdict. The report also raises a hard error if any reference
return position in the contract is absent from the pinned inventory, so the
dimension cannot become partially measured.

`ALLOWLIST_ENTRIES` stays 0 and `UNMEASURED_STRUCTURAL_CATEGORY` stays 0.

## Proof that the rules bite

- **115 self-tests** in `return_nullability.py`, run on every invocation. They
  include the lattice laws, the four-way matrix being non-empty, the scope
  reconciling to 2,964 members, a two-sided per-overload bound proving the
  arity merge decides every in-scope method's fallibility, and five mutations
  that must each flip a verdict: substituting the `ldnull` in
  `EffectParameterCollection.Item`, initialising `GraphicsDeviceManager.device`
  in the constructor and removing every later null store, storing `ldnull` into
  `BlendState.Opaque` from the type initialiser, and making every registered
  implementor of `IGraphicsDeviceService.GraphicsDevice` non-null. Mutations
  substitute instructions rather than deleting them, so a verdict changes
  because the evidence changed and not because the stack stopped balancing.
- **2,197 verifier self-tests** (was 2,182), including fifteen new synthetic
  models covering all six failure modes above plus the correct `T? throws`
  reader staying silent, the unproven return projecting non-Optional silently,
  and a plain wrong return type staying exactly one diagnostic.
- **17 Symbol Graph negative fixtures** (was 12). The five new ones cut the
  real emitted graph the way a wrong declaration would: `Resource` losing its
  `?`, `Resource` gaining a `throws`, `Name` gaining a `?`,
  `BoundingBox.GetCorners` returning `[Vector3?]`, and
  `GameTime.ElapsedGameTime` returning `Duration?`.
- **Four isolated-consumer negative compiles.** The qualification tool now
  builds four consumer programs that must *fail*: binding
  `\ResourceCreatedEventArgs.Resource` to a non-Optional key path, assigning an
  Optional return to a non-Optional binding, calling a fallible reader without
  `try`, and forming a key path to a throwing getter. A projection whose
  Optional and `throws` were decorative would let all four through.
- **14 new `PURE_XNA_DERIVED` observations** (1,752 → 1,766), including the
  cached-args null that `FireCreatedEvent` writes, the `FireDestroyedEvent`
  asymmetry, and one that keeps a nil result and a thrown failure as two
  distinct observations rather than one failure bucket.

## A latent defect found and fixed

`accessor_fallibility.method_identity` cut a method's name at the first `<` of
its last whitespace-separated token. A generic parameter list that carries a
constraint contains spaces — `DrawUserIndexedPrimitives<valuetype .ctor
(IVertexType) T>` — so 38 generic methods across `GraphicsDevice`, `Texture`,
`Texture2D`, `Texture3D`, `TextureCube`, `VertexBuffer`, `IndexBuffer`, their
dynamic subclasses, `BuiltInEffectReader<T>` and `Helpers` were all keyed under
the name `T>`. That merged unrelated methods into one bucket and dropped the
call edges their properly spelled call sites carry.

The whole balanced group is now removed before the name is taken. The pinned
accessor inventory regenerates **byte-identically** afterwards
(`5923c95d3723936ad53dce7f45c7fff74d399791ea38e3e800a96565150cac6e`), so the
defect never changed an accessor verdict — but it would have, and the
return-nullability analysis reads the same graph.

## What this milestone did not decide

- **`System.Exception` / `ExternalException`.** Untouched. Eight XNA exception
  types remain a separate public-error architecture question, because CNA
  already has `CNAError` and introducing XNA exception *types* creates a second
  error model.
- **`Collection<T>` / `ReadOnlyCollection<T>`.** Untouched, and still blocked
  on the absence of an XNA-era Microsoft `mscorlib` as a behaviour authority.
- **Reference parameters.** The selective `optionalReferenceParameters` policy
  is unchanged. Return nullability being measured does not reopen it, and no
  parameter rule was found inconsistent with the new evidence.
- **Property setters.** The Foundation 22 rule stands unchanged. Accessor
  fallibility is still per accessor, nullable setter *input* semantics are
  still independent of both setter fallibility and return nullability.
