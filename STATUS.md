# cna-backrooms visual goal status

## Current state

The product goal remains **active**. Three distinct families, connected deterministic rooms, bounded 5-by-5 streaming, collision, conventional mouse look, Shift toggling, native profiles, licensed audio, transitions and harmless entities work. Level 0 has original wallpaper/carpet, acoustic ceilings, warmer height-dependent fluorescent light, irregular rooms, six offset partition plans, sparse furniture and alcoves. Builds and numeric tests do not finish the visual audit.

## Current pass

Format 35 removes cell-boundary fragments inside Level 1 open, storage and column bays and aligns roof beams with supports. Sixteen paired directions were inspected; they exposed fixtures hidden by the new beams. Industrial lights now hang on short mounts below the beam soffits, with the same height used by the bake. Final Release/Debug suites, sixteen refined directions, twelve family views and thirteen route views pass inspection. The 528.3 m industrial return completes all 48 waypoints with 25 maximum chunks and 1.97 ms peak build. All six live entrance/frame cases pass. The seven-seed scan and all 63 physical squares pass. The completed 28.8 km tunnel sweep reaches a late RSS plateau.

## Completed passes

- **35:** complete open industrial bays, framing above supports and suspended fixtures below beam soffits. Sixteen composition pairs, sixteen fixture refinements, twelve family views and thirteen route views were inspected. Release/Debug CTest suites, seven-seed sampling, all 63 physical squares, the 528 m return and all six live entrance cases pass.

- **34:** tapered harmless cloth figures, rounded heads and soft foot contact spots. Eighteen matched pairs, eighteen grounded views, two twelve-view family rounds and eleven controller/regression views were inspected. Final suites and the 356 m return pass. Fresh normal-device captures verify hum, footsteps and an isolated cue on the unmuted hardware output; subjective listening remains unverified.

- **33:** original cast concrete and height/face samples on service supports. All 24 multi-seed pairs and twelve family views were inspected; Debug/Release suites pass. The 9.6 km tunnel sweep holds 25 chunks, median 39 draw FPS and 9.40 ms peak, but still has buffer-capacity RSS growth.
- **32:** eight-sided small pipes, continuous cylindrical UVs and shared circumferential shading. All nine matched pairs and Debug/Release tests pass; a 2.4 km sweep holds median 39 draw FPS, 25 chunks and 9.34 ms peak build.
- **31:** original granular concrete walls/floors, a shared native texture, two-metre UVs and separate procedural fallbacks. Nine material pairs, floor views, 24 distant directions and twelve final family views were inspected; the 449 m real return and missing/corrupt launches pass.
- **30:** native four-sample MSAA, whole-chunk native frustum checks and actual draw timing. Nine culled/unculled images are pixel-identical; 48 directional views and sixteen controller/regression views were inspected. Direct coordinate bounds remove expensive native object copies. Slow-build logs split geometry/upload time.
- **29:** long/L/T/staggered/short/U-shaped office partitions, with shared clipped geometry/collision and unchanged connectors. Twenty-four paired views, 32 distant directions, 24 controller and eight alcove views were inspected.
- **Earlier:** original office materials, human scale, local wall lighting, composed room entrances, alcoves/doors, industrial bays, weathered tunnels and bounded caches. Detailed results are in [validation history](docs/validation-history.md).

## Three highest-priority deficiencies

1. The industrial floor remains visibly cloudy, and wall seams are coarse. Improve these inexpensive material details using the existing mineral texture and native materials.
2. Inspect arbitrary office regions again and fix the largest remaining composition or material problems.
3. Nearby figures disappear abruptly at the current distance cutoff; soften their retreat without adding interaction or AI complexity.

## Latest validation

- Format 35 Release/Debug CTest suites pass. The 528.3 m collision return completes 48 waypoints in 288.6 seconds, eight player chunks, maximum 25 active chunks, 1.97 ms peak build and warmed RSS 188.0–188.4 MiB. All six live entrances/outside/frame cases pass. Sixteen composition pairs, sixteen fixture refinements, twelve family views and thirteen route views were inspected.

- Format 34 final suites pass; eighteen grounded figure views, twelve family views and eleven controller/regression views were inspected. The 356.0 m collision-enabled return crosses five player chunks, stays at 25 loaded chunks and has 9.18 ms peak build. RSS falls from 186.3 MiB to 157.3 MiB; the sampled warmed range is 157.3–174.9 MiB. A twelve-view repeat verifies the new readiness guard. The longer tunnel sweep remains pending.

- The final format 34 tunnel sweep reaches 28.8 km with at most 25 chunks and 18 sampled spare buffers. It creates 390 buffers and reuses 28,699. Packed capacity peaks at 57.2 MiB; late RSS is 295.5–300.6 MiB, with final twelve samples 295.5–298.7 MiB. Median draw FPS is 39, median rolling p95 27.22 ms, sampled maximum interval 72.57 ms, median build 3.87 ms and peak 25.29 ms. Ten builds exceed 16.67 ms in about 640 seconds; occasional spikes remain. Collision is bypassed in this diagnostic, not in controller walks.
- **Timing correction:** titles through format 29 labeled fixed-step updates as FPS. Historical rates near 59 are UPS. Actual format 30 views/sweeps draw around 39–41 FPS on the isolated Weston/Xwayland display; no real desktop presentation rate is inferred. The title now separates actual FPS, 120-interval p95/max, CPU submission and UPS.
- Earlier long collision returns, physical audits, fallbacks and family-specific finite measurements are retained in [validation history](docs/validation-history.md).
- The explicit CNA context lease remains necessary for reused-buffer uploads, transitions and deletion; [bugs.md](bugs.md) records the regression. No sibling edits. Latest dependency heads: CNA `b2fd47a45757c32326cbbb5c2b39afffdb7392c5`, Sharp Runtime `fc033a0e8541a81498c4a496f56a0f59475c6e34`.
- Fresh format 34 captures use the game's own normal PipeWire/PulseAudio stream on the unmuted Ryzen speaker output: hum -32.3 dBFS RMS, footsteps -10.8 dBFS peak and an isolated transition without footsteps -13.5 dBFS peak, without clipping. Subjective listening remains unverified. Dummy-audio visual tests do not validate audibility.

## Next pass

Compare improved industrial wall/floor materials, including missing-asset behavior. Continue with arbitrary office views and near-entity behavior; the product goal remains active. Continue screenshot-driven refinement and actual traversal; the goal remains active.
