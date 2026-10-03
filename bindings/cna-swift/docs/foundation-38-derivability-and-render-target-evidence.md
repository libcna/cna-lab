# Foundation 38 — derivability, the graphics resource bases, and `RenderTarget2D`

`NONDERIVABLE_UNSEALED_CLASSES` was 5. It is 0, and the record that counted it
is now a rule the verifier enforces.

## 1. The five classes, decided one at a time

The strict report named five classes XNA leaves derivable and this projection
had sealed. Unsealing a public class is an API decision, so each was taken on
its own evidence rather than by a blanket edit.

| Class | CLR | Decision |
|---|---|---|
| `GameTime` | not sealed, 3 public constructors | `open`. Nothing about it is native: three stored values and three constructors, so a subclass costs the projection nothing. |
| `SpriteBatch` | not sealed, 1 public constructor | `open`, **and rebased on `GraphicsResource`**, which is where its `Dispose(Boolean)` override point actually is. |
| `Texture2D` | not sealed, 2 public constructors | `open`, **and rebased on `Texture`**. This is the one that mattered: `RenderTarget2D` derives from it and was inexpressible while it was `final`. |
| `GraphicsDeviceManager` | not sealed, 1 public constructor | `open`. Its public `GraphicsDeviceManager(Game)` constructor is projected, so a consumer can genuinely derive from it. |
| `GraphicsDevice` | not sealed, 1 public constructor | `open` with construction still `private`. XNA's own constructor is not yet projected, so the class is formally derivable and not yet constructible from outside — the honest state of a runtime-partial type rather than a strengthened one. |

### The rule, promoted from a record

`sealed_class_evidence` diagnosed only one direction: a CLR `sealed` class that
is not a Swift `final` class. The converse was recorded and deliberately not
diagnosed, because five real types failed it and unsealing them was an API
decision rather than a side effect.

With all five decided, the converse is now a diagnostic:

> a class XNA leaves derivable **and gives a public constructor** must not be a
> Swift `final` class.

The condition is narrow on purpose. A class XNA leaves open but gives no
accessible constructor is not derivable outside its own assembly anyway, so a
`final` projection strengthens nothing; that case is still counted and is not
diagnosed. The self-test that required the converse to stay a mere record is
**inverted** rather than deleted, and a third case is added for the
derivable-but-unconstructible class. `API_COMPAT_SELF_TESTS` 2415 → 2416.

## 2. `GraphicsResource` and `Texture`, the two bases

`BASE_MAPPING_MISMATCH` was 2, and both entries were the same shape: XNA gives
`SpriteBatch` the base `GraphicsResource` and `Texture2D` the base `Texture`,
and this projection gave neither anything. It is 0 now.

`GraphicsResource` is where the native handle, its ownership and its
destruction now live, together with the surface all of it carries:
`IsDisposed`, `Name`, `Tag`, `GraphicsDevice`, `Dispose()`, `Dispose(Boolean)`,
`ToString()` and the `Disposing` event. `Texture` adds `LevelCount` and
`Format`.

### Why four of those are managed rather than native

CNA has routes for all of them — `cna_graphics_resource_get_is_disposed`,
`_get_name`/`_set_name`, `_get_tag`/`_set_tag`, `_get_graphics_device`. None is
used, and the reason is the pinned fallibility verdicts: `get_IsDisposed`,
`get_Name`, `set_Name`, `get_Tag`, `set_Tag` and `get_GraphicsDevice` are every
one of them `IL_NO_FAILURE_PATH`, so their Swift projections **must not
throw** — and a native round trip can always fail on the owner thread or a
stale generation. `Name` and `Tag` are managed fields, which is exactly what
they are in the CLR (`_localName`, `_localTag`), and `GraphicsDevice` is the
device facade the resource was created from, which is what `_parent` is.

`Tag` has a second reason. `cna_graphics_resource_set_tag` takes a
`CNA_GraphicsResourceTag`, documented as a "C-owned opaque tag token" and typed
`uint64_t`. XNA's `Tag` is a `System.Object` reference. Squeezing a Swift object
identity into an integer token would be a fabrication.

`Texture.LevelCount` and `Format` come from `cna_texture_get_info` — read
**once**, while the resource is being constructed, and stored. That is the same
shape `Texture2D.Width` and `.Height` already had, and it is what lets an
infallible CLR getter stay infallible in Swift.

