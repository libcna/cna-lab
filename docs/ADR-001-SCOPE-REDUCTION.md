# ADR-001 — CNA Studio is a companion tool, not an engine editor

| | |
|---|---|
| **Status** | Accepted |
| **Date** | 2026-09-22 |
| **Decided by** | The repository owner |
| **Supersedes** | The scope of the master development plan, now archived as [`ROADMAP-ARCHIVE.md`](ROADMAP-ARCHIVE.md) |
| **Affects** | [`../plan.md`](../plan.md), [`ROADMAP-BACKLOG.md`](ROADMAP-BACKLOG.md), [`ROADMAP-OUT-OF-SCOPE.md`](ROADMAP-OUT-OF-SCOPE.md), [`../plans/`](../plans/), [`ARCHITECTURE.md`](ARCHITECTURE.md) |

> This is the first architecture decision record in this repository. Later ones take the next
> number and the same shape: context, decision, consequences, and what would have to happen to
> reverse it.

## Context

CNA Studio began as an imported editor prototype and grew a 36-phase, 582-task roadmap describing a
multi-thousand-hour engineering programme. That roadmap was honest about what it was: it said in
its own first paragraph that it was "not a project with an end date this year", and it expected to
reach "the low thousands" of tasks as later phases were decomposed.

405 of those tasks are complete, and the result is substantial: a native editor UI with docking and
a command registry, a Project Hub with working templates, a Content Browser, 2D and 3D viewports
with gizmos, an Outliner, a descriptor-driven Inspector, material and lighting workflows, a
separate player process with a live play bridge, autosave and crash recovery, an asset database
with importers, a build runner that drives the project's own CMake, and a test suite that includes
architecture guards, golden images, sanitizers and a CNA-backed CI leg.

The remaining 176 tasks are a different kind of thing. They include a C++ reflection ecosystem, a
shader graph, skeletal animation authoring, particle and audio authoring, terrain and physics
tooling, an integrated profiler, a frame debugger, a plugin SDK, and an open-ended visual-quality
campaign benchmarked against Unreal, Unity, Godot and Blender. Each is individually defensible.
Together they are a general-purpose game engine editor, and finishing them would take years.

Two facts forced the decision:

1. **The trajectory had drifted from the need.** The reason CNA Studio exists is to make CNA
   application development easier. Most of the remaining roadmap does not do that; it makes CNA
   Studio resemble the tools it was benchmarked against.
2. **The budget is finite and small.** The engineering budget for the remainder of this product is
   approximately **40 Opus 5 hours**, with **60 hours as a hard ceiling**. That is not a forecast
   that can be revised upward when a feature turns out to be expensive. It is part of the product
   definition.

## Decision

### CNA Studio is a lightweight visual development companion for CNA applications

C++ source code, CMake, CLion and other IDEs, and ordinary CNA application development remain
first-class and authoritative. Studio provides visual workflows only where they materially improve
CNA development.

### CNA Studio is not

- a replacement for CLion, Visual Studio or any other IDE
- a full C++ IDE
- Unity
- Unreal Engine
- Godot
- a general-purpose DCC tool
- a reason to reproduce functionality that standard development tools already provide adequately

### The product is bounded by a Core workflow

A developer can create or open a CNA project; browse, import and manage assets; create, open and
edit scenes; use the viewport, selection and transform gizmos; edit the object, material and light
properties Studio already models; save reliably; launch and play the application; configure and
invoke normal CMake builds; see useful build and runtime diagnostics; recover safely from crashes
and interrupted work; and continue normal C++ work in an external IDE.

The completion criteria are in [`../plan.md`](../plan.md) and are measurable, line by line, against
tests.

### Studio features are demand-driven, not roadmap-driven

A feature is built because a real, maintained CNA application demonstrated a recurring need that
the existing workflow could not meet — not because it appears in a backlog, not because a
professional editor has it, and not because it was already in the plan.

### When the Core is complete, CNA Studio enters maintenance mode

Planned feature development stops. Maintenance is bug fixes, compatibility fixes, adaptation to CNA
API changes, security and correctness work, small usability fixes inside the Core workflow, and
features justified by demonstrated needs in real maintained CNA applications. It is not
implementing items because they remain listed somewhere.

### The unfinished work is classified, not deferred

Marking 176 tasks "deferred" would have preserved the assumption that all of them are still
intended. Instead every unfinished task is in exactly one of three places, and a test in
`tests/ArchitectureGuardTests.cpp` fails if one is dropped, duplicated or invented:

| Where | Count | What it means |
|-------|------:|---------------|
| [`../plan.md`](../plan.md) | 24 tasks, consolidated into 11 deliverables | Authorised work |
| [`ROADMAP-BACKLOG.md`](ROADMAP-BACKLOG.md) | 100 | May be reconsidered on a demonstrated need; each carries an activation condition |
| [`ROADMAP-OUT-OF-SCOPE.md`](ROADMAP-OUT-OF-SCOPE.md) | 52 | No work under this roadmap; reintroduction needs a new ADR |

