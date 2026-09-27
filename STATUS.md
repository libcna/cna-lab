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
- Version 8 adds rare 60-metre empty halls and tall partial walls in Level 0, a smaller wallpaper ornament, more open Level 2 tunnels, and pipes, cabinets and overhead ducts. Three additional twelve-view screenshot rounds inspected all levels, plus focused views of an empty hall and tall partition. The seven-seed scan found zero cells without an exit in version 8.
- Version 8 also shades wall ends from nearby fluorescent samples and builds a full mip chain for each procedural material through CNA's Texture2D API. Focused before/after screenshots show the ceiling-grid moire removed at distance. A fourth 12-view round inspected the result.
- The Level 0 carpet now tiles at a finer scale, carries brighter beige textile flecks and retains worn patches. A generated wood material and lighter upholstery distinguish rare tables and chairs from their formerly black placeholder appearance. Focused screenshots show the improvement.

## Three highest-priority deficiencies

1. Level 2's pipes and broader passages help, but arbitrary views can still look like repeated 5-metre cells. More varied tunnel silhouettes are needed.
2. Level 1 and 2 need more oblique visual QA, including random distant regions.
3. Level 0 furniture silhouettes remain crude and real-device subjective audio loudness remains unverified.

## Latest validation

- Current version 8 passes Debug and Release builds and world tests, including the carpet and furniture material pass. The game launched on isolated OpenGL ES and passed live 0→1→2 and both reverse routes.
- Version 6 Level 0 completed a 9.6 km GPU sweep across both coordinate signs with 24–25 active chunks, near 59 FPS, peak chunk build 26.5 ms, and warmed RSS about 175–178 MB. The earlier Level 2 sweep held near 195–196 MB.
- Version 8 Level 2 completed a 9.6 km GPU sweep with at most 25 active chunks, near 59 FPS, 25.2 ms peak chunk build, 5,735 buffer reuses, and warmed RSS near 198 MB. Multiple 12-view QA sets plus focused screenshots are stored under ignored `build/qa-*` paths. The visual goal remains active.

## Next pass

Inspect oblique views and distant regions in Level 2, then refine repeated tunnel shapes. Recheck memory after mipmapped textures during a later sweep.
