# Foundation 52: the manager's raisers, its `Disposed` event, and `Dispose(Bool)`

Six of the eleven members `GraphicsDeviceManager` still had missing.
`TOTAL_DIAGNOSTICS` 198 → 191, `MISSING_MEMBER` 84 → 78, and
`OVERLOAD_MAPPING_MISMATCH` 14 → 13 as `Dispose(Bool)` completed that overload
set.

The five that remain — `FindBestDevice`, `CanResetDevice`, `RankDevices`,
`OnPreparingDeviceSettings` and the `PreparingDeviceSettings` event — all take
or carry `GraphicsDeviceInformation` or `PreparingDeviceSettingsEventArgs`,
neither of which is projected. They are absent because their types are, which
is a `MISSING_TYPE` away rather than a decision.

## The raisers forward the caller's sender

```text
OnDeviceCreated(object sender, EventArgs args):
  IL_0000: ldarg.0; ldfld deviceCreated
  IL_0006: brfalse.s IL_0015
  IL_0008: ldarg.0; ldfld deviceCreated
  IL_000e: ldarg.1          // sender
  IL_000f: ldarg.2          // args
  IL_0010: callvirt Invoke(object, !0)
```

`ldarg.1` — the **caller's** sender. `Game.OnActivated` has the identical
signature and pushes `ldarg.0`, ignoring its own `sender` parameter, and this
projection already records that quirk. Two raisers of the same shape in the
same framework doing different things with the argument is exactly the kind of
detail that a reasonable guess gets wrong, so each is transcribed from its own
IL and `testTheRaisersForwardTheCallersSender` pins this one.

The native device events now go **through** the virtual raisers rather than
around them. XNA's own device path calls `OnDeviceCreated` and its three
neighbours instead of touching the delegate fields, so a subclass that
overrides one sees the device events it overrode for. The projection raised the
sources directly until now, which would have skipped every override silently;
the mutation that puts that back is caught by a subclass test.

## `Dispose(Bool)`

```text
if (!disposing) return;
if (game != null) {
    if (game.Services.GetService(IGraphicsDeviceService) == this)
        game.Services.RemoveService(IGraphicsDeviceService);
    game.Window.ClientSizeChanged       -= GameWindowClientSizeChanged;
    game.Window.ScreenDeviceNameChanged -= GameWindowScreenDeviceNameChanged;
    game.Window.OrientationChanged      -= GameWindowOrientationChanged;
}
if (device != null) { device.Dispose(); device = null; }
if (Disposed != null) Disposed(this, EventArgs.Empty);
```

Three things in that body are stated rather than left to be inferred.

**It removes `IGraphicsDeviceService` and not `IGraphicsDeviceManager`.** The
constructor registers both; disposal takes back one. A test asserts the manager
service is still registered afterwards, because the obvious symmetry is wrong.

**The `== this` guard is reachable, and takes its other branch on a second
`Dispose`.** After the first, the container holds no device service, so the
comparison fails and nothing is removed — while `Disposed` is raised anyway,
because that raise is not inside the `game != null` block. `Dispose` is
therefore *not* idempotent in the event it raises, which the test pins at two
raises for two calls.

**`device.Dispose()` has no counterpart here.** XNA's manager owns a device it
created; CNA owns this one, hands it out per callback and destroys it with the
game, so the facade is a borrowed token and disposing it would be wrong. The
native manager's own destruction is what `storage.dispose` performs, and that
is the honest counterpart. The three window unsubscriptions have no counterpart
either — `GameWindow` is not projected, so there was never a subscription — and
the constructor already records the same absence at the other end.

`Disposed` is raised **last**, after the removal and the native teardown. The
mutation that hoists it to the top of the method is caught by a handler that
looks at the container while it runs.

`Dispose()` is `Dispose(true)` followed by `GC.SuppressFinalize(this)`. There
is no finalizer to suppress in Swift; `deinit` is the finalizer's projection,
and the forwarding is what a subclass overriding `Dispose(Bool)` observes.

## Falsifiability

Six mutations, taking the harness from 90 to 96:

```text
raiser-substitutes-itself-for-the-sender  `self` where the IL passes ldarg.1
two-raisers-crossed                        OnDeviceReset raising the wrong event
native-events-bypass-the-raisers           an override that never runs
dispose-false-still-disposes               the `disposing` guard defeated
dispose-removes-the-manager-service        the wrong service taken back
disposed-raised-before-the-removal         a handler seeing a stale container
```

Two survived their first spot check. `native-events-bypass-the-raisers` had no
test driving the native dispatch through an override, and
`disposed-raised-before-the-removal` was written to move the raise above the
wrong two statements — above the native teardown rather than above the service
removal, which changed nothing a handler could see. Both were fixed by making
the effect observable rather than by softening the mutation.

## Result

```text
588 tests, 0 failures
PROJECTION_MUTATIONS=96 CAUGHT=96 SURVIVORS=0
TOTAL_DIAGNOSTICS=191 MISSING_MEMBER=78 OVERLOAD_MAPPING_MISMATCH=13
MESSAGE_COVERAGE_FINDINGS=0
```
