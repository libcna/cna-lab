# Foundation 34 — `GameComponent`, and the first visible cost of reference counting

With the engine in place, `GameComponent` closes: its base is `System.Object`,
its three interfaces — `IGameComponent`, `IUpdateable`, `System.IDisposable` —
are all projected, and its `Dispose(bool)` needs `Game.Components`, which now
exists. It is complete on the first build, thirteen declared members.

## The three details a reimplementation gets wrong

```text
.ctor(Game game)     enabled = true          // BEFORE base..ctor()
                     base..ctor()
                     game = game             // no null check
set_Enabled(v)       if (enabled == v) return
                     enabled = v; OnEnabledChanged(this, EventArgs.Empty)
set_UpdateOrder(v)   if (updateOrder == v) return
                     updateOrder = v; OnUpdateOrderChanged(this, EventArgs.Empty)
OnEnabledChanged(sender, args)
                     if (EnabledChanged != null) EnabledChanged(this, args)
```

- a component starts **enabled** and at `UpdateOrder` zero;
- both setters compare first and raise **nothing** when the value is unchanged,
  so assigning the same order twice makes the game re-sort once, not twice;
- `OnEnabledChanged` and `OnUpdateOrderChanged` **ignore the `sender` they are
  handed** and raise with `this`. The parameter is part of the signature and
  not part of the behaviour. A test passes a different sender and asserts the
  component comes back.

## `Dispose`

```text
Dispose()            Dispose(true); GC.SuppressFinalize(this)   // virtual FINAL
Dispose(disposing)   if (!disposing) return
                     lock (this) {
                         if (Game != null) Game.Components.Remove(this)
                         if (Disposed != null) Disposed(this, EventArgs.Empty)
                     }
```

`Dispose()` is `virtual final` — a sealed `IDisposable` implementation — so it
is `final` here and `Dispose(_:)` is the override point. `SuppressFinalize` has
no analogue: Swift has no finalizer to suppress, and inventing one would add
lifetime behaviour XNA does not have.

The removal is what drives the game's `GameComponentRemoved` handler, so a
disposed component stops being updated. `Disposed` is raised **after** the
removal, so a handler observes a component the game has already let go of — a
test reads `Game.Components.Count` from inside the handler and asserts zero.
`disposing: false` is the finalizer path and does nothing at all, which is
transcribed rather than dropped.

`lock (this)` becomes a private lock: the same mutual exclusion, without
publishing the object as a monitor — which nothing in the projected surface
would let a caller use anyway.

## The parent reference is a cycle, and it is stated

`get_Game` is a plain field read of the constructor's argument, so a component
holds its game and `Game.Components` holds the component. That is a reference
cycle. The CLR has the identical cycle and a collector that does not care;
Swift has neither.

It is reproduced rather than weakened, because the pinned inventory proves
`Game` **non-null** and a `weak` field would have had to answer nil — a state
XNA cannot produce, and exactly the kind of invention the return-nullability
rule forbids in the other direction too.

`Dispose()` is what XNA gives a consumer to break it: it removes the component
from `Game.Components`, dropping the game's half. A component that is never
disposed and whose game is dropped keeps both alive.

**This is the first projected type where the difference between a garbage
collector and reference counting is visible at all**, and it is recorded here
rather than discovered later. Everything projected before it was either a value
type, an owned child, or a support class with no back-reference.

## `DrawableGameComponent` is still blocked, and by what

Its own IL is as plain as `GameComponent`'s, but `get_GraphicsDevice` resolves
`IGraphicsDeviceService` out of `Game.Services` — which needs
`GameServiceContainer`, which needs `System.Type`. Nothing here fabricates a
graphics service producer, so the type stays missing with a named blocker
rather than being half-built.

## Scoreboard

```text
TARGET_TYPES=141                     (140 -> 141)
TARGET_MEMBERS=1762                  (1749 -> 1762)
TOTAL_DIAGNOSTICS=271                (272 -> 271)
COMPLETE_TYPES=134                   (133 -> 134)
MISSING_TYPE=116                     (117 -> 116)
MISSING_MEMBER=130                   unchanged
every mismatch/leak category unchanged
UNMEASURED_STRUCTURAL_CATEGORY=0     ALLOWLIST_ENTRIES=0

DEBUG_TESTS=399 PASS                 (393 -> 399)
```
