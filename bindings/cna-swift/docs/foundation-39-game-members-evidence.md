# Foundation 39 — `Game`'s timing, host state and events

`Game` had 18 missing members. It has 2: `Window` and `Content`, each blocked on
a type of its own.

## 1. What current CNA made reachable

Every one of these was a recorded blocker under CNA 0.7.0 and is a route now:

```text
Tick                 cna_game_tick
SuppressDraw         cna_game_suppress_draw
ResetElapsedTime     cna_game_reset_elapsed_time
IsActive             cna_game_get_is_active
IsMouseVisible       cna_game_get_is_mouse_visible / _set_is_mouse_visible
IsFixedTimeStep      cna_game_get_is_fixed_time_step / _set_is_fixed_time_step
TargetElapsedTime    cna_game_get_target_elapsed_time_ticks / _set_…
InactiveSleepTime    cna_game_get_inactive_sleep_time_ticks / _set_…
Activated Deactivated Exiting Disposed
                     cna_game_subscribe / cna_game_unsubscribe over
                     CNA_GAME_EVENT_ACTIVATED/_DEACTIVATED/_EXITING/_DISPOSED
```

Fourteen routes, one mirrored callback (`CNA_GameEventCallback`), no new
structure. `BOUND_FUNCTIONS` 36 → 50, `PROTOTYPE_TYPE_POSITIONS` 113 → 154,
`CALLBACKS` 3 → 4, zero mismatches.

## 2. Why five properties are managed mirrors

`IsActive`, `IsMouseVisible`, `IsFixedTimeStep`, `TargetElapsedTime` and
`InactiveSleepTime` all have `IL_NO_FAILURE_PATH` getters in the pinned
accessor verdicts, so **none of them may throw** — and a native round trip can
always fail on the owner thread or a stale generation. Each is therefore a
managed field, which is exactly what XNA has, seeded from the host at
construction.

The mirror moves **only when a write the host accepted succeeded**. A refused
write leaves it alone, so the getter never reports a state the host does not
hold; after disposal the setters are inert rather than lying. That is the one
place an infallible CLR setter meets a fallible host, and this is the rule.

Two of the CLR setters are *fallible* and project to throwing writer methods
instead, because Swift has no throwing property setter:

```text
SetTargetElapsedTime   refuses value <= Zero   ArgumentOutOfRangeException("value", TargetElaspedCannotBeZero)
SetInactiveSleepTime   refuses value <  Zero   ArgumentOutOfRangeException("value", InactiveSleepTimeCannotBeZero)
```

The two messages read alike and the two conditions do not: **zero separates
them**, and a test asserts both directions. XNA's own spelling of "Elasped" in
the resource key is kept.

`IsActive` deserves its own note. XNA's getter is
`isActive && !Guide.IsVisible` when GamerServices is initialized; GamerServices
is outside the selected profile and is never initialized, so the value is the
field. The field is seeded from `cna_game_get_is_active` and updated by CNA's
own Activated/Deactivated notifications — the same mechanism XNA uses.

`IsFixedTimeStep`'s setter is a recorded divergence in mechanism: the CLR setter
stores a field and calls nothing, because in XNA that field *is* what `Tick`
reads. Here the host owns the loop, so the write has to reach it or the
property would be inert.

## 3. The events, and the raise sites

`OnActivated`, `OnDeactivated` and `OnExiting` are the raise sites, as they are
in XNA, so an override that does not call `super` suppresses the event. The IL
raises with **`this`** as the sender and ignores the declared `sender`
parameter; that quirk is reproduced and asserted.

### The two things CNA calls "exiting"

This is the one the milestone was warned about, and it was real. A pure-C probe
measured them apart:

```text
                           at request_exit   at destroy (game never ran)
CNA_GameCallbacks.exiting       fires               fires
CNA_GAME_EVENT_EXITING          fires               does not fire
```

XNA raises `Exiting` when the game is exiting; disposing a game that never ran
raises nothing. So the **event** is the one that matches, and the lifecycle
callback is a teardown notification with no XNA counterpart.

The pre-existing code mapped the **callback** onto `Game.OnExiting`. That was
invisible for as long as `OnExiting`'s body was empty, and became a real
divergence the moment this milestone gave it a body: `Exiting` fired on
`Dispose()` for a game that had never run. The callback now maps to nothing,
the event is subscribed, and a test pins both halves of the table above.

`Disposed` is raised by `Dispose(Boolean)` after the children are released and
the native teardown has happened, which is where XNA raises it, and exactly
once.

## 4. `Dispose` split into XNA's two members

`Dispose()` is `virtual final` in the metadata — the sealed `IDisposable`
implementation — so it is `final` and its body is `Dispose(true)`.
`Dispose(Boolean)` is `family newslot virtual`: the one override point, and
where the whole teardown now lives.

## 5. `ShowMissingRequirementMessage`

Returns **`Boolean`**, not `void`. The CLR body is
`host != null ? host.ShowMissingRequirementMessage(e) : false`, and `GameHost`'s
own base returns `false`; only `WindowsGameHost` overrides it with a
`System.Windows.Forms` message box. CNA is the host here and shows no message,
so `false` is this host's answer rather than a stand-in — and it tells a caller
exactly what XNA tells it: the message was not shown, so rethrow.

## 6. Scoreboard

```text
TARGET_MEMBERS   1785 -> 1801     MISSING_MEMBER   128 -> 112
TOTAL_DIAGNOSTICS 262 -> 245      COMPLETE_TYPES   138 (unchanged)
Game missing members 18 -> 2      (Window, Content)
```

`Game` stays PARTIAL on exactly those two, and on the `GraphicsDevice` property
whose XNA getter resolves `IGraphicsDeviceService` out of `Services` — the next
milestone's subject.

## 7. Falsifiability

Six new controls, all caught:

```text
CAUGHT  event-sender-is-the-parameter          an On... method passing its sender parameter
CAUGHT  target-elapsed-accepts-zero            the strict bound loosened to the sleep-time one
CAUGHT  inactive-sleep-refuses-zero            the sleep-time bound tightened to the other
CAUGHT  mirror-moves-on-a-refused-write        the mirror reporting a state the host never took
CAUGHT  teardown-callback-mapped-onto-exiting  CNA's teardown notification mapped onto Exiting
CAUGHT  host-subscriptions-leaked              disposal leaving the four subscriptions alive
```

`mirror-moves-on-a-refused-write` survived its first form. It reordered two
statements *after* the handle guard, and on the only reachable failing path —
a disposed game — the guard returns before either of them, so the reorder
changed nothing. Hoisting the assignment above the guard, which is the shape
the defect would actually take, is caught.

One control was **deleted rather than fixed**: an earlier
`exiting-raised-by-teardown` mutation matched two sites, and
`teardown-callback-mapped-onto-exiting` tests the same property at the place the
defect actually lived.
