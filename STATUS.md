# cna-backrooms visual goal status

## Delivery schedule

Owner deadline: **2026-09-28 around 20:00 Europe/Prague (18:00 UTC)**.
Continue autonomous implementation and product QA until about **19:30 local
(17:30 UTC)**. Reserve the final half hour for stabilization, build/runtime
checks, documenting remaining deficiencies and committing the delivered state.
Hard outer limit for completing delivery: **20:55 local (18:55 UTC)**, when the
owner expects subscription/session termination. Aim to finish at 20:00; the
extra time is contingency for stabilization or committing, not new features.
This supersedes the original approximate 24-hour target; do not finish early
merely because a milestone passes. Check the actual clock at major checkpoints.

## Current state

The product goal remains **active**. Three distinct families, connected deterministic rooms, bounded 5-by-5 streaming, collision, conventional mouse look, Shift toggling, native profiles, licensed audio, transitions and harmless entities work. Level 0 has original wallpaper/carpet, acoustic ceilings, warmer height-dependent fluorescent light, irregular rooms, six offset partition plans, sparse furniture and alcoves. Builds and numeric tests do not finish the visual audit.

## Current pass

Format 41's native fluorescent diffusers now use one local UV rectangle and a
long-axis prism/tube texture. All forty-eight paired office directions, six
pitched ceiling pairs and twelve family views are inspected; Release/Debug suites
pass. A fresh 28.8 km tunnel capacity sweep is running. Next implementation:
close exposed cell-wall ends and the small missing outer corner wedges found in
uncurated office views, then smooth coarse floor illumination pools.

## Completed passes

- **41:** coherent fluorescent diffusers, including service fixtures and entrance lights. Forty-eight paired office directions, six close ceiling pairs and twelve family views inspected. Release/Debug suites pass. Additional reproducible office samples cover three new seeds and weak circuits.

- **40:** continuous tunnel pipe runs and curved wall returns. Thirty-six matched directions, twelve family views and sixteen route views inspected. Release/Debug suites pass; the 448.8 m collision return holds at most 25 chunks, with 10.07 ms peak build and final RSS 272.53 MiB.

- **39:** smooth harmless-figure retreat from 5.0 to 1.5 metres, native depth-only resolution and shared fading contact spots. All 33 controller pairs, 18 entity views and 12 family views inspected. Release/Debug suites and six live entrance cases pass.

- **38:** wall-supported roof ledges and ceiling-mounted duct hangers. Twelve tunnel pairs, twelve family views and sixteen route views inspected. Refreshed Release/Debug suites pass; the 448.3 m return completes all 60 waypoints in 269.9 seconds, maximum 25 chunks, 6.84 ms peak build and final RSS 246.70 MiB.

- **37:** painted, locally lit opening returns. Twenty-four office pairs, twelve family views and eleven route/regression views inspected. The 355.1 m return completes 28 waypoints in 191.6 seconds, at most 25 chunks and steady warmed RSS 187.79 MiB.

- **36:** mineral industrial walls/floors, quieter fallbacks and pitch-controlled QA. Nineteen matched material pairs, twelve family views, six fallback views and forty-eight uncurated directions were inspected. Release/Debug suites and the 9.6 km industrial sweep pass. The new views expose office soffit and tunnel beam problems for the next passes.

- **35:** complete open industrial bays, framing above supports and suspended fixtures below beam soffits. Sixteen composition pairs, sixteen fixture refinements, twelve family views and thirteen route views were inspected. Release/Debug CTest suites, seven-seed sampling, all 63 physical squares, the 528 m return and all six live entrance cases pass.

- **34:** tapered harmless cloth figures, rounded heads and soft foot contact spots. Eighteen matched pairs, eighteen grounded views, two twelve-view family rounds and eleven controller/regression views were inspected. Final suites and the 356 m return pass. Fresh normal-device captures verify hum, footsteps and an isolated cue on the unmuted hardware output; subjective listening remains unverified.

