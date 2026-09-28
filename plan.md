# cna-backrooms: development plan

Target: Linux desktop, CNA `next`, sharp-runtime `next`, EasyGL `OPENGLES3`.
The original budget target was about 24 hours. The owner's revised delivery
schedule is 2026-09-28 around 20:00 Europe/Prague: continue implementation and
product QA until about 19:30, then stabilize, document remaining deficiencies
and commit the delivered state during the final half hour. The initial prototype
was Phase 0; a successful build is not completion of the visual product goal.

| Priority | Scope | State |
| --- | --- | --- |
| MUST HAVE | Launch, conventional first-person controls, collision, recognizable Level 0, deterministic connected generation, bounded streaming | Implemented; visual refinement remains active |
| SHOULD HAVE | Distinct industrial and tunnel families, environmental transitions, harmless figures, audio, long traversal | Implemented; current composition and runtime passes below |
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

## Current backlog and remaining target allocation

Industrial bay composition, mineral surfaces, opening returns, attached service
roof details, smooth figure approaches and continuous pipe returns are complete
through format 43. The remaining delivery window prioritizes product evidence:

1. **Office/product QA:** vary the uniform column-room layouts, audit the default
   seed and additional arbitrary family locations, then walk actual collision
   returns. Floor/junction passes and remote weak/broad-room returns are complete.
   Fix the largest remaining material, scale, lighting or composition defects.
2. **Long-run stability:** repeat the finite tunnel capacity sweep after the new
   geometry and inspect positive/negative/return streaming for growth or spikes.
3. **Atmosphere and runtime audit:** refresh audio routing after external runtime
   changes, inspect additional family views and verify live transitions.
4. **19:30 stabilization:** stop feature additions, rebuild/test/run the delivered
   state, document remaining issues and commit for the owner's 20:00 target.

Continue achievable visual or gameplay improvements until the stabilization
window if these checks finish early. No new renderer or engine framework is needed.

These are scope estimates, not permission to stop at a milestone. Continue the
highest-value achievable deficiency while the goal is unsatisfied. Cut optional
features before weakening walking, collision, streaming or maintainability.
STATUS.md is current project memory; docs/validation-history.md retains exact
finite measurements and limitations. Screenshots and passing tests alone do not
establish that the complete visual/product audit has passed.
