# Frontier research: device state, the sampler collections, and `VertexDeclaration`

Measured and read while Foundation 42a was under test. **Nothing here is
implemented.** It is recorded because the measurements cost probe runs and IL
reads that should not have to be repeated, and because three of the findings are
facts about the boundary rather than about any particular milestone.

Native observations below come from `build-probe/f42_states.c` and
`build-probe/f42b_identity.c` on the qualified CNA 0.21.0 HEADLESS artifact
(`c32bfbd3…b731b`). Every XNA fact comes from the pinned
`Microsoft.Xna.Framework.Graphics.dll` `560080fc39021c61` or
`Microsoft.Xna.Framework.dll`. CNA observations are never XNA authority.

---

## Part 1 — `GraphicsDevice.BlendState` and the sampler collections

### CNA supports all of it

`graphics_state.h` publishes get/set for all four states, `graphics_device.h`
publishes get/set for the texture collections, and the probe exercised them:

```text
get_blend_state:              src=0 dst=1 fn=0 mask=-1
blend_state_init(AlphaBlend): src=0 dst=5
set_blend_state -> 0, read back src=0 dst=5        (a real round trip)
sampler stage=0 slot 0..15  get=0 set=0  filter=0 addrU=0 aniso=4
sampler stage=0 slot 16,17  get=1 set=1            (INVALID_ARGUMENT)
sampler stage=1 slot 0..15  get=0 set=0            (same on the vertex stage)
texture stage=0/1 slot 0    get=0 handle=0
texture stage=0/1 slot 17   get=1
```

CNA's per-slot default sampler is Linear/Wrap/anisotropy 4 and its default blend
state is One/Zero/Add/-1 — the same values XNA's `SetDefaults` writes. That is
**corroboration only**; the Swift defaults come from the pinned IL.

### Blocker 1 — the device handle is a per-callback token, not an identity

`build-probe/f42b_identity.c` reads `cna_game_get_graphics_device` twice in each
of four callbacks, over two runs:

```text
--- run 1 ---
load_content   first=4294967298  second=4294967298  same=yes
update#1       first=8589934594  second=8589934594  same=yes
draw#2         first=12884901890 second=12884901890 same=yes
update#3       first=17179869186 second=17179869186 same=yes
--- run 2 ---   the sequence continues, still ascending
```

Stable inside one callback, different in every callback.

XNA's `GraphicsDevice` is one long-lived object. `cachedBlendState` is a field on
it, `set_BlendState`'s early-out is *reference* equality against that field, and
the two `SamplerStateCollection`s are constructed once in the device constructor
and handed out by field read. None of that survives on a facade keyed by a token
that changes every callback — and recovering an identity by reading into the
token's bit layout would be reading into CNA's representation.

**Resolution when this is implemented:** device-owned managed state is anchored
to the *game*, which is the object that has the device's lifetime, and the
`GraphicsDevice` facade stays the thin callback-scoped view it is. That
preserves every observable semantic — reference identity of the cached state
object, the `value != cached` early-out, `isBound` becoming true exactly once,
one collection per device per run — and diverges only in where the storage
physically sits, which is not observable.

### Blocker 2 — `BlendFunction.Min` and `.Max` are numbered the other way round

| | `Min` | `Max` |
| --- | --- | --- |
| XNA, pinned IL | `int32(0x00000003)` | `int32(0x00000004)` |
| CNA, `graphics_state.h:53-62` | `UINT32_C(4)` | `UINT32_C(3)` |

Every other state enum agrees value for value: `Blend` 0–12,
`ColorWriteChannels` (0/1/2/4/8/15), `CompareFunction` 0–7, `StencilOperation`
0–7, `CullMode` 0–2, `FillMode` 0–1, `TextureAddressMode` 0–2, `TextureFilter`
0–8. Eight conversions that work by accident and one that does not, so every
enum must cross through an exhaustive explicit map rather than a raw cast.

Not a CNA defect — CNA is entitled to number its own enum — but an interop
hazard for any binding that assumes XNA numbering.

### The shapes, already settled by the recorded verdicts

`xna40-accessor-fallibility.json`:

| Member | Getter | Setter |
| --- | --- | --- |
| `GraphicsDevice.{Blend,DepthStencil,Rasterizer}State` | `IL_NO_FAILURE_PATH` | `IL_DIRECT_THROW` `[ArgumentNullException]` |
| `GraphicsDevice.{Sampler,VertexSampler}States`, `{,Vertex}Textures` | `IL_NO_FAILURE_PATH` | `ACCESSOR_ABSENT` |
| `SamplerStateCollection.Item` | `IL_DIRECT_THROW` `[ArgumentOutOfRangeException]` | `IL_DIRECT_THROW` `[ArgumentNullException, ArgumentOutOfRangeException]` |

So the three device states are plain readers with throwing `SetX` writer
methods, the four collections are plain readers with no writer, and the
collection indexer is a throwing indexed getter plus a throwing `SetItem`.

