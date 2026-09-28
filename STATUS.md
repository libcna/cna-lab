# cna-backrooms status

## Delivery state

The September milestone is stabilized and delivered around the owner's
2026-09-28 20:00 Europe/Prague target. Final **world format 49**, recipe **11**;
implementation revision **975535a**. The dedicated visual/product audit passes
this low-budget exploration scope; exact evidence and qualifications are in
[product audit](docs/product-audit.md) and
[known limitations](docs/known-limitations.md).

Three connected, distinct families provide patterned office Backrooms,
industrial storage bays and mechanical tunnels. Conventional first-person
controls, Shift walk/run toggling, collision, environmental entrances and
bounded deterministic streaming work. Three harmless creature types use shared
meshes, bounded drift, near fade and sparse licensed voices. No combat or harm.

## Completed passes

- Materials/renderer: reusable original wallpaper, carpet and concrete;
  acoustic panel seams/fissures; native mipmaps, anisotropy/MSAA; warm baked
  illumination and wall/floor/contact response. No replacement renderer.
- Composition: connected zones, asymmetric openings, six offset partition
  plans, varied supports, occasional large halls and sparse furniture/false doors.
- Family geometry: storage supports/shelves/hanging fixtures, attached beams
  with closed undersides, service roof details and continuous pipe returns.
- Atmosphere: Wanderer, Watcher and Crawler, nine actual approaches/returns,
  99 inspected stages, licensed attenuated/panned CC0 voices.
- Validation: repeated real collision returns, negative coordinates,
  conventional mouse/recapture, six actual entrances and normal-output audio.

## Latest validation

Final Release/Debug compilation and all three CTest suites pass. Final format 49
controls, entrances and five normal-output captures finish successfully.
Hum RMS is -32.26 dBFS; walking peak -10.77; isolated transition peak -13.41;
creature peaks -14.66/-13.18/-11.38. The game stream is unmuted at 100%, speaker
sink unmuted at 57%; subjective listening is unperformed.

Three format 48 collision returns cover **1,186.3/528.3/448.1 m** in office,
storage and tunnels. All **104 panorama views** were inspected. Office warmed
RSS is 174.25–192.95 MiB; storage 184.53–185.36; tunnels 263.02–281.68.
Format 49 changes only industrial beam attachment/undersides, checked in 16
new views; room layout and horizontal collision are unchanged.

The four-lap native sweep completes **38.4 km** across 121 player chunks, with
maximum 25 active chunks/17 sampled spares, 339 buffers created/38,283 reused,
79.8 MiB peak packed capacity. RSS is flat late in laps two/three at 388.66 MiB
and finishes at 363.34. Median/peak build 5.14/39.31 ms; maximum sampled frame
106.34 ms. The sweep bypasses collision and uses dummy audio; actual-controller
and normal-output checks are separate. Shared scheduling is not isolated.

## Three remaining qualifications

1. Subjective sound balance/quality requires listening on the owner's machine;
   routing, event presence and levels are technically checked.
2. Native Wayland real input remains unqualified on the headless compositor.
   Startup/long streaming pass; SDL X11/Xwayland is the validated interactive path.
3. Occasional measured generation/presentation outliers remain. Lighting is
   baked, creature animation is simple drift, and room plans remain axis aligned.

## Dependencies and continuation

Observed next heads: CNA **200d08fb67f538317fca8363acb04e0363a19857**;
Sharp Runtime **007280bd1cc789f851f7f454a5041c8ce2479e13**. Siblings also have
external working-copy changes; this agent never modified them. The game-side
context lease and standalone developer-audit setting are documented in bugs.md.

No more features are scheduled for this delivery. Later work should qualify
subjective audio/native input and address measured spikes before optional
animation, saving or additional compositions. Start from README and
[world architecture](docs/world-generation.md); preserve bounded streaming and
harmless entities. The full finite pass history remains in
[validation history](docs/validation-history.md).

## Post-delivery package correction

The owner's copied build/bbb package now has run.sh to resolve its local CNA
library and assets without the deleted original build directory. A 400 m actual
isolated GPU launch check exits zero and confirms all packaged materials loaded.
The existing binary is unchanged; the CMake origin-relative setting applies to
future builds. This is local launch qualification, not a universal Linux bundle.
