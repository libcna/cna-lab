# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

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
