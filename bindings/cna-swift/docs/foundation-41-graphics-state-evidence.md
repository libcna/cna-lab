# Foundation 41 — the four graphics state objects, and a metadata attribute the contract had never recorded

`BlendState`, `DepthStencilState`, `RasterizerState` and `SamplerState` are the
four settings objects XNA hands to a `GraphicsDevice`. They were the highest
`reach` cluster in the dependency graph — 43 types wait behind them — and they
are the first types this binding has added that carry **`initonly` public
fields**. That turned out to matter more than the types themselves.

## 1. The defect this milestone found: `initonly` was not in the contract

XNA declares each preset as

```il
.field public static initonly class Microsoft.Xna.Framework.Graphics.BlendState Opaque
```

`initonly` is `FieldAttributes.InitOnly` — CLR metadata, the same kind of fact
as `sealed` or `static`, and exactly the kind of fact this binding claims to
reproduce. The retained contract recorded `static`, `constant` and `value` for
every field and **did not record `initonly` at all**. `tools/api_compat/verify.py`
therefore computed

```python
mutable = not bool(source.get("constant"))
```

and demanded a mutable Swift `var` for a field XNA makes read-only. Writing the
presets as `public static let` produced sixteen `FIELD_MAPPING_MISMATCH`
diagnostics; writing them as `var` would have cleared the diagnostics while
projecting the wrong shape.

Nothing detected this before because no previously projected type had an
`initonly` field. The audit's parser already *recognised* the token — it was in
the `known` attribute set — and then discarded it.

### The repair

1. `pinned_assembly_audit.py` records `"readonly": "initonly" in attributes` on
   every field, and `member_key` compares it, so the retained value is now
   proven against the registered binaries rather than asserted.
2. The contract carries `readonly` on all **557** fields. The change to the file
   is exactly that: stripping the new key from both versions leaves them
   byte-identical.
3. `verify.py` projects `mutable = not constant and not readonly`, with the
   three CLR field shapes — plain, `initonly`, `literal` — asserted directly on
   the model builder.

### Both directions were shown to fail

| Gate | Planted defect | Result |
| --- | --- | --- |
| `verify.py --self-test` | the readonly term removed from the field rule | `initonly field mutability projection` — FAILS |
| `pinned_assembly_audit.py` | `SamplerState.PointWrap` flipped to `readonly: false` in the contract | `Microsoft.Xna.Framework.Graphics.dll … mismatched= 1` |
| audit self-tests | `flipped field readonly identity`, run over six calibration subjects | caught; `AUDIT_SELF_TESTS` 70 → 80 |

`BlendState` was added as a sixth calibration subject specifically because the
first five have no `initonly` field, so the mutation could otherwise only ever
flip `false` to `true`.

### An independent disassembler agrees

`ikdasm` and `monodis` share no code. Both were asked which contract fields
carry `InitOnly` — `monodis --fields`' attribute words against `ikdasm`'s IL
tokens — and both name exactly the same **23**:

```text
Microsoft.Xna.Framework.Audio.Microphone.Name
Microsoft.Xna.Framework.Graphics.BlendState.{Opaque,AlphaBlend,Additive,NonPremultiplied}
Microsoft.Xna.Framework.Graphics.DepthStencilState.{None,Default,DepthRead}
Microsoft.Xna.Framework.Graphics.RasterizerState.{CullNone,CullClockwise,CullCounterClockwise}
Microsoft.Xna.Framework.Graphics.SamplerState.{PointWrap,PointClamp,LinearWrap,LinearClamp,
                                               AnisotropicWrap,AnisotropicClamp}
Microsoft.Xna.Framework.Graphics.VertexPosition*.VertexDeclaration        (4)
Microsoft.Xna.Framework.GraphicsDeviceManager.{DefaultBackBufferWidth,DefaultBackBufferHeight}
```

Two of those were also simply **missing** from the projection.
`GraphicsDeviceManager.DefaultBackBufferWidth` and `…Height` are
`public static initonly int32`, assigned `0x320` and `0x1e0` by the class
constructor: **800** and **480**. They are now projected, as `let`.

## 2. The second design decision: no invented base class

The four types share `ThrowIfBound`, an `isBound` flag and a `Dispose(bool)`
override. The obvious Swift move is a shared base class — and it is wrong. XNA's
metadata derives all four **directly** from `GraphicsResource`:

```text
BASE_MAPPING_MISMATCH  expected base …Graphics.GraphicsResource,
                       found …Graphics.GraphicsStateObject
UNEXPECTED_TYPE        public XNA-namespace type has no mapped reference identity
```

The first attempt inserted `GraphicsStateObject` between them and earned four
base mismatches and one unexpected type. The shared code now lives in an
**internal** Swift protocol of the same name with a default implementation: the
code is shared, the public surface is XNA's.

`GraphicsResource.storage` became `NativeHandleStorage?` for this milestone.
These four are the first `GraphicsResource` subclasses that own no native
object — CNA models the same settings as POD descriptors with no handle — so
`IsDisposed` is `storage?.isDisposed ?? managedDisposed` and `Dispose(Boolean)`
branches on whether there is anything native to release.

## 3. Every default and every preset comes from the pinned IL

`SetDefaults()` in `Microsoft.Xna.Framework.Graphics.dll`
(`560080fc39021c61`), not from CNA and not from a comparator:

| Type | Defaults |
| --- | --- |
| `BlendState` | colour and alpha both `One`/`Zero`/`Add`; all four `ColorWriteChannels` `All`; `BlendFactor` `Color.White`; `MultiSampleMask` `-1` |
| `DepthStencilState` | depth test and write **on**, `LessEqual`; stencil off, `Always` + `Keep` on both faces; masks `-1`; reference `0` |
| `RasterizerState` | `CullCounterClockwiseFace`, `Solid`, scissor off, **`MultiSampleAntiAlias` on**, both biases `0` |
| `SamplerState` | `Linear`, `Wrap` on all three axes, anisotropy `4`, mip level `0`, LOD bias `0` |

The presets come from each `.cctor()` and the private presetting constructor it
calls. Two shapes are easy to get wrong and are asserted explicitly: the
`BlendState` constructor writes its source/destination pair onto **both** the
colour and the alpha channel, and the `SamplerState` constructor writes its one
address mode onto **all three** axes.

`RasterizerState.MultiSampleAntiAlias` defaulting to `true` is the single value
most likely to be transcribed wrong, and has its own mutation.

Every one of the 42 `SetDefaults()` field writes was extracted from the IL and
compared against the Swift defaults, so no value here rests on having read a
disassembly listing by eye.

## 4. The static presets are born bound

The first version of this projection made the four `.cctor` presets ordinary
mutable objects. The IL says otherwise. Every presetting constructor's last act,
after `SetDefaults()`, after the two or three preset writes, and after
`GraphicsResource::set_Name`, is:

```il
IL_0048:  ldc.i4.1
IL_0049:  stfld      bool Microsoft.Xna.Framework.Graphics.BlendState::isBound
```

while the parameterless constructor ends with `ldc.i4.0` into the same field.
All four classes do this. `BlendState.Opaque` is therefore **permanently
read-only**, and so are the other fifteen presets — which is the mechanism that
stops one caller silently changing a process-wide shared object for every other
caller. A projection that left them writable would have compiled, passed every
shape check, and been wrong in the most damaging way available.

`GraphicsResource.set_Name` is the exception, and deliberately: it lives on the
base, has no `isBound` guard at all, and merely stores into `_localName` when
there is no native handle. Renaming a bound state is legal in XNA and is legal
here.

Two mutations pin this: removing `isBound = true` from a presetting
constructor, and adding it to the parameterless one.

## 5. `ThrowIfBound` names the declaring class, not the dynamic one

```il
IL_0011: ldtoken    Microsoft.Xna.Framework.Graphics.BlendState
IL_0016: call       Type Type::GetTypeFromHandle(RuntimeTypeHandle)
IL_001b: callvirt   instance string MemberInfo::get_Name()
…
IL_0026: call       string FrameworkResources::get_BoundStateObject()
IL_002c: call       string String::Format(IFormatProvider, string, object[])
IL_0031: newobj     instance void InvalidOperationException::.ctor(string)
```

That is `ldtoken` on the **static** type, not `GetType()`. A user subclass of
`BlendState` bound to a device still reports `BlendState`. The projection
therefore carries a static `boundStateTypeName` per class rather than reflecting
`self`, and `testBoundMessageNamesTheDeclaringClassNotTheDynamicOne` pins it
through a real subclass.

