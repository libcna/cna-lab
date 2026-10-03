# Foundation 45 — device state, and the first value this binding has applied to a device

`GraphicsDevice.BlendState`, `.DepthStencilState`, `.RasterizerState`,
`.SamplerStates`, `.VertexSamplerStates` and `SamplerStateCollection`. Eight
native routes bound, four PODs mirrored, and a state object that a consumer
builds now reaches CNA and comes back bound.

This is the first milestone that turns a complete *type* into a runtime
capability, which is the distinction `plan.md`'s rule 3 keeps insisting on: the
four state objects were complete in Foundation 41 and had never been applied to
anything.

## 1. Two measured divergences decided the design before any code

Both were measured while Foundation 42a was under test and recorded then, in
`docs/frontier-research-graphics-device-state-and-vertex-declaration.md`.

### The device handle is a per-callback token, so the cache cannot live on the facade

`build-probe/f42b_identity.c` reads `cna_game_get_graphics_device` twice inside
each of four lifecycle callbacks, twice over: stable within a callback,
**different in every callback**. XNA's `GraphicsDevice` is one long-lived object
whose `cachedBlendState` and two `SamplerStateCollection`s live as long as the
device, and whose `set_BlendState` early-out is *reference* equality against that
field.

So the cache lives on `RuntimeState` — one per game, the object that actually
has the device's lifetime — and `GraphicsDevice` stays the thin per-callback
view it already was. Every observable semantic survives: the cached instance is
the caller's own object, the `value != cached` early-out works, `isBound`
becomes true exactly once, and one collection serves the whole run. What
diverges is where the storage physically sits, which is not observable, and it
was decided by a measurement rather than by convenience.

The collections are created **lazily**, because `RuntimeState` exists before any
device does and XNA builds them when the device is created.

### `BlendFunction.Min` and `.Max` are numbered the other way round

| | `Min` | `Max` |
| --- | --- | --- |
| XNA, pinned IL | `int32(0x00000003)` | `int32(0x00000004)` |
| CNA, `graphics_state.h:53-62` | `UINT32_C(4)` | `UINT32_C(3)` |

`Blend`, `ColorWriteChannels`, `CompareFunction`, `StencilOperation`,
`CullMode`, `FillMode`, `TextureAddressMode` and `TextureFilter` all agree value
for value. Eight conversions that would work by accident and one that would not
is the worst possible ratio, so **all nine cross through an explicit map** and
none is a `rawValue` cast. A future divergence in any of them cannot hide, and
the round trip is asserted for every case of every enum.

## 2. The three setters are not symmetric, and are transcribed separately

| | early-out | extra cached state | dirty flag | `EffectStateFlags` bit |
| --- | --- | --- | --- | --- |
| `set_BlendState` | `!=` cached **or** `blendStateDirty` | `cachedBlendFactor`, `cachedMultiSampleMask` | yes | 1 |
| `set_DepthStencilState` | `!=` cached **or** `depthStencilStateDirty` | `cachedReferenceStencil` | yes | 2 |
| `set_RasterizerState` | `==` cached ⇒ return | none | **no** | 4 |

`RasterizerState` has no dirty flag at all: an identical instance is always a
no-op with no escape hatch. Writing all three from one template would have been
wrong in two different ways at once, and two mutations aim at exactly that.

All three raise `ArgumentNullException("value", NullNotAllowed)` — paramName
**first**, because that overload is `.ctor(String paramName, String message)`.
`NullNotAllowed` is now the 21st pinned resource key, read out of
`Microsoft.Xna.Framework.dll`'s own table.

There is no effect pass to end yet, so that step of each setter has nothing to
act on and is recorded rather than simulated.

## 3. `Apply` is one shared prologue

`SamplerState.Apply(device, samplerIndex)` and the three `Apply(device)` methods
open identically:

```csharp
if (isDisposed) throw new ObjectDisposedException(typeof(<declaring>).Name);
if (_parent != device) { _parent = device; isBound = true; }
// ...push...
```

`ldtoken` on the declaring class again, so the disposal message uses the same
static name `ThrowIfBound` does — which Foundation 42a's
`CNAObjectDisposedException` made projectable. One internal `attach(to:)` on the
state protocol covers all four; the push is per-class and belongs to the caller.

## 4. `SamplerStateCollection`