### `Dispose`, in XNA's own order

`Dispose()` is `virtual final` in the metadata — a sealed `IDisposable`
implementation, not an override point — so it is `final` in Swift. Its body is
`Dispose(true)` then `GC.SuppressFinalize(this)`. `Dispose(Boolean)` is
`family newslot virtual`: the one override point of the family.

The base body reproduces `~GraphicsResource()` exactly, and the order is
observable: nothing at all when already disposed, otherwise **release the
native resource and only then raise `Disposing`**. A handler therefore sees
`IsDisposed == true`. A mutation that swaps those two statements is caught.

## 3. `RenderTarget2D`

### CNA agrees with the inheritance, and that was measured

A pure-C probe (`build-probe/f38_rendertarget.c`, no Swift at all) established
the substitutability before any Swift was written:

```text
cna_render_target2d_create              = 0   handle 4294967300
cna_render_target_get_info              = 0   kind=2D 64x32 levels=1 fmt=Color
                                              depth=None ms=0 usage=Discard
                                              lost=0 renderer_available=1
cna_texture2d_get_info   ON THE TARGET  = 0   64x32 levels=1 fmt=Color
cna_texture_get_info     ON THE TARGET  = 0
cna_sprite_batch_submit_scaled_many     = 0   with the target as `texture`
cna_graphics_device_set_render_target2d = 0   count 1 while bound, 0 after
cna_render_target_destroy WHILE BOUND   = 3   CNA_RESULT_INVALID_STATE
cna_texture2d_destroy on a LIVE target  = 0   the two destroy routes are
cna_render_target_destroy after that    = 2   interchangeable for a 2D target
```

So a render-target handle **is** a texture handle to CNA's texture routes.
Substitutability is native here, not simulated by the binding.

`renderer_available` is true even on HEADLESS, so construction, binding and
every property are real. No pixel is claimed: HEADLESS has no window.

> A note on the probe, because the first run of it reported nonsense. Its first
> version read each out-parameter in the same `printf` as the call that fills
> it. C leaves argument evaluation order unspecified and this compiler
> evaluates right to left, so every out-parameter was read *before* the call.
> The result looked exactly like CNA returning success without writing
> anything. The calls are their own statements now.

### One handle, one owner, one destruction path

`Texture2D`'s designated initializer takes the destroy route as a **parameter**
rather than holding `cna_texture2d_destroy` as a constant. A `RenderTarget2D`
passes `cna_render_target_destroy`; an ordinary texture passes the texture one.
There is one `NativeHandleStorage`, held by the base, and the derived class adds
no second storage, no second `Dispose` and no downcast.

The destroy-route choice is about using the documented route, not about
avoiding a leak: the probe measured `cna_texture2d_destroy` releasing a live
render target successfully.

Tested: the target used where a `Texture2D` is expected, `SpriteBatch`
consuming it, disposal through the derived API, base members after disposal,
duplicate `Dispose`, the parent `Game`'s disposal releasing it, and disposal
while bound — which CNA refuses with `CNA_RESULT_INVALID_STATE`, reaching the
caller as a `CNAError` and never as a projected CLR exception.

### The three constructors, and what "preferred" means

The 3-argument constructor's IL pushes five zeros — `mipMap` false,
`SurfaceFormat.Color`, `DepthFormat.None`, multisample 0,
`RenderTargetUsage.DiscardContents` — and the 6-argument one pushes two. Those
defaults are read out of the IL.

Every property is read back from `cna_render_target_get_info` on the target CNA
actually made, never echoed from the request. That is what the `preferred` in
XNA's own parameter names means, and it is why a granted depth format that
differs from the requested one is recorded rather than asserted.

### `IsContentLost`, and a measured divergence

`get_IsContentLost` is `virtual final` — a sealed `IDynamicGraphicsResource`
implementation — so it is `final`, and it is infallible so it does not throw.

XNA latches its `_contentLost` field from `GraphicsDevice.IsDeviceLost` on every
read. There is no `IsDeviceLost` here, and CNA reports content loss by
**notification** instead. The projection therefore latches on CNA's own
`cna_render_target_subscribe_content_lost` callback and on the state the target
reported at construction. On the qualified HEADLESS renderer the value is
always false, because that renderer cannot lose a device — which is CNA's own
documented behaviour and not an omission here.

