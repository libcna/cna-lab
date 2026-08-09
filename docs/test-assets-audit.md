# Upstream test-asset provenance audit

## Status and scope

This audit covers every file under `src/Myra.Tests/Assets` at pinned Myra
revision `0d79b939310bfe1d00b21803fe15e291caf60aa1`. The inventory contains 35
files and 906457 bytes. No audited file has been copied into Myra-CNA.

The SHA-256 of the sorted `sha256sum` manifest (relative path and digest, one
file per line) is
`8d6ae480b533b3dc1892fc2ff386db7a9056e5127e9d8c40455df6d58cb7291f`.
The disposition codes used below are:

- **MIT**: Myra-authored text may be translated or copied with the Myra MIT
  notice; a dependency named in the notes must still be replaced first.
- **licensed**: third-party terms and an authoritative source were identified;
  the required complete notice must accompany any future copy.
- **defer**: the file belongs to the VisUI-derived default-skin decision in
  `docs/default-assets-audit.md` and must not be copied yet.
- **replace**: do not copy the upstream file; use a project-owned or separately
  provenance-cleared test fixture with equivalent behavioral coverage.

## Pinned inventory and disposition

| Myra test asset | Bytes | SHA-256 | Disposition |
| --- | ---: | --- | --- |
| `GridTests/SimpleAutoFill.xmmp` | 573 | `fc130422a3e0c90785c7f36668dc60558e65854ea9cf4e574a7acbea037de4c9` | MIT |
| `GridTests/SimpleProportionsPart.xmmp` | 487 | `524515190534c3987a3cc97a33d563b45815eb0b2e72f572bdfd5bc2780f243d` | MIT |
| `GridWithExternalResources.xmmp` | 618 | `e964c19709ce39e642590bd0a2d238eb7ddf690c5c86bc7d25ab6e79b4ac86f7` | MIT; rewrite logo/font paths |
| `MonoGameLogo.png` | 23520 | `33da15a2ef96c0da0ac4e6990d0a406d52141c41a663cc5474f56f999ddb33f1` | replace |
| `Stylesheets/Commodore64/commodore-64.fnt` | 10759 | `a1ca80f08751745bf07fba07b409cb6cf405cc3c274bc70b5d26184ea42ae6a6` | replace |
| `Stylesheets/Commodore64/ui_stylesheet.xmat` | 3842 | `a726358b5d5363d67242a9e2f77ba4a15746d6173e4fad698a26b041544f520a` | replace bundle; XML structure is reference-only |
| `Stylesheets/Commodore64/ui_stylesheet.xmms` | 2809 | `c2ed017061ffb0c02c95cf0fd362f3a03323be0b5a18d41c2f3d79f31b83573c` | replace bundle; XML structure is reference-only |
| `Stylesheets/Commodore64/ui_stylesheet_atlas.png` | 5964 | `418262643ad828ee8e8798d301ad00e0e01b31172abc97779d830e5d1d01139f` | replace |
| `Stylesheets/Default/DroidSans.ttf` | 190044 | `f542e056fcd118dbb16c0a3b3ae8bb19e6e16ce0a22caddf772f5d79353cbd5d` | replace |
| `Stylesheets/Default/Inter-Regular.ttf` | 287928 | `1041a8cf17dab7579acef0cc46b21f6497ec1ae01918ddc3495416efb81a4780` | licensed, OFL-1.1; packaging still gated |
| `Stylesheets/Default/default_ui_skin.png` | 71734 | `479266d3d26a3b28fd6fd43b25923f97a6c557e78c21258b7595686337485ff9` | defer, P0-015b |
| `Stylesheets/Default/default_ui_skin.xmat` | 10768 | `c1d413600d0a3b3d8bcb7f4bcca2cdcad25dec83fe0fc60f21383187e75ec120` | defer, P0-015b |
| `Stylesheets/Default/default_ui_skin.xmms` | 9446 | `1d73e3773c9b413798af359b1e979017a3298b147cb936b4c6b15305e951b817` | defer, P0-015b |
| `Stylesheets/Default/default_ui_skin_2x.png` | 119004 | `cb2722530c9c0f5f329c0970aef6756ec5e71efab09c55ede80ac0881cdbcfb0` | defer, P0-015b |
| `Stylesheets/Default/default_ui_skin_2x.xmat` | 10866 | `a0d56879af901eeb8dbcac20de8c8e4c06ec6cc7ccd6a12b01d1166b93cdfc5d` | defer, P0-015b |
| `Stylesheets/Default/default_ui_skin_2x.xmms` | 9294 | `1869ac3b4a9373336718f0b514c6a181c7ba679c1266333924fcf62d34e3f8f1` | defer, P0-015b |
| `Stylesheets/Default/default_ui_skin_another_font.xmms` | 8560 | `41a6338a07c35de46ef7dba5d4baffcea741254af9cb614ab325f7f01cfef741` | MIT; rewrite skin/font paths |
| `Stylesheets/Default/default_ui_skin_no_existing_texture.xmms` | 8470 | `6acb637c0fef237adc642c5b97cddbba26a9c62bc7f5abfa596005c4db8d8e2c` | MIT; rewrite skin paths |
| `Stylesheets/LibGDX/ui_font.fnt` | 11542 | `b86cbe859847599c7a40a60362b32f6ac8b3d603f39a75265cb03959848c5baf` | licensed, Apache-2.0 + Myra MIT conversion |
| `Stylesheets/LibGDX/ui_stylesheet.xmat` | 3373 | `2d005aacd960fe2f78d7b66c02e2dcd38faa9fa220162ad2cdff222e2142e640` | licensed, Apache-2.0 + Myra MIT conversion |
| `Stylesheets/LibGDX/ui_stylesheet.xmms` | 4189 | `1595dc55d95442ea4ad39bf5d8c5d8c29403b5fdc54f7dcddf92d065afe104e5` | licensed, Apache-2.0 + Myra MIT conversion |
| `Stylesheets/LibGDX/ui_stylesheet_atlas.png` | 22779 | `1fe38f06b63a465e18db1042ff0c2c0242ce4c27e5c6af7fd01030223d3500cc` | licensed, Apache-2.0 |
| `allControls.xmmp` | 6382 | `d543f64a9a25ef7f662efe2efc12206be043eb1293a311849caf209c7cbeb559` | MIT |
| `allControlsBasic.xmmp` | 2989 | `247d43bb526928e6b266fed40687dd3d416ccb46d342e2ae379cb51320ffc1d8` | MIT; rewrite stylesheet path |
| `allControlsC64.xmmp` | 3035 | `4b80e6fee213c3b338f4ca9b99300579648fcf47c66b9439cade2070834332a5` | MIT; rewrite stylesheet path |
| `arial64.fnt` | 25661 | `bfd25146e5998dac81193179d59ab12de70f97a9c09996e87e1d61d4ea01860c` | replace |
| `arial64_0.png` | 49070 | `da6c55ebd4a78dfbf686ed4ddeacab26b69bd434c7e55225c9a775dd12080a5a` | replace |
| `checkButton.xmmp` | 364 | `bddf73f33272b9f38215efd9785863dd8fec76d22d6128846ef19dcb5be028fb` | MIT |
| `comboView.xmmp` | 467 | `c178ecc1966c2a9347b452a9995bae2d28a975c19d8be9073ca07866fc3d6ea2` | MIT; rewrite logo path |
| `labelWithPaddings.xmmp` | 182 | `68dc2d3ad97f9ec1ba9574fd89cdf7951806b5543c16975d5bcb0c2f2a555df3` | MIT |
| `listView.xmmp` | 563 | `2e5bfb7962709d8612eae917c31c25447d5493d323f6536d0299cebf8531fe7a` | MIT |
| `marginBorderPadding.xmmp` | 279 | `83bef97f0a4103991171c3386cf56c851218d31c0f4903187f6353ec09336437` | MIT |
| `newButtons.xmmp` | 739 | `1e28b6fe6400b28e2c7b4b7723635e52b5f9be3565290c920342a02f0b5fd9e2` | MIT |
| `scrolledTextField.xmmp` | 153 | `e7e52861d4b90a8ea39b3708a87af374841da9d9153ef0b2ad527c678fec268f` | MIT |
| `test.txt` | 4 | `532eaabd9574880dbf76b9b8cc00832c20a6ec113d682299550d7a6e0f345e25` | MIT |

