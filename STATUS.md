# cna-backrooms visual goal status

## Current state

The visual/product goal remains **active**. The game has three distinct families, deterministic connected rooms, bounded 5-by-5 streaming, collision, conventional mouse look, Shift walk/run toggling, native level profiles, licensed audio, transitions and harmless entities. Current Level 0 views show office wallpaper, acoustic ceilings, warm fixtures, broad/irregular rooms and sparse architectural anomalies. Builds and numeric tests do not finish the visual audit.

## Pass in progress

Format 28 selects height-dependent office wall lighting and a modestly warmer profile after matched comparisons. The next composition pass replaces the repeated long/T-shaped freestanding wall with several sparse region-level plans. Collision and actual room traversal will be rechecked for that geometry change.

## Completed passes

- **Format 28:** sampled wall height and distance to ceiling fixtures, with a more local fluorescent response; office faces use two height bands. Nine matched views compared the first and refined responses; twelve standard and 32 distant directional views were inspected. Five warm/neutral pairs select a modestly warmer office tint; final twelve views and six controller views were inspected. The CPU bake is isolated in `Lighting.cpp`, with shared-boundary, negative-coordinate, floor-contact and cache-eviction checks. Other families retain their prior lighting response.
- **Format 27:** native `SamplerState::AnisotropicWrap` (four taps requested, driver cap/fallback handled by CNA). Nine paired views show clearer mid-distance carpet and ceiling seams without conspicuous moire. All twelve standard views and six actual controller views were inspected. No renderer expansion.
- **Format 26:** an original generated loop-pile carpet bitmap replaces smooth procedural floor detail, with native CNA decoding, mipmaps and a procedural fallback. Seven paired close/wide/chunk-border views exposed oversized loops at four metres; the two-metre footprint is selected, shared by contact shading. The twelve-view round and all six controller-corner views were inspected. Missing bitmap views and an actual corrupt-file launch both fall back successfully. Asset provenance is documented and the source PNG is retained unchanged.

- **Format 25:** sparse shallow office alcoves along solid walls, shared collision, a lower acoustic lid and occasional painted false doors with projecting steel levers. Candidates stay in one cell and exclude props/columns/offset partitions; fixtures avoid the lid. Initial eight paired views exposed a lintel drawn in the wrong axis; the correction, eight refined views and close/angled views were inspected. The twelve-view round, 72 distant views covering all four directions, and controller views were inspected. QA now includes actual back-wall/side-wall/exit tests in all four alcove orientations. `world_quality --alcoves` reports deterministic samples for reproduction.
- **Format 24:** six fixed views compared separate lighting, ceiling-height and combined variants. The chosen 2.75 m ceiling, 2.4 m openings and lower ambient/bounce add office depth and human scale. Twelve standard, eighteen distant and seven close views were inspected. All six entrance cases and a 355 m controller return pass. A 9.6 km sweep stays bounded at 25 chunks/256 cached layouts, 4.23 ms peak and latter-half RSS 179.1–179.4 MiB.
- **Format 23:** Sharp Runtime File/JsonDocument loads profile format 1 for exactly three families. Packaged and built-in defaults match; malformed data reports a path error. Startup/route source fingerprints prevent mismatched QA configurations. Missing/invalid-file launches and controller rejection of mismatched profiles pass.
- **Format 22:** one composed entrance per neighboring connected room pair, preserving the original contracted tree. A bounded 256-layout cache does not alter generation. Two negative-coordinate controller returns over 355/770 m and a 9.6 km sweep pass.
- **Earlier visual and engineering passes:** original patterned wallpaper, refined carpet and cast finishes, thicker walls with occluded/face-dependent baked light, acoustic-grid fixtures/mipmaps, asymmetric openings, offset partitions, rare 60 m halls, human-scale service entrances/furniture, open storage bays/supports and distinct weathered tunnels with sparse mechanical equipment. Exact evidence is retained in [validation history](docs/validation-history.md).

## Three highest-priority deficiencies

