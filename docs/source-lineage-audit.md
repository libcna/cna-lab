# Upstream production-source lineage audit

Status: complete for P0-014 at Myra revision
[`0d79b939310bfe1d00b21803fe15e291caf60aa1`](https://github.com/MyraUI/Myra/commit/0d79b939310bfe1d00b21803fe15e291caf60aa1).
This is an engineering provenance audit, not legal advice.

## Scope and method

The audit enumerated every `*.cs` file below the pinned `src/Myra` tree,
reviewed all source comments containing copyright, licence, borrowing, origin,
author, or URL markers, inspected the three licence files shipped at the
repository root, and checked the FNA project selection. The sorted relative-path
inventory contains 189 files and has SHA-256:

```text
11bfaad70b94bce29929ed31cd064992ef5479a33ab43eb82b1b873ab98c1850
```

The reproducible inventory command is:

```bash
LC_ALL=C find src/Myra -type f -name '*.cs' -printf '%P\n' |
  LC_ALL=C sort
```

The default SDK compile glob selects 182 of the 189 files for FNA after
`Myra.FNA.Core.csproj` removes all seven `Platform/**` files. One of those 182,
`MyraAssetManagerExtensions.PlatformAgnostic.cs`, contributes no code because
its entire body is guarded by `PLATFORM_AGNOSTIC`; see
`docs/font-audit.md` and P3-018.

Every file not listed in the exception table below is classified as Myra-team
source under the repository's MIT licence. This default class covers 179 files.
Imports or calls into FNA/XNA, FontStashSharp, XNAssets, and framework APIs are
dependency relationships, not evidence that their implementation was copied;
those dependencies are tracked separately.

### Inventory cross-check by path group

| Path group | Files |
| --- | ---: |
| `Attributes` | 8 |
| `Events` | 9 |
| `Graphics2D/UI` | 121 |
| `Graphics2D/TextureAtlases` | 5 |
| other `Graphics2D` files/groups | 7 |
| `MML` | 8 |
| `Utility` | 13 |
| `Platform` | 7 |
| `TextCopy` | 5 |
| root-level production files plus `Properties` | 6 |
| **Total** | **189** |

The three `*.Generated.cs` files identify MyraPad itself as their generator:

- `Graphics2D/UI/ColorPicker/ColorPickerPanel.Generated.cs`;
- `Graphics2D/UI/DebugOptionsWindow.Generated.cs`; and
- `Graphics2D/UI/File/FileDialog.Generated.cs`.

They remain Myra-generated source under Myra's MIT licence. Their eventual C++
translations must still be reviewed and manifested like handwritten source.

## Non-Myra source lineage

| Upstream file(s) | Evidence and licence | FNA status | Myra-CNA treatment |
| --- | --- | --- | --- |
| `Utility/InputExtension.cs` | File says it was borrowed from MonoGame.Extended; bundled `LICENSE.MonoGame.Extended.txt` is MIT, copyright 2015 Dylan Wilson | Active | Ported with dual attribution and the full notice |
| `Graphics2D/RenderContext.Shapes.cs` | Same explicit MonoGame.Extended statement and bundled MIT notice | Active | Ported with dual attribution and the full notice |
| `Graphics2D/Transform.cs` | The transform builder says it was borrowed from MonoGame `SpriteBatch.DrawString` | Active | Algorithm ported with MonoGame attribution and the complete Microsoft Public License notice; exact historical MonoGame source revision remains a release-audit item |
| `Utility/CurrentPlatform.cs` | File carries the MonoGame Team copyright header and refers to MonoGame's `LICENSE.txt` | Active | Do not translate: this internal helper is replaced by CNA `getCurrentPlatform()`/`getCurrentDesktopOS()` for FileDialog, while TextBox uses CNA Clipboard; exact historical MonoGame revision is therefore not introduced into Myra-CNA |
| `Platform/Keys.cs` | Same explicit MonoGame Team header | Excluded by `Myra.FNA.Core.csproj` | Do not translate: the selected build excludes it and active FNA code already uses FNA/XNA keys, mapped directly to CNA keys in Myra-CNA |
| `TextCopy/BashRunner.cs`, `Clipboard.cs`, `LinuxClipboard.cs`, `OsxClipboard.cs`, `WindowsClipboard.cs` | Repository ships `License.TextCopy.txt`: MIT, copyright 2018 Simon Cropp | Active | Preserve the notice, but P6-026 replaces native/process clipboard implementations with CNA's UTF-8 Clipboard API; do not translate these five files |

No other source-level copyright, borrowed-code statement, origin URL, or
separate bundled source licence was found in the 189-file inventory.

The official [MonoGame repository](https://github.com/MonoGame/MonoGame)
describes the project as Microsoft Public License except for separately noted
portions. The exact historical MonoGame revision used by Myra was not recorded
in the pinned Myra repository, so this audit does not invent one.

## External API/dependency boundary

Seventy-four files import `Microsoft.Xna.Framework`; Myra-CNA maps their value,
graphics, and input API usage to CNA. That is an API compatibility relationship,
not permission to copy MonoGame/FNA implementation. The three explicit
MonoGame-derived cases above are the only source-lineage exceptions found.

Forty-nine files mention FontStashSharp. Their active API surface, exact package
revisions, zlib notice, and implementation options are covered by
`docs/font-audit.md`; P3-004 remains an explicit incorporation gate.

XNAssets is referenced by the FNA project and provides the active asset loader,
but no XNAssets implementation has been copied. Its audited 0.8.5 source is MIT;
any future translation of loader behavior must add a file-level manifest row
and preserve that notice before incorporation.

The root licence set is complete and internally consistent with the source
markers found by this audit:

- `LICENSE.txt`: Myra Team MIT;
- `LICENSE.MonoGame.Extended.txt`: MonoGame.Extended MIT; and
- `License.TextCopy.txt`: TextCopy MIT.

Myra does not bundle a separate MonoGame `LICENSE.txt` for the two MonoGame-
header files or the `SpriteBatch.DrawString` algorithm. Myra-CNA therefore
retains the complete Ms-PL notice already recorded in
`THIRD_PARTY_NOTICES.md`, while avoiding translation of the two whole copied
files.

## Porting rules resulting from the audit

1. Every ordinary translation carries Myra MIT provenance and an exact manifest
   row.
2. `InputExtension` and `RenderContext.Shapes` additionally carry the
   MonoGame.Extended MIT attribution already present in the repository.
3. `Transform` retains its MonoGame `SpriteBatch.DrawString` attribution and
   Ms-PL notice; do not claim an unverified historical revision.
4. Do not create Myra-CNA copies of `Utility/CurrentPlatform.cs`,
   `Platform/Keys.cs`, or `TextCopy/**`; use the selected CNA APIs documented in
   the manifest.
5. Treat a generated MyraPad file as reviewed source, not as disposable
   generated output, because its generator is not part of this build.
6. Continue to gate FontStashSharp, default assets/fonts, and test assets on
   their separate P3/P0 audits. This audit does not approve any asset.

P0-015 and P0-016 remain open because asset and font-file redistribution cannot
be inferred from production-source licensing.