## Findings by lineage

### Myra-authored text fixtures

The 14 general `.xmmp`/text fixtures are Myra-authored test inputs under the
repository's MIT licence. The two 2026 default-stylesheet variants are likewise
Myra-authored XML. They may be translated or copied with the Myra notice, but
references to blocked/replaced binaries must be changed before use. In
particular, `GridWithExternalResources.xmmp` names both `MonoGameLogo.png` and
`arial64.fnt`, `comboView.xmmp` names the logo, and the stylesheet-dependent
forms name upstream visual bundles.

### Default stylesheet files

The six core skin files are exact duplicates of the production resources
already audited in `docs/default-assets-audit.md`; their hashes match that
audit. They remain blocked by `needs_human` P0-015b. The two additional XMMS
files test existing-texture behavior and an alternate font. Their XML is
Myra-authored, but it must point at the eventual replacement skin and alternate
font fixture.

The test copy of `Inter-Regular.ttf` is also byte-identical to the production
resource. Its Inter 3.012/OFL-1.1 provenance and complete notice are already
recorded. It remains unbundled until the P3-004/P3-020 font decisions.

`DroidSans.ttf` was introduced in Myra commit
`57590c41b4bb2aaefecb90f2a799d526a738222b` for the alternate-font test. Its
name table says Droid Sans Regular, `Version 1.00 build 112`, copyright 2007
Google Corporation/Ascender Corporation, and Apache-2.0. The official AOSP
`froyo/data/fonts/DroidSans.ttf` is also 190044 bytes but is build 113, has
SHA-256 `4e2371bc0e4cf6983342e150412f140da79d674c9be0b56458401f581072ecd3`,
and differs at 33 byte positions. The exact build-112 source and accompanying
upstream notice were not found in the pinned Myra history. Because the test
only needs a second font, use the selected project's provenance-cleared font
fixture instead of copying this binary.

