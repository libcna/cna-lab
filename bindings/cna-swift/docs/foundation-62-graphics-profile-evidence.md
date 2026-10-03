# Foundation 62 — `GraphicsProfile`, and the capability table nine messages waited on

```text
MISSING_MEMBER      58 -> 57      TOTAL_DIAGNOSTICS  158 -> 157
MESSAGES_DEFERRED   13 -> 4       MESSAGES_REPRODUCED 114 -> 123
XNA_RESOURCE_STRING_PROJECTIONS  46 -> 55
BOUND_FUNCTIONS    106 -> 107     ABI_MISMATCHES=0
tests              647 -> 654
```

Nine deferred messages left `recorded-message-absences.json`: `Texture2D`'s six
and the two buffer constructors' three. All nine were waiting on the same thing
— the device's own profile — and Foundation 60's re-measurement found the route
that answers it.

## The table is extracted, not transcribed

`ProfileCapabilities` is a `private` XNA class whose class constructor builds one
instance per profile out of literal stores:

```text
newobj  ProfileCapabilities::.ctor();  stloc.1
ldloc.1; ldc.i4 0xffff;                stfld MaxPrimitiveCount
ldloc.1; ldfld ValidTextureFormats; ldc.i4.0; callvirt List`1::Add(!0)
ldloc.1;                               stsfld ProfileCapabilities::Reach
```

Thirty-two values per profile, sixty-four in total, of which nine limits and one
format list are load-bearing for the messages above. A table that size copied by
hand is wrong in one place and never noticed, so it is **not** copied by hand.

`tools/api_compat/profile_capabilities.py` walks that class constructor in the
hash-registered `Microsoft.Xna.Framework.Graphics.dll`, reads each `ldc`/`stfld`
pair and each `ldfld`/`Add` triple, and writes
`reference/xna40-profile-capabilities.json`. `--check` re-extracts and fails on
any drift, exactly as the resource-string reference is checked. The Swift table
is generated from that file, and
`Foundation62ProfileCapabilityTests.testTheSwiftTableMatchesTheExtractedOne`
compares all sixty-four values back against it.

The extractor refuses rather than defaults. A field it does not know, a field a
profile never stores, a `List.Add` with nothing before it, an unexpected static
— each is an error, because a limit that silently reads zero is a check that
silently never fires. One assumption it *does* make is stated: an empty format
list is a **real value**, not a parse failure, because Reach genuinely supports
no volume textures and no vertex textures and has no invalid filter or blend
formats — four of its eight lists are empty. What no profile can have is an
empty `ValidTextureFormats` or `ValidDepthFormats`, and that is the invariant
that would catch a list walk which read nothing.

Measured, from the assembly:

```text
                      Reach        HiDef
