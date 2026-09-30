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
or renderer capability support, and its runs check frame counts and a clean
exit rather than pixels.

`--extensions-smoke` is a separate, opt-in mode that exercises the CNA
extension surface -- the capabilities CNA has and XNA 4.0 never did. It prints
the platform, the renderer and that backend's category and maturity, writes one
message through CNA's logger, and reports whether this build carries the
extended graphics layer. Inside one real frame it asks which renderers the build
has, which is selected and active, sets and reads back a physically based
effect, compiles a `ShaderEffect`, and -- where the layer is present -- queues a
debug box and checks it is twelve edges; where it is absent it checks that
`DebugDraw` is refused rather than handed back. It is deliberately not part of
`HelloGame`: the starter stays an XNA program an XNA developer recognizes, and
nothing in `HelloGame` imports a CNA extension.

What the smoke proves is narrow and honest: the extension packages compile
against the published artifact from outside the binding, the JNI routes behind
them exist, the availability query answers rather than guesses, and a build
without the extended layer says `NOT_SUPPORTED` rather than doing something
else. It does not claim any rendering happened.

It also builds content the way a build step would, outside the game entirely:
it writes a `.cnb` texture with CNA's own encoder, reads it back and compares
the pixels, and imports a WAV written from that format's published layout. None
of that needs a window, a device or a frame, which is the point -- it is the
half of a content pipeline that runs before a game exists.

```bash
./gradlew :game:run -PcnaRepository=/path/to/repo \
    -Dcna.java.jniLibrary=/path/to/libcna_java_jni.so \
    -Dcna.native.library=/path/to/libcna_c_api.so \
    --args=--extensions-smoke
```

Gradle's `run` starts a fresh JVM, so the two library properties have to be
forwarded rather than inherited; `game/build.gradle` does that. Leave them out
only if both libraries are already on the system library path.

| Target | Status |
| --- | --- |
| Linux x86-64, CNA C ABI 0.35.0 HEADLESS | Runtime verified 2026-09-30 (60 frames, 600 frames, extensions smoke incl. content, generated project) |
| Linux x86-64, CNA C ABI 0.35.0 OPENGLES3 | Runtime verified 2026-09-30 in CNA's private GPU runner (same runs; extended layer and compiled effects present) |
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
a fresh Maven repository in that checkout's `build-consumer/`, builds this
project against that exact artifact and JNI library, and verifies a freshly
generated project. When `CNA_NATIVE_LIBRARY` is set it also runs the 60-frame
smoke test, the extensions smoke and the 600-frame stability run.

## Generate a new project

```bash
python3 scripts/generate_project.py \
  --output ../asteroids-java \
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