- **33:** original cast concrete and height/face samples on service supports. All 24 multi-seed pairs and twelve family views were inspected; Debug/Release suites pass. The 9.6 km tunnel sweep holds 25 chunks, median 39 draw FPS and 9.40 ms peak, but still has buffer-capacity RSS growth.
- **32:** eight-sided small pipes, continuous cylindrical UVs and shared circumferential shading. All nine matched pairs and Debug/Release tests pass; a 2.4 km sweep holds median 39 draw FPS, 25 chunks and 9.34 ms peak build.
- **31:** original granular concrete walls/floors, a shared native texture, two-metre UVs and separate procedural fallbacks. Nine material pairs, floor views, 24 distant directions and twelve final family views were inspected; the 449 m real return and missing/corrupt launches pass.
- **30:** native four-sample MSAA, whole-chunk native frustum checks and actual draw timing. Nine culled/unculled images are pixel-identical; 48 directional views and sixteen controller/regression views were inspected. Direct coordinate bounds remove expensive native object copies. Slow-build logs split geometry/upload time.
- **29:** long/L/T/staggered/short/U-shaped office partitions, with shared clipped geometry/collision and unchanged connectors. Twenty-four paired views, 32 distant directions, 24 controller and eight alcove views were inspected.
- **Earlier:** original office materials, human scale, local wall lighting, composed room entrances, alcoves/doors, industrial bays, weathered tunnels and bounded caches. Detailed results are in [validation history](docs/validation-history.md).

## Three highest-priority deficiencies

1. Original cell walls have uncapped exposed ends and small incomplete outer corner joints.
2. Broad office floor light pools show coarse triangular interpolation; refine only this bake if measurements permit.
3. Reassess column-room repetition, finish the fresh tunnel capacity measurement and refresh real audio routing. Subjective listening remains unavailable.

## Latest validation

- Format 41 Release/Debug builds and all three CTest suites pass. Two consecutive office sample selections are identical. All 48 baseline/new directions across twelve locations, six pitched fixture pairs and twelve family views were inspected. The diffuser no longer has repeated dark UV blocks; failed fixtures remain visibly unlit. Geometry/collision, fixture placement and source profiles are unchanged. Screenshot timing is not a performance comparison, and dummy audio is not listening evidence.

- Format 40 Release/Debug builds and all three CTest suites pass. The 448.8 m collision-enabled tunnel return completes 60 waypoints in 270.0 seconds, five player chunks and at most 25 active chunks. Peak sampled build is 10.07 ms; warmed RSS is 258.14–273.47 MiB, final 272.53 MiB, with 208 buffer creations and 313 reuses. Draw rate is about 39 FPS on the private display. The two-segment return replaces a costlier four-segment prototype; loaded spawn geometry rises about 30% from format 38. Thirty-six matched directions, twelve family views and sixteen route views were inspected. No compiler or other game owned by this agent ran during the measured walk. Dummy audio is not listening evidence.

- Format 39 Release/Debug builds of the game and all three registered test targets pass, with all CTest suites passing. Actual six-metre figure approaches/returns finish in all three families with measured positions and mouse headings. Nearest-distance/opacity telemetry reaches zero within 1.5 m and returns to one beyond 5 m; all 33 paired stages were inspected. Native depth/alpha state restores correctly in 18 entity and 12 family views. All six actual entrance directions/outside/frame cases pass. The initial QA driver accumulated small key-duration drift; calculating each next move from the measured position resolves it. Baseline screenshot timing overlaps compilation and is not a performance comparison. Dummy audio is not listening evidence.

