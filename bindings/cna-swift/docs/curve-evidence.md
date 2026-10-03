# Managed XNA Curve-family evidence

## Exact closure and authority

Foundation Milestone 5 closes exactly these six Microsoft XNA Framework 4.0
Windows runtime types and 49 mapped member identities:

| Type | CLR kind | Sealed | Members | Swift kind | Local diagnostics |
|---|---|---:|---:|---|---:|
| `Curve` | class | no | 11 | open class | 0 |
| `CurveKey` | class | no | 15 | open class | 0 |
| `CurveKeyCollection` | class | no | 13 | open class | 0 |
| `CurveContinuity` | enum | yes | 2 | `Int32` enum | 0 |
| `CurveLoopType` | enum | yes | 5 | `Int32` enum | 0 |
| `CurveTangent` | enum | yes | 3 | `Int32` enum | 0 |

Public shape comes from the retained metadata contract with SHA-256
`7207908eb7926cc90a156d0370c907add4dda465421cea1cbec51afba2f97fdc`.
Behavior and subclassability come from the corresponding Microsoft assembly,
SHA-256
`38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`,
using its disassembled IL and independently retained decompilation. The
Windows reference inputs are retained outside this source package. The
direct-runtime query source is retained as
`tools/behavior/XnaCurveReferenceProbe.cs`; no Microsoft binary is copied into
the repository or archive.

The implementation is scalar managed Swift. It imports no CNAShim API, creates
no native handle, and changes no CNA C ABI entry.

## Enums and reference classes

The raw values are `Smooth=0`, `Step=1`; `Constant=0`, `Cycle=1`,
`CycleOffset=2`, `Oscillate=3`, `Linear=4`; and `Flat=0`, `Linear=1`,
`Smooth=2`. None is a flags enum and the synthetic CLR `value__` fields remain
formal language-storage exclusions.

`Curve`, `CurveKey`, and `CurveKeyCollection` are non-sealed CLR reference
classes, hence open Swift classes. Assignment aliases the same object. No
struct copy helper, backing native object, generation, disposal, or deinit
path exists.

## `CurveKey`

The two-argument constructor initializes both tangents to positive zero and
continuity to `Smooth`. The four-argument constructor also defaults continuity
to `Smooth`; the five-argument constructor stores every supplied scalar. The
position is immutable after construction. Value, both tangents, and continuity
remain mutable.

`Clone` always constructs a distinct base `CurveKey` with all five fields
copied. Subsequent mutable-field changes are independent. Equality is field
value equality in position, value, tangent-in, tangent-out, continuity order;
it is not reference identity. IEEE `==` is used for every Single, so either
signed zero compares equal while any NaN field compares unequal, including
when an object is compared with itself. Typed `Equals(nil)` and object
`Equals(nil)` return false. The two operators represent null explicitly:
null/null is equal, exactly one null is unequal, and non-null values use the
same field comparison.

`GetHashCode` is wrapping `Int32` addition in the same field order. Each Single
uses the established CLR helper: both zeros hash as zero and every other value
uses its exact signed bit interpretation, including NaN payloads. Continuity
contributes its `Int32` raw value. Swift `Hashable` and randomized `hashValue`
are not used.

### Authoritative `CompareTo` result

The reference IL is unambiguous:

1. dereference `other.Position`;
2. if `Position == other.Position`, return zero;
3. if `Position < other.Position`, return minus one;
4. otherwise return plus one.

This is not `System.Single.CompareTo`. The exact results are:

| Left / right position | Result |
|---|---:|
| finite less / equal / greater | -1 / 0 / +1 |
| `+0` / `-0` or `-0` / `+0` | 0 |
| `NaN` / finite | +1 |
| finite / `NaN` | +1 |
| `NaN` / `NaN` | +1 |
| `-Infinity` / `+Infinity` | -1 |
| `+Infinity` / `-Infinity` | +1 |
| non-null / null | CLR null-reference failure |

Swift maps the final row to a throwing optional-reference call and never traps.
This independently resolves the previous cross-binding conflict. Any sibling
implementation reporting `NaN < finite` or `NaN == NaN` for this method needs
external reconciliation; no sibling repository is changed here.

`System.IComparable<CurveKey>` therefore maps only to this exact `CompareTo`
member, with no Swift `Comparable` conformance. The existing
`System.IEquatable<CurveKey>` rule remains `Equals` plus the XNA operators,
with no Swift `Equatable` conformance.

