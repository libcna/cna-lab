# cna-ts-template

Canonical project template for both TypeScript and JavaScript consumers of `cna-ts`.

## Measured status

This repository is both a portable managed/build canary and an opt-in real CNA 2D game. The same
`HelloGame` source exercises managed components/services without a backend in browser builds and,
when an explicit Node CNA backend is loaded, runs `Game`, uploads an embedded PNG with
`Texture2D.FromStream`, moves it, and draws it through `SpriteBatch`.

| Target | Status | Evidence |
| --- | --- | --- |
| TypeScript project | build verified | strict TypeScript plus Vite production build |
| JavaScript project | build verified | generated ordinary `.js`, Vite build, Node managed smoke |
| Node CNA runtime | opt-in Linux HEADLESS 2D drawing verified | public `Texture2D.FromStream` and `SpriteBatch`; explicit CNA library and bridge |
| Browser CNA runtime | opt-in WebGL2 runtime verified | 60 and 600 real frames in headless Chromium against a `cna_c_api` WebAssembly module |
| Modern CNA extensions | opt-in runtime services verified | platform, renderer selection and log through `cna-ts/extensions/runtime` |
| Electron | planned | no runtime or build claim |
| Android / iOS | planned | no WebView/native runtime or build claim |

The old `cna-js` package, Electron, and Capacitor are not dependencies. A browser bundle alone is
still not described as CNA runtime support: the browser row above is a frame count from a real
WebGL2 context, not a Vite build that succeeded.

## Opt-in Node CNA smoke

The Node path executes the template's actual `HelloGame`: `GameTime`, `GraphicsDevice`,
`Texture2D.FromStream`, `Clear`, `SpriteBatch.Begin/Draw/End`, moving-sprite state,
keyboard/mouse/gamepad polling, and deterministic disposal through a prebuilt CNA library. The
template pins no CNA ABI number of its own: `cna-ts` refuses a library outside the generation it
targets, so what the canary checks is that a real backend loaded and reported a version. Set
`CNA_EXPECTED_ABI` to require an exact one for a pinned deployment. The embedded PNG is a raw-image
stream and is deliberately not passed to `Content.Load`:

```bash
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so \
CNA_NODE_BRIDGE=/absolute/path/to/cna_node_bridge.node \
npm run smoke:native
```

Set `CNA_NATIVE_FRAMES=600` for the longer stability run. CNA-TS ships bridge source and a build
helper, but no platform-specific native binary or CNA library.

## Opt-in browser CNA runtime

The same `HelloGame` runs in a browser on the CNA WebAssembly runtime. Nothing about the game
changes: a browser owns its event loop, so frames come from `requestAnimationFrame` and
`IsFixedTimeStep` is off, which is what `Game.RunOneFrame` exists for.

```bash
CNA_WASM_ARTIFACT_DIR=/absolute/path/holding/cna_c_api.mjs \
npm run smoke:browser
```

Set `CNA_BROWSER_FRAMES=600` for the longer run. The harness serves the built bundle and the
artifact, drives the game in headless Chromium, and fails on any uncaught page error; a CNA `INFO`
line on stderr is runtime logging rather than a page error and is classified as such. Playwright is
not a dependency of this template -- the harness uses one where it is already installed and says so
where it is not.

Open the dev server with `?wasm=<url to cna_c_api.mjs>` to do the same by hand.

## Opt-in modern CNA extension smoke

The default `HelloGame` touches nothing outside `Microsoft.Xna.Framework`, which is the point of
the modern surface living on its own subpath. The extension smoke asks CNA which host it is on,
which renderer it selected, what else it could run, and whether the extended graphics layer is
compiled in -- reporting the truthful `NOT_SUPPORTED_BACKEND` branch where it is not.

```bash
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so \
CNA_NODE_BRIDGE=/absolute/path/to/cna_node_bridge.node \
npm run smoke:extensions
```

## Work with the canonical TypeScript canary

Requirements: Node `^20.19.0 || >=22.12.0` and npm.

```bash
npm install
npm run check
npm run build
npm run dev
```

The repository pins `cna-ts` `0.1.0`. During sibling development, verification installs the exact
packed `cna-ts-0.1.0.tgz` with `--no-save`; normal generated projects never reference sibling
source directories.

## Generate a fresh project

TypeScript is the one maintained source. The JavaScript form is transpiled by the generator and
does not carry TypeScript source or a TypeScript application dependency.

```bash
npm run create:typescript
npm run create:javascript
```

Or choose explicit empty output directories:

```bash
node tools/create-project.mjs --language typescript --output /tmp/my-cna-ts-game
node tools/create-project.mjs --language javascript --output /tmp/my-cna-game-js
```

To verify both generated forms against a locally packed canonical artifact:

```bash
node tools/verify-generated.mjs --package /path/to/cna-ts-0.1.0.tgz
```

The verifier creates fresh temporary projects, checks for legacy/sibling references, installs the
tarball, builds both forms, and runs the generated JavaScript managed smoke test.

## Next functional slice

The Node-native 2D slice is complete. SpriteFont/XNB, model loading, and managed BasicEffect state
are verified in CNA-TS itself; they should enter this template only as reproducible sample assets
and renderer capability permit. Browser execution remains blocked until CNA supplies a consumable
C-ABI WebAssembly ESM artifact; a successful Vite bundle is not runtime evidence.

## License

MIT; see `LICENSE`.
