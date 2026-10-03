# Foundation Milestone 15 — PresentationParameters and the general `System.IntPtr` projection

Foundation 15 completes exactly one entirely missing pure-managed XNA type,
`Microsoft.Xna.Framework.Graphics.PresentationParameters`, carrying **13 mapped
Swift XNA identities**, and formalises the general `System.IntPtr -> Swift Int`
language projection that its `DeviceWindowHandle` property is the first XNA
public signature to exercise.

No CNA source, C ABI, native binding, renderer, device, adapter, buffer,
texture, sprite-batch, effect, audio-engine, callback, thread-affinity, or
filesystem work is included, and none of the five runtime-partial types was
touched.

## Reference provenance

Public shape comes from the pinned contract SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`. The type
was independently re-derived this milestone from the pinned, hash-matched
`Microsoft.Xna.Framework.Graphics.dll`, SHA-256
`560080fc39021c611ca9d076dcebed312faf6d7d1413c2dc523683ea635e9f55`, which was
re-hashed on the qualification host and matched the retained record exactly.
The IL was read with `ikdasm` and machine-compared against the pinned contract.
No Microsoft binary or extracted proprietary source is in the repository or the
archive.

## Pinned contract, exactly as declared

```text
.class public auto ansi beforefieldinit
    Microsoft.Xna.Framework.Graphics.PresentationParameters
    extends [mscorlib]System.Object
```

- **Class kind:** class.
- **Sealed:** no. **Abstract:** no. **Base:** `System.Object`.
- **Interfaces:** none, direct or otherwise.
- **Custom attributes on the type:** none.
- **Public constructors:** exactly one, `.ctor()`, parameterless.
- **Methods:** exactly one, `Clone()`, `hidebysig instance` and **not**
  `virtual`.
- **Properties:** eleven. Ten are read/write; `Bounds` is get-only.
- **No `Clear`**, no `ToString`/`Equals`/`GetHashCode` override, no operator,
  no event, no field, and no nested public type. This is an observed absence in
  the pinned binary, not an assumption: the class body contains exactly one
  nested `assembly` type, one `assembly` field, one `.ctor`, one method,
  twenty-one accessors and eleven `.property` entries, and nothing else.

Because the CLR class is public, non-sealed, and has a public constructor, it
is externally constructible **and** externally derivable, so it maps to an
`open` Swift class. Every declared member is `hidebysig` without `virtual`, so
each maps to a `public` — never `open` — Swift member: a CLR non-virtual member
can be hidden by a derived `new` but never overridden, and a `public` member of
an `open` Swift class is likewise not overridable outside the module.

### Storage

```text
.class sequential ansi sealed nested assembly beforefieldinit Settings
       extends [mscorlib]System.ValueType
{
  .field public int32                BackBufferWidth
  .field public int32                BackBufferHeight
  .field public valuetype SurfaceFormat      BackBufferFormat
  .field public valuetype DepthFormat        DepthStencilFormat
  .field public int32                MultiSampleCount
  .field public DisplayOrientation   DisplayOrientation
  .field public valuetype PresentInterval    PresentationInterval
  .field public valuetype RenderTargetUsage  RenderTargetUsage
  .field public native int           DeviceWindowHandle
  .field public int32                IsFullScreen
}
.field assembly valuetype PresentationParameters/Settings settings
```

Both the nested struct and the field are `assembly`, so both map to `internal`
Swift and neither reaches the public Symbol Graph. The struct is transcribed
verbatim because it is what makes `Clone` a single wholesale value copy exactly
as the IL performs it, and because `IsFullScreen` is stored as `int32`, not
`bool`.

### Members and mapped identities

| # | Pinned member | CLR type | Swift projection | Mutability |
|---|---|---|---|---|
| 1 | `.ctor()` | — | `public init()` | — |
| 2 | `Clone()` | `PresentationParameters` | `public func Clone() -> PresentationParameters` | — |
| 3 | `BackBufferWidth` | `System.Int32` | `Int32` | get/set |
| 4 | `BackBufferHeight` | `System.Int32` | `Int32` | get/set |
| 5 | `BackBufferFormat` | `Graphics.SurfaceFormat` | `SurfaceFormat` | get/set |
| 6 | `DepthStencilFormat` | `Graphics.DepthFormat` | `DepthFormat` | get/set |
| 7 | `MultiSampleCount` | `System.Int32` | `Int32` | get/set |
| 8 | `DisplayOrientation` | `DisplayOrientation` | `DisplayOrientation` | get/set |
| 9 | `PresentationInterval` | `Graphics.PresentInterval` | `PresentInterval` | get/set |
| 10 | `RenderTargetUsage` | `Graphics.RenderTargetUsage` | `RenderTargetUsage` | get/set |
| 11 | `DeviceWindowHandle` | `System.IntPtr` | `Int` | get/set |
| 12 | `IsFullScreen` | `System.Boolean` | `Bool` | get/set |
| 13 | `Bounds` | `Rectangle` | `Rectangle` | **get only** |

All six XNA public-signature dependencies were strict-complete before this
milestone: `SurfaceFormat` (Foundation 10), `DepthFormat` (11),
`RenderTargetUsage` (13), `PresentInterval` (14), `DisplayOrientation`, and
`Rectangle`.

## Exact defaults, derived from IL and not guessed

```text
.method public hidebysig specialname rtspecialname instance void .ctor()
  IL_0000:  ldarg.0
  IL_0001:  call       instance void [mscorlib]System.Object::.ctor()
  IL_0006:  ldarg.0
  IL_0007:  ldc.i4.1
  IL_0008:  call       instance void PresentationParameters::set_IsFullScreen(bool)
  IL_000d:  ret
