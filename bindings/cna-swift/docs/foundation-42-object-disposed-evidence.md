# Foundation 42a — `System.ObjectDisposedException`, and a CLR-shaped failure that was on the wrong channel

Where XNA guards a graphics resource against use after disposal it raises
`ObjectDisposedException`, and all four state objects' `Apply` open the same
way. This binding reported the same condition as `CNAError.disposedObject` —
the CNA runtime channel — so `catch is CNAException`, which is meant to see
every projected CLR failure, could not see it.

Fixing that needed a BCL authority the binding did not have.

## 1. The family was not admitted

`tools/api_compat/bcl-authorities.json` admitted six raised exception families
and eleven support types. `System.ObjectDisposedException` was not among them,
so there was nothing to project a payload from. It is now the seventh, admitted
out of the same `mscorlib` `5634668d…acc63` and by the same standards:

```text
BCL_AUTHORITY_TYPES=28   (was 27)     BCL_AUTHORITY_MEMBERS=254  (was 247)
BCL_IDENTITY_CHECKS=21                BCL_SENTINEL_CHECKS=433    (was 413)
BCL_MANIFEST_CHECKS=51   (was 48)     BCL_MUTATION_SELF_TESTS=462 (was 454)
BCL_CROSS_CHECKS=141 (monodis)        BCL_RESOURCE_CHECKS=21     (was 19)
BCL_IL_LITERAL_CHECKS=1               BCL_STATIC_TABLE_CHECKS=1
BCL_NEGATIVE_CONTROLS=4  all still refused
BCL_AUTHORITY_STATUS=PASS
```

The four Mono binaries are still refused on 16, 11, 10 and 13 of 21 identity
checks each — the admission of a new family did not loosen the gate.

## 2. It is the one family that is not a `SystemException` specialization

```il
.class public auto ansi serializable beforefieldinit System.ObjectDisposedException
       extends System.InvalidOperationException
```

`catch (InvalidOperationException)` therefore sees a use-after-dispose **and** a
write to a bound state object alike. That is a real behavioural fact and it has
its own sentinel and its own mutation.

Two more differences from the `ArgumentException` template it superficially
resembles, both of which a copy-and-adapt projection would get wrong:

| | `ArgumentException.ParamName` | `ObjectDisposedException.ObjectName` |
| --- | --- | --- |
| `getOverridable` in metadata | **true** — `virtual` | **false** — not virtual |
| getter on a nil field | returns the field, i.e. null | `objectName ?? String.Empty` |

So `ObjectName` is a plain `public var` and not `open`, and it is non-Optional
because the CLR getter is proven never to answer null. `Message` *is*
overridable and is `open override`.

The mutation `ObjectName made overridable, as ParamName is` plants exactly that
confusion. It needed a new mutation helper: `unseal_public_member` only ever
touched methods, so a property-specific `unseal_property(name)` was added —
the mirror of the existing `seal_property`.

## 3. The constructors do not follow the family pattern either

```il
.ctor(string objectName)                 -> this(objectName, ObjectDisposed_Generic)
.ctor(string objectName, string message) -> InvalidOperationException(message);
                                            HResult 0x80131622; objectName = objectName
.ctor(string message, Exception inner)   -> InvalidOperationException(message, inner);
                                            HResult 0x80131622;  NO object name
```

Three public constructors and **no parameterless one** — an object name is
always required. The single-`String` overload takes an *object name*, which is
the opposite of `ArgumentException`, `NotSupportedException`,
`InvalidOperationException` and `KeyNotFoundException`, whose single-`String`
overload is a message. The third overload takes a *message* first and stores no
object name at all, so it cannot share a Swift label with the second.

Each of those is a separate sentinel, including a negative one — `require(not
signature_present(…, []))` — because a parameterless constructor appearing here
would mean the family had been confused with its neighbours.

## 4. The composed message

```text
ObjectDisposed_Generic          = "Cannot access a disposed object."
ObjectDisposed_ObjectName_Name  = "Object name: '{0}'."
```

Both read out of the admitted binary's own embedded string table, pinned, and
compared against the Swift source. `get_Message` is
`String.IsNullOrEmpty(ObjectName) ? base.Message
: base.Message + Environment.NewLine + Format(template, ObjectName)`, and
because `ObjectName` already collapses nil to empty, a nil and an empty object
name compose identically here — which is *not* true of the argument family,
where nil and empty are different states of the same field.

