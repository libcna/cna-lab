# Foundation 66 — `TextureCollection`, and three checks that had nothing to check

```text
COMPLETE_TYPES   160 -> 161     MISSING_TYPES  91 -> 90
TOTAL_DIAGNOSTICS 144 -> 141    MISSING_MEMBER 49 -> 47
BOUND_FUNCTIONS  123 -> 125     LAYOUTS 41 -> 42     ABI_MISMATCHES=0
MESSAGES_REPRODUCED 180 -> 189  MESSAGES_DEFERRED 10 -> 4
RESOURCE_STRINGS_REPRODUCED 59 -> 62
tests            693 -> 704     PROJECTION_MUTATIONS 177 -> 189
```

The last type standing between this binding and `Effect`. It also closes three
recorded absences at once, because all three were waiting for the same thing:
something to be *bound*.

| absence | why it was unreachable | now |
| --- | --- | --- |
| `MustResolveRenderTarget` in `TextureCollection.set_Item` | the type did not exist | implemented |
| `MustResolveRenderTarget` in `Texture2D`/`TextureCube` `CopyData` | `isActiveRenderTarget` was never set | implemented |
| `ResourceInUse` in all three `CopyData`s | nothing could bind a texture to a sampler | implemented |

`MESSAGES_DEFERRED` goes 10 → 4, and the four that remain are not a family.

## The getter answers from a cache, and CNA asked for exactly that

`cna_graphics_device_get_texture` reports whether a slot is occupied and, when a
C caller owns the texture, which handle is in it. What it cannot do is hand back
an **object**, and the header does not merely omit that — it argues the point:

> **There is deliberately no route from a native object back to a handle**, here
> or anywhere else in this ABI … Reversing the direction would mean either a
> process-wide native-pointer-to-handle map, which would keep every object a C
> caller ever saw alive forever and answer with a stale handle after any reuse
> of the address, or a slot on every canonical graphics type for a C concept
> that has no business being there. Neither is worth what it buys.
>
> The practical consequence, for a consumer whose own `Textures[i]` getter must
> return the object it set: cache what you bind and answer from the cache, and
> use `bound` to tell "something else owns this slot now" from "the slot is
> empty".

XNA's own getter calls `Texture2D.GetManagedObject` — which *is* such a map. So
the cache reproduces what XNA does rather than working around what CNA lacks,
and `bound` is read too, because it covers the one case a cache cannot: a slot
filled by canonical CNA code reports occupied with no handle, and this
collection answers `nil`, because the object there is not one any caller of this
binding ever held.

## Reach has no vertex samplers, and that changes two answers

`MaxVertexSamplers` is **0** on Reach in the extracted table, so
`VertexTextures` has no slots at all and every index is out of range — including
zero. CNA would have taken them: `cna_graphics_device_set_texture` accepts slots
0 through 15 on the vertex stage too (`build-probe/f66_slots.c`). The profile is
the narrower rule and is the one that decides.

**The reader and the writer then report different things, and that is XNA's
order rather than an inconsistency.** `set_Item` checks the *value* before the
*index*:

```text
if (value != null) {
    Helpers.CheckDisposed(value, value.GetComPtr());
    if (value.isActiveRenderTarget)
        throw new InvalidOperationException(MustResolveRenderTarget);
    if (_textureOffset > 0 && !value.pStateTracker->validVertexTexture)
        Throw(ProfileVertexTextureFormatNotSupported, value.Format);
}
if (index < 0 || index >= _maxTextures)          // IL_0088, after both
    throw new ArgumentOutOfRangeException("index");
```

Reach's `ValidVertexTextureFormats` is empty, so every texture fails the format
rule *before* its slot number is looked at. A Reach caller binding into the
zero-slot vertex collection is told about the **format**, not the slot. Only a
`nil` value skips the value tests and reaches the bounds check. `get_Item` has
no value to check and reports the index.

Both of this milestone's test failures were the same mistake in the opposite
direction — tests asserting an order XNA does not have. The first asserted the
index here; the second asserted that `MustResolveRenderTarget` preceded the
empty-array test in `CopyData`, when `data == null || data.Length == 0` is
`IL_0015` and the render-target test is `IL_0022`. The code was right in both
cases and the tests were corrected; the doc comment claiming the vertex-format
rule was "unreachable on Reach" was wrong and was corrected with them.

## A recorded divergence that stopped being necessary

All four collections — `Textures`, `VertexTextures`, `SamplerStates` and
`VertexSamplerStates` — are now sized from the device's own profile. Until this
milestone the two sampler collections were **16 slots each**, with a recorded
divergence whose stated reason was:

> XNA's vertex collection is 0 long under Reach and 4 under HiDef, and **this
> binding has no profile selection**, so both collections are the length CNA
> actually accepts.

Foundation 62 made that untrue and nothing went back to check. A 16-slot vertex
sampler collection accepted fifteen indices XNA raises
`ArgumentOutOfRangeException` for, on a profile with no vertex samplers at all,
and the bounds check is observable. The divergence is resolved rather than
re-recorded, and `texture-collections-not-profile-sized` is the mutation that
keeps it resolved.

Worth stating as a rule: **a divergence recorded because something was missing
has to be re-read when that thing arrives.** Nothing in the gate set does that;
it took writing a second collection of the same shape to notice.

## The three checks, and what CNA does about them

`Texture.isActiveRenderTarget` is set on every target the device binds and
cleared on every one it unbinds, from the single place both happen. Three
members read it, so there is one writer and three readers rather than three
copies of the rule.

CNA enforces the sampling half itself, and says so in as many words
(`build-probe/f66_slots.c`):

```text
set_texture(ACTIVE target)     -> 3
      A texture that is currently bound as a render target cannot be bound for
      sampling.
```

The two agree. The managed check runs first so the message is XNA's, which is
the same shape every other agreed refusal in this binding takes.

`ResourceInUse` is `E_ABORT` — `0x80004004` — and `Helpers.GetExceptionFromResult`
maps it to `new InvalidOperationException(ResourceInUse)`, so what a caller sees
is an ordinary `InvalidOperationException` whose message is about `SetData`, not
a COM code. The scan is guarded by `isSetting`, so **`GetData` on a bound
texture is allowed**, and a test asserts that rather than assuming it.

The cube's transfer is scanned too, and that is the one worth spelling out: this
renderer refuses a cube transfer anyway with `NOT_SUPPORTED`. The managed
refusal has to come **first**, or the caller would see the renderer's failure on
the runtime channel where XNA raises `InvalidOperationException`. The test
asserts both outcomes — the native code when unbound, the managed exception when
bound — so the ordering is what is measured, not just the exception.

## Falsifiability

Twelve new mutations and four re-aimed sites, all planted and all caught.

`disposed-texture-stays-in-the-collection` survived **twice** before the test
was right, and both rounds were informative. The first assertion read
`Textures[i]` and found `nil` either way, because CNA unbinds a destroyed
texture itself and `Item` asks the device. The second read the cache directly —
but *after* `Item`, which corrects the cache on its way past. Only reading
`holds` **before** any `Item` call separates the two mechanisms, and that is the
claim: a caller who never reads a slot gets no correction, and the collection
would go on holding a strong reference to a dead texture.

Both failures were a test measuring the wrong thing, not the code being wrong —
which is the same shape as the two ordering mistakes above, and the reason a
surviving mutation is worth more than a passing test.

`PROJECTION_MUTATIONS=189`; the full run is repeated before the final handoff.
