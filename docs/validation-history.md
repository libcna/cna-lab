# Earlier validation passes

Through format 29, window-title “FPS” counted fixed-step updates. All historical values near 59 in this file are update rates (UPS), not measured draw rates. Format 30 adds real start-to-start draw intervals and separate CPU submission timing.

## Formats 25–30


- **Format 30:** native four-sample MSAA, actual draw timing and native whole-chunk frustum checks. Nine MSAA pairs were inspected; nine culled/unculled images are pixel-identical. All 48 directions at twelve locations and all sixteen controller/regression views were inspected. Visible submission falls from 25 to 9–13 chunks in matched office views, without a demonstrated FPS increase. Bounds use direct coordinate comparisons after the prototype's native-object copies caused tunnel spikes. Slow-build logs split geometry/upload time. MSAA can be disabled from the CLI.

- **Format 29:** six sparse freestanding office plans (long, L, T, staggered, short and U-shaped dead space), with shared clipped geometry/collision and unchanged border connectors. Twenty-four matched views, twelve standard views, 32 distant directions, all 24 controller views and eight alcove views were inspected. Shorter screens and staggered/U plans create different open and dead spaces. `world_quality --partitions` and guarded capture views reproduce each plan.

- **Format 28:** sampled wall height and distance to ceiling fixtures, with a more local fluorescent response; office faces use two height bands. Nine matched views compared the first and refined responses; twelve standard and 32 distant directional views were inspected. Five warm/neutral pairs select a modestly warmer office tint; final twelve views and six controller views were inspected. The CPU bake is isolated in `Lighting.cpp`, with shared-boundary, negative-coordinate, floor-contact and cache-eviction checks. Other families retain their prior lighting response.
- **Format 27:** native `SamplerState::AnisotropicWrap` (four taps requested, driver cap/fallback handled by CNA). Nine paired views show clearer mid-distance carpet and ceiling seams without conspicuous moire. All twelve standard views and six actual controller views were inspected. No renderer expansion.
- **Format 26:** an original generated loop-pile carpet bitmap replaces smooth procedural floor detail, with native CNA decoding, mipmaps and a procedural fallback. Seven paired close/wide/chunk-border views exposed oversized loops at four metres; the two-metre footprint is selected, shared by contact shading. The twelve-view round and all six controller-corner views were inspected. Missing bitmap views and an actual corrupt-file launch both fall back successfully. Asset provenance is documented and the source PNG is retained unchanged.

- **Format 25:** sparse shallow office alcoves along solid walls, shared collision, a lower acoustic lid and occasional painted false doors with projecting steel levers. Candidates stay in one cell and exclude props/columns/offset partitions; fixtures avoid the lid. Initial eight paired views exposed a lintel drawn in the wrong axis; the correction, eight refined views and close/angled views were inspected. The twelve-view round, 72 distant views covering all four directions, and controller views were inspected. QA now includes actual back-wall/side-wall/exit tests in all four alcove orientations. `world_quality --alcoves` reports deterministic samples for reproduction.
- **Format 24:** six fixed views compared separate lighting, ceiling-height and combined variants. The chosen 2.75 m ceiling, 2.4 m openings and lower ambient/bounce add office depth and human scale. Twelve standard, eighteen distant and seven close views were inspected. All six entrance cases and a 355 m controller return pass. A 9.6 km sweep stays bounded at 25 chunks/256 cached layouts, 4.23 ms peak and latter-half RSS 179.1–179.4 MiB.
- **Format 23:** Sharp Runtime File/JsonDocument loads profile format 1 for exactly three families. Packaged and built-in defaults match; malformed data reports a path error. Startup/route source fingerprints prevent mismatched QA configurations. Missing/invalid-file launches and controller rejection of mismatched profiles pass.
- **Format 22:** one composed entrance per neighboring connected room pair, preserving the original contracted tree. A bounded 256-layout cache does not alter generation. Two negative-coordinate controller returns over 355/770 m and a 9.6 km sweep pass.
- **Earlier visual and engineering passes:** original patterned wallpaper, refined carpet and cast finishes, thicker walls with occluded/face-dependent baked light, acoustic-grid fixtures/mipmaps, asymmetric openings, offset partitions, rare 60 m halls, human-scale service entrances/furniture, open storage bays/supports and distinct weathered tunnels with sparse mechanical equipment. Exact evidence is retained in [validation history](docs/validation-history.md).


### Runtime evidence