1. Freestanding office partitions repeat a long straight/T-shaped plan. Add sparse L-shaped, staggered and dead-space compositions, preserving door clearance and physical reachability.
2. Some wall lighting gradients are still broad and simple. Reassess during traversal after the composition pass.
3. Industrial supports and tunnel pipe profiles remain conspicuously simple in some close views. Reassess with turned/distant views before adding geometry.

## Latest validation

- Format 28 Debug/Release builds and world/profile/lighting tests pass; source fingerprint `274dca75c397b6e3`. The height-lighting implementation completes a 355.3 m, 28-waypoint real return in 199.7 seconds near 59 FPS, 25 chunks, 2.60 ms peak and warmed RSS 178.0–179.3 MiB. Its 2.4 km office sweep holds 25 chunks, 3.37 ms peak and 195 created/1,990 reused buffers; latter-half RSS is 180.6–181.8 MiB. These longer tests precede the tint-only selection. The final warm profile completes a 65.7 m return across negative chunks in 44.1 seconds, 59 FPS, 6.92 ms peak and warmed RSS 176.8–177.2 MiB. A tunnel screenshot cold build reached 20.03 ms. After avoiding duplicate office-only height lookups in other families, the finite 2.4 km tunnel sweep holds 25 chunks, near 59 FPS and 9.18 ms peak; latter-half RSS is 254.9–259.1 MiB. The cold outlier is recorded, not claimed absent.
- Format 27 Debug/Release builds and world/profile tests pass. A real 65.8 m negative-coordinate return completes six waypoints in 42.1 seconds near 59 FPS, maximum 25 chunks, 3.12 ms peak and warmed RSS 175.0–175.3 MiB. A 2.4 km sweep stays near 59 FPS, maximum 25 chunks, 3.76 ms peak and 203 created/1,982 reused buffers; latter-half RSS is 178.6–179.7 MiB. Generation/collision are unchanged.
- Format 26 Debug/Release builds and world/profile tests pass. A real 65.5 m negative-coordinate return crosses a chunk boundary in six waypoints, near 59 FPS, maximum 25 chunks and 4.10 ms peak; RSS is 173.6–173.8 MiB. The 2.4 km sweep stays near 59 FPS and maximum 25 chunks, with 3.41 ms peak and 203 created/1,982 reused buffers; warmed RSS is 178.2–179.5 MiB. One concurrent screenshot launch had a 16.99 ms cold build peak; it did not recur in the finite sweep. Geometry/collision are unchanged from format 25.

- Format 25 Debug/Release builds and world/profile tests pass. The loaded-profile seven-seed structural scan and all 63 collision-level 90 m squares pass. Office alcoves occur 43–61 times per 16,384 sampled cells depending on seed; other families have none.
- All four real-controller alcove orientations pass back/side collision and leaving the recess. A new outward/return walk completes 128 waypoints over 972.2 m in 579.6 seconds through ten player chunks, near 59 FPS, maximum 25 active chunks and 2.86 ms peak build. Warmed RSS is 177.0–178.1 MiB. The walk uses the refined alcove geometry before the cosmetic lever change; final levers were inspected separately.
- The final format 25 2.4 km positive/negative sweep stays near 59 FPS, maximum 25 chunks, 3.08 ms peak, 203 created/1,982 reused buffers and warmed RSS 179.2–180.5 MiB. These finite observations supplement the longer earlier runs; they are not an unlimited-duration claim.
- Refined tunnels completed a 28.8 km sweep at 25 chunks, near 59 FPS and 7.20 ms peak. Last twelve RSS samples were 291.4–292.2 MiB as larger buffer capacities settled.
- A game-side CNA renderer context lease prevents stale reused-buffer uploads; the exact regression location is covered by controller QA and [bugs.md](bugs.md). No sibling repositories are modified.
- Native PipeWire capture verifies routing and signal levels: hum about -32 dBFS RMS, steps near -10 dBFS peak, isolated transition below -9.6 dBFS, no clipping. The hardware sink is muted, so subjective listening remains unverified. Visual/controller tests use dummy audio and do not validate audibility.

## Next pass

Vary sparse freestanding office partition compositions. Build, audit physical connectivity, capture matched/distant views and walk a newly generated return route. The goal remains active.
