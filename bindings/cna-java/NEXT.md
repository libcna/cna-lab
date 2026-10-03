# CNA-Java continuation handoff

**Updated:** 2026-09-30 -- final requalification against CNA C ABI 0.35.0 before this repository
was archived into `cna-lab`. The earlier handoff (2026-09-01, ABI 0.21.0, a five-renderer
`cnanext` build) is in Git history.

Read `plan.md`, this file, `docs/backlog.json`, `docs/runtime-capabilities.json`,
`tools/native-abi/probes/README.md` and `docs/cna-c-api-coverage-summary.json` before new work.

## Dependencies

```text
CNA            sibling ../cna, branch next (artifacts staged from 5b4edd6cc; see PROVENANCE.txt)
sharp-runtime  sibling ../sharp-runtime 88f6b11f
ABI            0.35.0 (runtime-reported), 3,202 canonical routes
HEADLESS       ~/deps/cna-c-abi-0.35.0             Debug, SDL3 platform + audio, no CNAEXT, no devices
OPENGLES3      ~/deps/cna-c-abi-0.35.0-opengles3-fx Release, CNAEXT, CNA_DEVICES, compiled effects
```

`CNA_ROOT` defaults to the sibling `../cna`. The OPENGLES3 library opens a window and runs only
inside CNA's private GPU runner, never on a desktop session:

```sh
CNA_NATIVE_LIBRARY=~/deps/cna-c-abi-0.35.0/lib/libcna_c_api.so ./gradlew --max-workers=8 clean check
../cna/tools/platform/run_gpu_tests_private.sh --exec env \
    CNA_NATIVE_LIBRARY=~/deps/cna-c-abi-0.35.0-opengles3-fx/lib/libcna_c_api.so \
    SDL_VIDEODRIVER=x11 SDL_AUDIO_DRIVER=dummy ./gradlew --max-workers=8 check
```

## What changed for ABI 0.35.0

- CNA retired `engine_layer.h` at ABI 0.30 (render pipeline, post-processing, shadows, clustered
  lighting, probes, particles, compute, instancing, material objects): 841 bound routes, their
  Java families, tests and probes were removed. The 19 `graphics_ext.h` routes that survived
  (DebugDraw, image-based light and indirect-draw values) moved to `NativeGraphicsExtensionRoutes`.
  Retired renderer identities left `GraphicsRendererType`; the Guide's two internal setters CNA
  removed at 0.34 went with their `protected` Java shims.
- Newly bound: the camera family (15 routes, `extensions.devices.Camera`), `CnaModel.Load` and
  `getContentTag`, the renderer selection count/selected/latched queries, and the achievement and
  profile picture copy routes (the pictures used to be zero-filled buffers of the right length).
- CNA's own semantics moved toward XNA and the projection followed: the Guide keyboard input and
  message box are genuinely asynchronous (Begin returns with the screen up, End waits, the XNA
  callback runs from the dispatcher pump); a second `GamerServicesDispatcher.Initialize` is XNA's
  `InvalidOperationException`; unconfigured social screens refuse; no signed-in gamer is invented
  (tests sign one in through `CNA_GAMER_SERVICES_AUTO_SIGN_IN`); `NetworkSessionProperties` is
  eight fixed slots; Reach-profile limits (32-bit indices, occlusion queries) are enforced.

## Exact state (2026-09-30)

```text
NATIVE    CANONICAL_FUNCTIONS=3202  BOUND=1952  LIBRARY_SYMBOL_CHECK=PASS (1952/1952)  ABI_POLICY=PASS
          DEFERRED_TRACKED=320  DELIBERATE_NON_BINDING=930  BLOCKED_UPSTREAM=0
          ACTIONABLE_LOCAL=0  BOUND_BUT_UNREACHED=0  BOUND_WITHOUT_JAVA_CALL_SITE=0  RULE_PROBLEMS=0
          EXTENSION_CENSUS=41  NATIVE_TOOL_TESTS=167/167  generateJniCheck CHANGED_FILES=0
API       XNA 4.0 Windows profile 257 types/2964 members  TOTAL_DIAGNOSTICS=0  ALLOWLIST=0
          full runtime superset 331 types/3640 members       TOTAL_DIAGNOSTICS=0  ALLOWLIST=0
TESTS     ./gradlew clean check, 74 suites / 367 tests, 0 failures, 0 skipped, on each of
          HEADLESS and OPENGLES3 (private GPU runner); HEADLESS also clean under -PcheckJni
TEMPLATE  scripts/verify-template.sh (exact published Maven artifact + JNI library, in
          build-consumer/): 60-frame smoke, extensions smoke, 600-frame stability and a
          generated project -- all pass on HEADLESS and on OPENGLES3
```

Not qualified here: Windows, macOS, Android and iOS; a real camera frame (the host camera is
enumerated only); a real network peer; renderers other than HEADLESS and OPENGLES3.

## Upstream findings

All 23 `JAVA-UPSTREAM-*` records are closed; `docs/backlog.json` says how each was re-measured.
Fixed in CNA: -001, -002, -004, -014 (BINDFIX-041), -016, -017 (BINDFIX-038), -019, -022, -023.
Removed with the engine layer: -005..-013, -015. Withdrawn as probe artifacts: -018, -020, -021
(the probes printed an output inside the same `printf` that called the route, and GCC read it
first). -003 is CNA's own suite, green on both artifacts outside the private runner's audio-less
runtime directory.
