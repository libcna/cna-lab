# Native ABI boundary

CNA-Go binds only CNA's canonical C ABI. It never links or reflects CNA's C++
ABI and does not route through another language binding.

## Admission and loading

CNA-Go admits **CNA C ABI major 0 with minor 35 or newer**, qualified at
0.35.0 (encoded `0x00002300`). A different major, a lower minor, a missing
required symbol, a resolved pointer that belongs to a different symbol, or a
loader failure rejects the library before Game creation. A rejection names the
library path, the version it reported, and the admitted range.

The floor is the qualified minor. CNA's 0.x ABI takes a minor increment for
incompatible changes (`docs/c-api/ABI_VERSIONING.md` in CNA), so a lower minor is
a runtime whose behaviour this binding was not qualified against: between 0.21
and 0.35 CNA started enforcing XNA's Reach profile, which the native evidence
now asserts. The upper end stays open because a later minor that still
declares every bound route with the same prototype is caught by the checks
below if it does not; nothing is taken on trust: after the version check every
required symbol is resolved by name, and every resolved address is confirmed
with `dladdr` to belong to the symbol the manifest lists.

Foundation 1 admitted exactly `0x00000700` (0.7.0) and Foundation 44 moved to
"0.21 or newer"; both are history, recorded in
[Foundation 44](foundation-44-abi-migration-evidence.md).

On qualified Linux builds, `internal/interop/bridge.c` uses `dlopen` with
`RTLD_NOW|RTLD_LOCAL` and resolves one reviewed manifest. An explicit runtime
artifact is selected with:

```text
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so
```

The value must be an absolute regular-file path. With no override the platform
loader searches for `libcna_c_api.so`; CNA-Go has no repository-relative or
developer-machine fallback. A native library is not embedded in the Go module.

Foundation 1 requires Linux, cgo, and a C compiler. Pure-Go/no-cgo use and cgo
cross-compilation are not claimed. Apple `dlopen` and Windows
`LoadLibrary/GetProcAddress` implementations remain platform work.

## Qualified artifacts

Requalified on 2026-09-30. The final evidence was taken at CNA `5b4edd6cc`;
the same runs at `4228ff913`, `41c6bedef` and `0f7166cd8` earlier that day produced
identical counters. Both artifacts were built from the CNA `next` checkout and staged by the retirement pass under `~/deps`, each with a
`PROVENANCE.txt`:

```text
CNA HEAD                  5b4edd6cc25d656e8eaf0aee304e75ae7ee5a90d (next)
sharp-runtime HEAD        88f6b11fbb8b9d1db1b9451e86f8835e1c9cafaa
compiler                  GCC 14.2.0
reported ABI              0.35.0 (8960)
canonical declarations    3202, exported cna_* routes 3202 (exact correspondence)
header tree sha256        a9a4648c7268e5f054d8930ce7f11d8c744ed9104c022a22953fa5a63382110e (60 .h files)

HEADLESS   ~/deps/cna-c-abi-0.35.0/lib/libcna_c_api.so
           Debug, CNA_PLATFORM=SDL3, CNA_AUDIO_PLATFORM=SDL3, NET on, VIDEO off,
           CNAEXT off, DEVICES off
           sha256 3a6f0edc718a2368d76cd6acd597ac468e33b5f2d23bcd3404fa93f1c1809030
OPENGLES3  ~/deps/cna-c-abi-0.35.0-opengles3-fx/lib/libcna_c_api.so
           Release, SDL3 platform and audio, EASYGL_COMPILED_EFFECTS, CNAEXT,
           DEVICES, NET and VIDEO on
           sha256 7b35fd1312660ae2d2bcb8a47670602ead94f885f8b5316254d219f585fe2556
```

`tools/native_abi` measures both identically: 696 bound routes, 457 manifest
layout agreements, 102 deliberately unbound routes, 0 mismatches. Every bound
prototype and layout is unchanged from 0.21.0; the 855 routes CNA removed
between them were never bound. The committed report is the HEADLESS one:

```sh
go run ./tools/native_abi \
  -headers ~/deps/cna-c-abi-0.35.0/include \
  -library ~/deps/cna-c-abi-0.35.0/lib/libcna_c_api.so
```

The report records `canonical_header_sha256` and `native_library_sha256`
because a path is not an identity (Milestone 55 measured a live header tree
drifting from a pinned library).

