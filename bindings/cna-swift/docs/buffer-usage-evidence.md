# Foundation Milestone 8 — BufferUsage evidence

## Exact one-type closure and authority

Foundation Milestone 8 closes exactly one public XNA type:
`Microsoft.Xna.Framework.Graphics.BufferUsage`. No buffer class, vertex type,
vertex declaration, GraphicsDevice member, resource operation, or native route
is part of this closure.

Public shape comes from the pinned Microsoft XNA Framework 4.0 Windows runtime
contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
The selected metadata entry is a sealed CLR enum with
`System.FlagsAttribute`, `System.Int32` underlying storage, no direct
interfaces, and exactly these identities:

| CLR identity | Contract value |
|---|---:|
| `value__` | `System.Int32` storage |
| `None` | 0 |
| `WriteOnly` | 1 |

Thus `SOURCE_MEMBERS=3`. The existing enum-storage language rule excludes the
synthetic CLR `value__` identity, so `EXPECTED_SWIFT_MEMBERS=2`. There is no
declared constructor, method, property, event, or operator in the selected XNA
contract.

## Swift OptionSet projection

The established CNA-Swift `[Flags]` mapping produces a Swift `OptionSet` in
the existing `Microsoft.Xna.Framework.Graphics` namespace. `rawValue` and
`init(rawValue:)` both use exact `Int32` storage. `None` is the empty option set
with raw value zero, and `WriteOnly` has the explicit raw value one.

The compiler Symbol Graph and source supplement jointly establish the struct
kind, `OptionSet` conformance, `Int32` raw type, and both literal values. The
verifier reports exactly two target XNA identities and zero local diagnostics.
Swift's `rawValue`, raw-value initializer, and inherited OptionSet operations
are language surface and do not become XNA member identities. No
BufferUsage-specific diagnostic allowlist, member exclusion, or raw-value
exception exists.

## Swift mapping qualification

The separately counted Swift projection test establishes:

- `BufferUsage(rawValue: 0)` is equivalent to the `None` empty bit set;
- `WriteOnly.rawValue == 1` and it contains `WriteOnly`;
- `None.union(.WriteOnly)` and idempotent `WriteOnly` union both produce 1;
- raw bit 2 unioned with `WriteOnly` produces 3;
- intersections of raw 3 and raw 2 with `WriteOnly` produce 1 and 0;
- arbitrary raw values 2, 3, `1 << 20`, and -1 retain their exact `Int32`
  patterns;
- assignment copies the value, and mutating the copy does not mutate the
  original.

These operations qualify the established Swift language projection. They are
not XNA declared members and are not counted as pure XNA runtime observations.
There is no mask to bit 0, unknown-bit rejection, zero normalization, custom
operator, helper property, `String`, `ToString`, `description`, or debug
renderer.

## Pure XNA-derived evidence

The compact `BUFFER_USAGE` group records only the pinned XNA facts: flags
status, `System.Int32` underlying storage, `None=0`, and `WriteOnly=1`. The
flags and raw-type facts are enforced by the test's compile-time generic
constraint; the two literal observations move the pure corpus from 1,243 to
1,245 observations/assertions, with zero failures.

Both BufferUsage tests pass with `CNA_NATIVE_LIBRARY` unset. They initialize no
Game, GraphicsDevice, GamePad, native function table, runtime registry, or
CNAShim route.

## Structural result and strict-zero matrix

| Measurement | Result |
|---|---:|
| CLR identities | 3 |
| Expected Swift XNA identities | 2 |
| Target Swift XNA identities | 2 |
| Local diagnostics | 0 |
| CLR kind | enum |
| Swift kind | `OptionSet` struct |
| Underlying type | `Int32` |
| Flags | true |

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

The whole-profile projection counters are unchanged. The missing type already
contributed its `value__` to the 49 enum-storage exclusions in the expected
profile, so implementation does not move that count. All manual/applied
allowlists and unmeasured structural categories remain zero.

Focused verifier mutations reject the missing type, wrong namespace, ordinary
Swift enum, wrong raw type, absent flags conformance, both wrong literals,
missing `WriteOnly`, an extra named XNA field, a public description helper, a
malformed OptionSet conformance, and an incorrectly required public `value__`.

## Native and deferred runtime boundary

BufferUsage is completely managed. No CNAShim declaration, NativeManifest
row, NativeFunctions entry, C layout, callback, constant, canonical CNA source,
or ABI symbol changed. The exact ABI counters remain 29 functions, 91
prototype positions, 91 C/Swift measurements, 18 layouts, two callbacks, and
214 constants.

Capability is limited to `BufferUsage: VERIFIED_MANAGED`. In particular, this
does not claim `VertexBuffer`, `DynamicVertexBuffer`, `IndexBuffer`,
`DynamicIndexBuffer`, `VertexBufferBinding`, `VertexDeclaration`,
`IVertexType`, built-in vertex structs, SetData/GetData, GPU allocation,
mapping, upload/download, or any GraphicsDevice buffer/draw operation. All of
those remain deferred.