- **Timing correction:** through format 29, the title labeled update rate as FPS. Historical rates around 59 are UPS, not actual render measurements. The format 30 isolated Weston/Xwayland display draws around 39–41 FPS. Its 120-interval metrics include streaming/presentation; CPU submission is reported separately. No real-desktop presentation rate is inferred from this test display.
- Format 30 final Release/Debug builds and world/profile/lighting tests pass. CNA `next` is `b2fd47a45757c32326cbbb5c2b39afffdb7392c5`, Sharp Runtime `next` is `fc033a0e8541a81498c4a496f56a0f59475c6e34`; neither sibling has been edited by this game.
- A real 355.2 m/28-waypoint return finishes in 204.8 seconds, maximum 25 chunks, 3.55 ms peak build and warmed RSS 175.5–176.3 MiB. Views include the pooled-buffer regression. This precedes the bounds-only optimization; geometry/collision are unchanged. The 2.4 km office sweep has median 40 draw FPS, median rolling p95 26.27 ms and sampled maximum 31.31 ms; latter-half RSS is 175.9–177.1 MiB.
- The first tunnel prototype reached 54.45 ms build/86.57 ms sampled draw interval and exposed expensive per-vertex `Vector3::Min/Max` object copies. Direct component comparisons replace them. Two final 2.4 km tunnel sweeps (MSAA 0/4) both hold 25 chunks and median 39 draw FPS. Build medians/peaks are 3.04/9.28 and 2.72/7.11 ms; median rolling p95 intervals are 26.38/27.18 ms, sampled maxima 35.09/32.65 ms. Latter-half RSS is 249.0–253.7 and 251.2–256.0 MiB. Neither emits a build over 16.67 ms. These finite observations support retaining four samples; they do not prove absence of future spikes.

- Format 29 Debug/Release world/profile/lighting tests, seven-seed structural scan and all 63 physical 90 m squares pass. The real controller completes 136 waypoints over 1,162.5 m in 674 seconds through 15 player chunks, near 59 update FPS, maximum 25 chunks and 2.70 ms peak build. Warmed RSS is 178.4–179.8 MiB. All four alcove back/side/exit cases pass. A 9.6 km sweep holds 25 chunks and 256 cached layouts, 4.54 ms peak, 209 created/8,041 reused buffers and latter-half RSS 180.5–181.4 MiB. Finite observations, not an unlimited-duration claim. Some controller snapshots face nearby walls; distant four-direction rounds supplement those views.

- Format 28 Debug/Release builds and world/profile/lighting tests pass; source fingerprint `274dca75c397b6e3`. The height-lighting implementation completes a 355.3 m, 28-waypoint real return in 199.7 seconds near 59 FPS, 25 chunks, 2.60 ms peak and warmed RSS 178.0–179.3 MiB. Its 2.4 km office sweep holds 25 chunks, 3.37 ms peak and 195 created/1,990 reused buffers; latter-half RSS is 180.6–181.8 MiB. These longer tests precede the tint-only selection. The final warm profile completes a 65.7 m return across negative chunks in 44.1 seconds, 59 FPS, 6.92 ms peak and warmed RSS 176.8–177.2 MiB. A tunnel screenshot cold build reached 20.03 ms. After avoiding duplicate office-only height lookups in other families, the finite 2.4 km tunnel sweep holds 25 chunks, near 59 FPS and 9.18 ms peak; latter-half RSS is 254.9–259.1 MiB. The cold outlier is recorded, not claimed absent.
- Format 27 Debug/Release builds and world/profile tests pass. A real 65.8 m negative-coordinate return completes six waypoints in 42.1 seconds near 59 FPS, maximum 25 chunks, 3.12 ms peak and warmed RSS 175.0–175.3 MiB. A 2.4 km sweep stays near 59 FPS, maximum 25 chunks, 3.76 ms peak and 203 created/1,982 reused buffers; latter-half RSS is 178.6–179.7 MiB. Generation/collision are unchanged.
- Format 26 Debug/Release builds and world/profile tests pass. A real 65.5 m negative-coordinate return crosses a chunk boundary in six waypoints, near 59 FPS, maximum 25 chunks and 4.10 ms peak; RSS is 173.6–173.8 MiB. The 2.4 km sweep stays near 59 FPS and maximum 25 chunks, with 3.41 ms peak and 203 created/1,982 reused buffers; warmed RSS is 178.2–179.5 MiB. One concurrent screenshot launch had a 16.99 ms cold build peak; it did not recur in the finite sweep. Geometry/collision are unchanged from format 25.

