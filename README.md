# CNA Lab

`cna-lab` is a single interactive C++23 application built directly against the sibling
`../cnanext` and `../sharp-runtimenext` checkouts.  It is deliberately a laboratory rather
than a game: one window demonstrates the old XNA-shaped API and CNA's modern extensions
together, while loading the same model through several real content paths.

## What it exercises

- Animated 2D `SpriteBatch` field using runtime-created `Texture2D` data and alpha blending.
- Classic XNA 3D (`BasicEffect`, depth testing, textured immediate geometry).
- Modern CNA graphics (`CNA_CNAEXT`, `PbrEffect`, tangent-space vertex layout, metallic/roughness
  parameters), plus glTF's PBR material path.
- The same glTF 2.0 binary model via direct runtime `.glb` import, generated `.cnj`, and compiled
  `.cnb`; use Space to cycle these three sources.
- An actual MonoGame-produced `.xnb` texture after explicit XNB reader registration.
- A WAV compiled to `.cnb` and played through `SoundEffect`; press `S`.
- An FFmpeg-backed MP4 compiled to a `Video` `.cnb`, then decoded and drawn as a live `Texture2D`
  by `VideoPlayer`; press `V` to play/pause/resume.

The content is created during the build in the executable's `Content/` directory.  It is not a
pretend format showcase: the build invokes CNA's `gltf_to_cnj`, `gltf_to_cnb`, and
`source_to_cnb` tools, and the running program uses `ContentManager::Load` for every produced
asset.

## Build and run

```bash
cmake -S . -B build -DCNA_GRAPHICS_RENDERER=OPENGLES3
cmake --build build --parallel 2
./build/cna_lab
```

On the first OpenGLES3 configuration CNA may fetch its pinned FNA3D dependency.  All runtime
library sources themselves come from `../cnanext` and `../sharp-runtimenext`.

Video playback is intentionally required by this project (`CNA_ENABLE_VIDEO=ON`), so native Linux
builds need FFmpeg development packages discoverable by `pkg-config`.  The checked workspace already
provides them.  To make a display-free full content/media verification use:

```bash
cmake -S . -B build-headless -DCNA_GRAPHICS_RENDERER=HEADLESS
cmake --build build-headless --parallel 2
ctest --test-dir build-headless --output-on-failure
```

Controls: `Space` changes the active glTF/CNJ/CNB model, `S` plays the compiled sound, `V` toggles
video play/pause/resume, and `Esc` exits.  The window title reports the selected model source,
media state, and renderer.

The XNB fixture is copied only into the build output from CNA's test assets; it is not committed in
this repository.  The glTF and sound come from `../cnanext`; the small MP4 fixture comes from
`../cna-examples`.  They are all copied only during the build.

## Repository files

- `LICENSE` contains the license for cna-lab's own source code.
- `NOTICE.md` identifies the adjacent runtime projects and demonstration assets.
- `.gitignore` excludes generated CMake, binary and editor artefacts.
- `.editorconfig` provides the shared whitespace and indentation policy.

## License

cna-lab is released under the [MIT License](LICENSE).  CNA and sharp-runtime remain separate
dependencies with their respective licenses; see [NOTICE.md](NOTICE.md).
