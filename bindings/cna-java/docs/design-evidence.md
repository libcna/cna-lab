# Design converter evidence

**Updated:** 2026-08-23

**Authority:** Microsoft XNA Framework 4.0 Windows runtime metadata, IL, and direct managed-runtime observations

**Runtime:** managed Java 17; no CNA native calls

## Result

All thirteen selected `Microsoft.Xna.Framework.Design` types are implemented as one coherent
family and are locally diagnostic-free:

```text
BoundingBoxConverter     BoundingSphereConverter
ColorConverter           MathTypeConverter
MatrixConverter          PlaneConverter
PointConverter           QuaternionConverter
RayConverter             RectangleConverter
Vector2Converter         Vector3Converter
Vector4Converter
```

The Design-only strict transition was:

```text
TARGET_TYPES=251 -> 264
TARGET_MEMBERS=3150 -> 3203
TOTAL_DIAGNOSTICS=14 -> 1
MISSING_TYPE=14 -> 1
Design=13 -> 0
GamerServices=1 -> 1
```

`MISSING_MEMBER`, every mismatch/unexpected/leak/mapping category, and the allowlist remained
zero. The later separately justified `GamerServicesComponent` work reached project-wide strict
zero; see `gamerservices-evidence.md` and `plan.md`.

## Formal Java TypeConverter projection

Java has no `System.ComponentModel.TypeConverter` designer ecosystem. Adding fictional CLR
infrastructure would increase the Java type surface without supplying its semantics. The formal
projection in `docs/xna-java-mapping.md` and `tools/api-compat/mapping-rules.json` is therefore:

```text
TypeConverter / ExpandableObjectConverter base -> Object
XNA MathTypeConverter declared surface         -> Java converter base contract
System.Type                                     -> Class<?>
CultureInfo                                     -> Locale
ITypeDescriptorContext                          -> omitted parameter
PropertyDescriptorCollection metadata           -> LinkedHashMap<String, Class<?>>
GetProperties value decomposition               -> LinkedHashMap<String, Object>
InstanceDescriptor                              -> java.beans.Expression
IDictionary                                     -> Map<String, Object>
Attribute[] property filtering                  -> omitted parameter
```

The context and attribute-array parameters have no observable XNA converter semantics outside a
CLR component host: XNA passes the context to primitive converters and ignores the property
attribute filter. Their omission is an exact, verifier-enforced language mapping, not an allowlist.
No synthetic ComponentModel type was added.

XNA metadata establishes that `MathTypeConverter` is public, concrete, directly constructible,
and derives from `ExpandableObjectConverter`. Its mapped contract exposes conversion support,
create-instance support, property support, ordered property metadata, and value decomposition.

## Authoritative converter behavior

Direct XNA runtime probing corrected an important assumption in comparison implementations:
string input is enabled only for `Point`, `Vector2`, `Vector3`, `Vector4`, `Quaternion`, and
`Color`. `Rectangle`, `Matrix`, `BoundingBox`, `BoundingSphere`, `Plane`, and `Ray` deliberately
disable string input. Their conversion to `String` falls through to the CLR base behavior and is
therefore the mapped value's normal `toString()` representation, not a fabricated component list.

All concrete converters support constructor reconstruction through `Expression` and
`CreateInstance`. The exact ordered property graph is:

| Converter | Ordered properties |
|---|---|
| Point | `X`, `Y` |
| Rectangle | `X`, `Y`, `Width`, `Height` |
| Vector2 | `X`, `Y` |
| Vector3 | `X`, `Y`, `Z` |
| Vector4 | `X`, `Y`, `Z`, `W` |
| Quaternion | `X`, `Y`, `Z`, `W` |
| Color | `R`, `G`, `B`, `A` |
| Matrix | `Translation`, `M11` through `M44` |
| BoundingBox | `Min`, `Max` |
| BoundingSphere | `Center`, `Radius` |
| Plane | `Normal`, `D` |
| Ray | `Position`, `Direction` |

`Matrix.Translation` is observable in the descriptor/decomposition order, but XNA reconstruction
uses only the sixteen scalar constructor values. Nested mutable values are snapshotted when
decomposed or placed in an `Expression`, preserving CLR value-type behavior at the Java boundary.

`CreateInstance` requires every constructor property by its exact name and exact boxed Java type;
missing, null, or incompatible values fail. Unrelated extra entries are ignored because XNA reads
only its named dictionary entries. `Color` reconstructs from byte-domain `R/G/B/A` values, not a
packed integer, vector, or named-color lookup; each component is validated in `0..255`.

## String and culture behavior

The mapped culture is `Locale`; null means the Java default locale, matching CLR current-culture
selection. Decimal-comma locales use semicolon as the list separator; other locales use comma.
Formatting adds one ASCII space after each separator.

Floating converters deliberately implement CLR `Single` general formatting: seven significant
digits, culture-specific decimal and infinity tokens, scientific notation at the observed
thresholds, and both signed zeros formatted as `0`. Parsing preserves binary32 values and accepts
the observed NaN/infinity/scientific forms. Integer and byte paths reject decimal syntax and
overflow. Empty components, wrong counts, malformed values, incompatible decimal separators, and
finite overflow fail deterministically.

Focused tests cover `Locale.ROOT`, `Locale.US`, and `Locale.GERMANY`; Int32 boundaries; byte
boundaries; ordinary, negative, signed-zero, NaN, infinity, scientific, underflow-scale, and
maximum-finite float values; whitespace; wrong/missing/extra components; maps; decomposition;
nested snapshots; all constructor expressions; and round trips.

## Behavior corpus

The Design corpus contains forty stable observations derived from XNA metadata, IL, and direct
reference-runtime probes:

```text
PRIOR_OBSERVATIONS=127
DESIGN_OBSERVATIONS=40
TOTAL_OBSERVATIONS=167
FAILURES=0
```

It records base/concrete conversion flags, every property order/type, ROOT/German formatting and
parsing, binary32 special-value bits, XNA fallback string behavior, invalid conversion classes,
map reconstruction, nested snapshots, and all twelve concrete constructor expressions. It does
not use Java output as its own authority.

## Native and ownership impact

The complete Design subsystem is managed-only. At its milestone boundary it added zero CNA routes,
zero JNI callbacks, and zero owned/borrowed native handles:

```text
BOUND_FUNCTIONS=720 -> 720
```

The later three-function increase belongs solely to the independently qualified
`GamerServicesComponent` lifecycle bridge.
