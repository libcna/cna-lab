# Concrete PackedVector evidence

## Closure and authority

Foundation Milestone 4 closes exactly the 17 public concrete XNA 4.0 Windows
runtime packed-vector structs. Public shape comes from the pinned 257-type,
2,964-member metadata contract. Behavior comes from the corresponding Microsoft
XNA runtime IL in `Microsoft.Xna.Framework.dll`, whose SHA-256 is
`38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`.
The retained probe source at `tools/behavior/XnaPackedVectorReferenceProbe.cs`
documents direct Windows-runtime observation inputs without retaining or
shipping any Microsoft binary. CNA, FNA, MonoGame, and sibling bindings are not
behavior authorities.

All implementation is managed scalar Swift. No type owns a native handle, no
file imports CNAShim, CNA and its C ABI are unchanged, and every value has one
private fixed-width packed field. `PackedValue` directly gets and sets that
field; no assignment normalizes or decodes it.

## Compiler surface

| Type | Expected | Target | Local diagnostics | TPacked | Public converter | Projected Swift witnesses |
|---|---:|---:|---:|---|---|---|
| `Alpha8` | 9 | 9 | 0 | `UInt8` | `ToAlpha` | `PackFromVector4`, `ToVector4` |
| `Bgr565` | 10 | 10 | 0 | `UInt16` | `ToVector3` | `PackFromVector4`, `ToVector4` |
| `Bgra4444` | 10 | 10 | 0 | `UInt16` | `ToVector4` | `PackFromVector4` |
| `Bgra5551` | 10 | 10 | 0 | `UInt16` | `ToVector4` | `PackFromVector4` |
| `Byte4` | 10 | 10 | 0 | `UInt32` | `ToVector4` | `PackFromVector4` |
| `HalfSingle` | 9 | 9 | 0 | `UInt16` | `ToSingle` | `PackFromVector4`, `ToVector4` |
| `HalfVector2` | 10 | 10 | 0 | `UInt32` | `ToVector2` | `PackFromVector4`, `ToVector4` |
| `HalfVector4` | 10 | 10 | 0 | `UInt64` | `ToVector4` | `PackFromVector4` |
| `NormalizedByte2` | 10 | 10 | 0 | `UInt16` | `ToVector2` | `PackFromVector4`, `ToVector4` |
| `NormalizedByte4` | 10 | 10 | 0 | `UInt32` | `ToVector4` | `PackFromVector4` |
| `NormalizedShort2` | 10 | 10 | 0 | `UInt32` | `ToVector2` | `PackFromVector4`, `ToVector4` |
| `NormalizedShort4` | 10 | 10 | 0 | `UInt64` | `ToVector4` | `PackFromVector4` |
| `Rg32` | 10 | 10 | 0 | `UInt32` | `ToVector2` | `PackFromVector4`, `ToVector4` |
| `Rgba1010102` | 10 | 10 | 0 | `UInt32` | `ToVector4` | `PackFromVector4` |
| `Rgba64` | 10 | 10 | 0 | `UInt64` | `ToVector4` | `PackFromVector4` |
| `Short2` | 10 | 10 | 0 | `UInt32` | `ToVector2` | `PackFromVector4`, `ToVector4` |
| `Short4` | 10 | 10 | 0 | `UInt64` | `ToVector4` | `PackFromVector4` |

The total is 168/168 mapped public members. Every row is a Swift `struct` and
directly conforms to `IPackedVectorOfT<TPacked>` with the exact primary
associated-type witness. The verifier preserves the pinned direct generic CLR
interface rather than treating transitive `IPackedVector` as another direct
interface.

## Packing matrix

All lane positions below are numeric shifts and masks; host endianness is not
observable.