- Format 25 Debug/Release builds and world/profile tests pass. The loaded-profile seven-seed structural scan and all 63 collision-level 90 m squares pass. Office alcoves occur 43–61 times per 16,384 sampled cells depending on seed; other families have none.
- All four real-controller alcove orientations pass back/side collision and leaving the recess. A new outward/return walk completes 128 waypoints over 972.2 m in 579.6 seconds through ten player chunks, near 59 FPS, maximum 25 active chunks and 2.86 ms peak build. Warmed RSS is 177.0–178.1 MiB. The walk uses the refined alcove geometry before the cosmetic lever change; final levers were inspected separately.
- The final format 25 2.4 km positive/negative sweep stays near 59 FPS, maximum 25 chunks, 3.08 ms peak, 203 created/1,982 reused buffers and warmed RSS 179.2–180.5 MiB. These finite observations supplement the longer earlier runs; they are not an unlimited-duration claim.
- Refined tunnels completed a 28.8 km sweep at 25 chunks, near 59 FPS and 7.20 ms peak. Last twelve RSS samples were 291.4–292.2 MiB as larger buffer capacities settled.
- A game-side CNA renderer context lease prevents stale reused-buffer uploads; the exact regression location is covered by controller QA and [bugs.md](bugs.md). No sibling repositories are modified.
- Native PipeWire capture verifies routing and signal levels: hum about -32 dBFS RMS, steps near -10 dBFS peak, isolated transition below -9.6 dBFS, no clipping. The hardware sink is muted, so subjective listening remains unverified. Visual/controller tests use dummy audio and do not validate audibility.


## Earlier formats

- Format 24 applies the six-view independent/combined office comparison: 2.75 m ceiling, 2.4 m openings, lower ambient/bounce and slightly stronger local fluorescents. Debug/Release builds, world/profile tests and the loaded-profile seven-seed scan pass. Twelve standard, eighteen distant, seven close/detail views and controller screenshots were inspected. Furniture, partitions, acoustic panels and maintenance frames fit; all six live entrance cases pass. The repeated real-controller regression completes 28 waypoints over 355.3 m in 196.8 seconds, near 59 FPS, maximum 25 chunks, 2.90 ms peak and warmed RSS 176.2–177.3 MiB. The first interrupted run has no summary and is not counted as a pass. The 9.6 km Release sweep stays near 59 FPS, maximum 25 chunks and 256 cached layouts, with 4.23 ms peak, 221 created/7,918 reused buffers and latter-half RSS 179.1–179.4 MiB. Close views show office materials at sensible scale; long-wall uniformity remains, and several arbitrary camera headings face walls rather than spaces. Dummy audio was used for these visual/controller checks.

- Format 23 loads profile format 1 from `assets/levels.json` through existing Sharp Runtime File/JsonDocument APIs. It centralizes heights, fog, baked-light contributions, tints, ordered room weights and entity rarity for the three fixed families. Game-owned immutable profiles preserve the default format 22 recipe; missing definitions use built-in values, invalid definitions report a path error. Source-byte fingerprints appear in startup logs and route JSON; the controller rejects mismatched definitions. Debug/Release builds and world/profile tests pass, including default geometry equivalence, malformed-field rejection and native file/fallback paths. The loaded-profile seven-seed scan and 63-square physical audit pass. A twelve-view round was inspected, all six live entrance cases pass, and a 60 m matching-profile controller return passes near 59 FPS, 25 chunks and 2.43 ms peak. Actual missing/invalid-file launches and mismatched-route rejection pass. Six-view contrast, ceiling-height and combined probes expose better office depth and scale; those values are not yet applied to the release defaults.

- Format 22 selects one entrance per neighboring connected room pair instead of retaining every cell-tree crossing. Interiors are open; disconnected pieces are labeled separately, and original tree contraction preserves connectivity. The optional game-owned cache retains at most 256 layouts and has no effect on generated results. Six paired office views, six additional turned views, the twelve-view round and both controller walks were inspected. Debug/Release tests, cached/uncached equivalence and eviction checks, the seven-seed scan, 63-square physical audit and all six entrance cases pass. The regenerated pooled-buffer regression route passes 28 waypoints over 355 m in 196.1 seconds (3.06 ms peak, warmed RSS 175.9–177.2 MiB). A second seed passes 100 waypoints over 770 m in 466.9 seconds (3.28 ms, 177.0–177.2 MiB). Both stay near 59 FPS and maximum 25 chunks. A 9.6 km Release sweep confirms the 256-layout cap, maximum 25 chunks, 3.82 ms peak and latter-half RSS 179.1–179.4 MiB; 221 buffers are created and 7,918 reused.

- The refined format 21 tunnel pass also completed a 28.8 km positive/negative sweep near 59 FPS, maximum 25 chunks and 7.20 ms peak. RSS increases through new high-water buffer capacities: quarter ranges are 231.3–277.5, 277.5–284.7, 284.7–287.1 and 287.1–292.2 MiB. The last twelve samples are 291.4–292.2 MiB. These are finite-run observations, not an unlimited-duration claim; active chunks and spare buffers remain bounded. It creates 383 buffers and reuses 28,706.

