# Changelog

CNA Studio's releases, newest first.

## The convention

`plan.md` `CORE-11`. Short, because a convention nobody can remember is one nobody follows.

- **Versions are `MAJOR.MINOR.PATCH`**, set in `CMakeLists.txt`'s `project()` and reported by
  `cna-studio --version` and by Help > About. There is one place to change it.
- **`MAJOR`** changes when the Core workflow's shape changes — a step added, a step removed, or a
  project file that an older Studio can no longer open. Under maintenance mode this is not
  expected to move.
- **`MINOR`** changes when a maintained behaviour changes in a way a user would notice: a fix that
  alters what a gesture does, an adaptation to a CNA API change, a small usability fix inside the
  Core workflow.
- **`PATCH`** changes for a fix that does not change what anything does — a crash, a leak, a
  sanitizer or warning finding, a correctness bug whose correct behaviour is what people already
  expected.
- **Every entry names what changed for a *user***, not what changed in the code. "The Content
  Browser's search no longer builds a card per match" is a code note; "searching a large project
  no longer stutters" is the entry. Where an entry has a task id, it cites it.
- **An entry is written when the change lands**, under an `Unreleased` heading, and the heading is
  renamed on the day a version is cut. A changelog reconstructed from the log at release time is a
  changelog written by whoever can be bothered.

There are no pre-releases, no release branches and no tags older than this file. `docs/ORIGIN.md`
records where the code came from before any of it.

---

## Unreleased

### Fixed

- **The light theme now reaches the viewport.** Choosing Light in Preferences rethemed the panels
  and left the scene viewport black, because the CNA scene renderer carried its own colours instead
  of reading the theme's. The viewport's background, grid and origin line now come from the same
  theme the panels do, and the headless preview and the CNA renderer agree about them for the first
  time. The game view and the camera preview are unchanged: those clear to the *camera's* colour,
  because they are pictures of the game rather than of the editor.

---

## 1.0.0 — 2026-09-23

**The Core workflow is complete, and CNA Studio enters maintenance mode.**

What 1.0.0 means here is narrower than it usually is, and is worth stating: the eleven steps
[`plan.md`](plan.md) names are built, tested and documented, and planned feature development has
stopped. It is not a claim of parity with Unity, Unreal, Godot or any other tool —
[`docs/ADR-001-SCOPE-REDUCTION.md`](docs/ADR-001-SCOPE-REDUCTION.md) says at length why that
comparison is not one this product is trying to win.

### You can now

- **Build your project and read the failure.** The Build panel runs your own CMake, shows every
  command before it runs and hands them to the clipboard untruncated. Clean and incremental are two
  gestures. A compiler error — GCC, Clang, MSVC or CMake's own — is a row naming the file, the line
  and the message, and activating it opens that place in your editor. The whole unparsed log is one
  button away. (`CORE-01`)
- **Hand the project back to your IDE.** Tools > Open Project in External Editor, and Open Main
  Source, using the editor you name in Preferences. The preference had existed since the prototype
  and had never had a caller: Studio was claiming an integration it did not have. (`CORE-02`)
- **Say what a tri-state CNA option should be.** `CNA_ENABLE_VIDEO` is `OFF`, `AUTO` or `ON` in the
  target profile, so a project that *requires* video can say so. Projects written before this build
  exactly as they did. (`CORE-01`)
- **See what you have selected.** A box round a whole multi-selection and a cross at the point it
  turns about, in both viewports. (`CORE-03`)
- **Read a property grid.** The label column is sized from its labels instead of being 38% of the
  panel, so a property called `x` sits beside its value rather than a screen away from it.
  (`CORE-04`)
- **Search a large project without stutter.** A search over a hundred thousand assets costs a
  screenful. It used to build a card for every match, twice a frame, while forty were on screen.
  (`CORE-05`)
- **Resize the window and have nothing else change.** The scene no longer stretches for a frame.
  (`CORE-07`)

### Under it

- **Paint order is separable from input order.** A control that must win a click against the row it
  sits on is described first, as the router requires, and painted last, through
  `StudioFrame::paintOverSurface`. Four defects came out of the two being the same thing — a
  disclosure triangle that vanished on alternate rows, a visibility toggle that was never once on
  screen — and the per-widget guards are replaced by one that fails on the shape. (`CORE-06`)
- **One test walks the whole workflow** against a real CNA build: create, import, edit, save,
  reopen, build, play, kill the player, recover. (`CORE-10`)
- **Four questions the roadmap left open are answered**, in
  [`docs/ADR-002`](docs/ADR-002-THE-FOUR-SILENT-QUESTIONS.md) — including that no compatibility
  shims are owed for the renamed API and no migration is owed for the prototype's state directory,
  each with the reasoning and a test. (`CORE-08`)
- **The product is documented**: [getting started](docs/GETTING-STARTED.md), a
  [user guide](docs/USER-GUIDE.md) that says plainly what Studio does not do, and the
  [renderer and platform model](docs/RENDERERS-AND-PLATFORMS.md) for contributors. (`CORE-09`)

### Known and deliberate

Everything in the user guide's *What Studio does not do* list. The one worth repeating here is that
**non-Latin text renders as boxes**: localisation and accessibility were cut on budget rather than
on merit, and that is recorded as the least comfortable line in `ADR-001`.

### From here

Maintenance: bug fixes, compatibility fixes, adaptation to CNA API changes, security and
correctness work, small usability fixes inside the Core workflow, and features justified by a
demonstrated recurring need in a real maintained CNA application. Not working through a backlog.
