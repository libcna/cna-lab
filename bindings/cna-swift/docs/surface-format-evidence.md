# Foundation Milestone 10 — SurfaceFormat evidence

## Exact one-type closure and authority

Foundation Milestone 10 closes exactly one public XNA type:
`Microsoft.Xna.Framework.Graphics.SurfaceFormat`. No display mode, adapter,
presentation, texture, render-target, GraphicsDevice, GraphicsDeviceManager,
format conversion, renderer operation, or native route is part of this closure.

Public shape comes from the pinned Microsoft XNA Framework 4.0 Windows runtime
contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
The selected metadata entry is a sealed, non-flags CLR enum with
`System.Int32` underlying storage, no direct interfaces, and exactly 21 CLR
identities:

| CLR identity | Contract value |
|---|---:|
| `value__` | `System.Int32` storage |
| `Color` | 0 |
| `Bgr565` | 1 |
| `Bgra5551` | 2 |
| `Bgra4444` | 3 |
| `Dxt1` | 4 |
| `Dxt3` | 5 |
| `Dxt5` | 6 |
| `NormalizedByte2` | 7 |
| `NormalizedByte4` | 8 |
| `Rgba1010102` | 9 |
| `Rg32` | 10 |
| `Rgba64` | 11 |
| `Alpha8` | 12 |
| `Single` | 13 |
| `Vector2` | 14 |
| `Vector4` | 15 |
| `HalfSingle` | 16 |
| `HalfVector2` | 17 |
| `HalfVector4` | 18 |
| `HdrBlendable` | 19 |

Thus `SOURCE_MEMBERS=21`. The existing enum-storage language rule excludes
the synthetic CLR `value__` identity, so `EXPECTED_SWIFT_MEMBERS=20`. There is
no declared constructor, method, property, event, or operator in the selected
XNA contract.

## Swift ordinary-enum projection

The established CNA-Swift non-flags mapping produces a Swift `enum` in the
exact `Microsoft.Xna.Framework.Graphics` namespace. Its raw type is `Int32`,
and every case above has an explicit raw value. The compiler Symbol Graph and
source supplement establish enum kind, exact storage, non-flags status, all 20
literal names and values, and zero local diagnostics.

The CLR storage identity `value__` is not exposed in Swift. Compiler-provided
`rawValue`, `init?(rawValue:)`, equality, and value copying are Swift language
surface and do not become XNA member identities. No SurfaceFormat-specific
diagnostic suppression, member omission, or raw-value exception exists.

This type does not conform to `OptionSet`, `CustomStringConvertible`,
`CustomDebugStringConvertible`, `CaseIterable`, or `Codable`. It has no
`description`, `debugDescription`, `String`, `ToString`, union, intersection,
contains, bitwise operation, alias, format metadata, conversion, or convenience
helper. Case names shared with PackedVector types are only scoped enum cases;
the implementation has no PackedVector import, association, or conversion.

## Swift mapping qualification

The separately counted Swift projection test contains all 20 `(Int32,
SurfaceFormat)` pairs and establishes:

- every raw initializer from 0 through 19 returns exactly its corresponding
  case;
- every case exposes the exact typed `Int32` raw value;
- raw values 20, -1, and `Int32.max` return `nil`;
- assignment copies the enum value, and reassigning the copy leaves the
  original unchanged.

Unknown raw values returning `nil` are ordinary Swift enum semantics, not a
claim that CLR enum storage rejects unknown integers.

## Pure XNA-derived evidence

The compact `SURFACE_FORMAT` group records the pinned XNA facts: enum kind,
non-flags status, `System.Int32` underlying storage, and the complete 20-value
table. Kind, flags, and storage are independently enforced by the compiler and
verifier. The grouped table adds two source assertion sites to the pure corpus
without miscounting Swift raw initialization or value-copy behavior as XNA
runtime observations.

Both SurfaceFormat-focused tests pass with `CNA_NATIVE_LIBRARY` unset. They
initialize no Game, GraphicsDevice, GraphicsDeviceManager, Texture2D, native
function table, runtime registry, or CNAShim route.

## Regenerated dependency effect

The same public-signature dependency graph used for milestone selection was
regenerated after completion. SurfaceFormat has these nine direct dependents
whose types remain missing before and after Foundation 10:

- `Microsoft.Xna.Framework.Graphics.DisplayMode`
- `Microsoft.Xna.Framework.Graphics.DisplayModeCollection`
- `Microsoft.Xna.Framework.Graphics.GraphicsAdapter`
- `Microsoft.Xna.Framework.Graphics.PresentationParameters`
- `Microsoft.Xna.Framework.Graphics.RenderTarget2D`
- `Microsoft.Xna.Framework.Graphics.RenderTargetCube`
- `Microsoft.Xna.Framework.Graphics.Texture`
- `Microsoft.Xna.Framework.Graphics.Texture3D`
- `Microsoft.Xna.Framework.Graphics.TextureCube`

It also directly appears in the already-partial `Texture2D` and
`GraphicsDeviceManager` contracts; neither partial changed. The transitive
reverse closure contains 61 implemented-profile types: 56 still-missing types
and all five unchanged partials. `DisplayMode` now has only complete XNA
public-signature dependencies (`SurfaceFormat` and `Rectangle`), so it is
dependency-complete, but it was not implemented or started.

DisplayMode and DisplayModeCollection remain deferred because they form a
separate display contract with width, height, aspect-ratio, title-safe-area,
format, collection, and string behavior. GraphicsAdapter and
PresentationParameters remain deferred because they require adapter,
presentation, and platform state. Texture, Texture2D constructors, Texture3D,
TextureCube, RenderTarget2D, and RenderTargetCube remain deferred because no
format-aware storage, resource creation, mipmap, or render-target behavior was
selected. SurfaceFormat-dependent GraphicsDeviceManager and GraphicsDevice
members likewise remain deferred. No dependent implementation was started.

## Structural result and strict-zero matrix

| Measurement | Result |
|---|---:|
| CLR identities | 21 |
| Expected Swift XNA identities | 20 |
| Target Swift XNA identities | 20 |
| Local diagnostics | 0 |
| CLR kind | enum |
| Swift kind | enum |
| Underlying type | `Int32` |
| Flags | false |

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

The whole-profile formal projection counters are unchanged. The missing type
already contributed `value__` to the 49 enum-storage exclusions in the
expected profile, so implementation does not move that count. Manual/applied
allowlists and unmeasured structural categories remain zero.

The 145-test verifier suite includes focused SurfaceFormat mutations for a
missing type, wrong namespace, struct kind, OptionSet projection, wrong raw
type, incorrect flags metadata, selected low/middle/high wrong literals,
missing middle and final cases, an incorrectly required `value__`, an
unexpected extra enum case, and public description/helper leakage. The generic
enum comparison validates every one of the 20 values.

## Native and renderer boundary

SurfaceFormat is managed metadata only. No CNAShim declaration,
NativeManifest row, NativeFunctions entry, C layout, callback, constant,
canonical CNA source, or ABI symbol changed. Exact ABI counts remain 29
functions, 91 prototype positions, 91 C/Swift measurements, 18 layouts, two
callbacks, and 214 constants, with zero header/library/ABI mismatches.

Capability is limited to `SurfaceFormat: VERIFIED_MANAGED`. Enum completeness
does not imply actual CNA renderer or GPU support for any format. Full GPU
format support, DXT1/3/5, half formats, HDR blending, format negotiation,
pixel conversion, and native format mapping are not implemented or claimed.