- Format 21 gives Level 2 weathered neutral concrete, cast ceilings, face-dependent occluded light, framed service cabinets, supported wall pipes and sparse pressure banks. Six matched tunnel views exposed excessive mirrored equipment and crude hexagonal ends; the refinement reduces density and uses finer circular profiles. Six refined pairs, three close equipment views, eighteen distant multi-seed views, twelve standard views and controller-route screenshots were inspected. Debug/Release tests, the seven-seed scan, 63-square physical audit and all six live entrance cases pass. A real 711 m return walk passes 114 waypoints in 462.2 seconds, maximum 25 chunks, near 59 FPS and 6.32 ms peak. Warmed RSS grows from 230.5 to 240.8 MiB as the buffer pool fills. A 9.6 km sweep holds near 59 FPS and 25 chunks, 7.72 ms peak, with 270 buffers created and 9,504 reused. Its latter-half RSS is 269.5–281.8 MiB; the active set and spare pool are bounded, but a longer plateau check is still needed for the larger pipe geometry.

- Format 20 reduces storage-bay internal wall fragments and adds two to four cast-concrete supports around each sparse shelf layout. Shared pillar placement suppresses overlapping props and shifts fixtures. Six paired storage views were inspected; two initially faced nearby walls, so additional turned views inspect their actual bays. Debug/Release builds and tests, the seven-seed structural scan, 63-square collision audit and all six live entrance cases pass. The regenerated Level 1 return route passes 40 waypoints and approximately 512 m in 269.2 seconds, near 59 FPS, 25 maximum chunks, 2.03 ms peak and warmed RSS 173.4–173.9 MiB. A 9.6 km sweep stays near 59 FPS, 25 chunks, 2.43 ms peak and stable late RSS about 174.5 MiB, with 162 buffers created and 6,948 reused.

- Format 19 replaces the cloudy industrial roof with a quieter cast-concrete finish and subtle formwork lines, also used on pillars and service-chamber ceilings. Carpet uses finer 4 mm-scale grain while retaining metre-scale wear. Paired close views exposed overbright wall-base strips; their colors now sample the existing floor triangles. Debug/Release builds and tests, six paired industrial views, three close carpet views and a new twelve-view round pass. A 2.4 km Level 0 Release sweep stays near 59 FPS, maximum 25 chunks, 4.48 ms peak, warmed RSS 177.1–177.8 MB.

- A QA-only collision route planner now emits versioned JSON waypoints with 0.55 m clearance and avoids transition triggers. The controller driver accepts these routes, walks outward and back with real input, captures arbitrary route views and records process RSS. On format 18, Level 1 passed 44 waypoints over approximately 512 m in 284.8 seconds; Level 2 passed 116 over approximately 703 m in 464.1 seconds. Both stay near 59 FPS and at most 25 active chunks. Warmed RSS ranges are 165.8–166.5 and 181.7–183.5 MiB, with 2.15/2.54 ms peak builds respectively. Route screenshots were inspected; they expose excessive storage-bay fragments and overly uniform brown tunnels as the next composition/material deficiencies.

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



## Concrete material, format 31

An original 1,254-square concrete image replaces regular tunnel wall spots and floor noise. Native decoding downsamples to 1,024, builds mipmaps and shares one GPU texture between wall/floor slots. A two-metre footprint is selected. Missing or corrupt images restore separate procedural fallbacks. Nine matched wall pairs, close floor views, 24 distant directions, sixteen controller views and twelve final family views were inspected. Release/Debug world/profile/lighting tests pass. The real 449.3 m return passes 60 waypoints in 275.3 seconds, five player chunks, maximum 25 chunks and 10.12 ms peak build; warmed RSS is 231.0–241.2 MiB while capacities settle. A 2.4 km sweep holds 25 chunks, median 39 draw FPS, median rolling p95 28.57 ms, sampled maximum 39.70 ms and 9.74 ms peak build. Latter-half RSS is 255.2–262.3 MiB; this short run does not establish a final plateau.


## Pipe profiles, format 32

Small service pipes use eight radial sides, shared circumferential vertex shades and continuous cylindrical UVs; sixteen-sided pressure assemblies keep their geometry. All nine matched camera pairs were inspected. The loaded tunnel triangle count rises by about 10% in the sampled spawn area (375,312 to 414,244), while the 2.4 km sweep holds median 39 draw FPS and at most 25 chunks. Median rolling p95 is 26.70 ms, sampled maximum 37.68 ms, median build 3.54 ms and peak build 9.34 ms, with no build above 16.67 ms. Latter-half RSS is 266.5–279.2 MiB as the bounded buffer pool settles; a longer final tunnel plateau check remains. Release/Debug builds and world/profile/lighting suites pass. Collision, generation and profile bytes are unchanged. This visual pass uses dummy audio and does not validate audibility.


## Service supports, format 33

