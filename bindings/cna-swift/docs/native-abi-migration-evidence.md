# Migrating the native boundary from CNA C ABI 0.7.0 to 0.21.0

The binding's native boundary was pinned to CNA C ABI `0.7.0` from Foundation
Milestone 1 until this migration. Live CNA had moved fourteen minor generations
past it. This document records what the current ABI actually is, what changed
under every route the binding already had, what did not, and the one runtime
behaviour that did.

Historical `0.7.0` evidence is not rewritten: `docs/native-abi.md` keeps the
measurement that was made, and this file is the migration on top of it.

## 1. The live dependency, measured rather than assumed

```text
cnanext            HEAD 0a6158e4ff764907065cd7259e3d29e331a52088  branch next
sharp-runtimenext  HEAD 4a49afb0cfe6a41e6e0af0bb62dc5175976731bb  branch next
```

Neither repository was modified, and nothing was committed in either.

`modules/c-api/include/CNA/C/abi.h` declares

```text
CNA_ABI_VERSION_MAJOR 0   CNA_ABI_VERSION_MINOR 21   CNA_ABI_VERSION_PATCH 0
CNA_ABI_VERSION       CNA_ABI_VERSION_ENCODE(0, 21, 0) == 0x00001500
```

### The admitted artifact

```text
path        ~/deps/cna-c-abi-0.21.0/libcna_c_api.so
sha256      c32bfbd307d695664f906ccf2834ec3f9ebc240fa388d544ac21ee3ebaeb731b
size        166,420,656 bytes
reported    cna_get_abi_version() == 0x00001500 (0.21.0)
platform    Linux x86-64
renderer    HEADLESS
audio       SDL3            (0.7.0's evidence library was NULL)
devices     CNA_DEVICES=OFF
engine      CNA_CNAEXT=OFF
build type  Debug
exports     4,054 `cna_*` symbols
```

Four independent identity facts, each checked rather than asserted:

1. its `include/CNA/C` tree is **byte-identical** to live cnanext HEAD's
   `modules/c-api/include/CNA/C` — `diff -rq` reports no difference;
2. the file is **byte-identical** to `cnanext/cmake-build-headless/modules/c-api/libcna_c_api.so`,
   so its configuration is that tree's `CMakeCache.txt`, quoted above;
3. its exported `cna_*` symbol set is **exactly** the 4,054 names in cnanext's
   own checked-in `tools/c-api/abi_baseline.json` — zero missing, zero extra;
4. the version it reports at run time equals the version its headers declare,
   which the verifier now asserts as its own check.

