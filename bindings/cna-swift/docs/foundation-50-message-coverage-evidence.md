# Foundation 50: the messages implemented members raise

## Why this is a gate and not a fourth reading

Foundation 48 found `Clear(Color)` clearing the wrong buffers. Foundation 49
found `set_Viewport` and `set_ScissorRectangle` performing none of XNA's
validation. All three had been implemented, tested and green for many
milestones, and all three were found the same way: by reading the pinned IL
rather than by a failing test.

Three is a pattern. This milestone turns the reading into
`tools/api_compat/message_coverage.py`, which asks one question of the whole
projection at once:

> for every member this binding implements, is every user-visible message its
> pinned IL can raise either reproduced in the Swift sources, or recorded with
> a reason?

It found a fourth defect on its first honest run.

## The fourth defect

`DrawableGameComponent` has exactly two message sites in its pinned IL:

```text
get_GraphicsDevice -> PropertyCannotBeCalledBeforeInitialize
Initialize         -> MissingGraphicsDeviceService
```

The projection raised `MissingGraphicsDeviceService` at **both**. The file's own
comment reasoned carefully about `MissingGraphicsDeviceService` versus
`Game`'s `NoGraphicsDeviceService` and concluded the two "are not
interchangeable" — correctly, and about the wrong pair. There is a third key,
and it is the one the getter uses:

```text
"The GraphicsDevice property cannot be used before Initialize has been called."
```

A user asking a component for its device before `Initialize` was told the game
had no graphics device service, which is a different and wrong diagnosis.

## SpriteBatch's begin/end rule

`SpriteBatch` raises three messages the projection did not:

```text
Begin        if (inBeginEndPair)  -> EndMustBeCalledBeforeBegin
End          if (!inBeginEndPair) -> BeginMustBeCalledBeforeEnd
InternalDraw if (!inBeginEndPair) -> BeginMustBeCalledBeforeDraw
```

CNA already refuses the same sequences — `cna_sprite_batch_begin` answers
`CNA_RESULT_INVALID_STATE` when an interval is open, `_end` and `_submit_many`
when none is — and `testTheNativeRouteRefusesTheSameSequence` pins that it
still does. So the projected `inBeginEndPair` flag is **not** what prevents a
bad call. It is what decides *which channel reports it*: breaking XNA's
begin/end rule is a CLR-shaped mistake and must raise
`InvalidOperationException` with XNA's message, not a `CNAError.nativeFailure`
carrying a CNA result code. That line — what caused the failure, not which
layer noticed it — is the one this binding has drawn since Foundation 30, and
this is the same line applied to a state machine.

Three details from the IL rather than from the shape of the problem. The flag
is set *after* the native begin succeeds and cleared *after* the native end
succeeds, because XNA sets `inBeginEndPair = true` at the end of `Begin` and
clears it after `Flush()`; a failing flush leaves the pair open. All three
messages say "called **successfully**", which is the same fact stated in the
resource strings.

And the rule is checked **before** the handle. None of `Begin`, `End` or
`InternalDraw` calls `Helpers.CheckDisposed` at all — each opens with the flag
test — so on a disposed batch XNA reports the rule that was broken, not the
disposal. `Begin(); Dispose(); Begin()` raises `EndMustBeCalledBeforeBegin`.
The first version of this projection resolved the native handle first and
would have reported the disposal; `testTheRuleIsCheckedBeforeTheHandle` pins
the order and a mutation puts the handle back in front of it.

`CannotNextSpriteBeginImmediate` is not projected. It guards
`SpriteSortMode.Immediate`, and the only `Begin` overload this binding
implements is the parameterless one, which passes `Deferred`. The branch has no
input that reaches it, so it is recorded rather than written as code that
cannot run — the same rule `SamplerStateCollection.SetItem`'s null check
follows.

## The gate

```text
MESSAGE_COVERAGE_TYPES=157 RESOLVED_TYPES=145 UNRESOLVED_TYPES=12
STRING_TABLES_READ=2 MEMBERS_READ=1230 MESSAGES_REPRODUCED=98
MESSAGES_UNREACHABLE=6 MESSAGES_NATIVE_OWNED=7 MESSAGES_DEFERRED=4
MESSAGE_COVERAGE_FINDINGS=0
```

