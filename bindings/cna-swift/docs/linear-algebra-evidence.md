# Binary32 linear-algebra evidence

## Closure and authority

Foundation Milestone 2 implements the complete pinned XNA 4.0 Windows runtime
contracts for `Vector2`, `Vector3`, `Vector4`, `Quaternion`, `Matrix`, and
`Graphics.Viewport`. Shape is measured from the retained metadata contract and
the Swift compiler Symbol Graph. Behavior is scalar managed Swift qualified
against the retained `PURE_XNA_DERIVED` observations; no CNA math entry point,
native handle, SIMD representation, or runtime-generated implementation is
used.

| Type | Expected | Emitted | Local diagnostics |
|---|---:|---:|---:|
| `Vector2` | 77 | 77 | 0 |
| `Vector3` | 88 | 88 | 0 |
| `Vector4` | 85 | 85 | 0 |
| `Quaternion` | 55 | 55 | 0 |
| `Matrix` | 107 | 107 | 0 |
| `Graphics.Viewport` | 14 | 14 | 0 |

For every row, field/property, overload, parameter, return, operator, and
ref/out mapping diagnostics are zero. The five primary values are Swift
`struct`s with direct `Float` fields. Assignment and return therefore copy the
value; there is no shared box, backing array, native handle, or CNA dependency.

## Binary32 policy

`System.Single` maps to Swift `Float`. Arithmetic stays in `Float`, including
intermediate multiply/add/subtract/divide ordering. XNA transcendental calls
are represented explicitly as a binary32 input to the corresponding
`System.Math`-shaped operation followed immediately by a `Float` rounding
boundary. This reproduces the reference large-angle sine/cosine observations
without promoting an entire algorithm to `Double`.

Exact `Float.bitPattern` evidence covers zero normalization, reciprocal-once
scalar division, signed-zero negation, large-angle rotations, quaternion
multiplication grouping, Slerp, singular inversion, asymmetric transforms, and
Viewport projection. Zero normalization and zero quaternion inversion produce
NaNs. Singular Matrix inversion uses XNA's fixed adjugate expansion and returns
NaN components; it does not throw or substitute Identity.

XNA `GetHashCode` is implemented as wrapping addition of component
`System.Single.GetHashCode` projections, with both signed-zero encodings hashing
as zero. Swift's randomized `hashValue` is not used. Parameterless `ToString`
uses the XNA field labels/order and a narrow invariant general-float formatter.
That formatter reproduces the CLR "G7" general format — seven significant
digits, trailing zeros removed, and scientific notation with an upper-case
marker and a signed two-digit exponent once the decimal exponent leaves
`[-4, 6]`. Full `CultureInfo` behavior remains outside this value-only
milestone.

## Vectors and array transforms

`Vector3.Forward` is `(0, 0, -1)` and `Backward` is `(0, 0, 1)`.
`Cross(UnitX, UnitY)` is `UnitZ`; reversing the operands produces `-UnitZ`.
Vectors transform as row vectors. Translation is read from Matrix `M41` through
`M43`; normal transforms omit translation.

All selected value and ref/out overloads are independently emitted. CLR
`ref`/`out` maps to Swift `inout`. CLR destination arrays also map to Swift
`inout Array`, because a non-`inout` Swift value parameter cannot preserve
writes. Source arrays remain value snapshots. Consequently an overlapping
source/destination call reads the original Swift Array snapshot while writing
the supplied destination; this is the explicit copy-on-write language mapping,
not an undisclosed allocation-return substitute. Empty, single, multiple,
overlap, negative length, negative index, and short destination/source paths are
tested. Recoverable Swift errors project CLR array exceptions instead of
allowing an unrecoverable Array subscript trap.

## Quaternion conventions

Quaternion multiplication uses XNA's scalar grouping and operand order.
`Concatenate(value1, value2)` is `value2 * value1`. Yaw/pitch/roll and
axis-angle creation use the reference half-angle ordering. Rotation-matrix
conversion follows XNA's trace/major-diagonal branches.

Slerp flips the second quaternion for a negative dot product, uses the linear
branch above `0.999999`, and otherwise uses the reference acos/sine weights.
Lerp applies the same shortest-path sign decision and normalizes the result.
Zero normalize/inverse behavior, large angles, grouped multiplication,
matrix conversion, and quaternion/matrix round trips are qualified by exact or
rotation-equivalent observations.

## Matrix conventions

Matrix publicly exposes exactly mutable `M11` through `M44`; no public Array or
SIMD representation exists. It is row-major in its named field presentation and
uses XNA's row-vector convention. Matrix multiplication is ordinary ordered
composition for that convention. Asymmetric scale/rotation/translation cases
prove that multiplication order is not commuted.

`Right`, `Up`, and `Backward` are the first three components of rows one,
two, and three. `Left`, `Down`, and `Forward` negate those rows.
`Translation` reads and writes `M41`, `M42`, and `M43`.

`CreateLookAt` is the XNA right-handed view construction: camera backward is
`Normalize(position - target)`, with translation formed by negative basis dot
products. Perspective creation places `-1` in `M34`, uses a `[0, 1]` depth
mapping, and validates FOV and near/far planes exactly through the formal Swift
error projection. XNA does not reject zero/NaN aspect ratio, so those values are
not clamped. Infinite far-plane arithmetic remains IEEE-observable. Orthographic
creation performs no invented validation.

The full 107-member contract includes billboard, constrained billboard,
axis/quaternion/yaw-pitch-roll, look-at/world, orthographic/perspective,
reflection/shadow, decompose, transform, transpose, determinant, invert, lerp,
arithmetic, all inout forms, and all operators. Mirror decomposition and
degenerate look-at observations match the retained XNA corpus.

## Viewport

`Project` composes `world * view * projection`, transforms the source, performs
XNA's conditional homogeneous divide, flips screen Y, and maps depth through
`MinDepth...MaxDepth`. `Unproject` reverses viewport/depth mapping, inverts the
same composition with XNA Matrix inversion, and applies the same divide rule.
The retained asymmetric viewport/world/view/projection case matches exact XNA
bit patterns in both directions, including singular inversion producing NaNs.

## Closed Plane dependency

The authoritative Matrix contract exposes `Plane` in `CreateShadow` and
`CreateReflection`. A public strict-clean Matrix therefore cannot compile
without a public Plane identity. Pinned metadata then proves that complete Plane
recursively requires PlaneIntersectionType, Ray, BoundingBox, BoundingSphere,
BoundingFrustum, and ContainmentType. Foundation Milestone 2 therefore includes
exactly that dependency closure. All seven types are complete with zero local
diagnostics; no Plane diagnostic is hidden or allowlisted. The exact graph and
behavior evidence are in `geometry-intersection-evidence.md`.

## Verification summary

The pure corpus contains 360 observations/assertions with zero failures. The
API verifier has 29 mutation/self-tests. Genuine manual diagnostic suppressions
are zero. The 86 deterministic language-projection exclusions are reported as
49 enum storage fields, 28 finalizer mappings, six namespace markers, and three
inherited member projections; 18 destination-array mutation mappings are
reported separately. Native ABI measurements remain unchanged at 25 functions,
72 type positions, 15 layouts, two callbacks, and 168 constants, with zero ABI
mismatches. The native regression suite passes 20 Game, recreation, Texture2D,
SpriteBatch, and callback-error cycles with zero crashes, observed use-after-free,
or double-free. The unchanged maintained template passes 60-frame debug and
600-frame release canaries.