- Format 38 refreshed Release/Debug builds and all three CTest suites pass after externally changing dependencies become consistent. The 448.3 m collision-enabled tunnel return completes 60 waypoints, five player chunks and at most 25 active chunks. Peak sampled build is 6.84 ms, warmed RSS 236.5–246.7 MiB and final 246.7 MiB, with 208 buffer creations and 313 reuses. Draw rate is about 39 FPS on the private display. The short walk does not establish a final tunnel memory plateau; the earlier 28.8 km sweep does. Standard screenshot timing overlapped a dependency rebuild and is not used for performance claims.

- Format 36 Release/Debug CTest suites pass. The 9.6 km industrial diagnostic holds at most 25 chunks and 19 sampled spare buffers. It creates 251 buffers and reuses 9,143. Packed capacity stays at 5.8–6.4 MiB; warmed RSS holds at 188.74 MiB through the final three quarters. Median actual draw rate is 39 FPS, rolling p95 median 26.28 ms and maximum sampled interval 112.11 ms; median build is 1.13 ms and peak 5.80 ms. Occasional presentation outliers remain. Collision is deliberately bypassed in this diagnostic. Source profile fingerprint is `6075bcb6c5f3514c`.

- Format 35 Release/Debug CTest suites pass. The 528.3 m collision return completes 48 waypoints in 288.6 seconds, eight player chunks, maximum 25 active chunks, 1.97 ms peak build and warmed RSS 188.0–188.4 MiB. All six live entrances/outside/frame cases pass. Sixteen composition pairs, sixteen fixture refinements, twelve family views and thirteen route views were inspected.

- Format 34 final suites pass; eighteen grounded figure views, twelve family views and eleven controller/regression views were inspected. The 356.0 m collision-enabled return crosses five player chunks, stays at 25 loaded chunks and has 9.18 ms peak build. RSS falls from 186.3 MiB to 157.3 MiB; the sampled warmed range is 157.3–174.9 MiB. A twelve-view repeat verifies the new readiness guard. The final longer tunnel result follows below.

- The final format 34 tunnel sweep reaches 28.8 km with at most 25 chunks and 18 sampled spare buffers. It creates 390 buffers and reuses 28,699. Packed capacity peaks at 57.2 MiB; late RSS is 295.5–300.6 MiB, with final twelve samples 295.5–298.7 MiB. Median draw FPS is 39, median rolling p95 27.22 ms, sampled maximum interval 72.57 ms, median build 3.87 ms and peak 25.29 ms. Ten builds exceed 16.67 ms in about 640 seconds; occasional spikes remain. Collision is bypassed in this diagnostic, not in controller walks.
- **Timing correction:** titles through format 29 labeled fixed-step updates as FPS. Historical rates near 59 are UPS. Actual format 30 views/sweeps draw around 39–41 FPS on the isolated Weston/Xwayland display; no real desktop presentation rate is inferred. The title now separates actual FPS, 120-interval p95/max, CPU submission and UPS.
- Earlier long collision returns, physical audits, fallbacks and family-specific finite measurements are retained in [validation history](docs/validation-history.md).
- The explicit CNA context lease remains necessary for reused-buffer uploads, transitions and deletion; [bugs.md](bugs.md) records the regression. No sibling edits. Latest observed dependency heads: CNA `c90f0e39f45e7823623058a1063b364735a9214e`, Sharp Runtime `6c4a857de129cf29b5d43430bedf24157d594f12`. Sibling input/runtime files have external uncommitted edits; this agent does not change them. A transient Debug inconsistency and the successful retry are documented in bugs.md.
- Fresh format 34 captures use the game's own normal PipeWire/PulseAudio stream on the unmuted Ryzen speaker output: hum -32.3 dBFS RMS, footsteps -10.8 dBFS peak and an isolated transition without footsteps -13.5 dBFS peak, without clipping. Subjective listening remains unverified. Dummy-audio visual tests do not validate audibility.

## Next pass

Complete cell-wall joins/terminations, compare the exposed corners and clear
ends, and walk a fresh office return. Then refine coarse floor light pools and
continue the product audit, audio routing and finite capacity checks until the
scheduled stabilization window. The goal remains active.
