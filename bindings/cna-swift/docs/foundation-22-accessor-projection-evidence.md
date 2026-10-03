# Foundation 22 — the general CLR property accessor projection

The undecided question Foundation 20 stopped on is decided. Swift property
syntax is used for exactly the accessors Swift can express; a CLR setter it
cannot express becomes a named writer method. The rule is accessor-level and
general: it covers plain properties, indexed properties, getter-only,
setter-only, throwing getters, throwing setters and throwing indexed setters
through one model, and it is the generalisation of the existing `Item`/`SetItem`
handling rather than a second mechanism beside it.

## The decision

| CLR accessors | Swift projection |
|---|---|
| infallible get, no set | `var P: T { get }` |
| infallible get, infallible set | `var P: T { get set }` |
| infallible get, **throwing set** | `var P: T { get }` + `func SetP(_ value: T) throws` |
| **throwing get**, no set | `var P: T { get throws }` |
| **throwing get**, throwing set | `var P: T { get throws }` + `func SetP(_ value: T) throws` |
| throwing get, infallible set | `var P: T { get throws }` + `func SetP(_ value: T)` |
| set only | `func SetP(_ value: T)`, `throws` per the setter |

Indexed: a read-only indexed property stays a Swift `subscript`, gaining
`get throws` when fallible. When its writer becomes a method the reader is
spelled `Item(...)` beside `SetItem(...)`, which is the established rule,
unchanged.

The rule defines twelve reader/writer combinations. The pinned contract
exercises eight of them; the other four are measured as absent, not assumed
away:

```text
plain    infallible get, no set                468   var P { get }
plain    infallible get, infallible set        166   var P { get set }
plain    infallible get, throwing set           82   var P { get } + SetP throws
plain    throwing get,   no set                 73   var P { get throws }
plain    throwing get,   throwing set           27   var P { get throws } + SetP throws
plain    throwing get,   infallible set          0   (does not occur)
plain    set only                                0   (does not occur)
indexed  infallible get, no set                 10   subscript { get }
indexed  throwing get,   no set                 10   Item(...) throws
indexed  throwing get,   throwing set            4   Item(...) throws + SetItem(...) throws
indexed  infallible get, any set                 0   (does not occur)
indexed  set only                                0   (does not occur)
```

## Four compiler facts, not four preferences

Every branch of the rule is forced by something Swift 6.0.3 actually does.
Each was established by compiling a probe, not by reading documentation.

| Probe | Result |
|---|---|
| `var X: Int { get throws { … } ; set { … } }` | **error:** `'set' accessor is not allowed on property with 'get' accessor that is 'async' or 'throws'` |
| `subscript(_ i: Int32) -> Int { get throws { … } ; set { … } }` | same error |
| `protocol P { var X: Int { get } }` witnessed by `var X: Int { get throws { … } }` | **error:** `candidate throws, but protocol does not allow it` |
| `protocol P { var X: Int { get throws } }` witnessed by a *non-throwing* getter | compiles |
| `subscript(_ i: Int32) -> Int { get throws { … } }` | compiles |
| `protocol R { var X: Int { get throws }; func SetX(_:) throws }`, used through `any R` | compiles |

The first two are why a throwing getter forces the writer out of `set` syntax
even when the setter itself cannot fail. The third and fourth are why a
fallible interface accessor must be `{ get throws }`: the throwing direction is
the one a non-throwing witness can still satisfy, and the reverse is rejected
outright, so a protocol spelled the other way could never be conformed to by an
XNA-shaped implementor.

## The accessor fallibility inventory

`tools/api_compat/accessor_fallibility.py` derives, for every public property
accessor in the contract, whether the authoritative implementation has a
contract-relevant failure path. The result is pinned as
`tools/api_compat/reference/xna40-accessor-fallibility.json` and hash-checked by
the verifier exactly as the contract is, so the verifier still runs with no
Microsoft binary present. The human-readable form is
`docs/generated/accessor-fallibility-inventory.md`.

```text
PROPERTIES=840  GETTERS=840  SETTERS=279  INDEXED=24
THROWING_GETTERS=114  THROWING_SETTERS=113
ACCESSOR_SELF_TESTS=34 PASS   UNRESOLVED_ACCESSORS=0
```

An accessor is fallible when, and only when, one of these holds:

| Evidence | Meaning |
|---|---|
| `IL_DIRECT_THROW` | the accessor body itself executes `throw`/`rethrow` |
| `IL_REACHABLE_THROW` | a throw is reachable through calls that stay inside the seven registered assemblies; the shortest chain and the constructed exception types are recorded |
| `IL_UNGUARDED_INDEX_OPERATION` | a caller-supplied index reaches a CLR index operation with no preceding conditional branch that could bypass it |
| `IL_ABSTRACT_DECLARATION` | an abstract or interface accessor whose registered implementors are fallible |

