# Foundation 65 — the render-target family, and a retain cycle nothing was asking about

```text
COMPLETE_TYPES   158 -> 160     MISSING_TYPES  93 -> 91
TOTAL_DIAGNOSTICS 150 -> 144    MISSING_MEMBER 52 -> 49
TARGET_MEMBERS  2049 -> 2065    OVERLOAD_MAPPING_MISMATCH 5 -> 4
BOUND_FUNCTIONS  119 -> 123     LAYOUTS 39 -> 41     ABI_MISMATCHES=0
MESSAGES_REPRODUCED 167 -> 180  MEMBERS_READ 1310 -> 1325
tests            678 -> 693     PROJECTION_MUTATIONS 168 -> 177
```

`RenderTargetCube`, `RenderTargetBinding`, `SetRenderTarget`'s cube overload,
`SetRenderTargets` and `GetRenderTargets`. The whole family binds on the
qualified artifact, so unlike Foundation 64's cube *transfer* nothing here is
renderer-blocked — but what was always blocked still is: **no pixel that reaches
a target can be read**, and nothing below asserts a rendered result.

## The defect this milestone actually found

**Binding one vertex buffer leaked the entire runtime**, and had done since
Foundation 63.

A `GraphicsResource` holds its `GraphicsDevice` facade strongly, and the facade
holds the `RuntimeState` strongly. So any cache *on the runtime* that holds a
resource closes a cycle:

```text
RuntimeState -> RenderTargetBinding/VertexBufferBinding -> resource
             -> GraphicsDevice facade -> RuntimeState
```

Foundation 63 added the first such cache. After `Game.Dispose`, the runtime —
and with it every native handle it tracks and every registered child — stayed
alive for the life of the process.

Nothing caught it, and nothing was going to. A retain cycle is not a leak in
AddressSanitizer's sense: everything is still reachable, from itself. Eleven
clean ASan runs said nothing about it. `swift test` said nothing about it. It
was found by *asking the question the design raised* — this milestone needed a
render-target cache of exactly the same shape, and the question was whether that
shape was safe.

`RuntimeState.invalidateAfterNativeShutdown` now releases every cache that holds
a resource. That is not a Swift patch bolted on: XNA's `GraphicsDevice.Dispose`
releases `currentVertexBuffers`, `_currentIB` and `currentRenderTargetBindings`
along with the device, and after this point there is no device for anything to
be bound to. `SamplerStateCollection` was already exempt, having held its device
weakly since Foundation 46 for precisely this reason.

`RuntimeLifetimeTests` is what asks, and it is written as **two controls plus
the case**: a game that binds nothing and a game with an unbound buffer must
both release the runtime, so a green case cannot be green because the
measurement is broken. `bound-resources-not-released-at-shutdown` is the
mutation, and it is caught.

## RenderTargetCube does not read the cube route

`cna_texturecube_get_info` **accepts** a render-target-cube handle and answers
`CNA_RESULT_SUCCESS` — with size 0, level count 0 and format 0
(`build-probe/f65_rtcube.c`):

```text
render_target_cube_create(4)     -> 0
   rtcube info=0 kind=2 4x4 levels=1 fmt=0 depth=0 ms=0 usage=0 lost=0 avail=1
   texturecube_get_info(rtcube)  -> 0 size=0 levels=0 fmt=0
```

A projection that read `Size` through the inherited path would report a
zero-sized cube and **no call would have failed**. `cna_render_target_get_info`
answers the real 4×4, one level, `Color`, so that is what this type reads, and
the divergence is recorded in `docs/runtime-capabilities.json` rather than
worked around silently. `cube-target-reads-the-cube-info-route` is the mutation.

Beyond that the type is `RenderTarget2D` with `TextureCube` as its base: the same
single inherited handle, the same three read-back description properties, the
same `IsContentLost` divergence, and the same subscription released before the
base releases the handle. The two subscribe through **one** shared
implementation now — `RenderTargetContentLostSubscription` — because the two
cannot share a base class and a defect duplicated in two places is a defect
twice.

`RenderTargetCube.CreateRenderTarget` runs `TextureCube.ValidateCreationParameters`
against the device's profile, so a 1024 cube target and a non-power-of-two one
are refused with the same messages a plain `TextureCube` is refused with.

## What the array binder validates

The private `SetRenderTargets(RenderTargetBinding*, Int32)`, in order:

```text
if (count == currentRenderTargetCount
    && every binding's target AND face match the current ones) return;
if (count > MaxRenderTargets) Throw(ProfileMaxRenderTargets, MaxRenderTargets);
for (i = 0; i < count; i++) {
    Texture t = bindings[i]._renderTarget;
    if (t == null) throw new ArgumentException(NullNotAllowed);
    Helpers.CheckDisposed(t, t.GetComPtr());
    if (t.GraphicsDevice != this) throw new InvalidOperationException(InvalidDevice);
    if (i > 0) {
        for (j = 0; j < i; j++)
            if (t == bindings[j]._renderTarget)
                throw new ArgumentException(CannotSetAlreadyUsedRenderTarget);
        if (!RenderTargetHelper.IsSameSize(t, bindings[0]._renderTarget))
            throw new ArgumentException(RenderTargetsMustMatch);
    }
}
```

