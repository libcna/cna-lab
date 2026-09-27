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

Completed passes: the longer positive/negative coordinate sweep exposed steady memory growth from repeated vertex-buffer creation. A bounded buffer pool eliminated that growth in a repeat sweep. Region connectivity now uses a deterministic tree instead of long forced open lines. Screenshots checked the low partition, furniture, false door, storage rack, and darker tunnel family.

Current backlog: (1) verify the larger Level 2 geometry over a long sweep, (2) recheck its lighting after the entrance fixture change, (3) perform another play/collision pass around unusual props and transitions, (4) stabilize and commit the verified stage. Keep core streaming and collision correct while cutting optional polish if time is tight.