and infallible otherwise. Allocation, calls that leave the registered set,
`ldelem` on an internally computed index, and universal CLR failures such as
`OutOfMemoryException` are **not** XNA contract behaviour. Three worked
examples of the boundary:

- `BlendState.set_AlphaBlendFunction` has no throw of its own. It calls
  `ThrowIfBound`, which throws `InvalidOperationException` once the state
  object is bound to a device. **Fallible**, and the direct-throw scan that
  Foundation 20 ran would have missed all 41 such state-object setters.
- `EffectParameterCollection.Item[int]` passes its index to
  `List<EffectParameter>.get_Item` — but only after `blt`/`bge` range checks
  that return `null` instead. **Infallible.** `CurveKeyCollection.Item[int]`
  compiles to the same call with no guard at all. **Fallible.**
- `BoundingFrustum.Left` reads `planes[2]`. An array element access can throw,
  but the index is a constant and no caller can influence it. **Infallible** —
  and treating array indexing as fallibility would have wrongly condemned seven
  already-shipped `BoundingFrustum` accessors.

The one approximation is merging same-named overloads by arity. It is bounded
rather than assumed away: the *any-overload* closure is an upper bound on a
signature-precise analysis and the *all-overload* closure is a lower bound, and
a self-test requires them to agree on **every** contract accessor. They do, on
all 840, so the merge provably changed nothing.

Thirty-four self-tests hold the analysis to that, five of them mutations that
must flip a verdict: deleting `AudioEmitter`'s throw, deleting the
`ThrowIfBound` call, deleting `EffectParameterCollection`'s range check,
silencing every `IEffectFog` implementor, and poisoning one implementor of an
infallible interface accessor.

## Verifier, at accessor level

The verifier now measures getter and setter projection independently. Reader
and writer arrive as two compiler symbols and are recombined into the one
source property identity every scoreboard counts, so `EXPECTED_SWIFT_MEMBERS`
is **2887 before and 2887 after**: a projected writer is the CLR setter
accessor, not an invented XNA member. The full expected surface was regenerated
under the new rule rather than carried over.

New measured counters, computed over the whole contract rather than over what
happens to be implemented, so every branch is a number even where no type uses
it yet:

```text
ACCESSOR_PROJECTIONS=840            GETTER_ONLY_PROJECTIONS=561
INDEXED_ACCESSOR_PROJECTIONS=24     PROPERTY_SETTER_PROJECTIONS=166
THROWING_GETTER_PROJECTIONS=114     WRITER_METHOD_PROJECTIONS=113
WRITE_ONLY_PROJECTIONS=0            THROWING_WRITER_METHOD_PROJECTIONS=113
                                    INFALLIBLE_WRITER_METHOD_PROJECTIONS=0
MEASURED_ACCESSOR_PROJECTIONS=18
```

Rejected, each with its own diagnostic: a writable Swift property where the XNA
setter must throw; a missing writer method; a writer missing `throws`; a writer
throwing for an infallible setter; an ordinary `set` retained beside the writer;
a wrong writer parameter type, direction, return type or static identity; a
wrong writer name; a wrong writer mutating identity; a getter made throwing
where XNA's is infallible; and a getter left infallible where XNA's throws.
There is no allowlist and no unmeasured category.

`API_COMPAT_SELF_TESTS` rose 2,126 → 2,136 on synthetic models. Those prove the
comparison rules bite; they cannot prove the *reader* does. Twelve new
`SYMBOL_GRAPH_SELF_TESTS` close that gap by mutating the Symbol Graph the
compiler actually emitted — deleting `SetDopplerScale`, stripping its `throws`,
renaming it to `setDopplerScale` and to `TrySetDopplerScale`, retyping its
value, adding a `set` back to `DopplerScale`, making an infallible getter
throwing, making a read/write property get-only, stripping `throws` from
`IEffectFog.FogColor` and from `TouchCollection.Enumerator.Current`, and
deleting `IEffectFog.SetFogColor`. Each must introduce a diagnostic that the
unmutated graph does not already have, so a pre-existing mismatch on an
unrelated partial cannot make a fixture pass vacuously.

## AudioEmitter

`Microsoft.Xna.Framework.Audio.AudioEmitter` is the first real proof. It sits
beside the completed `AudioListener` — same file in the IL, same
`FlipHandedness` accessors, one extra property — and was blocked by exactly one
accessor.

