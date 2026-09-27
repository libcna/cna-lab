# cna-backrooms: first playable milestone

Target: Linux desktop, CNA `next`, sharp-runtime `next`, EasyGL `OPENGLES3`. Budget: about 24 hours. Build the walking loop first and stop adding features once it is stable.

| Priority | Scope | Estimate |
| --- | --- | ---: |
| MUST HAVE | CNA build/launch, first-person controls, wall collision, recognizable Level 0, deterministic chunks, bounded streaming | 10-14 h |
| SHOULD HAVE | second palette, physical level transition, streaming telemetry, deterministic tests, extended walking validation | 5-7 h |
| OPTIONAL | creatures, audio, procedural textures, save file, further levels | remaining time |

The world is an unbounded integer cell grid. Each 8x8-cell chunk is regenerated from a versioned seed and global cell coordinates. Doorways on shared cell edges use the same hash from either side. Only nearby GPU buffers stay live. No disk chunk format or third-party assets are needed for this milestone; the seed, algorithm version, and level id are the reproducible world format.

Checkpoints: (1) compile and launch one room, (2) navigate generated cells with collision, (3) stream and unload chunks, (4) transition and validate a long walk, (5) document and commit. Cut optional work in that order if time is tight.
