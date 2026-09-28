# cna-backrooms visual goal status

## Current state

The September 2026 playable foundation at `aaf67c0` has deterministic regions, bounded 5-by-5 chunk streaming, collision, three levels, fixed and rare transitions, harmless entities, audio, and a reusable GPU buffer pool. This is an engineering baseline, not the visual completion target.

## Completed passes

- Format 18 adds a distinct industrial column-region family, opens most internal bay boundaries and groups enclosed Level 1 areas into multi-cell room zones. Cast-concrete pillars share full-height collision; industrial fixtures vary by region and avoid pillars. Face-dependent, wall-occluded baked illumination now covers Level 1 too. Walls in all levels have the thickness used by collision. Six matched industrial comparisons and a complete twelve-view round were inspected. Debug/Release tests, the seven-seed scan, 63-square physical audit and six live entrance cases pass. A 2.4 km Level 1 Release sweep holds near 59 FPS, at most 25 chunks, 2.17 ms peak and warmed RSS 167.1–168.3 MB.

- Format 17 replaces broad black transition boxes with human-scale service entrances: 1.3 m openings, level-matching exterior finishes, ordinary casing and recessed dim concrete interiors. Shared portal boxes drive graphics, collision, light obstruction and fixture avoidance. Carpet contact shading now follows the actual floor triangles for columns as well as furniture. Paired views of all three entrance families were inspected; Debug/Release tests, the seven-seed scan and 63-square collision audit pass. Real controller input passes all four fixed transition directions, outside-gate traversal and frame collision.

- Format 16 compares an original imagegen wallpaper against the procedural material in the actual game. Its finer ornament/paper detail, thin painted baseboards, absent heavy top trim and quieter ceiling grid make the spaces less diagram-like. CNA loads the PNG and builds mipmaps; a fallback and executable-relative assets remain. Three matched FOV views favor a 60-degree vertical default, now configurable from the CLI. A twelve-view round and the 326 m controller return route were inspected; 36 waypoints passed in 223.7 seconds near 59 FPS, maximum 25 chunks, 2.24 ms peak and 347 buffer reuses.

- Format 15 removes abrupt cell-sized wallpaper tint patches in favor of a continuous low-amplitude finish variation, and subtly varies acoustic panels. A targeted view exposed false doors hidden inside the new wall thickness; a shared thickness constant and corrected face offset restore them. Debug/Release tests and a new twelve-view round pass. A direct comparison with the original-room photograph identifies overly strong ceiling/trim lines as the next visual deficiency.

- Format 14 replaces bulky concrete-looking chairs with smaller fabric seats, thin steel legs and back frames, and slimmer tables. Shared rotated bounds drive collision. Contact shadows match the floor triangles after a paired screenshot caught a bright patch. Walking/running defaults are now 2.4/4.8 m/s with corresponding step cadence; live measured distances and held-Shift toggles pass.

- Format 13 bakes wall-occluded and face-dependent fluorescent contribution into vertex colors. A temporary per-chunk sample cache reduces repeat work. Level 0 walls now have two independently shaded faces at their actual collision thickness. Fluorescent panels occupy exactly two acoustic tiles, shift away from full-height obstacles and use warmer white diffusers. A twelve-view round was inspected; deterministic fixture-placement tests and both builds pass.

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

1. The industrial ceiling has a coarse cloudy finish that weakens its concrete/service identity.
2. Extend normal-controller traversal and return routes in Levels 1 and 2; a QA-only collision route planner is being prepared.
3. Improve coarse carpet detail and audit floor contact shading in close and distant views.

## Latest validation

- Format 18 Debug/Release builds and world tests pass. Seven-seed structural and 63-square physical walkability scans pass. Six industrial before/after pairs and another twelve-view round were inspected. The Level 1 2.4 km Release GPU sweep has a 2.17 ms peak, 141 created/1,396 reused buffers, near 59 FPS, maximum 25 chunks and warmed RSS 167.1–168.3 MB. Six real-controller entrance cases pass after changed room geometry. Audio for these controller/streaming checks uses the dummy backend.

- Format 17 Debug/Release builds and world tests pass, along with seven-seed structural and 63-square physical walkability scans. All four transition directions, a clear path outside the entrance and collision with its narrow frame pass through actual keyboard/mouse input. Three paired entrance views were inspected. Dummy audio was used for this controller test; it does not supersede the separate real-backend audio evidence.

- Format 16 2.4 km Release GPU sweep: at most 25 chunks, near 59 FPS, 2.98 ms peak, warmed RSS 168.1–168.5 MB; 202 buffers created and 1,955 reused.

- Format 16 Debug/Release builds and world tests pass. PNG loading uses CNA, assets are packaged beside the executable, matched material/FOV screenshots were inspected and the 326 m controller route returned successfully. Eighteen distant views across seeds 0, 1 and 31337 were captured and inspected with tools/capture_views.py. A real-backend /tmp launch loaded the packaged PNG and all sounds; retimed walking/transition signals peak below -9.6 dBFS with no clipping.

- Format 15 Debug/Release builds and world tests pass. The paired door image confirms the occlusion fix, and six Level 0/three Level 1/three Level 2 views were inspected after continuous wall tint and ceiling-age changes.

- Format 14 Debug/Release builds, world tests, physical walkability and the seven-seed scan pass. Matched chair/table/embedded-prop views were inspected. A two-second live movement test measured 4.8 m walking and 9.7 m running; held Shift toggles once in each direction.
- Format 13 Debug/Release builds and world tests pass. Four seeds over 128 by 128 cells verify reproducible fixtures, acoustic-grid alignment and avoidance of full-height geometry. The latest twelve-view round shows stronger wall depth and properly fitted fixtures. Physical walkability and the seven-seed quality scan pass. A 2.4 km Release GPU sweep held near 59 FPS with at most 25 chunks, 2.36 ms peak build and warmed RSS about 166–168 MB.
- Corrected-upload 9.6 km sweeps finished for all three levels near 59 FPS and at most 25 active chunks. Level 0: 4.13 ms peak, warmed RSS 178.8–179.6 MB, 181 buffers created/6,829 reused. Level 1: 4.34 ms, 182.2–183.2 MB, 152/6,305. Level 2: 7.51 ms, 194.7–197.4 MB, 160/6,440. Earlier sweeps could render stale pooled buffers and do not validate the fixed rendering path.
- Format 12 Release normal-controller regression: 326 m return route in 225.8 seconds, all 36 waypoints, maximum 25 chunks, near 59 FPS and 2.05 ms peak. Exact failing view is now correct after 299 reuses. A 400 m automatic-exit check exercised cleanup.
- Real PipeWire capture of the game's own stream: hum about -32 dBFS RMS, steps near -10 dBFS peak, isolated transition -12.5 dBFS, no clipping. Routed to the Ryzen speaker sink, which is muted. Subjective listening remains unverified. Launch from /tmp loaded all four packaged sounds.

## Next pass

Replace the industrial ceiling's cloudy material and improve carpet grain at close range. Prepare actual controller return walks across generated Levels 1 and 2, with screenshots and bounded-memory observations. The goal remains active.
