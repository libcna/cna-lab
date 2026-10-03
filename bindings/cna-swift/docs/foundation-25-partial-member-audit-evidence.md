# Foundation 25 — auditing the runtime partials' missing members

The five protected runtime partials carried 131 `MISSING_MEMBER` diagnostics.
Every previous milestone treated that number as a block: those types need a real
graphics runtime, so their members wait. This milestone asked the narrower
question the earlier ones never put mechanically — **is every one of those 131
members actually native?** — and answered it from the IL rather than from the
type's reputation.

One is not, and it is now implemented.

## The measurement

A member is *native-tainted* when its transitive call graph inside the seven
registered assemblies reaches any of:

- a `calli` — an indirect call through a function pointer, which is how
  C++/CLI reaches unmanaged code;
- a `pinvokeimpl` declaration;
- a method with no body that is not abstract — runtime- or native-implemented;
- a `<global>` C++/CLI free function.

Tainting is propagated backwards over the call graph the accessor inventory
already builds, so a member is clean only if nothing it can reach is native. A
member is separately blocked when its signature names a still-missing XNA type
or an undecided BCL type.

Over the five partials:

| Outcome | Members |
|---|---:|
| reaches a native boundary | 79 |
| names a still-missing XNA type | 15 |
| clean, and already implemented | 8 |
| clean, and still missing | **29** |

The 29 are not automatically implementable: "XNA implements this in managed
code" is not "CNA can implement it faithfully". CNA's game loop, device
creation and window are native, so a managed-only reimplementation of a member
that feeds them would store a value nothing reads. That is fabrication, and the
protected-partial rule forbids it. Each of the 29 was judged on whether a
faithful implementation exists **using only what CNA already has**.

## The one that qualifies: `Texture2D.Bounds`

```text
Texture2D::get_Bounds
  IL_0000:  ldloca.s   V_0
  IL_0002:  ldc.i4.0
  IL_0003:  ldc.i4.0
  IL_0004:  ldarg.0
  IL_0005:  ldfld      int32 Texture2D::_width
  IL_000a:  ldarg.0
  IL_000b:  ldfld      int32 Texture2D::_height
  IL_0010:  call       instance void Rectangle::.ctor(int32, int32, int32, int32)
  IL_0015:  ldloc.0
  IL_0016:  ret
```

A rectangle at the origin over the texture's own dimensions. No branch, no call
out of the assembly, no failure path — the pinned accessor inventory agrees:
`IL_NO_FAILURE_PATH`.

`Width` and `Height` are already CNA members, read once from
`cna_texture2d_get_info` at construction and stored as `let`s, and `Rectangle`
is a complete type. So the Swift implementation composes members that already
exist, exactly as XNA composes its own fields:

```swift
public var Bounds: Microsoft.Xna.Framework.Rectangle {
    Microsoft.Xna.Framework.Rectangle(0, 0, Width, Height)
}
```

It reaches no native surface of its own, adds no ABI symbol, and cannot fail.

## The 28 that do not, and why

| Members | Why a managed implementation would be a fabrication |
|---|---|
| `GraphicsDeviceManager.PreferredBackBufferWidth`/`Height`/`Format`, `PreferredDepthStencilFormat`, `GraphicsProfile`, `IsFullScreen`, `SynchronizeWithVerticalRetrace`, `PreferMultiSampling`, `SupportedOrientations` | each stores a field and sets `isDeviceDirty`; XNA's `ApplyChanges` then builds the device from them. `cna_graphics_device_manager_apply_changes` takes only the manager handle, so a managed preference would be stored and then silently ignored by an `ApplyChanges` that *is* implemented. |
| `GraphicsDeviceManager.OnDeviceCreated`/`OnDeviceDisposing`/`OnDeviceReset`/`OnDeviceResetting` | protected raisers for four events CNA has no native callback for; the events would exist and never fire. |
| `Game.IsFixedTimeStep`, `TargetElapsedTime`, `InactiveSleepTime`, `ResetElapsedTime` | consumed by the game loop, which is `cna_game_run_one_frame` and native. |
| `Game.IsActive`, `IsMouseVisible` | window-system state with no managed source in CNA. |
| `Game.OnActivated`, `OnDeactivated` | raisers for events with no native callback. |
| `Game.Dispose(Bool)` | the protected disposal half of a lifecycle whose other half is native. |
| `Game.ShowMissingRequirementMessage(Exception)` | takes `System.Exception`, an undecided BCL type. |
| `GraphicsDevice.IsDisposed`, `GraphicsProfile`, `PresentationParameters` | device state CNA's borrowed wrapper does not own and the ABI does not expose. |
| `Texture2D.FromStream(_:stream:width:height:zoom:)` | delegates to the private decoding constructor with `flags = zoom ? 3 : 1`; the resize and crop semantics are the decoder's, and no available evidence establishes that CNA's native decoder reproduces XNA's. |

Nothing here was skipped silently: every one has a recorded reason, and the
reason is a property of CNA's runtime or of an undecided mapping, not of the
XNA IL.

## Scoreboard

```text
                      before   after
TARGET_MEMBERS          1695    1696
MISSING_MEMBER           131     130
TOTAL_DIAGNOSTICS        287     286
```

`Texture2D` stays a partial type, and the other 130 members stay absent. No
CNA ABI symbol was added and no native behaviour was invented.

## Evidence

- A native-runtime assertion in `NativeLifecycleTests`: after a real 60-frame
  game has created a texture, `Bounds` is `(0, 0, Width, Height)` against the
  same texture's own dimensions.
- A projection test: a key path proves the declared type is the XNA `Rectangle`
  and that the reader does not throw, without needing a device to hold an
  instance.
