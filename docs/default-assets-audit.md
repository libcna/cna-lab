# Default asset provenance audit

## Status and scope

This audit covers the seven files selected from `src/Myra/Resources` by the
pinned Myra FNA build:

- `Inter-Regular.ttf`;
- `default_ui_skin.png`, `.xmat`, and `.xmms`;
- `default_ui_skin_2x.png`, `.xmat`, and `.xmms`.

The source baseline is Myra commit
`0d79b939310bfe1d00b21803fe15e291caf60aa1`. No file audited here has been
copied into Myra-CNA. The existing C++ `DefaultAssets` implementation creates
only its own 1x1 opaque-white texture and is not derived from these resources.

## Pinned inventory

| Myra resource | Bytes | SHA-256 |
| --- | ---: | --- |
| `Inter-Regular.ttf` | 287928 | `1041a8cf17dab7579acef0cc46b21f6497ec1ae01918ddc3495416efb81a4780` |
| `default_ui_skin.png` | 71734 | `479266d3d26a3b28fd6fd43b25923f97a6c557e78c21258b7595686337485ff9` |
| `default_ui_skin.xmat` | 10768 | `c1d413600d0a3b3d8bcb7f4bcca2cdcad25dec83fe0fc60f21383187e75ec120` |
| `default_ui_skin.xmms` | 9446 | `1d73e3773c9b413798af359b1e979017a3298b147cb936b4c6b15305e951b817` |
| `default_ui_skin_2x.png` | 119004 | `cb2722530c9c0f5f329c0970aef6756ec5e71efab09c55ede80ac0881cdbcfb0` |
| `default_ui_skin_2x.xmat` | 10866 | `a0d56879af901eeb8dbcac20de8c8e4c06ec6cc7ccd6a12b01d1166b93cdfc5d` |
| `default_ui_skin_2x.xmms` | 9294 | `1869ac3b4a9373336718f0b514c6a181c7ba679c1266333924fcf62d34e3f8f1` |

## Inter-Regular.ttf

Myra introduced this exact 287928-byte file in commit
[`882984379aff4656762ead70d9fca8a33589bf2a`](https://github.com/MyraUI/Myra/commit/882984379aff4656762ead70d9fca8a33589bf2a)
on 2021-07-14 with the message `Changed default font to Inter`. Its embedded
name table identifies:

- family/style/full name: `Inter`, `Regular`, `Inter Regular`;
- version: `3.012`;
- build source: `git-06b166889`;
- copyright: `Copyright 2020 The Inter Project Authors
  (https://github.com/rsms/inter)`;
- licence: SIL Open Font License 1.1, with the upstream OFL URL.

The abbreviated build identifier resolves to official Inter commit
[`06b166889e335a2454c0767734a05b27f6403098`](https://github.com/rsms/inter/commit/06b166889e335a2454c0767734a05b27f6403098).
That source revision contains the complete SIL OFL 1.1 notice now preserved in
`THIRD_PARTY_NOTICES.md`. The repository tree at that commit does not contain
this generated TTF: it contains a 244440-byte `docs/font-files/Inter-Regular.otf`.
The later official v3.12 release's hinted `Inter-Regular.ttf` is also a distinct
385020-byte binary. The embedded source commit and licence are therefore exact,
but the particular build toolchain/output cannot be byte-matched to a tracked
official binary.

Conclusion: the unmodified Myra TTF may be redistributed under OFL 1.1 when
the copyright and complete licence accompany it. It must remain under OFL and
must not be relicensed as Myra-CNA MIT. No font is copied yet because font
packaging still depends on the P3-004 implementation decision and P3-020.

## Default skin

### Myra history and acknowledgement

Myra's README explicitly says that its default skin was borrowed from
[VisUI](https://github.com/kotcrab/vis-ui). The 1x raw asset set was introduced
in Myra commit
[`e3ef6c861fbd2f0277ec4c4e1f3935f855759394`](https://github.com/MyraUI/Myra/commit/e3ef6c861fbd2f0277ec4c4e1f3935f855759394)
on 2020-05-19. Later Myra commits repacked and updated the generated atlas.

The pinned Myra raw raster directories were compared by relative filename and
bytes against VisUI commit
[`4d5ce13b3bdef8c07a804c6a18e631fd3a64bc27`](https://github.com/kotcrab/vis-ui/commit/4d5ce13b3bdef8c07a804c6a18e631fd3a64bc27):

| Raw set | Myra files | Byte-identical | Different | Missing from VisUI comparison |
| --- | ---: | ---: | ---: | ---: |
| 1x (`assets-raw/1x` vs `ui/assets-raw/x1`) | 105 | 101 | `white.png` | 3 color-picker images |
| 2x (`assets-raw/2x` vs `ui/assets-raw/x2`) | 102 | 101 | `white.png` | 0 |

Thus 202 of the 207 Myra raster sources are byte-identical to the compared
VisUI files. Myra's packed PNG/XMAT resources are generated from these named
sources; byte differences in the final atlas do not establish independent
artwork.

### Upstream terms found

The inspected VisUI root is Apache-2.0 licensed. Its `ui/NOTICE` additionally
states that VisUI uses icons described as CC BY-ND 3.0 and points to
`ui/icons-license`, originating from Templarian/WindowsIcons. That file states,
among other conditions and explanations, `No Attribution and No Derived Works`,
requires an open-source project to include the licence file, lists particular
contributors/icons with attribution requirements, and says not to distribute
the entire icon package without first emailing the author.

The selected Myra repository includes its own MIT, MonoGame.Extended, and
TextCopy notices, but does not include VisUI's Apache licence, `NOTICE`, or
`icons-license` alongside the skin. This audit does not attempt to reinterpret
the interaction among those terms or declare that the packed atlas is safe to
redistribute.

### Decision boundary

The stylesheet XML expresses Myra behavior and can be independently ported as
MIT-licensed Myra source, but its current resource references cannot produce
the upstream appearance without the atlas. Treat the normal and 2x PNG/XMAT
bundles, and any direct copies of their raw visual sources, as blocked.

Before P3-020 can package a default skin, a human must choose one of these
paths:

1. obtain/record an authoritative permission or legal compliance decision for
   the VisUI-derived atlas, then preserve every applicable Apache/NOTICE/icon
   term; or
2. replace the atlas with original or clearly licensed artwork, document every
   source, and keep compatible region names/geometry where practical.

The second path avoids inheriting ambiguous per-icon restrictions and is the
safer engineering recommendation. This is recorded as `needs_human` P0-015b.

## Reproduction notes

The audit used read-only temporary checkouts and these checks:

```bash
sha256sum /tmp/myra-upstream/src/Myra/Resources/*
git -C /tmp/myra-upstream log --follow -- src/Myra/Resources/Inter-Regular.ttf
strings -el /tmp/myra-upstream/src/Myra/Resources/Inter-Regular.ttf
cmp -s <Myra raw asset> <VisUI raw asset>
```

Temporary checkout paths are not project inputs. All durable revisions,
hashes, findings, and decision gates are recorded above.