## 5. The raise site that moved, and the three that did not

`Helpers.CheckDisposed(object obj, native int pComPtr)`, which every native
member of an XNA graphics resource calls, is:

```il
if (pComPtr == IntPtr.Zero)
    throw new ObjectDisposedException(obj.GetType().Name);
```

`GetType()` — the **dynamic** type. Note the contrast with `ThrowIfBound` and
the four `Apply` methods, which use `ldtoken` on the declaring class. Both
conventions are real and this binding reproduces both:
`GraphicsResource.validatedHandle` reports `storage.typeName`, the derived name
the subclass recorded, so a disposed `RenderTarget2D` says `RenderTarget2D`.

### Where the binding guards more than XNA does, and why that is stated

XNA does not guard uniformly. Attributing every `Helpers.CheckDisposed` and
every `ObjectDisposedException` constructor in the Graphics assembly to its
enclosing class gives:

```text
 24  GraphicsDevice          5  Effect            4  TextureCollection
  2  BlendState              2  DepthStencilState 2  RasterizerState
  2  SamplerState            2  Texture3D         2  VertexBuffer
  1  Texture2D (CopyData<T> only)                 1  TextureCube
  1  EffectPass              1  EffectParameter   1  VertexDeclaration
  0  SpriteBatch             0  RenderTarget2D
```

So `SpriteBatch` has no disposal guard at all, `RenderTarget2D` has none of its
own, and `Texture2D`'s single guard is inside the `GetData`/`SetData` path.
Using a disposed one in XNA reaches Direct3D with a dead COM pointer.

This binding guards every native handle before it crosses into CNA, because
handing a released handle to the native runtime is not an option here. That is
**more** guarding than XNA performs, and it is a deliberate divergence rather
than a derived behaviour. What this milestone changes is only the *class*: where
XNA raises anything for this condition it raises `ObjectDisposedException`, and
the binding now raises the same class with the same composed payload instead of
a `CNAError` case no `catch is CNAException` could see. Reproducing XNA's
*absence* of a guard would mean deliberately passing a freed handle to C.

What the milestone did have to fix is that the binding's own guards were not
uniform. `SpriteBatch.Begin`, `.Draw` and `.End` reached their handle through
`nativeStorage.validatedHandle` — an artefact of Foundation 41's mechanical
`storage.` → `nativeStorage.` rewrite — which skips `GraphicsResource`'s guard
entirely. A disposed `SpriteBatch` therefore answered on the CNA runtime
channel while a disposed `Texture2D` answered with the projected class. All
three now go through the class's own guard, a test pins the `SpriteBatch`
payload, and a mutation puts one site back.

Four `CNAError.disposedObject` sites deliberately stay where they are, because
they are native-lifetime failures with no XNA counterpart:

| Site | Why it stays |
| --- | --- |
| `Runtime/NativeObject.swift` | the C object's lifetime ended |
| `Runtime/CallbackState.swift` | the Swift `Game` behind a callback is gone |
| `Xna/Framework/Game.swift` | the C game handle is gone |
| `Xna/Graphics/GraphicsDeviceManager.swift` | not a `GraphicsResource`; XNA's own `GraphicsDeviceManager` mentions neither `ObjectDisposedException` nor `Helpers.CheckDisposed`, so there is no CLR class to project |

Foundation 37's `testCNAErrorCarriesNoCLRShapedIdentity` still lists twelve
runtime failures including `.disposedObject`; this milestone splits one of its
cases rather than overturning it.

The `GraphicsResource` "no native storage" branch — reachable only if a state
object ever grew a native member — moved to `CNAError.producerInvariant`, which
is what it always was: a defect in this binding, not a consumer error.

## 6. A pinned digest nothing was reading

Regenerating the BCL manifest made the verifier's self-test fail on
`bclSelectedShapeSha256`, which is the guard working. Re-pinning it exposed the
neighbouring `referenceContractSha256`, whose recorded value was
`7207908e…7fdc` while the retained XNA contract had hashed to `681e55b3…de6d`
since Foundation 41 added `readonly` to all 557 of its fields.

Nothing had noticed, because **nothing read it**. `accessorFallibilitySha256`,
`returnNullabilitySha256` and `bclSelectedShapeSha256` are all consulted by
`verify.py`; `referenceContractSha256` was recorded and never checked. A pinned
digest no gate consults is decoration, not evidence.

