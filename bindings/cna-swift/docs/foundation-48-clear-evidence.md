# Foundation 48: `GraphicsDevice.Clear`, and a divergence in shipped code

## What this milestone repairs

`GraphicsDevice.Clear(Color)` has been in this binding since the earliest
graphics work. It called `cna_graphics_device_clear_rgba`, which clears the
**colour buffer only**. Pinned XNA's `Clear(Color)` is twenty bytes of IL and
every one of them forwards:

```
IL_0000:  ldarg.0
IL_0001:  dup
IL_0002:  call   instance ClearOptions GraphicsDevice::get_DefaultClearOptions()
IL_0007:  ldarg.1
IL_0008:  ldc.r4     1
IL_000d:  ldc.i4.0
IL_000e:  call   instance void GraphicsDevice::Clear(ClearOptions, Color, float32, int32)
IL_0013:  ret
```

`get_DefaultClearOptions` (private, 363982 in the disassembled
`Microsoft.Xna.Framework.Graphics` class body) is:

```
ClearOptions o = Target;                       // 1
DepthFormat f = currentRenderTargetCount > 0
    ? currentRenderTargets[0].depthFormat
    : pInternalCachedParams.DepthStencilFormat;
if (f != DepthFormat.None) {
    o = Target | DepthBuffer;                  // 3
    if (f == DepthFormat.Depth24Stencil8)
        o = Target | DepthBuffer | Stencil;    // 7
}
return o;
```

So XNA also clears depth to `1.0f` and stencil to `0` whenever those buffers
exist. The projection did not.

**The divergence was live on the ordinary path, not a theoretical one.** A
device created through `GraphicsDeviceManager` — the only way a projected game
gets one — reports `DepthFormat.Depth24`:

```text
build-probe/f48c_manager.c, qualified HEADLESS artifact
  manager create   -> 0
  create_device    -> 0
  bare game device -> back=800x480 depth_stencil_format=2
  manager device   -> back=800x480 depth_stencil_format=2
  same handle: yes
```

`DefaultClearOptions` on that device is `Target|DepthBuffer`, so every
`Clear(Color)` a CNA-Swift game made was silently skipping the depth clear that
XNA performs — the ordinary once-a-frame call at the top of `Draw`.
`Foundation48ClearTests.testBackbufferDefaultClearOptionsComeFromThePresentationParameters`
now pins that number.

A game that never creates a manager gets a device with no depth buffer at all
(`build-probe/f48b_params.c` measures `depth_stencil_format=0`), which is why
the earlier probing missed it: the first probe written for this milestone did
not create a manager, reported `0`, and made the old behaviour look correct.
The two probes are kept side by side for exactly that reason.

## The three overloads

| overload | IL | what it does |
| --- | --- | --- |
| `Clear(Color)` | 20 bytes | `Clear(DefaultClearOptions, color, 1.0f, 0)` |
| `Clear(ClearOptions, Vector4, Single, Int32)` | 20 bytes | `new Color(vector4)`, then the Color overload |
| `Clear(ClearOptions, Color, Single, Int32)` | 543 bytes | the real one |

The two forwarding overloads were absent; they were two of the sixteen
`OVERLOAD_MAPPING_MISMATCH` diagnostics, now fourteen.

## `CannotClearNullDepth` is a diagnosis, not a validation

The 543-byte overload does **not** check the mask before clearing. It clears,
and only if the clear failed does it decide which exception the failure
deserves:

```
IL_01ba:  ldloc.s    V_7            // the HRESULT
IL_01bd:  bge.s      IL_01e4        // >= 0: success, carry on
IL_01bf:  ldarg.1
IL_01c0:  ldc.i4.6
IL_01c1:  and                       // requested = options & (DepthBuffer|Stencil)
IL_01c4:  call       get_DefaultClearOptions()
IL_01cc:  and
IL_01cf:  beq.s      IL_01dc
IL_01d1:  call       FrameworkResources::get_CannotClearNullDepth()
IL_01d6:  newobj     InvalidOperationException::.ctor(string)
IL_01db:  throw
IL_01dc:  ldloc.s    V_7
IL_01de:  call       GraphicsHelpers::GetExceptionFromResult(uint32)
IL_01e3:  throw
```

