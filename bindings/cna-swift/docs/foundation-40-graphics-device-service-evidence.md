# Foundation 40 — the `IGraphicsDeviceService` producer and `DrawableGameComponent`

`DrawableGameComponent` had been blocked since Foundation 24 on one thing:
`Initialize` resolves `IGraphicsDeviceService` out of `Game.Services`, and
nothing produced that service. XNA's own producer is
`GraphicsDeviceManager`, which registers itself in its own constructor. That is
what this milestone supplied — the real one, not a fabricated stand-in.

## 1. The producer, and the two-container question

`GraphicsDeviceManager..ctor(game)`, read out of the IL:

```text
defaults: SynchronizeWithVerticalRetrace = true, depthStencilFormat = Depth24
game == null                                  -> ArgumentNullException("game", GameCannotBeNull)
Services.GetService(IGraphicsDeviceManager) != null
                                              -> ArgumentException(GraphicsDeviceManagerAlreadyPresent)
Services.AddService(IGraphicsDeviceManager, this)
Services.AddService(IGraphicsDeviceService, this)
Window.ClientSizeChanged       += GameWindowClientSizeChanged
Window.ScreenDeviceNameChanged += GameWindowScreenDeviceNameChanged
```

The null-game branch is unreachable through a non-Optional Swift class
parameter. The two window subscriptions are **not** projected and not faked:
`GameWindow` is not implemented, so there is no event to subscribe to.

**There is only one container, and it is the managed one.** CNA's native game
owns the device and reads nothing from `GameServiceContainer`; that container is
a CLR projection whose only consumers are `Game.GraphicsDevice`,
`DrawableGameComponent`, and whatever a caller looks up. So the registration
adds a lookup XNA has, and no ownership XNA does not. That is the trap this
milestone was warned about, and measuring what CNA reads is what settles it.

The duplicate check happens **before** the native manager is created, so a
refused second manager leaves no native object behind — which is also the order
XNA checks in.

## 2. `IGraphicsDeviceService.GraphicsDevice` is Optional and infallible

The pinned evidence had already settled the requirement's shape: the one
registered implementor's accessor is a bare field read, the field is null until
`CreateDevice` and null again after `Dispose`, and XNA's own `BeginDraw`/
`EndDraw` guard it with `brfalse` rather than treating null as an error. So
"no device right now" is a normal result.

A CNA graphics device is callback-scoped, and CNA enforces that itself:
`cna_graphics_device_manager_get_graphics_device` answers
`CNA_RESULT_INVALID_STATE` with a zero handle before a game runs and after it
exits, and succeeds inside a lifecycle callback — measured by
`build-probe/f40_devicescope.c`. Outside a callback there is therefore no device
this service can hand out, and the answer is nil, which is the same state XNA
reports before the loop has created one.

That also fixes both `PROPERTY_MAPPING_MISMATCH` diagnostics that had stood
against `GraphicsDeviceManager.GraphicsDevice`, and the
`INTERFACE_MAPPING_MISMATCH` against the class.

### Three protocol witnesses, and a rule that was too narrow

`CreateDevice`, `BeginDraw` and `EndDraw` are **not** on
`GraphicsDeviceManager`'s pinned surface: XNA implements all three explicitly,
so they are private on the class. Swift has no explicit conformance — a witness
must be at least as visible as the protocol — so the three are public here, and
they are registered as `protocolWitnessMemberProjections` like the
packed-vector witnesses before them.

The rule that validates those registrations was hard-coded to
`IPackedVector<T>` and rejected the new entries. It is general now: the forcing
interface is whichever **direct** CLR interface of the owner declares the
member, found by walking interface inheritance — which is what reaches the
non-generic `IPackedVector` through ``IPackedVector`1`` and what reaches
`IGraphicsDeviceManager` directly. Generalizing it broke twenty-three
packed-vector registrations on the first attempt, because the member is declared
on the base interface and not on the generic one; the self-tests caught that
immediately.

## 3. `Game.GraphicsDevice` goes through the container

XNA's getter caches `graphicsDeviceService`, resolves it from `Services` on the
first read, raises `InvalidOperationException(NoGraphicsDeviceService)` when
there is none, and otherwise returns `service.GraphicsDevice` — which is
Optional. All of that is projected, cache included.

Before this milestone the property borrowed the device straight from the host,
which skipped the container entirely: a game with **no** `GraphicsDeviceManager`
got a device where XNA raises. That is fixed, and it is why several tests and
the template had to unwrap an Optional they previously did not have.

## 4. `DrawableGameComponent`

Complete, with zero diagnostics. `Initialize`'s body is XNA's, in order:

```text
base.Initialize()
if (initialized) return
deviceService = Game.Services.GetService(IGraphicsDeviceService) as …
if (deviceService == null) throw InvalidOperationException(MissingGraphicsDeviceService)
deviceService.DeviceCreated   += DeviceCreated     -> LoadContent()
deviceService.DeviceResetting += DeviceResetting   -> nothing
deviceService.DeviceReset     += DeviceReset       -> nothing
deviceService.DeviceDisposing += DeviceDisposing   -> UnloadContent()
if (deviceService.GraphicsDevice != null) LoadContent()
initialized = true
```

**The `initialized` guard is on the resolution and the subscription, not on
`LoadContent`.** The `DeviceCreated` subscription is what reloads content after
a device reset, and it stays subscribed. Guarding the reload as well would fix
startup and break every subsequent reset — the exact trap named in advance — and
a mutation that does it is caught.

`MissingGraphicsDeviceService` is a **different** string from
`Game.GraphicsDevice`'s `NoGraphicsDeviceService`. A mutation that confuses them
is caught.

`Visible` and `DrawOrder` compare before they store, so writing the same value
raises nothing; `Dispose(Boolean)` unloads content, removes all four device
handlers, and only then runs the base's disposal.

## 5. Scoreboard

```text
TARGET_TYPES     145 -> 146      COMPLETE_TYPES   138 -> 139
TARGET_MEMBERS  1801 -> 1818     MISSING_TYPE     112 -> 111
MISSING_MEMBER   112 -> 108      TOTAL_DIAGNOSTICS 245 -> 236
INTERFACE_MAPPING_MISMATCH  1 -> 0
PROPERTY_MAPPING_MISMATCH   4 -> 1   (only GraphicsDevice.Viewport's writer remains)
BOUND_FUNCTIONS  50 -> 55         PROTOTYPE_TYPE_POSITIONS 154 -> 170
PURE_XNA_DERIVED 2036 -> 2125
```

## 6. Falsifiability

Six new controls; five are caught and one was **removed as unfalsifiable**,
which is recorded rather than quietly dropped:

```text
CAUGHT  manager-registers-only-one-service            registered under one interface, not both
CAUGHT  duplicate-manager-accepted                    a second manager where XNA refuses one
CAUGHT  drawable-reload-guarded-by-the-one-time-flag  the device-reset reload suppressed
CAUGHT  drawable-shares-the-game-message              the two missing-service messages confused
CAUGHT  drawable-visible-raises-without-a-change      an unchanged write raising the event
```

`service-hands-out-a-device-outside-a-callback` removed the binding's own
`isInsideCallback` guard and **survived**, because CNA refuses that route
outside a callback on its own — measured above. Removing the guard therefore
changes nothing observable. The guard stays, as defence in depth and as a
statement of the scope rule in the binding rather than a reliance on the host to
keep enforcing it, and the source says exactly that.