`SamplerStateCollection.Item`'s return is `UNKNOWN_REFERENCE_NULLABILITY`
(`origin: None` at its one return site), so the deferral rule makes it
non-Optional. That is also behaviourally right: `pSamplerList` starts null but
`InitializeDeviceState()` fills every slot from `SamplerState.LinearWrap` before
a consumer sees it. Both halves are recorded because if the analyser is ever
taught array-element origins the verdict may become `PROVEN_NULLABLE`.

### The three device setters are not symmetric

| | early-out | extra cached state | dirty flag | `EffectStateFlags` bit |
| --- | --- | --- | --- | --- |
| `set_BlendState` | `!=` cached **or** `blendStateDirty` | `cachedBlendFactor`, `cachedMultiSampleMask` | yes | 1 |
| `set_DepthStencilState` | `!=` cached **or** `depthStencilStateDirty` | `cachedReferenceStencil` | yes | 2 |
| `set_RasterizerState` | `==` cached ⇒ return | none | **no** | 4 |

All three raise `ArgumentNullException("value", FrameworkResources.NullNotAllowed)`
— paramName **first**, because that overload is `.ctor(paramName, message)`.
`NullNotAllowed` is `"This method does not accept null for this parameter."` and
is not yet pinned.

`SamplerState.Apply(device, samplerIndex)` and the three `Apply(device)` methods
share one prologue: `ObjectDisposedException(typeof(<declaring class>).Name)`
when disposed, then `if (_parent != device) { _parent = device; isBound = true; }`
and push. So one internal `attach(to:)` covers all four.

`SamplerStateCollection` itself: `.ctor(device, samplerOffset, maxSamplers)`
with offsets `0` for pixel and `0x101` for vertex, lengths
`ProfileCapabilities.MaxSamplers` = 16 in both profiles and `MaxVertexSamplers`
= 0 under Reach, 4 under HiDef. CNA offers 16 on both stages, which is a
divergence to record when the collection is projected.

### `TextureCollection` is a bigger job and should follow, not lead

`get_Item` does not read a managed array. It calls
`Helpers.CheckDisposed(_parent, device.pComPtr)`, bounds-checks against
`_maxTextures`, then queries the live D3D device for the bound
`IDirect3DBaseTexture9` and reconstructs a managed `Texture` from it through
three branches. `set_Item` **accepts null** — a null value unbinds the slot.
CNA's `cna_graphics_device_get_texture` answers a raw handle, so a faithful
`get_Item` needs a handle → managed `Texture` identity map this binding does not
have; returning a freshly wrapped `Texture` per read would break the reference
identity that `SamplerStateCollection`'s own `beq` early-out shows XNA relies on.

---

## Part 2 — `VertexDeclaration`

**Implemented in Foundation 43**, and the five types it unblocked in
Foundation 44. What follows is the research this section was written from;
`docs/foundation-43-vertex-declaration-evidence.md` records what was actually
built, including the CNA corroboration of the stride rule that this section only
proposed, and `docs/foundation-44-vertex-types-evidence.md` records
`IVertexType` and the four vertex structs.

Reach 39, dependency-complete, and **no callback-scope problem at all**: none of
CNA's eight `cna_vertex_declaration_*` routes takes a device handle, so a
declaration is an ordinary `OWNED` native object with a clean lifetime.

It unblocks `VertexBuffer`, `DynamicVertexBuffer`, `IVertexType` and the four
`VertexPositionX` structs — each of which declares
`public static initonly VertexDeclaration VertexDeclaration`, which is why
Foundation 41's `readonly` work had to land first.

### The constructors accept a null or empty array in silence

```text
.ctor(int32 vertexStride, VertexElement[] elements)      [elements is ParamArray]
    Object::.ctor()                     <- not GraphicsResource::.ctor
    try {
        if (elements == null)   leave;  <- accepted
        if (elements.Length==0) leave;  <- accepted
        _elements     = (VertexElement[]) elements.Clone();    <- a COPY
        _vertexStride = vertexStride;
        VertexElementValidator.Validate(vertexStride, _elements);
    } fault { Dispose(true); }

.ctor(VertexElement[] elements)                          [elements is ParamArray]
    ... same guards and Clone ...
    _vertexStride = VertexElementValidator.GetVertexStride(_elements);
    VertexElementValidator.Validate(_vertexStride, _elements);
```

A null or empty array leaves `_elements` null and `_vertexStride` 0 with no
complaint. `[ParamArray]` is a custom attribute that `member_key` does not read,
so the contract records a plain `VertexElement[]` and the mapped parameter is a
Swift array — a Swift variadic would mismatch, and no new mapping rule is needed.

### A gap in this repo's own analyser, found by reading ahead

`GetVertexElements()` is `ldfld _elements; callvirt Array::Clone(); castclass; ret`,
and `_elements` is null exactly when the constructor accepted a null or empty
array. So

