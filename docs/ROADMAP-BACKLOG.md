# CNA Studio — conditional future backlog

> **Nothing in this document is planned work, and nothing in it authorises implementation.**
>
> These are directions that were designed, decomposed and then removed from the active product by
> [`ADR-001-SCOPE-REDUCTION.md`](ADR-001-SCOPE-REDUCTION.md). Each may be reconsidered — once, and
> on evidence — if a **real, maintained CNA application** demonstrates a concrete recurring need
> that the existing code, data and external-tool workflow cannot meet.
>
> The active roadmap is [`../plan.md`](../plan.md). The full historical detail of every row below —
> its acceptance criteria, its verification and its dependencies — is in
> [`ROADMAP-ARCHIVE.md`](ROADMAP-ARCHIVE.md) and [`../plans/`](../plans/).

## How an item leaves this document

An item moves into `plan.md` only when its **activation condition** below is met and all four of
the general tests in `plan.md` (*How something legitimately becomes active work*) hold: a real
maintained application hits the need repeatedly; the existing workflow is a demonstrated
bottleneck; the proposed deliverable is bounded and does not oblige Studio to own a runtime
subsystem; and someone accepts the maintenance cost.

**Age is not evidence.** An item sitting here for years has accumulated no claim on anyone's time.
Neither is "a professional editor has it", nor "it was already in the plan".

**100 archived tasks are recorded here.**

---

## Authoring tools for runtime features

### Skeletal and clip animation authoring

**Activation condition.** A real maintained CNA application authors skeletal or clip animation
repeatedly, and the round trip through its DCC tool and the existing import path has become a
demonstrated bottleneck. Sprite animation already works and stays maintained; this is about
skeletons, timelines, curves and notifies.

| Ids | What it was |
|-----|-------------|
| `STUDIO-21001`, `STUDIO-21002`, `STUDIO-21003`, `STUDIO-21004`, `STUDIO-21005`, `STUDIO-21006`, `STUDIO-21007`, `STUDIO-21008`, `STUDIO-10009` | Skeletal preview, clip assets, playback controls, timeline, keyframes, curve editing, events and notifies, blend configuration, animation import |

### Particles and VFX authoring

**Activation condition.** A real maintained CNA application tunes particle effects often enough
that editing the definition data by hand is costing measurable time, **and** the CNA runtime owns
the particle system being authored. Studio never acquires a particle runtime.

| Ids | What it was |
|-----|-------------|
| `STUDIO-23001`, `STUDIO-23002`, `STUDIO-23003`, `STUDIO-23004` | Particle definition assets, emitter authoring, viewport preview, runtime-ownership guard |

### Audio authoring

**Activation condition.** A real maintained CNA application needs spatial audio placement or bus
and zone configuration that the Inspector's existing component editing and the existing audio
preview cannot express.

| Ids | What it was |
|-----|-------------|
| `STUDIO-24001`, `STUDIO-24002`, `STUDIO-24003`, `STUDIO-24004`, `STUDIO-24005`, `STUDIO-24006`, `STUDIO-24007`, `STUDIO-24008` | Audio source and listener authoring, attenuation, spatial preview, play/stop from the browser, zones and buses, waveform preview, audio metadata |

### Terrain and large-world tooling

**Activation condition.** A real maintained CNA application requires repeated large-scale terrain
editing and the existing code and data workflow has become a demonstrated development bottleneck.
Streaming and partitioning stay behind that again: they were already deferred on the same test.

| Ids | What it was |
|-----|-------------|
| `STUDIO-25001`, `STUDIO-25002`, `STUDIO-25003`, `STUDIO-25004`, `STUDIO-25005`, `STUDIO-25006`, `STUDIO-25007` | Terrain import, sculpting, layer painting, foliage and procedural placement, large-world organisation, streaming and partitioning |

### Physics and navigation tooling

**Activation condition.** A real maintained CNA application cannot place or verify colliders,
rigid bodies or navigation volumes through the Inspector and the viewport as they stand, **and**
the CNA runtime owns the physics and navigation being authored.

