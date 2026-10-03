# Foundation 43 — `VertexDeclaration`, and the validator behind it

`VertexDeclaration` had the widest reach of any dependency-complete missing type
that could be built without touching the graphics device: 39 types wait behind
it, including `VertexBuffer`, `DynamicVertexBuffer`, `IVertexType` and the four
`VertexPositionX` structs. It is also the first type this milestone could reach
*because* of Foundation 41 — each of those four structs declares
`public static initonly VertexDeclaration VertexDeclaration`, which the contract
could not describe until `readonly` was recorded.

## 1. No native route was bound, and that is the point

CNA publishes eight `cna_vertex_declaration_*` routes and **none of them takes a
device handle**, so unlike everything in `docs/frontier-research-graphics-device-state-and-vertex-declaration.md`
a declaration has no callback-scope problem at all: it would be an ordinary
`OWNED` native object.

It is still projected as a `GraphicsResource` with **no native object**, using
the `storage: NativeHandleStorage?` shape Foundation 41 introduced. Nothing in
this type's own public surface — two constructors, `VertexStride`,
`GetVertexElements()`, `Dispose(Boolean)` — needs a handle, and
`docs/native-abi.md` forbids binding a route with no member behind it. The
routes are bound when a consumer of a declaration exists.

## 2. The constructors accept an empty element array in silence

```text
.ctor(int32 vertexStride, VertexElement[] elements)      [elements is ParamArray]
    Object::.ctor()                     <- not GraphicsResource::.ctor
    try {
        if (elements == null)   leave;  <- accepted, nothing written
        if (elements.Length==0) leave;  <- accepted, nothing written
        _elements     = (VertexElement[]) elements.Clone();
        _vertexStride = vertexStride;
        VertexElementValidator.Validate(vertexStride, _elements);
    } fault { Dispose(true); }
```

Two `leave` instructions, not an oversight: an empty array leaves `_elements`
null and `_vertexStride` **zero**, discarding the stride the caller supplied.
XNA's null case is not expressible through a Swift `[VertexElement]`, and the
empty case reproduces it exactly — both leave by the same branch before any
field is written.

That state is observable, because `GetVertexElements()` is
`ldfld _elements; callvirt Array::Clone(); castclass; ret` and dereferences the
null. So `VertexDeclaration(vertexStride: 64, elements: [])` followed by
`GetVertexElements()` raises `NullReferenceException`, and the projection throws
`CNANullReferenceException` there.

XNA wraps each constructor body in a `fault` handler that calls
`Dispose(true)` when `Validate` throws; the projection validates after
`super.init`, so a failed construction leaves a fully initialised object that
Swift deinits instead. That difference is not observable — `Disposing` cannot
have a subscriber because the initializer has not returned and no reference has
escaped, there is no native storage to release, and
`GraphicsResource.Dispose(Boolean)` does nothing else — and it is written on the
initializer rather than left for the next reader to re-derive.

The pinned fallibility record for `GetVertexElements` says
`fallible: false, IL_NO_FAILURE_PATH`, which is **wrong** — the analyser does
not treat a `callvirt` on a possibly-null field as a failure path. Nothing
depends on that record (`verify.py` reads it only for one clause of a
surplus-Optional diagnostic, and a method's Swift `throws` is not compared at
all), so the IL is followed rather than the record. The record's inaccuracy is
written up in the frontier research document as its own scoped change.

`[ParamArray]` is a custom attribute the contract's `member_key` does not read,
so the mapped parameter is a plain Swift array. A Swift variadic would have been
a `PARAMETER_MAPPING_MISMATCH`, and so was `_` — the verifier requires the CLR's
own parameter names as labels, which is how `vertexStride:` and `elements:` got
there.

## 3. The stride is the maximum end offset

`VertexElementValidator.GetTypeSize` is a switch with a default of `0`:

| | | | |
| --- | --- | --- | --- |
| `Single` 4 | `Vector2` 8 | `Vector3` 12 | `Vector4` 16 |
| `Color` 4 | `Byte4` 4 | `Short2` 4 | `Short4` 8 |
| `NormalizedShort2` 4 | `NormalizedShort4` 8 | `HalfVector2` 4 | `HalfVector4` 8 |

and `GetVertexStride` is