Structural columns reuse the original concrete texture with profile tints and baked samples at three heights on each face. Industrial and tunnel chamber columns share one helper, base band and floor contact shading; collider bounds are unchanged. All 24 directions at six multi-seed service locations and twelve standard family views were inspected. The finish replaces plain beige posts; broad industrial views still expose narrow office-like wall fragments as a separate composition deficiency. Release/Debug world/profile/lighting suites pass. The 9.6 km tunnel sweep holds 25 chunks, median 39 draw FPS, 26.60 ms median rolling p95, 38.32 ms sampled maximum, 3.67 ms median build and 9.40 ms peak build. No build exceeds 16.67 ms. RSS quarter ranges are 10.5–273.8 (includes startup), 273.8–282.0, 282.0–287.9 and 287.9–299.4 MiB. The bounded pool creates 264 buffers and reuses 9,510, but this run still does not establish a final plateau; a longer final check remains.


## Harmless silhouettes and audio recheck, format 34

A single tapered cloth mesh replaces six box parts; rounded head/limbs use continuous facet shades. Two small native alpha-blended contact spots follow the feet. Deterministic spawn queries keep figures outside spawn/obstacles and out of collision. Eighteen old/new near/mid/distant views across two seeds and all families, eighteen final grounded views and twelve family views were inspected. The industrial backgrounds still expose narrow wall fragments. Debug/Release world/profile/lighting suites and seven-seed entity sampling pass. QA waits for 25 loaded chunks and guards actual camera angles, rather than assuming a three-second launch delay. The title reports packed GPU buffer capacity, separate from process RSS.

Fresh normal-device captures route only this game's sink input to the unmuted Ryzen speaker output at 58% hardware volume, with an unmuted/non-corked 100% stream. Hum RMS is -32.26 dBFS, footsteps peak at -10.83 dBFS. A separate entrance crossing from position 18.36,2.5 travels less than one footstep interval and captures the transition peak at -13.47 dBFS, with hum RMS -32.42 dBFS. Both enter Level 1 without clipping or voice-acquisition warnings. The permanent audio QA script preserves device settings and records routing. Technical capture is not subjective listening; that part remains a human check.

The collision-enabled Level 0 return completes all 28 waypoints over 356.0 m in 203.9 seconds, five player chunks, at most 25 loaded chunks and 9.18 ms peak build. All ten route views and the exact pooled-buffer regression view were inspected. Sampled RSS falls from 186.3 MiB to 157.3 MiB, with a warmed range of 157.3–174.9 MiB. Early draw rates around 36–38 rise to 39 on the return; no causal renderer claim is inferred from shared-machine timing. The first attempt ended before the title was initialized; waiting for actual streaming fixes the QA startup failure.

The final 28.8 km tunnel diagnostic crosses both coordinate signs and revisits the route, with collision deliberately bypassed. It holds at most 25 chunks, a sampled spare-pool maximum of 18 (limit 48), 390 buffer creations and 28,699 reuses. Packed GPU buffer capacity ranges from 33.1 to 57.2 MiB and ends at 54.2 MiB. RSS quarter ranges are 10.4–294.6 (startup included), 294.6–300.1, 300.1–300.6 and 295.5–300.6 MiB; the final twelve five-second samples are 295.5–298.7 MiB. This finite repeat reaches a late plateau rather than continuing the short-run growth. Median draw rate is 39 FPS, median rolling p95 27.22 ms, sampled maximum interval 72.57 ms, median sampled build 3.87 ms and peak 25.29 ms. Ten builds exceed 16.67 ms over about 640 seconds, so occasional spikes remain; there is no claim of hitch-free rendering. No compiler or other game owned by this agent ran during the measurement.


## Industrial bay composition, format 35

Open, column and storage bays now have no interior cell-boundary walls; enclosed utility rooms and shared region-border entrances retain narrower connections. Beam rows follow structural support centers. Sixteen matched directions at four multi-seed locations show the narrow fragments removed, but expose fixtures hidden by new beam positions. A refined sixteen-view comparison places the industrial fixtures on short mounts below beam soffits. Lamp height is shared with the lighting bake. All sixteen refined directions, twelve family views and thirteen real-route views were inspected. The floor's cloudy noise and coarse wall seams remain the next material deficiencies.

Release/Debug builds and all three suites pass through newly registered CTest entries. The seven-seed structural scan and all 63 physical 90-metre squares pass. The Level 1 return completes 48 waypoints over 528.3 m in 288.6 seconds, eight player chunks, maximum 25 active chunks and 1.97 ms peak build. Sampled warmed RSS is 188.0–188.4 MiB, final 188.4 MiB, with 206 buffer creations and 516 reuses. Draw rate holds around 39 FPS on the private display. Source profile fingerprint remains `274dca75c397b6e3`; the random recipe and other family composition are preserved.

All six live entrance checks pass after the refined industrial pass: four directed transitions, movement outside the gate and frame collision. The driver now waits for actual initial streaming before sending input.


## Industrial mineral surfaces, format 36

