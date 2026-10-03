# Foundation 68 — the draw family, five milestones after it was withheld

```text
MISSING_MEMBER    47 -> 38      TOTAL_DIAGNOSTICS 132 -> 123
BOUND_FUNCTIONS  191 -> 197     LAYOUTS 48 -> 50     CONSTANTS 221 -> 225
MESSAGES_REPRODUCED 195 -> 223  MESSAGES_NATIVE_OWNED 28 -> 68
RESOURCE_STRINGS_REPRODUCED 64 -> 69   ABI_MISMATCHES=0
tests            716 -> 730     PROJECTION_MUTATIONS 203 -> 215
```

Nine members: `DrawPrimitives`, `DrawIndexedPrimitives`,
`DrawInstancedPrimitives`, and the six user-primitive overloads.

Foundation 63 withheld all of them on a specific claim; Foundation 67 tested
that claim and it held; this lands the members.

## What a passing test here means

**A draw that returns is a draw the device accepted.** Nothing in this milestone
asserts that a pixel arrived anywhere, because nothing on this host can read one
back: `cna_graphics_device_get_backbuffer_data_window` still answers
`NOT_SUPPORTED`, as it has since Foundation 53. Geometry, blending, sampling and
sort order remain unverifiable and are not claimed.

That is a thin claim, and it is worth stating exactly because the *rest* of the
milestone is not thin. The managed half of every draw is reproduced and is what
the tests actually assert, and the two refusals a caller will really meet are
measured rather than assumed.

## Two refusals XNA never produces, forwarded rather than translated

```text
draw with no effect applied
    -> CNA_RESULT_INTERNAL, "no effect has been applied"
draw from a buffer whose SetData was never called
    -> CNA_RESULT_INVALID_ARGUMENT,
       "The requested primitive range exceeds the bound vertex buffer"
```

XNA raises `InvalidOperationException(CannotDrawNoShader)` for the first, from
`VerifyCanDraw`. It reaches that verdict by reading `pStateTracker` — the
Direct3D state tracker holding the bound shaders, declaration, sampler modes and
textures. CNA publishes no equivalent, so **the native answer is forwarded and a
managed exception is not invented**. Asserting `CannotDrawNoShader` here would
mean claiming a conclusion about a shader pipeline this binding cannot inspect.

The second has **no XNA counterpart at all.** CNA counts vertices *written*, not
capacity allocated; XNA draws undefined contents from an un-`SetData`'d buffer
rather than raising. It is forwarded for a different reason: a managed
pre-check would have to track every byte ever written to every buffer, which is
state this binding has no business keeping and which XNA does not keep either.

Both are asserted — including the native diagnosis text, so a change of message
is a test failure rather than a silent drift.

## What the milestone actually owns

Every managed test, in the order the IL puts it:

| test | members | message |
| --- | --- | --- |
| `Helpers.CheckDisposed` | all nine | `ObjectDisposedException` |
| `primitiveCount <= 0` | all nine | `MustDrawSomething` |
| `primitiveCount > MaxPrimitiveCount` | all nine | `ProfileMaxPrimitiveCount` (65,535 on Reach) |
| `numVertices <= 0` | indexed, instanced | `NumberVerticesMustBeGreaterZero` |
| `instanceCount <= 0`, `> Max` | instanced | the **same two** messages, named `instanceCount` |
| `instanceStreamMask != 0` | the three non-instanced | `NonZeroInstanceFrequency` |
| all-instanced **or** all-plain | instanced | `InvalidInstanceStreams` |
| `vertexData` empty | user | `ArgumentNullException("vertexData")` |
| `vertexOffset` outside the array | user | `OffsetNotValid` |
| offset + element count past the end | user | `MustBeValidIndex`, blaming `primitiveCount` |
| 32-bit indices on Reach | user indexed | `ProfileNoIndexElementSize32` |

Three of those are worth naming.

