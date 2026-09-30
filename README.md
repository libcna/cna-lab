# CNA-Go Linux desktop canary

This repository is a deliberately small consumer of CNA-Go. It runs a real
CNA-owned game loop with tick-exact lifecycle callbacks, a GraphicsDeviceManager
and its device, native viewport and clear, native PNG decoding into a
`Texture2D`, SpriteBatch drawing, keyboard polling (Escape exits), and a normal
native exit after exactly the requested number of Draw callbacks. It has no
pixel readback of its own; CNA-Go's `tools/native_stress` carries the pixel
evidence.

The runtime requires cgo and an admitted CNA C ABI shared library — major 0
with minor 35 or newer, qualified at 0.35.0:

```sh
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so go run ./cmd/desktop --frames 60
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so go run ./cmd/desktop --frames 600
```

A run prints `CNA_GO_CANARY_DRAW_FRAMES=<n>` and exits 0; `--frames` accepts
exactly 60 or 600.

## Two ways to consume CNA-Go

The checked-in `go.work` is the development workflow and resolves the sibling
`../cna-go` checkout. CNA-Go is not published, so an external consumer builds
against an extracted source tree instead, with no workspace file:

```sh
git -C ../cna-go archive --format=tar HEAD | tar -x -C <dir>/cna-go-src
rm go.work
go mod edit -replace github.com/openeggbert/cna-go=<dir>/cna-go-src
GOWORK=off GOFLAGS=-mod=mod go build ./cmd/desktop
```

## Measured on 2026-09-30

Against CNA-Go `9e46194` and CNA `next` `5b4edd6cc` (C ABI 0.35.0), Go 1.24.4,
Linux amd64:

| consumption | renderer | 60 frames | 600 frames |
|---|---|---|---|
| `go.work` | HEADLESS | exit 0 | exit 0 |
| `go.work` | OPENGLES3 | exit 0 | exit 0 |
| `git archive` + `GOWORK=off` + `replace` | HEADLESS | exit 0 | exit 0 |
| `git archive` + `GOWORK=off` + `replace` | OPENGLES3 | exit 0 | exit 0 |

Each run loaded the 128×128 logo and reported an 800×480 viewport. OPENGLES3
ran on the real GPU inside CNA's private compositor
(`tools/platform/run_gpu_tests_private.sh --exec`, `SDL_VIDEODRIVER=x11`,
`SDL_AUDIO_DRIVER=dummy`), never on a desktop session.

Only Linux amd64 desktop is qualified. Windows, macOS, Android, iOS, and
Web/Wasm remain unqualified.