Sealed, `System.Object` base, and exactly one public member. Both indexer
accessors are recorded `IL_DIRECT_THROW`, so the projection is a throwing
indexed reader plus a throwing `SetItem` writer, not a Swift `subscript`.

`set_Item` in order: bounds-check → `ArgumentOutOfRangeException("index")`;
refuse null → `ArgumentNullException("value", NullNotAllowed)`; return when the
value is the **same instance** already in the slot; `Apply`; store.

The null branch is **unreachable in Swift**, and finding that out was the
verifier's doing. The writer was written with an Optional parameter first, and
the answer came back

```text
PARAMETER_MAPPING_MISMATCH  SamplerStateCollection.SetItem
  property writer expects [('_', 'Int32'), ('_', '…SamplerState')],
  found [('_', 'Int32'), ('_', '…SamplerState?')]
```

The writer's parameter is the property's type, `Item`'s verdict is
`UNKNOWN_REFERENCE_NULLABILITY`, and the deferral rule therefore makes both
non-Optional — so a non-Optional `SamplerState` cannot be nil and the type
system enforces what XNA enforces at run time. That is the same situation as
`VertexElementValidator`'s usage-range check in Foundation 43, and it is
resolved the same way: say so rather than write dead code. `NullNotAllowed` is
still reproduced where it *is* reachable — the three `GraphicsDevice` setters,
whose properties are proven nullable and whose parameters are therefore
Optional.

Two facts about the slots:

* **Length.** XNA's is `ProfileCapabilities.MaxSamplers` — 16 in both profiles
  for the pixel collection, and `MaxVertexSamplers`, which is 0 under Reach and
  4 under HiDef, for the vertex one. `build-probe/f42_states.c` measured CNA
  accepting slots 0–15 on **both** stages and answering
  `CNA_RESULT_INVALID_ARGUMENT` for 16 and above. This binding has no profile
  selection, so both collections are 16 long — a recorded divergence, because
  the bounds check is observable.
* **An unwritten slot.** `pSamplerList` starts null and `InitializeDeviceState`
  fills every entry from `SamplerState.LinearWrap` before a consumer sees it.
  The recorded return verdict for `Item` is `UNKNOWN_REFERENCE_NULLABILITY`, so
  the deferral rule keeps it non-Optional; a slot this binding has never written
  answers `SamplerState.LinearWrap`, which is exactly what XNA's initialisation
  put there.

## 5. A nullability proof that changed the projection

`GraphicsDevice.SamplerStates` and `.VertexSamplerStates` were written
non-Optional first. The pinned verdict is `PROVEN_NULLABLE_SUCCESS` with the
evidence *"no constructor of `GraphicsDevice` assigns `pSamplerState`"* — the
collection is built when the device is **created**, not when the object is, so a
device that has not created its D3D device answers null. Both are Optional now,
and answer nil when the facade's handle no longer validates, which is the
closest analogue this binding has.

## 6. The verifier could not yet spell one type

The eight new routes' first verification run reported two mismatches:

```text
cna_graphics_device_get_sampler_state: parameters
  ['uint64_t', 'uint32_t', 'uint32_t', 'CNA_SamplerState*'] !=
  ['uint64_t', 'CNA_ShaderStage', 'uint32_t', 'CNA_SamplerState*']
```

`CNA_ShaderStage` was missing from the alias table the type-compatibility
comparison uses — `typedef uint32_t CNA_ShaderStage;` at `graphics_state.h:214`.
The textual canonical-declaration check had already passed on all 198 positions,
because the manifest keeps CNA's own spelling; adding an alias never weakens
that check. This is the same class of gap Foundation 38 hit on its first surface
and it is the check working, not failing.

## 7. Falsifiability, one test the gate rejected and one mutation withdrawn

Seven mutations, all caught:

| Planted defect |
| --- |
| `BlendFunction` cast through instead of mapped |
| the colour and alpha channels written to each other's POD fields |
| a device setter caching without attaching, so the state stays writable |
| `set_BlendState` caching the state but not the value it copies out |
| the collection storing a copy rather than the caller's instance |
| the collection rebuilt per access, losing every slot already written |
| a direct write cached without reaching the device it must push to |

### The copied-value test was blind, for a reason worth stating

