# Foundation 37 — the projected exception payload

Foundation 30 projected the three exception **bases** eight XNA types inherit
from. This milestone projects the eight classes the support layer actually
**raises**, and converts every CLR-shaped failure in the binding to the exact
class, constructor and message the admitted IL selects.

Before it, every one of those failures came out of `CNAError` with the right
message and the wrong class: `catch is CNAArgumentException` could not work,
`ParamName` did not exist, and `HResult` was not carried at all. Four of the
messages were also wrong, and nothing noticed, because the resource-string gate
only checked that each *pinned* value appeared somewhere in the source — never
that a message in the source was pinned.

## 1. The eight classes

Each is a real Swift class on the exact `mscorlib` base, so the CLR's own
`catch` hierarchy survives.

```text
System.ArgumentException            -> CNAArgumentException           : CNASystemException
System.ArgumentNullException        -> CNAArgumentNullException       : CNAArgumentException
System.ArgumentOutOfRangeException  -> CNAArgumentOutOfRangeException : CNAArgumentException
System.NotSupportedException        -> CNANotSupportedException       : CNASystemException
System.InvalidOperationException    -> CNAInvalidOperationException   : CNASystemException
Collections.Generic.KeyNotFound…    -> CNAKeyNotFoundException        : CNASystemException
System.NullReferenceException       -> CNANullReferenceException      : CNASystemException
System.IndexOutOfRangeException     -> CNAIndexOutOfRangeException    : CNASystemException  (final)
```

The bases are not decoration. A missing key is **not** an argument failure —
`KeyNotFoundException` sits on `SystemException` directly — and neither is a
read-only refusal, while a null argument and an out-of-range one **are**, which
is why both specialize `ArgumentException`. `IndexOutOfRangeException` is the
one class `mscorlib` seals, so it is the one projected `final`.

Every constructor calls up and then **overwrites** the HResult its base has just
assigned, so each class reports its own:

```text
ArgumentException            0x80070057   NotSupportedException      0x80131515
ArgumentNullException        0x80004003   InvalidOperationException  0x80131509
ArgumentOutOfRangeException  0x80131502   KeyNotFoundException       0x80131577
NullReferenceException       0x80004003   IndexOutOfRangeException   0x80131508
```

## 2. `Message` is composed, not stored

`ArgumentException.get_Message` reads `base.Message` and, when `ParamName` is
neither null nor empty, concatenates it with `Environment.NewLine` and
`Arg_ParamName_Name` formatted with the name. So the message a caller sees is
never the string the raise site passed.

**The newline was the reason this conversion was deferred twice**, on the
grounds that composing the message would mean asserting the reference
platform's line ending. The admitted assembly answers it: `System.Environment`'s
`get_NewLine` has the entire body

```text
IL_0000:  ldstr      "\r\n"
IL_0005:  ret
```

It is not probed from the platform at all. The separator is therefore a pinned
IL literal, and the composed message carries a carriage return even though this
binding runs on Linux. `bcl-authorities.json` gains a `selectedIlLiterals`
section for it, and the BCL authority audit now reads the literal back out of
the binary and requires the method's whole body to be exactly `ldstr`/`ret` — a
getter that branched or cached would not be a literal fact and is refused.

`ArgumentOutOfRangeException.ActualValue`, and the constructor that stores one,
are **forbidden rather than absent**. The `Message` branch that reads it calls
`System.Object.ToString()` on an arbitrary object; that is a virtual call whose
result is not reconstructible from IL, and a Swift `String(describing:)` in its
place would be this projection's formatting rather than the CLR's. With the
constructor absent, `ActualValue` is nil on every instance this projection can
build and the branch is unreachable rather than wrong.

## 3. The constructor overloads, which are transposed on purpose

```text
ArgumentException            (message, paramName)
ArgumentNullException        (paramName, message)     <- the other way round
ArgumentOutOfRangeException  (paramName, message)     <- the other way round
```

`ArgumentNullException`'s and `ArgumentOutOfRangeException`'s single-argument
constructors take a **parameter name**, not a message, and pass their own
substituted resource as the message. Every raise site's overload is read out of
the IL rather than guessed, and the Swift labels keep the CLR parameter names so
a transposition cannot be silent. A mutation that swaps them is caught.

## 4. Every converted raise site