```

The constructor touches exactly one property. Every other field keeps its CLR
default, and each mapped enum's default is its own pinned zero literal.

| Property | Default | Source |
|---|---|---|
| `IsFullScreen` | **`true`** | `ldc.i4.1` in `.ctor` |
| `BackBufferWidth` | `0` | CLR `int32` default |
| `BackBufferHeight` | `0` | CLR `int32` default |
| `MultiSampleCount` | `0` | CLR `int32` default |
| `BackBufferFormat` | `.Color` | `SurfaceFormat` zero literal |
| `DepthStencilFormat` | `.None` | `DepthFormat` zero literal |
| `PresentationInterval` | `.Default` | `PresentInterval` zero literal |
| `RenderTargetUsage` | `.DiscardContents` | `RenderTargetUsage` zero literal |
| `DisplayOrientation` | `.Default` (empty set) | `[Flags]` zero literal |
| `DeviceWindowHandle` | `0` | `IntPtr.Zero` |
| `Bounds` | `Rectangle(0, 0, 0, 0)` | derived from the two zero dimensions |

`IsFullScreen` defaulting to **`true`** is the pinned literal and is
deliberately **not** the remembered MonoGame/FNA default. It is read out of
`ldc.i4.1`, and it rarely surfaces in practice only because
`GraphicsDeviceManager` assigns the property explicitly before creating a
device.

## Mutation, validation, and derived state

Every setter is `ldflda settings; ldarg.1; stfld <field>` and every getter is
the matching load. **There is no validation anywhere in the type**: not a
single comparison, range check, clamp, or `throw` appears in any accessor. Out
of range, negative, and unnamed-flag values are stored and returned verbatim,
including `Int32.min`/`Int32.max` dimensions and an unnamed `DisplayOrientation`
bit.

`IsFullScreen` is the one property whose storage type differs from its
projection:

```text
get_IsFullScreen:  ldfld int32 ...IsFullScreen; ldc.i4.0; ceq; ldc.i4.0; ceq
set_IsFullScreen:  ldarg.1; brtrue.s L; ldc.i4.0; br.s S; L: ldc.i4.1; S: stfld
```

The setter stores exactly `1` or `0`; the getter's double comparison normalises
any non-zero word to `true`. The 0/1 storage is unobservable through the public
`Bool` projection, and the transcription is exercised directly from inside the
module so the normalisation is proved rather than assumed.

`Bounds` is:

```text
get_Bounds: ldc.i4.0; ldc.i4.0;
            ldflda settings; ldfld BackBufferWidth;
            ldflda settings; ldfld BackBufferHeight;
            newobj Rectangle::.ctor(int32, int32, int32, int32)
