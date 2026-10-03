# Foundation 49: the validation Foundation 47 dropped

## How this was found

Foundation 48 repaired `Clear(Color)`, a member that had been implemented,
tested and green for many milestones while disagreeing with the pinned IL. The
obvious question is whether it was the only one, and the obvious way to ask is
to rank every *implemented* `GraphicsDevice` member by the size of its pinned
IL and read the large ones:

```text
543  Clear                     (repaired in Foundation 48)
387  set_Viewport              <-- 5 throw sites, none projected
300  set_ScissorRectangle      <-- 3 throw sites, none projected
147  set_BlendFactor           no throw site; the size is D3D state pushing
120  set_BlendState            1 site: ArgumentNullException, already handled
103  set_DepthStencilState     1 site: the same
 96  set_MultiSampleMask       none
 93  set_ReferenceStencil      none
 89  get_ScissorRectangle      none
 76  set_RasterizerState       1 site: the same
 70  get_GraphicsDeviceStatus  none
 32  SetRenderTarget           none
 31  get_Viewport              none
```

Two members were narrower than XNA. Both are repaired here.

## `set_Viewport`

```text
Helpers.CheckDisposed(this, pComPtr)
if (X < 0 || Y < 0 || Width <= 0 || Height <= 0)          throw   // IL_0172
(targetW, targetH) = currentRenderTargetCount > 0
    ? (currentRenderTargets[0].width, .height)
    : (pInternalCachedParams.BackBufferWidth, .BackBufferHeight)
if (X + Width > targetW || Y + Height > targetH)          throw   // IL_0162
if (MinDepth < 0f || MinDepth > 1f)                       throw   // IL_0152
if (MaxDepth < 0f || MaxDepth > 1f)                       throw   // IL_0142
if (!((double)MaxDepth >= (double)MinDepth))              throw   // IL_0104
hr = SetViewport(...); if (hr < 0) throw GetExceptionFromResult(hr)
currentViewport = value
```

All five raise `ArgumentException(FrameworkResources.ViewportInvalid, "value")`
— message first, `paramName` second, which is the order the IL pushes them.

Three things the IL settles that a prose reading would not:

* **`blt` on the origin, `ble` on the extent.** `X = 0` is legal; `Width = 0`
  is not. The two comparisons are not interchangeable and the tests pin both.
* **`bge.un` on `float64` for the depth ordering.** An *unordered* branch: it
  throws when `MaxDepth < MinDepth` and also when either is NaN. Written as
  `MaxDepth < MinDepth` it would accept a NaN depth. The projection is
  `Double(value.MaxDepth) >= Double(value.MinDepth)`, which is false for NaN,
  and the mutation that rewrites it as the ordered form is caught.
* **`add` is unchecked.** `X + Width` wraps in CIL where Swift's `+` traps, so
  the projection uses `&+`. A trap is not a projection of a wrap.

## `set_ScissorRectangle`

The body first rewrites its own copy of the rectangle into a D3DRECT — the IL
addresses the struct by offset, `0=X 4=Y 8=Width 12=Height`, and stores
`X + Width` back into the `Width` slot and `Y + Height` into `Height`. Every
comparison after that is on **edges**, not extents:

```text
Helpers.CheckDisposed(this, pComPtr)
if (X < 0 || Width < 0 || Y < 0 || Height < 0)            throw   // IL_011b
right = X + Width;  bottom = Y + Height
(targetW, targetH) = the same render-target-or-backbuffer pair
if (X > targetW || right > targetW ||
    Y > targetH || bottom > targetH)                      throw   // IL_010b
if (right - X > targetW || bottom - Y > targetH)          throw   // IL_00fb
hr = SetScissorRect(&rect); if (hr < 0) throw GetExceptionFromResult(hr)
```

Here the negative test is `blt` on **all four** fields, so an empty scissor
rectangle is legal where a zero-width viewport is not.

### The pair that looks redundant and is not

With `X >= 0` and `X + Width <= targetW`, `Width <= targetW` follows — so the
last two comparisons appear unreachable, and the tempting thing is to drop them
as dead. They are reachable, and only through the unchecked arithmetic:

```text
X = 2, Width = Int32.max
  right      = 2 &+ Int32.max = -2147483647     // wraps
  right > targetW ?  -2147483647 > 800 ?  no    // slips past the edge test
  right &- X = -2147483647 &- 2 = Int32.max     // wraps back
  Int32.max > 800 ?  yes                        // caught here
```