## `CurveKeyCollection`

The direct `ICollection<CurveKey>` interface maps to its concrete XNA member
set: `Add`, `Clear`, `Contains`, `CopyTo`, `Remove`, `Count`, and `IsReadOnly`.
The verifier recognizes the pinned direct interface and checks those members,
but adds no fake BCL protocol and no Swift `Collection`, `MutableCollection`,
`Sequence`, or `RandomAccessCollection` conformance.

Transitive `IEnumerable<CurveKey>` and `IEnumerable` map `GetEnumerator` to
the public support type `CNAEnumerator<CurveKey>`, outside the
`Microsoft.Xna.Framework` namespace. Its `Next() throws -> CurveKey?` operation
is live over the source collection, preserves reference identity and sorted
order, and returns nil only after exhaustion. It is intentionally not an
`IteratorProtocol`: mutation invalidation must throw. Every call to
`GetEnumerator` creates an independent cursor.

The source List version behavior is reproduced exactly:

- successful `Add`, `Remove`, `RemoveAt`, and `Clear` invalidate enumerators;
- `Clear` invalidates even when already empty;
- same-position item replacement invalidates once;
- different-position replacement removes then inserts and invalidates;
- failed `Remove` and failed `RemoveAt` do not invalidate;
- `CopyTo`, whether successful or failing, does not mutate the List version;
- a mutation is detected even after an enumerator had reached its end.

The CLR read/write `Item[Int32]` property cannot be a throwing Swift setter.
It therefore has one formal source identity represented by two compiler
symbols:

```swift
func Item(_ index: Int32) throws -> CurveKey
func SetItem(_ index: Int32, _ value: CurveKey) throws
```

The verifier recombines these into one mutable property identity and separately
measures the accessor projection. Negative indices and `Count` fail through
`CNAError.argumentOutOfRange`, never `fatalError`, `precondition`, or an Array
trap. A null setter value is an immediate XNA `ArgumentNullException`; the
strict Swift setter uses a non-Optional parameter under the established
immediate-failure null policy.

`Add` is the exact `List<CurveKey>.BinarySearch` algorithm: each midpoint's
stored key invokes the direct-branch `CompareTo` against the new key. A found
ordinary equal-position run is scanned forward with Single `==`, then the new
reference is inserted after the run. This makes ordinary duplicate insertion
stable and permits the same object more than once. Signed zeros form one equal
run. NaNs are unordered by `==` and compare as +1 in both directions, so a
second NaN inserted into the simple finite/NaN fixture precedes the first NaN.
The implementation never appends and sorts globally.

Item replacement compares old and new positions with Single `==`. An equal
position replaces in place without cache invalidation. A different position,
including NaN/NaN, removes and re-adds through the sorted insertion algorithm.
`Contains`, `IndexOf`, and `Remove` use `CurveKey.Equals`, not reference
identity; the first field-equal entry is selected. Their optional item mapping
preserves the valid null results: false, -1, and false respectively. `Add`
uses a non-Optional parameter because null only causes immediate failure.

`CopyTo` maps the caller-owned array to `inout [CurveKey]`. It validates the
`Int32` index and capacity, preserves oversized prefixes/tails, never replaces
the destination Array, and copies the same key references. This is the one new
whole-profile caller-owned array mutation mapping.

`Clone` creates a distinct collection and a distinct backing Array, copies the
cached range values, forces its cache available, and shares every contained
`CurveKey` reference. Structural mutations are independent; key mutations are
visible through both collections.

## `Curve`, tangents, and cache

Construction creates one stable `Keys` collection and initializes both loop
modes to `Constant`. `IsConstant` is exactly `Keys.Count <= 1`: empty and
one-key curves are constant, while two keys are not, regardless of their
values. `Clone` creates a distinct Curve and shallow-cloned key collection;
the Curve and collection identities are independent, while key objects remain
shared. Loop values are copied.

`ComputeTangent` first selects previous/current/next values, substituting the
current key at either endpoint. `Flat` writes positive zero. `Linear` writes
`current.Value - previous.Value` on input and
`next.Value - current.Value` on output; positions do not normalize it.
`Smooth` uses:

```text
valueSpan = next.Value - previous.Value
positionSpan = next.Position - previous.Position
TangentIn  = valueSpan * abs(previous.Position - current.Position) / positionSpan
TangentOut = valueSpan * abs(next.Position - current.Position) / positionSpan
```

