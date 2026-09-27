# cna-backrooms: development plan

Target: Linux desktop, CNA `next`, sharp-runtime `next`, EasyGL `OPENGLES3`. Budget: about 24 hours. The first walking loop is Phase 0, not the quality target.

| Priority | Scope | Estimate |
| --- | --- | ---: |
| MUST HAVE | Conventional mouse look, credible textured Level 0, spatial variety, wall collision, deterministic bounded streaming, validated audio path | remaining core work |
| SHOULD HAVE | Distinct industrial Level 1 and service Level 2, environmental transitions, atmospheric entities, long traversal validation | after core |
| OPTIONAL | Save file, extra props and audio, visual effects | remaining time |

The world is an unbounded integer cell grid. Each 8x8-cell chunk is regenerated from a versioned seed and global cell coordinates. Doorways on shared cell edges use the same hash from either side. Only nearby GPU buffers stay live. No disk chunk format or third-party assets are needed for this milestone; the seed, algorithm version, and level id are the reproducible world format.

Working backlog: (1) fix look/strafe and verify, (2) add CNA textured materials and ceiling/lights, (3) replace the exposed grid with multi-cell room composition, (4) distinct levels and environmental transitions, (5) audio checks and harmless distant presence, (6) long walk, screenshots, fixes, documentation. Build and inspect after each major pass.