That ordering is observable and is reproduced: a projection that validated the
mask up front would reject clears XNA performs. The message is pinned from the
embedded string table of the registered `Microsoft.Xna.Framework.dll` —

```text
CannotClearNullDepth = "Cannot clear depth or stencil because the device does
                        not have an active depth or stencil buffer."
```

— through `tools/api_compat/registered-assemblies.json`, and the verifier
requires the Swift sources to reproduce it verbatim (`XNA_RESOURCE_STRING_PROJECTIONS=22`).

Both arms of the decision are exercised at runtime. CNA rejects an undeclared
option bit, which is the failure the diagnosis is applied to:

```text
build-probe/f48_clear.c, qualified HEADLESS artifact
  clear TARGET                 -> 0
  clear TARGET|DEPTH           -> 0
  clear TARGET|DEPTH|STENCIL   -> 0
  clear (no bits)              -> 0
  clear (undeclared bit 0x10)  -> 1   (CNA_RESULT_INVALID_ARGUMENT)
```

* `Stencil | 0x10` on the backbuffer, which has depth but no stencil, fails and
  **is** diagnosed as `CannotClearNullDepth`.
* the same call with a `Depth24Stencil8` render target bound fails and is
  **not**: the device has that buffer, so the native failure is surfaced as
  itself.
* `0x10` alone fails with no depth or stencil bit requested and is never
  diagnosed that way.

## What did not survive projection

The 543-byte body also saves and restores the D3D9 scissor render state around
the clear, installs a temporary full-target viewport when the current viewport
does not cover the whole surface, calls `SetContentLost(false)` on each bound
render target, and clears the matching bits of the private `lazyClearFlags`.

The first two are below CNA's abstraction: `cna_graphics_device_clear_options`
is one call, not a device-state sequence, and neither is observable through any
projected member. `lazyClearFlags` is private with no projected reader.
`SetContentLost(false)` is CNA's to decide, because CNA owns the content-lost
state that `RenderTarget2D.IsContentLost` reports; the projection does not
overwrite it from the outside.

## The mask conversion, and why it is compiled rather than tested

`NativeStateCodes.clearOptions` maps the three declared options one bit at a
time, as every enum crossing this boundary is mapped. Bits XNA does not declare
are carried across unchanged: XNA hands D3D9 the raw `ClearOptions` word, and
CNA refuses an unknown bit exactly as the driver would, so dropping them would
turn a clear XNA fails into one that silently succeeds.

No runtime observation can see which buffers a HEADLESS clear touched, so the
map has no behavioural witness. `tools/native_abi/probe.c` therefore compiles
the three canonical values, which takes `CONSTANTS` from 212 to 215:

```c
_Static_assert(CNA_CLEAR_OPTION_TARGET == 1, "ClearOptions.Target");
_Static_assert(CNA_CLEAR_OPTION_DEPTH_BUFFER == 2, "ClearOptions.DepthBuffer");
_Static_assert(CNA_CLEAR_OPTION_STENCIL == 4, "ClearOptions.Stencil");
```

## Routes: two added, one removed

`cna_graphics_device_clear_options` and
`cna_graphics_device_get_presentation_parameters` were bound, with
`CNA_PresentationParameters` mirrored field for field — the twenty-sixth
mirrored structure.

`cna_graphics_device_clear_rgba` was **un**bound. Once `Clear(Color)` forwards
through the real overload, no projected member consumes it, and a route no
member consumes is what this boundary exists to prevent. The count is therefore
74, not 75:

```text
BOUND_FUNCTIONS=74 ROUTE_PAIRINGS=74 PROTOTYPE_TYPE_POSITIONS=231
CANONICAL_DECLARATION_CHECKS=231 C_SWIFT_MEASUREMENTS=231
LAYOUTS=26 LAYOUT_FIELDS=222 CALLBACKS=4 CONSTANTS=215 SCALAR_FACTS=3
MISSING_HEADER_SYMBOLS=0 MISSING_LIBRARY_SYMBOLS=0 ABI_MISMATCHES=0
```