## The expensive areas, decided explicitly

Each of these was reviewed on its own rather than left accidentally active. The default was
removal, and "professional editor completeness", "Unity has it" and "it was already in the plan"
were not accepted as reasons to keep anything.

| Area | Decision | Why |
|------|----------|-----|
| General C++ gameplay reflection and component tooling | **Out of scope** | The largest new subsystem in the programme, and the defining feature of an engine editor. Obliges Studio to a metadata format, a generator, an ABI contract and a regeneration story, permanently |
| Skeletal animation authoring | **Conditional** | Sprite animation already works. Skeletons need a demonstrated authoring bottleneck in a real application first |
| Shader graph | **Out of scope** | A multi-dialect compiler with a user interface. The material workflow already authors what the runtime executes |
| Particles and VFX authoring | **Conditional** | Only if the CNA runtime owns the particle system and a real application tunes it often |
| Audio authoring | **Conditional** | Audio preview and component editing already exist |
| Terrain and world tooling | **Conditional** | Only on repeated large-scale terrain editing that the code and data workflow cannot carry |
| Physics tooling | **Conditional** | Studio integrates a runtime's physics; it never owns one |
| Navigation and navmesh tooling | **Conditional** | Same test as physics |
| Integrated profiler | **Conditional** | Only if external profilers and CNA diagnostics cannot answer a question a real project has |
| Frame debugger | **Out of scope** | RenderDoc already does this, better, on an ordinary native process |
| Plugin SDK | **Out of scope** | A public ABI contract is a permanent tax on internal change, paid by a product with no third-party audience. The existing plugin host stays and is maintained |
| Packaging and export abstractions | **Conditional** | The project's own CMake already produces a runnable game, and CI already builds an exported project with Studio absent |
| Localisation and accessibility | **Conditional** | Removed on budget, not on merit — and the entry most likely to be activated by an ordinary request |
| Broad production polish | **Out of scope as a campaign** | Its own exit criterion was "never done" and its benchmark was the products this ADR says Studio is not. Individual panels are still fixed when demonstrably hard to use |
| Advanced editor infrastructure built to support the above | **Out of scope** | Infrastructure whose purpose is enabling hypothetical Studio features rather than solving a current CNA development need |

Three rows that the campaign above would have covered were kept as active work, because each blocks
a Core step rather than a standard of finish: the viewport shows nothing where the selection is,
the property grid separates a label from its value by a gap wider than either, and neither the
Content Browser nor the Outliner can be searched.

## Consequences

**Good.**

- The remaining work is finite, estimated and checkable: 11 deliverables, roughly 35 hours, with a
  definition of done that names the test for each line.
- A future agent or contributor opening this repository finds one authoritative roadmap containing
  only authorised work, and three clearly-labelled documents that contain none.
- Maintenance mode becomes a state the product can actually reach, rather than an asymptote.
- The 40-hour budget is defended by construction: the only way to add work is to remove other work,
  because the ceiling does not move.

**Costs, stated plainly.**

- CNA Studio will be visibly less capable than Unity, Unreal or Godot, permanently and by design.
  Anyone comparing them will find it lacking, and that comparison is not a defect report.
- Some genuinely good design work — the shader graph decomposition, the reflection analysis, the
  visual quality criterion — will not be built. It is preserved as history, and preserving it is
  explicitly not a commitment to build it.
- Accessibility and localisation are cut on budget rather than on merit. That is the least
  comfortable line in this document and is recorded as such.
- Non-Latin text still renders as boxes. A developer who names assets in their own script will see
  that, and the activation condition in the backlog is written for exactly that person.

**Neutral, but worth knowing.**

- `STUDIO-PPNNN` ids are not renumbered, retired or reused. A commit message, code comment or test
  citing one still resolves, to an archived row that states what it meant.
- The plan-integrity guard tests were repointed from `plan.md` to `ROADMAP-ARCHIVE.md`, so the
  archived record's arithmetic is still checked and cannot rot, and a new guard checks that the
  three-way classification stays complete.

## Reversing this

This ADR is reversed only by a later ADR that supersedes it, written by someone who accepts the
budget consequence. The following are specifically **not** sufficient to reopen scope: an item
appearing in the archived roadmap or the backlog; a comparison with another tool; the age of a
backlog entry; a contributor's enthusiasm for a subsystem; or the discovery that an active
deliverable is harder than estimated — that last one reduces scope, by the rule in `plan.md`.

An individual backlog item can move into active scope without reversing this ADR, by the four-part
test in `plan.md`. That mechanism is the intended way for CNA Studio to grow, and it starts with a
real application and a real bottleneck.
