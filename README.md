# CNA-Java desktop starter

This repository is the end-to-end canary and reusable starter for
[`org.openeggbert:cna-java:0.1.0-SNAPSHOT`](https://github.com/openeggbert/cna-java).
Its public source follows the normative XNA-to-Java naming rules: XNA methods
remain PascalCase and properties become `getFoo()` / `setFoo(...)`.

## Current verified scope

The desktop module is real, but intentionally small. It creates a CNA-backed
`Game`, attaches `GraphicsDeviceManager`, receives `GameTime`, clears through
`GraphicsDevice`, configures the mapped `GameWindow` title before native
startup, captures CNA-backed `KeyboardState` snapshots (Escape exits), and exits
deterministically after a requested frame count. It also captures CNA-backed
`MouseState` snapshots (left click exits), decodes a real raw PNG through
`Texture2D.FromStream`, loads a separate generated uncompressed Color texture
through `ContentManager.Load(Texture2D.class, ...)`, and draws both through
`SpriteBatch`. The two Base64 files are transport-safe repository fixtures:
one decodes to PNG and the other to a deterministic 135-byte Windows XNB v5
asset. The starter does not yet claim SpriteFont XNB, gamepad/touch, 3D, Model,
or renderer capability support.

`--extensions-smoke` is a separate, opt-in mode that exercises the CNA
extension surface -- the capabilities CNA has and XNA 4.0 never did. It prints
the platform, the renderer and that backend's category and maturity, writes one
message through CNA's logger, reports whether this build carries the extended
graphics layer, and reads CNA's own defaults for the render pipeline and a
physically based material. It is deliberately not part of `HelloGame`: the
starter stays an XNA program an XNA developer recognizes, and nothing in
`HelloGame` imports a CNA extension.

What the smoke proves is narrow and honest: the extension packages compile
against the published artifact from outside the binding, the JNI routes behind
them exist, the availability query answers rather than guesses, and a build
without the extended layer says `NOT_SUPPORTED` rather than doing something
else. It does not claim any rendering happened.

```bash
./gradlew :game:run -PcnaRepository=/path/to/repo --args=--extensions-smoke
```

| Target | Status |
| --- | --- |
| Linux x86-64, HEADLESS CNA 0.20.0 | Runtime verified (60 frames, 600 frames, extensions smoke) |
| Windows desktop | Planned; no runtime evidence in this repository |
| macOS desktop | Planned; no runtime evidence in this repository |
| Android | Planned; the old non-running Activity scaffold was removed |
| iOS | Unsupported; no module or native packaging exists |
| Browser / WebAssembly | Unsupported; desktop JNI cannot run in GWT or TeaVM |

## Build

JDK 17 or newer is required. The pinned Gradle Wrapper is included. The CNA
Java artifact is resolved only from the repository supplied by
`-PcnaRepository`; no developer-global Maven cache is required by the sibling
verification workflow.

```bash
./gradlew clean test -PcnaRepository=/absolute/path/to/a/maven/repository
```

For a native run, also provide the JNI adapter and CNA C ABI library:

```bash
CNA_JNI_LIBRARY=/path/to/libcna_java_jni.so \
CNA_NATIVE_LIBRARY=/path/to/libcna_c_api.so \
./gradlew :game:run \
  -PcnaRepository=/absolute/path/to/a/maven/repository \
  --args='--smoke-test'
```

Execution modes are `--smoke-test` (60 frames), `--stability-test` (600
frames), and `--frames N` / `--frames=N` for an explicit positive limit. A run
without a frame limit continues until the platform requests exit.

From the sibling `cna-java` checkout, `scripts/verify-template.sh` publishes to
a temporary Maven repository, builds this project against that exact artifact,
and verifies a freshly generated project. When `CNA_NATIVE_LIBRARY` is set it
also executes the native smoke test.

## Generate a new project

```bash
python3 scripts/generate_project.py \
  --output /tmp/asteroids-java \
  --project-name 'Asteroids Java' \
  --package com.example.asteroids \
  --application-id com.example.asteroids \
  --game-class AsteroidsGame \
  --group com.example \
  --artifact-id asteroids-java
```

The generated copy has no dependency on this source checkout and builds with
the same `-PcnaRepository=...` workflow.

Licensed under the [MIT License](LICENSE).
