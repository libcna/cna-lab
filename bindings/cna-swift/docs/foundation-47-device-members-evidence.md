# Foundation 47 — `SetViewport`, `ScissorRectangle`, `GraphicsDeviceStatus`, and the last mapping mismatch

Three members and four routes. The interesting part is what the third of them
finished: **every remaining diagnostic is now an absence.**

## 1. The scoreboard no longer contains a disagreement

```text
100  MISSING_TYPE
 96  MISSING_MEMBER
 16  OVERLOAD_MAPPING_MISMATCH   — every one "required overload is absent"
```

`PROPERTY_MAPPING_MISMATCH` was 1 for the whole of this session and is now 0.
There is no `TYPE_KIND_MISMATCH`, no `BASE_MAPPING_MISMATCH`, no
`FIELD_MAPPING_MISMATCH`, no `INTERFACE_MAPPING_MISMATCH`, no
`PARAMETER_MAPPING_MISMATCH`, no `RETURN_MAPPING_MISMATCH`, no
`ENUM_VALUE_MISMATCH`, no `UNEXPECTED_TYPE`, no `UNEXPECTED_MEMBER`, no
`UNMEASURED_STRUCTURAL_CATEGORY`, and no leak of any kind.

The projection is **incomplete, not incorrect**: wherever it has projected an
XNA member it agrees with the pinned metadata, and what is left is what has not
been written yet. That is a different claim from a diagnostic count going down,
and it is the first time in this session it has been true.

## 2. `SetViewport` — the mismatch that had been asking by name

```text
the CLR setter cannot be a Swift `set`, so the projection requires a
SetViewport writer method; it is absent
```

`Viewport`'s reader has existed since the earliest graphics work, but its CLR
setter is fallible — `Helpers.CheckDisposed(this, pComPtr)` and then a push that
can raise `ArgumentException`, 6,208 bytes of IL — and Swift has no throwing
setter. The recorded accessor rule turns that into a `Set<Name>` writer method,
which is the projection of the CLR setter accessor and not a new XNA member.
`cna_graphics_device_set_viewport` takes `CNA_Viewport` **by value**, like
`set_blend_factor`.

## 3. `ScissorRectangle` and `GraphicsDeviceStatus`

`ScissorRectangle`'s accessors are both `IL_DIRECT_THROW`, so it is a throwing
reader plus a throwing `SetScissorRectangle` — the same shape as `Viewport`, for
the same reason. `GraphicsDeviceStatus` is `IL_DIRECT_THROW` with no setter, so
a throwing reader alone.

CNA and XNA agree on all three status values — `Normal` 0, `Lost` 1, `NotReset`
2 — and **the conversion is still an explicit map**. Foundation 45 is the reason:
eight of nine state enums agreed there and the ninth silently exchanged
`BlendFunction.Min` and `.Max`. "They agree, so a cast is fine" is not a claim
worth making cheaply, and a value outside the three raises
`CNAError.producerInvariant` rather than being forced into a case.

On the qualified HEADLESS renderer the device is never lost, so `Normal` is the
only value this environment can produce. The test says so rather than dressing
one observation up as coverage of three.

## 4. A second missing alias, found the same way as the first

The four routes' first verification run reported one mismatch:

```text
cna_graphics_device_get_status: parameters
  ['uint64_t', 'uint32_t*'] != ['uint64_t', 'CNA_GraphicsDeviceStatus*']
```

`typedef uint32_t CNA_GraphicsDeviceStatus;` at `graphics_device.h:34`, missing
from the type-compatibility alias table exactly as `CNA_ShaderStage` was in
Foundation 45. The textual canonical-declaration check had already passed on all
228 positions. Two milestones running, the first new surface has exposed one
unspelled typedef; that is the check doing its job, and it is cheap to fix
precisely because the two checks are independent.

## 5. What was deliberately not projected

`GraphicsDevice.IsDisposed` is dependency-complete, needs no route, and is
**not** projected. XNA's is a field read on a long-lived object. This
projection's device is a per-callback facade, so "my handle no longer validates"
is not the same proposition as "the device was disposed" — a facade read outside
its own callback is healthy, merely stale. Answering `IsDisposed` from handle
validity would be answering a question the shape cannot answer, so it stays
missing until the facade has a disposal identity to report.

## 6. Falsifiability

Three new mutations, all caught: `SetViewport` accepting a value and discarding
it rather than pushing, the scissor rectangle's origin and extent exchanged on
the way out, and the three status values mapped one place along.

## 7. Measurement

```text
BOUND_FUNCTIONS=73  (was 69)       PROTOTYPE_TYPE_POSITIONS=228 (was 216)
ABI_MISMATCHES=0                   LAYOUTS=25  LAYOUT_FIELDS=209
COMPLETE_TYPES=150                 MISSING_TYPE=100
MISSING_MEMBER=96   (was 98)       TOTAL_DIAGNOSTICS=212 (was 215)
PROPERTY_MAPPING_MISMATCH=0        (was 1, all session)
THROWING_GETTER_PROJECTIONS=114    WRITER_METHOD_PROJECTIONS=113
PROJECTION_MUTATIONS=60 CAUGHT=60 SURVIVORS=0
545 tests, 0 failures
```
