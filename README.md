# CNA-TS

`cna-ts` is the single canonical TypeScript and JavaScript binding for
[CNA](https://github.com/libcna/cna). TypeScript source is the only implementation source;
the package build emits the JavaScript used by both languages and the declarations used by
TypeScript.

> Status: the complete XNA 4.0 **runtime** surface is projected and verified. The seven-assembly
> Windows runtime profile (257 reference types, 2,964 members) and the GamerServices/Net/Avatar set
> (74 types) hold at zero differences, zero runtime-symbol differences and zero internal leaks,
> with no allowlist. The content pipeline is deliberately not projected: it runs in the content
> build rather than in a game.
>
> Two real backends run the same public API against CNA C ABI 0.35.0 (measured 2026-09-30 at CNA
> `next` 5b4edd6cc). An opt-in Node-API bridge imports 1,040 routes, each prototype-checked against
> CNA's headers; it runs on Linux against a HEADLESS library (SDL3 platform and audio) and against
> a windowed OPENGLES3 library under CNA's private display runner, where it draws and reads back
> real pixels. A WebAssembly backend reaches 1,014 of those routes and runs the same `Game`,
> `GraphicsDeviceManager`, `Texture2D` and `SpriteBatch` for 60 and 600 frames in headless
> Chromium on a WebGL2 context. No native binary and no CNA library is bundled, and without an
> explicitly loaded backend native operations fail rather than simulating execution.
>
> CNA surface outside XNA lives under `cna-ts/extensions`: `runtime` (platform identity, the 18
> renderer identities, the runtime log), `graphics` (the CRT, depth and ASCII screen effects, PBR
> effects, debug drawing, shader effects, capability queries and indirect-draw arguments -- CNA
> removed its engine layer at ABI 0.30 and this package followed), `content` (`.cnb`, CNA's own
> compiled content), `devices` (cores, memory, power, safe area, locales, clipboard, cameras),
> `sensors` and `input`. Gamer services run CNA's dispatcher and Guide screens; a signed-in gamer
> needs a gamer service, and `GamerServicesNotAvailableException` is what XNA raises without one.

## One package for both languages

JavaScript consumers do not need TypeScript in their application:

```js
import { Color, Game, Input, Vector2 } from "cna-ts";

const position = new Vector2(100, 100);
const clearColor = Color.CornflowerBlue;
const keys = new Input.KeyboardState([Input.Keys.Space]);
```

TypeScript consumers use the same imports and the same generated JavaScript:

```ts
import { Color, Game, Vector2 } from "cna-ts";
```

The strict XNA projection is also available from `cna-ts/xna`. CNA-specific functionality is
isolated under `cna-ts/extensions`, while backend status is exposed by `cna-ts/runtime`. Internal
backend modules are not package exports.

## Opt-in Node CNA runtime

The source distribution includes `native/cna_node_bridge.c` and a build helper. Build the adapter
against a CNA ABI 0.35 header checkout and Node 20+ headers, then load an explicit compatible
shared library:

```bash
CNA_SOURCE_PATH=/path/to/cna npm run build:native-bridge
```

```ts
import { LoadNodeNativeBackend } from "cna-ts/runtime";

await LoadNodeNativeBackend({
  CnaLibrary: "/absolute/path/to/libcna_c_api.so",
  BridgeModule: "/absolute/path/to/cna_node_bridge.node",
  // Optional: where TitleContainer resolves title content. Defaults to the working directory,
  // because CNA's own default is the executable's directory -- for Node, where node is installed.
  TitleLocation: "/absolute/path/to/app",
});
```

The adapter enforces the ABI 0.35 window. Every imported symbol has its function-pointer type
checked against the canonical headers under `-Wall -Wextra -Werror`, so a route whose signature
moves is a build failure rather than a runtime surprise. What each artifact configuration supplies
-- a mixer, FFmpeg durations, displays, cameras, compiled effects, CNA's device and graphics
extension layers -- is measured by the suites rather than assumed, and XNA's Reach profile limits
are CNA's and asserted as such. Linux evidence is not a Windows, Electron or mobile support claim.

## CNB, beside XNB

`.cnb` is CNA's own compiled content format. It is not `.xnb` and this package does not pretend
otherwise: XNB is Microsoft's, `ContentManager.Load` reads it, and CNB lives on its own subpath
because it carries asset types XNA never had and containers that are checksummed, versioned and
self-describing in ways XNB is not.

```ts
import { CnbDocument, CreateTexture2DFromCnb } from "cna-ts/extensions/content";

const document = CnbDocument.Parse(bytes, "Textures/Atlas.cnb");
try {
  const atlas = CreateTexture2DFromCnb(GraphicsDevice, document);
} finally {
  document.Dispose();
}
```

A parsed document is a container that is already structurally sound — magic, versions, both
structural checksums, every chunk checksum, alignment, table-of-contents ordering and exact
non-overlapping coverage are all applied before an accessor hands out a byte. What it exposes is a
copied immutable view: the table of contents, the `CMET` metadata, the `XREF` external references
and any chunk's logical bytes. `CreateTexture2DFromCnb` and `CreateSpriteFontFromCnb` turn a
document into ordinary owned XNA resources; `CnbTextureData` and `CnbSpriteFontData` also encode,
so a Node build script can compile content as well as read it.

The three objects that own native memory — the document and the two decoded descriptions — are
explicit `Dispose()`. Everything else is copied, because a small payload is safer as a JavaScript
copy than as a view into memory a `Dispose()` can take away.

XNB framing, reader tables/versions, shared resources, disposal tracking, and custom reader
dispatch are implemented in TypeScript. Windows XNB v5 supports uncompressed streams and the XNA
LZX frame/block wrapper, including persistent multi-frame decoding and exact decompressed-length
validation. External references resolve relative to the referring asset, reuse the normalized
ContentManager cache, detect cycles, and retain ordinary unload ownership; referenced assets may
themselves be compressed or contain shared resources. Consumers register TypeScript custom readers
through `RegisterContentTypeReader` from `cna-ts/extensions`. Raw PNG/JPEG bytes go through
`Texture2D.FromStream`, never `Content.Load`.

## Compatibility scope

The completed strict target is an **XNA 4.0 TypeScript/JavaScript API projection** for the
seven-assembly Windows runtime profile. It is not a claim that C# source compiles unchanged as TypeScript. The
language transformations are normative in
[`docs/xna-typescript-mapping.md`](docs/xna-typescript-mapping.md) and will be measured from the
actual XNA reference assemblies.

## Development

The package is pre-1.0 at version `0.1.0`, uses ESM, requires Node.js 20 or newer for development,
and pins TypeScript 5.9.2.

```bash
npm ci
npm run check
npm test
npm run test:differential
npm run api:report
npm run api:verify
npm run api:inventory
npm run verify:runtime
npm run verify:leaks
npm run runtime:inventory
npm run verify:build-reproducibility
npm run verify:package
npm run verify:package-reproducibility
```

When a CNA source checkout is available, its native contract can be audited without becoming a
package dependency:

```bash
npm run audit:cna-abi -- --cna-root /path/to/cna
```

Native integration is deliberately separate from pure managed tests:

```bash
CNA_SOURCE_PATH=/path/to/cna \
CNA_NATIVE_LIBRARY=/path/to/libcna_c_api.so \
npm run test:native
```

Windowed suites need a display. On a shared workstation run them through CNA's private display
server rather than the desktop:

```bash
/path/to/cna/tools/platform/run_gpu_tests_private.sh --exec env -u WAYLAND_DISPLAY \
  SDL_VIDEODRIVER=x11 SDL_AUDIO_DRIVER=dummy CNA_WINDOWED_LIBRARY=/path/to/libcna_c_api.so \
  npm run test:windowed:required
```

Browser suites run headless Chromium; unset `DISPLAY` so Chromium picks its headless WebGL2 path.

Generated `.js`, `.d.ts`, declaration maps, and source maps are written only to `dist/`. The
legacy `cna-js` package is not a dependency and is being retired.

The sibling `cna-ts-template` is the single maintained project template. Its canonical TypeScript
source generates both strict TypeScript and ordinary JavaScript projects; both are verified against
the exact packed `cna-ts` artifact.

See the [architecture](docs/architecture.md), [runtime capability inventory](docs/runtime-capabilities.md),
[C ABI audit](docs/cna-abi-audit.md), [measured roadmap](plan.md), and
[CNA-JS consolidation assessment](docs/cna-js-consolidation.md).

## License

CNA-TS is licensed under the [Microsoft Public License](LICENSE), matching CNA.
