# Geometry/intersection dependency-closure evidence

## Authoritative closure

The pinned Microsoft XNA 4.0 Windows runtime contract proves that Matrix's
complete public surface requires `Plane` through `CreateShadow` and
`CreateReflection`. Recursively traversing only public field, property,
parameter, and return types from Plane produces this exact additional closure:

| Type | Kind | Mapped members | Public XNA dependencies inside closure |
|---|---|---:|---|
| `Plane` | struct | 30 | BoundingBox, BoundingFrustum, BoundingSphere, PlaneIntersectionType |
| `PlaneIntersectionType` | enum | 3 | none |
| `Ray` | struct | 16 | BoundingBox, BoundingFrustum, BoundingSphere, Plane |
| `BoundingBox` | struct | 33 | BoundingFrustum, BoundingSphere, ContainmentType, Plane, PlaneIntersectionType, Ray |
| `BoundingSphere` | struct | 33 | BoundingBox, BoundingFrustum, ContainmentType, Plane, PlaneIntersectionType, Ray |
| `BoundingFrustum` | class | 33 | BoundingBox, BoundingSphere, ContainmentType, Plane, PlaneIntersectionType, Ray |
| `ContainmentType` | enum | 3 | none |

The only XNA dependencies outside this table are already-qualified Vector3,
Vector4, Quaternion, and Matrix. Primitive, Array, `System.Object`, and
`IEnumerable<Vector3>` signatures use formal Swift language mappings. No other
public XNA identity is required, so the Foundation-Milestone-2 correction stops
at these seven types.

## Language mappings exercised

`System.Nullable<Single>` maps to `Float?`. A nullable `out` result therefore
maps to `inout Float?`; `nil` represents `HasValue=false`, not an exception or
numeric no-hit sentinel. `IEnumerable<Vector3>` point sources map to
`[Vector3]`. Caller-owned `GetCorners(Vector3[])` storage maps to
`inout [Vector3]`, while value-returning `GetCorners()` remains a distinct
Symbol Graph identity.

## Compiler-measured closure

| Type | Expected | Emitted | Local diagnostics | Swift kind | Behavior |
|---|---:|---:|---:|---|---|
| `Vector2` | 77 | 77 | 0 | struct | PASS |
| `Vector3` | 88 | 88 | 0 | struct | PASS |
| `Vector4` | 85 | 85 | 0 | struct | PASS |
| `Quaternion` | 55 | 55 | 0 | struct | PASS |
| `Matrix` | 107 | 107 | 0 | struct | PASS |
| `Graphics.Viewport` | 14 | 14 | 0 | struct | PASS |
| `Plane` | 30 | 30 | 0 | struct | PASS |
| `PlaneIntersectionType` | 3 | 3 | 0 | enum | PASS |
| `Ray` | 16 | 16 | 0 | struct | PASS |
| `BoundingBox` | 33 | 33 | 0 | struct | PASS |
| `BoundingSphere` | 33 | 33 | 0 | struct | PASS |
| `BoundingFrustum` | 33 | 33 | 0 | class | PASS |
| `ContainmentType` | 3 | 3 | 0 | enum | PASS |

Every row has zero kind, base, interface, field, property, signature,
parameter, return, overload, generic, enum, operator, ref/out, and language
mapping diagnostics. `BoundingFrustum` intentionally preserves XNA class
reference semantics; all other non-enum additions preserve struct copy
semantics. Neither public nor private geometry code calls CNA.

## Plane and ray conventions

Plane dot operations preserve XNA Float operation order for `Vector4`,
coordinates, and normals. Matrix transformation multiplies the plane
coefficients by the inverse matrix in XNA's row-vector convention; it is not a
generic normal transform. Quaternion transformation rotates only the normal and
retains D. Unit, non-unit, zero, near-unit, nonuniform-scale, combined,
singular, and inout observations are covered. Zero normalization and singular
matrix transformation retain the reference NaN behavior.

Ray intersection distance is `Float?`: an actual hit carries distance, an
origin inside a volume carries zero, and a miss or forward-ray-ineligible hit
is `nil`. Plane parallelism and its near-zero negative-distance tolerance,
box slab handling, sphere tangent/inside/miss cases, frustum entry distance,
zero directions, and nullable inout results are qualified. No numeric sentinel
represents no hit.