`tools/native_stress` runs the full scenario set against each artifact
(`docs/generated/native-stress-report.json` and
`native-stress-report-opengles3.json`). HEADLESS refuses render-target and
back-buffer readback, cube data and `Texture3D`; OPENGLES3 does all four and
carries the pixel evidence. Neither artifact is sanitizer-instrumented, so
`NATIVE_SANITIZER_STATUS=NOT_RUN`. The SOFTWARE artifact Foundation 58 added was
not requalified at 0.35.0 and its report was removed.

## Upstream findings re-measured at 0.35.0

Re-measured with the stress run and three C probes kept in CNA's shared probe
directory (`build-probe/qual-probes/go-f57-f69.c`, `go-open-findings.c`,
`go-dispatcher.c`):

| finding | at 0.35.0 |
|---|---|
| F69 `cna_sprite_font_measure_utf8` adds the last right bearing unclamped | **fixed upstream** (CNA fb62662c9): "B" 6, "AB" 9, as XNA; a missing glyph is still CNA result 1 |
| F79 BasicEffect publishes no EffectParameters | **fixed upstream**: 21 parameters; CNA-Go still drives the typed stock-effect routes, now pixel-verified |
| F34 two subscriptions to one game event fire in reverse | **fixed upstream**: registration order; the one-subscription-per-event design is kept |
| F57 `cna_graphics_resource_dispose` is only a flag | **changed**: a disposed texture now refuses use (result 3); SpriteBatch handles (result 2) and embedded NUL names (result 11) are still refused |
| F85 SOFTWARE ignores vertex colour and lighting | the fixture could not tell; repaired, OPENGLES3 honours both. SOFTWARE not requalified |
| F99 no CNA dispatcher for GamerServicesComponent | **wrong**: CNA's dispatcher works; the type is `ACTIONABLE_LOCAL` |
| F88 sample size/duration arithmetic | **still diverges** from XNA's binary32: 1 s at 22050 Hz mono is 44100 bytes (XNA 44098); the managed body stays |
| F72-4 a Single written to a Vector3 EffectParameter | **still accepted**: result 0 and the value reads back (0,0,0), where XNA throws InvalidCastException |
| F47-1 `cna_game_run_one_frame` initializes the game | still present (`GAME_FRAME_STEP_INITIALIZATIONS`) |
| F84-1 undefined SetDataOptions refused by CNA | not re-measured: the projection's bit test never hands CNA an undefined value, and the 160 option uploads per artifact all succeed |
| F81-1 TextureCube destroy refused while an effect retains it | still refused (C ABI retention rule) |
| F83-1 OcclusionQuery.IsComplete inside a pair | HEADLESS still answers a stale TRUE; OPENGLES3 answers pending/fresh |
| F69-2 `cna_sprite_font_set_spacing(NaN)` | still refused (documented as "must be finite") |
| F51-1 non-finite clear depth | still refused |

New at 0.35.0, all XNA-faithful and asserted by `native_stress`: Reach refuses
separate alpha blending, 32-bit indices, occlusion queries, `GetBackBufferData`
and `Texture3D`, and clearing a depth buffer the device does not have is
`CannotClearNullDepth`. One ordering detail is recorded rather than asserted:
CNA's C shim checks a back-buffer window's capacity before the device's profile,
so an undersized array under Reach is result 14 where XNA's IL throws the
profile's NotSupportedException first.

### The retired Foundation 1 artifact

Foundation 1 through 43 were qualified against a separate 16,799,760-byte
`libcna_c_api.so` with SHA-256
`e912cd1d239d2c76d67677af4df643703e4348f6a7d6b8983904d95c937b116f`, built from
CNA revision `a09196a6477f69a7a57c8364f990658d31531a5b` with sharp-runtime
`54578590b328aa9612fe38bfddca9fd8ca795144`, GCC 14.2.0, Release, HEADLESS
rendering and **NULL** audio. That artifact declared ABI 0.7.0 and is retained
for history only. No current gate loads it.

## Typed manifest

`internal/interop/abi_manifest.h` records each bound symbol as a typed C
function pointer. `bridge.c` resolves those entries once and exposes narrow,
typed bridge functions to cgo. Go never casts or invokes a raw function pointer.
The current closure is:

- ABI/error: version and last-error copy routes;
- Game: create, frame hooks, run, request-exit, destroy;
- device manager/device: create, borrow device, viewport, clear;
- texture: decode from encoded memory, info, destroy;
- SpriteBatch: create, begin, scaled submission, end, destroy;
- keyboard: state snapshot.

