# Requalification against CNA C ABI 0.35.0 (2026-09-30)

The last pass before CNA-Swift is archived into `cna-lab`. The binding was last
qualified against CNA C ABI 0.21.0; this records what was re-measured against
CNA `next` at 0.35.0 and what changed.

## Artifacts

Both built from CNA `next` source and staged with a `PROVENANCE.txt`
(configuration, source revision, SHA-256): `~/deps/cna-c-abi-0.35.0` (HEADLESS,
Debug) and `~/deps/cna-c-abi-0.35.0-opengles3-fx` (OPENGLES3 on EasyGL, Release,
compiled effects, CNAEXT, devices, video). HEADLESS runs use SDL's dummy video
and audio drivers with no display; OPENGLES3 runs only inside CNA's
`tools/platform/run_gpu_tests_private.sh` (headless Weston + rootful Xwayland on
the real GPU), SDL audio on the dummy driver. Swift 6.0.3,
`x86_64-unknown-linux-gnu`.

## Results

The final run is against both artifacts restaged at CNA `next` `5b4edd6cc`,
after BINDFIX-044..050; it repeated every native gate below with the same
counts, except that SW-05's sub-pixel crop now decodes (see Findings).

| Gate | Result |
|---|---|
| `tools/native_abi/verify.py`, both libraries | 778 routes, 2657 prototype positions, 68 layouts / 522 fields, 9 callbacks, 228 constants, 0 missing, 0 mismatches |
| `tools/native_abi/mutations.py` | 14 of 14 planted defects caught |
| `tools/api_compat/verify.py` (+ both self-tests) | 257 of 257 types complete, 0 partial, 0 missing, 0 diagnostics; report unchanged |
| `swift test` (debug), HEADLESS | 996 tests, 0 failures, 0 skips |
| `CNAPackageTests.xctest`, OPENGLES3 | 996 tests, 0 failures, 0 skips |
| `tools/native_stress/run.py` | 0 crashes, 0 use-after-free, 0 double free |
| `tools/gamepad_native/run.py` | 0 failures (no controller attached) |
| `tools/behavior/run.py` | 2331 observations, 0 failures |
| `tools/consumer_canary/verify.py`, 60 and 600 frames | 32 checks, 0 findings |
| template `HelloGame`, HEADLESS 60 / 600 | exit 0; 60/60 and 600/600 updates/draws |
| template `HelloGame`, OPENGLES3 60 / 600 | exit 0; 60 and 600 draws with catch-up updates (87, 931), real 800x480 window |

The archive-consumer result is `docs/generated/package-qualification-report.json`.

## Binding changes

- **Admission**: major 0, minor 35 or later, qualified against 0.35.0.
- **`Game.Run` now calls the registered manager's `CreateDevice()`**, as XNA's
  `RunGame` does before `Initialize`. Without it a constructor-time
  `GraphicsProfile = .HiDef` never reached the device.
- **XNA refusals the binding had not reproduced**: `SetCurrentTechnique(nil)`
  raises `ArgumentNullException`; `SetData` on a bound vertex or index buffer
  raises `InvalidOperationException(ResourceInUse)` unless the lock carries
  `Discard`/`NoOverwrite`.
- The template requests HiDef, which its `OcclusionQuery` needs under XNA's rules.

## Findings re-measured

Native probes are in `cna/build-probe/qual-probes/sw-*.c` (CNA's gitignored probe
directory); every other finding is pinned by a test that passed on 0.35.

| Finding | 0.35 |
|---|---|
| SW-05 decode zoom | zoom=true: fixed by CNA BINDFIX-043, and a crop under one source pixel (2x2 to 8x2 or 2x8, `INVALID_ARGUMENT` at `4228ff913`) by its follow-up `5b814df79`, both pinned. zoom=false still ignores a square source's requested height (8x2 grants 2x2): an XNA-behaviour question no IL settles |
| SW-06 encoder alpha-0 | fixed: an alpha-0 texel is written (0,0,0,0) |
| SW-07 RenderTargetCube through `texturecube_get_info` | fixed: reports the real size and levels |
| SW-10 count of a never-begun query | fixed: refused with `INVALID_STATE` |
| SW-13 `mouse_get_window_handle` | not applicable: the route writes its output (0 on HEADLESS, a real handle on OPENGLES3) |
| SW-15 draw from a never-written buffer | fixed: drawn, as XNA draws it |
| SW-16 draw with no effect | fixed: `INVALID_STATE`, not `INTERNAL` |
| SW-22 duplicate render target | fixed: `INVALID_ARGUMENT`, not `INTERNAL` (the binding refuses first, as XNA does) |
| SW-26 byte transfers into a Color texture | fixed by CNA BINDFIX-042, mid-texel start included |
| SW-40 native `BasicEffect` parameters | fixed: 21 published |
| SW-45 / SW-46 readback, cube data | HEADLESS host limit unchanged; OPENGLES3 reads the back buffer back and stores all six cube faces, asserted through the binding |
| SW-01 leading fixed-step frame | the zero-elapsed Update is XNA's own `RunGame` pre-loop Update; the Draw before the first `Tick` update is the only open question |
| SW-12 title container | still one `IO` code for every failure; CNA's C route now reads an absolute path, which the binding refuses first with XNA's rooted-name `ArgumentException` |
| SW-04, SW-08, SW-09, SW-11 | still as recorded; the binding enforces XNA's rule |
| SW-02, SW-03, SW-14, SW-17 to SW-21, SW-23 to SW-25, SW-27, SW-29 to SW-39, SW-41 to SW-44, SW-47, SW-48, SW-50 | unchanged, pinned by passing tests; SW-44's catch-up updates also appear on OPENGLES3; SW-50's adapter now reports SDL's real display |
| SW-49 | not re-run (no mutation campaign in this pass) |

## Unqualified

macOS, iOS, tvOS, visionOS, Windows and Web/Wasm, as before. No live-desktop
output, no attached controller, no real capture device.
