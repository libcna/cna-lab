# CNA Studio — out of active product scope

> **No engineering work is performed on anything in this document under the current CNA Studio
> roadmap.** Reintroducing any of it would require an explicit future scope decision, recorded as a
> new ADR that supersedes [`ADR-001-SCOPE-REDUCTION.md`](ADR-001-SCOPE-REDUCTION.md) on the point.
>
> This is not the same as the [conditional backlog](ROADMAP-BACKLOG.md). The backlog holds features
> that could be activated by a demonstrated need in a real application. **These are directions that
> no longer belong to the product at all** — each would make CNA Studio into something the ADR says
> it is not: an IDE, an engine editor, a DCC tool, or a platform for other people's tools.
>
> The design work is preserved rather than deleted. Where a direction was thought through, the
> reasoning is still worth reading and is still in [`ROADMAP-ARCHIVE.md`](ROADMAP-ARCHIVE.md) and
> [`../plans/`](../plans/). **Preserving the design history is not a commitment to the design.**

"Out of active scope" does not mean *mathematically impossible forever*. It means: **no engineering
work is performed on it under the current roadmap.**

**52 archived tasks are recorded here.**

---

## A general-purpose C++ gameplay reflection ecosystem

**What it was.** A whole phase making project-defined C++ behaviour a first-class authoring
concept: a reflection mechanism for project components, property metadata with ranges and enums,
asset and entity reference properties, serialisation of component data, reading component metadata
without loading game code into Studio, Inspector editing of those properties, generating component
boilerplate, and attaching components to entities — plus the ownership rules for the files that
generation would create.

**Why it is out.** This is the largest single piece of new infrastructure in the whole programme
and it is the defining feature of the engine editors CNA Studio is not. It begins with an
unanswered architectural question — which reflection mechanism — whose answer obliges Studio to a
metadata format, a generator, a build integration, an ABI contract, and a regeneration story that
must never overwrite a user's edits. Every one of those is a permanent maintenance liability, and
none of them is needed to create a project, edit a scene, build it and play it. A CNA developer
declares components in C++ in their own editor, which is what C++ developers do.

**What remains instead.** The Inspector edits the descriptor-driven properties Studio already
models, and `CORE-02` opens the developer's real IDE on the code.

| Ids | What it was |
|-----|-------------|
| `STUDIO-15001`, `STUDIO-15002`, `STUDIO-15003`, `STUDIO-15004`, `STUDIO-15005`, `STUDIO-15006`, `STUDIO-15007`, `STUDIO-15008`, `STUDIO-15009`, `STUDIO-15011`, `STUDIO-31021` | The reflection decision, component identity and registration, property metadata, reference properties, component serialisation, out-of-process metadata reading, Inspector editing of project components, boilerplate generation, attaching components, runtime property synchronisation, generated-file ownership |

## Live property injection, native code reload, and embedding the game view

**What it was.** Editing a property in Studio and seeing it change in the running game; keeping
Studio's selection in step with the player's; reloading recompiled native code into a running
process; and displaying the player's output inside a Studio viewport.

**Why it is out.** The first two are downstream of the reflection ecosystem above and cannot exist
without it. Native code reload is a research row whose own acceptance admits the first shipped
answer would be *save, compile, restart, restore context* — which is what Stop and Play already do,
without a reload mechanism to maintain. Embedding the player's output inside Studio pushes against
the process separation that makes a crashing game survivable; the row itself says the architecture
must not be compromised to get an embedded image, and no uncompromising way to do it was found.

**What remains instead.** A separate player process with Play, Pause, Step, Stop and Restart, live
asset and scene reload, log routing, crash isolation and screenshot capture — all tested against a
real process.

| Ids | What it was |
|-----|-------------|
| `STUDIO-16005`, `STUDIO-16007`, `STUDIO-16020`, `STUDIO-16021`, `STUDIO-16022` | Live property edits into the player, selected-entity synchronisation, displaying player output in a Studio viewport, deciding the native reload strategy, implementing it |

## A general-purpose shader graph

**What it was.** A node-based authoring surface for CNA's modern graphics API: a graph canvas,
typed pins, link validation, node search, copy/paste/undo on the graph, comments and groups,
generated shader and effect output, live preview, compilation errors mapped back to nodes, and
dialect targeting across every shader language CNA supports.

**Why it is out.** It is a compiler with a user interface. Generated output across multiple shader
dialects, error mapping back to nodes, and live preview are each a subsystem, and together they
oblige Studio to track every dialect CNA ever supports. CNA Studio has a working material workflow
that authors what the runtime can execute; a shader graph is the feature that turns a companion
tool into an engine editor.

