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
- Version 6 shifts Level 0 door and wide-opening spans off center. The same span drives rendering and collision; broad collision tests sampled hundreds of openings. A softened carpet contact shadow adds depth at wall bases without creating a dark border.
- The seven-seed, three-level world-quality scan sampled 16,384 cells per seed/level. It found zero cells with no exits; Level 0 had roughly 59% open edges, 17% solid edges, and varied room-kind proportions. This is structural evidence, not a substitute for long player exploration.

## Three highest-priority deficiencies

1. Some Level 0 regions still expose repeated wall lengths and columns across long sightlines.
2. Level 2 needs stronger mechanical/service details to move beyond a dark palette and support its tunnel identity.
3. The wallpaper and carpet repeat over long walks, while real-device subjective audio loudness remains unverified.

## Latest validation

- Last committed build passed at `5bfd3b9`. Current version 6 passes Debug and Release builds and world tests. The Release executable launched on isolated OpenGL ES and reached Level 1; live 0→1→2 and both reverse transitions pass.
- Version 6 Level 0 completed a 9.6 km GPU sweep across both coordinate signs with 24–25 active chunks, near 59 FPS, peak chunk build 26.5 ms, and warmed RSS about 175–178 MB. The earlier Level 2 sweep held near 195–196 MB.
- Multiple 12-view QA sets plus focused screenshots are stored under ignored `build/qa-*` paths. The visual goal remains active; further Level 2 and long-run quality checks are pending.

## Next pass

Add a few low-cost Level 2 utility details and occasional Level 0 structural anomalies. Inspect another multi-location screenshot round, then stress Level 2 streaming and perform an extended play audit.