Three things in that worth stating plainly:

**The profile message names the limit, not the request.** `V_24` is
`MaxRenderTargets` and it is what is boxed — the same shape
`ProfileMaxVertexStreams` has.

**The duplicate and same-size tests are `i > 0` only**, so a one-element array
reaches neither. That is why both single-target overloads share this validator
without a special case.

**`IsSameSize` compares four things, not two**: both extents, the multisample
count, and the colour format's **byte size** — so two targets in `Color` and in
`Rgba1010102` are the *same* size to it, which is what the message means by "the
same multisample type and bit depth".

The null-target test has no reachable input through a Swift
`RenderTargetBinding`, whose stored target is non-Optional and whose both
constructors require one; it is recorded rather than written as dead code.

The device-identity test compares the **`RuntimeState`**, not the facade, for
Foundation 63's reason — a facade is a per-callback capability token, so
comparing facades would refuse every target created in one callback and bound in
another. `testATargetBindsInALaterCallbackThanItWasCreatedIn` is the test and
`render-target-device-compared-by-facade` is the mutation.

## Reach refuses what the artifact would accept

`build-probe/f65_rtcube.c` binds **two distinct 8×8 targets** and reads both
back. XNA does not allow it: Reach's `MaxRenderTargets` is 1, so the projection
refuses at two and the native capability is never reached. That is worth
recording precisely because it is the direction that is easy to get wrong —
the host is more capable than the API here, and the API wins.

The artifact's own refusals were measured too, and all three are documented
rather than assumed:

```text
set_render_targets(2, same rt2)  -> 12  the same render-target subresource cannot
                                        be bound to more than one slot
set_render_targets(slice 1)      ->  6  RenderTarget2D array slices are not
                                        supported; the array slice must be 0
set_render_targets(2d, face 2)   ->  1  A RenderTarget2D binding must name the
                                        positive-X face
destroy while bound              ->  3  INVALID_STATE
```

The projection sets `array_slice` to zero for both kinds and passes the
binding's own face, so none of the three is reachable through it.

## The single-target overloads keep the dedicated route

XNA's `SetRenderTarget(RenderTarget2D)` and `SetRenderTarget(RenderTargetCube,
CubeMapFace)` each build a one-element array and call the private array method.
This projection runs the same validator and then calls
`cna_graphics_device_set_render_target2d` / `_set_render_target_cube` rather
than the array route — and that is a **measured** equivalence:

```text
set_render_target2d(rt2)         -> 0
   after single 2d:   count=1 copy=0 got=1 matches=1 face=0
set_render_target_cube(rtc, f2)  -> 0
   after single cube: count=1 copy=0 got=1 matches=1 face=2
set_render_target2d(invalid)     -> 0
   after unbind:      count=0
```

Identical state to the array route. Routing the single overloads through the
array route would leave the two dedicated ones with no consuming member, which
`docs/native-abi.md` forbids.

**The probe lied once before it said this.** Its first draft read
`get_render_target_count`'s out-parameter in the same `printf` that filled it,
so every count printed as the 12345 sentinel — the same mistake Foundation 59
recorded a rule about, made again. The rule is now written into the probe beside
the fix.

## Falsifiability

Nine new mutations, each planted and each caught: the cube reading the wrong
info route, the cube target skipping the profile checks, its subscription not
released, a 2D binding carrying a face, the binding cache not updated, the
target limit unchecked, the device compared by facade, the pixel size ignored,
and the retain cycle reopened. Four existing sites drifted where
`currentRenderTarget` became the binding array and were re-aimed rather than
deleted.

**Three were withdrawn**, with the reason written where each stood:

* `render-target-identity-lost` was a **no-op** — a `RenderTargetBinding` is a
  struct with no identity of its own, so rebuilding one around the same target
  is indistinguishable from returning it. Replaced, not scored. And
  `testTheReturnedArrayIsACopy` asserts a guarantee no mutation can falsify
  either: a Swift `Array` is a value and simply cannot alias the cache. The test
  documents the language, and says so.
* `render-target-binder-skips-the-disposal-check` survived because the check is
  **redundant on every reachable input** — the handle is validated again where
  it is used and raises the identical exception. It stays because XNA has it and
  because its *order* matters, ahead of the device-identity test; observing that
  ordering needs two devices and CNA runs one game at a time.
* `cube-face-not-carried-to-the-device` survived for exactly the reason
  `vertex-offset-not-carried` was withdrawn in Foundation 63: `GetRenderTargets`
  reads the managed cache, as XNA's does, and nothing public observes the face
  the *device* received. `cna_graphics_device_copy_render_targets` does answer
  it — the probe reads face 3 back — but binding a route for a test alone is
  forbidden.

`same-size-test-ignores-the-pixel-size` survived too, and was **fixed rather
than withdrawn**. Only `SurfaceFormat.Color` can be created on this host and the
artifact grants a multisample count of zero, so no two render targets this
binding can construct differ in either field the extents do not already
separate. The four-way comparison is now a function of four numbers,
`sameRenderTargetShape`, and is asserted directly — the arithmetic is the claim,
so the arithmetic is what is tested.

`PROJECTION_MUTATIONS=177`; the full run is repeated before the final handoff.
