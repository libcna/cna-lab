# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

### Changed

- Builds against the sibling `../cna` (branch `next`) and `../sharp-runtime` checkouts; the
  former `../cnanext` and `../sharp-runtimenext` no longer exist. `EffectPassCollection[i]` is a
  pointer in current CNA.
- Requests the HiDef graphics profile. Under XNA's default Reach profile the first 32-bit index
  buffer is refused, so every run ended at the first `CreateMesh`.

### CNA findings

- `--seed 2026` ended at tick 23 with `Window::SetSize(ExclusiveFullscreen) failed: Couldn't find
  any matching video modes` (resizing the back buffer while in exclusive fullscreen). Fixed in CNA
  by `68719c5b8` (SAMPLE-071): a size with no matching video mode takes the desktop mode. Seed 2026
  now runs 1500 ticks cleanly, and on a single-mode Xvfb it takes that fallback four times in 400
  ticks. Seeds 11-13 (nightmare) and 14 (high) run 6000 ticks each without an error.
- `--max-ticks` logged its stop three times: CNA kept running a slow frame's catch-up `Update`s
  after `Exit()`, where XNA stops. Fixed in CNA by `8dc7a7b99` (KILLER-1).

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
