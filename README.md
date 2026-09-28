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

Run from the repository root or `build/` so the three WAV assets are found. Missing or unavailable audio is logged and the game continues. `--seed` accepts an unsigned decimal or `0x` hexadecimal integer; the default is reproducible. `--level 0|1|2 --position x z` starts at a chosen valid point for inspecting procedural rooms. `--walk-speed` and `--run-speed` set movement speeds in metres per second (defaults: 3.8 and 6.5). `--stream-test` runs a 9.6 km automated GPU streaming sweep and exits; `--stream-test-metres 19200` uses another distance and implies the test mode.

## Controls

| Input | Action |
| --- | --- |
| Mouse | Look |
| W/A/S/D | Move |
| Shift | Toggle walking and running |
| R | Return to the current level's spawn |
| Escape | Release mouse; press again to quit |
| Left click | Recapture mouse |

The dark maintenance entrances are level transitions. From the Level 0 spawn, one is about 15 metres ahead. From the Level 1 spawn, the return entrance is about 15 metres to the right and the Level 2 entrance is about 15 metres ahead. Level 2's return entrance is about 15 metres ahead. Rare additional entrances can be found far from the origin. Walking into an entrance changes level immediately.

## Levels and world generation

- **Level 0, The Yellow Rooms:** patterned yellow wallpaper with broad stains, dirty office carpet, suspended ceiling tiles with rare stained panels, fluorescent fixtures with occasional failed circuits, mixed room sizes, columns, long offset partitions, rare furniture, false doors, and occasional large empty halls.
- **Level 1, Service Storage:** tall concrete spaces, mixed open bays and narrow links, sparse steel shelving with varied shelf spacing and cardboard contents, structural beams and industrial lighting.
- **Level 2, Maintenance Tunnels:** lower ceilings, darker rust-colored materials, narrower passages, longer open runs, several region-level pipe layouts, service cabinets, occasional overhead ducts, and rare open service chambers.

The world format is algorithm version `12`, a 64-bit seed, and a level id (`0`–`2`). There are no chunk files. Integer cells are 5 metres wide; chunks are 8 by 8 cells. Six by six-cell regions pick spatial patterns, including open rooms, columns, irregular rooms, storage, and tunnels. Level 0's enclosed regions group cells into larger irregular room zones; walls follow zone boundaries. Open and column rooms use fewer cell-boundary fragments, relying on columns and longer offset walls. Longer room partitions sit away from cell lines, with shared deterministic geometry and collision. Rare selected 12 by 12-cell areas form unbroken 60-metre empty halls. A deterministic tree within each region and shared border connectors preserve connectivity. Level 1 storage bays choose one of several sparse shelf layouts and orientations. Level 2 uses more open tree passages to avoid a doorway at nearly every cell; some complete 6 by 6-cell regions instead form open service chambers with a few structural pillars. Stable hashes control asymmetric openings, props, room-level light layouts, rare distant entrances and entities. Recreating a chunk from the same seed gives the same geometry and collision, including across negative coordinates. Geometry-only revisions can retain the internal hash recipe so landmarks remain at reproducible locations.

The game retains at most a 5 by 5 neighborhood of chunks around the player. Each update prepares at most one missing chunk and uploads its CNA vertex buffers. Distant chunks are removed; a bounded pool holds up to 48 spare vertex buffers for reuse, then destroys excess buffers. Collision queries use the same deterministic walls, props and maintenance frames, independent of loaded geometry. Chunk vertices are local to their chunk to reduce float jitter far from the origin. The window title reports position, chunk, active chunks, triangles, buffer reuse, entities, build time, walk/run mode, audio state and FPS.

## Assets and limitations

Geometry and reusable textures are generated in code. Three CC0 sounds from the NOX SOUND Essentials Series provide fluorescent hum, footsteps and transition cues. See [asset license](assets/LICENSE.md). Furniture and figures are simple low-poly meshes. The figures wander in place and cannot interact with the player.

