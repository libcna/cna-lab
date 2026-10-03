# Foundation Milestone 12 — DisplayMode evidence

## Exact one-type closure and authority

Foundation Milestone 12 closes exactly one public XNA type:
`Microsoft.Xna.Framework.Graphics.DisplayMode`. No `DisplayModeCollection`,
`GraphicsAdapter`, `GraphicsDevice.DisplayMode`, `PresentationParameters`,
monitor discovery, display enumeration, resolution switching, renderer
operation, or native route is part of this closure.

Public shape comes from the pinned Microsoft XNA Framework 4.0 Windows runtime
contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
Structure, accessibility, member closure and behavior come from the
corresponding Microsoft assembly
`Microsoft.Xna.Framework.Graphics.dll`, SHA-256
`560080fc39021c611ca9d076dcebed312faf6d7d1413c2dc523683ea635e9f55`,
whose `Microsoft.Xna.Framework.dll` companion — the assembly that owns
`Rectangle` — has the already-retained SHA-256
`38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`.
The Windows reference inputs are retained outside this source package; no
Microsoft binary or extracted proprietary source is copied into the repository
or the exact source archive.

Two derived query sources are retained:

- `tools/behavior/XnaDisplayModeReferenceProbe.cs` is the direct-runtime probe
  against the real assembly. Because the public contract declares no
  constructor, it reaches the genuine non-public `.ctor` through reflection
  rather than by widening the source contract.
- `tools/behavior/XnaDisplayModeIlSurrogate.cs` is an independently authored
  IL-equivalent re-expression of the same method bodies.

Both pinned assemblies are mixed-mode C++/CLI images whose `<Module>`
initializer requires native Windows code. Their metadata is therefore readable
on the Linux qualification host — and the reference probe's metadata half was
executed there, reproducing every structural fact recorded below — while their
managed bodies cannot execute there. The observed binary32 and string results
were consequently produced by executing the disassembled IL's exact instruction
sequence through the surrogate on a CLR. Equivalence was established by
disassembling the surrogate and comparing method bodies against the reference
IL: the constructor, all five getters, the static title-safe helper and
`ToString` match instruction for instruction, modulo compiler-chosen branch
polarity/ordering in `get_AspectRatio`, `dup` versus `stloc.0`/`ldloc.0` array
staging in `ToString`, `maxstack`, and short versus long branch encodings. None
of those alters observable behavior.

The implementation is scalar managed Swift. It imports no CNAShim API, creates
no native handle, performs no display or monitor query, and changes no CNA C
ABI entry.

## Pinned public contract

| Property | Value |
|---|---|
| Namespace | `Microsoft.Xna.Framework.Graphics` |
| CLR kind | `class` |
| Sealed | `false` |
| Base type | `System.Object` |
| Direct interfaces | none |
| Flags | not applicable |
| Public declared constructors | **0** |
| `SOURCE_MEMBERS` | 6 |
| `EXPECTED_SWIFT_MEMBERS` | 6 |
| `TARGET_SWIFT_MEMBERS` | 6 |
| `LOCAL_DIAGNOSTICS` | 0 |

The six identities are exactly `ToString`, `Format`, `Height`, `Width`,
`AspectRatio`, and `TitleSafeArea`. Every property is get-only; `ToString` is
the parameterless `System.String` override.

## Non-public construction contract

The reference IL declares three `assembly` instance fields and exactly one
`assembly` constructor:

```text
.field assembly int32 _width
.field assembly int32 _height
.field assembly valuetype Microsoft.Xna.Framework.Graphics.SurfaceFormat _format

.method assembly hidebysig specialname rtspecialname
        instance void .ctor(int32 width,
                            int32 height,
                            valuetype Microsoft.Xna.Framework.Graphics.SurfaceFormat format)
```

Its body is `base..ctor()` followed by three verbatim `stfld` stores in
declaration order. There is no argument validation, clamping, normalisation,
reordering, or derived caching: zero, negative, `Int32.MinValue` and
`Int32.MaxValue` dimensions are all stored as supplied, and an out-of-range
`SurfaceFormat` is stored as supplied too.

Reflection over the real assembly confirms the same facts independently:

