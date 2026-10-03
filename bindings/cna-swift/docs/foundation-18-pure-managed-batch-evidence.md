# Foundation Milestone 18 — Pure Managed Batch D

Foundation 18 completes **three entirely missing pure-managed XNA types**
carrying **21 mapped Swift XNA identities**, all re-derived from registered,
hash-matched assemblies.

```text
1 Input.Touch.TouchLocation           struct  12 identities
2 Input.Touch.GestureSample           struct   7
3 Graphics.DisplayModeCollection      class    2
                                      TOTAL   21
```

No CNA source, C ABI, native binding, renderer, device, adapter, display
enumeration, touch panel, or filesystem work is included, and none of the five
runtime-partial types was touched.

## Reference provenance

`DisplayModeCollection` from `Microsoft.Xna.Framework.Graphics.dll`
(`560080fc…`), registered in Foundation 1. `TouchLocation` and `GestureSample`
from `Microsoft.Xna.Framework.Input.Touch.dll` (`b0585224…`), registered in
Foundation 17. Both assemblies re-passed the registration audit this milestone:
257 contract types and 2,964 contract members reproduced exactly, calibration
`PASS`, 60 audit self-tests `PASS`.

## `DisplayModeCollection` — a revised assessment, with the reason

Foundation 14 and 16 both deferred this type, at the top of the ranked
frontier, with the reason *"its only members enumerate `DisplayMode`, whose
instances exist solely through `GraphicsAdapter` mode enumeration, so any
implementation is permanently empty or fabricates adapter data."*

That reasoning is **wrong about this type**, and the pinned IL is what shows
it. The objection is a correct description of
`GraphicsAdapter.SupportedDisplayModes` — the *producer* — not of the
collection. The collection itself is:

```text
.class public auto ansi beforefieldinit DisplayModeCollection
       extends [mscorlib]System.Object
       implements IEnumerable`1<DisplayMode>, IEnumerable
  .field private List`1<DisplayMode> _displayModes
  .method assembly .ctor(List`1<DisplayMode> displayModes)   // stores it
```

Its only constructor is `assembly`. A consumer of the binding can therefore
**never obtain an instance at all**, so it can neither appear "permanently
empty" nor carry fabricated data. This is exactly the footing on which
`DisplayMode` itself was completed in Foundation 12: a descriptor with
non-public construction and no implemented producer. Applying one rule to
`DisplayMode` and the opposite to its collection was the inconsistency, and it
is resolved here in favour of the `DisplayMode` precedent.

Completing it claims **no** display or adapter capability. Nothing here queries
a display, enumerates an adapter, or invents a mode. `GraphicsAdapter` remains
missing and is now dependency-complete but still blocked on exactly the real
runtime capability the original objection described.

### Contract

The public contract is two identities. The explicit
`System.Collections.IEnumerable.GetEnumerator` in the IL is a private explicit
interface implementation and is not public contract.

- `GetEnumerator()` returns the backing `List<DisplayMode>` enumerator, so
  enumeration is in stored order. It maps to `CNAEnumerator<DisplayMode>`
  under the existing `IEnumerator<T>` rule. The list is never mutated after
  construction, so the CLR mutation-invalidation path that `CNAEnumerator`
  preserves can never fire for this type.
- `get_Item(SurfaceFormat)` walks the backing list once and returns a **new**
  `List<DisplayMode>` holding every mode whose `Format` equals the argument, in
  the original order. It is a fresh filtered sequence, not a live view, and an
  unmatched format yields an empty result rather than an error. Under the
  existing `IEnumerable<T>` rule it maps to `[DisplayMode]`, and being a
  read-only indexed property it maps to a Swift `subscript`.

The declared `IEnumerable<DisplayMode>` interface adds no automatic Swift
`Sequence` or `Collection` conformance, and no synthetic Microsoft collection
type is introduced. `NONPUBLIC_CONSTRUCTION_PROJECTIONS` rises from 4 to 5,
which is the counter measuring exactly this category working as intended.

## `GestureSample`

A sealed sequential value struct: six `assembly` fields, one public
six-argument constructor storing all six verbatim, six get-only property
loads. It declares **no** equality identity, no `GetHashCode` and no
`ToString`, so none is projected. `System.TimeSpan` maps to `Duration` under
the existing BCL rule.

Nothing produces a gesture: `TouchPanel` is not implemented and no touch
capability is claimed.

## `TouchLocation`

A sealed sequential value struct implementing `IEquatable<TouchLocation>`. Two
details of its storage matter and are transcribed rather than assumed:

- The position is stored as **two separate `float32` fields**, not as a
  `Vector2`. `get_Position` rebuilds a `Vector2` from them on every read.
- The current *and* the previous location live in one value: `id`, `state`,
  `x`, `y`, `prevState`, `prevX`, `prevY`. The pinned `assembly`
  seven-argument constructor that sets all seven directly is not public
  contract and is projected only as an internal initializer.

The three-argument constructor leaves the previous slots at `Invalid`/+0/+0.
The five-argument constructor sets them. Neither validates anything.

### `TryGetPreviousLocation`

```text
if (prevState == Invalid) {
    previousLocation = { id = -1, state = Invalid, x = 0, y = 0,
                         prevState = Invalid, prevX = 0, prevY = 0 };
    return false;
}
previousLocation = { id = this.id, state = this.prevState,
                     x = this.prevX,  y = this.prevY,
                     prevState = Invalid, prevX = 0, prevY = 0 };
