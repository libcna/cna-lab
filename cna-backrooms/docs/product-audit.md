# September delivery product audit

Date: 2026-09-28, final world format **49**, generation recipe **11**.
Implementation revision: **975535a**. Target: Linux desktop, EasyGL OpenGL ES 3.
This is the low-budget exploration milestone, not a claim of photorealism or
unlimited test coverage. The technical skeleton has undergone repeated material,
lighting, room-composition, ceiling, family-identity and actual-controller passes.

## Visual acceptance

| Question | Assessment and evidence |
| --- | --- |
| Does Level 0 immediately read as classic Backrooms? | Yes in the inspected default-route and sampled-seed views: patterned yellow walls, worn brown office carpet, low acoustic ceiling and inset fluorescent fixtures. |
| Do views look like locations rather than procedural cells? | Broad rooms, offset partitions, narrower connections, column plans, alcoves and occasional large halls create distinct compositions. All 52 final default-route directions were inspected. |
| Do the four main materials read correctly? | Wallpaper has a repeated printed motif, carpet has directional fibers/stains, acoustic panels have seams and fine fissures, and fluorescents have recessed frames. Overly bold ceiling fissures were rejected in matched captures. |
| Does lighting create depth? | Warm baked fixture influence, height response, corner/contact shading and restrained fog distinguish surfaces and local brightness. This remains an inexpensive approximation. |
| Is the underlying grid sufficiently hidden? | Walls are composed across room zones with asymmetric openings, offset partitions and multiple support plans; fixtures use room plans. Architecture remains axis aligned, but ordinary views no longer show one obvious wall per cell. |
| Are all three families distinct? | Office wallpaper/carpet/drop ceiling; taller concrete storage bays with supports/shelves/hanging lights; lower service spaces with pipes, tanks, cabinets and warmer fixtures. |
| Can the player wander without immediately exposing the recipe? | Final actual controller routes cover 1,186.3 m of the default office seed, 528.3 m of storage and 448.1 m of tunnels, including returns. Remote/multiple-seed views and prior broad-room passes supplement spawn views. Repetition is intentional; archetypes remain finite. |
| Are major fixable visual faults still visible? | Final inspection found industrial beams separated from the slab. The attachment and closed underside fix passed all 16 matched final views. Close-wall captures are retained rather than excluded from QA. No further blocking corruption, floor gap or floating attachment was identified in these samples. |
| Does important specified work remain? | Core exploration, three styles, environmental transitions, sparse furniture/false doors, harmless creatures, licensed sound, bounded streaming and maintainable definitions are implemented. Remaining qualification and deliberately limited features are listed separately. |

## Final engineering and runtime checks

- Final Release and Debug builds succeed; all three CTest suites pass in both.
- Conventional mouse directions, 2.4/4.8 m/s movement, Shift toggle/hold edges,
  Escape release and click recapture pass against the final executable.
- Six actual entrance cases pass: both directions of 0/1 and 1/2, outside-trigger
  rejection and doorway-frame collision.
- All 104 panorama views from the three final collision routes were inspected.
  Nine creature approaches/returns contribute another 99 inspected stages.
- Four native streaming laps cover **38.4 km**, 121 player chunks, maximum 25
  active chunks and 17 sampled spare buffers. RSS is flat late in laps two/three
  and decreases in lap four. This is finite evidence of bounded repeated-route
  behavior, not an assertion about every future driver or seed.
- Final normal-output captures contain hum, footsteps, transition and each
  creature voice without clipping. Subjective listening has not been performed.
- Three deterministic creature types have bounded idle drift and proximity fade.
  There are no attack, damage, pursuit, health, death or blocking mechanics.

## Delivery decision

The internal audit passes the intended small Backrooms exploration milestone.
The result is a textured, human-scale exploration game with distinct places and
stable bounded world state, rather than the original colored geometry skeleton.
The owner may assess the visual style differently; screenshots make the delivered
result reviewable. Later work should start with the explicit qualifications in
[known limitations](known-limitations.md), not a renderer rewrite.

[Gallery](screenshots.md) · [Validation history](validation-history.md) ·
[Build and controls](../README.md) · [Dependency workarounds](../bugs.md)
