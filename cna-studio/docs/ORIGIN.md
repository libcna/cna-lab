# Origin of CNA Studio

CNA Studio began as a one-time source import of the **CNA Editor prototype**, which was developed
inside the `cna-lab` monorepo. This document records exactly what was imported, from where,
and what the relationship between the two repositories is from that point on.

## The import

| Field | Value |
|-------|-------|
| Source repository | <https://github.com/libcna/cna-lab> |
| Source branch | `develop` |
| Source directory | `cna-editor/` |
| Source commit | `3bce82dd74e9a201a21e31308d43d2ee7761d641` |
| Source commit date | 2026-09-01 |
| Source commit subject | `Add 'cna-editor/' from commit 'f5e36a2fd361d38e8181c6eb3940149e51b3cc44'` |
| Date of import | 2026-09-14 |
| Destination | <https://github.com/libcna/cna-studio> (repository root) |
| Files imported | 214 tracked files |

The **contents** of `cna-editor/` became the **root** of the standalone Studio repository:
`cna-editor/CMakeLists.txt` became `./CMakeLists.txt`, `cna-editor/src/` became `./src/`,
and so on. The product was subsequently renamed to CNA Studio.

The import was performed with `git archive` at the exact commit above, so only files tracked
by the source repository were copied. Build directories, generated binaries and other ignored
artifacts were never part of the transfer.

Reproducing the imported tree:

```bash
git clone --branch develop https://github.com/libcna/cna-lab
cd cna-lab && git checkout 3bce82dd74e9a201a21e31308d43d2ee7761d641
git archive HEAD cna-editor | tar -x -C /destination --strip-components=1
```

## Verified baseline at the moment of import

Measured on the imported tree before any CNA Studio transformation, with the default
(dependency-free) build configuration — no CNA checkout, no GPU, no window:

| Measure | Value |
|---------|-------|
| Compiler | GCC 13.3.0, C++23 |
| CMake | 3.28.3 |
| Configure | `cmake -S . -B build -G Ninja` |
| Build | clean, **0 warnings** |
| Unit assertions | **442 passed, 0 failed** |
| CTest suites | **12 passed, 0 failed** (2.51 s) |
| Executables | `cna-editor`, `cna-player`, `cna-editor-tests` |
| Static libraries | 12 modules |

## What this repository is now

`libcna/cna-studio` is the **only** repository in which CNA Studio is developed. From the
import commit onward, CNA Studio is its own evolving product with its own history, its own
roadmap (`plan.md`) and its own release cadence.

The original `cna-lab/cna-editor` snapshot remains available in Git history at the source
commit above. On 2026-09-28, `cna-editor/` was removed from the current `cna-lab` tree after
being replaced by `cna-studio/`. The archive command above still reproduces the prototype.

The current `cna-lab/cna-studio` subtree is a later import of standalone Studio, added on
2026-09-26 from commit `0dd11d5bb11e4286194d031ea92fd80917ea1f16`. It is not the original
prototype snapshot.

There is **no automatic synchronisation** between the standalone Studio repository and its
`cna-lab` subtree. Development takes place in the standalone repository; any later subtree
refresh is an explicit import. The prototype baseline remains fixed at the source commit
recorded above.

## Dependencies, not write targets

CNA Studio is built on the CNA ecosystem. These repositories are consumed as dependencies and
are never modified by CNA Studio development:

| Repository | Branch audited | Commit audited at import |
|------------|----------------|--------------------------|
| <https://github.com/libcna/cna> | `next` | `e05b3d0f026e0926741f89459daf02579240399d` |
| <https://github.com/libcna/sharp-runtime> | `next` | `0c82d9b888bdf5f7d5663c77942f339bcb2a7445` |

When CNA Studio work reveals a deficiency in CNA, it is recorded in
[`docs/CNA-GAPS.md`](CNA-GAPS.md) rather than fixed in CNA's own repository.

## Licence and third-party obligations

The import carried the product's licence and notices unchanged:

- [`LICENSE`](../LICENSE) — Microsoft Public License (Ms-PL), matching CNA
- [`NOTICE.md`](../NOTICE.md)
- [`THIRD_PARTY_NOTICES.md`](../THIRD_PARTY_NOTICES.md)
- `third_party/cgltf/` — cgltf, with its own licence. `third_party/imgui/` — Dear ImGui, imported
  at the same time — was removed by `STUDIO-07031` once the UI it backed (`STUDIO-07030`) no longer
  needed it.

A product rename does not discharge third-party obligations. As dependencies are later added
or removed, these notices are updated to match what is actually shipped.