Profile                   0            1
MaxPrimitiveCount     65535      1048575
MaxTextureSize         2048         4096
MaxCubeSize             512         4096
MaxVolumeExtent           0          256
MaxTextureAspectRatio  2048         2048
MaxVertexBufferSize  67108863    67108863
MaxIndexBufferSize   67108863    67108863
MaxVertexStreams         16           16
MaxRenderTargets          1            4
MaxVertexSamplers         0            4
IndexElementSize32    false         true
NonPow2Unconditional  false         true
ValidTextureFormats   0...8       0...19
```

## The getter does not throw, so the read does not happen in it

`get_GraphicsProfile` is `ldarg.0; ldfld _profileCapabilities; ldfld Profile;
ret` — two field reads, `IL_NO_FAILURE_PATH`, so the Swift property must not
throw. Reading `cna_graphics_device_get_graphics_profile` from inside it would
put a fallible native call behind an infallible getter, which is the exact
mistake `GraphicsDevice.PresentationParameters` is **deliberately absent** to
avoid.

So the route is called **once**, where a failure can still be reported: the
first time a facade of this runtime is built, from both places that build one.
The answer is held on `RuntimeState`, for the same reason every other
device-owned value is — the handle is a per-callback capability token and a
facade cannot carry state across callbacks (Foundation 42's measurement). XNA
does the same thing for the same reason: it reads the profile once at device
creation and every later reader is a field read.

`GraphicsDeviceManager.GraphicsDevice` is an `IGraphicsDeviceService` getter that
cannot throw and already answers nil for every other reason the device is
unavailable, so a profile that cannot be read answers nil there rather than
inventing one.

## A test that was wrong, and the check that proved it

Foundation 60 and 61 both created **32-bit index buffers** and asserted they
round-trip. With the profile check in place, three tests failed:

```text
Foundation60BufferTests.testIndexDataRoundTripsAtBothWidths
Foundation60BufferTests.testTheIndexTypeOverloadMapsTheFourWidths
Foundation61DynamicBufferTests.testDynamicIndexBufferRoundTripsWithOptions
```

The device is `Reach`, and Reach's `IndexElementSize32` is **false**. XNA refuses
a 32-bit index buffer on this device, and the projection had been accepting one.
The tests asserted behaviour XNA does not have, and passed only because the
check was missing.

They now assert the refusal, with the message the assembly carries:

```text
XNA Framework Reach profile does not support 32 bit indices. Use
IndexElementSize.SixteenBits or a type that has a size of two bytes.
```

and the round-trips moved to the sixteen-bit width. This is the third time in
this session that implementing a check has falsified a test that was green:
a green test proves the code does what the test says, and says nothing about
whether the test says what XNA does.

## What each check reproduces

`Texture2D.ValidateCreationParameters`, after the two dimension guards the
constructor already had:

```text
if (!ValidTextureFormats.Contains(format))
    Throw(ProfileFormatNotSupported, "Texture2D", format);
if (width > MaxTextureSize || height > MaxTextureSize)
    Throw(ProfileTooBig, "Texture2D", MaxTextureSize);
if ((Max(w,h) + Min(w,h) - 1) / Min(w,h) > MaxTextureAspectRatio)
    Throw(ProfileAspectRatio, "Texture2D", MaxTextureAspectRatio);
if (!NonPow2Unconditional && !(IsPowerOfTwo(w) && IsPowerOfTwo(h))) {
    if (mipMap)     Throw(ProfileNotPowerOfTwoMipped, "Texture2D");
    if (compressed) Throw(ProfileNotPowerOfTwoDXT,    "Texture2D");
}
if (compressed && ((w & 3) != 0 || (h & 3) != 0))
    throw new ArgumentException(DxtNotMultipleOfFour);
```

The aspect-ratio arithmetic is transcribed literally rather than simplified: it
is a **ceiling** division of the longer side by the shorter, so 2048×1 is
exactly at Reach's limit and is accepted — which a test asserts, because a
rounded-down division would reject it.

`ThrowNotSupportedException` puts the **profile itself** in `{0}` and the
caller's arguments after it. A message naming only the limit would be a
different message, so the formatting is reproduced rather than approximated.

`VertexBuffer.CreateBuffer` compares `stride * count` against
`MaxVertexBufferSize`, and `IndexBuffer.CreateBuffer` refuses 32-bit indices
**before** it compares `count * width` against `MaxIndexBufferSize` — both
comparisons `ble.un`, unsigned, and both written that way.

## What is still deferred

Four messages, down from thirteen. They are no longer a profile family.

## Falsifiability

Six mutations, each planted and each caught: the table answering the other
profile, one extracted limit changed (which the pinned comparison must refuse),
the 32-bit refusal dropped, the ceiling division written as a floor, a message
that omits the profile, and a texture-size limit that is not checked.

`PROJECTION_MUTATIONS=149`; the full run is repeated before the final handoff.

## Runtime capabilities

`docs/runtime-capabilities.json` gains two rows: the device's profile, and the
capability table's provenance.
