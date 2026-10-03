# Foundation 64 — `TextureCube` and `Texture3D`

```text
COMPLETE_TYPES   156 -> 158     MISSING_TYPES  95 -> 93
TOTAL_DIAGNOSTICS 152 -> 150    TARGET_MEMBERS 2029 -> 2049
BOUND_FUNCTIONS  109 -> 119     LAYOUTS 33 -> 39     CONSTANTS 215 -> 221
MEMBERS_READ    1298 -> 1310    ABI_MISMATCHES=0
MESSAGES_REPRODUCED 130 -> 167  MESSAGES_NATIVE_OWNED 19 -> 25
MESSAGES_UNREACHABLE   8 -> 10  MESSAGES_DEFERRED      4 -> 10
tests            662 -> 678     PROJECTION_MUTATIONS 155 -> 168
```

The last two texture types. They also unblock `EffectParameter`'s
`GetValueTextureCube` and `GetValueTexture3D`, which is why they come before
the `Effect` family rather than after it.

## Two limits, and they are not the same kind of thing

**No `Texture3D` can be constructed on this device, and that is XNA's rule.**
`ValidateCreationParameters`' second test is
`if (MaxVolumeExtent == 0) Throw(ProfileFeatureNotSupported, "Texture3D")`, and
the table extracted from the pinned assembly in Foundation 62 gives Reach a
`MaxVolumeExtent` of **0** and an empty `ValidVolumeFormats`. Every constructor
call raises `NotSupportedException` before any native route is reached.

CNA reaches the same answer independently, which is worth stating because it is
the case where the two authorities happen to agree and it would be easy to
report the wrong one. `build-probe/f64_cube.c`:

```text
texturecube_create(4, Color)   -> 0
   info -> 0 size=4 levels=1 format=0
   face 0..5: set=6 get=6      (6 = CNA_RESULT_NOT_SUPPORTED)
texture3d_create(2x2x2, Color) -> 6
render_target_cube_create(4)   -> 0
```

The refusal this binding raises is XNA's message, not CNA's result code. The
type is **complete and implemented**, unreachable on this profile rather than
absent; on a `HiDef` device — extent 256, fifteen volume formats — the same code
constructs one.

**A `TextureCube` can be created here but its faces cannot be moved**, and that
is the *renderer's* limit rather than an XNA rule. `cna_texturecube_set_data`
and `_get_data` answer `NOT_SUPPORTED` for all six faces on the qualified
HEADLESS artifact. CNA's own header calls that a legitimate outcome —
*"unsupported storage returns NOT_SUPPORTED atomically"* — so this is a host
capability, not a defect, and the six transfer members ship: every XNA
validation in front of them is reachable and asserted, and the transfer is
asserted to be exactly that refusal on the runtime channel and nothing else.

This is deliberately **not** the Foundation 63 decision about the draws. The
draws would have been *wrong*: XNA raises `InvalidOperationException`
(`CannotDrawNoShader`) for a case a consumer can actually reach, and the
projection could only have answered a native result code there. Here XNA
**succeeds** and the host cannot; a binding is allowed to report a host limit,
and the `CNAError` channel is where it reports one.

### Native evidence: the same code on a different artifact

Run against CNA's SOFTWARE artifact, unchanged:

```text
texturecube_create(4, Color)   -> 0
   face 0: set=0 get=0 first=(0,0,0,255) last=(60,30,15,255)
   ... all six faces identical
texture3d_create(2x2x2, Color) -> 6
```

The faces round-trip, and `Texture3D` is refused there too. This is recorded as
**native evidence** and asserted nowhere: HEADLESS is the qualified renderer
(`docs/architecture.md`), and an observation from an unqualified artifact does
not belong in a test. It says the transfer code is right, not that this host
runs it.

`docs/runtime-capabilities.json` gains the row as `UNSUPPORTED_BY_RENDERER`,
beside pixel readback, which is the same shape.

## Four defects the IL found in code that was already green

Reading `Texture3D::CopyData` to write the new type turned up four things wrong
with the `Texture2D` that shipped in Foundation 57 and 59. This is the fourth
time in this session that implementing a check has falsified code that passed
its tests.

**1. An empty array is an `ArgumentNullException`.** `CopyData`'s second test is

```text
IL_0015: ldarg.3;  brfalse IL_0444      // data == null
IL_001b: ldarg.3;  ldlen; brfalse IL_0444   // data.Length == 0
...
IL_0444: ldstr "data"; call get_NullNotAllowed();
         newobj ArgumentNullException(string, string); throw
```

Both arms reach the same throw, so a zero-length array reports the parameter as
**null**. A Swift array cannot be null; it can be empty, and that is the
reachable half of XNA's own test. `VertexBuffer.CopyData` had this from
Foundation 60 and `Texture2D` did not.