`set_BlendState` copies `BlendFactor` and `MultiSampleMask` out of the state
into two fields of its own. The first version of the test assigned
`BlendState.Additive` and asserted the copies — and every preset carries
`SetDefaults`' `Color.White` and `-1`, which are **also** the runtime's starting
values. Dropping the copy entirely changed nothing the test could see. It now
assigns a state with a `CornflowerBlue` factor and a `0x0F0F` mask, and the
repair was verified against the planted defect directly.

### A mutation withdrawn as unfalsifiable

`the rasterizer setter given a dirty flag it does not have` survived, and the
reason is not a weak test. Nothing in this binding ever sets `blendStateDirty`
or `depthStencilStateDirty` to true: in XNA they are set by a device reset and
by an effect pass ending, and neither is projected. Both flags are therefore
permanently false, and no consumer can observe the difference between a setter
that has a flag and one that does not.

The flags are still transcribed, so that the setters are the algorithm XNA runs
rather than a simplification of it — but a mutation that cannot fail is not
evidence, so it was removed rather than left in the harness claiming coverage it
does not have. That is the same judgement the graphics-device-scope mutation
received in Foundation 40. When a device reset lands, the flags become
observable and the mutation can come back.

## 8. The gate was damaging the repository

The last survivor of this milestone was not a weak test. `fresh-state-born-bound`
reported `the mutation site occurs 0 times, not once`, and chasing that found
`Sources/CNA/CNAExceptions.swift` sitting in the working tree with

```swift
        HResult = CNAArgumentNullException.argumentNullHResult
```

**deleted** — a planted defect from a mutation run that a `timeout` had killed.
Every mutation is undone in a `finally`, which a normal exit or a Ctrl-C
honours; SIGTERM's default handler terminates the process outright and leaves
the defect behind. It had happened twice in this session, and both times the
next run attributed the stranded mutation to whatever noticed it first, forty
minutes in.

Two repairs, each verified:

* **Site staleness is a precondition.** Every mutation site is counted before
  the baseline runs, so a drifted or stranded site is reported in seconds
  instead of costing a full run and arriving disguised as a survivor. Proven by
  introducing a deliberate typo into one site and watching the run refuse to
  start.
* **SIGTERM and SIGHUP raise `KeyboardInterrupt`**, putting them on the same
  footing as Ctrl-C so the `finally` unwinds and the tree is restored. Proven by
  killing a run mid-flight with `timeout` and confirming afterwards that no
  mutation site had drifted. SIGKILL cannot be caught, which is precisely why
  the precondition exists as the backstop.

An uncommitted deletion of an `HResult` assignment is the kind of thing that
gets committed by accident. A gate that exists to catch defects was introducing
one.

## 9. The three values the state setters copy out

`set_BlendState` copies `BlendFactor` and `MultiSampleMask` out of the state it
accepts; `set_DepthStencilState` copies `ReferenceStencil`. All three are also
public properties of the device in their own right, and all three getters are
bare field reads of exactly the fields this milestone added — so the readers
came free with the cache.

The setters did not. Each is ~2,000 bytes of IL that opens with
`Helpers.CheckDisposed(this, pComPtr)` and then pushes to the device
independently of any state object, and each carries the verdict
`IL_REACHABLE_THROW` with `ObjectDisposedException`. CNA publishes a dedicated
route for every one of them —
`cna_graphics_device_set_blend_factor`, `…set_multi_sample_mask`,
`…set_reference_stencil` — so they are real writes rather than mirrors, and six
more routes came with them.

The ordering is observable and is tested: a direct write to `BlendFactor` is
overwritten by the next `SetBlendState`, because that setter copies the value
out of the state it was handed. Read without the IL that looks like a bug.

## 10. Measurement

```text
BOUND_FUNCTIONS=69  (was 55)       PROTOTYPE_TYPE_POSITIONS=216 (was 170)
LAYOUTS=25  (was 21)               LAYOUT_FIELDS=209 (was 157)
ABI_MISMATCHES=0                   NATIVE_ABI_MUTATIONS=14 CAUGHT=14
COMPLETE_TYPES=150  (was 149)      MISSING_TYPE=100  (was 101)
MISSING_MEMBER=98   (was 106)      TOTAL_DIAGNOSTICS=215 (was 224)
XNA_RESOURCE_STRING_PROJECTIONS=21 (was 20)
541 tests, 0 failures      PURE_XNA_DERIVED=2314
PROJECTION_MUTATIONS=57 CAUGHT=57 SURVIVORS=0  (was 50)
```