```text
.method public hidebysig specialname instance void set_DopplerScale(float32 'value')
    IL_0000:  ldarg.1
    IL_0001:  ldc.r4     0.0
    IL_0006:  bge.un.s   IL_0018
    IL_0008:  ldstr      "value"
    IL_000d:  call       string FrameworkResources::get_InvalidEmitterDopplerScale()
    IL_0012:  newobj     instance void ArgumentOutOfRangeException::.ctor(string, string)
    IL_0017:  throw
    IL_0018:  ldarg.0
    IL_0019:  ldflda     emitterData
    IL_001e:  ldarg.1
    IL_001f:  stfld      float32 XACT_EMITTER_DATA::_DopplerScale
    IL_0024:  ret
```

`bge.un.s` is the **unordered** form: the branch past the throw is taken when
the value is `>= 0` *or when the comparison is unordered*. The throw is
therefore reached only on an ordered `value < 0`. Every boundary case is
asserted:

| Value | XNA | Why |
|---|---|---|
| `+0.0` | accepted | ordered `>= 0` |
| `-0.0` | accepted | `-0.0 >= 0.0` is ordered and true |
| `Float.leastNonzeroMagnitude` | accepted | positive subnormal |
| `+Infinity` | accepted | ordered `>= 0` |
| `NaN`, `-NaN`, NaN with payload | **accepted** and stored | comparison is unordered |
| `-Float.leastNonzeroMagnitude` | throws | smallest ordered negative |
| `-0.5`, `-1`, `-greatestFiniteMagnitude` | throws | ordered `< 0` |
| `-Infinity` | throws | ordered `< 0` |

`get_DopplerScale` is a bare field read with no validation and no flip, so it
stays an ordinary Swift property getter. The public Swift surface is therefore

```swift
public var DopplerScale: Float { get }
public func SetDopplerScale(_ value: Float) throws
```

with no writable `DopplerScale` beside it — a projection test asserts the
compiler emits only a read `KeyPath` for it and not a `ReferenceWritableKeyPath`,
which is the compiler agreeing there is no second, unchecked write path.

Swift's `<` on `Float` is the same ordered comparison and is false for NaN, so
`guard !(value < 0)` reproduces the branch exactly with no NaN special case to
get wrong. The CLR failure is `ArgumentOutOfRangeException("value",
InvalidEmitterDopplerScale)`, projected through the existing error policy as
`CNAError.argumentOutOfRange("value")`. The `throws` is `LANGUAGE_PROJECTION`;
the condition and the exception identity are `PURE_XNA_DERIVED`.

Nothing else about the type changed: the `XACT_EMITTER_DATA` storage, the
`FlipHandedness` involution and the negative-zero default `Position.Z` and
`Velocity.Z` are the same observations `AudioListener` already carries. The
constructor's `ChannelCount`, `ChannelRadius` and `CurveDistanceScaler` seeds
are not public members of the pinned type and are not projected.

## Existing public API corrected

Auditing every already-complete type against the new inventory found exactly
one defect, and it is corrected here.

**`Microsoft.Xna.Framework.Graphics.IEffectFog.FogColor`**

```diff
-var FogColor: Microsoft.Xna.Framework.Vector3 { get set }
+var FogColor: Microsoft.Xna.Framework.Vector3 { get throws }
+func SetFogColor(_ value: Microsoft.Xna.Framework.Vector3) throws
```

`FogEnabled`, `FogStart` and `FogEnd` are unchanged: all five registered
implementors back them with plain fields, so they are infallible and keep
ordinary property syntax. `FogColor` is the one accessor pair every implementor
forwards to `EffectParameter`, whose `GetValueVector3` and all eighteen
`SetValue` overloads validate and throw. This was not a style change: the old
requirement could not have been conformed to by any XNA-shaped effect, because
the compiler rejects a throwing getter as the witness of a non-throwing
requirement. `SetFogColor` is non-mutating, because the CLR setter mutates a
class and every registered implementor is a class; the test witness is a
`final class` for the same reason.

`IEffectMatrices` is unchanged — `World`, `View` and `Projection` are cached
fields in every implementor and are infallible.

No other complete type's public signature changed.

## A defect the accessor rule newly exposes

`GraphicsDeviceManager.GraphicsDevice` is projected `{ get throws }`, but the
pinned getter is a bare field read:

```text
.method ... instance GraphicsDevice get_GraphicsDevice()
    ldarg.0; ldfld GraphicsDevice GraphicsDeviceManager::device; ret
```