**2. The array window is `ValidateCopyParameters`, not the total-size test.**
`Texture2D.transferPlan` checked `startIndex` and `elementCount` against the
array inline and raised `ArgumentException(InvalidTotalSize)` for both. XNA
calls `Helpers.ValidateCopyParameters`, which raises
`ArgumentOutOfRangeException(MustBeValidIndex)` naming **`dataIndex`** or
**`elementCount`** — a different exception class, a different message, a
different parameter name and a different `HResult`.

**3. And it runs before the size tests, not after.** `ValidateCopyParameters` is
`IL_0112`; `GetAndValidateSizes` is `IL_011d` and `GetAndValidateRect` is
`IL_0129`. A call that is wrong in both ways reported the element size where XNA
reports the window. Both are asserted now, in that order.

**4. `Texture3D`'s aspect ratio is over all three extents.** The volume
validator written earlier in this milestone compared `Max(width, height)` to
`Min(width, height)`. The IL is `Max(Max(width, height), depth)` over
`Min(Min(width, height), depth)` — a two-dimensional reading of a 256×256×1
volume computes a ratio of 1 where XNA computes 256.

Two of `CopyData`'s tests have **no reachable input** and are recorded in
`recorded-message-absences.json` rather than written as code that cannot run:

| test | why it is unreachable | when it returns |
| --- | --- | --- |
| `isActiveRenderTarget` → `MustResolveRenderTarget` | the flag is set by `GraphicsDevice.SetRenderTarget` | the render-target milestone |
| the `Textures`/`VertexTextures` scan → `ResourceInUse` (`E_ABORT`) | `TextureCollection` is not projected, so nothing can bind a texture | `TextureCollection` |

A third is **native-owned** and is not this binding's to raise:
`CannotUseFormatTypeAsManualWhenLocking` comes from
`IDirect3D9::CheckDeviceFormat(..., D3DUSAGE_DYNAMIC, D3DRTYPE_TEXTURE, format)`
— XNA asking the installed *driver* whether a format can be locked and
reporting its refusal. CNA publishes no route that asks that question, and
deciding for a driver would be an invented diagnosis.

## A gate that was reading less than it reported

`MESSAGES_REPRODUCED` went 130 → 167 in this milestone. Sixteen of those
thirty-seven are not new messages at all. `tools/api_compat/message_coverage.py` walks the call
graph inside a class so a message raised by a private helper counts for the
public member that reaches it. Its call-site pattern was `::(\w+)\(` — a name
immediately followed by a parenthesis. A call to a **generic** method is spelled

```text
call instance void Texture2D::CopyData<!!0>(!!0[], int32, int32, ...)
```

with the instantiation in between, so the pattern matched nothing and the walk
never entered `CopyData` at all. Every message reachable only through a generic
helper was invisible: the whole `CopyData<T>` family of `Texture2D`,
`TextureCube`, `Texture3D`, `VertexBuffer` and `IndexBuffer` — which is where
all texture and buffer transfer validation lives.

The gate reported `PASS` the entire time, because a message it cannot see is
neither reproduced nor a finding. Fixing the pattern raised the count by sixteen
on the types that were already implemented, and surfaced one real finding — the
driver-owned message above. The remaining twenty-one are `TextureCube` and
`Texture3D`'s own, which the same walk now reads.

`generic-call-sites-unreadable` is a new mutation control — the disassembly with
every generic call site renamed, which must lower the count — and a new
self-test asserts the walk enters a generic callee. `MESSAGE_COVERAGE_MUTATIONS`
goes 9 → 10 and `MESSAGE_COVERAGE_SELF_TESTS` 9 → 11.

## What each type reproduces

`TextureCube.ValidateCreationParameters`, in order:

```text
if (size <= 0) throw ArgumentOutOfRangeException("size", MustBeGreaterThanZero);
compressed = CheckCompressedTexture(format);
if (!ValidCubeFormats.Contains(format))
    Throw(ProfileFormatNotSupported, "TextureCube", format);
if (size > MaxCubeSize)  Throw(ProfileTooBig, "TextureCube", MaxCubeSize);
if (!NonPow2Cube && !IsPowerOfTwo(size))
    Throw(ProfileNotPowerOfTwo, "TextureCube");
if (compressed && (size & 3) != 0) throw ArgumentException(DxtNotMultipleOfFour);
```

Three differences from `Texture2D`'s, and each is a separate way to be wrong:
the format list is `ValidCubeFormats` (seven formats on Reach, against nine
texture formats); the size limit is `MaxCubeSize` (512, a quarter of
`MaxTextureSize`); and the power-of-two rule reads `NonPow2Cube` and applies
**unconditionally** — no mipmap or compression qualifier, and the plain
`ProfileNotPowerOfTwo` message rather than `Texture2D`'s mipmap-specific one. A
512 cube is accepted and a 1024 cube is refused, which is what makes `<=` and
`<` distinguishable.

