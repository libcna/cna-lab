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
| Node CNA runtime | opt-in Linux HEADLESS 2D drawing verified | public `Texture2D.FromStream` and `SpriteBatch`; explicit ABI-0.7 library and bridge |
| Browser CNA runtime | planned / blocked | no packaged CNA C-ABI ESM/Wasm artifact is available |
| Electron | planned | no runtime or build claim |
| Android / iOS | planned | no WebView/native runtime or build claim |

The old `cna-js` package, Electron, and Capacitor are not dependencies. A browser bundle alone is
not described as CNA runtime support.

## Opt-in Node CNA smoke

The Node path executes the template's actual `HelloGame`: `GameTime`, `GraphicsDevice`,
`Texture2D.FromStream`, `Clear`, `SpriteBatch.Begin/Draw/End`, moving-sprite state,
keyboard/mouse/gamepad polling, and deterministic disposal through a prebuilt CNA ABI 0.7 library.
The embedded PNG is a raw-image stream and is deliberately not passed to `Content.Load`. This
evidence neither changes nor implies the browser support status:

```bash
CNA_NATIVE_LIBRARY=/absolute/path/to/libcna_c_api.so \
CNA_NODE_BRIDGE=/absolute/path/to/cna_node_bridge.node \
npm run smoke:native
```

Set `CNA_NATIVE_FRAMES=600` for the longer stability run. CNA-TS ships bridge source and a build
helper, but no platform-specific native binary or CNA library.

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