```text
KIND isClass=True sealed=False abstract=False base=System.Object interfaces=0
PUBLIC_CTORS 0
CTOR assembly=True family=False private=False public=False params=3
    System.Int32:width System.Int32:height
    Microsoft.Xna.Framework.Graphics.SurfaceFormat:format
FIELD assembly=True System.Int32 _width
FIELD assembly=True System.Int32 _height
FIELD assembly=True Microsoft.Xna.Framework.Graphics.SurfaceFormat _format
```

## Swift class and construction projection

`DisplayMode` maps to a Swift `class` in the exact
`Microsoft.Xna.Framework.Graphics` namespace. It is deliberately **not `open`**
and **not `final`**:

- A CLR class whose only constructor is `assembly` accessible cannot be
  constructed or derived from outside its own assembly, so the established
  "externally subclassable CLR class → `open` where required" rule does not
  apply. `open` is not chosen mechanically from `sealed=false`.
- The metadata does say `sealed=false`, so `final` would assert a constraint
  the reference does not make. A plain Swift `public class` is subclassable
  only inside the defining module, which is exactly the CLR fact.

The Swift construction route is
`internal init(width: Int32, height: Int32, format: SurfaceFormat)`. It mirrors
the reference constructor's accessibility, arity, order and types, and it
performs the same three verbatim stores with no validation. Being `internal`,
it is invisible to the compiler-emitted public Symbol Graph and therefore adds
no XNA member identity and no `TARGET_SWIFT_MEMBERS`. No public static factory
substitutes for it, and no public initializer exists for testing convenience:
the focused tests are in-module and use the same internal route a future
framework-owned consumer would use.

This is recorded as a general mapping rule
(`nonPublicConstructionMapping`), not a DisplayMode special case, and it is
formally measured. `NONPUBLIC_CONSTRUCTION_PROJECTIONS` counts every
implemented reference class whose declared constructors are all non-public and
records the observed public Swift initializer count for each. It currently
reports four such types — `DisplayMode`, `GamePad`, `Keyboard`, and
`MathHelper` — each with zero public Swift initializers. `DisplayMode` is the
first non-sealed member of that set.

## Public state and read-only projection

`Width`, `Height` and `Format` are the three directly stored values; each
reference getter is a single `ldfld`. The Swift projection keeps `private let`
backing storage and exposes get-only computed properties, so the values are
externally immutable after construction and there is no `SetWidth`,
`SetHeight`, `SetFormat`, setter, or internal mutation path. `AspectRatio` and
`TitleSafeArea` are derived on each access.

## AspectRatio

The reference IL is a guarded binary32 division:

```text
IL_0000:  ldarg.0
IL_0001:  ldfld      int32 DisplayMode::_height
IL_0006:  brfalse.s  IL_0010
IL_0008:  ldarg.0
IL_0009:  ldfld      int32 DisplayMode::_width
IL_000e:  brtrue.s   IL_0016
IL_0010:  ldc.r4     0.0
IL_0015:  ret
IL_0016:  ldarg.0
IL_0017:  ldfld      int32 DisplayMode::_width
IL_001c:  conv.r4
IL_001d:  ldarg.0
IL_001e:  ldfld      int32 DisplayMode::_height
IL_0023:  conv.r4
IL_0024:  div
IL_0025:  ret
```

Equivalently: if `_height != 0 && _width != 0`, return
`(float)_width / (float)_height`; otherwise return `0f`. The guard is real and
was read from the IL, not assumed — it is the same shape `Viewport.AspectRatio`
uses. Consequently a zero height never produces infinity and a zero width never
produces NaN; both short-circuit to positive zero before any division. No
integer division occurs, and negative dimensions are neither clamped nor
absolute-valued: they follow ordinary signed `Single` division.

Both operands are converted with `conv.r4` before `div`, so the quotient is
computed in the binary32 domain. The Swift projection is
`Float(width) / Float(height)` and never widens to `Double` and narrows back.
`Float(Int32)` rounds to nearest-even exactly as `conv.r4` does.

## TitleSafeArea

```text
IL_0000:  ldc.i4.0
IL_0001:  ldc.i4.0
IL_0002:  ldarg.0
IL_0003:  ldfld      int32 DisplayMode::_width
IL_0008:  ldarg.0
IL_0009:  ldfld      int32 DisplayMode::_height
IL_000e:  call       Rectangle Viewport::GetTitleSafeArea(int32, int32, int32, int32)
```

and the Windows `Viewport.GetTitleSafeArea` it calls is exactly:

