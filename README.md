# cna-killer

A deliberately malicious "game" that exists purely to break [CNA](../cna), the C++
reimplementation of the XNA 4.0 programming model. It is a fuzz tester wearing a game's
clothes: every frame it hammers the CNA runtime with the kind of resource churn and state
abuse a real game would never do on purpose, hoping to make the framework crash, leak, or
misbehave.

## What it actually does

Each `Update()` tick, `cna-killer` picks one or more actions from a weighted table and runs them
entirely through CNA's public XNA-compatible API (`Microsoft::Xna::Framework::*`).
`--list-actions` prints the table. The actions come in families:

- **resource** — creates and destroys textures (with huge spikes), render targets (mip chains,
  depth/stencil, MSAA, content preservation), meshes, copies of textures, cube and volume
  textures, and churns `BasicEffect`s the way a shader hot-reload pipeline would.
- **render** — draws from `Update()`, outside the `Draw()` contract: every primitive type through
  every draw path (user, indexed 16/32-bit, vertex/index buffers, instancing), every stock effect
  with hostile parameters (NaN matrices, degenerate projections, 72 bones), SpriteBatch in every
  sort mode with thousands of hostile sprites, occlusion queries.
- **verify** — reads back what was written or drawn and compares it with XNA's result: texture
  `SetData`/`GetData` in all 19 HiDef formats by level and rectangle, render-target clears per
  format (with MSAA and `PreserveContents`), multiple render targets, cube faces, vertex and index
  buffers at byte offsets, the back buffer, and full-screen quads whose every pixel is known —
  blending, viewport, scissor, culling, depth and stencil tests, colour write masks, sprite
  placement and texture orientation.
- **misuse** — calls the API the way XNA 4.0 refuses and expects XNA's exception: SpriteBatch
  pairing, invalid viewports and scissor rectangles, bad draw arguments, active or disposed
  resources, occlusion-query pairing, audio parameters, game timing. Every rule was read from the
  XNA 4.0 assemblies (`../xna4-decomp`), not from FNA.
- **fuzz** — corrupted PNG/JPEG/GIF/BMP images, WAVE files, XNB assets (including LZX- and
  LZ4-compressed ones) and effect bytecode. Each starts from a valid file generated in memory and
  checks that the valid file loads correctly first.
- **audio** — procedural sounds, instances driven through every state and 3D positioning,
  streaming voices fed from their `BufferNeeded` event, the global settings.
- **window** / **device** — back buffer resizes (to 1x1 and 4096x4096), fullscreen, borderless,
  minimize/restore, UTF-8 and invalid-UTF-8 titles, raw `GraphicsDevice::Reset()` with mutated
  parameters, multisampling and back buffer/depth format changes.
- **loop** — `GameComponent`s that add, remove and destroy each other while `Game` iterates them,
  and changes to `TargetElapsedTime`, `IsFixedTimeStep` and `InactiveSleepTime`.
- **thread** — textures, meshes and render targets created, filled and destroyed on worker
  threads, which XNA 4.0 allows.
- **input** — mouse positions far outside the window, gamepad queries and vibration.

Whatever textures are still alive are drawn every frame so the chaos is visible on screen.

## Findings

A crash ends the process and is recorded by the log and the signal banner. Everything else is a
finding: an exception from a call XNA accepts, data or pixels that are not XNA's, a refusal XNA
makes that CNA does not (or makes with a different exception type), memory that keeps growing
while every pool stays bounded. A finding is logged with its tick and the run carries on;
`--strict` stops at the first one. At the end the run prints one line per distinct finding with
its count.

Exit status: `0` clean run without findings, `4` clean run with findings, `3` stopped by
`--strict`, `1` ended by an exception, `2` bad arguments.

`CNA_FINDINGS.md` lists what the runs have found so far.

## Reproducibility: the whole point of the seed

