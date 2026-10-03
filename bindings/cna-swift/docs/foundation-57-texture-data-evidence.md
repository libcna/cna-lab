# Foundation 57: SetData and GetData

Six members — the first generic ones this binding has — and the three
validations that stand in front of them. `TOTAL_DIAGNOSTICS` 175 → 169,
`MISSING_MEMBER` 69 → 63.

Texture data is the first thing this environment can actually **see**.
Foundation 53 established that nothing drawn is observable here: both readback
routes answer `CNA_RESULT_NOT_SUPPORTED`. A texture transfer is different —
`build-probe/f56_texdata.c` writes four texels and reads back exactly those
four — so these tests assert pixel values rather than "the call returned zero".

## Two more gate defects, found by trying to satisfy it

Foundation 56 fixed the first: the contract's `!!0` was never substituted, so a
generic method could not be expressed. Writing the members turned up two more,
both of which reported a *correct* projection as a disagreement.

**`GENERIC_MAPPING_MISMATCH` on all six.** The comparison decided whether a
member was generic by looking for an angle bracket in the mapped type
spellings. A method generic leaves none — `!!0[]` maps to `[T]` — so every
correctly projected generic method was flagged. The expected side now records
its own genericity from the contract's `genericParameters`, and the comparison
uses that.

**`REF_OUT_MAPPING_MISMATCH` on the three `GetData` overloads.** A CLR array a
member writes through becomes a Swift `inout Array`, because a Swift array is a
value; the rule listed the parameter *names* that mean "destination"
(`destinationArray`, `corners`). `Texture2D` calls its array `data` in both
directions, and only `GetData` writes. The rule gains a member-scoped list
beside the name list, and its own prose says why.

Both fixes carry self-tests demonstrated to fail without them, including the
two "other direction" assertions that a naive implementation would pass: a
method placeholder must not be filled from the owner's list, and the
member-scoped mutation rule must not leak to another owner.

## The size rules, decoded rather than recalled

`GetExpectedByteSizeFromFormat` is a 643-byte switch over ninety-eight D3D
formats, reached through `ConvertXnaFormatToWindows`. Composing the two gives
the table the projection carries, and it was decoded mechanically:

```text
Color 4   Bgr565 2   Bgra5551 2   Bgra4444 2   NormalizedByte2 2
NormalizedByte4 4    Rgba1010102 4   Rg32 4   Rgba64 8   Alpha8 1
Single 4   Vector2 8   Vector4 16   HalfSingle 2   HalfVector2 4
HalfVector4 8   HdrBlendable 8   Dxt1/Dxt3/Dxt5 block-compressed
```

`HalfVector4` and `HdrBlendable` share a D3D format and therefore a size —
that is the table's doing, not a transcription slip, and a test says so.

The three validations, each with its own message:

```text
GetAndValidateSizes<T>  sizeof(T) equals the format's size, or divides it
                        exactly -- so SetData<byte> on a Color surface is legal
GetAndValidateRect      a negative origin or non-positive extent; then an edge
                        past the surface, compared UNSIGNED (bgt.un) so an
                        overflowing origin+extent wraps into the refusal
ValidateTotalSize       the array window's bytes equal the region's, exactly
```

All three are tested, including the two an implementer would most likely lose:
a byte array covering a Color surface, and a rectangle whose extent overflows.

## What crosses the boundary

CNA's transfer names an element **type** where XNA blits bytes and lets the
surface's format decide. `CNA_Texture2DTransfer` maps one-for-one onto XNA's
`(level, rect, startIndex, elementCount)`, so the shape needed no invention.

The element type is the one that matches the texture's own format — the same
interpretation XNA performs implicitly, said out loud — and the element count
converts through bytes. That is what makes `SetData<byte>` work: sixteen bytes
become four `CNA_TEXTURE_DATA_COLOR` elements. Passing them as
`CNA_TEXTURE_DATA_BYTE` instead does **not** work; CNA refuses a byte-typed
transfer into a Color texture, measured, which is why the conversion goes
through the format's type.

One input XNA accepts and this refuses: a `startIndex` that lands mid-texel,
which CNA's element-indexed transfer cannot express. It is refused with
`InvalidTotalSize` rather than silently rounded — an honest refusal in place of
a wrong write, recorded where the conversion happens.

## Falsifiability

Seven mutations, 107 → 114:

```text
format-byte-size-wrong-for-color          the one creatable format mis-sized
element-size-rule-demands-an-exact-match  a dividing T refused, as XNA does not
rectangle-origin-test-loosened            a negative origin accepted
rectangle-edge-test-signed                the unsigned comparison made signed
total-size-allows-a-short-window          a window smaller than the region
array-window-offset-ignored               startIndex dropped
rectangle-never-reaches-the-transfer      validated, then not sent
```

`element-size-rule-demands-an-exact-match` survived its first spot check: no
test passed a T smaller than the format. The byte-array test above is what
closed it, and it is the more valuable half of the rule.

## A consumer build that lied

The template canary failed this milestone with three compile errors in
committed-looking code — `type 'Microsoft.Xna.Framework.Graphics' has no member
'expectedByteSize'` — while the package's own build, its release build and 609
tests were all green.

SwiftPM had cached the CNA target's **source file list** in the template's
`.build/build.db`, from before `TextureDataTransfer.swift` existed. A path
dependency's new file was therefore invisible to the consumer while every
existing file compiled, which reads exactly like a real error in the library.
Removing that one build plan fixed it; nothing in the sources was wrong.

Worth knowing because the canary is the gate that would catch a genuine version
of this, and a stale plan makes it cry wolf. `rm .build/build.db` in the
consumer, not a clean rebuild.

## Result

```text
609 tests, 0 failures
PROJECTION_MUTATIONS=114 CAUGHT=114 SURVIVORS=0
BOUND_FUNCTIONS=87 LAYOUTS=28 LAYOUT_FIELDS=243 ABI_MISMATCHES=0
TOTAL_DIAGNOSTICS=169 MISSING_MEMBER=63  every disagreement category 0
RESOURCE_STRINGS_REPRODUCED=38  MESSAGE_COVERAGE_FINDINGS=0
```