`Texture3D.GetAndValidateBox` is `GetAndValidateRect` in three dimensions:

```text
if (box.Right > Width) fail;  if (box.Left >= box.Right)  fail;
if (box.Bottom > Height) fail; if (box.Top >= box.Bottom) fail;
if (box.Back  > Depth)  fail;  if (box.Front >= box.Back) fail;
throw new ArgumentException(InvalidRectangle, "box");
```

All six comparisons are `bgt.un`/`bge.un`, **unsigned**, so a negative
coordinate wraps to a value near `UInt32.max` and is caught by the first test it
meets rather than by a signed `>= 0` guard XNA does not have. The parameter name
is `"box"`, which is not the name of any parameter the six overloads take; that
is XNA's and it is reproduced rather than corrected.

Because no `Texture3D` exists on a Reach device, the box arithmetic and the four
volume checks behind the zero-extent test would be untestable where they live.
The box predicate is therefore a shared helper in `TextureDataTransfer.swift`
and is tested directly, and the volume checks are tested against the **HiDef**
row of the extracted table — the same table
`Foundation62ProfileCapabilityTests` compares back against the assembly.

## The ten routes

`cna_texturecube_create`, `_destroy`, `_get_info`, `_set_data`, `_get_data` and
the five `cna_texture3d_*` counterparts, with six new mirrored layouts
(`CNA_TextureCubeCreateInfo`, `_Info`, `_Transfer` and the three volume ones)
and the `CNA_CubeMapFace` alias. `ABI_MISMATCHES=0`; `LAYOUT_FIELDS` 322.

`CONSTANTS` goes 215 → 221: `SetData` and `GetData` pass `CubeMapFace.rawValue`
straight into `CNA_TextureCubeTransfer::face`, so the six face values are a
boundary dependency — and one **no runtime observation on this artifact could
check**, because the transfer answers `NOT_SUPPORTED` whichever face it is
given. Six `_Static_assert`s in `tools/native_abi/probe.c` are the whole of the
evidence that the pass-through is safe. Foundation 45 found `BlendFunction`'s
`Min` and `Max` swapped between the two headers; agreement is checked, never
assumed.

`cna_render_target_cube_create` answers 0 on this artifact and is **not** bound:
`RenderTargetCube` is the next milestone's, and `docs/native-abi.md` forbids
binding a route ahead of the member that needs it.

## Falsifiability

Thirteen new mutations, each planted and each caught — after one round in
which `texture2d-empty-array-not-reported-as-null` **survived**, because the new
check on `Texture2D` had been written with a test only for `TextureCube`'s. The
test was added and the mutation re-planted:

```text
cube-size-limit-is-the-texture-limit          cube-power-of-two-test-dropped
cube-format-list-is-the-texture-list          volume-zero-extent-test-dropped
volume-extent-guards-run-after-the-profile-test
volume-aspect-ratio-ignores-the-depth         cube-empty-array-not-reported-as-null
cube-window-not-validate-copy-parameters      cube-disposal-checked-after-the-arguments
cube-transfer-does-not-reach-the-route        volume-box-test-signed-not-unsigned
texture2d-empty-array-not-reported-as-null
texture2d-window-not-validate-copy-parameters
```

`cube-transfer-does-not-reach-the-route` is the one that keeps the renderer's
refusal honest: replacing the native call with a success makes the transfer test
green in the wrong direction, and the test catches it because it asserts *which
route answered with which code*, not merely that something was thrown.

Nothing about the transfer **plan** — the face, the level, the rectangle — is
observable on this artifact, for the same reason `vertex-offset-not-carried` was
withdrawn in Foundation 63: the only thing that would notice is the transfer
itself, which the renderer refuses. Those claims are recorded as **not yet
evidence** and become falsifiable on an artifact whose storage answers.

Two existing controls needed repairing for a reason worth recording, because it
is the same mistake in two places: **a control whose subject a later milestone
can legitimately change is not a control.**

* `aspect-ratio-divides-the-wrong-way` matched a line that `Texture3D`'s
  validator now duplicates exactly. Its site is extended through the type name
  in the message, which is what actually distinguishes the two.
* `verify.py`'s self-test for the member-scoped `inout` rule used **`Texture3D`**
  as its negative control — an owner chosen precisely because it was
  unprojected. Projecting it turned the control into a positive and the
  self-test failed. The control is now an owner that cannot ever be named,
  `NoSuchTextureType`.

`PROJECTION_MUTATIONS=168`; the full run is repeated before the final handoff.
