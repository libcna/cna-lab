# cna-backrooms: development plan

Target: Linux desktop, CNA `next`, sharp-runtime `next`, EasyGL `OPENGLES3`. Budget target: about 24 hours. The initial walking prototype was Phase 0; visual, layout and validation passes are part of the playable milestone.

| Priority | Scope | Status |
| --- | --- | --- |
| MUST HAVE | Build, conventional first-person controls, collision, textured Level 0, deterministic connected world, bounded chunk streaming | Implemented and validated |
| SHOULD HAVE | Distinct industrial/storage and tunnel levels, environmental transitions, harmless entities, long traversal, audio diagnostics | Implemented and validated within the limits below |
| OPTIONAL | Extra props, visual effects, save file, ambient events | Sparse furniture and false doors added; others can be cut |

The world is an unbounded integer cell grid. Each 8 by 8-cell chunk is regenerated from algorithm version 6, seed, level and global cell coordinates. Six by six-cell regions select room patterns and shared border connectors. Level 0 enclosed regions group cells into larger room zones; openings shift off center with matching collision, and ceiling tiles and fluorescent layouts are independent of the structural cell grid. Rare maintenance entrances are generated from the same seed so transitions remain discoverable during long walks. Only nearby GPU buffers remain live. This is the complete world format for now; a general scene serialization format would add complexity without improving this game.

## Visual comparison and next passes

The [original Backrooms image and history](https://en.wikipedia.org/wiki/The_Backrooms) and [Level 0 description](https://backrooms-wiki.wikidot.com/level-0) emphasize yellow patterned walls, damp carpet, ceiling fluorescents, vacant spaces and long repetitive sightlines. Current screenshots have the main materials and ceiling language, but show too many perfectly straight cell boundaries and uniformly placed light strips. Rare chairs, tables, partly embedded chairs and false doors add uncanny details while preserving empty rooms.

Completed passes: the longer positive/negative coordinate sweep exposed steady memory growth from repeated vertex-buffer creation. A bounded buffer pool eliminated that growth in a repeat sweep. Region connectivity now uses a deterministic tree instead of long forced open lines. Screenshots checked the low partition, furniture, false door, storage rack, and darker tunnel family.

The heavier Level 2 scene completed another 9.6 km live GPU sweep after the version 4 changes, near 59 FPS with 25 or fewer active chunks and stable warmed resident memory. Release-mode tests check generated connectivity, negative coordinates, props, fixed and rare portals, and collisions at generated openings. Live movement crossed and returned over a chunk boundary; all four directed level transitions and a distant generated entrance were checked. A clean Release build passed its tests and launched on an isolated OpenGL ES GPU display; walking from its Level 0 spawn reached Level 1. Subjective audio loudness still requires a person listening on a normal audio device; the real PulseAudio/PipeWire sink path was verified.
