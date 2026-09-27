# cna-backrooms

A small first-person Backrooms exploration game in C++ and CNA. Walk through a deterministic, effectively unbounded office grid. There is no combat, damage, quest, inventory, or game over.

## Build and run

This milestone targets **Linux desktop with CNA's EasyGL `OPENGLES3` renderer**. Place the `cna` and `sharp-runtime` repositories beside this one, both on their `next` branches. You need CMake 3.21+, a C++23 compiler, and CNA's SDL/OpenGL ES build dependencies.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target cna_backrooms world_tests --parallel
./build/world_tests
./build/cna_backrooms --seed 31337
```

CNA normally builds its vendored SDL dependencies. If you have installed SDL3, SDL3_image, and SDL3_mixer CMake packages, configure with `-DCNA_USE_SYSTEM_SDL=ON` instead. This checkout was validated using CNA's existing SDL install as a package prefix:

```sh
cmake -S . -B build -DCNA_USE_SYSTEM_SDL=ON \
  -DCMAKE_PREFIX_PATH=../cna/.sdl-prebuilt-Linux-x86_64-wayland/install
```

Run from the repository root or `build/` so the three WAV assets are found. The game still runs if audio cannot load. `--seed` accepts an unsigned integer (decimal or `0x` hexadecimal); omission uses the default seed.

## Controls

| Input | Action |
| --- | --- |
| Mouse | Look |
| W/A/S/D | Move |
| Left Shift | Move faster |
| R | Return to the current level's spawn |
| Escape | Release mouse; press again to quit |
| Left click | Recapture mouse |

Walk over the cyan-lit floor patch to change levels. From the initial Level 0 spawn, it is about 15 metres straight ahead. Level 1's return patch is about 15 metres straight ahead from its spawn.

## World and streaming

Level 0 uses yellow walls, dull carpet, a ceiling grid, fluorescent panels, and repeating rooms and doorways. Level 1 uses a cooler concrete and service-space palette. Both are generated from the same structural rules; the palette and lighting differ.

The world format is algorithm version `1`, a 64-bit seed, and a level id (`0` or `1`). No chunk files are needed. Integer cells measure 5 metres; each chunk is 8 by 8 cells. A stable hash of global cell coordinates decides wall openings and subtle color variation. Shared chunk borders agree because every edge is derived from its global coordinate. Regular open rows and columns guarantee long routes through the world.

The game keeps at most a 5 by 5 neighborhood of chunks around the player. It creates at most one missing chunk and GPU vertex buffer per update, nearest first, and destroys buffers outside the radius. Collision queries regenerate nearby wall rectangles directly from the same seed, independent of loaded geometry. Chunk vertices are local to the chunk and rendered relative to the camera, avoiding large-world float jitter. The title bar reports level, seed, position, chunk, active chunks, triangle count, generation time, and FPS.

## Assets and limitations

The geometry and colors are generated in code. Three CC0 sounds from the NOX SOUND Essentials Series add fluorescent hum, footsteps, and a transition cue; see [asset license](assets/LICENSE.md) and the bundled source README PDF.

This first release has simple colored geometry rather than textures, no creatures, no save file, and no handcrafted landmarks beyond the transition patch. Chunk generation is synchronous but limited to one chunk per update. Only Linux desktop `OPENGLES3` is an intended target for this milestone. Movement is flat and wall collision uses a horizontal circle rather than full player physics.

## Validation performed

The fresh Linux `OPENGLES3` build and `world_tests` passed. Private-display checks confirmed launch, rendered Level 0 and Level 1, mouse look, movement through the transition patch, and a long out-and-back walk across several chunk boundaries. During that walk, active chunks stayed at 25, the title reported about 59 FPS, and process resident memory did not grow. The private display used SDL's dummy audio device, so audible output still needs a normal desktop check.