| Ids | What it was |
|-----|-------------|
| `STUDIO-26001`, `STUDIO-26002`, `STUDIO-26003`, `STUDIO-26004`, `STUDIO-26005`, `STUDIO-26006`, `STUDIO-26007`, `STUDIO-26008` | Collider visualisation and editing, rigid-body authoring, physics debug rendering, navigation volumes, navmesh generation, path debugging, runtime-ownership guard |

---

## Tooling that duplicates existing tools

### Integrated profiling and live runtime diagnostics

**Activation condition.** External profilers and CNA's own diagnostics cannot provide information
that a real CNA project requires, and the gap is named concretely rather than as "we would like a
profiler". Remote profiling stays behind local profiling, as it already did.

| Ids | What it was |
|-----|-------------|
| `STUDIO-27001`, `STUDIO-27002`, `STUDIO-27003`, `STUDIO-27004`, `STUDIO-27005`, `STUDIO-27006`, `STUDIO-27007`, `STUDIO-27008`, `STUDIO-27013` | Frame and CPU timing, GPU timing, draw-call and resource counts, memory reporting, asset load timing, build timing, capability reporting, live player status, remote profiling |

### Cooking, packaging and export from Studio

**Activation condition.** A real maintained CNA application ships often enough that driving its own
CMake install or packaging step by hand is a demonstrated cost. The invariant this once protected
is already proven without it: `STUDIO-02051` builds an exported project in a clean directory with
Studio absent, on every CI run, and a project's own CMake already produces a runnable game.

| Ids | What it was |
|-----|-------------|
| `STUDIO-18001`, `STUDIO-18002`, `STUDIO-18003`, `STUDIO-18004`, `STUDIO-18005`, `STUDIO-18006`, `STUDIO-18010`, `STUDIO-18011`, `STUDIO-18020`, `STUDIO-18021`, `STUDIO-18022` | Content cooking, staging, executable and runtime-library discovery, asset and licence staging, launching a packaged build, shipping-build exclusion and its guard, the standalone export test, project-owned build tooling, per-target smoke tests |

### Release engineering for Studio itself

**Activation condition.** People who are not building Studio from source need to run it. Until
then, `git clone` and CMake are the install. `STUDIO-34001` — the versioning scheme and release
notes — is **not** here: it is active work in `CORE-11`, because maintenance mode needs a version
number to talk about.

| Ids | What it was |
|-----|-------------|
| `STUDIO-34002`, `STUDIO-34003`, `STUDIO-34004`, `STUDIO-34005`, `STUDIO-34006` | Linux, Windows and macOS packaging, reproducible release builds, preference and layout upgrade paths |

---

## Reach: platforms, renderers, languages, people

### Accessibility and localisation

**Activation condition.** A contributor or user needs it. This one deserves saying plainly: it is
removed from Core on **budget**, not on merit, and it is the entry here most likely to be
activated by an ordinary request rather than by a bottleneck argument. Keyboard focus, Tab
navigation and a focus-visible style already exist; what is missing is completeness, platform
integration and externalised strings.

| Ids | What it was |
|-----|-------------|
| `STUDIO-03013`, `STUDIO-03027`, `STUDIO-32001`, `STUDIO-32002`, `STUDIO-32003`, `STUDIO-32004`, `STUDIO-32005`, `STUDIO-32006` | Accessibility metadata on every widget, IME support, metadata completeness, keyboard-only operation, focus and contrast audit, platform accessibility APIs, string externalisation, text-expansion-tolerant layout |

### Text outside the shipped faces

**Activation condition.** A real maintained CNA application names scenes, assets or entities in a
script the shipped IBM Plex faces do not cover, so a user sees replacement boxes where their own
names should be. The caret and cluster model already handle these scripts; only the glyphs are
missing, and supplying them means shipping and licensing fallback fonts.

| Ids | What it was |
|-----|-------------|
| `STUDIO-04019` | Font fallback for CJK, Hangul, Arabic, Devanagari and emoji |

### Renderer robustness and per-renderer coverage

**Activation condition.** A renderer Studio is actually hosted on fails in one of these ways — a
device loss that loses the frame, a renderer whose output is blank — or CI gains hardware that can
observe one. Device loss in particular cannot be tested honestly without a device that loses.

