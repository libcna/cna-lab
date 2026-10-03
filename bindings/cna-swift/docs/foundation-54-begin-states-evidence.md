# Foundation 54: `Begin`'s states, and when they reach the device

Two more `Begin` overloads. `TOTAL_DIAGNOSTICS` 181 → 177, `MISSING_MEMBER`
73 → 71, `OVERLOAD_MAPPING_MISMATCH` 8 → 6. The two that remain take an
`Effect`, which is not projected.

## A verdict corrected

Foundation 53's notes called these overloads `BLOCKED_UPSTREAM`, on the strength
of `cna_sprite_batch_begin`'s own doc comment:

> This initial slice always uses AlphaBlend, LinearClamp,
> DepthStencilState.None, CullCounterClockwise, the identity transform and no
> custom effect.

That describes *that route*, not the API. Two neighbours exist —
`cna_sprite_batch_begin_with_states` and `cna_sprite_batch_begin_with_effect` —
and the first takes exactly the four descriptors this projection has been able
to build since Foundation 45. The lesson is cheap to state and was not free to
learn: grep the neighbouring symbols before calling something upstream-blocked.

## Where the states are applied, and when

XNA's core `Begin` stores six fields and calls `SetRenderState` **only for
`Immediate`**. Every other sort mode applies them at `End`:

```text
Begin: … store fields …
       if (sortMode == Immediate) { SetRenderState(); immediateCount++; }
       inBeginEndPair = true; beginCount++;

End:   if (spriteSortMode != Immediate) SetRenderState();
       else immediateCount--;
       … flush …
```

`SetRenderState` writes to the **device**, with four `??` defaults, each an
`ldsfld` of a preset in its null branch:

```text
_parent.BlendState        = blendState        ?? BlendState.AlphaBlend
_parent.DepthStencilState = depthStencilState ?? DepthStencilState.None
_parent.RasterizerState   = rasterizerState   ?? RasterizerState.CullCounterClockwise
_parent.SamplerStates[0]  = samplerState      ?? SamplerState.LinearClamp
```

Both halves of the timing are observable and both are tested. A Deferred batch
leaves `GraphicsDevice.BlendState` alone between `Begin` and `End`; an
Immediate one changes it at `Begin`. And an Immediate `End` must **not** put
the batch's state back — a caller who changes the device's state inside the
pair keeps that change, which is the test that finally caught the mutation
making `End` unconditional.

The state writes go *through* the projected device members rather than around
them, so a state handed to `Begin` is bound and read-only afterwards, exactly
as it is in XNA. `testAStatePassedToBeginBecomesReadOnly` asserts the pinned
`BoundStateObject` message comes back from a later write.

## Why CNA still has to be told at `Begin`

Because its plain begin overwrites the device's blend state with its own
default. Measured rather than assumed:

```text
build-probe/f54_beginstate.c
  initial          color_src=0 color_dst=1   (One/Zero -- opaque)
  after set custom color_src=4 color_dst=0   (the custom state)
  plain begin  -> 0
  after begin      color_src=0 color_dst=5   (neither -- CNA's own default)
```

So the resolved descriptors cross the boundary at `Begin` through
`cna_sprite_batch_begin_with_states`, while the *managed* device state moves at
XNA's own moment. The two stay consistent because the managed device state is a
cache on `RuntimeState` — Foundation 45's — and that cache is what
`GraphicsDevice.BlendState` reads.

`cna_sprite_batch_begin` is now **unbound**: no projected member consumes it
once `Begin()` forwards through the state overload, and a test is not a member.
The Foundation 50 test that pins CNA's own begin/end refusal was retargeted at
the route the projection actually uses. Its mirrored `CNA_SpriteBatchBeginInfo`
went with it, taking the layout wall from 27 structures to 26. Net: 84 routes.

## The device token that could not be stored

The first version of `SetRenderState` applied the states through
`GraphicsResource._parent` — the stored facade, as XNA does. The sixty-frame
canary failed with *"The native operation requires an active Game lifecycle
callback"*, and it was right to: a `GraphicsDevice` here is a per-callback
capability token, measured in `build-probe/f42b_identity.c`, so a batch created
in `LoadContent` holds a token that is stale by the time `End` runs in `Draw`.

A current token is borrowed instead. There is one device per game, so it is the
same device XNA's `_parent` would have been — but the reason the stored one
cannot be used is a property of this binding, not of XNA, and is recorded where
the borrow happens.

## Four parameters that are Optional on evidence

`optionalReferenceParameters` grows from 26 entries to 30. The rule it enforces
is that a reference parameter becomes Optional **only** where null is an
observable selected operation rather than an immediate invalid argument, and
`SetRenderState`'s IL settles it for all four: each is `brfalse`-tested and the
null branch reaches a normal `ldsfld` of a preset, not a `throw`. Passing null
selects a different, well-defined state, and a caller can see which one the
device ends up with.

## Falsifiability

Six mutations, 97 → 103:

```text
deferred-batch-applies-its-states-at-begin   the Immediate branch made unconditional
immediate-batch-defers-its-states            End re-applying what Begin already set
null-blend-state-defaults-to-opaque          the wrong preset in the null branch
null-depth-state-defaults-to-default         likewise, on a different state
sampler-applied-to-the-wrong-slot            slot one instead of slot zero
render-state-applied-around-the-device-members  nothing binds, nothing caches
```

The second survived its first spot check, because applying the same states
twice reaches the same place. It is caught by the test above, which changes the
device's state inside the pair and looks at what survives `End`.

## Result

```text
597 tests, 0 failures
PROJECTION_MUTATIONS=103 CAUGHT=103 SURVIVORS=0
BOUND_FUNCTIONS=84 LAYOUTS=26 LAYOUT_FIELDS=228 ABI_MISMATCHES=0
TOTAL_DIAGNOSTICS=177 MISSING_MEMBER=71 OVERLOAD_MAPPING_MISMATCH=6
MESSAGE_COVERAGE_FINDINGS=0 over 1,255 implemented members
```
