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

Run from the repository root or `build/` so the three WAV assets are found. Missing or unavailable audio is logged and the game continues. `--seed` accepts an unsigned decimal or `0x` hexadecimal integer; the default is reproducible. `--level 0|1|2 --position x z` starts at a chosen valid point for inspecting procedural rooms. `--walk-speed` and `--run-speed` set movement speeds in metres per second (defaults: 3.8 and 6.5). `--stream-test` runs a 9.6 km automated GPU streaming sweep and exits.

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

- **Level 0, The Yellow Rooms:** patterned yellow wallpaper, stained office carpet, suspended ceiling tiles, fluorescent panels, mixed room sizes, columns, low partitions, rare furniture and false doors.
- **Level 1, Service Storage:** tall concrete spaces, support pillars, shelves and industrial lighting.
- **Level 2, Maintenance Tunnels:** lower ceilings, darker rust-colored materials, narrower passages and utility geometry.

The world format is algorithm version `5`, a 64-bit seed, and a level id (`0`–`2`). There are no chunk files. Integer cells are 5 metres wide; chunks are 8 by 8 cells. Six by six-cell regions pick spatial patterns, including open rooms, columns, irregular rooms, storage, and tunnels. Level 0's enclosed regions group cells into larger irregular room zones; walls follow zone boundaries. A deterministic tree within each region and shared border connectors preserve connectivity. Stable hashes control openings, props, room-level light layouts, rare distant entrances and entities. Recreating a chunk from the same seed gives the same geometry, including across negative coordinates.

The game retains at most a 5 by 5 neighborhood of chunks around the player. Each update prepares at most one missing chunk and uploads its CNA vertex buffers. Distant chunks are removed; a bounded pool holds up to 24 spare vertex buffers for reuse, then destroys excess buffers. Collision queries use the same deterministic walls, props and maintenance frames, independent of loaded geometry. Chunk vertices are local to their chunk to reduce float jitter far from the origin. The window title reports position, chunk, active chunks, triangles, buffer reuse, entities, build time, walk/run mode, audio state and FPS.

## Assets and limitations

Geometry and reusable textures are generated in code. Three CC0 sounds from the NOX SOUND Essentials Series provide fluorescent hum, footsteps and transition cues. See [asset license](assets/LICENSE.md). Furniture and figures are simple low-poly meshes. The figures wander in place and cannot interact with the player.

Chunk generation is synchronous and capped at one upload per update. Walking is flat; collision uses a horizontal circle, not full character physics. The world has no save file, vertical traversal, moving doors or authored story. Linux desktop `OPENGLES3` is the only intended platform for this milestone.

## Validation

The Linux Release `OPENGLES3` build and deterministic world tests pass. The tests execute in Release mode and check connectivity, negative coordinates, props, rare entrances, portal geometry and collisions. Private-display play checks covered conventional mouse look, Shift walk/run toggling, custom movement speeds, collision, furniture, low partitions, false doors, and all four directed level transitions after the version 5 room change. A distant generated entrance at negative coordinates also reached Level 1 in the earlier format. A real controller walk crossed a chunk boundary and returned with 25 active chunks and buffer reuse. The default seed and another seed both reached Level 2 in the earlier format. Twelve varied locations across all three levels were captured before and after the first visual pass, and again after room composition and lighting changes.

Separate 9.6 km live GPU sweeps of Levels 0 and 2 crossed positive and negative coordinates. Both held 24–25 active chunks near 59 FPS. The Level 2 sweep was repeated after the version 4 world change: peak chunk build was 25.7 ms, resident memory stayed within 195.4–196.4 MB after warmup, and the buffer pool reused 5,732 buffers. The earlier Level 0 sweep stayed below 179 MB after warmup; an earlier run without pooling had risen from roughly 172 to 192 MB. These are observations from the test distances, not a claim about unlimited-duration memory behavior.

Audio initialization and event playback have been checked through CNA. A live PulseAudio/PipeWire sink input was observed while the game ran with a normal audio backend. Subjective audibility still needs a person listening on a real machine; private-display runs use SDL's dummy audio device.