If `abs(valueSpan) < 1.1920928955078125e-7`, the selected tangent is positive
zero before any division. Otherwise the exact binary32 multiply/divide order
is retained, including signed zero, infinity, and NaN for duplicate or special
positions. In/out modes are computed independently. Whole-curve computation
walks indices from zero upward; formulas read only position/value, so repeated
calls are stable and earlier tangent writes cannot affect later formulas.
Invalid single-key indices throw the mapped argument-out-of-range error.

The collection caches `TimeRange` and its binary32 reciprocal for loop modes.
Add, successful removal, `RemoveAt`, `Clear`, different-position replacement,
and even successful `CopyTo` mark that evaluation cache unavailable exactly as
the IL does. Failed `Remove` also marks the cache unavailable although it does
not invalidate an enumerator. Same-position replacement leaves the cache
valid because position cannot change.

## Evaluation, Hermite, and loops

An empty curve returns positive zero and a one-key curve returns that key's
value for every input. Multi-key segment selection scans forward and selects
the first next position `>=` the target. Equal-position spans and spans at most
`1e-10` produce an interpolation amount of zero. The fraction is intentionally
computed after widening the two key positions and target to Double, then
narrowed once to Float. The retained differentiating fixture uses position bit
patterns `C75E47C4`, `46194550`, and `44A2282C`: XNA produces result
`3F748F88`; a Float-only fraction produces `3F748F84`.

`Step` returns the first key while amount is strictly less than one and the
second at exactly one. Smooth continuity evaluates the reference Float Hermite
grouping with `t²`, `t³`, both values, first tangent-out, and second tangent-in.
The asymmetric retained fixture evaluates to `0x400C2F5B`. No spline library,
FMA rewrite, or whole-expression Double promotion is used. NaN, infinity,
signed zero, zero tangents, duplicate positions, and exact key boundaries are
covered.

Loop behavior is:

- `Constant`: return the first or last endpoint value unchanged;
- `Linear`: `first.Value - first.TangentIn * (first.Position-position)` or
  `last.Value - last.TangentOut * (last.Position-position)`;
- `Cycle`: wrap position without a value offset;
- `CycleOffset`: wrap and add
  `(last.Value-first.Value) * cycleNumber`;
- `Oscillate`: odd cycles reverse from the last position, even cycles advance
  from the first.

Cycle calculation is binary32
`(position-first.Position) * inverseTimeRange`. Any negative result is reduced
by one before the IL's truncating `conv.i4`, including an exact negative
integer. Thus the exact pre-boundary one range before the first key uses cycle
-2, not -1. Signed integer bit-and determines parity, so negative odd/even
cycles match CLR behavior. The managed unchecked conversion returns
`Int32.min` for NaN/infinity/out-of-range inputs instead of allowing Swift's
numeric conversion to trap. Exact positive and negative boundaries, adjacent
cycles, large finite magnitudes, and infinity are qualified.

## Compiler and regression result

The final compiler scoreboard is 55 target types and 1,247 mapped target
members: 50 complete, the same five runtime partials, and 202 missing. Global
diagnostics are 353, comprising only 202 missing types, 131 members on the five
partials, two base mismatches, one interface mismatch, one property mismatch,
and 16 overload mismatches. Every Curve row has zero kind, base, interface,
field, property, method, parameter, return, overload, generic, enum, flags,
operator, ref/out, language, leak, and unmeasured diagnostics.

The verifier has 66 mutation/self-tests. Formal whole-profile projection
counters are one comparable-interface projection, one collection-interface
projection, nine enumerator-support return projections, four read/write
indexed-property accessor projections, two optional global-operator
placements, and 19 caller-owned array mutation mappings. Only the selected
Curve collection currently emits the new enumerator and read/write indexer
symbols. Manual/applied allowlists and unmeasured categories remain zero.

The `PURE_XNA_DERIVED` corpus contains 986 observations/assertions with zero
failures. New groups are `CURVE_ENUMS=1`, `CURVE_KEY=1`,
`CURVE_COLLECTION=2`, `CURVE_TANGENTS=1`, `CURVE_EVALUATE=1`, and
`CURVE_LOOPS=1`. Every Curve test runs without `CNA_NATIVE_LIBRARY`,
RuntimeState, or CNAShim. Debug, release, warnings-as-errors, Symbol Graph,
ABI, native lifecycle, unchanged-template, exact-archive, and isolated-consumer
results are retained in the generated reports and final handoff.