| Ids | What it was |
|-----|-------------|
| `STUDIO-04010`, `STUDIO-04015`, `STUDIO-29005`, `STUDIO-02010`, `STUDIO-02011` | Render-resource lifetime and device loss, per-renderer smoke tests, Studio-host renderers in CI, re-measuring CNA gaps G-03 and G-05 across the renderer set |

### CI reach and visual-test depth

**Activation condition.** Studio is supported on a platform CI does not build, or a visual
regression reaches a user because the current goldens could not see it. The suite already runs
dependency-free, CNA-backed, sanitizer and `OPENGL4`-under-Xvfb legs and keeps captures as
artifacts.

| Ids | What it was |
|-----|-------------|
| `STUDIO-33010`, `STUDIO-33012`, `STUDIO-33015`, `STUDIO-33021`, `STUDIO-35082` | Graphical CI on real hardware, canonical visual scenes, visual regressions as artifacts, a Linux/Windows/macOS matrix, tolerant golden comparison and occupancy probes |

### Scale and stress coverage beyond what is measured

**Activation condition.** A real project hits a size the current fixtures do not cover and Studio
is slow on it. The performance suite already covers 100 000 assets, large hierarchies and large
logs.

| Ids | What it was |
|-----|-------------|
| `STUDIO-30023`, `STUDIO-30024` | Stress benchmarks for very large logs, and for large property lists and imported models |

---

## Refinements to things that already work

### Play-mode expansion

**Activation condition.** A real maintained CNA application needs to test gameplay in a way that
Play, Pause, Step, Stop and Restart against the real player cannot reach. Renderer preview
selection is included deliberately: Play already picks the build matching the active target
profile and falls back to whatever was built, which is the shipped answer until someone needs to
compare two renderers often.

| Ids | What it was |
|-----|-------------|
| `STUDIO-16008`, `STUDIO-16009`, `STUDIO-16011` | Simulation mode, possession and eject, renderer preview selection among installed player builds |

### Build feature profiles

**Activation condition.** A real project is offered a target combination that cannot be built, or
needs to declare renderer requirements that the buildability check cannot express. Renderer and
platform selection is already validated against what CNA can build; this is the capability layer
above it, and it is new infrastructure.

| Ids | What it was |
|-----|-------------|
| `STUDIO-17007`, `STUDIO-17008` | Feature profile for what a game requires of a renderer; a GUI offering only meaningful combinations |

### Visual refinement of existing panels

**Activation condition.** A specific panel is demonstrably hard to use — not merely plainer than a
commercial editor. The **CNA Studio Visual Quality 1.0** campaign that these belonged to is retired
from the active product; see [`ROADMAP-OUT-OF-SCOPE.md`](ROADMAP-OUT-OF-SCOPE.md). The three rows
that were genuinely blocking the Core workflow — the property grid, the two search filters and
viewport selection feedback — were kept and are `CORE-03`, `CORE-04` and `CORE-05`.

| Ids | What it was |
|-----|-------------|
| `STUDIO-35022`, `STUDIO-35023`, `STUDIO-35034`, `STUDIO-35035`, `STUDIO-35051`, `STUDIO-35052`, `STUDIO-35062`, `STUDIO-35071`, `STUDIO-03030` | Typographic scale and font review, reference fields that preview their target, multi-selection in Details, a ground-plane grid, a viewport orientation widget, a lock concept, toolbar ergonomics, an animation model |

### Editor-internal structure and open research

**Activation condition.** The structure gets in the way of a maintenance fix. These are
maintainability work with no user-visible effect, which is exactly the kind of work maintenance
mode does when it is needed and not before.

| Ids | What it was |
|-----|-------------|
| `STUDIO-02058`, `STUDIO-05015` | Moving panel binding into per-panel binders; a floating panel in a real second OS window |

### Documentation beyond what Core needs

**Activation condition.** An external consumer of Studio's public API exists. `CORE-08` records
whether one does; if the answer is no, this stays here.

| Ids | What it was |
|-----|-------------|
| `STUDIO-33005` | Public API documentation coverage |

### Carried-over deferrals

These were already deferred with a recorded reason before the scope reduction, and the reason still
holds. They are listed so that the classification of unfinished work is complete.

| Ids | What it was |
|-----|-------------|
| `STUDIO-02087` | Making the toolchain-path preference per-language, which is owed only when a second language adapter exists |