`testTheScissorOverflowBranchIsReachedAndRejects` exercises exactly that, in
both dimensions, and the mutation that removes the pair is caught. This is why
the arithmetic had to be `&+`/`&-` and not merely "the same comparison": a
checked `+` would have trapped on the input XNA rejects.

## Where the bounds come from

Both members read the same pair `get_DefaultClearOptions` reads for the depth
format: the bound render target if there is one, the presentation parameters
otherwise. XNA writes that branch out three times; it is written once here as
`GraphicsDevice.currentTargetBounds()`, because all three copies read the same
two fields of the same two sources.

`RuntimeState.currentRenderTarget` — Foundation 48's — is what makes the first
arm reachable, and `RenderTarget2D` already projects `Width` and `Height`. The
tests prove the arm matters rather than assuming it: a 800×480 viewport is
accepted against the backbuffer and rejected against a bound 64×64 target, and
a 64×64 viewport the other way round.

## The two messages

Read out of the embedded string table of the registered
`Microsoft.Xna.Framework.dll` and pinned in `registered-assemblies.json`, so
the verifier requires the Swift sources to reproduce them verbatim
(`XNA_RESOURCE_STRING_PROJECTIONS` 22 → 24, and 29 once the five
unguarded messages below joined them):

```text
ViewportInvalid = "The viewport is invalid. The viewport cannot be larger than
                   or outside of the current render target bounds. The
                   MinDepth and MaxDepth must be between 0 and 1."
ScissorInvalid  = "The scissor rectangle is invalid. The scissor rectangle
                   cannot be larger than or outside of the current render
                   target bounds."
```

The composed `Message` is the .NET Framework 4.0 form —
`message + "\r\n" + "Parameter name: value"` — not .NET Core's
`(Parameter 'value')`. The first version of these tests asserted the Core form
and failed; the projection was right and the test was wrong, which is what the
pinned `environmentNewLine` and the `ArgumentException` payload work of
Foundation 30 exist to make impossible to get wrong in the source.

## Five messages that were right, and unguarded

Auditing *how* the two new messages get pinned turned up a gap in the gate
rather than in the code. Five message literals in the Swift sources were
byte-identical to a resource string in the registered
`Microsoft.Xna.Framework.Game.dll` and were **not** in
`selectedResourceStrings`, so nothing compared them against the assembly:

```text
GraphicsDeviceManagerAlreadyPresent   GraphicsDeviceManager..ctor
MissingGraphicsDeviceService          DrawableGameComponent.Initialize
NoGraphicsDeviceService               Game.get_GraphicsDevice
InactiveSleepTimeCannotBeZero         Game.set_InactiveSleepTime
TargetElaspedCannotBeZero             Game.set_TargetElapsedTime
```

Every one was correct — including the two consecutive spaces inside
`GraphicsDeviceManagerAlreadyPresent`, and Microsoft's own misspelling in the
key `TargetElaspedCannotBeZero`. That is luck rather than method: `plan.md`
claims every reproduced message is *read* out of the registered binary and
compared by the verifier, and for these five it was transcribed and nothing
checked it. They are pinned now, which takes `RESOURCE_STRINGS_REPRODUCED` from
22 to 29 and makes the claim true.

## Falsifiability, and the two mutations that failed to be mutations

Eight controls landed, taking the harness from 67 to 75:

```text
viewport-origin-test-loosened          a negative origin accepted
viewport-extent-test-uses-blt          a zero-width viewport accepted
viewport-bounds-checked-additively     the extent compared without the origin
viewport-depth-comparison-ordered      NaN passes an ordered comparison
viewport-bounds-always-the-backbuffer  the bound render target ignored
scissor-negative-test-uses-ble         an empty scissor rectangle rejected
scissor-overflow-pair-dropped          the pair dropped as "redundant"
invalid-bounds-messages-exchanged      the two messages swapped
```

Nine were written. Two survived the first full run, and each survivor said
something.

### `viewport-depth-comparison-ordered` survived, and the code was wrong

The first version of the depth validation was one `guard` chain of
requirements: `MinDepth >= 0, MinDepth <= 1, MaxDepth >= 0, MaxDepth <= 1,
MaxDepth >= MinDepth`. That rejects every input XNA rejects — but it rejects a
NaN depth at the *range* test, because `NaN >= 0` is false, where XNA's `blt`
and `bgt` are both false for NaN and let it fall through to the `bge.un`
ordering test. Same outcome, different branch, and the consequence is that the
ordering test became unreachable for the one input that distinguishes an
unordered comparison from an ordered one. The mutation could not be caught
because the code had already made the distinction invisible.