```text
IL_0000:  ldarg.0
IL_0001:  ldarg.1
IL_0002:  ldarg.2
IL_0003:  ldarg.3
IL_0004:  newobj     instance void Rectangle::.ctor(int32, int32, int32, int32)
```

`Rectangle`'s four-argument constructor is itself a plain four-field store.
`DisplayMode.TitleSafeArea` on the pinned Windows runtime is therefore exactly
`Rectangle(0, 0, Width, Height)` — the full display bounds, with no overscan
inset. This is the Windows implementation read directly from the reference; the
Xbox title-safe policy is not inferred, imported, or implemented, and no
percentage inset appears anywhere.

`Rectangle` is a Swift struct, so every access yields an independent value.
There is no shared mutable backing object, no cached reference, no native
state, no `GraphicsDevice` dependency, and no platform query at access time.
Mutating a retrieved rectangle cannot reach the `DisplayMode`, and the next
access returns the original rectangle again.

## ToString

The reference builds a four-element `object[]` and calls
`String.Format(IFormatProvider, string, object[])`:

```text
IL_0000:  call    CultureInfo::get_CurrentCulture()
IL_0005:  ldstr   "{{Width:{0} Height:{1} Format:{2} AspectRatio:{3}}}"
          [0] = _width          boxed System.Int32
          [1] = _height         boxed System.Int32
          [2] = get_Format()    boxed SurfaceFormat
          [3] = get_AspectRatio() boxed System.Single
IL_004a:  call    String::Format(IFormatProvider, string, object[])
```

The doubled braces are composite-format escapes, so the emitted text carries a
single brace pair. The member order is Width, Height, Format, AspectRatio; the
separator is one space; each label is followed by a colon and no space. The
boxed `SurfaceFormat` renders its declared CLR literal name, and the boxed
`Single` renders through the general "G7" format: seven significant digits,
trailing zeros removed, scientific notation with a signed two-digit exponent
once the decimal exponent leaves `[-4, 6]`.

`CultureInfo.CurrentCulture` makes only the `Single` element culture sensitive;
the labels, braces, spacing and `Int32` digits are invariant across the probed
cultures. `de-DE`, `fr-FR` and `sv-SE` render `1,777778` where the invariant
culture renders `1.777778`. CNA-Swift projects the invariant/en-US rendering
through the established narrow general-float formatter, matching XNA under
`InvariantCulture` and `en-US`; full `CultureInfo` behavior remains outside the
value milestones exactly as recorded for the existing value types.

Qualifying `DisplayMode.ToString` exposed one genuine defect in that shared
formatter: `%.7g` selects the same notation, digits and exponent width as CLR
"G7" but spells the exponent marker in lower case. `DisplayMode` is the first
qualified type whose ordinary `Int32` construction inputs reach the exponent
range, so `xnaFloatString` now normalises the marker to the CLR's upper case.
The fix is general rather than DisplayMode-specific, changes nothing in the
fixed-point domain that every previously qualified type uses, and is covered by
the retained `1.677722E+07`, `2.147484E+09` and `4.656613E-10` observations
below.

SurfaceFormat itself is untouched: it still has 21 CLR identities, 20 mapped
Swift identities, zero local diagnostics, and no public `description`,
`String()`, `ToString()`, or other string helper. The literal-name table
`DisplayMode.ToString` needs is a private static function on `DisplayMode`.

## Retained reference observations

Every row was produced by executing the reference instruction sequence under
`InvariantCulture`.