| Raise site | Class | Constructor | Message resource |
|---|---|---|---|
| `CNAList` indexer, `RemoveAt` | `ArgumentOutOfRangeException` | `(paramName, message)` | `ArgumentOutOfRange_Index` |
| `CNAList.Insert` | `ArgumentOutOfRangeException` | `(paramName, message)` | `ArgumentOutOfRange_ListInsert` |
| `CNAList` enumerator | `InvalidOperationException` | `(message)` | `InvalidOperation_EnumFailedVersion` |
| `CNACollection` read-only guard | `NotSupportedException` | `(message)` | `NotSupported_ReadOnlyCollection` |
| `CNACollection` setter / `RemoveAt` | `ArgumentOutOfRangeException` | `(paramName, message)` | `ArgumentOutOfRange_Index` |
| `CNACollection.Insert` | `ArgumentOutOfRangeException` | `(paramName, message)` | `ArgumentOutOfRange_ListInsert` |
| `CNADictionary` capacity | `ArgumentOutOfRangeException` | `(paramName, message)` | `ArgumentOutOfRange_NeedNonNegNum` |
| `CNADictionary` indexer getter | `KeyNotFoundException` | `()` | `Arg_KeyNotFound` |
| `CNADictionary.Add` duplicate | `ArgumentException` | `(message)` | `Argument_AddingDuplicate` |
| `CNADictionary` enumerator | `InvalidOperationException` | `(message)` | `InvalidOperation_EnumFailedVersion` |
| `CNADictionary` key/value `CopyTo` index | `ArgumentOutOfRangeException` | `(paramName, message)` | `ArgumentOutOfRange_NeedNonNegNum` |
| `CNADictionary` key/value `CopyTo` short | `ArgumentException` | `(message)` | `Arg_ArrayPlusOffTooSmall` |
| `GameComponentCollection.InsertItem` | `ArgumentException` | `(message)` | `CannotAddSameComponentMultipleTimes` |
| `GameComponentCollection.SetItem` | `NotSupportedException` | `(message)` | `CannotSetItemsIntoGameComponentCollection` |
| `GameServiceContainer` null provider | `ArgumentNullException` | `(paramName, message)` | `ServiceProviderCannotBeNull` |
| `GameServiceContainer` duplicate | `ArgumentException` | `(message, paramName)` | `ServiceAlreadyPresent` |
| `GameServiceContainer` assignability | `ArgumentException` | `(message)` | `ServiceMustBeAssignable` |
| `TouchCollection` seven mutators | `NotSupportedException` | `()` | `Arg_NotSupportedException` |
| `TouchCollection` ctor / indexer / `CopyTo` | `ArgumentOutOfRangeException` | `(paramName)` | `Arg_ArgumentOutOfRangeException` |
| `BoundingSphere..ctor` | `ArgumentException` | `(message)` | `NegativeRadius` |
| `BoundingSphere.CreateFromPoints` | `ArgumentException` | `(message)` | `BoundingSphereZeroPoints` |
| `BoundingBox.CreateFromPoints` | `ArgumentException` | `(message)` | `BoundingBoxZeroPoints` |
| `BoundingBox`/`BoundingFrustum.GetCorners` | `ArgumentOutOfRangeException` | `(paramName, message)` | `NotEnoughCorners` |
| `Matrix.CreatePerspectiveFieldOfView` | `ArgumentOutOfRangeException` | `(paramName, message)` | `OutRangeFieldOfView` |
| `Matrix` plane checks | `ArgumentOutOfRangeException` | `(paramName, message)` | `NegativePlaneDistance`, `OppositePlanes` |
| `Curve.ComputeTangent` | `ArgumentOutOfRangeException` | `(paramName)` | `Arg_ArgumentOutOfRangeException` |
| `CurveKeyCollection` indexer | `ArgumentOutOfRangeException` | `(paramName, message)` | `ArgumentOutOfRange_Index` |
| `CurveKeyCollection` enumerator | `InvalidOperationException` | `(message)` | `InvalidOperation_EnumFailedVersion` |
| `CurveKey.CompareTo(nil)` | `NullReferenceException` | `()` | `Arg_NullReferenceException` |
| vector transform range helper | `ArgumentException` / `IndexOutOfRangeException` | `(message)` / `()` | `NotEnoughSourceSize`, `NotEnoughTargetSize` |
| `ContentSerializer*` empty setters | `ArgumentNullException` | `(paramName)` | `ArgumentNull_Generic` |
| `AudioEmitter.SetDopplerScale` | `ArgumentOutOfRangeException` | `(paramName, message)` | `InvalidEmitterDopplerScale` |

## 5. Five wrong messages the conversion found

The old gate compared each *pinned* resource against the Swift source. It could
not see a message in the source that was pinned to nothing, and four were:

| Site | What the source said | What the assembly says |
|---|---|---|
| `Collection<T>` read-only guard | "Specified method is not supported." | **"Collection is read-only."** (`NotSupported_ReadOnlyCollection`, not `Arg_NotSupportedException`) |
| `BoundingSphere..ctor` | "The sphere radius must be greater than or equal to zero." | **"Radius must be greater than 0."** |
| `BoundingBox`/`BoundingSphere.CreateFromPoints` | one invented sentence for both | **two different resources**, one with a trailing period and one without |
| vector transform range helper | "The source/destination array is too small." | **"Source array must be equal or bigger than requested length."** / **"Target array size must be equal or bigger than source array size."** |

The fifth is `AudioEmitter.SetDopplerScale`, whose message was not reproduced at
all. All ten of the newly needed XNA messages are now pinned in
`xna40-selected-resource-strings.json`, taking that file from 4 entries to 14.

## 6. Two payloads that are deliberately not the CLR's