so XNA's getter is **infallible** and the Swift one is not. This is now a
measured `PROPERTY_MAPPING_MISMATCH` instead of an invisible difference. It is
**not** corrected here, and not because it is inconvenient: the CLR getter can
return `null`, the Swift return type is a non-optional class, and CNA has no
device to hand back before one exists. Making it non-throwing would require
either fabricating a device or trapping, both of which the error policy forbids.
The real question underneath it is the projection of a **nullable CLR reference
return**, which is a separate undecided mapping and is reported as such. The
diagnostic belongs to the `GraphicsDeviceManager` runtime partial, which is
otherwise untouched.

## Protected partials

The new expected projection applies to the 131 still-missing members of `Game`,
`GraphicsDeviceManager`, `GraphicsDevice`, `Texture2D` and `SpriteBatch`, so
their eventual native implementations now have a known writer shape — eleven of
them are writer methods. None was implemented: a managed setter for a native
device state would be fabricated behaviour. `MISSING_MEMBER` holds at **131**,
and `GraphicsDevice.Viewport` simply changed which diagnostic it carries, from
"expected mutable=True" to "requires a `SetViewport` writer method; it is
absent".

## Scoreboard

```text
                                    before   after
EXPECTED_SWIFT_MEMBERS                2887     2887
COMPLETE_TYPES                         117      118
PARTIAL_TYPES                            5        5
MISSING_TYPE                           135      134
TARGET_TYPES                           122      123
TARGET_MEMBERS                        1684     1690
TOTAL_DIAGNOSTICS                      286      286
MISSING_MEMBER                         131      131
PROPERTY_MAPPING_MISMATCH                1        2
API_COMPAT_SELF_TESTS                 2126     2136
SYMBOL_GRAPH_SELF_TESTS                  0       12
ACCESSOR_SELF_TESTS                      0       34
DEBUG_TESTS                            231      239
```

`BASE_MAPPING_MISMATCH=2`, `INTERFACE_MAPPING_MISMATCH=1` and
`OVERLOAD_MAPPING_MISMATCH=16` are unchanged, everything else is zero,
`ALLOWLIST_ENTRIES=0` and `UNMEASURED_STRUCTURAL_CATEGORY=0`. The ABI is
untouched at 29/91/91/18/2/214: this milestone is pure Swift.

`PROPERTY_MAPPING_MISMATCH` rising from 1 to 2 is the newly measured
`GraphicsDeviceManager.GraphicsDevice` getter above. Both entries belong to
runtime partials.

## The frontier after the decision

`DEPENDENCY_COMPLETE_MISSING_TYPES` is 33. The claim that the throwing-writer
decision would unblock four types was too optimistic by three: it unblocked
exactly **one**, `AudioEmitter`. `GameWindow`, `ContentManager` and `SpriteFont`
were each blocked by a *second* thing as well, and still are.

Every one of the 33 was re-checked mechanically against the decided mapping
table. Nine have no undecided BCL type left at all — `EffectAnnotation`,
`AudioCategory`, `RendererDetail`, `SoundEffectInstance`, `MediaSource`,
`FrameworkDispatcher`, `Input.Mouse`, `TouchPanel`, `TitleContainer` — and every
one of those needs a real runtime, real hardware, or a value the CLR does not
specify:

| Type | Why it is still blocked |
|---|---|
| `EffectAnnotation` | every accessor reads `ID3DXBaseEffect` |
| `AudioCategory` | `SetVolume`/`Pause`/`Resume`/`Stop` drive the XACT engine |
| `SoundEffectInstance` | wraps an XACT voice |
| `MediaSource` | `GetAvailableMediaSources` enumerates real device sources |
| `FrameworkDispatcher` | `Update` pumps audio, media and networking |
| `TitleContainer` | `OpenStream` opens real title storage |
| `Input.Mouse`, `TouchPanel` | real hardware |
| `RendererDetail` | `GetHashCode` is `_name.GetHashCode() ^ _id.GetHashCode()`, and `System.String.GetHashCode` is unspecified — reproducing a value would be fabrication |

The remaining 24 each need a BCL decision that this milestone did not make:
`System.Exception` and `ExternalException` (8 types), `System.Attribute` (5),
`ReadOnlyCollection<T>` (4), `Collection<T>`, `Dictionary<K,V>`, `System.Type`
and `IServiceProvider`, `System.ComponentModel.TypeConverter`,
`SerializationInfo`/`StreamingContext`, and `System.Action<T>`.

`GameComponentCollection` remains deferred and machine-guarded, exactly as
Foundation 21 left it. The throwing-accessor rule says nothing about the
`Collection<IGameComponent>` base it needs, and that base is where almost all of
its usable surface comes from.
