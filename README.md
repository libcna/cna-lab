# CNA-Go Linux desktop canary

This repository is a deliberately small Foundation 1 consumer. It exercises a
real CNA-owned game loop, tick-exact lifecycle callbacks, native viewport and
clear, native PNG decoding, SpriteBatch drawing, keyboard polling, and normal
native exit. It contains no ContentManager/XNB, effect/3D, renderer-name guess,
or capability placeholder.

The checked-in `go.work` is the development workflow and resolves the sibling
`../cna-go` checkout. CNA-Go is not published yet. Isolated qualification must
instead replace `github.com/openeggbert/cna-go` with an extracted, audited CNA-Go
source artifact; it must not use this workspace file or the development tree.

The runtime requires cgo and an admitted CNA C ABI shared library — major 0
with minor 21 or newer, qualified at 0.21.0:

```sh
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so go run ./cmd/desktop --frames 60
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so go run ./cmd/desktop --frames 600
```

Only Linux amd64 desktop is targeted by Foundation 1. Windows, macOS, Android,
iOS, and Web/Wasm remain unqualified future work.