| Type | Input domain and clamp | Rounding | Packed bit positions | Unpack rule |
|---|---|---|---|---|
| `Alpha8` | UNorm `[0,1]` | scaled Float, `Math.Round` ties-to-even | W `[7:0]` | W/255; protocol vector `(0,0,0,W)` |
| `Bgr565` | three UNorm `[0,1]` | ties-to-even | X `[15:11]`, Y `[10:5]`, Z `[4:0]` | X/31, Y/63, Z/31; protocol W=1 |
| `Bgra4444` | four UNorm `[0,1]` | ties-to-even | W `[15:12]`, X `[11:8]`, Y `[7:4]`, Z `[3:0]` | each lane/15 |
| `Bgra5551` | four UNorm `[0,1]` | ties-to-even | W `[15]`, X `[14:10]`, Y `[9:5]`, Z `[4:0]` | colors/31, W/1 |
| `Byte4` | raw unsigned `[0,255]` | unscaled ties-to-even | X/Y/Z/W at shifts 0/8/16/24 | raw byte values as Float |
| `HalfSingle` | XNA half scalar | bit-level ties-to-even | one UInt16 | XNA `HalfUtils.Unpack`; protocol `(X,0,0,1)` |
| `HalfVector2` | two XNA half scalars | bit-level ties-to-even | X `[15:0]`, Y `[31:16]` | per-lane half; protocol Z=0, W=1 |
| `HalfVector4` | four XNA half scalars | bit-level ties-to-even | shifts 0/16/32/48 | per-lane half |
| `NormalizedByte2` | SNorm `[-1,1]` | scaled by 127, ties-to-even | X/Y at shifts 0/8 | signed byte/127; code `0x80` special-cases to -1 |
| `NormalizedByte4` | SNorm `[-1,1]` | scaled by 127, ties-to-even | X/Y/Z/W at shifts 0/8/16/24 | signed byte/127; `0x80` -> -1 |
| `NormalizedShort2` | SNorm `[-1,1]` | scaled by 32767, ties-to-even | X/Y at shifts 0/16 | signed short/32767; `0x8000` -> -1 |
| `NormalizedShort4` | SNorm `[-1,1]` | scaled by 32767, ties-to-even | X/Y/Z/W at shifts 0/16/32/48 | signed short/32767; `0x8000` -> -1 |
| `Rg32` | two UNorm `[0,1]` | ties-to-even | X `[15:0]`, Y `[31:16]` | each lane/65535; protocol Z=0, W=1 |
| `Rgba1010102` | four UNorm `[0,1]` | ties-to-even | X `[9:0]`, Y `[19:10]`, Z `[29:20]`, W `[31:30]` | X/Y/Z divided by 1023; W/3 |
| `Rgba64` | four UNorm `[0,1]` | ties-to-even | X/Y/Z/W at shifts 0/16/32/48 | each lane/65535 |
| `Short2` | raw signed `[-32768,32767]` | unscaled ties-to-even | X/Y at shifts 0/16 | exact two's-complement Int16 as Float; protocol Z=0, W=1 |
| `Short4` | raw signed `[-32768,32767]` | unscaled ties-to-even | X/Y/Z/W at shifts 0/16/32/48 | exact two's-complement Int16 as Float |

`PackUtils` first performs any scale in binary32, then checks NaN, infinity,
and finite clamp bounds, widens the already-rounded binary32 value to Double,
and calls parameterless `System.Math.Round`, hence midpoint-to-even. NaN maps
to zero. Positive infinity maps to the maximum and negative infinity to the
minimum. For SNorm the packable minimum is -127 or -32767, so -1 packs as
`0x81` or `0x8001`; the otherwise assignable minimum codes `0x80` and `0x8000`
also decode to exactly -1. Raw signed formats instead use the full Int16 range.

`Bgra5551` therefore packs an exact W=0.5 to zero; the immediately larger
binary32 value packs W=1. `Rgba1010102` packs exact W=0.5 to lane value 2.
Byte and raw-short half-integers follow the same midpoint-to-even rule rather
than truncation or add-one-half behavior.

## Half conversion

The implementation is a transparent UInt32/UInt16 port of XNA `HalfUtils`, not
Swift `Float16`. It preserves both signs of zero, all 1,023 subnormals per sign,
normal values, and tie-to-even rounding, including the IL's pre-shift behavior
at the smallest-subnormal midpoint.

XNA's packed-half behavior is deliberately not IEEE special-value behavior:

