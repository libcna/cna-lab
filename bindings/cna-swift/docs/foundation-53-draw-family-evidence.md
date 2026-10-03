# Foundation 53: the SpriteBatch Draw family, and the limit of what can be seen

Five missing `Draw` overloads, each also an `OVERLOAD_MAPPING_MISMATCH`, so
closing them removed **ten** diagnostics: `TOTAL_DIAGNOSTICS` 191 → 181,
`MISSING_MEMBER` 78 → 73, `OVERLOAD_MAPPING_MISMATCH` 13 → 8.

## Two families, and the flag that separates them

Every one of XNA's seven `Draw` overloads builds a `Vector4` and calls the
private `InternalDraw` with a `scaleDestination` flag:

```text
position family     (X, Y, scaleX, scaleY)    ldc.i4.1
destination family  (X, Y, Width,  Height)    ldc.i4.0
```

That flag is the entire difference between them, and CNA splits its commands
along exactly the same line:

```text
CNA_SpriteScaledCommand   position + scale     cna_sprite_batch_submit_scaled_many
CNA_SpriteCommand         destination rect     cna_sprite_batch_submit_many
```

So each family maps onto its own command with no arithmetic invented in
between. The alternative — folding the destination family into the scaled
command — would have meant fabricating a scale by dividing a destination
rectangle by a source size, which is a computation XNA never performs and
which has nothing to divide by when the source rectangle is absent.
`testADestinationDrawNeedsNoSourceRectangle` is that argument as a test.

`cna_sprite_batch_submit_many` and its mirrored `CNA_SpriteCommand` are the
milestone's only new boundary: 83 routes → 84, 26 layouts → 27, 222 fields →
232.

## The absent source rectangle

CNA states the convention this projection was already relying on, in the
`CNA_SpriteScaledCommand` header:

> A rectangle of zero width **and** zero height draws the whole texture, which
> is what an empty optional means to the canonical call.

So `nil` → a zeroed rectangle is documented, not assumed. **A present rectangle
of zero size is a divergence**: XNA would sample an empty region and draw
nothing, and CNA cannot be told the difference through this command. That is
recorded in the code where the conversion happens rather than left for someone
to discover.

## What this environment cannot see

Three more mutations were written for this milestone — a uniform scale written
to one axis, a transposed destination rectangle, and an absent source rectangle
sent as something other than zero-by-zero — and **all three survived a full
run**.

The reason is not a missing test. It is that nothing here can see what a sprite
command contains:

```text
build-probe/f53_readback.c   back-buffer readback  -> CNA_RESULT_NOT_SUPPORTED (6)
build-probe/f53_rtread.c     render-target readback -> CNA_RESULT_NOT_SUPPORTED (6)
```

The qualified HEADLESS artifact has no pixel readback of any kind, and CNA
accepts every field value those mutations produce, so the call returns zero
either way. The first probe returned `CNA_RESULT_BUFFER_TOO_SMALL` (14) rather
than `NOT_SUPPORTED`, which looked like a refusal to rasterize and was not —
worth re-reading a result code before drawing a conclusion from it.

The three are withdrawn, with that measurement written where they stood. What
remains covered is the structure rather than the payload: which command shape
each overload uses, and the begin/end rule every overload obeys —
`destination-draw-unguarded` is caught, because the new family could easily
have been written without the guard the old one has.

**This bounds every drawing milestone that follows, not just this one.** No
test in this environment can assert that a pixel ended up anywhere. Claims
about geometry, blending, sampling or sort order are unverifiable here and must
be recorded as such rather than asserted from a call that returned zero.

## Result

```text
591 tests, 0 failures
PROJECTION_MUTATIONS=97 CAUGHT=97 SURVIVORS=0 (three withdrawn as unobservable)
BOUND_FUNCTIONS=84 LAYOUTS=27 LAYOUT_FIELDS=232 ABI_MISMATCHES=0
TOTAL_DIAGNOSTICS=181 MISSING_MEMBER=73 OVERLOAD_MAPPING_MISMATCH=8
```
