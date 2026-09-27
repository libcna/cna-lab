# cna-backrooms visual goal status

## Current state

The September 2026 playable foundation at `aaf67c0` has deterministic regions, bounded 5-by-5 chunk streaming, collision, three levels, fixed and rare transitions, harmless entities, audio, and a reusable GPU buffer pool. This is an engineering baseline, not the visual completion target.

## Completed passes

- Playable foundation and long positive/negative coordinate GPU sweeps.
- Initial procedural wallpaper, carpet, ceiling, concrete, and tunnel textures.
- Distinct level palettes and basic architectural props.
- First 12-view screenshot baseline: 6 Level 0 views and 3 each of Levels 1 and 2.
- Material and ceiling pass: stronger wallpaper motif, warmer finer carpet, 0.625 m acoustic tiles, region-patterned fluorescent panels including rare dead fixtures. Same 12 views were captured after the change.
- Visual QA caught a Level 0 regression where removing the old ceiling strips accidentally exposed Level 2 ceiling rails. The branch was corrected and a live screenshot confirmed their removal.

## Three highest-priority deficiencies

1. Cell-by-cell partition choices still expose a regular maze instead of larger composed rooms and asymmetric passages.
2. Walls remain uniformly bright from floor to ceiling despite local fixtures; room corners lack depth.
3. The new carpet has a better color but reads too smooth at walking height; wallpaper motifs may be slightly too prominent.

## Latest validation

- Last committed build and Release tests passed at `aaf67c0`. The current material pass builds and world tests pass; the follow-up ceiling screenshot has no dark rails.
- Previous 9.6 km Level 2 GPU sweep held 24–25 active chunks near 59 FPS with warmed memory around 195–196 MB.
- New visual goal has not yet passed its screenshot audit. Baseline and first material round are stored under ignored `build/qa-*` paths; another full round follows the architecture change.

## Next pass

Compose Level 0 rooms across several cells using deterministic zone boundaries while preserving the existing connected graph. Then add a cheap wall-height and nearby-fixture lighting gradient and compare the same 12 views.
