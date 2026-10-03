# Font and rich-text dependency audit

Status: completed engineering/provenance audit; implementation remains gated on
P3-004. This document is not legal advice.

No font source, binary, asset, package, submodule, or new dependency was added
to Myra-CNA as part of this audit. The recommendation below is deliberately
non-binding until a maintainer makes the dependency decision recorded in
P3-004.

## Pinned source set

The selected Myra source is revision
[`0d79b939310bfe1d00b21803fe15e291caf60aa1`](https://github.com/MyraUI/Myra/commit/0d79b939310bfe1d00b21803fe15e291caf60aa1).
Its `Directory.Build.props` declares Myra 1.6.5, FontStashSharp 1.5.6, and
XNAssets 0.8.5.

| Component | Audited revision | Declared licence | Relevance |
| --- | --- | --- | --- |
| FontStashSharp | [tag 1.5.6, commit `24f3dc46d59dcda0dddb59754a99aeaefc9bd369`](https://github.com/FontStashSharp/FontStashSharp/tree/1.5.6) | [zlib](https://github.com/FontStashSharp/FontStashSharp/blob/1.5.6/LICENSE.txt) | Myra's public font and rich-text surface |
| FontStashSharp.Base | [1.2.3 commit `670ecb8c7a323fcce6d9c176fa3692b4dd28b959`](https://github.com/FontStashSharp/FontStashSharp.Base/commit/670ecb8c7a323fcce6d9c176fa3692b4dd28b959) | zlib | Atlas, glyph, layout, and rasterizer abstractions used by FontStashSharp 1.5.6 |
| XNAssets | [0.8.5 commit `928a178da1b3fd83a10536a455390f8d0ff7477b`](https://github.com/rds1983/XNAssets/commit/928a178da1b3fd83a10536a455390f8d0ff7477b) | MIT | Active FNA asset-loader integration used by Myra |
| StbTrueTypeSharp | 1.26.12 through `FontStashSharp.Rasterizers.StbTrueTypeSharp` 1.2.3 | Public-domain declaration by the wrapper; upstream `stb_truetype.h` offers MIT or public domain | Selected .NET rasterizer path; not yet selected for C++ |
| StbImageSharp | 2.30.15 | Public-domain declaration | Transitive FontStashSharp FNA texture-loading dependency; not needed if CNA owns atlas textures |
| Cyotek.Drawing.BitmapFont | 2.0.4 | MIT | Static BMFont support in the selected .NET package graph |

The exact NuGet package records independently confirm the FontStashSharp
[1.5.6](https://www.nuget.org/packages/FontStashSharp/1.5.6),
[Base 1.2.3](https://www.nuget.org/packages/FontStashSharp.Base/1.2.3), and
[StbTrueTypeSharp rasterizer 1.2.3](https://www.nuget.org/packages/FontStashSharp.Rasterizers.StbTrueTypeSharp/1.2.3)
versions and dependency declarations.

## Active Myra/FNA source boundary

The pinned `Myra.FNA.Core.csproj` removes the seven files under `Platform/**`.
Myra-CNA therefore must not port FontStashSharp's platform-agnostic renderer
interfaces merely because those files exist in the repository.

`src/Myra/MyraAssetManagerExtensions.PlatformAgnostic.cs` is physically visible
to the FNA project, but all of its code is guarded by `#if PLATFORM_AGNOSTIC`.
The FNA project defines `FNA`, not `PLATFORM_AGNOSTIC`, so the file contributes
no active API. The active dynamic-font loader comes from XNAssets 0.8.5. This is
recorded as an intentionally inactive source in `UPSTREAM_MANIFEST.md`; no
parallel platform-agnostic C++ API should be created.

The `FSColor` aliases similarly belong to non-FNA conditional branches. The
selected FNA build uses XNA `Color`, which maps directly to CNA `Color` in this
port.

## Direct FontStashSharp API inventory

The following inventory comes from all production call sites under the pinned
Myra `src/Myra` tree. It describes the compatibility surface, not a proposed
one-to-one C++ namespace design.

| Type | Members used directly by active Myra/FNA code |
| --- | --- |
| `SpriteFontBase` | `GetWhite(GraphicsDevice)`, `DrawText(...)`, `LineHeight`, and read/write `Name`; used as the public font type throughout labels, text boxes, styles, windows, menus, MML, and the property grid |
| `StaticSpriteFont` | `FromBMFont(string, Func<string, TextureWithOffset>)` |
| `TextureWithOffset` | construction from a texture and point offset |
| `DynamicSpriteFont` | `FontSystem` |
| `FontSystem` | direct `GetFont(float)`; active XNAssets integration constructs a system and calls `AddFont(byte[])` |
| `FontSystemSettings` | construction plus `ExistingTexture` and `ExistingTextureUsedSpace` in the active XNAssets loader |
| `RichTextLayout` | construction; `Font`, `Text`, `VerticalSpacing`, `Width`, `Height`, `AutoEllipsisMethod`, `AutoEllipsisString`, `Lines`, `Size`, `CalculateGlyphs`, `SupportsCommands`, and `IgnoreColorCommand`; `Measure(int?)`, cursor/line/glyph queries, and the FNA `Draw(...)` overload |
| `TextHorizontalAlignment` | `Left`, `Center`, and `Right` |
| `AutoEllipsisMethod` | `None` is referenced directly; `Character` and `Word` remain values of a public Myra-facing property |
| `TextLine` | `LineIndex`, `Count`, `Size`, `TextStartIndex`, `Chunks`, `GetGlyphInfoByIndex`, and `GetGlyphIndexByX` |
| `TextChunk` | runtime type checks plus inherited `LineIndex`, `Size`, and `Glyphs` |
| `BaseChunk` | indirectly exposed by heterogeneous `TextLine.Chunks` |
| `TextChunkGlyph` | `Codepoint`, `Bounds`, `XAdvance`, `LineTop`, and `TextChunk` |
| `ColorStorage` | `FromName`, `GetColorName`, and `ToHexString`; this is font-independent behavior currently gating MML color strings and two graphics `ToString()` methods |

`IFontStashRenderer`, `IFontStashRenderer2`, FontStashSharp's
`VertexPositionColorTexture`, and the platform-agnostic settings/renderer calls
are not part of the selected FNA surface. Glyph drawing in Myra-CNA should flow
through CNA `SpriteBatch` and the existing `RenderContext`.

## Required behavioral closure

Implementing only the directly named symbols is insufficient. Their observable
behavior closes over:

- dynamic font loading, glyph lookup/rasterization, kerning, metrics, fallback,
  atlas allocation, atlas growth, and cache lifetime;
- rich-text command parsing, line breaking, wrapping, ellipsis, horizontal
  alignment, measurement, glyph rectangles, and cursor hit testing;
- heterogeneous text/image/space chunks and text effects/styles where Myra
  exposes them;
- the active XNAssets existing-atlas fields and multi-font loading behavior;
- deterministic CNA texture ownership and device-loss handling; and
- static AngelCode BMFont loading for `StaticSpriteFont` compatibility.

The selected FontStashSharp rich-text command set is `c`, `e`, `f`, `i`, `s`,
`t`, and `v` (color, effect, font, image, space, text style, and vertical
offset), together with newline and escaped-command handling. Some resolvers are
optional and may be null. Myra creates a default `RichTextLayout`, catches
layout exceptions in `Label`, and exposes them as an `RTL Error:` string; error
behavior is therefore observable.

FontStashSharp supports an optional text shaper, but its default is null and
the pinned Myra FNA code does not enable HarfBuzz. Initial compatibility should
preserve that selected-upstream level. Adding HarfBuzz for complex shaping is a
separate capability and dependency decision, not a prerequisite to claim the
default Myra path.

### Unicode/indexing finding

FontStashSharp 1.5.6 is internally mixed:

- `TextSource` combines UTF-16 surrogate pairs and counts Unicode code points;
- layout traversal and substring positions still use raw UTF-16 code-unit
  indices; and
- `TextLine.TextStartIndex` can consequently be a UTF-16 index while
  `TextLine.Count` counts code points.

Myra's C# `TextBox` also works with UTF-16 string indices. A C++ UTF-8 port
cannot silently label all of these values as byte, code-point, or grapheme
indices. P3-006 must define an explicit conversion/mapping policy and add
astral-plane tests. Correcting the API to grapheme semantics would be a public
behavior change and requires an explicit compatibility decision.

## Alternatives

| Option | What it supplies | Compatibility/build implications | Audit conclusion |
| --- | --- | --- | --- |
| Required-subset FontStashSharp translation plus direct `stb_truetype.h` | Closest layout, atlas, measure, ellipsis, and cursor behavior; small native rasterizer | Translation and maintenance cost; zlib attribution for translated FontStashSharp behavior; choose the stb MIT branch explicitly; upstream stb warns that untrusted fonts are not range checked | Best current fidelity candidate, subject to P3-004 and a trusted-font policy |
| Required-subset FontStashSharp translation plus FreeType | Same rich-layout translation with a mature native rasterizer | More native build/package surface and possible metric/rasterization drift; FreeType is only a glyph engine and does not replace rich layout | Reasonable alternative when untrusted-input robustness or raster quality outweighs dependency cost |
| FreeType plus HarfBuzz | Rasterization plus complex-script shaping | Still requires Myra/FontStashSharp-compatible rich layout, atlas, hit testing, and ellipsis; shaping can change selected-upstream results | Useful optional later layer, not a complete substitute |
| SDL_ttf | SDL wrapper around FreeType/HarfBuzz with a zlib licence | SDL-specific while CNA has multiple backends; still lacks Myra rich-layout/atlas semantics | Reject as the backend-neutral core |
| CNA `SpriteFont` | Existing CNA measurement/drawing of prebuilt XNB/CNJ fonts | Static/prebuilt assets; no runtime TTF/OTF or FontStashSharp rich layout | Reuse behind the abstraction where behavior matches; not a replacement |
| cna-extended `BitmapFont` | Existing `.fnt` parsing, kerning, measurement, and drawing | Static bitmap font only; no runtime TTF/OTF or rich layout | Useful adapter and test oracle; not a replacement |
| New independent layout engine plus stb | Avoids translating FontStashSharp layout internals | Highest parity risk while retaining the same rasterizer/security decision | Inferior to a scoped compatibility translation for this port |

Authoritative alternative references:

- [FreeType licence](https://freetype.org/license.html) and
  [overview](https://freetype.org/freetype2/docs/index.html): the FreeType
  License or GPLv2, and a glyph font engine rather than a high-level text-layout
  system.
- [HarfBuzz](https://github.com/harfbuzz/harfbuzz) is a shaping engine under its
  bundled permissive licence notices.
- [SDL_ttf](https://wiki.libsdl.org/SDL3_ttf/FrontPage) is zlib-licensed and
  wraps FreeType and HarfBuzz (with their own notices).
- [`stb_truetype.h` 1.26](https://github.com/nothings/stb/blob/master/stb_truetype.h)
  offers MIT or public-domain terms and prominently warns that it performs no
  range checking for untrusted font data.

No usable project-declared FreeType, HarfBuzz, or SDL_ttf dependency was found
in Myra-CNA/CNA, and none of their pkg-config packages was available as a
portable baseline on the audit host. A system library discovered incidentally
inside one backend build cache is not a project dependency guarantee.

## Recommendation and implementation gate

For highest selected-upstream fidelity, the current engineering recommendation
is:

1. define a Myra-owned narrow text abstraction whose public behavior matches
   the API inventory above;
2. translate only the required FontStashSharp 1.5.6/Base 1.2.3 layout, atlas,
   and `ColorStorage` behavior under their zlib terms;
3. draw through CNA `SpriteBatch`, without the inactive platform renderer API;
4. use CNA/cna-extended static-font facilities only as compatible adapters or
   test oracles; and
5. if approved, use the upstream C `stb_truetype.h` 1.26 under its MIT option
   for an initial trusted-font-only rasterizer, leaving HarfBuzz as a later
   separately gated extension.

Before P3-005 or any third-party incorporation begins, P3-004 requires a human
to decide all of the following:

- approve the required-subset FontStashSharp translation and its zlib notice,
  or select a different layout strategy;
- approve direct `stb_truetype.h` under MIT, select FreeType, or select another
  rasterizer;
- decide whether initial font inputs are restricted to trusted packaged/project
  assets in light of stb's untrusted-input warning; and
- keep optional HarfBuzz shaping out of the initial selected-upstream-compatible
  scope (recommended), or approve it as an additional dependency/capability.

After that decision, the next implementation step is P3-005: design the narrow
Myra-CNA text interface and an index-domain contract, then pin the approved
sources and notices before translating code.