```

so it always starts at the origin, always tracks the two back-buffer fields
live, is never cached, and never consults a device, adapter, or window.
`IsFullScreen`, the orientation, and the window handle are not read by it and
cannot influence it. Negative dimensions propagate into the rectangle
unchanged.

## Clone semantics

```text
Clone:  newobj  instance void PresentationParameters::.ctor()
        stloc.0
        ldloc.0
        ldarg.0
        ldfld   valuetype PresentationParameters/Settings ...::settings
        stfld   valuetype PresentationParameters/Settings ...::settings
        ldloc.0
        ret
```

- A fresh instance is constructed, so the constructor's `IsFullScreen = true`
  runs first.
- The whole `settings` value struct is then copied wholesale in one
  `ldfld`/`stfld` pair, which **overwrites** that `true`. The result is
  therefore an exact field-by-field copy of the source, a false
  `IsFullScreen` included.
- The copy is a value struct, so the clone is fully independent: mutating
  either side leaves the other untouched.
- `Clone` is **not** virtual and constructs `PresentationParameters` through
  `newobj` on the base constructor rather than the dynamic type, so a derived
  instance still clones to a base instance. That is pinned IL behaviour
  preserved through the Swift mapping, not a Swift limitation.
- There is no `Clear` or reset member to qualify: the pinned type declares
  none.

## `DeviceWindowHandle` and the runtime boundary

`DeviceWindowHandle` is a `native int` field with a plain load/store accessor
pair. It is qualified as **pure managed descriptor state**. The pinned managed
code never dereferences it, never compares it against a real window, and never
rejects a value; zero is `IntPtr.Zero` and carries no behaviour beyond being
the constructor default.

Completing this property authorises none of the following, and none was done:
dereferencing the value, validating it against a native window, SDL window
lookup, platform window creation, native CNA window mapping, `GraphicsDevice`
construction, device reset or recreation, or presentation backend work.

## General mapping rule: `System.IntPtr -> Swift Int`

The rule already existed in the repository — `tools/api_compat/mapping-rules.json`
carried `"System.IntPtr": "Int"` in `typeMappings`, and
`docs/xna-swift-mapping.md` carried it in the BCL table. Foundation 15 does not
invent it; it **promotes it to a documented general language rule with verifier
support and negative controls**, because this milestone is the first time an
XNA public signature actually uses it.

`System.IntPtr` maps to Swift `Int`: the opaque pointer-width signed numeric
value of the CLR IntPtr, with `IntPtr.Zero` mapping to `0`. It is a language
projection and is not a Swift pointer, a dereferenceable address, a CNA native
handle, an SDL window, a `GraphicsDevice`, proof that the handle is valid, or
proof that CNA can consume it.

`Int` is correct precisely because it follows the host pointer width rather
than fixing one, and because CLR IntPtr is signed. The public XNA projection
never exposes `UnsafeRawPointer`, `UnsafeMutableRawPointer`, or `OpaquePointer`
merely because the CLR source type is `IntPtr`, and holding an XNA IntPtr value
never requires unsafe pointer manipulation from a consumer.

### Leak policy

The expected projection is **not** counted as `RAW_HANDLE_LEAK`. The exemption
is narrow: it applies only to the mapped XNA IntPtr value, never to a CNA FFI
or native implementation handle. `RAW_HANDLE_LEAK` and
`PUBLIC_NATIVE_FFI_LEAK` both remain required at zero and both are zero.

### Negative controls

Ten distinguishing cases are asserted, each proved **twice** — once against a
synthetic owner so the rule is general, and once against the real selected
`PresentationParameters.DeviceWindowHandle` identity:

| # | Case | Required diagnostics | Forbidden |
|---|---|---|---|
| 1 | expected `IntPtr -> Int` | none | `RAW_HANDLE_LEAK`, `PUBLIC_NATIVE_FFI_LEAK`, `PROPERTY_MAPPING_MISMATCH` |
| 2 | accidental `UnsafeRawPointer` | `RAW_HANDLE_LEAK`, `PUBLIC_NATIVE_FFI_LEAK` | — |
| 3 | accidental `UnsafeMutableRawPointer` | `RAW_HANDLE_LEAK`, `PUBLIC_NATIVE_FFI_LEAK` | — |
| 4 | accidental `OpaquePointer` | `RAW_HANDLE_LEAK`, `PUBLIC_NATIVE_FFI_LEAK` | — |
| 5 | fixed-width `Int64` | `PROPERTY_MAPPING_MISMATCH` | `RAW_HANDLE_LEAK` |
| 6 | unsigned `UInt` | `PROPERTY_MAPPING_MISMATCH` | `RAW_HANDLE_LEAK` |
| 7 | `CNA_Handle` wrapper | `PUBLIC_NATIVE_FFI_LEAK`, `PROPERTY_MAPPING_MISMATCH` | — |
| 8 | `CNASwift_WindowHandle` wrapper | `PUBLIC_NATIVE_FFI_LEAK`, `PROPERTY_MAPPING_MISMATCH` | — |
| 9 | `nativeHandle` projection | `RAW_HANDLE_LEAK`, `PUBLIC_NATIVE_FFI_LEAK`, `PROPERTY_MAPPING_MISMATCH` | — |
| 10 | `SDL_Window` wrapper | `PROPERTY_MAPPING_MISMATCH` | — |

Case 10 is included deliberately: a platform wrapper whose *name* falls outside
the FFI naming pattern still cannot pass, because the mapping check rejects it
on type identity alone. Three further fixtures reject invented public members
that would cross the descriptor boundary — `GetWindow`,
`ResolveDeviceWindow`, and `NativeWindowHandle` — as `UNEXPECTED_MEMBER`.

The fixtures were mutation-proved live against the verifier itself. Removing
`OpaquePointer` from the leak pattern makes cases 4 fail in both the synthetic
and the real anchoring; changing `"System.IntPtr"` to `"Int64"` in
`mapping-rules.json` makes cases 1 and 5 fail in both. Neither regression can
pass silently.

## Verifier coverage

`PresentationParameters` was added to the generic per-member structural
mutation matrix, which is built from its own pinned reference model, so no
member can be renamed, retyped, made read-only, made writable, made static, or
dropped without a diagnostic. In particular the matrix proves that `Bounds`
must stay get-only and that the other ten properties must stay settable.

```text
API_COMPAT_SELF_TESTS  1259 -> 1396   (+137)
```

## Structural scoreboard, start -> end

```text
                              start    end
