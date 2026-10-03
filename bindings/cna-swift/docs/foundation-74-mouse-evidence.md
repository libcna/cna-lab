# Foundation 74 — `Input.Mouse`, and a getter that had no business failing

```text
COMPLETE_TYPES 181 -> 182   MISSING_TYPES 70 -> 69   TOTAL_DIAGNOSTICS 106 -> 105
BOUND_FUNCTIONS 314 -> 317  ABI_MISMATCHES=0         tests 809 -> 814
PROJECTION_MUTATIONS 287 -> 290
```

Three members, and **no managed half at all**: `GetState` is `GetCursorPos`,
then `ScreenToClient` when a window is hooked, then `GetAsyncKeyState` once per
button; `SetPosition` is `ClientToScreen` then `SetCursorPos`; `WindowHandle` is
a raw `HWND` field. `MouseState` and `ButtonState` already stood, so this is the
last piece of its family rather than the first.

## The gate rewrote the design, and it was right

The first projection made all three members throwing forwards, `WindowHandle`
included — a getter that called `cna_mouse_get_window_handle` and a
`SetWindowHandle` writer method beside it. The api-compat gate refused it in
three separate ways, and the middle one is the interesting one:

```text
PROPERTY_MAPPING_MISMATCH  expected mutable=True, found mutable=False
PROPERTY_MAPPING_MISMATCH  CLR getter is infallible, so the Swift reader must
                           not throw; found throws=True
UNEXPECTED_MEMBER          Mouse.SetWindowHandle(_:Int)
```

**`get_WindowHandle` is `ldsfld hHookedHandle; ret`.** It reads XNA's own static
field and never asks the platform anything, so it cannot fail, so the Swift
reader must not throw — and the pinned fallibility measurement says so
independently of anyone's reading.

So `WindowHandle` is a stored value here, exactly as it is in XNA. It reads
outside a lifecycle callback, where the route-backed version refused; the setter
records, and the recorded handle is pushed at the next member that already
throws. That is the deferred-write architecture the infallible XNA setters
elsewhere already use, and it is the *faithful* shape rather than a workaround.

One route fell out of the design as a result: `cna_mouse_get_window_handle` has
no consuming member now, so **it is not bound**. Foundation 67 set that rule for
the five `..._collection_find` routes and it applies unchanged.

## What the probe measured, including one thing it could not settle

`build-probe/f74_mouse.c`, on HEADLESS:

```text
get_state -> 0   x=0 y=0 wheel=0 hwheel=0 buttons=0x0
set_window_handle(0x1234) -> 0, reads back 4660
set_position(40,50) -> 0, and get_state then reports x=40 y=50
set_position(-5,-5) -> 0
```

**The logical cursor position is observable**, which makes this milestone less
thin than Foundation 72's: `SetPosition` and `GetState` round-trip, negatives
included, and the test asserts the pair rather than the fact that a call
returned.

The one thing the probe could **not** settle is recorded as withdrawn rather
than guessed. Seeded with 123, a successful `cna_mouse_get_window_handle` with
no window bound left the buffer at 123 — it had not written, though its own
header promises *"zero when none is bound"*. A second probe calling that route
first, in both `load_content` and `update`, wrote zero every time. The only
difference is whether `cna_mouse_get_state` ran before it, and this host cannot
say why. The mutation that would have proved the projection's
zero-initialisation load-bearing is withdrawn with that reasoning written where
it stood — and the question then dissolved anyway, because the infallible-getter
finding above removed the buffer entirely.

## `MouseState` has nowhere to put the horizontal wheel

`CNA_MouseState` carries `scroll_wheel` **and** `horizontal_scroll_wheel`. XNA's
`MouseState` has one scroll value. The horizontal one is taken as a parameter
and dropped, rather than omitted, so that reading the wrong one is a visible
mistake in the code — folding it into the vertical would invent a number XNA
never reports.

## Falsifiability

Four mutations, and the two interesting ones survived first: on this host every
button reads `Released` and both wheels read zero, so a transposed button bit
and a wheel read from the wrong field are equally invisible through `GetState`.
Neither was a missing assertion — no input reaches the difference.

Same remedy as Foundation 73's `volumeAspectExtremes` and Foundation 68's
`arrayHolds`: `projectedState` is extracted as a function of its numbers and
tested with masks that distinguish. Each of the five bits is asserted alone,
against every other button being released, so a transposition cannot hide behind
a neighbour — CNA orders the bits left, middle, right while XNA's constructor
takes them left, **middle**, right *after* the scroll wheel.

**And the harness caught the same two a second time.** A careless edit of the
test file while reworking `WindowHandle` deleted that test, and both mutations
came straight back `SURVIVED`. Nothing else in the milestone noticed: 813 tests
still passed. That is the whole argument for running the mutations after every
edit rather than once at the end.

`PROJECTION_MUTATIONS=290`; the full run is repeated before the final handoff.
