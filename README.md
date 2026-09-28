# cna-backrooms

A small first-person Backrooms exploration game in C++ and CNA. Walk through deterministic, effectively unbounded rooms, storage spaces, and maintenance tunnels. There is no combat, damage, quest, inventory, or game over. The occasional distant figure is harmless.

## Build and run

The supported target is **Linux desktop, CNA's EasyGL `OPENGLES3` renderer**. Put `cna` and `sharp-runtime` beside this repository, both on their `next` branches. You need CMake 3.21+, a C++23 compiler, and CNA's SDL/OpenGL ES dependencies.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target cna_backrooms world_tests --parallel
./build/world_tests
./build/cna_backrooms --seed 31337
```

CNA normally builds its vendored SDL dependencies. If SDL3, SDL3_image, and SDL3_mixer CMake packages are installed, use `-DCNA_USE_SYSTEM_SDL=ON`. This checkout was validated with CNA's existing SDL install:

```sh
cmake -S . -B build -DCNA_USE_SYSTEM_SDL=ON \
  -DCMAKE_PREFIX_PATH=../cna/.sdl-prebuilt-Linux-x86_64-wayland/install
```

The build copies assets beside the executable, so the game can be launched from any working directory. Missing or unavailable audio is logged and the game continues. `--seed` accepts an unsigned decimal or `0x` hexadecimal integer; the default is reproducible. `--level 0|1|2 --position x z` starts at a chosen valid point for inspecting procedural rooms. `--walk-speed` and `--run-speed` set movement speeds in metres per second (defaults: 2.4 and 4.8). `--fov` sets vertical field of view in degrees, from 45 to 90 (default: 60, about 91 degrees horizontally at 16:9). `--stream-test` runs a 9.6 km automated GPU streaming sweep and exits; `--stream-test-metres 19200` uses another distance and implies the test mode.

## Controls

| Input | Action |
| --- | --- |
| Mouse | Look |
| W/A/S/D | Move |
| Shift | Toggle walking and running |
| R | Return to the current level's spawn |
| Escape | Release mouse; press again to quit |
| Left click | Recapture mouse |

Narrow, framed maintenance entrances with recessed dim interiors are level transitions. From the Level 0 spawn, one is about 15 metres ahead. From the Level 1 spawn, the return entrance is about 15 metres to the right and the Level 2 entrance is about 15 metres ahead. Level 2's return entrance is about 15 metres ahead. Rare additional entrances can be found far from the origin. Walking into an entrance changes level immediately.

## Levels and world generation

- **Level 0, The Yellow Rooms:** patterned yellow wallpaper with broad stains, dirty office carpet, suspended ceiling tiles with rare stained panels, fluorescent fixtures with occasional failed circuits, mixed room sizes, columns, long offset partitions, rare furniture, false doors, and occasional large empty halls.
- **Level 1, Service Storage:** tall concrete spaces, mixed open bays and narrow links, sparse steel shelving with varied shelf spacing and cardboard contents, structural beams and industrial lighting.
- **Level 2, Maintenance Tunnels:** lower ceilings, weathered concrete, narrower cabinet-lined passages, longer open runs, several supported pipe layouts, sparse pressure assemblies, occasional overhead ducts, and rare open service chambers.

The world format is algorithm version `21`, a 64-bit seed, and a level id (`0`–`2`). There are no chunk files. Integer cells are 5 metres wide; chunks are 8 by 8 cells. Six by six-cell regions pick spatial patterns, including open rooms, columns, irregular rooms, storage, and tunnels. Level 0's enclosed regions group cells into larger irregular room zones; walls follow zone boundaries. Open and column rooms use fewer cell-boundary fragments, relying on columns and longer offset walls. Longer room partitions sit away from cell lines, with shared deterministic geometry and collision. Rare selected 12 by 12-cell areas form unbroken 60-metre empty halls. A deterministic tree within each region and shared border connectors preserve connectivity. Level 1 has larger connected service bays, cast-concrete column rooms and enclosed multi-cell spaces; storage bays choose one of several sparse shelf layouts and orientations, with two to four structural supports and few internal wall fragments. Region-level industrial fixtures and baked light obstruction give the concrete spaces their own identity. Level 2 uses more open tree passages to avoid a doorway at nearly every cell; some complete 6 by 6-cell regions instead form open service chambers with a few structural pillars. Stable hashes control asymmetric openings, props, room-level light layouts, rare distant entrances and entities. Recreating a chunk from the same seed gives the same geometry and collision, including across negative coordinates. Geometry-only revisions can retain the internal hash recipe so landmarks remain at reproducible locations.

The game retains at most a 5 by 5 neighborhood of chunks around the player. Each update prepares at most one missing chunk and uploads its CNA vertex buffers. Distant chunks are removed; a bounded pool holds up to 48 spare vertex buffers for reuse, then destroys excess buffers. The streamer holds a CNA renderer context token while uploading and retiring buffers; see [CNA findings](bugs.md). Collision queries use the same deterministic walls, props and maintenance frames, independent of loaded geometry. Chunk vertices are local to their chunk to reduce float jitter far from the origin. The window title reports position, chunk, active chunks, triangles, buffer reuse, entities, build time, walk/run mode, audio state and FPS.

## Assets and limitations

Geometry and most reusable textures are generated in code. An original generated wallpaper PNG adds faded office ornament and paper detail; CNA decodes/resizes it and the game builds mipmaps. A procedural fallback remains. Four CC0 sounds from the NOX SOUND Essentials Series provide fluorescent hum, muted carpet steps, hard-floor steps and transition cues. See [asset licenses](assets/LICENSE.md) and [generation record](assets/wallpaper-generation.md). Sparse chairs use a generated fabric material and thin steel frames; tables have wooden tops and conservative shared collision bounds. Furniture and figures are simple low-poly meshes. The figures wander in place and cannot interact with the player.

Chunk generation is synchronous and capped at one upload per update. Walking is flat; collision uses a horizontal circle, not full character physics. The world has no save file, vertical traversal, moving doors or authored story. Linux desktop `OPENGLES3` is the only intended platform for this milestone.

## Validation

Debug and Release builds and deterministic world tests pass for format 21. Tests cover negative coordinates, connected regions, asymmetric openings, obstacle placement, fixed/rare entrances, narrow-frame collision and outside-gate movement, and grid-aligned fluorescent fixtures that avoid columns and partitions. `world_quality` samples seven seeds per level; `world_walkability` checks 63 complete 90-metre squares using actual player collision on a one-metre lattice. The latter caught a narrow partition gap which was fixed. Every sampled square now has one substantial connected component.

`python3 tools/capture_views.py --game build/cna_backrooms --output build/visual-qa` captures the standard twelve views on X11/Xwayland. Add `--distant` for eighteen views across seeds 0, 1 and 31337, hundreds of metres from spawn. Install `xdotool` and ImageMagick; use a separate test display. The tool keeps screenshots, logs and a manifest for comparison.

Multiple screenshot rounds cover six Level 0 locations and three each of Levels 1 and 2, with additional distant views across several seeds. Format 13 adds face-dependent, wall-occluded fluorescent illumination baked into vertex colors, wall thickness matching collision, and ceiling fixtures fitted to the acoustic tile grid. This uses CNA BasicEffect and generated mipmapped textures, without a new renderer. A 2.4 km Release GPU sweep with the new lighting held near 59 FPS, at most 25 chunks and a 2.36 ms peak build. Format 16 replaces the simple wallpaper pattern, uses smaller painted baseboards, removes heavy ceiling-edge trim and quiets the acoustic grid. Paired screenshots and a normal-controller return walk were inspected. Screenshot evidence is retained under ignored `build/qa-*` paths. The product-quality goal remains active; build and test success alone do not complete it.

After fixing an EasyGL context ownership problem, all three levels completed 9.6 km GPU sweeps across positive and negative coordinates near 59 FPS and at most 25 chunks. Level 0 peaked at 4.13 ms per chunk and warmed RSS stayed about 179–180 MB; Level 1 peaked at 4.34 ms and about 182–184 MB; Level 2 peaked at 7.51 ms and about 195–198 MB. These observations describe these runs, not unlimited-duration behavior. Earlier numeric sweeps could display stale reused buffers and are superseded by these checks.

A format 16 2.4 km Release sweep with the new bitmap and painted trim held near 59 FPS, at most 25 chunks, a 2.98 ms peak build and warmed RSS about 168–169 MB.

The format 16 326 m Release normal-controller route and return passed all 36 waypoints in 223.7 seconds, with near 59 FPS and a 2.24 ms peak chunk build. The dedicated screenshot shows complete geometry at the original pooled-buffer failure location after 169 reuses; the full route finishes with 347. For this regression, install Python 3, `xdotool` and ImageMagick, then run `python3 tools/controller_walk.py --game build/cna_backrooms` on X11/Xwayland. It retains screenshots and a trace under ignored `build/controller-qa/`. Check `pooled-buffer-regression.png` for a complete floor and ceiling. The game-side CNA context workaround is documented in [bugs.md](bugs.md).

Matched furniture screenshots caught a contact-shading brightness error; it now follows the floor triangle illumination. A live default-speed check measured 4.8 m walking and 9.7 m running over two seconds, and verified one toggle per Shift hold.

Private-display play checks covered conventional mouse look, Shift walk/run toggling, configurable speeds, collision, low partitions, furniture, false doors and all four directed transitions. A 400 m automatic-exit GPU test also exercised final GPU cleanup.

Audio was captured from only the game's real PulseAudio/PipeWire sink input routed to the Ryzen hardware speaker output. Idle hum measured about -32 dBFS RMS; walking produced peaks near -10 dBFS, and an isolated transition reached -12.5 dBFS without footsteps. No clipping occurred. The hardware sink was muted in this agent environment, so subjective audibility/loudness remains a real-machine listening check. Screenshot and streaming runs use SDL's dummy audio device.

For additional normal-controller walks, build `world_route`, generate a route, and feed it to the input driver (X11, Python 3, xdotool and ImageMagick):

```sh
./build/world_route 1 12345 -122.5 -117.5 > build/route.json
python3 tools/controller_walk.py --route-file build/route.json --speed 2.4
```

The planner is a QA tool, not in-game pathfinding. It leaves clearance around walls and avoids entrances; the driver walks out and back using actual collision and records screenshots, chunk counts and process RSS. Format 18 return walks covered approximately 512 m in Level 1 and 703 m in Level 2, both near 59 FPS and maximum 25 chunks. Their warmed RSS ranges were 165.8–166.5 and 181.7–183.5 MiB. Format 19 refines carpet and concrete-ceiling materials and fixes overbright wall contact shading; a Level 0 2.4 km Release sweep remains bounded near 59 FPS with a 4.48 ms peak build and warmed RSS 177.1–177.8 MB.

Format 21 refines the mechanical tunnel finishes and light depth. Paired views caught excessive equipment symmetry, which was reduced. Debug/Release tests, physical connectivity and all six entrance cases pass. The real controller completed a 711 m outward/return route with 114 waypoints near 59 FPS and a 6.32 ms peak. A 9.6 km Level 2 sweep stays within 25 chunks and 48 spare buffers, with a 7.72 ms peak. The larger geometry raises retained memory: latter-half RSS is 269.5–281.8 MiB, with a longer plateau check pending.
