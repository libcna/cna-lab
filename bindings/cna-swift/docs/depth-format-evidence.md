# Foundation Milestone 11 — DepthFormat evidence

## Exact one-type closure and authority

Foundation Milestone 11 closes exactly one public XNA type:
`Microsoft.Xna.Framework.Graphics.DepthFormat`. No `GraphicsAdapter`,
`PresentationParameters`, render-target type, GraphicsDeviceManager member,
`DepthStencilState`, renderer operation, depth/stencil allocation, or native
route is part of this closure.

Public shape comes from the pinned Microsoft XNA Framework 4.0 Windows runtime
contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
The selected metadata entry is a sealed, non-flags CLR enum with
`System.Int32` underlying storage, no direct interfaces, and exactly five CLR
identities:

| CLR identity | Contract value |
|---|---:|
| `value__` | `System.Int32` storage |
| `None` | 0 |
| `Depth16` | 1 |
| `Depth24` | 2 |
| `Depth24Stencil8` | 3 |

Thus `SOURCE_MEMBERS=5`. The existing enum-storage language rule excludes the
synthetic CLR `value__` identity, so `EXPECTED_SWIFT_MEMBERS=4`. There is no
declared constructor, method, property, event, or operator in the selected XNA
contract.

## Swift ordinary-enum projection

The established CNA-Swift non-flags mapping produces a Swift `enum` in the
exact `Microsoft.Xna.Framework.Graphics` namespace. Its raw type is `Int32`,
and its only cases are `None=0`, `Depth16=1`, `Depth24=2`, and
`Depth24Stencil8=3`.

The compiler Symbol Graph and source supplement establish enum kind, exact
storage, non-flags status, all four literal names and values, and zero local
diagnostics. `Depth24Stencil8` is one ordinary enum literal; it is not a flag
combination, and there is no `Stencil8` literal.

The CLR storage identity `value__` is not exposed in Swift. Compiler-provided
`rawValue`, `init?(rawValue:)`, equality, and value copying are Swift language
surface and do not become XNA member identities. No DepthFormat-specific
diagnostic suppression, member omission, or raw-value exception exists.

This type does not conform to `OptionSet`, `CustomStringConvertible`,
`CustomDebugStringConvertible`, `CaseIterable`, or `Codable`. It has no
`description`, `debugDescription`, `String`, `ToString`, union, intersection,
contains, bitwise operation, depth/stencil metadata, native conversion, or
convenience helper.

## Swift mapping qualification

The separately counted Swift projection test contains the authoritative table
`[(0, .None), (1, .Depth16), (2, .Depth24), (3, .Depth24Stencil8)]` and
establishes:

- every raw initializer from 0 through 3 returns exactly its corresponding
  case;
- every case exposes the exact typed `Int32` raw value;
- `DepthFormat.None.rawValue == 0`;
- raw values 4, -1, and `Int32.max` return `nil`;
- assignment copies the enum value, and reassigning the copy leaves the
  original unchanged.

Unknown raw values returning `nil` are ordinary Swift enum semantics, not a
claim that CLR enum storage rejects unknown integers. The tests pass with
`CNA_NATIVE_LIBRARY` unset and initialize no native or graphics runtime state.

## Pure XNA-derived evidence

The compact `DEPTH_FORMAT` group records only the pinned XNA facts: enum kind,
non-flags status, `System.Int32` underlying storage, and the complete
four-value table. Kind, flags, and storage are independently enforced by the
compiler and verifier. The grouped table adds two source assertion sites to
the pure corpus without counting Swift raw initialization, unknown-value
rejection, or value-copy behavior as XNA runtime observations.

## Regenerated dependency boundary

The public-signature dependency graph was regenerated after completion.
DepthFormat directly appears in four types that remain missing:

- `Microsoft.Xna.Framework.Graphics.GraphicsAdapter`
- `Microsoft.Xna.Framework.Graphics.PresentationParameters`
- `Microsoft.Xna.Framework.Graphics.RenderTarget2D`
- `Microsoft.Xna.Framework.Graphics.RenderTargetCube`

It also directly appears in the already-partial
`Microsoft.Xna.Framework.GraphicsDeviceManager`; that type and its
`PreferredDepthStencilFormat` member remain unchanged. The transitive reverse
closure contains 54 still-missing types and all five unchanged partial types.
No dependent implementation was started.

## Structural result and strict-zero matrix

| Measurement | Result |
|---|---:|
| CLR identities | 5 |
| Expected Swift XNA identities | 4 |
| Target Swift XNA identities | 4 |
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

The 161-test verifier suite includes focused DepthFormat mutations for a
missing type, wrong namespace, struct kind, OptionSet projection, wrong raw
type, incorrect flags metadata, every wrong literal, missing middle and final
cases, renamed `Depth24Stencil8`, an incorrectly required `value__`, an
unexpected extra enum case, and public description/helper leakage.

## Native and renderer boundary

DepthFormat is managed metadata only. No CNAShim declaration, NativeManifest
row, NativeFunctions entry, C layout, callback, constant, canonical CNA source,
or ABI symbol changed. Exact ABI counts remain 29 functions, 91 prototype
positions, 91 C/Swift measurements, 18 layouts, two callbacks, and 214
constants, with zero header/library/ABI mismatches.

Capability is limited to `DepthFormat: VERIFIED_MANAGED`. Enum completeness
does not prove that CNA can allocate or use Depth16, Depth24, or
Depth24Stencil8. Depth buffers, stencil buffers, render-target depth
attachments, depth/stencil clears and testing, native format mapping, renderer
capability detection, and GPU depth/stencil support are not implemented or
claimed.