## `GraphicsDevice.PresentationParameters` stays absent, deliberately

The route is bound, its values are correct, and the type is already
`COMPLETE` — but the property is not projected, and this is the reason.

XNA's getter is `IL_NO_FAILURE_PATH`: it reads `pInternalCachedParams`, a field
written when the device was created or reset. The pinned verdict therefore
requires a non-throwing Swift reader. This binding has exactly one source for
those values — a fallible CNA route — and no device-creation moment it observes
at which to cache them infallibly. `GraphicsDeviceManager.GraphicsDevice` hands
out the facade without throwing, and `enterCallback` would have to pay for a
device lookup and a parameter fetch on every callback to keep a cache warm.

Projecting it as `get throws` would contradict the pinned fallibility verdict;
backing it with an empty `PresentationParameters()` would publish a value XNA
never had. It stays a `MISSING_MEMBER` — an absence, which this project treats
as an honest diagnostic — until a device-creation moment is measured that can
carry the cache. `IGraphicsDeviceManager.CreateDevice()` and the native
`DeviceCreated`/`DeviceReset` events are the two candidates.

## The render target `DefaultClearOptions` reads

`RuntimeState.currentRenderTarget` records what `SetRenderTarget` bound, which
is XNA's `currentRenderTargets[0]` / `currentRenderTargetCount` pair reduced to
the one slot this binding can honestly track — only
`cna_graphics_device_set_render_target2d` is bound, so there is one target, not
an array of four.

It lives on `RuntimeState` rather than the facade for the reason Foundation 45
established: CNA's device handle is a per-callback capability token, not the
device's identity. It is `weak` where XNA's array is strong, because a strong
reference would close a runtime → target → facade → runtime cycle that nothing
would break. The difference between the two is confined to a target the caller
drops while it is still bound, and CNA will not let that state be left cleanly
anyway:

```text
build-probe/f48b_params.c
  destroy while bound -> 3   (CNA_RESULT_INVALID_STATE)
  unset rt            -> 0
  destroy unbound     -> 0
```

All four branches of the rule are reachable and are tested, because the
qualified artifact grants whatever depth format is asked for:

| requested | granted | `DefaultClearOptions` |
| --- | --- | --- |
| `None` | `None` | `Target` |
| `Depth16` | `Depth16` | `Target｜DepthBuffer` |
| `Depth24` | `Depth24` | `Target｜DepthBuffer` |
| `Depth24Stencil8` | `Depth24Stencil8` | `Target｜DepthBuffer｜Stencil` |
| *(unset)* | backbuffer `Depth24` | `Target｜DepthBuffer` |

## Falsifiability

Seven mutations were added to `tools/projection_mutations/run.py`, taking it
from 60 to 67. Each was proven to be caught:

```text
CAUGHT default-clear-options-ignores-the-render-target
CAUGHT default-clear-options-drops-stencil
CAUGHT set-render-target-forgets-to-record
CAUGHT null-depth-diagnosis-fires-on-every-failure
CAUGHT null-depth-diagnosis-never-fires
CAUGHT clear-options-drops-undeclared-bits
CAUGHT clear-options-swaps-depth-and-stencil
```

**One realistic defect has no control, and it is named rather than covered.**
Mutating `Clear(Color)`'s forwarded mask — `defaultClearOptions` to `.Target`,
which is precisely the bug this milestone repairs — cannot be caught by any
test that runs here. CNA accepts every declared mask on the HEADLESS device
(measured above), so a colour-only clear and a colour-and-depth clear are
indistinguishable from outside. Planting that mutation would have produced a
`SURVIVED` line, and planting it in a form that some unrelated test happens to
catch would be worse than not planting it.

