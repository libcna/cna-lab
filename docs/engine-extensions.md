# `cna.extensions.engine`

CNA retired its modern engine layer (`engine_layer.h`: compute, PBR materials,
post-process chains, shadows, clustered lighting, light probes, culling and
instancing) at C ABI 0.30 (`MOD-RETIRE-1`, CNA commit `5572f3ca1`). Two
standalone pieces survived in `graphics_ext.h`, and they are all this package
projects now:

| Public name | Routes |
|---|---|
| `DebugDraw`, `DebugLineVertex` | `cna_debug_draw_*` (16) |
| `AsciiEffect`, `AsciiQuantizeMode` | `cna_ascii_post_process_effect_*` (8) |
| `is_available()` | `cna_graphics_ext_is_available` |

The 2026-09-30 requalification removed eleven modules and 849 bound routes with
it; the full projection and its evidence are in Git history before commit
`3e098d3`.

It is a CNA extension, not XNA: nothing here appears in `Microsoft.Xna.Framework`,
and `tools/verify_extensions.py` asserts that importing XNA loads no `cna` module.

**Two kinds of "not supported".** Every route is exported by every CNA build. A
build without `CNA_CNAEXT` answers `CNA_RESULT_NOT_SUPPORTED`, which is reported
as `EngineUnavailableError` after asking `cna_graphics_ext_is_available`; a
present layer that still cannot do something raises `EngineUnsupportedError`.

**Routes not bound.** The CRT and colour-depth retro effects (never part of the
selected profile) and the value initialisers whose consumers CNA retired
(`cna_image_based_light_ext_*`, `cna_indirect_draw_*_arguments_init`); each has a
written rule in `tools/route-census-rules.json`.

**The eight ENGINE findings** this binding recorded against the engine layer
(ENGINE-001..008: compute compile diagnostics, counted-view getters, GPU timer
first sample, chain timing flag, PBR material textures, clustered constructor
parameters, GGX normal, cullable-instance default) are **no longer applicable**:
every route they concerned was removed. CNA had fixed or re-documented most of
them before the removal (BINDFIX-003/005/012/013/021/024/027/029).

Evidence: `tests/test_engine_debug.py`, `tests/test_engine_oracles.py`,
`tools/mutation/engine_mutations.py`; rows in `docs/runtime-capabilities.json`.