return true;
```

Three facts here are easy to get wrong and are each asserted: the out
parameter is **written even when the method returns false**, with id `-1`; the
result carries **this** location's `id`, not a previous id; and its own
previous slots are cleared, so **a returned previous location never itself has
a previous location**. CLR `out` maps to Swift `inout`.

### The equality asymmetry

This is the type's sharpest quirk, and it is preserved rather than normalised:

| Member | Fields compared |
|---|---|
| `Equals(TouchLocation)` | `id`, `x`, `y`, `prevX`, `prevY` — **neither state** |
| `op_Equality` / `op_Inequality` | all seven, **including both states** |

So two locations differing only in `State` and `PreviousState` are `Equals` but
not `==`. `Equals(object)` is `isinst` then the typed `Equals`, so it inherits
the typed behaviour. `op_Inequality` is the exact negation of `op_Equality`
over the same seven fields.

`IEquatable<TouchLocation>` maps to the exact typed `Equals` member plus the
operators and adds no Swift `Equatable` or `Hashable` conformance.

### `GetHashCode` and `ToString`

```text
id.GetHashCode() + x.GetHashCode() + y.GetHashCode()      // unchecked int32
```

`Int32.GetHashCode()` is the value itself; `Single.GetHashCode()` on the
pinned .NET Framework is the raw bit pattern **except that both signed zeroes
hash to 0**, which is the existing `xnaFloatHash` projection. Neither state
field nor the previous position participates, and the addition wraps rather
than trapping — `(0, x: 2, y: 2)` hashes to `Int32.min`. Asserted literals:
`(7, 1, 2) -> 2139095047`, `(1, -1.5, 0.25) -> -29360127`,
`(0, 0, 0) -> 0`, `(-3, 0, -0) -> -3`, `(0, 2, 2) -> Int32.min`.

`ToString` is
`String.Format(CultureInfo.CurrentCulture, "{{Position:{0}}}", Position)`,
whose doubled braces are composite-format escapes and whose single argument is
the boxed `Vector2`'s own `ToString()`:
`(1, 2) -> "{Position:{X:1 Y:2}}"`.

## Deferred, with reasons

- **`Media.Video`** is dependency-complete and pure managed, and is deferred:
  its pinned `assembly` constructor takes a `GraphicsDevice`, which is a
  protected runtime partial, and its only producer is `ContentManager`, which
  is not implemented. Projecting it would mean either storing a reference to a
  partial or deviating from the pinned internal shape, for one reach-1 type.
- **`GraphicsAdapter`** became dependency-complete this milestone (reach 52)
  precisely because `DisplayModeCollection` completed. It is blocked on real
  adapter enumeration — `Adapters`, `DefaultAdapter`, `CurrentDisplayMode`,
  `IsProfileSupported`, `QueryBackBufferFormat` — and any implementation would
  fabricate hardware data.
- **`Input.Mouse`** became dependency-complete because `MouseState` completed.
  `Mouse.GetState()` needs a real mouse device.
- **`Media.MediaSource`** — `GetAvailableMediaSources()` needs platform
  enumeration. **`Audio.AudioCategory`** — needs the XACT engine.
  **`Media.VisualizationData`** — needs `ReadOnlyCollection<T>`.
  **`Graphics.SpriteFont`** — depends on the `Texture2D` partial.

## Verifier coverage

All three types joined the generic per-member structural mutation matrix.

One real verifier defect was found and fixed while adding them. The matrix's
`good` fixture models carried **no declaration text**, but the generic-shape
check reads `"<" in candidate.declaration`. Every member whose mapped type is
generic therefore reported a spurious `GENERIC_MAPPING_MISMATCH` against its
own reference model — the fixture was internally inconsistent, and
`DisplayModeCollection.GetEnumerator()` returning `CNAEnumerator<DisplayMode>`
is the first matrix member with a generic mapped type, so it exposed it by
failing. Fixture members now carry a declaration synthesized from their own
mapped types, which is what the compiler Symbol Graph emits.

```text
API_COMPAT_SELF_TESTS  1822 -> 1994   (+172)
```

## Structural scoreboard, start -> end

```text
                                     start    end
