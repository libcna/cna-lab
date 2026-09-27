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
- Level 0 enclosed regions now use deterministic multi-cell room zones with long boundaries and sparse openings. The 12-view screenshot round shows broader spaces, alcoves and less frequent cell walls; Level 0 triangle counts around the sampled origin fell from roughly 67–74k to 32–33k.
- Wall-height shading, darker opening reveals, smoother local fluorescent contribution and an 8 m carpet pattern were tried. QA caught carpet moire from regular fibers; irregular flecks removed the strongest checker artifact.

## Three highest-priority deficiencies

1. Most openings are still centered on 5 m cell boundaries, leaving recognizable repeated doorway proportions.
2. Walls meet carpet without much contact shadow; room corners still need more depth.
3. The wallpaper and carpet repeat over long sightlines, and some regions still have conspicuous straight wall runs.

## Latest validation

- Last committed build passed at `b55acce`. Current version 5 changes pass Debug and Release builds and world tests. The Release executable launched on isolated OpenGL ES and reached Level 1. Live 0→1→2 and both reverse transitions pass after the room change.
- Previous 9.6 km Level 2 GPU sweep held 24–25 active chunks near 59 FPS with warmed memory around 195–196 MB.
- Four 12-view QA sets plus focused screenshots are stored under ignored `build/qa-*` paths. The visual goal remains active; further rendering and long-run quality checks are pending.

## Next pass

Offset generated openings deterministically, with matching collision and visual geometry. Add subtle carpet contact shadow at walls. Then sample many seeds/regions for repetition and run long streaming validation.
