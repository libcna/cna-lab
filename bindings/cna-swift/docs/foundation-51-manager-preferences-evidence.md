# Foundation 51: the GraphicsDeviceManager preferences

## What landed

Nine preferences, the two writer methods that validate, and `ToggleFullScreen`
— ten of the twenty-one members `GraphicsDeviceManager` was missing:

```text
GraphicsProfile              PreferredBackBufferFormat
PreferredBackBufferWidth     PreferredBackBufferHeight
PreferredDepthStencilFormat  IsFullScreen
PreferMultiSampling          SynchronizeWithVerticalRetrace
SupportedOrientations        ToggleFullScreen()
```

`TOTAL_DIAGNOSTICS` 208 → 198, `MISSING_MEMBER` 94 → 84.

## Why the values live in managed fields

Every one of XNA's getters is a bare `ldfld` and therefore `IL_NO_FAILURE_PATH`
— a non-throwing Swift reader. Reading a preference back from CNA on each
access would make every getter fallible and contradict the pinned verdict,
which is the same wall `GraphicsDevice.PresentationParameters` hit in
Foundation 48.

That is not a workaround; it is XNA's own design. The setters store a field and
set `isDeviceDirty`, and nothing reaches the device until `ChangeDevice`, which
`ApplyChanges`, `ToggleFullScreen` and `CreateDevice` all call. CNA's header
describes its side of the same division in as many words: *"Every preference
route here records a request; `cna_graphics_device_manager_apply_changes` is
what acts on it."*

So the managed field is the value, and CNA is told at the moments XNA tells
Direct3D. Nine setter routes are bound. **The nine matching getters are not**:
no projected member reads a preference back, and a route no member consumes is
what the native boundary exists to refuse.

## The two sides start in the same place

Measured, not assumed. `build-probe/f51_manager_prefs.c` reads CNA's manager
before anything is applied:

```text
before create_dev profile=0 800x480 back=0 depth=2 full=0 multi=0 vsync=1 orient=0
```

Every one of those is the pinned `.ctor`'s value, including the two that are
**not** the CLR default:

```text
IL_0000: ldarg.0; ldc.i4.1; stfld synchronizeWithVerticalRetrace   // true
IL_0007: ldarg.0; ldc.i4.2; stfld depthStencilFormat               // Depth24
IL_000e: ldsfld DefaultBackBufferWidth  = 0x320 = 800
IL_0019: ldsfld DefaultBackBufferHeight = 0x1e0 = 480
```

A projection that took `SynchronizeWithVerticalRetrace` or
`PreferredDepthStencilFormat` from the CLR zero would be wrong twice, and both
mistakes have their own mutation.

## The validation is XNA's, because CNA does not do it

```text
set_PreferredBackBufferWidth(int32 value):
  if (value <= 0)
      throw new ArgumentOutOfRangeException("value", BackBufferDimMustBePositive);
  backBufferWidth = value;
  useResizedBackBuffer = false;
  isDeviceDirty = true;
```

`bgt` against zero, so zero is refused with every negative. CNA would have
taken either: its header says the canonical setter "records whatever it is
given, including a value that no adapter can present", and the probe watched it
accept `-5` and read it back. The message names both dimensions whichever
setter raised it, and is pinned from the registered
`Microsoft.Xna.Framework.Game.dll` (`RESOURCE_STRINGS_REPRODUCED` 33 → 34).

`useResizedBackBuffer` is stored because the setters store it. Nothing reads
it: XNA's readers are all inside `ChangeDevice`'s window negotiation, and
`GameWindow` is not projected. That absence of readers is recorded in the field
itself rather than hidden by leaving the field out.

## The short-circuit `ApplyChanges` did not have

```text
ApplyChanges():
  if (device != null && !isDeviceDirty) return;
  ChangeDevice(false);
```

Twenty-five bytes, and the first fourteen are a guard this projection did not
have: with a device already made and no preference touched since,
`ApplyChanges` does **nothing**. It used to call
`cna_graphics_device_manager_apply_changes` unconditionally — a device
reconfiguration on every call.

"Does nothing" is hard to see, and the first two attempts to test it failed for
the same reason: CNA's preference setters *record*, so the device does not move
until something applies, and a device read after a call that pushed and one
that did not both answer 800. The test that works writes 512 into the native
preference behind the projection's back, calls the clean `ApplyChanges`, and
then applies natively — asking which preference was left standing:

* short-circuited → the 512 is still there → the device comes back **512**
* pushed → the managed 800 overwrote it → the device comes back **800**

`ToggleFullScreen` is tested the same way, and needed to be: the first version
of that test asserted only that the property flipped, and the mutation that
deletes its `ChangeDevice` call survived a full run.

## Falsifiability

Eight mutations, taking the harness from 82 to 90:

```text
vsync-default-taken-from-the-clr             the ctor's ldc.i4.1 lost
depth-preference-default-taken-from-the-clr  the ctor's ldc.i4.2 lost
dimension-setter-accepts-zero                `>= 0` instead of `> 0`
dimension-refusal-still-stores               a refused value taking effect
apply-changes-loses-its-short-circuit        reconfiguring on every call again
a-preference-setter-forgets-the-dirty-flag   a store that never applies
toggle-full-screen-does-not-apply            the flip without the device change
a-pushed-preference-is-dropped               the height never handed to CNA
```

Two of them were written before the tests could catch them, and each was fixed
by making the effect observable rather than by weakening the mutation.

**A false CAUGHT was recorded and thrown away along the way.** Spot-checking
`toggle-full-screen-does-not-apply` reported CAUGHT while the test file had a
compile error: a tree that does not build fails every mutation. The real
harness runs a baseline first and refuses when it is red; the ad-hoc loop used
for the spot check did not, and the result was believed for one minute. Re-run
with a verified baseline, the mutation genuinely survived, which is what led to
the test above.

## The two deferred messages, re-measured

Foundation 50 recorded `GamePad.InvalidController` and
`Keyboard.CouldNotReadKeyboard` as `deferred` — real error-channel differences
needing their own milestone. Looking at what that milestone would actually
contain changed the answer, and the recorded reasons now say so.

XNA splits XInput's result in two: `ERROR_DEVICE_NOT_CONNECTED` (0x48f) returns
a `GamePadState` with `IsConnected` false, and every *other* error raises
`InvalidOperationException(InvalidController)`. **CNA already matches the first
half**: `cna_gamepad_get_state` answers `SUCCESS` for a disconnected pad and
reports its real disconnected state, which
`docs/generated/gamepad-native-report.json` has been pinning all along.

The second half has no input that reaches it here. `PlayerIndex` is a closed
Swift enum, the handle is validated before the call, and no failure of that
route has been produced on the qualified artifact. Writing the diagnosis would
add a branch no test could reach and no mutation could falsify — which is the
rule Foundation 45's withdrawn dirty-flag mutation and Foundation 48's
unfalsifiable clear mask both follow. The entries stay `deferred`, but now with
the measurement and the actual blocker in them instead of a plan.

## Result

```text
581 tests, 0 failures
PROJECTION_MUTATIONS=90 CAUGHT=90 SURVIVORS=0
BOUND_FUNCTIONS=83 (74 + 9 setters) PROTOTYPE_TYPE_POSITIONS=258 ABI_MISMATCHES=0
TOTAL_DIAGNOSTICS=198 MISSING_MEMBER=84 COMPLETE_TYPES=150
MESSAGE_COVERAGE_FINDINGS=0 over 1,249 implemented members
RESOURCE_STRINGS_REPRODUCED=34
```