```swift
let d = VertexDeclaration(elements: [])   // accepted
d.GetVertexElements()                     // NullReferenceException
```

is reachable, yet the record for it says `fallible: false,
IL_NO_FAILURE_PATH`. The analyser does not treat a `callvirt` on a reference
loaded from a possibly-null field as a failure path.

Three things about that, in order of what they change.

**Which file.** `GetVertexElements` is a method, not a property accessor, so it
is not in `xna40-accessor-fallibility.json` at all; the verdict lives in the
`fallibility` block of its `xna40-reference-return-nullability.json` record,
produced by `return_nullability.py`.

**What depends on it.** `verify.py` reads that block into `Member.return_fallible`
and uses it in exactly one place: a clause appended to the diagnostic for a
*surplus* Optional return. Nothing compares a method's Swift `throws` against
it — `throws` is compared only for property accessors, through
`THROWING_GETTER_PROJECTIONS` and the writer-method rules. So the inaccuracy is
not load-bearing, and it does **not** block projecting `GetVertexElements` as
throwing, which is what the IL requires.

**What to do.** Record it, and treat teaching the analyser the rule as its own
scoped change rather than a step inside a type's milestone: the pinned file
carries its own digest in `mapping-rules.json` and 115 self-tests, and a new
failure-path rule would reclassify an unknown number of unrelated members.
Measure that blast radius before changing it. What must not happen is the
reverse — writing a *non*-throwing `GetVertexElements` because a record says
`IL_NO_FAILURE_PATH` when the IL says otherwise.

### `Dispose(Boolean)` reduces to the base call, and it is shown

`Dispose(bool)` runs `~VertexDeclaration()` or `!VertexDeclaration()` and then
`base.Dispose(disposing)`; `~VertexDeclaration()` calls `!VertexDeclaration()`,
whose whole body is `Unbind()` — internal device-binding bookkeeping with no
Swift counterpart. Same conclusion as `Texture2D`, reached the same way.

### The stride rule

`VertexElementValidator.GetTypeSize(VertexElementFormat)` is a switch with a
default of `0`:

| | | | |
| --- | --- | --- | --- |
| `Single` 4 | `Vector2` 8 | `Vector3` 12 | `Vector4` 16 |
| `Color` 4 | `Byte4` 4 | `Short2` 4 | `Short4` 8 |
| `NormalizedShort2` 4 | `NormalizedShort4` 8 | `HalfVector2` 4 | `HalfVector4` 8 |

`GetVertexStride(elements)` is the **maximum end offset**, not a sum and not the
last element's end — elements may be given in any order and may overlap:

```csharp
int max = 0;
for (int i = 0; i < elements.Length; ++i) {
    int end = elements[i].Offset + GetTypeSize(elements[i].VertexElementFormat);
    if (max < end) max = end;
}
return max;
```

CNA's own `cna_vertex_declaration_create` also computes a stride. That is
corroboration to compare against, never the rule.

### The five validation failures, in the order they are checked

```text
1. vertexStride <= 0              ArgumentOutOfRangeException("vertexStride")
2. vertexStride & 3               ArgumentException(VertexElementOffsetNotMultipleFour)
   (build an owner map with one slot per byte of the vertex, all -1)
   for each element, in array order:
3. usage out of range             ArgumentException(VertexElementBadUsage)
4. does not fit within stride     ArgumentException(VertexElementOutsideStride)
5. offset & 3                     ArgumentException(VertexElementOffsetNotMultipleFour)
6. duplicate usage + usageIndex   ArgumentException(DuplicateVertexElement)
7. byte already owned             ArgumentException(VertexElementsOverlap)
```

The order is observable: an element that is both misaligned and overlapping
reports the alignment message, and one that is both out of range and out of
stride reports the usage message. The overlap message can name **both**
offending elements because the map records which element owns each byte.

The five messages, read out of `Microsoft.Xna.Framework.dll`'s own table and not
yet pinned in `registered-assemblies.json`:

```text
VertexElementOffsetNotMultipleFour
    Invalid VertexDeclaration. Vertex stride and VertexElement.Offset must be
    multiples of four.
VertexElementOutsideStride
    Invalid VertexDeclaration. Element {0}{1} does not fit within the specified
    vertex stride.
VertexElementsOverlap
    Invalid VertexDeclaration. Elements {0}{1} and {2}{3} are overlapping.
DuplicateVertexElement
    Invalid VertexDeclaration. Duplicate element {0}{1}.
VertexElementBadUsage
    Invalid VertexDeclaration. Usage {0}{1} is out of range.
```

The `{0}{1}` pairs are `VertexElementUsage` and `UsageIndex`, formatted with
`CurrentCulture`. A sixth key, `VertexStrideTooSmall`, exists in the same table
and `Validate` does not use it — do not pin what the projection does not
reproduce.