Every one of the decisions above — which action, how big a texture, which pooled resource to
kill — comes from a single seeded `System::Random`, and depends **only on the tick count**,
never on wall-clock time or frame duration. That means:

```
cna-killer --seed 123456789
```

always performs the *exact same sequence* of chaos actions, tick for tick, no matter the
machine or frame rate. If a run crashes, rerunning with the same seed reproduces it.

`--stop-at-tick N` exits cleanly right before tick `N` runs, so a crash can be bisected down to
a minimal repro: rerun with the seed that crashed and a `--stop-at-tick` a little past where the
previous run's log stopped, and narrow from there.

Draw-time sprite placement (purely cosmetic) uses a second, independent RNG stream so how the
chaos happens to *look* can vary run to run without ever perturbing the reproducible action
sequence above.

### The reproduction log

Every run appends to a log (default `cna-killer-<seed>.log`, override with `--log`). Each line
is flushed to disk immediately, and on Linux/macOS a signal handler for `SIGSEGV`/`SIGABRT`/
`SIGFPE`/`SIGILL`/`SIGBUS` prints a one-line "here's how to reproduce this" banner to stderr
before the process terminates normally — so even a hard crash that skips every C++ destructor
still leaves behind the seed and the tick it died on.

## Building

`cna-killer` builds against sibling checkouts of `../cna` (branch `next`) and `../sharp-runtime`,
exactly like `cna-samples` does — `CMakeLists.txt` adds `../cna` as a subdirectory itself,
so no extra setup is needed beyond having those two repositories checked out next to this one.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache
cmake --build build -j$(nproc)
```

Reuse the `build/` directory across sessions rather than reconfiguring from scratch — see the
workspace-root `CLAUDE.md` for why.

## Running

```sh
./build/cna-killer --seed=1 --intensity=nightmare
```

| Flag | Meaning |
| --- | --- |
| `--seed=N` | Seed the chaos engine. Reusing a seed reproduces the exact same run. |
| `--intensity=LEVEL` | `low`, `medium`, `high`, or `nightmare` (default `medium`). Controls how many actions run per tick, how many live resources are allowed to accumulate, and how often the most disruptive actions (resize/fullscreen/reset/window state) fire. |
| `--max-ticks=N` | Exit cleanly after `N` ticks. |
| `--duration=SECONDS` | Exit cleanly after this many seconds of wall-clock run time. |
| `--stop-at-tick=N` | Exit right before tick `N` runs, for bisecting a crash. |
| `--log=PATH` | Reproduction log path (default `cna-killer-<seed>.log`). |
| `--only=LIST` | Run only these actions or families (comma-separated), to narrow a finding down. |
| `--exclude=LIST` | Never run these actions or families. |
| `--strict` | Stop at the first finding. |
| `--list-actions` | Print every action with its family and weight, then exit. |
| `-h`, `--help` | Print usage and exit. |

Every flag above also has a `CNA_KILLER_*` environment variable equivalent (`CNA_KILLER_SEED`,
`CNA_KILLER_INTENSITY`, `CNA_KILLER_MAX_TICKS`, `CNA_KILLER_DURATION`, `CNA_KILLER_LOG`,
`CNA_KILLER_ONLY`, `CNA_KILLER_EXCLUDE`, `CNA_KILLER_STRICT`); a command-line flag always wins
over the matching environment variable.

The game asks for the HiDef profile: 32-bit index buffers, 2048-texel render targets and MSAA
are all refused under Reach, which is what an XNA game gets by default.

On a headless machine or a private test display with no sound server, `SoundEffect` creation
throws `NoAudioHardwareException`, as it does in XNA; run with `SDL_AUDIO_DRIVER=dummy` to keep the
audio actions in play without an audio device.

Press `Escape` or the gamepad Back button to quit at any time — the run is otherwise unbounded
by default, so it will keep hammering CNA until something gives or you stop it.

## License

MIT — see [LICENSE](LICENSE).