`List<T>.CopyTo` calls `Array.Copy`, whose five-argument public overload
forwards to an `internalcall` six-argument one. The validation that raises for a
negative index or a short destination therefore lives in CLR **native** code and
is in no admitted IL. The classes are the documented ones and are projected; no
message is claimed, so each reports its own class's substituted default. The
same applies to `CurveKeyCollection.CopyTo`, which delegates to the same place.

`CurveKey.CompareTo(nil)` and the vector transforms' negative indices are
runtime-raised too — `ldfld` and `ldelema` through a null or out-of-range
reference — so ECMA-335 fixes the class while the message is the runtime's. Each
carries the parameterless constructor's substituted resource, and the fact that
this is not the runtime's own sentence is stated rather than hidden.

These are the only messages in the layer that are not the CLR's, and each site
says so in its own comment.

## 7. `CNAError` after the conversion

`CNAError` declares **no CLR-shaped case at all**. `argumentNull`,
`argumentOutOfRange`, `indexOutOfRange`, `nullReference`, `collectionModified`,
`keyNotFound` and `notSupported` are gone, and the last one — `argument` — is
renamed `producerInvariant`, because its single remaining use guards
`VisualizationData.store`'s own producer invariant, which is internal and has no
XNA counterpart. What is left is a native library that will not load, an
admitted-ABI refusal, a missing symbol, a native call failure, an owner-thread
violation, a stale runtime generation, an unsupported platform, callback
lifecycle misuse, a stream failure and that one invariant.

`catch is CNAException` cannot swallow a native runtime error, and
`catch is CNAError` cannot swallow a projected one. Both directions are tested.

## 8. Authority

`mscorlib` `5634668d…acc63` gains ten families and eleven resource strings, and
one IL literal:

```text
BCL_AUTHORITY_TYPES=27          (19 -> 27)     BCL_AUTHORITY_MEMBERS=247  (205 -> 247)
BCL_SENTINEL_CHECKS=413         (314 -> 413)   BCL_MUTATION_SELF_TESTS=454 (346 -> 454)
BCL_RESOURCE_CHECKS=19          (8 -> 19)      BCL_IL_LITERAL_CHECKS=1     new
BCL_CROSS_CHECKS=136 (monodis)  BCL_MANIFEST_CHECKS=48   BCL_NEGATIVE_CONTROLS=1 refused
```

Two of the new sentinels were written and immediately failed their own mutation
controls — dropping *a* constructor from `ArgumentNullException` or
`ArgumentOutOfRangeException` went undetected until the sentinels pinned the
exact public constructor count of all six families — which is the mutation gate
doing its job on the gate's own author.

The pinned XNA resource strings go from 4 to 14, and
`RESOURCE_STRING_CHECKS` from 8 to 28.

## 9. Falsifiability

`tools/projection_mutations/run.py` plants ten realistic defects, runs the
tests, requires each to fail, and proves the tree is restored:

```text
CAUGHT  wrong-exception-class             a raise site throwing a neighbouring class
CAUGHT  wrong-base-class                  a projected class on the wrong CLR base
CAUGHT  dropped-message-composition       Message no longer appending ParamName
CAUGHT  wrong-newline                     the separator taken from the host, not the IL
CAUGHT  inherited-hresult                 a subclass keeping its base's HResult
CAUGHT  transposed-constructor            the two-argument overload's arguments swapped
CAUGHT  confused-range-resource           Insert reporting the indexer's message
CAUGHT  confused-not-supported-resource   the read-only guard reporting the parameterless default
CAUGHT  swift-class-name-in-a-message     a projected class reporting its Swift name
CAUGHT  reverted-to-the-runtime-channel   a CLR-shaped failure back on CNAError

PROJECTION_MUTATIONS=10 CAUGHT=10 SURVIVORS=0
```

**The first version of that harness reported three survivors, and the harness
was the thing at fault.** It filtered `swift test` to the suites that assert a
projected payload; this toolchain honours only one `--filter` pattern, so an
alternation regex and repeated flags both selected a single suite, and the
baseline plus every mutation ran fifteen tests instead of four hundred. It runs
the whole suite now, which costs about fifteen seconds a mutation and cannot be
wrong in that way.

Two defects were found by the new tests themselves:

1. **`Exception.get_Message` reported the Swift class name.** `GetClassName()`
   formats `Exception_WasThrown`, and the support-class name table still held
   only the three Foundation-30 classes, so
   `CNAArgumentException(message: nil).Message` read
   `"Exception of type 'CNAArgumentException' was thrown."` All eight new
   classes are now in the table, and a test asserts every one.
2. **The behaviour corpus appeared to shrink while it grew.** `assertProjected`
   asserts four separately observable facts about one exception, and the corpus
   counter only knew `XCTAssert*`, so converting a one-assertion test into a
   four-fact one lost three. The counter knows the helper now:
   `PURE_XNA_DERIVED` 2002 → 2036 and `PURE_BCL_DERIVED` 210 → 292.
