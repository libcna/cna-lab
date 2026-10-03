# Foundation 33 — the managed component engine, and where `LoadContent` comes from

Foundation 29 audited `Game.Components` and **stopped**: the collection alone
would have let a consumer write `game.Components.Add(c)` and get no
`Initialize`, no `Update`, no `Draw` and no error — a silent lie. The engine
that makes it real lives in three already-implemented public members whose
Swift bodies were empty, and changing those bodies was outside that
milestone's authorization. It is inside this one.

## What was empty, and what it is now

```text
Game.Initialize()  drain notYetInitialized, initializing each component
Game.Update(t)     copy updateableComponents, then Update each Enabled one
Game.Draw(t)       copy drawableComponents,   then Draw   each Visible one
```

All of it is **managed** XNA behaviour. The native CNA host owns frame
scheduling and nothing else, which is exactly the split Decision 3 states: the
host is the driver, the Swift `Game` base owns the component semantics.

`Game..ctor` now allocates the one `GameComponentCollection` and the one
`LaunchParameters` XNA allocates, and subscribes the two private handlers that
are the engine:

```text
GameComponentAdded    inRun ? component.Initialize() : notYetInitialized.Add(c)
                      IUpdateable -> insert sorted, subscribe UpdateOrderChanged
                      IDrawable   -> insert sorted, subscribe DrawOrderChanged
GameComponentRemoved  !inRun -> notYetInitialized.Remove(c)
                      remove from each sorted list, unsubscribe
```

`self` is captured **weakly** in both. The CLR delegate holds a strong
reference and its cycle is a garbage collector's problem; here the collection
is owned *by* the game, so a strong capture would keep every game alive
forever.

## The comparer is why equal orders keep insertion order

`UpdateOrderComparer.Compare` is not a comparison by `UpdateOrder`:

```text
if (x == null && y == null) return 0
if (x == null) return 1                 // null sorts LAST
if (y == null) return -1
if (x.Equals(y)) return 0               // the SAME object
return x.UpdateOrder < y.UpdateOrder ? -1 : 1
```

For two **distinct** components it never returns 0, even when their orders are
equal. So `BinarySearch` finds a non-negative index only for a component
already in the list — which is what makes `GameComponentAdded`'s `if (i >= 0)
return` a duplicate guard rather than a same-order guard — and the insertion
then scans past every equal-ordered element:

```text
i = ~i
while (i < Count && list[i].UpdateOrder == c.UpdateOrder) i++
list.Insert(i, c)
```

Equal orders therefore keep **insertion order**. A projection that sorted by
`UpdateOrder` with an ordinary comparison would have been unstable, and a test
adds four components with the same order to prove it is not.

## The copy is why mutation during traversal is safe

`Update` and `Draw` both copy their sorted list into a second list, walk the
copy, then clear it. That is transcribed rather than simplified, and two tests
show why it is observable:

- a component that **adds** another during its own `Update` does not see the
  new one run until the next pass;
- a component that **removes** another during its own `Update` does not stop
  it running in that pass.

`Enabled` and `Visible` are read per component at the moment it is reached, so
a component disabled by an earlier one in the same pass is skipped.

## `LoadContent` — the dispatch question, answered by measurement

Foundation 29 named this as the blocker: XNA's `Game.Initialize()` base body
ends with `LoadContent()`, and CNA's host already drives a `load_content`
callback, so transcribing the base body would double-call.

It was resolved by **measuring the host** rather than reasoning about it. A
recording game subclass reports:

```text
RunOneFrame  Initialize, LoadContent, Update, Draw, UnloadContent
Run          Initialize, LoadContent, BeginRun, Update, EndRun, UnloadContent
```

which is XNA's `RunGame` sequence exactly:

```text
graphicsDeviceManager.CreateDevice()
Initialize()          // whose base body calls LoadContent()
inRun = true
BeginRun()
Update(gameTime)      // the pre-loop update
... loop ...
EndRun()
inRun = false
```

So **the native `load_content` callback is the projection of the `LoadContent()`
call inside `Game.Initialize()`** — one XNA lifecycle occurrence, one virtual
invocation. `Game.Initialize()`'s base body therefore does *not* call
`LoadContent`, and the invariant holds. The measurement is a permanent test,
not a note.

`BeginRun` and `EndRun` set and clear `inRun` at the points the host issues
them, which is where XNA sets and clears it — so the `inRun` branch in
`GameComponentAdded` is real, and a test shows a component added after
`BeginRun` being initialized immediately.

### Two differences that are recorded, not half-fixed

Both need `IGraphicsDeviceService`, which is not projected:

- XNA calls `LoadContent` only `if (graphicsDeviceService != null &&
  GraphicsDevice != null)`. Here the host decides, so a game with no
  `GraphicsDeviceManager` still receives it;
- XNA's `LoadContent` fires from **inside** the base `Initialize`, so an
  override that does not call `super.Initialize()` never receives it. Here it
  arrives on its own callback and so always does.

A flag saying "the base body was reached" was implemented and then **removed**.
It fixes the second difference, but XNA calls `LoadContent` from a *second*
place too — the device-created/reset handlers `HookDeviceEvents` installs — and
a flag gating this callback would also suppress a reset-driven reload, which
this host's behaviour has not been measured for. Trading a known divergence for
an unmeasured one is not an improvement. `HookDeviceEvents` itself is absent
for the same reason and nothing is invented in its place.

That removal is worth recording because the flag *did* work: it broke the
existing 60-frame canary, whose probe overrides `Initialize` without calling
`super` — which is precisely the XNA footgun the flag reproduces. The failure
was correct behaviour and the decision to drop it was about the second caller,
not about the test.

## The five private lists are not projections

`Game` holds `updateableComponents`, `currentlyUpdatingComponents`,
`drawableComponents`, `currentlyDrawingComponents` and `notYetInitialized`. The
CLR uses `List<T>` for all five, but nothing public exposes them, so a Swift
array is the same thing observed from outside. `Components` — the one that *is*
public — remains `GameComponentCollection`, that is `CNACollection<any
IGameComponent>`, and drives everything through its own events.

Order-changed subscriptions are kept beside the component they belong to and
matched back with `cnaDefaultEquals`: the CLR removes them with `-=` on
delegate identity, which Swift closures do not have, so the token the event
returns stands in for it. That is the same substitution `CNAEvent` already
documents.

## Scoreboard

```text
TARGET_TYPES=140                     unchanged
TARGET_MEMBERS=1749                  (1747 -> 1749)
TOTAL_DIAGNOSTICS=272                (274 -> 272)
COMPLETE_TYPES=133                   unchanged
MISSING_MEMBER=130                   (132 -> 130)
Game.Components and Game.LaunchParameters resolved
every mismatch/leak category unchanged
UNMEASURED_STRUCTURAL_CATEGORY=0     ALLOWLIST_ENTRIES=0

DEBUG_TESTS=393 PASS                 (377 -> 393)
NATIVE_TESTS=11 PASS                 (10 -> 11)
```

`Game` stays PARTIAL: `Services` needs `System.Type`, `Content` needs
`ContentManager`, `Window` needs `GameWindow`, and the timing and activation
members need host state that is not bound. The engine is complete regardless of
those, because none of them is on its path.

## Behaviour

Fifteen new tests cover the engine and are gated on the native library, because
constructing a `Game` loads it — the behaviour under test is entirely managed.
They are deliberately **not** counted in the pure managed corpus, the same
convention `NativeLifecycleTests` follows, so a skipped run cannot inflate it.
