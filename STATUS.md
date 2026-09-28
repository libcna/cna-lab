# cna-backrooms visual goal status

## Current state

The September 2026 playable foundation at `aaf67c0` has deterministic regions, bounded 5-by-5 chunk streaming, collision, three levels, fixed and rare transitions, harmless entities, audio, and a reusable GPU buffer pool. This is an engineering baseline, not the visual completion target.

## Completed passes

- Surface-aware steps now use muted carpet and separate hard-floor sounds from the existing CC0 collection, with small pitch variation. An isolated transition was captured without footsteps. The build copies all four sounds beside the binary; a real-backend launch from /tmp loaded them successfully.

- A normal-controller route exposed a pooled-buffer upload regression. GPU readback and a deterministic replay identified missing EasyGL context ownership between frame leases. Streaming, transitions and final GPU cleanup now hold CNA context tokens. The 326 m return route, exact failing view and an automatic-exit check pass; bugs.md records the diagnosis without sibling changes.

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
- Version 9 adds rare 30-metre Level 2 service chambers with a few structural pillars and denser, irregular ceiling fixtures. Six-sided shaded pipes replace flat-colored wall strips. The concrete floor no longer changes color abruptly from one cell to the next. A 12-view QA round and focused chamber screenshot show a much larger Level 2 space without floor color seams.
- Version 10 changes Level 1 storage bays from the same four-rack grid to four deterministic layouts with two to four racks. Beams change direction and spacing by region, and shelf metal and boxes are easier to see. A focused rack view and a further 12-view round show less obvious repeated framing.
- A four-seed, 12-view Level 0 audit sampled spawn and locations hundreds of metres away. It found one camera inside a harmless entity. Entity rendering now stops within 3.5 metres; a repeated screenshot at the exact seed and position shows clear vision.
- Format 11 adds region-wide Level 0 partitions offset from cell boundaries. They create longer walls and alcoves; the same split boxes drive rendering and collision. Screenshots exposed flat obstacle faces, so office trim, local shading and carpet contact shadows were added. A four-seed scan of over 25,000 opening centers exposed one narrowed doorway; the partition endpoint now leaves clearance and all sampled openings pass.
- A focused weak-circuit view and another 12-view screenshot round inspected the stronger lit/dark contrast, lighter carpet and framed fluorescent panels. Six Level 0, three Level 1 and three Level 2 views were reviewed again. The visual goal remains active.
- The bounded spare GPU buffer pool grew from 24 to 48 to hold a retired row of chunk materials. A format 11 9.6 km Level 0 sweep cut fresh buffer creations from 972 to 181 and the warmed RSS plateaued near 181.4 MB.
- An 18-view distant visual audit sampled seeds 0, 1 and 31337 hundreds of metres from spawn across all three levels. It confirmed distinct environment families but exposed nearly identical Level 2 pipe strips. Level 2 now chooses four pipe layouts per region and uses inset service-cabinet fronts; matched six-view screenshots confirmed the variation.
- Level 0 now samples fluorescent influence at the center and edges of each 2.5 m floor/ceiling tile and halfway along walls. Paired screenshots show visible warm pools around fixtures. The wallpaper texture is 512 by 512 for a longer, less repetitive span; rare ceiling panels show water staining. Another twelve-view round inspected the result. A 2.4 km GPU pass held near 59 FPS, with a 24.9 ms peak chunk build and warmed RSS near 182.4 MB.
- Five matched distant storage views exposed nearly identical concrete-looking racks and orange boxes. The rack now uses slimmer steel posts, diagonal braces, varied shelf spacing and varied cardboard boxes with tape. Five wider views inspected the result.
- A collision-level walkability tool sampled 63 complete 90 m squares at one-metre spacing with actual player movement. It found one narrow partition gap that the cell graph could not detect. Partition offsets and endpoint clearance were increased; the repeat audit has one substantial component in every sample. A live Release controller stopped at a partition, strafed around its end and continued forward with matching collision and geometry.
- Format 12 removes most short cell-boundary fragments in Level 0 open offices and column rooms. The random hash recipe stays stable so exact before/after views remain comparable. Six matched views across three seeds show substantially emptier column rooms, plus longer offset walls. A further twelve-view round inspected all levels. Debug/Release tests, the 63-square physical walkability audit, all live transition directions and a 2.4 km GPU pass passed; the pass held near 59 FPS with a 24.4 ms peak and at most 25 chunks.

## Three highest-priority deficiencies

1. Repeat longer GPU sweeps after the context fix: previous sweeps counted chunks but could display stale GPU geometry after reuse.
2. Improve sparse furniture proportions and readable upholstery/frame materials.
3. Level 0 still has overly uniform wall lighting and crude furniture proportions. Improve those against matched screenshots rather than adding more level types.

## Latest validation

- Real PipeWire capture of the game's own sink stream shows idle hum around -32 dBFS RMS, carpet/hard-floor step peaks near -10 dBFS and an isolated transition peak at -12.5 dBFS with no clipping. It is routed to the Ryzen hardware speaker sink, but that sink is muted in the agent environment. Subjective listening remains unverified.

- The format 12 Release normal-controller regression traversed 326 m and returned in 225.8 seconds, including negative chunk coordinates. All 36 waypoints passed; the exact pooled-buffer regression view is correct after 299 reuses, at most 25 chunks, near 59 FPS and a 2.05 ms peak build. A 400 m automatic-exit GPU check also passed, exercising final GPU cleanup. Earlier numeric sweeps did not expose the visually incorrect reused-buffer uploads; their timing/memory results are historical, not visual validation of the fixed renderer.

- Current version 12 passes Debug and Release builds, deterministic world tests, the seven-seed quality scan, 63-square physical walkability and all four directed live transition checks.
- Current Level 1 and Level 2 completed 9.6 km GPU sweeps near 59 FPS with at most 25 chunks. Level 1 peaked at 25.1 ms, reused 6,305 buffers and ended near 187.6 MB RSS; Level 2 peaked at 25.5 ms, reused 6,442 buffers and plateaued near 202.4 MB. The 63-square collision walkability audit passed after the clearance fix.
- Version 6 Level 0 completed a 9.6 km GPU sweep across both coordinate signs with 24–25 active chunks, near 59 FPS, peak chunk build 26.5 ms, and warmed RSS about 175–178 MB. The earlier Level 2 sweep held near 195–196 MB.
- Version 10 Level 0 and 1 each completed 9.6 km GPU sweeps with at most 25 active chunks and near 59 FPS. Level 0 peaked at 36.5 ms for a chunk and warmed RSS rose from roughly 178 to 181 MB; Level 1 peaked at 26.3 ms and stabilized near 186 MB. Version 9 Level 2 held near 203 MB. A 19.2 km version 10 Level 0 sweep ended at 25 chunks and 184.5 MB after 11,406 buffer reuses and 1,728 new allocations. Version 11 Level 0 completed 9.6 km at 25 chunks, near 59 FPS, 25.2 ms peak build, 181 new allocations, 6,829 reuses and a warmed RSS plateau near 181.4 MB. Multiple 12-view QA sets plus focused screenshots are stored under ignored `build/qa-*` paths. A real PipeWire sink input appeared during a walking test and CNA reported audio ready; subjective sound quality is untested. The visual goal remains active.

## Next pass

Repeat long GPU sweeps with the fixed uploads and inspect further screenshots. Temporary direct GL diagnostics have been removed; the game uses only CNA APIs. Audio routing and isolated cues are now checked, including launch from /tmp. Continue with matched lighting and furniture screenshots after the long fixed-upload sweeps.