It is now enforced alongside the other three, re-pinned to the current value,
and shown to fail — flipping a single `readonly` in the contract makes the
self-test report

```text
the retained XNA contract digest 8bffdb57…38ec does not match the pinned 681e55b3…de6d
```

`API_COMPAT_SELF_TESTS` is 2420.

## 7. Falsifiability

| Gate | Planted defect | Result |
| --- | --- | --- |
| BCL audit | rebased onto `SystemException` | caught |
| BCL audit | made sealed | caught |
| BCL audit | `ObjectName` dropped | caught |
| BCL audit | `ObjectName` given a setter | caught |
| BCL audit | `ObjectName` made overridable, as `ParamName` is | caught |
| BCL audit | `Message` made non-overridable | caught |
| BCL audit | one constructor dropped | caught |
| BCL audit | the whole family removed from the manifest | caught |
| projection | disposal put back on `CNAError` | caught |
| projection | the class on the wrong base | caught |
| projection | `ObjectName` not collapsed to empty | caught |
| projection | the single-argument constructor read as a message | caught |
| projection | one `SpriteBatch` site back past its own guard | caught |
| verifier self-test | one `readonly` flipped in the retained contract | caught |

## 8. Measurement

```text
BCL_AUTHORITY_TYPES=28  BCL_AUTHORITY_MEMBERS=254  BCL_AUTHORITY_STATUS=PASS
BCL_OBSERVATIONS=323  (was 292)
PURE_XNA_DERIVED=2125  unchanged — no XNA fact moved
API_COMPAT_SELF_TESTS=2420  (was 2419)
496 tests, 0 failures
PROJECTION_MUTATIONS=40 CAUGHT=40 SURVIVORS=0  (was 35)
```

## 9. Two measurements taken for the next milestone, recorded now

Both are stated in full, with the rest of the frontier research they came from,
in `docs/frontier-research-graphics-device-state-and-vertex-declaration.md`.

Foundation 42b — `GraphicsDevice.BlendState`/`DepthStencilState`/
`RasterizerState` and `SamplerStateCollection` — was researched in this session
and not implemented. Two things it found are facts about the boundary, not about
that milestone's code, so they are recorded here and in
`docs/runtime-capabilities.json` rather than held in a plan.

### CNA's graphics-device handle is a per-callback token, not an identity

`build-probe/f42b_identity.c` reads `cna_game_get_graphics_device` twice inside
each of four different lifecycle callbacks, twice over:

```text
--- run 1 ---
load_content   first=4294967298  second=4294967298  same=yes
update#1       first=8589934594  second=8589934594  same=yes
draw#2         first=12884901890 second=12884901890 same=yes
update#3       first=17179869186 second=17179869186 same=yes
--- run 2 ---   … the sequence continues, still ascending
```

Stable within a callback, different in every callback. XNA's `GraphicsDevice`
is one long-lived object whose `cachedBlendState` and two
`SamplerStateCollection`s live as long as the device does, and its
`set_BlendState` early-out is *reference* equality against that field. None of
that can be anchored to a facade keyed by a token that changes every callback,
and reading into the token's bit layout to recover an identity would be reading
into CNA's representation. So when those members are projected, their storage
must be anchored to the **game** — the object that actually has the device's
lifetime — with the facade left as the thin view it is. That is a divergence in
where the storage sits, which is not observable, decided by a measurement
rather than by convenience.

### `BlendFunction.Min` and `.Max` are numbered the other way round

| | `Min` | `Max` |
| --- | --- | --- |
| XNA, from the pinned IL | `int32(0x00000003)` | `int32(0x00000004)` |
| CNA, from `graphics_state.h` | `UINT32_C(4)` | `UINT32_C(3)` |

A `rawValue` pass-through would silently exchange them. Every other state enum —
`Blend`, `ColorWriteChannels`, `CompareFunction`, `StencilOperation`,
`CullMode`, `FillMode`, `TextureAddressMode`, `TextureFilter` — agrees value for
value, which is the worst possible ratio: eight conversions that work by
accident and one that does not. When the state routes are bound, every enum
crosses through an exhaustive explicit map, not just the divergent one, so a
future divergence cannot hide behind a raw cast.

This is not a CNA defect — CNA is entitled to number its own enum — but it is an
interop hazard for any binding that assumes XNA numbering.