| Width | Height | `AspectRatio` bits | `TitleSafeArea` | `ToString()` |
|---:|---:|---|---|---|
| 800 | 480 | `0x3FD55555` | `(0, 0, 800, 480)` | `{Width:800 Height:480 Format:Color AspectRatio:1.666667}` |
| 1920 | 1080 | `0x3FE38E39` | `(0, 0, 1920, 1080)` | `{Width:1920 Height:1080 Format:Color AspectRatio:1.777778}` |
| 1024 | 768 | `0x3FAAAAAB` | `(0, 0, 1024, 768)` | `{Width:1024 Height:768 Format:Color AspectRatio:1.333333}` |
| 1280 | 720 | `0x3FE38E39` | `(0, 0, 1280, 720)` | `{Width:1280 Height:720 Format:Color AspectRatio:1.777778}` |
| 1366 | 768 | `0x3FE3AAAB` | `(0, 0, 1366, 768)` | `{Width:1366 Height:768 Format:Color AspectRatio:1.778646}` |
| 1600 | 900 | `0x3FE38E39` | `(0, 0, 1600, 900)` | `{Width:1600 Height:900 Format:Color AspectRatio:1.777778}` |
| 640 | 480 | `0x3FAAAAAB` | `(0, 0, 640, 480)` | `{Width:640 Height:480 Format:Color AspectRatio:1.333333}` |
| 1 | 3 | `0x3EAAAAAB` | `(0, 0, 1, 3)` | `{Width:1 Height:3 Format:Color AspectRatio:0.3333333}` |
| 1023 | 769 | `0x3FAA473E` | `(0, 0, 1023, 769)` | `{Width:1023 Height:769 Format:Color AspectRatio:1.330299}` |
| 7 | 13 | `0x3F09D89E` | `(0, 0, 7, 13)` | `{Width:7 Height:13 Format:Color AspectRatio:0.5384616}` |
| 3 | 7 | `0x3EDB6DB7` | `(0, 0, 3, 7)` | `{Width:3 Height:7 Format:Color AspectRatio:0.4285714}` |
| 16777217 | 1 | `0x4B800000` | `(0, 0, 16777217, 1)` | `{Width:16777217 Height:1 Format:Color AspectRatio:1.677722E+07}` |
| 16777217 | 3 | `0x4AAAAAAB` | `(0, 0, 16777217, 3)` | `{Width:16777217 Height:3 Format:Color AspectRatio:5592406}` |
| 0 | 480 | `0x00000000` | `(0, 0, 0, 480)` | `{Width:0 Height:480 Format:Color AspectRatio:0}` |
| 800 | 0 | `0x00000000` | `(0, 0, 800, 0)` | `{Width:800 Height:0 Format:Color AspectRatio:0}` |
| 0 | 0 | `0x00000000` | `(0, 0, 0, 0)` | `{Width:0 Height:0 Format:Color AspectRatio:0}` |
| -800 | 480 | `0xBFD55555` | `(0, 0, -800, 480)` | `{Width:-800 Height:480 Format:Color AspectRatio:-1.666667}` |
| 800 | -480 | `0xBFD55555` | `(0, 0, 800, -480)` | `{Width:800 Height:-480 Format:Color AspectRatio:-1.666667}` |
| -800 | -480 | `0x3FD55555` | `(0, 0, -800, -480)` | `{Width:-800 Height:-480 Format:Color AspectRatio:1.666667}` |
| -1 | -1 | `0x3F800000` | `(0, 0, -1, -1)` | `{Width:-1 Height:-1 Format:Color AspectRatio:1}` |
| -1 | 3 | `0xBEAAAAAB` | `(0, 0, -1, 3)` | `{Width:-1 Height:3 Format:Color AspectRatio:-0.3333333}` |
| 2147483647 | 1 | `0x4F000000` | `(0, 0, 2147483647, 1)` | `{Width:2147483647 Height:1 Format:Color AspectRatio:2.147484E+09}` |
| 1 | 2147483647 | `0x30000000` | `(0, 0, 1, 2147483647)` | `{Width:1 Height:2147483647 Format:Color AspectRatio:4.656613E-10}` |
| 2147483647 | 2147483647 | `0x3F800000` | `(0, 0, 2147483647, 2147483647)` | `{Width:2147483647 Height:2147483647 Format:Color AspectRatio:1}` |
| -2147483648 | 1 | `0xCF000000` | `(0, 0, -2147483648, 1)` | `{Width:-2147483648 Height:1 Format:Color AspectRatio:-2.147484E+09}` |
| 1 | -2147483648 | `0xB0000000` | `(0, 0, 1, -2147483648)` | `{Width:1 Height:-2147483648 Format:Color AspectRatio:-4.656613E-10}` |
| -2147483648 | -2147483648 | `0x3F800000` | `(0, 0, -2147483648, -2147483648)` | `{Width:-2147483648 Height:-2147483648 Format:Color AspectRatio:1}` |
| 2147483647 | -2147483648 | `0xBF800000` | `(0, 0, 2147483647, -2147483648)` | `{Width:2147483647 Height:-2147483648 Format:Color AspectRatio:-1}` |
| -2147483648 | 2147483647 | `0xBF800000` | `(0, 0, -2147483648, 2147483647)` | `{Width:-2147483648 Height:2147483647 Format:Color AspectRatio:-1}` |