**The vertex count is tested before the primitive count.** `numVertices <= 0` is
`IL_0018` and `primitiveCount`'s own test is `IL_002d`, so a call wrong in both
ways reports the vertices.

**`instanceStreamMask` is derived, not tracked.** XNA keeps a bitmask that
`SetVertexBuffers` fills from each binding's `InstanceFrequency`; this binding
already holds those bindings, so the mask is computed from them. One fewer thing
that can fall out of step.

**`DrawInstancedPrimitives` needs a *mixture*.** It does not merely tolerate an
instanced stream — it refuses when every bound stream is instanced *or* none is:

```text
allZero = (mask == 0);
allOne  = (mask == (1 << currentVertexBufferCount) - 1);
if (allZero || allOne) throw new InvalidOperationException(InvalidInstanceStreams);
```

An instanced draw needs per-instance data *and* per-vertex data, and either
extreme supplies only half. That message was the last coverage finding of the
milestone and is fully derivable from state this binding holds, so it is
implemented rather than recorded.

## `VerifyCanDraw`'s eight messages, and why three of them are not implemented

Forty-one coverage findings arrived at once — eight messages across the draw
members. Five read shader state outright and are plainly native-owned:
`CannotDrawNoShader`, `CannotDrawNoData`, `CannotMixShader2and3`,
`MissingVertexShaderInput`, `MissingVertexShaderInputDetails`.

The other three are the interesting ones, because they **look implementable**:
`ProfileInvalidBlendFormat`, `ProfileInvalidFilterFormat` and
`ProfileNoWrapNonPow2` compare a bound texture or render target against
`InvalidBlendFormats`, `InvalidFilterFormats` and `NonPow2Unconditional` — all
three pinned in the table extracted in Foundation 62, and the bindings are
cached here since Foundations 65 and 66.

They are still recorded native-owned, and the reason is the point: what decides
them is the **tracker's** applied sampler state and the **tracker's** bound
textures, not this binding's caches. `CNA_TextureSlotInfo` reports a slot as
occupied without naming the object when canonical CNA code owns it — a
`SpriteBatch` flush is exactly that — so a check reading the managed view would
pass a state XNA refuses. **A check that silently does not fire is worse than an
honest absence**, which is why the temptation to implement the three that had
the data available was refused.

## Falsifiability

Twelve mutations, all caught after two rounds. The two survivors were the two
familiar shapes.

`user-draw-element-count-test-is-signed` survived because the unsigned
comparison **cannot be reached through a draw on this profile**: the sum can only
overflow when the element count is enormous, and `primitiveCount` is capped at
65,535 several tests earlier while the offset is capped below the array's length.
The comparison is now a function of its three numbers, `arrayHolds`, tested
directly — the same remedy `sameRenderTargetShape` needed in Foundation 65.

`primitive-type-not-carried-to-the-route` survived for Foundation 63's reason —
no draw's *result* is observable — and this time it was made observable rather
than withdrawn. CNA refuses a draw whose vertex range exceeds what the buffer
holds, and that range depends on the topology: four written vertices are enough
for a `LineStrip` of three primitives and not for a `TriangleList` of three. A
forced topology turns an accepted draw into a refused one, so the pass-through
is falsifiable after all.

**And one assertion in that new test was wrong.** It claimed the unsigned
comparison also catches a negative offset. It does not: read unsigned, `-1 + 1`
wraps to zero and passes. That is precisely why XNA guards `vertexOffset < 0`
separately with `OffsetNotValid` several instructions earlier, and the test now
asserts `true` there and says so — asserting the catch would have been asserting
something neither XNA nor this projection does.

The four `CNA_PrimitiveType` constants are pinned with `_Static_assert`s for the
same reason `CubeMapFace`'s six were: the topology is passed through as a raw
value, and a swap between two enumerations would be invisible to every
observation this host can make. `CONSTANTS` goes 221 → 225.

`PROJECTION_MUTATIONS=215`; the full run is repeated before the final handoff.
