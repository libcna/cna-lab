# CNA C ABI audit

Audit date: 2026-09-30, at CNA C ABI **0.35.0** (CNA `next` 5b4edd6cc, sharp-runtime 88f6b11f).
CNA and sharp-runtime are read-only evidence here; neither was modified.

```text
CNA_SOURCE=/rv/data/development/github.com/libcna/cna
HEADLESS_ARTIFACT=~/deps/cna-c-abi-0.35.0            (HEADLESS renderer, SDL3 platform and audio)
OPENGLES3_ARTIFACT=~/deps/cna-c-abi-0.35.0-opengles3-fx  (compiled effects, CNAEXT, DEVICES, video)
WASM_ARTIFACT=~/deps/cna-c-abi-0.35.0-wasm-webgl2    (WEBGL2 Release, compiled effects, CNAEXT, DEVICES)
```

Each artifact carries a `PROVENANCE.txt` naming the CNA revision it was built from and the ABI it
reports at runtime.

## The 0.21 to 0.35 crossing

CNA removed `engine_layer.h` at 0.30 -- the render pipeline, post-process chain and passes,
clustered lighting, shadow maps, probes, atmosphere, particles, prepass and decals, instancing,
compute, GPU timers and the PBR material value -- and with it 855 of the routes 0.21 exported. The
binding was measured before anything was admitted: every route it imported that left the ABI
belonged to a removed family (no survivor was mixed into one), the rest compiled against the 0.35
prototypes unchanged, and the contract probe below proved the enumerations and scalars again.
Then `src/internal/abi.ts` moved to 0.35 and the removed families' bridge functions, backends,
public classes, tests and capability rows went with them. Two routes were added:
`cna_title_location_set_path_ext` (the loader's `TitleLocation`) and
`cna_avatar_description_create_random_for_body_type` (CNA's `CreateRandom` now keeps the body
type).

```bash
CNA_SOURCE_PATH=/path/to/cna \
CNA_WASM_ARTIFACT_DIR=/path/to/wasm-artifact \
npm run audit:cna-abi
```

## Earlier crossing: 0.7 to 0.20 (record)

The 0.7 to 0.20 crossing itself, kept as the record of that event. The import count below is what
it was at the end of that migration; the *current* one is in "The contract counts this audit holds"
further down, and grew because later sessions bound more of the ABI, not because anything here
changed.

```text
PREVIOUS_TARGET_ABI=0.7.0
LIVE_TARGET_ABI=0.20.0
PREVIOUS_PUBLIC_HEADERS=59
LIVE_PUBLIC_HEADERS=61
PREVIOUS_CANONICAL_DECLARATIONS=2861
LIVE_CANONICAL_DECLARATIONS=4051
IMPORTED_SYMBOLS_BEFORE=360
IMPORTED_SYMBOLS_AFTER=361
RETAINED_IMPORTS=360
REMOVED_IMPORTS=0
RENAMED_IMPORTS=0
ADDED_IMPORTS=1
NODE_BRIDGE_SIGNATURES_VERIFIED=361
NODE_BRIDGE_SIGNATURE_MISMATCHES=0
MISSING_NODE_BRIDGE_SYMBOLS=0
MISSING_QUALIFIED_LIBRARY_IMPORTS=0
UNEXPLAINED_IMPORTED_SYMBOL_GAPS=0
```

Every one of the 360 routes the binding imported against ABI 0.7 still exists in ABI 0.20 with an
identical prototype: the whole import list recompiles against the live `CNA/C/*.h` under
`-std=c11 -Wall -Wextra -Werror`, with each imported symbol assigned to its declared
function-pointer type, so a changed return type, parameter count, signedness, pointer depth or
callback shape would be a compile error rather than a silent mismatch. The single added import is
`cna_vertex_buffer_set_data_raw_at_with_options` (see below).

## What actually changed for this binding

### 1. The version acceptance policy is derived, not literal

The adapter used to require the encoded version to equal `CNA_ABI_VERSION` exactly and said so in a
message that named `0.7.0`. It now applies the policy `docs/c-api/ABI_VERSIONING.md` states -- reject
a different major, require a minimum minor -- with the experimental-`0.x` refinement that an
incompatible change is a minor increment, so under `0.x` the minor must match exactly and the patch
component is free. The window is taken from `CNA_ABI_VERSION_MAJOR`/`CNA_ABI_VERSION_MINOR` in the
headers the adapter compiles against, and the same window is declared once in TypeScript in
`src/internal/abi.ts`. `npm run audit:cna-abi` fails when those two disagree
(`TARGETED_ABI_MATCHES_HEADERS`), so the declared generation cannot drift away from the headers.

### 2. A documented ABI-0.7 limitation is gone

ABI 0.16.0 added the options-carrying raw vertex upload in its windowed form. CNA-TS refused
`VertexBuffer.SetData` with both an `offsetInBytes` and `Discard`/`NoOverwrite`, which was accurate
against 0.7 and is no longer accurate. The adapter now imports
`cna_vertex_buffer_set_data_raw_at_with_options` and routes non-`None` options through it;
`CNA_SET_DATA_NONE` keeps taking `cna_vertex_buffer_set_data_raw_at`, which `vertex_resources.h`
documents as the same operation, so both routes stay reached rather than one becoming a stale
import.

### 3. A documented ABI-0.9 contract change invalidated a binding test

`SoundEffectInstance.Apply3D` used to refuse every listener count but one; ABI 0.9.0 made it accept
any positive count, and made it refuse with `CNA_RESULT_INVALID_STATE` on a playing instance that
was never positioned. The native integration suite asserted the old refusal and now asserts both
halves of the new contract against the live artifact.

### 4. Stale version attribution in public diagnostics

Four public messages named "CNA ABI 0.7" as the reason for a limitation. Each was re-measured
against the live headers: the `VertexBuffer` one was false and was removed, and the `GameWindow`
supported-orientation, `Texture3D` and `TextureCube` element-type boundaries are still real and now
name the measured generation through the shared constants instead of a frozen literal.

## Upstream C API test observations

`ctest -R '^CApi_'` in this configuration reports 86 of 92 passing. The six failures are recorded
here as observations about this build's option combination rather than as CNA-TS defects, and none
of them touch a route this binding imports:

```text
CApi_AudioSmoke=CONFIGURATION_NULL_AUDIO (duration of a PCM16 effect is zero without a mixer)
CApi_AudioUnavailableSmoke=CONFIGURATION_NULL_AUDIO
CApi_ContentSmoke=CONFIGURATION_NULL_AUDIO (the sound asset in its fixture set)
CApi_DevicesSmoke=CONFIGURATION_HEADLESS_PLATFORM (CNA_DEVICES=ON with no SDL platform)
CApi_CoreExtSmoke=UNEXPLAINED_UPSTREAM (logger route validation, exit code 5)
CApi_Utf8Oracle=TARGET_NOT_BUILT (no cna_c_api_utf8_oracle_test target in this tree)
```

`CApi_LifecycleSmoke`, which validates game create/run/destroy, update and draw callbacks, sprite
submission and texture readback, passes -- so the game loop this binding depends on is exercised and
sound in this configuration. Rebuilding against `cnanext` 17b5a90a reproduced the same 86 of 92 with
the same six names, which is what makes it a baseline rather than a snapshot.

The wider `ctest -R CApi` selection additionally runs three upstream record-keeping gates
(`CApiCoverageMatrix`, `CApiLimitations`, `CApiReleaseGate`). All three currently fail on
`cnanext`'s own plan bookkeeping -- `CBIND-037` and `CBIND-114` are recorded complete while still
owning planned rows -- with no C API surface involved. That is another session's work in progress in
a repository this one does not modify, and it is recorded here only so a future run does not mistake
it for a regression this binding caused.

## The contract counts this audit holds

```text
ABI_VERSION=0.35.0
TARGETED_ABI_MATCHES_HEADERS=1
PUBLIC_HEADERS=60
EXPORTED_FUNCTIONS=3202
NODE_BRIDGE_IMPORTED_SYMBOLS=1040
NODE_BRIDGE_SIGNATURES_VERIFIED=1040
NODE_BRIDGE_SIGNATURE_MISMATCHES=0
MISSING_NODE_BRIDGE_SYMBOLS=0
WASM_ARTIFACT_EXPORTED_FUNCTIONS=3204
WASM_BACKEND_ROUTES=1014
MISSING_WASM_BACKEND_EXPORTS=0
WASM_ARTIFACT_ASYNCIFY_RUNTIME=0
WASM_ARTIFACT_WEBGL_MAJOR_VERSIONS=2
WASM_ARTIFACT_LINK_CONTRACT=OK_ASYNCIFY_OFF_WEBGL2
BROWSER_ARTIFACT_STATUS=PRESENT_NOT_EXECUTION_VERIFIED
```

`NODE_BRIDGE_IMPORTED_SYMBOLS` and `WASM_BACKEND_ROUTES` are *backend reachability* — how many C
routes each adapter actually imports. They are not the same dimension as the coverage report's
purpose classification, and neither is the canonical declaration count. All three are printed
separately here and in `docs/cna-api-coverage.md` for exactly that reason.

`WASM_ARTIFACT_LINK_CONTRACT` is new in this generation: it measures, out of the artifact's own
generated JavaScript, the two Emscripten link properties this package used to supply itself before
CNA repaired them. See `docs/upstream-cna-findings.md` items 3 and 4.

`BROWSER_ARTIFACT_STATUS` used to be derived from whether a `.wasm` was *committed* to the CNA
worktree, which answered nothing: the artifact is built out of tree and never checked in, so the
audit reported `MISSING` beside a module that had just run 600 browser frames. It now measures the
artifact the binding actually loads -- hashes, byte size, and whether the loader exposes every route
`WasmBackend` resolves at construction. The route names come from the ESM loader rather than the
`.wasm` export section, because a Release link minifies wasm export names to `Mi`, `Ni`, ... and the
loader is what maps a readable `Module["_cna_..."]` onto one. `PRESENT_NOT_EXECUTION_VERIFIED` is
deliberate: a complete artifact is not a running one, and execution evidence belongs to
`npm run test:wasm-browser`.

## The header-derived contract

Hand-maintaining hundreds of prototypes and constants beside a moving ABI is how a binding drifts.
`tools/cna-abi/contract.json` states, once, what CNA-TS depends on at the C boundary, and
`npm run verify:cna-contract` proves it by *generating a C translation unit and compiling it*
against `CNA/C/*.h` under `-std=c11 -Wall -Wextra -Werror`. Every claim is a `_Static_assert`, so an
absent constant, a changed value, a changed scalar width or a changed descriptor version is a
compile error. The TypeScript half of each claim is read out of `src/` rather than copied into the
contract, so what the package actually publishes is what gets proved.

```text
TYPESCRIPT_ENUMS=105
VERIFIED_ENUM_FAMILIES=104
MANAGED_ONLY_ENUMS=1
ENUM_MEMBER_CLAIMS=920
IDENTICAL_CLAIMS=917
TRANSLATED_CLAIMS=3
SCALAR_ASSERTIONS=31
RESULT_CODE_ASSERTIONS=15
STRUCT_VERSION_ASSERTIONS=6
STATIC_ASSERTIONS_COMPILED=PASS
CNA_ONLY_FAMILY_CONSTANTS=46
DECLARED_CNA_ONLY_CONSTANTS=46
DIAGNOSTICS=0
```

### What the first run found

429 of the 432 projected enum members already carry the identical number on both sides. Exactly
three do not, and two of them were live defects:

- **`BlendFunction.Min`/`Max` were exchanged on every native path.** XNA 4.0 numbers `Min = 3` and
  `Max = 4`; the CNA C ABI numbers `CNA_BLEND_FUNCTION_MAX = 3` and `CNA_BLEND_FUNCTION_MIN = 4`.
  The adapter passed the XNA number straight into `CNA_BlendState`, so a game asking for a minimum
  blend got a maximum and vice versa, through `GraphicsDevice.BlendState` and through both
  `SpriteBatch.Begin` overloads that take one. `src/internal/cna-enums.ts` now translates both
  directions and the three call sites use it.
- **`GamePadType.BigButtonPad`** is `0x300` in XNA and `9` in the C ABI. The adapter already handled
  this, as an unexplained inline `=== 9 ? 0x300` in the capability path; it is now a named,
  contract-declared translation with a round-trip test.

The 46 constants that share a mapped family's prefix without an XNA counterpart -- `_MAXIMUM`
sentinels, the `_EXT` surface beyond XNA 4.0, state presets, and sub-families such as
`CNA_KEY_MODIFIER_*` -- are listed in the contract, so a newly added one arrives as a diagnostic
rather than passing unnoticed. `CNA_SURFACE_FORMAT_BC7_EXT`, `CNA_PRIMITIVE_POINT_LIST_EXT` and
their neighbours are modern-CNA surface and belong outside `Microsoft.Xna.Framework.*`.

### Mutation controls

`test/cna-abi-contract.test.mjs` proves the verifier can fail, which is the only thing that makes a
green run evidence. Thirteen deliberate mutations are each asserted to produce their own diagnostic:
a wrong scalar width, a wrong result code, a wrong descriptor version, a suffix override naming no
constant, a prefix pointing at the wrong family, a dropped family, a translation with no translator,
a translation claiming the wrong value, a removed translation, an undeclared CNA-only constant, a
declared constant the headers no longer define, and a TypeScript enum member whose value drifts in a
copied source tree.