Industrial walls/floors now share the existing original mineral texture, with cooler profile tints preserving approximate brightness and a two-metre wall footprint. The procedural fallback has quieter grain and seams. All nineteen matched floor/wide comparisons, twelve standard family views and six missing/corrupt-image launches were inspected. Native decoding errors select independent procedural surfaces without a launch failure. Release/Debug CTest suites pass. Profile fingerprint changes to `6075bcb6c5f3514c`; geometry, collision and hash recipes are unchanged.

A finite 9.6 km industrial GPU diagnostic crosses both coordinate signs and revisits the route. Maximum active chunks are 25, sampled spare buffers 19, with 251 creations and 9,143 reuses. Packed capacity ranges from 5.8 to 6.4 MiB. RSS reaches 188.74 MiB in the first quarter and remains there in the final three quarters. Median actual draw rate is 39 FPS, median rolling p95 26.28 ms and sampled maximum interval 112.11 ms. Median sampled build is 1.13 ms and peak build 5.80 ms. This diagnostic bypasses collision; it is not a replacement for controller exploration. No compiler or other GPU job owned by this agent ran during measurement. Dummy audio does not validate listening.

The visual QA driver supports pitch through actual relative mouse input and checks measured yaw/pitch before capture. The new `world_quality --views` selects twelve reproducible uncurated positions across six new seeds, off the cell centers and up to four kilometres from the origin. Only collision/entrances are rejected; headings are not chosen for attractive views. Two consecutive selections are byte-identical. All forty-eight directions were captured and inspected. Office opening soffits are too bright and use rotated wallpaper strips; tunnel edge beams float below the roof and form an obvious repeated lattice; nearby figure cutoff remains abrupt. These are the next three visual deficiencies. The office sample spans all five room archetypes without hand-selection. No missing floors, ceilings or reset camera positions were observed.


## Lit opening returns, format 37

Office openings use a muted painted return instead of wallpaper across narrow side/end surfaces. Two height bands share the wall's light samples. Lintel undersides now receive per-corner illumination in all families; office soffits retain the painted material. Geometry/collision positions and profile fingerprint are unchanged. All twenty-four paired uncurated office directions, twelve family views and eleven actual-route/regression views were inspected. The close seed 6502/88801/293781 views no longer show bright sideways wallpaper strips. No floor/ceiling upload regression was observed.

Release/Debug CTest suites pass. The collision-enabled return completes all 28 waypoints over 355.1 metres in 191.6 seconds, crosses five player chunks and holds at most 25 active chunks. Peak sampled build is 2.59 ms; warmed RSS remains 187.79 MiB, with 185 buffer creations and 333 reuses. Private-display draw rate holds around 39 FPS. The tunnel floating roof beams and abrupt entity cutoff remain the next visible deficiencies.


## Attached tunnel roof details, format 38

Unconditional floating cell-edge beams are replaced by roof ledges on real solid walls. Their upper edges meet the ceiling; two small hangers attach overhead duct covers. Room connectors, collision positions, the hash recipe and source profiles are unchanged. Twelve matched uncurated directions across three seeds, twelve family views and sixteen actual-route views were inspected. No missing floor, ceiling or reused-buffer corruption is observed. Abrupt pipe ends near openings remain a separate visible deficiency.

Refreshed Release/Debug builds and all three CTest suites pass after externally changing CNA/Sharp Runtime sources become consistent. The initial temporary Debug failure and successful retry are recorded in bugs.md; no sibling file is edited by this agent. The new binary completes all 60 collision-enabled waypoints and return over 448.3 m in 269.9 seconds, five player chunks and at most 25 active chunks. Peak sampled build is 6.84 ms. Warmed RSS ranges from 236.5 to 246.7 MiB, ending at 246.7 MiB, with 208 buffer creations and 313 reuses. Draw rate holds around 39 FPS on the private display. This short return does not establish a memory plateau. No compiler or other game owned by this agent runs during the walk; earlier standard screenshot timing overlapped a dependency rebuild and is not used for performance conclusions. Dummy audio does not validate listening.


## Smooth harmless-figure retreat, format 39

Figures fade with a smooth proximity curve between 5.0 and 1.5 metres. Their contact spots follow the same opacity. A game-owned native BlendState disables color writes for a depth-only draw of fading bodies; the subsequent premultiplied BasicEffect/AlphaBlend pass reads that depth, avoiding overlapping rear cloth surfaces. Opaque blend, default depth and alpha one are restored after each figure. One immutable mesh remains shared, with no collision, interaction or attack behavior. The title reports nearest loaded-figure distance/opacity for QA. No engine or custom shader changes are used.

The new world-quality approach selection finds clear six-metre paths with real 0.55 m clearance and rejects entrances. The actual-controller driver captures eleven approach/return stages per family and checks measured position/yaw. Thirty-three baseline/new pairs, eighteen near/mid/distant figure views and twelve family views were inspected; vision remains unobstructed at close range and figures restore smoothly on retreat. The original driver accumulated small timed-key drift; calculating each next move from measured position resolves the QA error, and all three complete returns pass. Baseline captures overlap compilation and are not used for timing comparisons.