The validation is now written as XNA's three rejections rather than one chain
of requirements, each branch keeping its own reachability, and the mutation is
caught.

### `scissor-edge-test-drops-the-origin` survived, and was withdrawn

Removing XNA's `X <= targetW` and `Y <= targetH` comparisons changes nothing
observable: with `X >= 0` and `Width >= 0` already established, `right <=
targetW` implies `X <= targetW`, and the overflowing width that breaks the
implication is rejected by the following pair anyway. The comparisons stay in
the projection, because XNA has them and this is a transcription; the mutation
does not stay in the harness, because a gate that cannot fail is not evidence.

A third, `viewport-depth-range-rejects-nan-early`, was written while repairing
the first survivor and withdrawn for the same reason: it changes which branch
rejects a NaN depth, and both branches raise the same exception with the same
message. Both withdrawals are recorded in `run.py` at the point where the
mutation would have been, not deleted quietly.

## AddressSanitizer was reporting leaks, and nothing was reading them

Running the sanitizer sweep for this milestone with a wider filter than
`Executed N tests` showed what earlier sweeps had not:

```text
SUMMARY: AddressSanitizer: 292987 byte(s) leaked in 3729 allocation(s)
```

The tests themselves pass — `564 tests, 0 failures` — and the process exits
non-zero only because LeakSanitizer runs at exit. Earlier milestones grepped
the output for the test tally and for `ERROR: AddressSanitizer`, and a
LeakSanitizer summary says `SUMMARY:`, so "ASan green" meant "the tests
passed", not "the sanitizer was satisfied".

The leaks are not this binding's. Measured by narrowing the run:

```text
--filter PureValueTests   (never starts the CNA runtime)
  117875 byte(s) leaked in 1412 allocation(s)
--filter Foundation49     (starts a CNA game, a device, a render target)
    9665 byte(s) leaked in 81 allocation(s)
```

The suite that touches no native code at all leaks twelve times as much as the
one that starts a whole game, and every frame in those reports is inside
`libswiftCore`, `libglib-2.0` or the test harness — none is in `Sources/`. This
is process-lifetime allocation by the toolchain and XCTest, not a defect here.

`ASAN_OPTIONS=detect_leaks=0` therefore runs clean end to end (`rc=0`), and
that is how the memory-error gate is now invoked and documented: ASan is here
for use-after-free and out-of-bounds, which it reports as `ERROR:`, and those
are zero. The leak numbers are recorded above rather than suppressed silently,
because "we turned a check off" is only honest if the measurement that
justified it is written down next to it.

## Result

```text
564 tests, 0 failures (debug, release, ASan with detect_leaks=0, TSan)
PROJECTION_MUTATIONS=75 CAUGHT=75 SURVIVORS=0
NATIVE_ABI_MUTATIONS=14 CAUGHT=14 SURVIVORS=0
API_COMPAT_SELF_TESTS=2420  SYMBOL_GRAPH_SELF_TESTS=17  AUDIT_SELF_TESTS=80
RESOURCE_STRINGS_REPRODUCED=29  XNA_RESOURCE_STRING_PROJECTIONS=29
LANGUAGE_MAPPING_MISMATCH=0
TOTAL_DIAGNOSTICS unchanged at 208 — this milestone adds no member, it
repairs two that were already counted as present
template: requested=600 updates=600 draws=600 viewport=800x480
```

## What this milestone found next

The same reading, generalised: for every member the projection implements, list
the resource keys its pinned IL raises and check the message is reproduced.
The first version of that audit looked each key up in its own assembly's string
table and reported no gaps for the entire Graphics assembly — which has no
string table at all, because `Microsoft.Xna.Framework.Graphics.Resources`
resolves against `Microsoft.Xna.Framework.dll`'s. A false negative across a
whole assembly, from one wrong assumption about where a table lives.

Corrected, it finds a fourth member that disagrees with the pinned IL:
`DrawableGameComponent.GraphicsDevice` raises `MissingGraphicsDeviceService`
where XNA raises `PropertyCannotBeCalledBeforeInitialize`. That type has
exactly two message sites and the projection uses the same message at both.
`SpriteBatch`'s begin/end state machine — `EndMustBeCalledBeforeBegin`,
`BeginMustBeCalledBeforeEnd`, `BeginMustBeCalledBeforeDraw` — is absent
entirely.

Four divergences in three milestones, all found by reading IL rather than by a
failing test, is enough of a pattern to make the reading a standing gate rather
than a fourth one-off.