Every pinned `SurfaceFormat` literal at 800x480:

| Raw | Literal | `ToString()` |
|---:|---|---|
| 0 | `Color` | `{Width:800 Height:480 Format:Color AspectRatio:1.666667}` |
| 1 | `Bgr565` | `{Width:800 Height:480 Format:Bgr565 AspectRatio:1.666667}` |
| 2 | `Bgra5551` | `{Width:800 Height:480 Format:Bgra5551 AspectRatio:1.666667}` |
| 3 | `Bgra4444` | `{Width:800 Height:480 Format:Bgra4444 AspectRatio:1.666667}` |
| 4 | `Dxt1` | `{Width:800 Height:480 Format:Dxt1 AspectRatio:1.666667}` |
| 5 | `Dxt3` | `{Width:800 Height:480 Format:Dxt3 AspectRatio:1.666667}` |
| 6 | `Dxt5` | `{Width:800 Height:480 Format:Dxt5 AspectRatio:1.666667}` |
| 7 | `NormalizedByte2` | `{Width:800 Height:480 Format:NormalizedByte2 AspectRatio:1.666667}` |
| 8 | `NormalizedByte4` | `{Width:800 Height:480 Format:NormalizedByte4 AspectRatio:1.666667}` |
| 9 | `Rgba1010102` | `{Width:800 Height:480 Format:Rgba1010102 AspectRatio:1.666667}` |
| 10 | `Rg32` | `{Width:800 Height:480 Format:Rg32 AspectRatio:1.666667}` |
| 11 | `Rgba64` | `{Width:800 Height:480 Format:Rgba64 AspectRatio:1.666667}` |
| 12 | `Alpha8` | `{Width:800 Height:480 Format:Alpha8 AspectRatio:1.666667}` |
| 13 | `Single` | `{Width:800 Height:480 Format:Single AspectRatio:1.666667}` |
| 14 | `Vector2` | `{Width:800 Height:480 Format:Vector2 AspectRatio:1.666667}` |
| 15 | `Vector4` | `{Width:800 Height:480 Format:Vector4 AspectRatio:1.666667}` |
| 16 | `HalfSingle` | `{Width:800 Height:480 Format:HalfSingle AspectRatio:1.666667}` |
| 17 | `HalfVector2` | `{Width:800 Height:480 Format:HalfVector2 AspectRatio:1.666667}` |
| 18 | `HalfVector4` | `{Width:800 Height:480 Format:HalfVector4 AspectRatio:1.666667}` |
| 19 | `HdrBlendable` | `{Width:800 Height:480 Format:HdrBlendable AspectRatio:1.666667}` |

An unnamed raw `SurfaceFormat` renders its integer instead of a name — the
reference produces `Format:999` and `Format:-5`. Swift's ordinary raw-value
enum mapping cannot represent an unnamed value at all, and `DisplayMode` has no
public Swift constructor through which one could be supplied, so this remains
the already-established Swift projection boundary for CLR enums. Foundation 10
is not reopened, `SurfaceFormat` is not redesigned, and no
`LANGUAGE_MAPPING_MISMATCH` is introduced.

## Swift mapping qualification

The separately counted projection tests establish, as Swift/CLR mapping facts
rather than XNA runtime observations:

- construction is reachable only inside the module, and the compiler Symbol
  Graph exposes no public `DisplayMode` init;
- the CLR class maps to Swift reference identity — `let alias = mode` aliases
  one instance, two separately constructed instances stay distinct, and no
  synthetic XNA equality is implied or available;
- public state is immutable after construction, and repeated derived-property
  reads neither disturb it nor return a cached shared object;
- `TitleSafeArea` has independent `Rectangle` value semantics;
- `AspectRatio` equals the binary32 quotient exactly, and every zero-dimension
  case yields positive zero with no NaN, infinity, or negative zero;
- the whole contract runs with `CNA_NATIVE_LIBRARY` unset.

## Structural result and strict-zero matrix

| Measurement | Result |
|---|---:|
| CLR identities | 6 |
| Expected Swift XNA identities | 6 |
| Target Swift XNA identities | 6 |
| Local diagnostics | 0 |
| CLR kind | class |
| Swift kind | class (not `open`, not `final`) |
| Base | `System.Object` language mapping |
| Public constructors | 0 |