What is covered instead is the value being forwarded: `defaultClearOptions` is
tested directly on all four branches, and the forwarding itself is a single
line with no arithmetic in it. This follows the same rule as Foundation 45's
withdrawn `rasterizer-given-a-dirty-flag`: a gate that cannot fail is not
evidence, and saying so is worth more than a green line that means nothing.

## The gate that corrupted itself, and the lock that came out of it

The first run of the projection-mutation gate for this milestone reported
`67 CAUGHT, 0 SURVIVORS` and was **thrown away**. `tools/native_abi/mutations.py`
had been started while it was still running, and that harness mutates
`NativeManifest.swift`, `NativeFunctions.swift`, `CNAShim.h` and
`Keyboard.swift` — four files under `Sources/`. A `swift test` in the
projection gate can compile the other gate's planted defect and fail for a
reason that has nothing to do with the mutation under test, which is recorded
as a `CAUGHT` that mutation did not earn. That is a false pass, and a false
pass is the direction that hides a survivor.

Both harnesses now take an exclusive `flock` on `.mutation-gate.lock` before
they touch anything, and refuse to start when the other holds it:

```text
MUTATION_GATE=BUSY — another mutation harness holds .mutation-gate.lock;
these two gates cannot share a working tree
```

Demonstrated the way every gate here has to be: a third process took the lock,
**both** harnesses were started and both refused with exit 1, and the
projection harness reached its baseline again once the lock was released. The
lock is advisory and holds between these two scripts only; it is not a claim
that the tree is otherwise untouched.

The result recorded below is from a run with no other harness on the machine,
against the harness as committed.

## One ThreadSanitizer run is recorded as unresolved

The first ThreadSanitizer run of this sweep reported `555 tests, with 1
failure`. The failing test's identity was not captured, and it has not
reproduced: eleven subsequent full TSan runs were clean, three of them under a
saturating CPU load meant to reproduce the cold, contended conditions of the
first, and eight targeted runs of `NativeLifecycleTests` — the suite whose
fixed-time-step assertions are the most plausible candidate — were clean too.

It is written down as an unreproduced failure rather than as a pass. No
ThreadSanitizer *report* was emitted in any run; the count came from a test
assertion, not from a detected race.

## Result

```text
555 tests, 0 failures (debug, release, ASan; TSan 11 of 12 runs — see above)
PROJECTION_MUTATIONS=67 CAUGHT=67 SURVIVORS=0
NATIVE_ABI_MUTATIONS=14 CAUGHT=14 SURVIVORS=0
API_COMPAT_SELF_TESTS=2420  SYMBOL_GRAPH_SELF_TESTS=17  AUDIT_SELF_TESTS=80
BCL_MUTATION_SELF_TESTS=462  BCL_NEGATIVE_CONTROLS=4
RESOURCE_STRINGS_REPRODUCED=22
TOTAL_DIAGNOSTICS=208 COMPLETE_TYPES=150
MISSING_TYPE=100 MISSING_MEMBER=94 OVERLOAD_MAPPING_MISMATCH=14
every disagreement category 0
template: requested=600 updates=600 draws=600 viewport=800x480
```

## What this milestone found next

Reading the pinned IL for the members already implemented, ranked by code size,
turned up the same class of defect twice more:

* `set_Viewport` (387 bytes) raises `ArgumentException(ViewportInvalid,
  "value")` from five separate branches — negative origin, non-positive extent,
  an extent past the render target's bounds, a depth outside 0…1, and
  `MaxDepth < MinDepth`. Foundation 47's `SetViewport` packs six fields and
  pushes, with none of it.
* `set_ScissorRectangle` (300 bytes) raises `ArgumentException(ScissorInvalid,
  …)` from three, on the same render-target-or-backbuffer bounds.

Both need exactly the infrastructure this milestone built — the bound render
target on `RuntimeState`, and the presentation parameters — plus the target's
width and height, which `RenderTarget2D` already projects. They are the next
milestone, ahead of new surface, because a correctness repair outranks an
addition. The research is complete and the two messages are read out of the
registered assembly.
