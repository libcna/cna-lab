# cna-backrooms: development plan

Target: Linux desktop, CNA `next`, sharp-runtime `next`, EasyGL `OPENGLES3`.
The original budget target was about 24 hours. The owner's revised delivery
schedule is 2026-09-28 around 20:00 Europe/Prague: continue implementation and
product QA until about 19:30, then stabilize, document remaining deficiencies
and commit the delivered state during the final half hour. The initial prototype
was Phase 0; a successful build is not completion of the visual product goal.

| Priority | Scope | State |
| --- | --- | --- |
| MUST HAVE | Launch, conventional first-person controls, collision, recognizable Level 0, deterministic connected generation, bounded streaming | Delivered; final product audit recorded |
| SHOULD HAVE | Distinct industrial and tunnel families, environmental transitions, harmless figures, audio, long traversal | Delivered; three types and normal-device audio checked |
| OPTIONAL | More props, advanced saving, async loading, additional effects | Sparse furniture and false doors exist; other work can be cut |

## Architecture and CNA audit

CNA supplies Game, GraphicsDevice, textured vertex buffers, BasicEffect, native
textures/mipmaps, anisotropic sampling, multisampling, fog and frustum helpers.
Its input APIs drive the controller; SoundEffect supplies licensed audio.
Sharp Runtime File/JsonDocument loads the three immutable level profiles.
Geometry uses boxes, quads and simple pipe/figure meshes. Paired screenshots
selected inexpensive baked fluorescent lighting; no new renderer is needed.

The versioned world consists of a seed, level, algorithm version and exact
profile bytes. Five-metre cells are an implementation detail. Six-cell regions
compose larger rooms; eight-cell chunks stream around the player. Shared border
connectors and contracted room trees preserve connectivity. Collision queries
regenerate the same boxes independently of loaded rendering. Only 25 chunks,
48 spare buffers and 256 cached room layouts remain live. Context leases around
uploads/deletion are a game-side workaround documented in bugs.md; sibling
repositories remain unmodified.

Office spaces combine room zones, asymmetric openings, six offset partition
plans, columns, rare 60-metre halls, shallow alcoves and sparse furniture.
Industrial spaces combine broad storage/column bays and enclosed utility rooms.
Tunnels use narrower pipe-lined spaces, cabinets and occasional service chambers.
Profiles tune family heights, fog, lighting, tints, ordered room weights and
entity rarity. This is a small generated-world recipe, not a generic scene format.

## Reference and completed passes

The [original image/history](https://en.wikipedia.org/wiki/The_Backrooms) and
[Level 0 description](https://backrooms-wiki.wikidot.com/level-0) guide patterned
yellow walls, worn carpet, acoustic ceilings, fluorescent illumination and vacant
irregular spaces. Original generated wallpaper/carpet and height-dependent baked
light now supply these materials. Human scale, ceiling grids, fixture placement,
wall thickness, contact shading, partitions and narrow-gap collision have gone
through repeated screenshot and actual-controller passes.

[Level 1](https://backrooms-wiki.wikidot.com/level-1) guides concrete warehouse/service
bays and supports; [Level 2](https://backrooms-wiki.wikidot.com/level-2) guides weathered
concrete and intrusive piping. Original mineral concrete, rounded pipe shading
and cast supports have been compared in matched views. Reference images are not
bundled assets. Tapered, faceless figures are atmospheric and cannot harm players.
Normal PipeWire captures verify hum, footsteps and an isolated transition cue on
an unmuted output; subjective listening remains a human check.

## Final delivery pass

Feature additions stopped in the stabilization window. Final acoustic-ceiling
refinement and the industrial beam attachment/underside correction were compared
in game captures; rejected candidates were not delivered. Three harmless creature
types and licensed, spatially attenuated voices are implemented.

The final pass includes clean Release compilation, current Release/Debug tests,
actual collision returns in all three families, conventional mouse/Shift/recapture
checks, six live entrance cases, normal-device hum/footstep/transition/creature
captures and a four-lap 38.4 km native streaming sweep. Full finite measurements
are in docs/validation-history.md. The screenshot gallery and dedicated product
audit record the visual result; known limitations state what remains unqualified.

No further feature work is planned in this delivery. Possible later development
should first address measured generation/presentation outliers and subjective
sound balance, then native Wayland interactive qualification. More room
compositions, articulated creature movement and saves are optional extensions.
CNA and Sharp Runtime were inspected only; neither sibling was modified.