The artifact was linked from a tree three commits behind HEAD. Those three
commits are `0a6158e4f` (XNB song/video sibling resolution), `89024e0d4` (the
EasyGL renderer's DualTextureEffect UVs) and `0fd4d4e39` (a C API test). None
touches a header, a bound route, or any code the bound routes reach, so the
artifact is current for this binding's surface. It was **not** rebuilt: other
bindings' sessions have recorded that tree's digest, and relinking it would
invalidate their evidence for no gain here.

## 2. The admission policy, derived from CNA rather than chosen

`docs/c-api/ABI_VERSIONING.md` states the consumer contract:

> A consumer must reject a different major and may require a minimum minor.

and `modules/c-api/CMakeLists.txt` enforces exactly that for the installed
package (`COMPATIBILITY SameMajorVersion`, `CBIND-062`). Under `0.x` an
incompatible change is what moves the minor, and `0.20.0`'s removal of eleven
renderer identities is the worked example.

`NativeABI` encodes that rule and nothing more:

```text
admittedMajor      0        exact
minimumMinor      21        the generation this binding is qualified against
patch                       unconstrained
qualifiedVersion  0.21.0
```

A later minor is admitted because CNA's published rule admits it — and because
the protection against a later minor that *removed* a route is not a version
number but `dlsym`: every one of the 29 routes must resolve by name before the
runtime starts, and a missing one throws `CNAError.missingNativeSymbol`.

The diagnostic names all three facts a caller needs:

```text
CNA native library /home/…/libcna_c_api.so reports C ABI 0.7.0 (0x00000700);
CNA-Swift admits major 0 with minor 21 or later (qualified against 0.21.0)
```

`Tests/CNATests/NativeABIPolicyTests.swift` exercises the window itself: the
packing, a different major, every minor from 0 through 20 — 0.7.0 among them —
a later minor, an arbitrary patch, and the diagnostic's three facts.

## 3. Every existing route, audited against the current header

All 29 bound symbols still exist and are still exported. Every prototype is
unchanged in return type, parameter count, order, width, signedness, pointer
depth and constness. The compiler proves it: the whole
`__builtin_types_compatible_p` wall the binding already carried compiles green
against the 0.21.0 headers, and the only assertion that failed on the first
attempt was the literal `CNA_ABI_VERSION == 0x00000700`.

**One manifest row family was wrong, and the audit found it.** Three routes —
`cna_graphics_device_manager_create`, `_apply_changes` and `_destroy` — take
`CNA_GraphicsDeviceManagerHandle`, not `CNA_Handle`. The manifest recorded
`CNA_Handle`. The two spell the same 64-bit type, so `__builtin_types_compatible_p`
could never tell them apart, and the row had been wrong since Foundation 1.
**This is a CNA-Swift defect, not an upstream change**: the 0.7.0 header
`runtime_graphics_manager.h` already used the typed handle. It is corrected, and
the new canonical-declaration check is what makes it impossible to reintroduce.

The 18 mirrored structures are identical between 0.7.0 and 0.21.0, field for
field, and identical to the shim: 129 fields, each with a matching name, offset
and width. Both mirrored callbacks are unchanged. All 52 canonical constants and
all 160 `Keys` literals hold.

## 4. The one behaviour that changed: the `GameTime` a callback receives

Migration turned an existing native test red, which is what it is for. The
divergence is upstream and is reproduced by a 30-line pure-C probe that uses no
Swift at all (`build-probe/abi21_gametime.c`,
`build-probe/abi21_gametime_oneframe.c`), with `is_fixed_time_step = CNA_TRUE`
and `target_elapsed_time_ticks = 166667`:

```text
              Update                 Draw
              total     elapsed      total     elapsed
CNA 0.7.0
  frame 0     166667    166667       166667    166667
  frame 1     333334    166667       333334    166667
  frame 2     500001    166667       500001    166667

CNA 0.21.0
  frame 0          0         0            0    166667
  frame 1          0    166667       166667    166667
  frame 2     166667    166667       333334    166667
```

Pinned XNA is the authority for what those numbers should be, and
`Microsoft.Xna.Framework.Game.dll` `b5dffdd8125abef2…` answers it exactly.
`Game::Tick`'s fixed-step path:

- computes `V_3 = (accumulated + elapsed) / targetElapsedTime` and, at
  `IL_00de`, **returns immediately when that count is zero** — before
  `AdvanceFrameTime`, before `Update`, and before `DrawFrame`;
- otherwise sets `gameTime.ElapsedGameTime = targetElapsedTime` (`IL_016d`,
  `V_4`) — a constant, never a measured or zero value — and
  `gameTime.TotalGameTime = totalGameTime` **before** the `finally` at
  `IL_01da` adds one step;
- `DrawFrame` then re-reads the already advanced `totalGameTime` (`IL_0032`)
  and sets `ElapsedGameTime = lastFrameElapsedGameTime` (`IL_0043`).

So XNA's sequence is `U(0, T) D(T, T) U(T, T) D(2T, T) …`.

**CNA 0.21.0 frames 1 onward reproduce that exactly**, which CNA 0.7.0 did not:
0.7.0's `Update` saw a total one step ahead of XNA's, and its `Draw` saw the
same total as its `Update` rather than the advanced one. The current generation
is closer to XNA on both counts.

**CNA 0.21.0's frame 0 is a divergence.** It issues one leading frame whose
`Update` carries `elapsed = 0`, where XNA's `Tick` would return without calling
`Update` or `DrawFrame` at all. No XNA callback can observe a zero
`ElapsedGameTime` under a fixed time step.

CNA-Swift reports what the host reports. Substituting `TargetElapsedTime` for
the zero would make the projection agree with XNA by fabricating a value CNA
never sent, and would hide the upstream defect; the binding does neither. The
whole measured sequence is pinned instead, as a **native runtime observation and
not XNA behaviour evidence**, in
`NativeLifecycleTests.testHostGameTimeSequenceIsMeasuredNotAssumed`, which
carries the XNA derivation above in its documentation comment.

### Upstream defect, for CNA

> Under `IsFixedTimeStep`, the first frame of a C-created game invokes the
> `update` callback with `elapsed_game_time_ticks == 0`. XNA 4.0's `Game.Tick`
> divides the accumulated time by `targetElapsedTime` and returns without
> invoking `Update` or `DrawFrame` when that quotient is zero, so no XNA
> callback observes a zero elapsed time under a fixed time step. Reproduced on
> `0.21.0` HEADLESS with both `cna_game_run` and `cna_game_run_one_frame`; not
> present on `0.7.0`.

## 5. The verifier, strengthened

| Measurement | 0.7.0 gate | 0.21.0 gate |
|---|---|---|
| `BOUND_FUNCTIONS` | 29 | 29 |
| `ROUTE_PAIRINGS` | — | 29 |
| `PROTOTYPE_TYPE_POSITIONS` | 91 | 91 |
| `CANONICAL_DECLARATION_CHECKS` | — | 91 |
| `C_SWIFT_MEASUREMENTS` | 91 | 91 |
| `LAYOUTS` | 18 | 18 |
| `LAYOUT_FIELDS` | 67 hand-picked offsets | 129 fields, offset **and** width |
| `CALLBACKS` | 2, printed | 2, compiled |
| `CONSTANTS` | 214, a literal | 212, derived from the source |
| `SCALAR_FACTS` | — | 3 |
| `MISSING_HEADER_SYMBOLS` | hard-coded 0 | derived |

Five things are new, and each closes a hole that was open:

1. **Canonical declaration checks.** The manifest's C declaration for a symbol
   is now compared *textually* to the declaration in the CNA headers, parameter
   names included. This is what caught the `CNA_Handle` /
   `CNA_GraphicsDeviceManagerHandle` rows: type-compatibility never could.
2. **Route pairing.** Every Swift stored property is paired with exactly one
   symbol and one route type. Each route type is *derived* from its symbol
   (`cna_game_run` → `GameRunRoute`), so two routes cannot share one, a
   property cannot resolve another route's symbol, and a declared route type
   that no route uses is a mismatch. The old gate resolved symbol → alias and
   never looked at the property, so `gameRun` resolving `cna_game_run_one_frame`
   through the shared `HandleOperation` alias was invisible.
3. **Whole-structure layout.** The 67 hand-picked `OFFSET(...)` lines are gone.
   The verifier parses both headers, requires the field-name sequences to be
   equal, and generates an offset *and* a width assertion for all 129 fields —
   so a field that is omitted, added, transposed or resized fails. Under the old
   wall, `CNA_GameCallbacks` had exactly one field checked out of six.
4. **Callback ABI, actually compiled.** `CALLBACKS=2` was a `printf` literal
   with no assertion behind it; nothing compared the two callback typedefs, and
   a function pointer is eight bytes whatever its signature, so the struct
   layout wall could not see through it. `__builtin_types_compatible_p` cannot
   compare them directly either — the shim declares its own structure types, so
   the pointer types are formally incompatible even when identical. The gate
   proves three linked facts instead: the shim typedef *is* the shape parsed
   from `CNAShim.h`; that shape, with every `CNASwift_` name rewritten to its
   canonical counterpart, *is* the canonical typedef; and every scalar typedef
   named on both sides is the same underlying type. The structures named on both
   sides are proven layout-identical by (3).
5. **No hand-maintained counts.** `LAYOUTS`, `CALLBACKS` and `CONSTANTS` were
   `printf` literals a maintainer had to remember to update. Every count is now
   derived from the source that was just compiled. `CONSTANTS` moves 214 → 212
   for that reason and not because coverage shrank: the old literal counted 54
   where the probe proves 52 *named* constants, its other two facts being the
   `CNA_Bool` width and the ABI version identity, which are not named constants
   and are now reported as `SCALAR_FACTS` and as the ABI fields.

## 6. Falsifiability

`tools/native_abi/mutations.py` plants fourteen realistic defects, one at a
time, runs the unmodified verifier and requires it to fail, then restores the
tree and proves every touched file is byte-identical to how it started.

```text
CAUGHT  wrong-parameter-width        a bound route's parameter width
CAUGHT  wrong-parameter-spelling     a canonical parameter type recorded as a compatible alias
CAUGHT  stale-symbol                 a bound symbol that no longer exists
CAUGHT  swapped-route-symbol         one property resolving another route's symbol
CAUGHT  swapped-route-type           one property resolving through another route's type
CAUGHT  shared-route-type            two routes sharing one structurally identical type
CAUGHT  wrong-swift-position-width   a Swift route position with the wrong width
CAUGHT  wrong-abi-window             an admitted ABI window the header does not satisfy
CAUGHT  omitted-struct-field         a mirrored structure missing one canonical field
CAUGHT  reordered-struct-fields      two mirrored fields transposed
CAUGHT  narrowed-struct-field        a mirrored field of the wrong width
CAUGHT  wrong-callback-signature     a mirrored callback missing one parameter
CAUGHT  wrong-constant               a canonical constant value
CAUGHT  wrong-key-constant           a projected Keys literal

NATIVE_ABI_MUTATIONS=14 CAUGHT=14 SURVIVORS=0
```

`wrong-parameter-spelling`, `swapped-route-symbol`, `shared-route-type`,
`omitted-struct-field`, `reordered-struct-fields` and `wrong-callback-signature`
would all have **survived** the 0.7.0 gate.

The window is also shown to reject rather than merely to accept: run against the
0.7.0 headers the probe fails to compile ("canonical header minor is below the
admitted CNA C ABI minimum"), and run against the 0.21.0 headers with the 0.7.0
library it reports two mismatches — the loaded ABI is outside the window, and it
disagrees with the header's.

## 7. Requalification on CNA 0.21.0

```text
DEBUG_BUILD=PASS                       RELEASE_BUILD=PASS
DEBUG_TESTS=414 PASS                   RELEASE_TESTS=414 PASS
WARNINGS_AS_ERRORS=PASS_DEBUG_AND_RELEASE_INCLUDING_TESTS
NATIVE_ABI=29/29/91/91/91/18/129/2/212/3  MISSING=0/0  MISMATCHES=0
NATIVE_ABI_MUTATIONS=14 CAUGHT=14 SURVIVORS=0
NATIVE_STRESS  GAME_CYCLES=20 GAME_RECREATION_CYCLES=20 TEXTURE2D_CYCLES=20
               SPRITEBATCH_CYCLES=20 CALLBACK_ERROR_CYCLES=20
               GAMEPAD_GET_STATE_CYCLES=50 GAMEPAD_CAPABILITIES_CYCLES=20
               NATIVE_CRASHES=0 OBSERVED_UAF=0 OBSERVED_DOUBLE_FREE=0 MODE_FAILURES=0
GAMEPAD_NATIVE=0 FAILURES  HARDWARE_AVAILABLE=NO
SWIFT_ASAN=PASS_414_TESTS_DETECT_LEAKS_DISABLED
SWIFT_TSAN=3_RUNS_0_RACES_0_FAILURES
STRICT_VERIFIER  unchanged: 142/1767, 269 diagnostics, 135 complete, 7 partial
LEAK_ONLY=PASS   INTERNAL_TYPE_LEAK=0 RAW_HANDLE_LEAK=0 PUBLIC_NATIVE_FFI_LEAK=0
TEMPLATE  Debug 60 PASS  Debug 600 PASS  Release 60 PASS  Release 600 PASS
          updates==draws==requested, viewport 800x480, texture 128x128
```

The renderer is still HEADLESS, so there is still no visible window and no
pixel evidence. The audio backend, however, is no longer `NULL`: this artifact
is built with `CNA_AUDIO_PLATFORM=SDL3`, which retires the audio half of the
0.7.0 platform statement as a *configuration* limit rather than a product one.