Chunk generation is synchronous and capped at one upload per update. Walking is flat; collision uses a horizontal circle, not full character physics. The world has no save file, vertical traversal, moving doors or authored story. Linux desktop `OPENGLES3` is the only intended platform for this milestone.

## Validation

The deterministic world tests pass for format 12. They check variable storage layouts, connectivity, negative coordinates, props, rare entrances, asymmetric openings, empty halls, service chambers, portal geometry and collisions. A four-seed scan of more than 25,000 door and wide-opening centers caught and fixed one partition that narrowed a doorway. Private-display play checks covered conventional mouse look, Shift walk/run toggling, custom movement speeds, collision, furniture, low partitions, false doors, and all four directed level transitions. A distant generated entrance at negative coordinates reached Level 1 in an earlier format. A real controller walk crossed a chunk boundary and returned with 25 active chunks and buffer reuse. Multiple twelve-view screenshot rounds cover six Level 0 locations and three each from Levels 1 and 2, including format 12. Matched format 11/12 views across three seeds show fewer short boundary fragments in open offices and column rooms. Another twelve Level 0 views sampled four seeds at the spawn and two locations hundreds of metres away. One view exposed a silhouette around the camera; near entities now disappear before they can obstruct vision, and the exact location was recaptured to verify the fix.

Separate 9.6 km live GPU sweeps of Levels 0, 1 and 2 crossed positive and negative coordinates. They held 24–25 active chunks near 59 FPS. The format 10 Level 0 sweep peaked at 36.5 ms per chunk, ended with 5,692 buffer reuses, and warmed resident memory rose gradually from roughly 178 to 181 MB. The format 10 Level 1 sweep peaked at 26.3 ms per chunk, ended with 5,710 buffer reuses, and stabilized near 186 MB. The format 9 Level 2 sweep peaked at 25.0 ms per chunk, ended with 5,731 buffer reuses, and stabilized near 203 MB. These are observations from the test distances, not a claim about unlimited-duration memory behavior. `cmake --build build --target world_quality && ./build/world_quality` samples 128 by 128 cells for seven seeds per level and reports edge, dead-end and sightline distributions. In format 10 the seven-seed scan found no cells without an exit; the selected seed contains 1,200 empty-hall cells and 1,308 service-chamber cells in the sampled square.

The format 11 Level 0 sweep also covered 9.6 km near 59 FPS with at most 25 chunks. Its larger 48-buffer spare pool reduced fresh GPU buffer allocations from 972 to 181 versus the format 10 sweep, with 6,829 reuses. Peak chunk build time was 25.2 ms. Resident memory plateaued around 181.4 MB after warming. A separate 19.2 km format 10 sweep stayed bounded at 25 chunks but continued replacing buffers; its warmed resident memory rose roughly 5 MB, motivating the larger spare pool. A later 2.4 km Level 0 check with denser lighting meshes also held near 59 FPS and peaked at 24.9 ms per chunk. Format 12 completed a 2.4 km check near 59 FPS with a 24.4 ms peak and at most 25 chunks. The current format 12 Release build, world tests, quality scan, physical walkability audit and all four live transition directions passed.

Current Level 1 and Level 2 each completed another 9.6 km sweep near 59 FPS with at most 25 chunks. Level 1 peaked at 25.1 ms, reused 6,305 buffers and ended near 187.6 MB RSS; Level 2 peaked at 25.5 ms, reused 6,442 buffers and plateaued near 202.4 MB. `cmake --build build --target world_walkability && ./build/world_walkability` audits 63 complete 90-metre squares using the player's collision movement on a one-metre lattice. It caught a narrow gap beside a Level 0 partition; after increasing clearance, every sample has one substantial connected component. Tiny corner pockets below ten sample points are excluded from useful room space. A live Release controller check stopped against an offset wall, strafed past its end and continued through the passage.

Audio initialization and event playback have been checked through CNA. A live PulseAudio/PipeWire sink input was observed while the game ran and walked with a normal audio backend. Subjective audibility still needs a person listening on a real machine; screenshot and streaming runs use SDL's dummy audio device.
