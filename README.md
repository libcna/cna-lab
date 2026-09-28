# cna-backrooms

A small first-person Backrooms exploration game in C++ and CNA. Walk through deterministic, effectively unbounded rooms, storage spaces, and maintenance tunnels. There is no combat, damage, quest, inventory, or game over. The occasional distant figure is harmless.

## Build and run

The supported target is **Linux desktop, CNA's EasyGL `OPENGLES3` renderer**. Put `cna` and `sharp-runtime` beside this repository, both on their `next` branches. You need CMake 3.21+, a C++23 compiler, and CNA's SDL/OpenGL ES dependencies.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target cna_backrooms world_tests level_profile_tests --parallel
./build/world_tests
./build/level_profile_tests
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

The world format is algorithm version `23`, a 64-bit seed, and a level id (`0`–`2`). The three families are tuned by a small versioned [level-definition file](docs/level-format.md), loaded through Sharp Runtime. Startup logs record its source fingerprint; a missing file uses built-in defaults and an invalid file reports an error. There are no chunk files. Integer cells are 5 metres wide; chunks are 8 by 8 cells. Six by six-cell regions pick spatial patterns, including open rooms, columns, irregular rooms, storage, and tunnels. Level 0's enclosed regions group cells into larger irregular room zones; walls follow zone boundaries, with one composed entrance per neighboring connected room pair. Open and column rooms use fewer cell-boundary fragments, relying on columns and longer offset walls. Longer room partitions sit away from cell lines, with shared deterministic geometry and collision. Rare selected 12 by 12-cell areas form unbroken 60-metre empty halls. Contracting the deterministic cell tree onto connected rooms and retaining shared border connectors preserves connectivity. Level 1 has larger connected service bays, cast-concrete column rooms and enclosed multi-cell spaces; storage bays choose one of several sparse shelf layouts and orientations, with two to four structural supports and few internal wall fragments. Region-level industrial fixtures and baked light obstruction give the concrete spaces their own identity. Level 2 uses more open tree passages to avoid a doorway at nearly every cell; some complete 6 by 6-cell regions instead form open service chambers with a few structural pillars. Stable hashes control asymmetric openings, props, room-level light layouts, rare distant entrances and entities. Recreating a chunk from the same seed gives the same geometry and collision, including across negative coordinates. Geometry-only revisions can retain the internal hash recipe so landmarks remain at reproducible locations.

The game retains at most a 5 by 5 neighborhood of chunks around the player. Each update prepares at most one missing chunk and uploads its CNA vertex buffers. Distant chunks are removed; a bounded pool holds up to 48 spare vertex buffers for reuse, then destroys excess buffers. The streamer holds a CNA renderer context token while uploading and retiring buffers; see [CNA findings](bugs.md). Collision queries use the same deterministic walls, props and maintenance frames, independent of loaded geometry. Chunk vertices are local to their chunk to reduce float jitter far from the origin. A game-owned cache keeps at most 256 room layouts; uncached generation has identical results. The window title reports position, chunk, active chunks, triangles, buffer reuse, cached rooms, entities, build time, walk/run mode, audio state and FPS.

## Assets and limitations

Geometry and most reusable textures are generated in code. An original generated wallpaper PNG adds faded office ornament and paper detail; CNA decodes/resizes it and the game builds mipmaps. A procedural fallback remains. Four CC0 sounds from the NOX SOUND Essentials Series provide fluorescent hum, muted carpet steps, hard-floor steps and transition cues. See [asset licenses](assets/LICENSE.md) and [generation record](assets/wallpaper-generation.md). Sparse chairs use a generated fabric material and thin steel frames; tables have wooden tops and conservative shared collision bounds. Furniture and figures are simple low-poly meshes. The figures wander in place and cannot interact with the player.

Chunk generation is synchronous and capped at one upload per update. Walking is flat; collision uses a horizontal circle, not full character physics. The world has no save file, vertical traversal, moving doors or authored story. Linux desktop `OPENGLES3` is the only intended platform for this milestone.

## Validation

The product-quality goal remains active. Builds and numeric tests do not complete the visual audit. Current evidence and remaining deficiencies are recorded in [STATUS.md](STATUS.md).

Debug/Release builds, deterministic world tests and native JSON profile tests pass. `world_quality` samples seven seeds per level. `world_walkability` checks 63 complete 90-metre squares using actual collision at one-metre spacing; all have one substantial connected component. Live tests cover conventional mouse look, Shift toggling, collision and all directed entrances. A game-side CNA context workaround prevents stale reused buffers; see [bugs.md](bugs.md).

Repeated screenshot rounds cover six Level 0 locations and three each in Levels 1 and 2, with distant multi-seed and controller-route views. `python3 tools/capture_views.py --game build/cna_backrooms --output build/visual-qa` captures the standard twelve views; add `--distant` for eighteen views hundreds of metres away. The tools require X11/Xwayland, `xdotool` and ImageMagick. Use a separate test display; screenshots, logs and manifests go under ignored `build/` paths.

On format 22, two Level 0 controller return walks covered 355 m and 770 m across two seeds near 59 FPS, maximum 25 chunks and 3.06/3.28 ms peak builds. A 9.6 km sweep held at most 256 cached layouts, 3.82 ms peak and latter-half RSS 179.1–179.4 MiB. The refined tunnels completed 28.8 km near 59 FPS, 7.20 ms peak; final twelve RSS samples were 291.4–292.2 MiB as larger buffer capacities settled. These observations describe finite runs, not unlimited-duration guarantees. Format 23 preserves the default recipe and verifies packaged/file-fallback profiles in actual launches.

For a real-controller return walk, build `world_route`, generate a route and feed it to the driver:

```sh
cmake --build build --target world_quality world_walkability world_route
./build/world_route 1 12345 -122.5 -117.5 > build/route.json
python3 tools/controller_walk.py --game build/cna_backrooms --route-file build/route.json --speed 2.4
```

The route planner is a QA tool, not in-game pathfinding. It leaves clearance around walls, avoids entrances and records the profile fingerprint. The driver walks out and back using actual input/collision and records views, chunk counts and RSS. Without `--route-file`, it follows the current Level 0 regression route to the original pooled-buffer failure location; inspect `pooled-buffer-regression.png` for a complete floor and ceiling.

Audio was captured from the game's own real PulseAudio/PipeWire sink input routed to the Ryzen hardware speaker output. Hum measured about -32 dBFS RMS, walking peaks near -10 dBFS, and the isolated transition stayed below -9.6 dBFS, with no clipping. The hardware sink was muted in this agent environment, so subjective audibility/loudness remains a real-machine listening check. Screenshot and streaming runs use SDL's dummy audio device.