Release/Debug builds of the game and all three registered test targets pass; all CTest suites pass. The unrelated default all-target builds were interrupted; final gates explicitly build only the game/audit/test targets in the existing trees. All six live maintenance entrance cases pass against the refreshed external CNA runtime: four directed transitions, movement outside the gate and frame collision. Geometry, collision, profile bytes and hash recipe are unchanged. Dummy audio does not validate listening.


## Continuous service pipes, format 40

Pipe presence follows a region plan instead of changing at each five-metre wall.
Adjacent deterministic edge queries preserve uninterrupted runs across chunk
borders. Exposed ends receive short curved returns through the supporting wall;
no path graph, renderer or collision changes are needed. The four-segment
prototype increased spawn triangle count by about 62%; the two-segment refinement
reduces that to about 30%, with eight radial facets and 32 triangles per return.
Thirty-six old/new directions across distant and uncurated multi-seed locations,
twelve family views and all sixteen actual-route views were inspected. No missing
floor, ceiling or reused-buffer corruption is observed. Profile fingerprint and
random hash recipe remain unchanged.

Release/Debug game/audit/test targets and all three CTest suites pass. The real
collision-enabled return completes all 60 waypoints over 448.8 m in 270.0 seconds,
five player chunks and at most 25 active chunks. Peak sampled build is 10.07 ms;
warmed RSS is 258.14–273.47 MiB, final 272.53 MiB. The bounded pool creates 208
buffers and reuses 313, ending with eight spares and 48.1 MiB packed capacity.
Actual draw rate holds about 39 FPS on the private display. This short return is
not final plateau evidence; a fresh long sweep remains planned. No compiler or
other GPU job owned by this agent runs during the measured walk. Dummy audio does
not validate audibility.


## Coherent fluorescent diffusers, format 41

A shared native textured quad maps each fixture exactly once along its physical
long axis. The office's two world-UV tube overlays are removed; they wrapped dark
texture edges into small blocks on the diffuser. A quiet procedural prism texture
and two broad tube contributions retain a recognizable office panel. Service
fixtures and small entrance lights share the normalized mapping. Fixture plans,
lighting values, collision, source profiles and hash recipe are unchanged.

A new deterministic office selection covers three new seeds and twelve off-grid
locations in empty halls, broad rooms, weak circuits and enclosed spaces. Headings
are hashed, not selected for appealing compositions. Two consecutive selections
are byte-identical. All forty-eight baseline/new directions, six pitched ceiling
pairs and twelve family views were inspected. Weak-circuit samples include only
one to six live fixtures per region; no missing floor or ceiling is observed.
They also expose coarse floor light pools, incomplete outer wall corners and
regular column-room composition for the next passes. Release/Debug game/audit/test
targets and all three CTest suites pass. Screenshot timing is not a benchmark;
dummy audio does not validate audibility. A fresh long tunnel sweep is separate.


## Pipe capacity sweep after format 41

The fresh 28.8 km tunnel GPU diagnostic crosses both coordinate signs and returns,
with collision deliberately bypassed. Active chunks remain at most 25, sampled
spares at most 17, with 413 buffer creations and 28,676 reuses. Packed capacity is
41.9–77.2 MiB, ending at 71.4 MiB. RSS quarter ranges are 10.6–332.0 (including
startup), 332.0–340.1, 322.2–340.1 and 321.1–332.7 MiB; the last twelve samples stay
at 321.1–332.7 MiB. This finite repeat establishes settling after the added pipe
geometry rather than continuing growth through the final quarter.

Median actual draw rate is 39 FPS, median rolling p95 27.55 ms and maximum sampled
interval 108.82 ms. Median sampled build is 4.93 ms and peak build 55.50 ms, with
46 builds above 16.67 ms during about 640 seconds. The largest geometry component
is 52.86 ms and the largest upload component 24.48 ms. No compiler or other GPU
job owned by this agent runs during measurement; shared-machine scheduling is
not isolated, and no cause is assigned to the concentrated timing outliers.
These occasional spikes remain a refinement target. Dummy audio is not listening
evidence, and this diagnostic does not replace actual collision traversal.

## Format 42: closed wall junctions and remote office traversal

Cell-boundary wall faces stop at a shared full-thickness node core. Only exposed
core faces are emitted, with wallpaper continuations and painted office ends;
internal faces stay absent. This closes the old ceiling/baseboard corner wedges
and uncapped isolated ends without coplanar extensions. Collision extends only
the outer segment endpoints to the same core; doorway spans are unchanged.
Small four-quadrant probes cover isolated ends and perpendicular outer corners.
Industrial enclosures have no isolated wall ends, so their dedicated close views
check continuous joins instead. Eighteen matched views compare format 39 with 42
(the baseline also predates pipe/diffuser changes). All pairs and twelve family
views are inspected. Recipe 11 and profile source `6075bcb6c5f3514c` stay unchanged.

