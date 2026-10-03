# Foundation 63 — vertex and index buffer binding

```text
MISSING_MEMBER  57 -> 52       TOTAL_DIAGNOSTICS 157 -> 152
BOUND_FUNCTIONS 107 -> 109     LAYOUTS 32 -> 33      ABI_MISMATCHES=0
MESSAGES_REPRODUCED 123 -> 130
tests           654 -> 662
```

Five members: `SetVertexBuffer` twice, `SetVertexBuffers`, `GetVertexBuffers`
and `Indices` with its writer.

## Why the draws are not in this milestone

`GraphicsDevice.VerifyCanDraw` reads the state tracker's vertex- and
pixel-shader slots and raises `CannotDrawNoShader` when either is null, so in
XNA **every** draw without an applied `EffectPass` throws. `Effect` is not
projected, so no consumer of this binding could ever satisfy that check.

CNA agrees, and says so in as many words. `build-probe/f63_draw.c`:

```text
draw with no vertex buffer bound     -> 12
draw with a vertex buffer, no shader -> 12
      GraphicsDevice::DrawPrimitives: no effect has been applied.
draw_user_primitives, no shader      -> 12
      GraphicsDevice::DrawUserPrimitives: no effect has been applied.
```

Projecting the draw family now would ship eight members that always fail, and
fail on the *runtime* channel with a native result code where XNA raises
`InvalidOperationException(CannotDrawNoShader)`. That is worse than an honest
absence, so the draws wait for `Effect` and the binding half — which does not
depend on a shader — lands on its own.

The same probe measured the binding half working:

```text
set_vertex_buffer            -> 0
get_vertex_buffer_count      -> 0  count=1
get_vertex_buffer            -> 0  same handle
copy_vertex_buffers          -> 0  count=1, handle matches, offset 0, frequency 0
set_vertex_buffer(invalid)   -> 0
count after unbind           -> 0
```

## The cache is a reproduction, not an invention

XNA keeps `currentVertexBuffers` and `_currentIB` as fields; `GetVertexBuffers`
copies the array and `get_Indices` is a bare field read. Both hand back **the
objects that were bound**.

CNA can say which native handle occupies a slot, but its header states plainly
that it publishes no route from a native object back to a handle, and prescribes
the remedy: *"cache what you bind and answer from the cache, and use `bound` to
tell 'something else owns this slot now' from 'the slot is empty'."* XNA's
`DeviceResourceManager` is that cache.

So the managed bindings live on `RuntimeState` — not on the facade, because a
facade is a per-callback capability token (Foundation 42) — and the tests assert
**object identity**, which is the one thing a handle-keyed lookup could not give.

## The device-identity test is the runtime, not the facade

`SetVertexBuffers` refuses a buffer created on a different device:

```text
if (binding._vertexBuffer.GraphicsDevice != this)
    throw new InvalidOperationException(InvalidDevice);
```

Comparing facades here would be wrong in a way that looks right: two facades of
the *same* device are different objects, so a buffer created in `LoadContent`
and bound in `Update` would be refused. The identity that survives a callback
boundary is the `RuntimeState`, and that is what is compared.
`testABufferBindsInALaterCallbackThanItWasCreatedIn` creates a buffer in
`LoadContent`, binds it in `Update`, and asserts the binding holds — and
`device-identity-compared-by-facade` is the mutation that proves the test bites.

## What each member reproduces

`SetVertexBuffer(vertexBuffer)` and `SetVertexBuffer(vertexBuffer, vertexOffset)`
each build a `VertexBufferBinding` and hand it to the private
`SetVertexBuffers(binding*, 1)`; a **null** buffer calls
`SetVertexBuffers(null, 0)` and unbinds every stream. That null branch reaches a
normal `ret`, which is why both parameters are Optional — a gap
`docs/foundation-23-parameter-nullability-observations.md` recorded when the
members were still absent, and selected here under the same unchanged rule.
`optionalReferenceParameters` goes from 30 entries to 32.

Because the offset overload builds a `VertexBufferBinding`, the offset is
validated against the buffer's own vertex count **before** the device sees it,
and a refused call leaves the previous binding untouched. Both are asserted.

The private `SetVertexBuffers` checks the stream count against the profile's
`MaxVertexStreams` — 16 on Reach, from the extracted table — and raises
`ProfileMaxVertexStreams` naming the profile and the limit. Its null-buffer test
is unreachable through a Swift `VertexBufferBinding`, whose stored buffer is
non-Optional and whose three constructors all require one; that is recorded in
`recorded-message-absences.json` rather than written as code that cannot run.
The *array* being null is a different thing entirely — it is the selected
unbind-all operation, and it is implemented.

`set_Indices` checks the device, then the buffer, then short-circuits on
identity before touching the device. The short-circuit is reproduced: setting
the buffer that is already bound reaches no native route.

## A mutation that survived, and was withdrawn rather than kept

`vertex-offset-not-carried` — dropping the binding's vertex offset on the way to
CNA — was planted and **survived**. Nothing in the current public surface
observes the offset the *device* received: `GetVertexBuffers()` reads the
managed cache exactly as XNA's does, and the only thing that would notice a
dropped offset is a draw. `cna_graphics_device_copy_vertex_buffers` would
answer, but binding a route for a test alone is what `docs/native-abi.md`
forbids.

So the mutation is withdrawn with the reason written where it stood, and the
claim "the offset reaches the device" is recorded as **not yet evidence**. It
becomes falsifiable when the draw family lands, and the mutation goes back then.

Six mutations remain, all planted and all caught: the binding cache not
updated, the identity lost, the device compared by facade, the stream limit
unchecked, the index cache not updated, and the index setter skipping its
disposal check.
