# Foundation 105 — Design converters and InstanceDescriptor

Foundation 105 closes all thirteen `Microsoft.Xna.Framework.Design` types.
Microsoft XNA 4.0 metadata remains the shape authority and its pinned IL and
resource table remain the behavior/message authority.

## Selected BCL closure

The previously measured ComponentModel base grew only by the identities the
selected Design contract reaches. The admitted .NET Framework 4.0 authorities
now cover 57 types and 706 members across `mscorlib.dll` and `System.dll`.
This milestone adds `TextInfo`, `MemberTypes`, `MemberInfo`, `MethodBase`,
`ConstructorInfo`, `ParameterInfo`, `TypeConverter.StandardValuesCollection`
and `ComponentModel.Design.Serialization.InstanceDescriptor`.

The runtime is self-contained Swift. `CNAInstanceDescriptor` holds a real
`CNAConstructorInfo`, preserves parameter metadata and arguments, and invokes
the represented constructor. No Microsoft assembly is needed at runtime and no
string-only type or constructor identity is exposed.

## Converter behavior

`MathTypeConverter` and the Point, Rectangle, Vector2, Vector3, Vector4,
Quaternion, Color, BoundingBox, BoundingSphere, Plane, Ray and Matrix
converters reproduce the selected XNA surface. String-capable converters use
the culture's decimal and list separators. Invalid component counts use the
pinned `InvalidStringFormat` resource. All converters expose ordered property
descriptors, invocable constructor descriptors and `CreateInstance` behavior.

## Evidence

The Foundation 105 suite executes eight tests with zero failures and skips.
The full debug baseline executed 986 tests with zero failures under the
project-owned isolated HOME/XDG roots and `SDL_AUDIODRIVER=dummy`. Four source
mutations were executed and caught: invalid component count accepted, ignored
culture list separator, inert instance descriptor, and reversed property
order. Survivors, hangs and unscored mutations are zero.

The live strict report is:

```text
TARGET_TYPES=251 TARGET_MEMBERS=2804
COMPLETE_TYPES=244 PARTIAL_TYPES=7 MISSING_TYPES=6
TOTAL_DIAGNOSTICS=23 MISSING_TYPE=6 MISSING_MEMBER=14
OVERLOAD_MAPPING_MISMATCH=3
ALLOWLIST_ENTRIES=0 UNMEASURED_STRUCTURAL_CATEGORY=0
```

Every disagreement and public-leak category is zero. The BCL authority audit
passes with 651 sentinel checks, 514 mutation self-tests, 287 second-
disassembler cross-checks and four rejected negative controls. Message
coverage has zero findings over 2,310 implemented members.

No native ABI route was added or changed. No dependency repository, foreign
path or user resource was modified or deleted.