Scope is derived, never hand-written: `docs/generated/api-compat-report.json`
names the COMPLETE and PARTIAL types, and for the partial ones its
`MISSING_MEMBER` diagnostics say which members are absent. Start points are
**contract** members only — XNA's private helpers are not members this binding
could implement — but the walk follows calls from a contract member into the
private methods of its own class, because that is what a caller can actually
reach. `SpriteBatch.Draw` raises nothing itself; `InternalDraw` does.

An absence carries a kind, and the three are counted separately so a deferred
gap cannot hide inside a total:

* `unreachable` (6) — no input reaches the branch through the projected
  surface. A non-Optional `Game`, a non-Optional device, a Swift metatype that
  cannot be nil.
* `native-owned` (7) — the failure is CNA's and is reported on the runtime
  channel, and XNA's message describes something that did not happen. All
  seven are `GraphicsDeviceManager`'s Direct3D adapter-ranking and
  device-creation diagnoses; CNA owns device creation and is not Direct3D.
* `deferred` (4) — an acknowledged gap that names the work.
  `GamePad.InvalidController` and `Keyboard.CouldNotReadKeyboard` are real
  error-channel differences: XNA raises `InvalidOperationException` when XInput
  returns any error other than `ERROR_DEVICE_NOT_CONNECTED`, and this binding
  surfaces CNA's failure on the runtime channel. Settling that needs CNA's
  input failure modes measured against XInput's two-case split, which is its
  own milestone and is written down as one rather than quietly reproduced.

## The gate proved to fail

A coverage gate is the easiest kind to write so that it always passes — read no
members, resolve no types, look in the wrong table. Each of those is planted
and each must be detected:

```text
CAUGHT reproduced-message-deleted        a message the projection reproduces, removed
CAUGHT absence-entry-removed             a recorded absence deleted from the list
CAUGHT absence-kind-invented             an absence carrying a kind the gate does not define
CAUGHT string-table-partially-missing    keys the projection reproduces absent from the tables
CAUGHT string-table-empty                no string table read at all
CAUGHT disassembly-missing               no class body resolves
CAUGHT contract-emptied                  no member qualifies as a start point
CAUGHT call-walk-disabled                keys reachable only through a private callee
CAUGHT harness-restores-its-inputs       the measurement is unchanged afterwards
MESSAGE_COVERAGE_MUTATIONS=9 CAUGHT=9 SURVIVORS=0
```

`string-table-partially-missing` is not hypothetical. **The first draft of this
gate had that bug and reported no gaps at all.** It looked each resource key up
in its own assembly's table, and `Microsoft.Xna.Framework.Graphics.dll` has no
embedded string table: its `Microsoft.Xna.Framework.Graphics.Resources`
resolves against `Microsoft.Xna.Framework.dll`'s 318-string table. One wrong
assumption about where a table lives produced a clean bill of health for an
entire assembly, SpriteBatch's three missing messages included. The gate now
searches every table and fails when it can read none.

Two more floors exist for the same reason: a run that reads no member, or
resolves no type, reports `UNMEASURED` rather than `PASS`. `contract-emptied`
and `disassembly-missing` are the controls that prove it.

`UNRESOLVED_TYPES=12` is reported on every run rather than folded into a
percentage. Those twelve — Touch, Media, Storage, Audio, PackedVector — have no
disassembly in the IL cache, so their messages are unmeasured, and unmeasured
is not clean.

## Result

```text
573 tests, 0 failures
PROJECTION_MUTATIONS=82 CAUGHT=82 SURVIVORS=0
MESSAGE_COVERAGE_FINDINGS=0  MESSAGE_COVERAGE_MUTATIONS=9 CAUGHT=9
MESSAGE_COVERAGE_SELF_TESTS=9
RESOURCE_STRINGS_REPRODUCED=33
TOTAL_DIAGNOSTICS unchanged at 208 — no member is added; four are repaired
```