### libGDX stylesheet

Myra introduced this sample lineage in commit
`25f9ffbefda8ef7c563e9a8c89d82f437fc9a930`. The pinned atlas PNG is
byte-identical to libGDX's `tests/gdx-tests-android/assets/data/uiskin.png` at
commit `134da9bee14bd91517660b6e96c0bdf4a8a5d5f4`; both have SHA-256
`1fe38f06b63a465e18db1042ff0c2c0242ce4c27e5c6af7fd01030223d3500cc`.
Myra's original atlas descriptor is likewise byte-identical to that revision's
`uiskin.atlas` after acquisition (SHA-256
`772ac026559d7d984c6aaa7f07219b96d30364157f162a50315486118b1e4cb3`).

After normalizing line endings, the current `ui_font.fnt` differs from that
revision's `default.fnt` only in its page reference: Myra points glyphs into
`ui_stylesheet.xmat:default`. The XMAT/XMMS files are Myra's converted
representation of the same libGDX skin. The four-file bundle may be used under
libGDX's Apache-2.0 terms plus Myra's MIT attribution, but the complete Apache
licence/notice must be added before any file is copied.

### Assets that must be replaced

`MonoGameLogo.png` is the official 64x64 logo. Myra copied it into the test
project in commit `b66483e8305bba5b46275a895712f6ca238b2698`; identical copies remain
in its samples. MonoGame's official logo guidance says the registered logo is
not part of MonoGame's Ms-PL source licence, restricts it to secondary-brand
credit/link uses, requires it to remain unaltered, and requires explicit
permission for commercial products outside game credits. The tests exercise
only PNG loading, dimensions, and tint wrapping. Replace it with an original
64x64 image fixture rather than redistributing the mark.

`arial64.fnt` declares face `Arial`, and `arial64_0.png` contains the generated
glyph raster. Myra introduced the pair as a sample in commit
`4d466c0cb26837e223ceec7149597974968df549`. No Arial redistribution licence
accompanies it. Replace both with a small project-owned or clearly licensed
BMFont fixture; preserve only the parser, page-reference, size, and measurement
cases that the tests actually need.

The Commodore64 descriptor declares `Commodore 64 Pixelized`; Myra introduced
the sample bundle in commit `f43bfd6d5836dc364cc960d977b90d0c9006e8e6`.
The pinned repository contains no font/artwork licence. Third-party catalogue
labels such as “100% Free” are not an authoritative, preserved grant for this
exact generated bitmap and atlas. Replace the four-file visual/font bundle
with an original style fixture. Its XMMS structure may inform behavioral tests,
but exact pixels, glyphs, and branding must not be copied.

## Implementation gate

P0-016's inventory/provenance audit is complete. Before the corresponding
asset-loading and stylesheet tests are ported:

1. create an original 64x64 PNG with deterministic pixel assertions;
2. create or select a clearly licensed minimal BMFont descriptor/atlas;
3. create an original stylesheet atlas for cases currently using C64 or the
   blocked default skin;
4. rewrite the MIT XML fixtures to those paths and record all new assets in
   `UPSTREAM_MANIFEST.md` as project-owned replacements;
5. append the complete Apache-2.0 licence and attribution if the libGDX bundle
   is ultimately copied rather than replaced.

These replacements preserve test behavior without making a public API or
architecture decision. They are tracked as P0-016a, not as `needs_human`.

## Reproduction notes

The inventory digest can be reproduced from a clean pinned checkout with:

```bash
cd src/Myra.Tests/Assets
find . -type f -print0 | sort -z | xargs -0 sha256sum \
  | sed 's#  \./#  #' | sha256sum
```

History was checked with `git log --follow` and `git show`. Binary comparisons
used `sha256sum`, `cmp`, embedded TTF name-table strings, and exact files from
the pinned AOSP and libGDX revisions. Temporary downloads are not project
inputs.
