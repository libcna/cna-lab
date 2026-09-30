# cna-killer

A deliberately malicious "game" that exists purely to break [CNA](../cna), the C++
reimplementation of the XNA 4.0 programming model. It is a fuzz tester wearing a game's
clothes: every frame it hammers the CNA runtime with the kind of resource churn and state
abuse a real game would never do on purpose, hoping to make the framework crash, leak, or
misbehave.

## What it actually does

Each `Update()` tick, `cna-killer` rolls the dice and performs one or more of the following,
entirely through CNA's public XNA-compatible API (`Microsoft::Xna::Framework::*`):

- **Textures** — creates `Texture2D`s of wildly varying sizes (occasionally huge spikes) filled
  with adversarial noise, and destroys existing ones out of order.
- **Render targets** — creates and destroys `RenderTarget2D`s (including mip chains,
  depth/stencil formats, MSAA, and content-preservation combinations), and binds/clears them
  *from `Update()`*, outside the `Draw()`/`BeginDraw()`/`EndDraw()` contract a well-behaved game
  would respect.
- **Meshes** — creates and destroys `VertexBuffer`/`IndexBuffer` pairs and draws them with
  `DrawIndexedPrimitives`.
- **Audio** — synthesizes short, loud procedural PCM tone bursts into `SoundEffect`s, plays
  them, and destroys them.
- **"Shader hot reload"** — churns `BasicEffect` instances (disposing and recreating them with
  randomized parameters) to reproduce what a real hot-reload pipeline does to a running game:
  swap the effect object out from under whatever is mid-frame. CNA's public API only accepts
  pre-compiled effect bytecode, same as real XNA/FNA, so there is no runtime shader source
  compiler to hot-swap here — this is the closest faithful reproduction of that failure mode.
- **Window chaos** — resizes the back buffer to arbitrary (including degenerate) dimensions,
  toggles fullscreen and borderless, changes the title to garbage/unicode/absurdly long
  strings, and minimizes/restores the window ("alt-tab").
- **Device loss** — calls `GraphicsDevice::Reset()` directly, sometimes with a mutated
  `PresentationParameters`, bypassing `GraphicsDeviceManager` entirely (a real game only ever
  goes through `ApplyChanges()`; this deliberately doesn't).
- **Render state spam** — randomizes `BlendState`, `DepthStencilState`, `RasterizerState`, and
  the scissor rectangle every tick.

Whatever textures and meshes are still alive are drawn every frame so the chaos is visible on
screen, not just in a log file.

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
| `-h`, `--help` | Print usage and exit. |

Every flag above also has a `CNA_KILLER_*` environment variable equivalent (`CNA_KILLER_SEED`,
`CNA_KILLER_INTENSITY`, `CNA_KILLER_MAX_TICKS`, `CNA_KILLER_DURATION`, `CNA_KILLER_LOG`); a
command-line flag always wins over the matching environment variable.

The game asks for the HiDef profile: 32-bit index buffers, 2048-texel render targets and MSAA
are all refused under Reach, which is what an XNA game gets by default.

On a headless machine or a private test display with no sound server, `SoundEffect` creation
throws `NoAudioHardwareException`, as it does in XNA; run with `SDL_AUDIO_DRIVER=dummy` to keep the
audio actions in play without an audio device.

Press `Escape` or the gamepad Back button to quit at any time — the run is otherwise unbounded
by default, so it will keep hammering CNA until something gives or you stop it.

## License

MIT — see [LICENSE](LICENSE).