| Local diagnostic category | Count |
|---|---:|
| `MISSING_MEMBER` | 0 |
| `UNEXPECTED_MEMBER` | 0 |
| `TYPE_KIND_MISMATCH` | 0 |
| `BASE_MAPPING_MISMATCH` | 0 |
| `INTERFACE_MAPPING_MISMATCH` | 0 |
| `FIELD_MAPPING_MISMATCH` | 0 |
| `PROPERTY_MAPPING_MISMATCH` | 0 |
| `METHOD_SIGNATURE_MAPPING_MISMATCH` | 0 |
| `PARAMETER_MAPPING_MISMATCH` | 0 |
| `RETURN_MAPPING_MISMATCH` | 0 |
| `OVERLOAD_MAPPING_MISMATCH` | 0 |
| `GENERIC_MAPPING_MISMATCH` | 0 |
| `ENUM_VALUE_MISMATCH` | 0 |
| `FLAGS_MAPPING_MISMATCH` | 0 |
| `EVENT_MAPPING_MISMATCH` | 0 |
| `OPERATOR_MAPPING_MISMATCH` | 0 |
| `REF_OUT_MAPPING_MISMATCH` | 0 |
| `LANGUAGE_MAPPING_MISMATCH` | 0 |
| leak/safety categories | 0 |

Manual and applied allowlists and unmeasured structural categories remain zero.

## Absent public surface

The pinned public contract declares none of the following for this profile, so
none is implemented. Their absence is a deliberate part of the six-identity
closure, not an omission:

- no public constructor and no public static factory;
- no property setter of any kind;
- no `Equals`, `GetHashCode`, `op_Equality`, or `op_Inequality`;
- no Swift `Equatable` or `Hashable` conformance;
- no `description`, `debugDescription`, `CustomStringConvertible`, or other
  string helper beyond the declared `ToString`;
- no convenience helper, copy constructor, `With…` builder, native format
  accessor, or renderer query.

Another implementation carrying extra `DisplayMode` API is an engineering
comparator only and was not used as authority.

## Verifier coverage

The verifier suite grew from 161 to 201 self-tests. The 34 focused DisplayMode
mutations plus six invariant checks cover a missing type, the wrong namespace,
struct/protocol/enum kinds instead of class, a wrongly mapped base, a missing
`ToString`, `ToString` with the wrong return type, wrong arity, property kind
and static identity, each of the five properties missing, each of the five
properties with the wrong type, `Format` projected as a method, `Width`
projected as static, an accidentally public initializer, extra public `Equals`,
`GetHashCode`, `op_Equality`, `op_Inequality`, `description` and convenience
helpers, and a generic sweep proving that making any one of the five
properties writable is a diagnostic. Further checks assert that the reference
model itself is diagnostic-free, that the expected member count is exactly six,
that the six identities are exactly the pinned names, that no expected property
is writable, and that no expected constructor identity exists. No manual
allowlist entry was added.

## Deferred boundary

`DisplayMode` completion is a dependency leaf only. None of the following was
implemented or started, and none is claimed:

- `Microsoft.Xna.Framework.Graphics.DisplayModeCollection` — no collection,
  indexer, enumeration, or SurfaceFormat filtering;
- `Microsoft.Xna.Framework.Graphics.GraphicsAdapter` — no
  `CurrentDisplayMode`, `SupportedDisplayModes`, `DefaultAdapter`, `Adapters`,
  monitor handle, device identifier, format query, or profile support;
- `GraphicsDevice.DisplayMode` — the existing partial `GraphicsDevice` is
  unchanged and the global `MISSING_MEMBER` count stays at 131.

No `SDL_GetCurrentDisplayMode`, monitor enumeration, window inspection,
backbuffer query, CNA call, display-mode C structure, native constant, layout,
callback, or function was added. The exact ABI remains 29 functions, 91
prototype positions, 91 C/Swift measurements, 18 layouts, two callbacks and 214
constants, with zero header, library and ABI mismatches.

Capability is limited to `DisplayMode: VERIFIED_MANAGED`. A managed descriptor
class is not platform display integration: monitor enumeration, display-mode
discovery, resolution switching, fullscreen mode management and native display
support remain unimplemented and unclaimed.