## Bounds construction and ordering

`BoundingBox.GetCorners` returns the exact observable order:

1. `(Min.X, Max.Y, Max.Z)`
2. `(Max.X, Max.Y, Max.Z)`
3. `(Max.X, Min.Y, Max.Z)`
4. `(Min.X, Min.Y, Max.Z)`
5. `(Min.X, Max.Y, Min.Z)`
6. `(Max.X, Max.Y, Min.Z)`
7. `(Max.X, Min.Y, Min.Z)`
8. `(Min.X, Min.Y, Min.Z)`

Destination forms mutate only the first eight elements, preserve oversized
tails, and reject insufficient storage through the established Swift error
projection. Empty point sources fail; nonempty point factories stay in Float.
Containment keeps the XNA boundary rules and even the observable repeated
X-width check in sphere containment rather than silently correcting reference
behavior.

`BoundingSphere.CreateFromPoints` uses XNA's extreme-axis seed and expansion
pass, not a replacement minimal-enclosing-sphere package. Merge retains an
already containing sphere and otherwise expands across the center line.
Transformation moves the center through Matrix and scales the radius by the
square root of the largest squared Matrix basis-row length. Thus nonuniform and
negative/reflection scales follow the reference largest-axis rule.

## Frustum extraction and intersections

`BoundingFrustum` is an open Swift class because the pinned XNA type is a
non-sealed class. Its mutable Matrix property immediately extracts and
normalizes Near, Far, Left, Right, Top, and Bottom and recomputes all corners;
an alias observes the same replacement and no cached derived state is stale.
Plane extraction follows the qualified XNA row-vector, right-handed view and
`[0,1]` depth convention.

Corners are the intersections, in order, of near top-left, near top-right,
near bottom-right, near bottom-left, far top-left, far top-right, far
bottom-right, and far bottom-left plane triples. Exact bit patterns from an
asymmetric view/projection fixture qualify representative planes and corners;
all eight indices and both destination forms are separately checked.

Frustum/box, frustum/sphere, and frustum/frustum convex intersections use a
private scalar XNA-style GJK implementation. The helper and simplex state are
not public XNA identities and do not appear in the Symbol Graph. Plane and ray
relations use the same extracted planes. Independent inside, boundary,
intersecting, disjoint, tangent, and distant-frustum fixtures cover the complete
pairwise public relation surface.

## Equality, hash, strings, and enums

Plane, Ray, BoundingBox, and BoundingSphere equality is component-based.
BoundingFrustum equality is Matrix-based as in XNA while the object still has
class reference semantics. Explicit deterministic XNA `GetHashCode` methods use
the established component hash helpers, not Swift's randomized `hashValue`.
Signed zero, NaN, equal values, differing components, and XNA label/order string
forms are covered.

The fixed-width enum values are:

- `PlaneIntersectionType`: Front=0, Back=1, Intersecting=2.
- `ContainmentType`: Disjoint=0, Contains=1, Intersects=2.

Synthetic CLR `value__` storage is excluded only by the formal enum-storage
projection; it is not an allowlist entry.

## Qualification result

The final compiler scoreboard is 30 target types and 883 target members, with
24 complete, six unchanged partial, and 227 missing types. It contains 525
diagnostics globally, all owned by deferred work; no geometry/intersection
diagnostic remains. The verifier's 29 self-tests pass, manual and applied
allowlists are zero, deterministic language projections remain 86, array
mutation mappings are 18, unmeasured structural categories are zero, and
leak-only verification passes.

The `PURE_XNA_DERIVED` corpus grew from 187 to 360 observations/assertions with
zero failures. Debug and release each pass 27 tests. Warnings-as-errors and
Symbol Graph extraction pass. The native ABI is unchanged at 25 bound
functions, 72 prototype/type measurements, 15 layouts, two callbacks, and 168
constants with zero mismatches. Native lifecycle stress remains green and the
unchanged maintained template passes 60- and 600-frame canaries.
