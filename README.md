# cna-backrooms

A small first-person Backrooms exploration game in C++ and CNA. Walk through deterministic, effectively unbounded rooms, storage spaces, and maintenance tunnels. There is no combat, damage, quest, inventory, or game over. Rare wanderers, tall watchers and low crawlers are harmless.

## Build and run

The supported target is **Linux desktop, CNA's EasyGL `OPENGLES3` renderer**. Put `cna` and `sharp-runtime` beside this repository, both on their `next` branches. You need CMake 3.21+, a C++23 compiler, and CNA's SDL/OpenGL ES dependencies.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target cna_backrooms world_tests level_profile_tests lighting_tests --parallel
ctest --test-dir build --output-on-failure
./build/cna_backrooms --seed 31337
```

The standalone build disables CNA’s dependency-wide developer source-inventory audit; the reason is recorded in [bugs.md](bugs.md). Game tests remain enabled.

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

The generated world uses algorithm version `49`, a 64-bit seed and one of the
three [level profiles](docs/level-format.md). Six-cell regions compose connected
room zones, asymmetric entrances and offset partitions; rare 60-metre halls use
coherent ceiling-light plans. Geometry and collision regenerate deterministically,
including at negative coordinates. [Generation details](docs/world-generation.md)
explain the shared border rules and material bake.

Only a 5 by 5 neighborhood of 40-metre chunks stays active. Each update builds
at most one missing chunk; distant chunks unload. Buffer reuse (48 spares) and
room caching (256 layouts) are bounded. Geometry is chunk-relative and native
frustum checks reduce drawing. The window title reports positions, loaded/drawn
chunks, triangles, buffers, memory capacity, entities, loading times, actual FPS
and UPS. A required game-side context lease is documented in [bugs.md](bugs.md).

## Assets and limitations

Geometry and most reusable textures are generated in code. Original generated wallpaper, loop-pile carpet and concrete PNGs add faded office ornament, worn textile detail and mineral texture; CNA decodes/resizes them and the game builds mipmaps; native anisotropic sampling preserves shallow-angle detail. Industrial and tunnel walls/floors share one concrete texture with independent family tints and baked colors. Industrial gray multipliers balance the material brightness; procedural fallbacks remain for each surface. Office walls use height-dependent baked fluorescent influence, with wall obstruction and local cache ownership in `Lighting.cpp`. Procedural fallbacks remain for missing or undecodable files. Seven CC0 sounds from the NOX SOUND Essentials Series provide fluorescent hum, muted carpet steps, hard-floor steps, transition cues and rare creature breaths/rustles. See [asset licenses](assets/LICENSE.md) and [wallpaper record](assets/wallpaper-generation.md) and [carpet record](assets/carpet-generation.md) and [concrete record](assets/concrete-generation.md). Sparse chairs use a generated fabric material and thin steel frames; tables have wooden tops and conservative shared collision bounds. Tunnel pipe runs query adjacent wall plans across chunk borders and turn through their supporting walls at exposed ends. Furniture and figures are simple low-poly meshes. Three distinct silhouettes share one immutable mesh per type: a clothed wanderer, a tall narrow watcher and a low four-legged crawler. They drift slowly within their original clear cell and cannot interact with the player. Creature sounds use one global 18–31-second cooldown, line-of-sight checks, distance attenuation and stereo pan; no per-creature history accumulates during exploration. They fade smoothly as the player approaches, with depth-resolved native alpha blending and matching fading contact spots; they are fully absent within 1.5 metres.

Chunk generation is synchronous and capped at one upload per update. Walking is flat; collision uses a horizontal circle, not full character physics. Lighting is an inexpensive bake, and large-room floor pools remain approximate. The world has no save file, vertical traversal, moving doors or authored story. Linux desktop `OPENGLES3` is the only intended platform for this milestone.

## Validation

Debug/Release deterministic, profile and lighting suites pass. Real controller
checks cover mouse directions, walk/run toggling, collision, entrances and
harmless creature approaches. Repeated screenshot audits and long return walks
are recorded in [validation history](docs/validation-history.md).
[Validation tools](docs/validation-tools.md) reproduce visual, audio and streaming
checks; [STATUS.md](STATUS.md) records the current pass and remaining work.

Private-display rendering is around 39 FPS with about 59 updates per second;
these measurements do not establish normal desktop presentation performance.
Own-stream captures verify audio routing and signals. Subjective sound quality
and native Wayland input still require real-machine checks.

[Known limitations](docs/known-limitations.md) records the remaining lighting,
animation and desktop qualification limits, including the validated Xwayland
launch command.

See the [delivery screenshots](docs/screenshots.md), [product audit](docs/product-audit.md) and [known limitations](docs/known-limitations.md).
