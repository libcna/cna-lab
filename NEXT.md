# CNA-Python tactical handoff

Date: 2026-09-30. Requalified against CNA C ABI `0.35.0` (CNA `next` `4228ff913`,
sharp-runtime `88f6b11fb`). The previous handoff (2026-09-02, ABI 0.21) is in Git
history before commit `3e098d3`.

```text
STOP_CONDITION=held  COUNTERS_CHECKED=43  FAILURES=0
DEFAULT_PROFILE_TYPES=257  DEFAULT_PROFILE_MEMBERS=2423
```

## Structural surface (unchanged by the requalification)

| profile | types | diagnostics |
| --- | --- | --- |
| `xna40-windows-runtime` | 257 | 0 |
| `xna40-windows-online` | 74 | 0 |
| `xna40-windows-content-pipeline` | 128 | 0 |
| `xna40-xbox360-runtime` | 318 | 0 |

Windows Phone stays `BLOCKED_REFERENCE_ASSET` (assemblies absent).

## Native boundary

```text
CANONICAL_ROUTES=3202  BOUND=1770  UNREVIEWED=0  RULE_CONTRADICTIONS=0
PROTOTYPES_COMPILER_VERIFIED=1770  C_LAYOUT_MEASUREMENTS=2105
MISSING_SYMBOLS=0  ABI_MISMATCHES=0
REACHABILITY: DIRECT=1673 TEMPLATE=18 ADMITTED=37 UNJUSTIFIED=0 STALE=0
EXTENSIONS: MODULES=51 PUBLIC_NAMES=751 DIAGNOSTICS=0
```

What 0.35 changed for this binding: CNA retired `engine_layer.h` (0.30), the
avatar real-rendering extension (0.33) and the Guide setters (0.34), 849 bound
routes in all; `cna.extensions.engine` is now DebugDraw and the ASCII effect.
The pending-route stub is gone. Findings: `docs/online-upstream-findings.md`,
`docs/engine-extensions.md`, `docs/cna-abi-audit.md`.

## Tests (2026-09-30)

| artifact | tests | skipped | result |
| --- | --- | --- | --- |
| no CNA library | 913 | 486 | OK |
| `~/deps/cna-c-abi-0.35.0` (HEADLESS, Debug, no FFmpeg) | 913 | 116 | OK |
| `~/deps/cna-c-abi-0.35.0-opengles3-fx` (OPENGLES3, compiled effects, CNAEXT, devices; private Weston + Xwayland on the real GPU) | 913 | 4 | OK |

Optional fixtures used: `CNA_PYTHON_COMPILED_EFFECT_FIXTURE` =
`cna/modules/renderers/fna3d/effects/CnaConformanceEffect.fxb`,
`CNA_CONTENT_TOOLS` = `cna/cmake-build-debug`. The four OPENGLES3 skips are the
two absence-only cases, the LZX fixture and one tool cross-check.

Mutation: engine 7/7 (the availability mutant dies on the HEADLESS artifact, the
other six on OPENGLES3), device/input/online 76/76, CNB 35/35, pipeline 99/99.

Packaging: wheel byte- and content-reproducible, sdist content-reproducible;
`audit_package.py` clean; `verify_consumer.py` against the sibling template on
both artifacts: 60/600 frames PASS, CNB and engine smokes PASS, frame pixels
`drawn` on OPENGLES3 and `unavailable` (no pixel storage) on HEADLESS.

## Commands

```sh
export PYTHONPATH=$PWD/src:$PWD CNA=/rv/data/development/github.com/libcna/cna
export TMPDIR=$PWD/build/tmp            # tools use tempfile; keep it in the repo
for p in xna40-windows-runtime xna40-windows-online \
         xna40-windows-content-pipeline xna40-xbox360-runtime; do
  python3 tools/api_compat/verify.py --profile "$p" --check
done
python3 tools/route_census.py --cna-root "$CNA" \
  --output docs/generated/cna-route-census.json --markdown docs/generated/cna-route-census.md
python3 tools/verify_prototypes.py --cna-root "$CNA"
python3 tools/audit_cna_abi.py --cna-root "$CNA" \
  --library ~/deps/cna-c-abi-0.35.0/lib/libcna_c_api.so --output docs/generated/cna-abi-report.json
python3 tools/verify_route_reachability.py --output docs/generated/route-reachability.json
python3 tools/verify_extensions.py --output docs/generated/extension-surface-report.json
python3 tools/verify_stop_condition.py --output docs/generated/stop-condition.json
CNA_NATIVE_LIBRARY=~/deps/cna-c-abi-0.35.0/lib/libcna_c_api.so python3 -m unittest discover -s tests -t .
# windowed artifacts only inside CNA's private runner, never on the desktop:
$CNA/tools/platform/run_gpu_tests_private.sh --exec <script exporting SDL_VIDEODRIVER=x11 and the library>
python3 -m build --wheel --sdist --outdir dist .
python3 tools/verify_consumer.py --wheel dist/cna_python-0.1.0.dev0-py3-none-any.whl \
  --template ../cna-python-template --library <libcna_c_api.so>
```

## What is left

Waiting on CNA: online finding 3 (no route reads a network gamer's gamertag),
the `cnb.h` SoundEffect schema constant and importer prose, `net.h`'s stale
packet-colour prose, and the BLOCKED_UPSTREAM capability rows. Waiting on
hardware or platforms: Windows, macOS, mobile, real sensors, a console. Nothing
is pushed.