The subscription is a real native one. Its `void*` addresses a retained box
holding the target **weakly**, so a subscription cannot keep a target alive; the
registration is released in `Dispose(Boolean)` *before* the base releases the
handle, so no native callback can address freed Swift state. A test asserts the
registration is non-zero while live and zero after disposal, and a mutation that
drops the release is caught.

## 4. `GraphicsDevice.SetRenderTarget(RenderTarget2D)`

Bound, with the Optional parameter registered as a measured decision rather
than assumed. XNA's IL null-tests the argument with `brfalse.s` and the null
branch reaches a normal `ret` after `SetRenderTargets(null, 0)` — restoring the
backbuffer, which is a **different and meaningful operation** and not an
immediate invalid argument. That is exactly the selected-operation half of the
Foundation-23 parameter rule, so the parameter is Optional and the entry is in
`optionalReferenceParameters` with its evidence.

## 5. Native boundary

Seven routes, three mirrored structures and one mirrored callback:

```text
cna_texture_get_info                       cna_render_target_get_info
cna_render_target2d_create                 cna_render_target_destroy
cna_graphics_device_set_render_target2d    cna_render_target_subscribe_content_lost
                                           cna_render_target_unsubscribe_content_lost
```

```text
BOUND_FUNCTIONS            29 -> 36     LAYOUTS          18 -> 21
ROUTE_PAIRINGS             29 -> 36     LAYOUT_FIELDS   129 -> 157
PROTOTYPE_TYPE_POSITIONS   91 -> 113    CALLBACKS         2 -> 3
CANONICAL_DECLARATION_CHECKS 91 -> 113  CONSTANTS       212 (unchanged)
C_SWIFT_MEASUREMENTS       91 -> 113    MISMATCHES        0
NATIVE_ABI_MUTATIONS=14 CAUGHT=14 SURVIVORS=0
```

The first run of the new routes failed on two positions the verifier could not
yet spell — `CNA_RenderTargetEventRegistrationHandle` and `void*` — which is
the canonical-declaration check doing its job on the first surface added since
it existed.

## 6. Scoreboard

```text
TARGET_TYPES     142 -> 145     COMPLETE_TYPES   135 -> 138
TARGET_MEMBERS  1767 -> 1785    PARTIAL_TYPES      7 -> 7
MISSING_TYPE     115 -> 112     MISSING_MEMBER   129 -> 128
TOTAL_DIAGNOSTICS 269 -> 262
BASE_MAPPING_MISMATCH        2 -> 0
NONDERIVABLE_UNSEALED_CLASSES 5 -> 0
DEPENDENCY_COMPLETE_MISSING_TYPES 16 -> 22
```

Three complete types (`GraphicsResource`, `Texture`, `RenderTarget2D`), both
base mismatches closed, and six newly dependency-complete graphics types —
`BlendState`, `DepthStencilState`, `RasterizerState`, `SamplerState`,
`TextureCollection` and `VertexDeclaration` — which were blocked on
`GraphicsResource` alone.

## 7. Falsifiability

Five new controls in `tools/projection_mutations/run.py`, all caught:

```text
CAUGHT  texture2d-resealed                Texture2D final again, as it was
CAUGHT  disposing-raised-before-release   Disposing raised before the native release
CAUGHT  dispose-not-idempotent            a second Dispose reaching the dead handle
CAUGHT  derived-type-name-lost            the storage naming the base, not the derived type
CAUGHT  content-lost-subscription-leaked  disposal leaving the native subscription alive
```

`dispose-not-idempotent` **survived its first run**, and the reason was worth
finding: `RenderTarget2D` carries its own already-disposed guard because it has
a subscription to release first, so it short-circuits before the base body ever
runs, and no test reached `GraphicsResource.Dispose(Boolean)`'s own guard. A
test through `SpriteBatch` — which does not override — now does.

### What is NOT falsifiable here, stated rather than implied

A mutation that echoes the *requested* width, format or usage into the
properties instead of the granted ones **survives** on this host, because
HEADLESS grants exactly what is asked. The read-back is the right design and
the test cannot currently tell it from the wrong one; a renderer that refuses a
preference is what would make it falsifiable, and this host is not one.
