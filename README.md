# cna-backrooms

A small first-person Backrooms exploration game in C++ and CNA. Walk through deterministic, effectively unbounded rooms, storage spaces, and maintenance tunnels. There is no combat, damage, quest, inventory, or game over. The occasional distant figure is harmless.

## Build and run

The supported target is **Linux desktop, CNA's EasyGL `OPENGLES3` renderer**. Put `cna` and `sharp-runtime` beside this repository, both on their `next` branches. You need CMake 3.21+, a C++23 compiler, and CNA's SDL/OpenGL ES dependencies.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target cna_backrooms world_tests level_profile_tests lighting_tests --parallel
ctest --test-dir build --output-on-failure
./build/cna_backrooms --seed 31337
```

CNA normally builds its vendored SDL dependencies. If SDL3, SDL3_image, and SDL3_mixer CMake packages are installed, use `-DCNA_USE_SYSTEM_SDL=ON`. This checkout was validated with CNA's existing SDL install:

```sh
cmake -S . -B build -DCNA_USE_SYSTEM_SDL=ON \
  -DCMAKE_PREFIX_PATH=../cna/.sdl-prebuilt-Linux-x86_64-wayland/install
```

The build copies assets beside the executable, so the game can be launched from any working directory. Missing or unavailable audio is logged and the game continues. `--seed` accepts an unsigned decimal or `0x` hexadecimal integer; the default is reproducible. `--level 0|1|2 --position x z` starts at a chosen valid point for inspecting procedural rooms. `--walk-speed` and `--run-speed` set movement speeds in metres per second (defaults: 2.4 and 4.8). `--fov` sets vertical field of view in degrees, from 45 to 90 (default: 60, about 91 degrees horizontally at 16:9). `--msaa 0|4` controls native multisampling (default: four samples, capped by the driver; use zero on slower hardware). `--stream-test` runs a 9.6 km automated GPU streaming sweep and exits; `--stream-test-metres 19200` uses another distance and implies the test mode.

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

- **Level 0, The Yellow Rooms:** patterned yellow wallpaper with broad stains, dirty office carpet, suspended ceiling tiles with rare stained panels, fluorescent fixtures with occasional failed circuits, mixed room sizes, columns, long/L/T/staggered/short offset partitions and U-shaped dead spaces, rare furniture, shallow empty alcoves, painted false doors, and occasional large empty halls.
- **Level 1, Service Storage:** tall concrete spaces, mixed open bays and narrow links, sparse steel shelving with varied shelf spacing and cardboard contents, structural beams and suspended industrial lighting.
- **Level 2, Maintenance Tunnels:** lower ceilings, weathered concrete, narrower cabinet-lined passages, longer open runs, several supported pipe layouts with smooth baked circumferential shading, sparse pressure assemblies, occasional overhead ducts, and rare open service chambers.

The world format is algorithm version `39`, a 64-bit seed, and a level id (`0`–`2`). The three families are tuned by a small versioned [level-definition file](docs/level-format.md), loaded through Sharp Runtime. Startup logs record its source fingerprint; a missing file uses built-in defaults and an invalid file reports an error. There are no chunk files. Integer cells are 5 metres wide; chunks are 8 by 8 cells. Six by six-cell regions pick spatial patterns, including open rooms, columns, irregular rooms, storage, and tunnels. Level 0's enclosed regions group cells into larger irregular room zones; walls follow zone boundaries, with one composed entrance per neighboring connected room pair. Open and column rooms use fewer cell-boundary fragments, relying on columns and longer offset walls. Six sparse partition plans sit away from cell lines, with shared deterministic geometry and collision. Rare selected 12 by 12-cell areas form unbroken 60-metre empty halls. Contracting the deterministic cell tree onto connected rooms and retaining shared border connectors preserves connectivity. Level 1 has larger connected service bays, textured cast-concrete column rooms and enclosed multi-cell spaces; storage bays choose one of several sparse shelf layouts and orientations, with two to four structural supports and no interior cell-boundary walls. The roof framing follows support rows and fixtures hang below the beam soffits; neighboring enclosed utility rooms and shared border entrances retain narrower connections. Region-level industrial fixtures and baked light obstruction give the concrete spaces their own identity. Level 2 uses more open tree passages to avoid a doorway at nearly every cell; some complete 6 by 6-cell regions instead form open service chambers with a few structural pillars. Tunnel roof ledges meet solid walls and small hangers attach overhead duct covers. Stable hashes control asymmetric openings, props, room-level light layouts, rare distant entrances and entities. Recreating a chunk from the same seed gives the same geometry and collision, including across negative coordinates. Geometry-only revisions can retain the internal hash recipe so landmarks remain at reproducible locations.

The game retains at most a 5 by 5 neighborhood of chunks around the player. Each update prepares at most one missing chunk and uploads its CNA vertex buffers. Distant chunks are removed; a bounded pool holds up to 48 spare vertex buffers for reuse, then destroys excess buffers. The streamer holds a CNA renderer context token while uploading and retiring buffers; see [CNA findings](bugs.md). Collision queries use the same deterministic walls, props and maintenance frames, independent of loaded geometry. Chunk vertices are local to their chunk to reduce float jitter far from the origin. A game-owned cache keeps at most 256 room layouts; uncached generation has identical results. Native `BoundingFrustum` checks reject whole chunks outside the camera, using bounds computed from their actual vertices. This does not change streaming or collision. The title reports loaded/drawn chunks and triangles, buffer reuse, packed buffer capacity, cached rooms, entities, camera angles, build time, walk/run mode, audio state, actual draw FPS, rolling 120-frame p95/maximum intervals, CPU submission time and update rate (UPS). Slow builds log geometry/upload times separately.

## Assets and limitations

Geometry and most reusable textures are generated in code. Original generated wallpaper, loop-pile carpet and concrete PNGs add faded office ornament, worn textile detail and mineral texture; CNA decodes/resizes them and the game builds mipmaps; native anisotropic sampling preserves shallow-angle detail. Industrial and tunnel walls/floors share one concrete texture with independent family tints and baked colors. Industrial gray multipliers balance the material brightness; procedural fallbacks remain for each surface. Office walls use height-dependent baked fluorescent influence, with wall obstruction and local cache ownership in `Lighting.cpp`. Procedural fallbacks remain for missing or undecodable files. Four CC0 sounds from the NOX SOUND Essentials Series provide fluorescent hum, muted carpet steps, hard-floor steps and transition cues. See [asset licenses](assets/LICENSE.md) and [wallpaper record](assets/wallpaper-generation.md) and [carpet record](assets/carpet-generation.md) and [concrete record](assets/concrete-generation.md). Sparse chairs use a generated fabric material and thin steel frames; tables have wooden tops and conservative shared collision bounds. Furniture and figures are simple low-poly meshes. The tapered cloth figures share one immutable mesh, wander in place and cannot interact with the player. They fade smoothly as the player approaches, with depth-resolved native alpha blending and matching fading contact spots; they are fully absent within 1.5 metres.

Chunk generation is synchronous and capped at one upload per update. Walking is flat; collision uses a horizontal circle, not full character physics. The world has no save file, vertical traversal, moving doors or authored story. Linux desktop `OPENGLES3` is the only intended platform for this milestone.

## Validation

The product-quality goal remains active. Builds and numeric tests do not complete the visual audit. Current evidence and remaining deficiencies are recorded in [STATUS.md](STATUS.md).

Before format 30, the title labeled fixed-step **update rate** as FPS; historical values near 59 are UPS, not measured rendering rates. Format 30 separates the two. On the isolated Weston/Xwayland test display, current views and finite sweeps draw around 39–41 FPS; the game submits geometry in under a millisecond in ordinary sampled views. These display-specific measurements do not establish real desktop presentation performance.

Debug/Release builds, deterministic world tests, native JSON profile tests and lighting boundary/eviction tests pass. `world_quality` samples seven seeds per level. `world_walkability` checks 63 complete 90-metre squares using actual collision at one-metre spacing; all have one substantial connected component. Live tests cover conventional mouse look, Shift toggling, collision and all directed entrances. A game-side CNA context workaround prevents stale reused buffers; see [bugs.md](bugs.md).

Repeated screenshot rounds cover six Level 0 locations and three each in Levels 1 and 2, with distant multi-seed and controller-route views. `python3 tools/capture_views.py --game build/cna_backrooms --output build/visual-qa` captures the standard twelve views; add `--distant` for eighteen locations hundreds of metres away and `--all-directions` for four headings at each location. Use `--sampled --world-quality build/world_quality` for twelve uncurated off-grid positions and headings across six new seeds, up to four kilometres from the origin. Selection rejects collision and entrances; it does not select attractive compositions. Use `--partitions --world-quality build/world_quality --all-directions` for six reproducible office partition plans. Use `--entities --world-quality build/world_quality` for eighteen near/mid/distant views across two seeds and all three levels; recorded camera angles guard against incomplete turns. The tools require X11/Xwayland, `xdotool` and ImageMagick. Use `--level 1 --pitch -25` for industrial floor detail. `python3 tools/figure_approach.py --game build/cna_backrooms --world-quality build/world_quality` walks clear six-metre approach/return paths in all three families, checks actual position and mouse angles, and captures the proximity fade. Use a separate test display; screenshots, logs and manifests go under ignored `build/` paths.

Actual controller returns cover 1.16 km in Level 0 and 449 m on the current tunnel material, with longer earlier family-specific walks. Streaming sweeps reach 9.6 km in the offices and 28.8 km in the tunnels with bounded chunks/buffer pools. Exact timings, memory ranges and limitations are retained in [validation history](docs/validation-history.md) and the current status. These are finite observations, not unlimited-duration guarantees.

For a real-controller return walk, build `world_route`, generate a route and feed it to the driver:

```sh
cmake --build build --target world_quality world_walkability world_route
./build/world_route 1 12345 -122.5 -117.5 > build/route.json
python3 tools/controller_walk.py --game build/cna_backrooms --route-file build/route.json --speed 2.4
```

The route planner is a QA tool, not in-game pathfinding. It leaves clearance around walls, avoids entrances and records the profile fingerprint. The driver walks out and back using actual input/collision and records views, chunk counts and RSS. `world_quality --alcoves` reports rare office recess locations. `python3 tools/validate_alcoves.py --game build/cna_backrooms --world-quality build/world_quality` tests entering, wall collision and leaving recesses in all four directions. Without `--route-file`, the walk driver follows the current Level 0 regression route to the original pooled-buffer failure location; inspect `pooled-buffer-regression.png` for a complete floor and ceiling.

Fresh captures from the game's own normal PipeWire/PulseAudio stream reach the unmuted Ryzen speaker output. Hum measures about -32.3 dBFS RMS, footsteps peak at -10.8 dBFS, and an isolated transition without footsteps peaks at -13.5 dBFS, with no clipping. `python3 tools/validate_audio.py --game build/cna_backrooms` records routing and events; add `--isolated-transition` to separate the cue. This technical capture does not establish subjective audibility or loudness, which remains a human listening check. Screenshot and streaming runs use SDL's dummy audio device.