For every entry the manifest fixes return type, parameter order, pointer depth,
fixed-width representation, and callback signature. Image bytes and command
arrays are caller-owned for the duration of the call and are synchronously
copied/consumed. Native strings are length-delimited `CNA_StringView` values;
last-error text is copied into Go-owned memory.

## Independent verification

`go run ./tools/native_abi -headers /absolute/path/to/cna/modules/c-api/include -library /absolute/path/to/libcna_c_api.so`
performs five independent checks:

1. GCC compiles pointer assignments from every private manifest typedef to the
   declaration of the **same name** in canonical CNA headers, with
   incompatible-pointer warnings as errors. Each route is paired with its own
   canonical declaration, not with a compatible neighbour.
2. A canonical-header probe measures sizes, alignments, offsets, field widths,
   callback types, ABI/result constants and the admission policy itself. It is
   the only translation unit that sees the canonical header, CNA-Go's private
   manifest and `bridge.h` at once, so it is where all three mirrors are
   compared rather than trusted.
3. A **manifest-only** probe measures the same list with no canonical header in
   scope at all — the exact environment cgo gives `bridge.c`. The two
   measurement sets are compared key by key. Without this, CNA-Go's own struct
   declarations were never measured against anything: the canonical probe
   measures canonical types, because `abi_manifest.h` suppresses its private
   definitions whenever a CNA header is present. The manifest probe refuses to
   compile if a canonical header reaches it.
4. The production loader admits the selected library under the range policy,
   proves every manifest export is present, and confirms with `dladdr` that
   every resolved pointer belongs to the symbol the manifest names. That last
   check is what separates routes which share a prototype: `cna_game_run`,
   `cna_game_request_exit` and `cna_game_destroy` are all
   `CNA_Result(CNA_Handle)`, so a mis-pairing among them would compile cleanly.
5. The canonical declaration count and the library's exported `cna_*` count are
   compared, and the header-declared ABI is compared with the ABI the loaded
   library reports.

The route table is no longer maintained beside the manifest: `tools/native_abi`
parses `CNA_GO_REQUIRED_SYMBOLS` and each route's own `_fn` typedef out of
`abi_manifest.h`, which is the file the cgo build compiles. A required symbol
with no prototype of its own is an error rather than a route counted as taking
zero arguments.

The generated result is `docs/generated/native-abi-report.json`. The canonical
headers verify the bridge; CNA-Go's private declarations do not verify
themselves.

## Go/cgo safety and callbacks

The goroutine entering `Game.Run` calls `runtime.LockOSThread` before loading,
creation, callbacks, resource cleanup, and destruction. It unlocks only after
native callback registration is dead and the Game is destroyed. Native
thread-affine operations compare the current native thread ID with this owner.
On a wrong-thread destroy, the resource handle is preserved so the owner thread
can retry.

Persistent callback state uses `runtime/cgo.Handle` converted to an integer
`user_data` value. C never retains or dereferences a pointer into Go-managed
memory. The handle remains live from callback-table installation through Game
destruction and is deleted afterward. Transient Go byte slices passed to C are
valid only for a synchronous call; no Go pointer is retained by CNA.

Every exported callback trampoline recovers panics. Callback errors and recovered
panics are stored in generation-owned Go state, reported to C as
`CNA_RESULT_CALLBACK`, and returned from the outer `Game.Run`; neither a panic
nor a Go error crosses C.

## Ownership and generation

The private ownership categories are `MANAGED_VALUE`, `OWNED`, `BORROWED`,
`PARENT_OWNED`, and `PROCESS_GLOBAL`.

| Facade/state | Category | Authority |
|---|---|---|
| Game | OWNED | created and destroyed by one `Run` generation |
| GraphicsDeviceManager | OWNED Game child | explicit `Dispose`; children first |
| GraphicsDevice | BORROWED | reacquired during each lifecycle callback |
| Texture2D | OWNED Game child | explicit idempotent `Dispose` |
| SpriteBatch | OWNED Game child | explicit idempotent `Dispose` |
| callback context | Go-owned registration state | `cgo.Handle`, deleted after destroy |

Each run receives a monotonically increasing generation. Operations reject a
stale generation deterministically. Native children are destroyed in reverse
registration order before the Game. Failed destruction does not clear a
handle. Go finalizers are not used; explicit disposal and Game teardown are the
only correctness mechanisms.
