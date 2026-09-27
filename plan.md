# cna-backrooms: development plan

Target: Linux desktop, CNA `next`, sharp-runtime `next`, EasyGL `OPENGLES3`. Budget target: about 24 hours. The initial walking prototype was Phase 0; visual, layout and validation passes are part of the playable milestone.

| Priority | Scope | Status |
| --- | --- | --- |
| MUST HAVE | Build, conventional first-person controls, collision, textured Level 0, deterministic connected world, bounded chunk streaming | Implemented; continue play checks |
| SHOULD HAVE | Distinct industrial/storage and tunnel levels, environmental transitions, harmless entities, long traversal, audio diagnostics | Implemented; extend validation |
| OPTIONAL | Extra props, visual effects, save file, ambient events | Sparse furniture and false doors added; others can be cut |

The world is an unbounded integer cell grid. Each 8 by 8-cell chunk is regenerated from an algorithm version, seed, level and global cell coordinates. Six by six-cell regions select room patterns and shared border connectors. Only nearby GPU buffers remain live. This is the complete world format for now; a general scene serialization format would add complexity without improving this game.

## Visual comparison and next passes

The [original Backrooms image and history](https://en.wikipedia.org/wiki/The_Backrooms) and [Level 0 description](https://backrooms-wiki.wikidot.com/level-0) emphasize yellow patterned walls, damp carpet, ceiling fluorescents, vacant spaces and long repetitive sightlines. Current screenshots have the main materials and ceiling language, but show too many perfectly straight cell boundaries and uniformly placed light strips. Rare chairs, tables, partly embedded chairs and false doors add uncanny details while preserving empty rooms.

Current backlog: (1) complete a long positive/negative coordinate streaming sweep and inspect memory growth, (2) inspect screenshots away from the spawn route, especially portals and false doors, (3) improve spatial irregularity and light placement where visual checks justify it, (4) run a longer manual exploration/collision pass, (5) update documentation and commit stable stages. Keep core streaming and collision correct while cutting optional polish if time is tight.