The message itself is not transcribed. `BoundStateObject` is now the 16th
registered key in `xna40-selected-resource-strings.json`, read mechanically out
of `Microsoft.Xna.Framework.dll`'s embedded string table:

```text
Cannot change read-only {0}. State objects become read-only the first time they
are bound to a GraphicsDevice. To change property values, create a new {0}
instance.
```

The template substitutes the name **twice**, and the verifier compares the Swift
literal against the pinned value.

## 6. What is deliberately not claimed

`isBound` can only be set from inside the module, because **no device route is
bound yet**. `GraphicsDevice.BlendState`, `.DepthStencilState`,
`.RasterizerState`, `.SamplerStates` and `.VertexSamplerStates` remain missing
members, and `SamplerStateCollection` remains a missing type. CNA does publish
`cna_graphics_device_set_blend_state` and its neighbours in `graphics_state.h`,
so this is an implementable next step and not a blocker — it is simply not done,
and `docs/runtime-capabilities.json` records the status as
`VERIFIED_MANAGED_NOT_YET_BOUND_TO_A_DEVICE` rather than claiming a device
round trip that has not happened.

CNA's own preset descriptors were **not** consulted for any value in this
milestone. CNA is never XNA behaviour authority; every number above is from the
pinned assembly.

## 7. A coverage loss the mutation harness was hiding

`tools/projection_mutations/run.py` runs the whole suite for every mutation.
It carries 35 mutations after this milestone, all caught.
Sixteen of its mutations are caught only by suites that start a CNA runtime, and
those suites **skip** when `CNA_NATIVE_LIBRARY` is unset — they do not fail. Run
without the variable, the harness reported fifteen SURVIVORS that were nothing
of the kind. It now refuses to start without a selected native library file, so
a missing runtime is reported as a precondition failure rather than as fifteen
projection defects.

Two mutation sites also had to be re-pointed at the rewritten
`GraphicsResource.Dispose(Boolean)`; the harness reported those honestly, as
`the mutation site occurs 0 times, not once`.

## 8. Measurement

```text
COMPLETE_TYPES=143  (was 139)      PARTIAL_TYPES=7      MISSING_TYPE=107
MISSING_MEMBER=106                 TOTAL_DIAGNOSTICS=230 (was 236)
UNEXPECTED_TYPE=0  BASE_MAPPING_MISMATCH=0  FIELD_MAPPING_MISMATCH=0
XNA_RESOURCE_STRING_PROJECTIONS=16 (was 15)
API_COMPAT_SELF_TESTS=2419         AUDIT_SELF_TESTS=80  (was 64)
PROJECTION_MUTATIONS=35 CAUGHT=35 SURVIVORS=0     (was 26)
RESOURCE_STRING_CHECKS=32          RESOURCE_STRINGS_REPRODUCED=16
CONTRACT_TYPES_REPRODUCED=257      CONTRACT_MEMBERS_REPRODUCED=2964
```

## 9. What this unlocked

`tools/api_compat/dependency_graph.py` re-ranked after the milestone.
`DEPENDENCY_COMPLETE_MISSING_TYPES` fell from 22 to 19, and two of the three
that left the list did so by being **implemented**; the rest of the drop is
`SamplerStateCollection` and `TextureCollection` **entering** it, because their
only dependencies — `SamplerState` and `Texture` — now exist.

The four highest-reach dependency-complete missing types are now:

| Type | Transitive missing reverse reach |
| --- | --- |
| `GraphicsAdapter` | 42 |
| `VertexDeclaration` | 39 |
| `SamplerStateCollection` | 39 |
| `TextureCollection` | 39 |

`SamplerStateCollection` is the one whose native support is already measured:
`cna_graphics_device_get_sampler_state` and `…set_sampler_state` accept slots
0–15 on both the pixel and vertex stage and answer `CNA_RESULT_INVALID_ARGUMENT`
for 16 and above, measured with `build-probe/f42_states.c` on the qualified
artifact. That measurement also found the divergence a projection of it will
have to record: XNA's vertex sampler collection is 0 long under Reach and 4 long
under HiDef, and CNA offers 16 on both stages.
