# Foundation 55: the Texture2D constructors

Two constructors. `TOTAL_DIAGNOSTICS` 177 → 175, `MISSING_MEMBER` 71 → 69, and
the first way this binding can make a texture that did not come from an encoded
image.

## The short constructor is the long one with two literals

```text
.ctor(GraphicsDevice, int32 width, int32 height)      30 bytes
  CreateTexture(device, width, height, false, 0, 1, SurfaceFormat.Color)

.ctor(GraphicsDevice, int32, int32, bool mipMap, SurfaceFormat format)   32 bytes
  CreateTexture(device, width, height, mipMap, 0, 1, format)
```

`ldc.i4.0` for `mipMap` and `ldc.i4.0` for `format` — `SurfaceFormat.Color` is
the zero-valued case — so the three-parameter constructor is not a different
creation path. The two arguments between them are `ldc.i4.0` and `ldc.i4.1` in
**both**, so neither is a projected parameter: they are `CreateTexture`'s own
D3D usage and pool, and CNA's create info has no counterpart for either.

Both constructors wrap the call in a `try`/`finally` whose handler is
`this.Dispose(true)` — a texture whose creation throws disposes itself. A Swift
initializer that fails never produces an object, so there is nothing to dispose
at that level; but the *native* texture can already exist when the follow-up
`get_info` fails, and it is destroyed there. That is XNA's `finally` at the
only place a Swift initializer has for one.

## The validation that runs first

`Texture2D.ValidateCreationParameters(width, height, format, mipMap)` opens with
two `bgt` tests against zero:

```text
if (width  <= 0) throw new ArgumentOutOfRangeException("width",  ResourcesMustBeGreaterThanZeroSize);
if (height <= 0) throw new ArgumentOutOfRangeException("height", ResourcesMustBeGreaterThanZeroSize);
```

One message, two parameter names, and the width first — so a texture invalid in
both dimensions blames the width, which the test pins. The message is pinned
from the registered assembly (`RESOURCE_STRINGS_REPRODUCED` 34 → 35).

## Seven messages the gate demanded, and the blocker that answers them

The message-coverage gate turned red the moment these constructors stopped
being missing — which is what it is for — with eight findings. One of them,
`ResourcesMustBeGreaterThanZeroSize`, is projected above. The rest are the
**GraphicsProfile capability family**:

```text
DxtNotMultipleOfFour      ProfileNotPowerOfTwoDXT     ProfileTooBig
ProfileAspectRatio        ProfileNotPowerOfTwoMipped  ProfileFormatNotSupported
```

Every one of those checks reads the **device's** profile, and
`GraphicsDevice.GraphicsProfile` is not a projected member. The limits
themselves are XNA authority and could be transcribed; the profile in force is
not knowable here, and inventing one would fabricate the fact the check depends
on. They are recorded as `deferred` with that blocker named, and the
`MESSAGES_DEFERRED` count is printed on its own line so the gap cannot hide.

CNA refuses what it cannot create on its own, which corroborates rather than
replaces that: `build-probe/f55_grants.c` asks for a DXT1 texture and gets
`CNA_RESULT_NOT_SUPPORTED`, on the runtime channel.

`StreamNotSeekable` is recorded as `unreachable` instead: it guards a seek back
after probing the stream, and this projection's `FromStream` reads forward-only
into memory and never seeks — `Foundation.InputStream` has no seek to offer.

## What CNA grants, and the mutation that had to move

```text
build-probe/f55_grants.c
  asked 8x8       mip=0 -> granted 8x8       levels=1
  asked 7x3       mip=0 -> granted 7x3       levels=1
  asked 1x1       mip=0 -> granted 1x1       levels=1
  asked 16x16     mip=1 -> granted 16x16     levels=5
  asked 5000x5000 mip=0 -> granted 5000x5000 levels=1
  asked 8x8 DXT1        -> CNA_RESULT_NOT_SUPPORTED
```

The dimensions come back exactly as asked, every time — non-power-of-two and
five thousand pixels alike. So a projection that reported the *request* and one
that reported the *grant* are indistinguishable in width and height, and the
mutation written to separate them survived. It was retargeted at the one field
where the grant is not the request: a mipped 16×16 comes back with five levels,
and nothing in the request says five.

Reading the values back from CNA is still what the code does, for the reason
`RenderTarget2D` does it: "preferred" is what the parameter name means, and a
future renderer that rounds a texture up would be reported honestly rather than
quietly.

## Falsifiability

Four mutations, 103 → 107:

```text
texture-dimension-guard-accepts-zero               `>= 0` instead of `> 0`
texture-dimension-guard-blames-the-wrong-parameter the height refusal naming width
short-texture-constructor-defaults-to-mipmaps      the forwarded ldc.i4.0 changed
created-texture-reports-a-level-count-of-its-own   the granted level count replaced
```

## Result

```text
601 tests, 0 failures
PROJECTION_MUTATIONS=107 CAUGHT=107 SURVIVORS=0
BOUND_FUNCTIONS=85 LAYOUTS=27 LAYOUT_FIELDS=235 ABI_MISMATCHES=0
TOTAL_DIAGNOSTICS=175 MISSING_MEMBER=69
MESSAGE_COVERAGE_FINDINGS=0  MESSAGES_DEFERRED=10
RESOURCE_STRINGS_REPRODUCED=35
```
