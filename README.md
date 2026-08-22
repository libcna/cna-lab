# cna-ts-template

Canonical project template for both TypeScript and JavaScript consumers of `cna-ts`.

## Measured status

This repository is a truthful managed/build canary, not yet a playable CNA game. The TypeScript
source exercises the strict XNA projection's lifecycle shell, `GameTime`, `TimeSpan`, `Vector2`,
and `Color`. It reports the real runtime status instead of inventing a graphics backend.

| Target | Status | Evidence |
| --- | --- | --- |
| TypeScript project | build verified | strict TypeScript plus Vite production build |
| JavaScript project | build verified | generated ordinary `.js`, Vite build, Node managed smoke |
| Browser CNA runtime | planned / blocked | no packaged CNA C-ABI ESM/Wasm artifact is available |
| Electron | planned | no runtime or build claim |
| Android / iOS | planned | no WebView/native runtime or build claim |

The old `cna-js` package, Electron, and Capacitor are not dependencies. A browser bundle alone is
not described as CNA runtime support.

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

Once CNA supplies a consumable C-ABI WebAssembly ESM artifact, the canary can advance in this
order: real `Game.Run` lifecycle and shutdown, device/canvas, raw-image `Texture2D` loading,
`SpriteBatch`, keyboard/mouse, resize, 60-frame smoke, then 600-frame stability. Raw PNG files will
not be passed to `Content.Load` as though they were compiled XNB assets.

## License

MIT; see `LICENSE`.
