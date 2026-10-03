# Foundation Milestone 9 — FillMode evidence

## Exact one-type closure and authority

Foundation Milestone 9 closes exactly one public XNA type:
`Microsoft.Xna.Framework.Graphics.FillMode`. No `RasterizerState`,
`GraphicsDevice` member, drawing operation, renderer state, polygon mode,
wireframe implementation, or native route is part of this closure.

Public shape comes from the pinned Microsoft XNA Framework 4.0 Windows runtime
contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
The selected metadata entry is a sealed, non-flags CLR enum with
`System.Int32` underlying storage, no direct interfaces, and exactly these
identities:

| CLR identity | Contract value |
|---|---:|
| `value__` | `System.Int32` storage |
| `Solid` | 0 |
| `WireFrame` | 1 |

Thus `SOURCE_MEMBERS=3`. The existing enum-storage language rule excludes the
synthetic CLR `value__` identity, so `EXPECTED_SWIFT_MEMBERS=2`. There is no
declared constructor, method, property, event, or operator in the selected XNA
contract.

## Swift ordinary-enum projection

The established CNA-Swift non-flags mapping produces a Swift `enum` in the
existing `Microsoft.Xna.Framework.Graphics` namespace. Its raw type is exactly
`Int32`; its only cases are `Solid=0` and `WireFrame=1`, retaining XNA's exact
PascalCase spelling.

The compiler Symbol Graph and source supplement establish enum kind, `Int32`
raw storage, non-flags status, and both literal values. The verifier reports
exactly two target XNA identities and zero local diagnostics. Compiler-provided
`rawValue`, `init?(rawValue:)`, equality, and value-copy behavior are Swift
language surface and do not become XNA member identities. No FillMode-specific
diagnostic suppression, member exclusion, or raw-value exception exists.

This type does not conform to `OptionSet`. It has no union, intersection,
contains, bitwise, `HasFlag`, custom string, `description`, `debugDescription`,
`ToString`, parameterless initializer, alias, or helper API.

## Swift mapping qualification

The separately counted Swift projection test establishes:

- `Solid.rawValue == 0` and `WireFrame.rawValue == 1` using exact `Int32`;
- `FillMode(rawValue: 0) == .Solid`;
- `FillMode(rawValue: 1) == .WireFrame`;
- `FillMode(rawValue: 2) == nil`;
- `FillMode(rawValue: -1) == nil`;
- assignment copies the enum value, and reassigning the copy leaves the
  original unchanged.

Unknown raw values returning `nil` are normal Swift ordinary-enum semantics.
They are a language projection qualification, not a claim about CLR enum
storage behavior. The verifier separately proves that `OptionSet` conformance
and public string/helper surface are absent.

## Pure XNA-derived evidence

The compact `FILL_MODE` group records only the pinned XNA facts: enum kind,
non-flags status, `System.Int32` underlying storage, `Solid=0`, and
`WireFrame=1`. Kind, flags, and storage are compiler/verifier constraints; the
two literal assertions move the pure corpus from 1,245 to 1,247 observations
and assertions, with zero failures. Swift raw initialization, unknown-value
rejection, and copy behavior remain separate mapping qualification.

Both FillMode-focused tests pass with `CNA_NATIVE_LIBRARY` unset. They
initialize no `Game`, `GraphicsDevice`, `RasterizerState`, native function
table, runtime registry, or CNAShim route.

## Structural result and strict-zero matrix

| Measurement | Result |
|---|---:|
| CLR identities | 3 |
| Expected Swift XNA identities | 2 |
| Target Swift XNA identities | 2 |
| Local diagnostics | 0 |
| CLR kind | enum |
| Swift kind | enum |
| Underlying type | `Int32` |
| Flags | false |

| Local diagnostic category | Count |
|---|---:|
| `MISSING_MEMBER` | 0 |
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

The 126-test verifier suite includes focused FillMode mutations for a missing
type, wrong namespace, struct kind, OptionSet projection, wrong raw type,
incorrect flags metadata, both wrong literals, missing `WireFrame`, an
incorrectly required `value__`, an unexpected third case, and public
description/helper leakage. The flags check is a general ordinary-enum
invariant, not a FillMode suppression.

## Native and deferred runtime boundary

FillMode is completely managed. No CNAShim declaration, NativeManifest row,
NativeFunctions entry, C layout, callback, constant, canonical CNA source, or
ABI symbol changed. Exact ABI counts remain 29 functions, 91 prototype
positions, 91 C/Swift measurements, 18 layouts, two callbacks, and 214
constants, with zero header/library/ABI mismatches.

Capability is limited to `FillMode: VERIFIED_MANAGED`. `RasterizerState`,
`GraphicsDevice.RasterizerState`, drawing, wireframe rendering, backend polygon
mode, and GPU renderer support remain deferred. The existence of the
`WireFrame` enum case is not runtime rendering evidence.