TARGET_TYPES                     98     99
TARGET_MEMBERS                 1560   1573   (+13)
TOTAL_DIAGNOSTICS               310    309
COMPLETE_TYPES                   93     94
PARTIAL_TYPES                     5      5
MISSING_TYPE                    159    158
MISSING_MEMBER                  131    131   (unchanged)
```

Every other structural, mapping, and safety counter is byte-identical:

```text
REFERENCE_TYPES=257 REFERENCE_MEMBERS=2964
EXPECTED_SWIFT_TYPES=257 EXPECTED_SWIFT_MEMBERS=2887
UNEXPECTED_TYPE=0 UNEXPECTED_MEMBER=0 TYPE_KIND_MISMATCH=0
BASE_MAPPING_MISMATCH=2 INTERFACE_MAPPING_MISMATCH=1
FIELD_MAPPING_MISMATCH=0 PROPERTY_MAPPING_MISMATCH=1
METHOD_SIGNATURE_MAPPING_MISMATCH=0 PARAMETER_MAPPING_MISMATCH=0
RETURN_MAPPING_MISMATCH=0 OVERLOAD_MAPPING_MISMATCH=16
GENERIC_MAPPING_MISMATCH=0 ENUM_VALUE_MISMATCH=0 FLAGS_MAPPING_MISMATCH=0
EVENT_MAPPING_MISMATCH=0 OPERATOR_MAPPING_MISMATCH=0 REF_OUT_MAPPING_MISMATCH=0
LANGUAGE_MAPPING_MISMATCH=0
INTERNAL_TYPE_LEAK=0 RAW_HANDLE_LEAK=0 PUBLIC_NATIVE_FFI_LEAK=0
UNMEASURED_STRUCTURAL_CATEGORY=0
ALLOWLIST_ENTRIES=0 APPLIED_ALLOWLIST_ENTRIES=0
LANGUAGE_PROJECTION_EXCLUSIONS=114 ENUM_STORAGE_FIELD_EXCLUSIONS=49
FINALIZER_LANGUAGE_MAPPINGS=28 NAMESPACE_MARKERS=8
INHERITED_MEMBER_PROJECTIONS=3 PROTOCOL_WITNESS_MEMBER_PROJECTIONS=26
ARRAY_MUTATION_MAPPINGS=19 COMPARABLE_INTERFACE_PROJECTIONS=1
COLLECTION_INTERFACE_PROJECTIONS=1 ENUMERATOR_SUPPORT_PROJECTIONS=9
INDEXED_PROPERTY_ACCESSOR_PROJECTIONS=4 GLOBAL_OPTIONAL_OPERATOR_PROJECTIONS=2
NONPUBLIC_CONSTRUCTION_PROJECTIONS=4
```

`NONPUBLIC_CONSTRUCTION_PROJECTIONS` stays at 4 because
`PresentationParameters` declares a *public* constructor and is therefore
outside that category. `NAMESPACE_MARKERS` stays at 8 because
`Microsoft.Xna.Framework.Graphics` already had its marker.

The four diagnostics whose subject mentions `PresentationParameters` are all
owned by the protected partial `GraphicsDevice` — its two `Reset` overloads,
its three-argument constructor, and its `PresentationParameters` property.
They were already part of that type's 57 diagnostics and 54 missing members
before this milestone, and none of the five partials was touched.
`PresentationParameters` itself carries zero diagnostics.

## Behaviour corpus

```text
PURE_XNA_DERIVED  1348 -> 1443 observations / 1443 assertions / 0 failures
```

Six new pure XNA-derived groups, all in `PureValueTests`:

```text
PRESENTATION_PARAMETERS_DEFAULTS
PRESENTATION_PARAMETERS_MUTATION
PRESENTATION_PARAMETERS_IS_FULL_SCREEN
PRESENTATION_PARAMETERS_BOUNDS
PRESENTATION_PARAMETERS_DEVICE_WINDOW_HANDLE
PRESENTATION_PARAMETERS_CLONE
```

Swift projection qualification stays in a separate
`PresentationParametersProjectionTests` suite and is deliberately **not**
counted as XNA behaviour. It covers public construction and open derivation,
CLR reference identity, the `IntPtr -> Int` projection type identity, every
mapped property's static Swift type, and the internal `Settings`
transcription.

## Swift 6.0.3 compiler workaround

`swift-frontend` 6.0.3 aborts in SILGen — `SILGenLValue.cpp` assertion
`addr.getType().isAddress() && "resolving lvalue did not give an address"` in
`emitAddressOfLValue` — when `type(of:)` is applied to a stored property of an
internal value struct reached through a class property *inside an XCTAssert
autoclosure*. Binding the value to a local `let` first is the documented
workaround; it does not weaken the identity check. This is a second, distinct
Swift 6.0.3 workaround alongside the Foundation 14 nested-existential one, and
both are retained.

## Gates

```text
SWIFT_VERSION=6.0.3
SWIFT_TARGET=x86_64-pc-linux-gnu
DEBUG_BUILD=PASS
RELEASE_BUILD=PASS
DEBUG_TESTS=158 PASS
RELEASE_TESTS=158 PASS
MANAGED_TESTS=148 PASS_WITHOUT_CNA_NATIVE_LIBRARY
WARNINGS_AS_ERRORS=PASS_DEBUG_AND_RELEASE
SYMBOL_GRAPH=PASS
API_SELF_TESTS=1396 PASS
NORMAL_STRICT=EXPECTED_RED_DEFERRED_PROFILE_ONLY
LEAK_ONLY=PASS
PURE_XNA_DERIVED=1443/1443/0
NATIVE_ABI=29/91/91/18/2/214 MISSING=0/0 MISMATCHES=0
```

Foundation 15 has zero native surface, so the native ABI report is unchanged on
disk and was re-derived against the retained ABI-0.7.0 artifact SHA-256
`c62949d23d3745964f5e557a06665875621ed4cb6e2930e3f282afd5911f2dcb`.

## Not done, deliberately

Dependency completion is not permission to cross the runtime boundary. None of
the following was implemented, and `GraphicsDevice` remains an untouched
partial: `GraphicsDevice` constructors, `GraphicsDevice.Reset`,
`GraphicsDevice.PresentationParameters`, adapter selection, back-buffer
creation, native window handling, presentation interval behaviour, and
render-target creation.