- `+Infinity` and every positive NaN pack to `0x7FFF`;
- `-Infinity` and every negative NaN pack to `0xFFFF`;
- input NaN payloads are discarded while the sign is retained;
- `0x7C00...0x7FFF` and signed counterparts decode as finite extended values,
  not infinity/NaN; `0x7C00` decodes to 65,536 and `0x7FFF` to 131,008;
- all 65,536 UInt16 bit patterns decode to an exact Float bit pattern and
  `pack(unpack(bits)) == bits` for every pattern.

Retained edge fixtures include positive/negative zero, minimum and maximum
subnormal, minimum normal, `0x7BFF`, `0x7C00`, `0x7FFF`, infinities, signed
NaNs, normal midpoint neighbors, and the smallest-subnormal transition. The
exhaustive sweep reports 65,536 decodes, 65,536 exact repacks, and zero
failures.

## Protocol witnesses

The CLR structs implement `IPackedVector` operations privately as explicit
interface members. Swift requires public conformance witnesses. The formal
mapping records each compiler `sourceOrigin` only when all four facts hold:

1. the concrete CLR struct directly implements `IPackedVector<TPacked>`;
2. non-generic `IPackedVector` declares the requirement;
3. the member is absent from the concrete type's public declared contract;
4. the Swift compiler emits a matching public conformance witness.

There are 17 new `PackFromVector4` projections and eight new reduced-format
`ToVector4` projections, or 25 for this milestone. Together with Color the
whole-profile formal count is 26. `Alpha8` consumes W; `HalfSingle` consumes X;
`Bgr565` consumes XYZ; every two-component format consumes XY; four-component
formats consume XYZW. Reduced expansion fills are `(0,0,0,W)` for Alpha,
`(XYZ,1)` for Bgr, `(X,0,0,1)` for HalfSingle, and `(XY,0,1)` for two-component
formats.

The hardened verifier validates witness parameters, return, mutating identity,
and compiler source origin before excluding it from the concrete public-member
count. Its 47 self-tests prove that a missing witness, wrong return,
nonmutating pack operation, wrong `TPacked`, and unrelated extra public member
remain diagnostics. No broad protocol-method suppression exists.

## Equality, hash, and strings

Every format compares the canonical packed integer for typed equality,
object equality, `==`, and `!=`; decoded Float values are never compared. Half
extended patterns and different encoded payloads therefore remain distinct.

XNA primitive integer hash behavior is retained: UInt8/UInt16 widen to Int32,
UInt32 reinterprets its bits as Int32, and UInt64 XOR-folds upper and lower
32-bit halves before reinterpretation. Swift randomized `hashValue` is unused.

Alpha and all non-half multi-lane formats render uppercase invariant hex with
exact widths 2, 4, 8, or 16. `HalfSingle` renders its decoded invariant Single;
`HalfVector2` and `HalfVector4` delegate to the exact Vector2/Vector4 XNA string
shape. No CultureInfo or Design surface is introduced.

## Qualification result

The retained `PURE_XNA_DERIVED` corpus has 762 source observations/assertions
and zero failures. Independent literal fixtures cover every constructor,
component one-hot layout, midpoint neighbor, clamp endpoint, non-finite input,
direct packed assignment, converter, witness fill, equality/operator, hash,
and string family.

The declared exhaustive decode sweeps are:

| Type | Iterations | Failures |
|---|---:|---:|
| `Alpha8` | 256 | 0 |
| `Bgr565` | 65,536 | 0 |
| `Bgra4444` | 65,536 | 0 |
| `Bgra5551` | 65,536 | 0 |
| `HalfSingle` | 65,536 | 0 |

The compiler scoreboard is 49 target types and 1,198 target members: 44
complete, five unchanged runtime partials, and 208 missing. All 17 formats are
complete with zero local diagnostics. Global mismatches remain only the same
deferred runtime owners: two base, one interface, one property, and 16
overload mismatches. Manual/applied allowlists and unmeasured structural
categories remain zero. Formal language projections are 113, including 26
compiler-observed protocol-witness projections.
