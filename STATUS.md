# cna-backrooms status

## Delivery schedule

Owner target: **2026-09-28 20:00 Europe/Prague (18:00 UTC)**.
Continue implementation and QA until about **19:30 local (17:30 UTC)**, then
freeze features, stabilize/build/run, document remaining issues and commit the
delivery. **20:55 local (18:55 UTC)** is contingency, not a feature extension.
The visual/product goal remains **active** until its dedicated audit passes.

## Current state

The accepted format **48** candidate (committed foundation: **47**) provides three distinct connected level
families, patterned office materials, composed rooms/partitions/column plans,
coherent huge-hall fixtures, sparse furniture/false doors, environmental level
transitions, conventional FPS input, Shift walk/run toggling and collision.
World state stays bounded: 25 active chunks, 48 spare buffers and 256 cached
room layouts. Three harmless creature types share immutable meshes and drift
within a clear cell. They fade near the player and emit sparse licensed voices.
There is no combat, health, damage, chasing, inventory or required quest.

Format 48 refines acoustic-panel fissures and inset seams. All 21 matched
pitched office/hall views and 12 family views have been inspected. The first,
overly bold candidate was rejected; the final fine fissures retain quiet distant
panels. Geometry, collision and recipe 11 remain unchanged. A native Wayland
28.8 km format 47 run completes with bounded chunks/buffers, but rising RSS does
not establish a plateau. The current format 48 run repeats the same 9.6 km route
four times to distinguish repeated growth from new-region high water.

## Completed passes

- Materials: original wallpaper, office carpet and mineral concrete, native mipmaps/anisotropy, human scale, warm baked wall/floor illumination and contact agreement.
- Composition: connected room zones, asymmetric openings, six offset partitions, five office support plans, rare 60-metre halls with three coherent fixture plans, service bays and narrower utility tunnels.
- Geometry: closed wall junctions, painted opening returns, suspended industrial lights, attached ducts/roof ledges and continuous curved pipe returns.
- Creatures: three deterministic types, shared bodies/contact meshes, bounded drift, smooth proximity fade and sparse attenuated/panned CC0 breath/rustle cues.
- Runtime: repeated real controller returns, negative-coordinate regeneration, buffer reuse, live entrances, recapture regression fix and technical normal-device audio checks.

Exact pass history and finite limitations: [validation history](docs/validation-history.md).

## Three highest-priority deficiencies

1. Complete and interpret the repeated native Wayland capacity run; retain exact memory/loading/presentation measurements.
2. Refresh clean Release validation and actual controller return after externally changed CNA dependencies.
3. Complete the delivery product audit, controls/entrances/normal audio and documentation. Native Wayland real input and subjective listening remain unverified; compositor screenshot access is denied.

## Latest validation

- Format 48 Release/Debug builds and all three Debug CTest suites pass. Final clean Release validation follows. Kind selection, cached/uncached generation and bounded entity drift are checked.
- All nine actual six-metre creature approaches/returns pass position and mouse-angle checks. All **99 captured stages** are inspected: distinct silhouettes, grounded feet, ceiling clearance, near fade and restored native depth/alpha state. Artifacts: build/creatures47-approach and build/creatures47-review-0..16.png.
- All three stationary normal own-stream voice captures pass without footsteps or clipping. Event peaks: **-14.00/-13.85/-11.37 dBFS**, against hum RMS about -32.4 dBFS. Stream is unmuted at 100%, Ryzen speaker sink unmuted at 57%. Artifacts: build/creatures47-audio-0..2. Listening by ear is not performed.
- Format 46 strict controls cover all mouse directions, walk/run speeds, Shift behavior and Escape/click recapture. All six live entrance cases and all 33 original figure stages pass; images are inspected. Hum is -32.27 dBFS RMS, walking peak -10.13 dBFS, isolated transition peak -13.58 dBFS, no clipping.
- Format 46 visual evidence includes 48 matched office directions, 36 huge-hall directions, nine pitched floor pairs and twelve family views, all inspected.
- The unchanged default-world collision route completes **1,187.7 m**, all 136 waypoints, sixteen player chunks and maximum 25 active; all 52 route directions are inspected. Warmed RSS 162.45–181.18 MiB, final 162.58; 185 buffers created/977 reused; peak build 25.40 ms with seven >16.67 ms outliers. No other owned GPU/compiler ran during measurement; shared scheduling is not isolated.
- Earlier 28.8 km tunnel sweep holds 25 chunks, 17 sampled spares and final-quarter RSS 321.1–332.7 MiB. Native Wayland 45 short run completes 2.4 km, but its short duration does not establish a plateau. The current longer native sweep follows.

## Dependencies and practical limits

Latest observed read-only next heads: CNA **a62c40b09e868462077fbf4914ff1a153fa5d324**,
Sharp Runtime **007280bd1cc789f851f7f454a5041c8ce2479e13**. Siblings are changed
externally; this game agent never modifies them. The explicit CNA context lease
remains necessary for recycled-buffer uploads, transitions and deletion; see
[bugs.md](bugs.md). Recheck dependency heads during final validation.

Lighting is an inexpensive bake; large-room floor pools remain approximate.
Creature movement is simple drift, not articulated walking. Private-display
rendering is roughly 39 FPS/59 UPS, not evidence of normal desktop performance.
Occasional generation/presentation outliers remain. Walking is flat, and saves,
vertical traversal, moving doors and a story are outside this milestone.

## Next pass

Finish the repeated native capacity run. Then rebuild/test the final Release,
walk the default-world controller return with panorama captures, and refresh
controls, entrances and technical normal-device audio. At 19:30 stop additions;
complete the delivery product audit, document deficiencies and commit the state.