**What remains instead.** The material and lighting workflows of phases 19 and 20, both complete.
Shaders are written as shaders, in the project, in the developer's editor.

| Ids | What it was |
|-----|-------------|
| `STUDIO-22001`, `STUDIO-22002`, `STUDIO-22003`, `STUDIO-22004`, `STUDIO-22005`, `STUDIO-22006`, `STUDIO-22007`, `STUDIO-22008`, `STUDIO-22009`, `STUDIO-22010`, `STUDIO-10008` | Graph canvas, node and pin model, typed links, node search, graph clipboard and undo, comments and groups, generated output, live preview, error mapping, dialect targeting, shader and effect assets |

## A frame debugger

**What it was.** Capturing a frame and inspecting its render passes, resources and individual draw
calls.

**Why it is out.** RenderDoc exists, is better at this than anything Studio would build, and works
on a CNA game today because a CNA game is an ordinary native process. Reproducing it inside Studio
is the clearest possible case of duplicating a standard development tool.

| Ids | What it was |
|-----|-------------|
| `STUDIO-27010`, `STUDIO-27011`, `STUDIO-27012` | Frame debugger, render-pass and resource inspection, draw-call inspection |

## A plugin SDK and third-party extension ecosystem

**What it was.** A versioned plugin API with explicit ABI checks, plugins contributing importers,
component descriptors, panels, commands, inspectors, gizmos, exporters, validators and build
integration, plus SDK documentation and a worked example.

**Why it is out.** Every contribution point is a public contract that must keep working across
Studio versions, and an ABI-checked plugin API for a C++ application is a permanent tax on every
internal change — paid by a product whose own feature development has stopped. There is no
third-party audience for it.

**What remains instead.** The plugin host the prototype already provides continues to load plugins
that add panels, menus, component types and importers, and stays maintained. It is not extended,
not versioned as a public SDK, and not documented as one.

| Ids | What it was |
|-----|-------------|
| `STUDIO-28001`, `STUDIO-28002`, `STUDIO-28003`, `STUDIO-28004`, `STUDIO-28005`, `STUDIO-28006`, `STUDIO-28007`, `STUDIO-28008`, `STUDIO-28009`, `STUDIO-28010`, `STUDIO-28011`, `STUDIO-28015`, `STUDIO-33004` | API versioning and ABI checks, safe failure of a bad plugin, contributed importers, descriptors, panels, commands, inspectors, gizmos, exporters and validators, build integration, SDK documentation and example, unload notification |

## The broad production-polish campaign

**What it was.** **CNA Studio Visual Quality 1.0** and the audits around it: a consistent visual
language audit across every panel, an interaction predictability audit, an error-message quality
pass, an empty-state pass, tooltip coverage, drag-and-drop polish everywhere, context-menu
coverage, progress and cancellation coverage, and a sensible-defaults review — held to the standard
that Studio must read as *a serious modern 3D game-development environment* benchmarked against
Unreal, Unity, Godot and Blender.

**Why it is out.** Its own exit criterion says *never "done"*, and its benchmark is the class of
product the ADR says CNA Studio is not. An unbounded campaign measured against professional engine
editors cannot coexist with a 40-hour budget, and "professional editor completeness" is explicitly
not a reason to keep work. Individual panels can still be fixed when they are demonstrably hard to
use — that is maintenance, and the specific visual rows are in
[`ROADMAP-BACKLOG.md`](ROADMAP-BACKLOG.md).

**What remains instead.** Thirteen rows of the campaign already landed and are kept: tab strips,
panel chrome, asset icons, list rows, vector fields, the content card grid, cached thumbnails, the
viewport toolbar, outliner visibility toggles, status-bar density and the visual regression suite.
Three more that genuinely block the Core workflow were kept as active work: `CORE-03`, `CORE-04`
and `CORE-05`. One hygiene row — no unfinished debug text in a normal workflow — is folded into
`CORE-10`.

| Ids | What it was |
|-----|-------------|
| `STUDIO-35001`, `STUDIO-35002`, `STUDIO-35003`, `STUDIO-35004`, `STUDIO-35005`, `STUDIO-35006`, `STUDIO-35007`, `STUDIO-35008`, `STUDIO-35009` | Visual language audit, interaction predictability audit, error message quality, empty states, tooltips, drag-and-drop polish, context menus, progress and cancellation, sensible defaults |

---

## Also out, and already recorded as such

The archived roadmap's *Deliberately not built* table predates this decision and still stands. An
embedded C++ IDE, visual scripting, a physics engine owned by Studio, a particle runtime owned by
Studio, a proprietary build system, and a mandatory binary project database were never in scope and
are not in it now. See [`ROADMAP-ARCHIVE.md`](ROADMAP-ARCHIVE.md).
