# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

### Added

- Checking actions next to the churn: read-back verification of textures, render targets,
  buffers, the back buffer and pixel-exact full-screen quads; calls XNA 4.0 refuses, each
  expecting XNA's exception; corrupted images, WAVE files, XNB assets and effect bytecode;
  components that rearrange the collection they are iterated from; worker-thread resources;
  instancing, occlusion queries, every stock effect, cube and volume textures, streaming audio.
- Findings: a deviation that does not crash is recorded with its tick and the run carries on;
  a summary lists every distinct finding at the end. `--strict`, `--only`, `--exclude` and
  `--list-actions`; exit status 4 when a clean run had findings. `CNA_FINDINGS.md` lists them.

### Changed

- Builds against the sibling `../cna` (branch `next`) and `../sharp-runtime` checkouts; the
  former `../cnanext` and `../sharp-runtimenext` no longer exist. `EffectPassCollection[i]` is a
  pointer in current CNA.
- Requests the HiDef graphics profile. Under XNA's default Reach profile the first 32-bit index
  buffer is refused, so every run ended at the first `CreateMesh`.
- `WorkerThreadResources` no longer waits for its thread inside `Update`: CNA hands a worker the
  device between frames, so waiting there deadlocks (as on FNA and MonoGame). A job is started,
  the game keeps running frames, and the result is collected once the thread has finished.
- The "Viewport reaching past the back buffer" check used a rectangle that fits exactly in an
  odd-width back buffer (KF-10 was this); it now reaches one pixel past for every width.
- `SpriteBatch.End` after a texture was disposed accepts either CNA's documented behaviour
  (drawing it) or XNA's `ObjectDisposedException` (KF-11).
- A back-buffer read-back mismatch reports where the cleared colour actually is, with the window
  and viewport sizes.

### CNA findings

- `--seed 2026` ended at tick 23 with `Window::SetSize(ExclusiveFullscreen) failed: Couldn't find
  any matching video modes` (resizing the back buffer while in exclusive fullscreen). Fixed in CNA
  by `68719c5b8` (SAMPLE-071): a size with no matching video mode takes the desktop mode. Seed 2026
  now runs 1500 ticks cleanly, and on a single-mode Xvfb it takes that fallback four times in 400
  ticks. Seeds 11-13 (nightmare) and 14 (high) run 6000 ticks each without an error.
- `--max-ticks` logged its stop three times: CNA kept running a slow frame's catch-up `Update`s
  after `Exit()`, where XNA stops. Fixed in CNA by `8dc7a7b99` (KILLER-1).
- KF-1 to KF-17 (`CNA_FINDINGS.md`): twelve fixed in CNA, KF-3 gone with KF-2, KF-4 and KF-10 not
  defects, KF-11 kept deliberately, KF-6 fixed for scaled windows and otherwise a documented
  limitation of the back buffer being the window's surface. KF-17 (a WAVE with zero channels
  killing the process in SDL_mixer) was found by the final matrix run.

### Added

- Initial `cna-killer` implementation: an XNA-style `Game` built on `../cnanext` and
  `../sharp-runtimenext` that continuously creates and destroys textures, render targets,
  meshes, and audio resources; resizes and toggles the window between windowed/fullscreen/
  borderless; simulates minimize/restore; forces raw `GraphicsDevice` resets; and churns
  `BasicEffect` instances to simulate a shader hot-reload pipeline.
- Deterministic chaos: a single seeded `System::Random` drives every decision, so
  `--seed N` always reproduces the same crash tick-for-tick. `--stop-at-tick` supports
  bisecting a crash down to a minimal repro.
- An append-only, flush-per-line reproduction log with POSIX signal handlers that print a
  "how to reproduce this" banner on `SIGSEGV`/`SIGABRT`/`SIGFPE`/`SIGILL`/`SIGBUS`.
- `README.md`, MIT `LICENSE`, `.gitignore`, and `.gitattributes`.
