# Color and packed-protocol evidence

## Exact dependency closure

Foundation Milestone 3 closes exactly `Microsoft.Xna.Framework.Color`,
`Microsoft.Xna.Framework.Graphics.PackedVector.IPackedVector`, and the mapped
generic protocol `IPackedVectorOfT<TPacked>`. The shape authority is the pinned
XNA 4.0 Windows runtime contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
Behavior was independently observed from the retained Microsoft
`Microsoft.Xna.Framework.dll` with SHA-256
`38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`.
No Microsoft binary is copied into this repository or its source archive.

The dependency query was regenerated from public signatures and
`directInterfaces` in the pinned contract:

| Owner | Declared members | Direct interface/signature references |
|---|---:|---|
| `Color` | 165 | `IPackedVector<UInt32>`, `Vector3`, `Vector4`, and `Color` |
| `IPackedVector<TPacked>` | 1 | base `IPackedVector`; `TPacked` storage |
| `IPackedVector` | 2 | `Vector4` |

`Vector3` and `Vector4` were already complete in Foundation Milestone 2.
Following the only new direct interface reaches the two protocol identities and
stops. No concrete packed-vector struct occurs in a Color signature, protocol
signature, base, or direct interface. Consequently Alpha8 through Short4 are
not dependencies and were not started.

## Swift protocol representation

CLR `IPackedVector<TPacked>` collides with non-generic `IPackedVector` in a
Swift scope, so the established deterministic name mapping remains
`IPackedVectorOfT<TPacked>`. Swift 6.0.3 with tools version 5.9 accepts it as a
primary-associated-type protocol:

```swift
public protocol IPackedVector {
    func ToVector4() -> Microsoft.Xna.Framework.Vector4
    mutating func PackFromVector4(_ vector: Microsoft.Xna.Framework.Vector4)
}

public protocol IPackedVectorOfT<TPacked>: IPackedVector {
    associatedtype TPacked
    var PackedValue: TPacked { get set }
}
```

`mutating` is required: XNA packed values are structs and
`PackFromVector4` replaces their packed storage. Neither protocol is
class-bound and neither uses `AnyObject`. The compiler Symbol Graph emits both
as `swift.protocol`, exposes the primary parameter as exactly `TPacked`, emits
`IPackedVectorOfT`'s conformance to `IPackedVector`, and records mutable
`PackedValue: Self.TPacked { get set }`.

Color declares `TPacked = UInt32`. The compiler emits both the generic protocol
conformance and the associated-type witness. The verifier combines those
compiler facts into the exact mapped interface
`IPackedVectorOfT<UInt32>`. XNA implements `PackFromVector4` explicitly through
the interface, while Swift requires a public conformance witness. The compiler
marks Color's method with the protocol requirement as `sourceOrigin`; that one
forced witness is a formal language projection, not a diagnostic suppression
or a 166th XNA Color member.

## Packed storage and conversion

Color stores one private `UInt32`. Public `PackedValue` is mutable and its exact
layout is `R | G<<8 | B<<16 | A<<24`. The channel properties mask only their
own byte. Independent tests set every channel and the full packed value in both
directions.

Float construction, vector construction, and `PackFromVector4` share the XNA
`PackUNorm(255, value)` algorithm. Multiplication by 255 occurs in binary32,
NaN maps to zero, infinities and finite out-of-range values clamp, and the
bounded result rounds to nearest with ties to even. Tests include signed zero,
subnormal values, exact and adjacent half-byte boundaries, `1/255`, `127/255`,
`128/255`, NaN, and both infinities. Integer constructors clamp each signed
component before unsigned conversion, including both `Int32` extremes.

`ToVector3` and `ToVector4` perform `Float(channel) / Float(255)` directly.
They do not introduce a Double division. Exact binary32 results are checked for
channels 0, 1, 2, 127, 128, 254, and 255. Returned vectors are fresh values.

## Premultiplication, interpolation, and multiplication

The Vector4 `FromNonPremultiplied` overload multiplies RGB by W in binary32
before the shared pack operation and packs W unchanged as alpha. The integer
overload performs each `Int64(rgb) * Int64(alpha) / 255` first, with signed
integer truncation toward zero, then independently clamps RGB and alpha. It is
not routed through Float. Negative values, values above 255, both integer
extremes, special floats, and alpha boundaries are qualified.

`Lerp` first packs `amount` as UNorm with a 65,536 scale, then applies XNA's
signed fixed-point channel expression
`first + (((second - first) * factor) >> 16)`. This preserves decreasing-channel
arithmetic-shift asymmetry. Amounts below zero, above one, NaN, and infinities
use XNA clamping. `Multiply` truncates the binary32 `scale * 65536` factor,
multiplies each byte, shifts by 16, and saturates each result to 255. The static
method and operator share the qualified behavior.

## Named palette and object behavior

The complete pinned palette contains 141 static Color properties, including
Transparent. Production properties contain reviewed packed constants from the
decompiled, hash-identified assembly. A separate test table was captured by
runtime reflection against that assembly. It independently names all 141
properties and validates `PackedValue`, R, G, B, and A for every entry.
Production never reads the test table. `Transparent` is exactly zero; all four
channels are zero. Struct-copy tests prove static results have no shared mutable
backing.

Equality, both `Equals` overloads, and the equality operators compare the exact
packed value. `GetHashCode` returns the signed `Int32` bit interpretation of
the packed `UInt32`; it does not use Swift randomized hashing. `ToString`
matches `{R:... G:... B:... A:...}` with XNA field order and spacing.

## Compiler and behavior matrix

| Type | Kind | Generic parameters | Inheritance | Expected | Target | Local diagnostics |
|---|---|---|---|---:|---:|---:|
| `Color` | struct | none | `IPackedVectorOfT<UInt32>` | 165 | 165 | 0 |
| `IPackedVector` | protocol | none | none | 2 | 2 | 0 |
| `IPackedVectorOfT` | protocol | `TPacked` | `IPackedVector` | 1 | 1 | 0 |

The verifier has 41 mutation/self-tests, including missing/wrong protocol kind,
inheritance, generic identity, packed type, property mutability, method
argument/return, mutating requirement, and Color's specialized conformance.
Manual and applied allowlists remain zero. The dedicated Color behavior group
has 13 test cases; the full independent palette has 141 entries. The full
`PURE_XNA_DERIVED` corpus has 415 source observations/assertions and zero
failures.

This closure is entirely managed Swift. Color retains the pre-existing private
shim-value conversion needed by deferred graphics runtime code, but none of the
new Color/protocol behavior invokes CNA and the C ABI surface is unchanged.
Concrete PackedVector formats are explicitly **not started**.