```csharp
int max = 0;
for (int i = 0; i < elements.Length; ++i) {
    int end = elements[i].Offset + GetTypeSize(elements[i].VertexElementFormat);
    if (max < end) max = end;
}
return max;
```

Not a sum, and not the last element's end: elements may be given in any order
and may overlap. Both of those are asserted, because summing is the obvious
wrong implementation and has its own mutation.

**Corroboration, not authority.** `build-probe/f43_stride.c` asked CNA's own
`cna_vertex_declaration_create` + `cna_vertex_declaration_get_stride` for four
layouts chosen to separate the rules:

```text
Vector3@0 + Color@12               cna=16 xna=16 AGREE
Single@16 + Vector3@0 (unordered)  cna=20 xna=20 AGREE
Vector4@0 + Single@0 (overlap)     cna=16 xna=16 AGREE
HalfVector4@8                      cna=16 xna=16 AGREE
```

CNA agrees on all four, including the two that would have exposed a sum or an
order dependency. The Swift rule still comes from the pinned IL; had CNA
disagreed, the IL would still have won and the divergence would have been
recorded the way `BlendFunction.Min`/`.Max` was.

## 4. The five validation failures, and the order that makes them observable

```text
1. vertexStride <= 0              ArgumentOutOfRangeException("vertexStride")
2. vertexStride & 3               ArgumentException(VertexElementOffsetNotMultipleFour)
   (build an owner map, one slot per byte of the vertex, all -1)
   for each element, in array order:
3. usage outside 0...12           ArgumentException(VertexElementBadUsage)
4. offset < 0 or end > stride     ArgumentException(VertexElementOutsideStride)
5. offset & 3                     ArgumentException(VertexElementOffsetNotMultipleFour)
6. duplicate usage + usageIndex   ArgumentException(DuplicateVertexElement)
7. byte already owned             ArgumentException(VertexElementsOverlap)
```

The order is observable and three tests pin it: outside-stride beats
misalignment, misalignment beats overlap, and duplicate beats overlap. The
duplicate scan compares only against **earlier** elements, so the message always
carries the later element's own identity; the overlap message names **both**,
because the map records which element owns each byte.

Four messages are read out of `Microsoft.Xna.Framework.dll`'s own embedded
string table, pinned in `xna40-selected-resource-strings.json`, and compared
against the Swift literals by the verifier —
`XNA_RESOURCE_STRING_PROJECTIONS` is 20.

**Check 3 is not one of them.** `VertexElementUsage` is a Swift enum with
thirteen cases and no way to hold a value outside `0...12`, so the type system
enforces what XNA enforces at run time and the branch is unreachable. Writing it
as dead code, or pinning a message the projection never produces, would both be
worse than saying so: `VertexElementBadUsage` is deliberately not registered,
for the same reason `VertexStrideTooSmall` — which lives in the same table and
which `Validate` never uses — is not.

## 5. `Dispose(Boolean)` reduces to the base call, shown rather than assumed

`Dispose(bool)` runs `~VertexDeclaration()` or `!VertexDeclaration()` and then
`base.Dispose(disposing)`. `~VertexDeclaration()` calls `!VertexDeclaration()`,
whose whole body is `Unbind()` — internal device-binding bookkeeping with no
Swift counterpart. Same conclusion `Texture2D` reached in Foundation 42a, and
reached the same way. A test pins that the elements survive disposal, which is
what "only unbinds" means from outside.

## 6. Falsifiability

Six new mutations, all caught:

| Planted defect |
| --- |
| the stride summed instead of maximised |
| one `VertexElementFormat` size off by a half |
| the duplicate check ignoring the usage index |
| the overlap check dropped |
| the two per-element checks in the wrong order |
| an empty element array refused where XNA accepts it |

## 7. Measurement

```text
COMPLETE_TYPES=144  (was 143)      MISSING_TYPE=106  (was 107)
TOTAL_DIAGNOSTICS=229  (was 230)   PARTIAL_TYPES=7   MISSING_MEMBER=106
PARAMETER_MAPPING_MISMATCH=0       XNA_RESOURCE_STRING_PROJECTIONS=20 (was 16)
PURE_XNA_DERIVED=2193  (was 2125)  RESOURCE_STRINGS_REPRODUCED=20 (was 16)
PROJECTION_MUTATIONS=46 CAUGHT=46 SURVIVORS=0  (was 40)
517 tests, 0 failures
```