TARGET_TYPES                           110    113
TARGET_MEMBERS                        1620   1641   (+21)
TOTAL_DIAGNOSTICS                      298    295
COMPLETE_TYPES                         105    108
PARTIAL_TYPES                            5      5
MISSING_TYPE                           147    144
MISSING_MEMBER                         131    131   (unchanged)
NONPUBLIC_CONSTRUCTION_PROJECTIONS       4      5
```

Every mapping and safety counter is unchanged. `UNEXPECTED_TYPE`,
`UNEXPECTED_MEMBER`, `TYPE_KIND_MISMATCH`, `FIELD_MAPPING_MISMATCH`,
`METHOD_SIGNATURE_MAPPING_MISMATCH`, `PARAMETER_MAPPING_MISMATCH`,
`RETURN_MAPPING_MISMATCH`, `GENERIC_MAPPING_MISMATCH`, `ENUM_VALUE_MISMATCH`,
`FLAGS_MAPPING_MISMATCH`, `EVENT_MAPPING_MISMATCH`, `OPERATOR_MAPPING_MISMATCH`,
`REF_OUT_MAPPING_MISMATCH`, `LANGUAGE_MAPPING_MISMATCH`, `INTERNAL_TYPE_LEAK`,
`RAW_HANDLE_LEAK`, `PUBLIC_NATIVE_FFI_LEAK` and
`UNMEASURED_STRUCTURAL_CATEGORY` remain **0**. `NAMESPACE_MARKERS` stays at 10
and `LANGUAGE_PROJECTION_EXCLUSIONS` at 116: both namespaces already had their
markers. All 151 non-`MISSING_TYPE` diagnostics remain owned exclusively by the
five partials.

## Behaviour corpus

```text
PURE_XNA_DERIVED  1523 -> 1595 observations / 1595 assertions / 0 failures
```

Six new pure XNA-derived groups: `DISPLAY_MODE_COLLECTION`, `GESTURE_SAMPLE`,
`TOUCH_LOCATION_CONSTRUCTION`, `TOUCH_LOCATION_PREVIOUS`,
`TOUCH_LOCATION_EQUALITY`, `TOUCH_LOCATION_HASH_AND_STRING`. The report gains
`touchLocationContract` and `displayModeCollectionContract`, which record the
storage layout, the equality asymmetry, the hash rule and the `Item` semantics
explicitly. Swift projection qualification stays in
`Foundation18ProjectionTests` and is not counted as XNA behaviour.

## Gates

```text
SWIFT_VERSION=6.0.3
DEBUG_BUILD=PASS
RELEASE_BUILD=PASS
DEBUG_TESTS=186 PASS
RELEASE_TESTS=186 PASS
WARNINGS_AS_ERRORS=PASS_DEBUG_AND_RELEASE_INCLUDING_TESTS
SYMBOL_GRAPH=PASS
API_SELF_TESTS=1994 PASS
PINNED_ASSEMBLY_AUDIT=257/2964 CALIBRATION=PASS SELF_TESTS=60 PASS
NORMAL_STRICT=EXPECTED_RED_DEFERRED_PROFILE_ONLY
LEAK_ONLY=PASS
PURE_XNA_DERIVED=1595/1595/0
NATIVE_ABI=29/91/91/18/2/214 MISSING=0/0 MISMATCHES=0
```

Foundation 18 has zero native surface, so the native ABI report is unchanged
and was re-derived against the retained ABI-0.7.0 artifact SHA-256
`c62949d23d3745964f5e557a06665875621ed4cb6e2930e3f282afd5911f2dcb`.
