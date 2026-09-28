# Level definitions

`assets/levels.json` is profile format `1`, with exactly three entries identified
by `id` 0, 1 and 2. These are the fixed office, storage and tunnel families.
Their geometry/material rules remain in C++; this file tunes this game rather
than describing arbitrary scenes.

| Field | Meaning |
| --- | --- |
| `name` | Level label in the window title |
| `ceiling_height`, `doorway_height` | Metres; a doorway must leave a lintel |
| `fog.start`, `fog.end`, `fog.color` | Metres and three RGB channels, 0–255 |
| `lighting.ambient` | Base baked illumination, 0.15–0.85 |
| `lighting.fluorescent_strength` | Local fixture contribution, 0.1–0.9 |
| `lighting.wall_bounce`, `ceiling_bounce` | Ambient blend for those surfaces |
| `tints` | RGB multipliers for generated/textured geometry |
| `regions` | Ordered `{kind, weight}` entries, unique kinds, total weight 100 |
| `entity_rarity` | Hash divisor, 250–100000; larger means fewer harmless figures |

Office kinds are `open_office`, `columns`, `rooms`, `halls`, `irregular`.
Storage uses `storage`, `open_office`, `columns`, `halls`, `rooms`.
Tunnels use `tunnels`, `irregular`, `halls`. Region entry order affects the
seeded selection. Origin storage/tunnel regions and first entrances remain
fixed so a new player can reach the other families.

The game and world QA tools load the file once using Sharp Runtime's File and
JsonDocument APIs. The executable's adjacent assets take precedence over the
working directory. Rebuild to copy edited source assets beside the executable.
A missing file uses built-in defaults; an invalid file reports its path and
stops. `level_profile_tests` checks that packaged definitions match the defaults
and rejects invalid versions, dimensions, colors and room weights.

A reproduction record consists of world algorithm version, seed, level and
the exact profile source. Startup logs print an FNV-1a source-byte fingerprint;
whitespace changes that fingerprint too. It is a diagnostic identifier, not a
security checksum. Route JSON includes this identifier, and controller QA
rejects a route whose definitions differ from the running game. Retain the
JSON alongside any seed used for a custom configuration.

Chunks still regenerate from coordinates. There are no scene/chunk files,
binary caches, runtime profile editing or asset bundles inside the format.
