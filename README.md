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

Run from the repository root or `build/` so the three WAV assets are found. Missing or unavailable audio is logged and the game continues. `--seed` accepts an unsigned decimal or `0x` hexadecimal integer; the default is reproducible. `--stream-test` runs a 9.6 km automated GPU streaming sweep and exits.

## Controls

| Input | Action |
| --- | --- |
| Mouse | Look |
| W/A/S/D | Move |
| Left Shift | Toggle walking and running |
| R | Return to the current level's spawn |
| Escape | Release mouse; press again to quit |
| Left click | Recapture mouse |

The dark maintenance entrances are level transitions. From the Level 0 spawn, one is about 15 metres ahead. Level 1 has a return entrance to the north and a Level 2 entrance to the east. Level 2 has a return entrance to the north. Walking into the entrance changes level immediately.

## Levels and world generation

- **Level 0, The Yellow Rooms:** patterned yellow wallpaper, stained office carpet, suspended ceiling tiles, fluorescent panels, mixed room sizes, columns, rare furniture and false doors.
- **Level 1, Service Storage:** tall concrete spaces, support pillars, shelves and industrial lighting.
- **Level 2, Maintenance Tunnels:** lower ceilings, darker rust-colored materials, narrower passages and utility geometry.

The world format is algorithm version `2`, a 64-bit seed, and a level id (`0`–`2`). There are no chunk files. Integer cells are 5 metres wide; chunks are 8 by 8 cells. Six by six-cell regions pick spatial patterns, including open rooms, columns, irregular rooms, storage, and tunnels. A deterministic hash controls region type, openings, props, lighting variation and entities. Region boundary connectors preserve connectivity, including across chunks and negative coordinates. Recreating a chunk from the same seed gives the same geometry.

The game retains at most a 5 by 5 neighborhood of chunks around the player. Each update prepares at most one missing chunk and uploads its CNA vertex buffers; distant chunks and buffers are destroyed. Collision queries use the same deterministic walls and prop positions, independent of loaded geometry. Chunk vertices are local to their chunk to reduce float jitter far from the origin. The window title reports position, chunk, active chunks, triangles, entities, build time, walk/run mode, audio state and FPS.

## Assets and limitations

Geometry and reusable textures are generated in code. Three CC0 sounds from the NOX SOUND Essentials Series provide fluorescent hum, footsteps and transition cues. See [asset license](assets/LICENSE.md). Furniture and figures are simple low-poly meshes. The figures wander in place and cannot interact with the player.

Chunk generation is synchronous and capped at one upload per update. Walking is flat; collision uses a horizontal circle, not full character physics. The world has no save file, vertical traversal, moving doors or authored story. Linux desktop `OPENGLES3` is the only intended platform for this milestone.

## Validation

The Linux `OPENGLES3` build and deterministic world tests pass. Private-display play checks covered conventional mouse look, walk/run toggling, collision, screenshots of all three levels, and Level 0→1→2 transitions. A 4.8 km live GPU out-and-back sweep stayed at or below 25 active chunks and about 59 FPS, with a peak chunk build near 25 ms; resident memory rose from about 171 MB after warmup to 180 MB, so longer memory observation remains useful. The current `--stream-test` also crosses negative coordinates and is intended for that follow-up.

Audio initialization and event playback have been checked through CNA. A live PulseAudio/PipeWire sink input was observed while the game ran with a normal audio backend. Subjective audibility still needs a person listening on a real machine; private-display runs use SDL's dummy audio device.
