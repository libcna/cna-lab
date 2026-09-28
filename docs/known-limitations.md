# Known limitations at the September milestone

These describe the implemented game and finite validation, rather than an
additional feature plan.

- **Lighting:** fluorescent influence and wall/floor depth are baked into vertices.
  Large-room floor pools remain approximate. Creatures have contact spots;
  they do not cast real moving shadows.
- **Composition:** rooms, offset partitions and support plans are deterministic
  and varied, but remain mostly axis aligned. Material repetition and empty
  spaces are intentional. This is a small set of architectural families.
- **Movement:** walking stays on a flat floor. Horizontal circle collision
  prevents ordinary wall traversal; there are no stairs, jumping or crouching.
- **Creatures:** three simple low-poly types drift within a clear cell, look
  toward the player and fade nearby. They have no articulated walking animation,
  physical obstacle, attack, damage, health or pursuit.
- **Streaming:** generation is incremental and synchronous. The measured fast
  four-lap tunnel sweep contains occasional chunk builds up to 39.31 ms and
  sampled frames up to 106.34 ms. These private-display measurements are not
  desktop-wide performance guarantees. Active chunks, buffers and room caches
  are bounded; the repeated-route memory result is in the validation history.
- **Audio:** normal PipeWire output and captured hum, footsteps, transitions
  and each creature voice are checked technically. Subjective loudness and
  quality have not been checked by listening.
- **Desktop qualification:** real mouse/keyboard/collision captures use SDL
  X11 under Xwayland. Native Wayland startup and long streaming pass, but its
  actual input is unverified on the headless test compositor. Its screenshot
  interface denies capture. On a Wayland desktop, the validated interactive
  path can be selected with the command below; the renderer remains OpenGL ES.
- **Persistence and interaction:** reproduction uses the seed, level, position,
  algorithm version and exact level definitions. There is no save file,
  inventory, combat, quest, moving door or authored story.

```sh
SDL_VIDEODRIVER=x11 ./build-clean/cna_backrooms
```

[README](../README.md) describes controls and ordinary builds.
[Validation history](validation-history.md) retains exact finite evidence.
[bugs.md](../bugs.md) records the CNA context workaround; this game does not
modify either sibling dependency.
