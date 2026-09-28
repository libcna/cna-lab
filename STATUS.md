# cna-backrooms visual goal status

## Current state

The product goal remains **active**. Three distinct families, connected deterministic rooms, bounded 5-by-5 streaming, collision, conventional mouse look, Shift toggling, native profiles, licensed audio, transitions and harmless entities work. Level 0 has original wallpaper/carpet, acoustic ceilings, warmer height-dependent fluorescent light, irregular rooms, six offset partition plans, sparse furniture and alcoves. Builds and numeric tests do not finish the visual audit.

## Current pass

Format 32 compares eight-sided small pipes with shared circumferential vertex shades and continuous cylindrical UVs. The explicit sixteen-sided pressure assemblies retain their geometry. This addresses flat face bands and repeated strip UVs without adding runtime lights or shaders. All nine matched pairs were inspected; pipes look rounder without the previous flat color bands. Release/Debug builds and all three test suites pass. A 2.4 km sweep keeps at most 25 chunks, median 39 draw FPS, 26.70 ms median rolling p95, 37.68 ms sampled maximum, 3.54 ms median and 9.34 ms peak build. Latter-half RSS is 266.5–279.2 MiB as capacities settle; a longer final tunnel run remains necessary.

## Completed passes

- **31:** original granular concrete walls/floors, a shared native texture, two-metre UVs and separate procedural fallbacks. Nine material pairs, floor views, 24 distant directions and twelve final family views were inspected; the 449 m real return and missing/corrupt launches pass.
- **30:** native four-sample MSAA, whole-chunk native frustum checks and actual draw timing. Nine culled/unculled images are pixel-identical; 48 directional views and sixteen controller/regression views were inspected. Direct coordinate bounds remove expensive native object copies. Slow-build logs split geometry/upload time.
- **29:** long/L/T/staggered/short/U-shaped office partitions, with shared clipped geometry/collision and unchanged connectors. Twenty-four paired views, 32 distant directions, 24 controller and eight alcove views were inspected.
- **Earlier:** original office materials, human scale, local wall lighting, composed room entrances, alcoves/doors, industrial bays, weathered tunnels and bounded caches. Detailed results are in [validation history](docs/validation-history.md).

## Three highest-priority deficiencies

1. Industrial supports look uniformly colored in close views. Give their existing shape cast finishes and face-dependent light.
2. Entities have an obvious six-box silhouette. Refine the inexpensive mesh while preserving harmless behavior.
3. Recheck all family views and the longer tunnel RSS plateau after the geometry passes.

## Latest validation

- Format 31 Release/Debug builds and world/profile/lighting tests pass. Geometry/collision are unchanged; source fingerprint remains `274dca75c397b6e3`. The real controller completes 60 waypoints over 449.3 m in 275.3 seconds, five player chunks, maximum 25 chunks and 10.12 ms peak build. Warmed RSS grows from 231.0 to 241.2 MiB as buffer capacities settle. All sixteen route views were inspected. A 2.4 km sweep holds 25 chunks, median 39 draw FPS, median rolling p95 28.57 ms, sampled maximum 39.70 ms and 9.74 ms peak build; latter-half RSS is 255.2–262.3 MiB. The pool is bounded, but this short material-only run does not establish a final plateau. Missing and corrupt PNGs restore separate procedural wall/floor materials.
- Format 30 final builds/tests pass. A 355.2 m real return holds 25 chunks, 3.55 ms peak and warmed RSS 175.5–176.3 MiB. Final 2.4 km tunnel sweeps with MSAA 0/4 both hold 25 chunks and median 39 draw FPS, 9.28/7.11 ms peak builds, median rolling p95 26.38/27.18 ms and sampled maxima 35.09/32.65 ms. Latter-half RSS is 249.0–253.7/251.2–256.0 MiB. No build over 16.67 ms in those finite repeats.
- **Timing correction:** titles through format 29 labeled fixed-step updates as FPS. Historical rates near 59 are UPS. Actual format 30 views/sweeps draw around 39–41 FPS on the isolated Weston/Xwayland display; no real desktop presentation rate is inferred. The title now separates actual FPS, 120-interval p95/max, CPU submission and UPS.
- Format 29's 1,162.5 m return, seven-seed scan, all 63 physical squares and four real alcove orientations pass. Its 9.6 km sweep stays at 25 chunks/256 layouts, 4.54 ms peak and latter-half RSS 180.5–181.4 MiB. Earlier tunnel sweeps reach 28.8 km with bounded objects/buffers and a late RSS plateau; see history for finite observations.
- The explicit CNA context lease remains necessary for reused-buffer uploads, transitions and deletion; [bugs.md](bugs.md) records the regression. No sibling edits. Latest dependency heads: CNA `b2fd47a45757c32326cbbb5c2b39afffdb7392c5`, Sharp Runtime `fc033a0e8541a81498c4a496f56a0f59475c6e34`.
- Native PipeWire capture verifies routing/levels: hum about -32 dBFS RMS, steps near -10 dBFS peak, transition below -9.6 dBFS, no clipping. Muted hardware prevents subjective listening. Dummy-audio visual tests do not validate audibility.

## Next pass

Improve industrial-support depth, then the harmless silhouette. Continue screenshot-driven refinement and actual traversal; the goal remains active.