Release/Debug game/audit/test builds and all three CTest suites pass; all 63
physical walkability squares contain one substantial component. Consecutive
wall-end selections are byte-identical. The remote route planner accepts a start
position and bounds the route extent to one kilometre per axis. A fresh Level 0
weak-circuit return, seed 8723, starts at `(1066.5,1711.5)`, completes all 52
waypoints and walks 426.5 metres in 277.1 seconds. Six player chunks, at most 25
active chunks, 12.67 ms peak build and warmed/final RSS 184.93–184.96 MiB are
observed. It creates 176 buffers and reuses 318; packed capacity ends at 8.5 MiB.
All fourteen route views are inspected. Draw rate is approximately 37–40 FPS on
the private display. No compiler or other game owned by this agent runs during
measurement. The preceding close screenshot batch overlaps dependency rebuilds
and its timings are not performance comparisons. Dummy audio is not audibility
evidence.

Fresh controller checks expose a game-side camera jump when clicking after
Escape: the click frame still contains absolute cursor coordinates, which the
game interpreted as relative motion. The game now skips mouse-look consumption
on that frame. The QA driver checks recapture orientation explicitly, in addition
to conventional four-direction relative input, default walk/run distances, a
held Shift and the next Shift, mouse release and normal exit. The strict repeat
passes: recapture retains `(90,0)` degrees and a rightward relative move reaches
`(78.7,0)`, instead of the old absolute-coordinate jump to `(9.3,-45.4)`. Refreshed dependency
heads are CNA `967305dd7b93f992f7c177a7055bd5892ba8523e` and Sharp Runtime
`6c4a857de129cf29b5d43430bedf24157d594f12`; siblings remain unmodified by this agent.

Largest remaining product deficiencies: coarse office floor light pools, regular
column-room composition and the fresh audio/presentation audit.

## Format 43: finer office floor illumination

Level 0 samples the existing local fluorescent field on 1.25-metre floor
triangles, replacing 2.5-metre interpolation. Floor contact shading uses the
same shared subdivision count, including negative coordinates and diagonal
boundaries. Ceilings and the two service-family floors keep their original
2.5-metre vertices. Lights, materials, room geometry, collision, profile bytes
and recipe 11 are unchanged. This adds 1,536 triangles per office chunk, or
38,400 at the 25-chunk cap, instead of refining all surfaces.

All forty-eight matched office directions, six pitched floor pairs, twelve
family views and fifteen route views are inspected. Light pools have finer,
less angular transitions, with no missing floor/ceiling or bright contact
patch observed. Existing broad-hall fixture regularity and uniform column
arrangements remain composition targets. Release/Debug game/audit/test builds
and all three suites pass; the lighting tests cover shared vertices, negative
chunk boundaries and continuity on both halves of a floor quad.

The actual remote return, seed 402717, starts at `(-2476.5,2051.5)`, completes
68 waypoints and walks 663.2 metres in 433.0 seconds. Nine player chunks, maximum
25 active, 171 buffer creations and 512 reuses are observed. Packed capacity ends
at 11.9 MiB; RSS peaks at 187.84 MiB, then holds at 162.18–163.61 MiB warmed,
ending at 162.25 MiB. Peak build is 35.51 ms. Geometry and upload spikes remain
in the log; shared-machine scheduling is not isolated. No compiler or other
GPU job owned by this agent runs during measurement. Screenshot comparison
frames are not performance comparisons and dummy audio is not listening evidence.
A matched short capacity comparison and fresh normal-device audio check follow.

The sequential 2.4 km diagnostics use the same seed 12345, coordinates and
private display, without another compiler/GPU job owned by this agent. Baseline
42 and refined 43 both hold at most 25 chunks and median 39 draw FPS. Sampled
build medians are 2.41 and 2.98 ms; rolling p95 medians are 27.33 and 27.41 ms.
Packed capacities peak at 9.4 and 13.1 MiB. Latter-half RSS ranges are
182.14–182.80 and 189.89–191.83 MiB. Refined peak build is 6.02 ms on this
route (baseline 16.43 ms); neither logs a build over 16.67 ms. These finite
shared-machine observations do not attribute the separate remote return's
35.51 ms outlier to the subdivision change or prove hitch-free rendering.

Fresh normal PipeWire captures use only this game's unmuted, noncorked stream,
at 100% stream gain, on the unmuted Ryzen speaker sink at 57%. Hum measures
-32.34 dBFS RMS, walking peaks at -10.60 dBFS and the separate transition without
a footstep peaks at -12.79 dBFS, without clipping. Both actual cases enter Level 1
and log audio ready, without audio warnings. No other stream or device setting
is changed. This establishes technical routing and signal, not subjective
listening, which remains unperformed by the agent.
