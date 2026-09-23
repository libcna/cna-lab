# The CNA Studio user guide

`plan.md` `CORE-09` (`STUDIO-33002`).

CNA Studio is **a lightweight visual development companion for CNA applications**. It is organised
around eleven steps, and this guide is one section per step. When those eleven are reliable, the
product is complete — which it now is, so this guide describes what Studio does rather than what it
is going to do.

**Read [what Studio does not do](#what-studio-does-not-do) before you go looking for something.**
It is a short list and it is deliberate.

---

## 1. Create or open a project

**Project Hub** — New, Open, and a recent list that greys out what has moved rather than failing
when you click it.

Four templates ship: `basic-sample`, `empty-2d`, `empty-3d` and `xna-compatible`. Each is created,
reopened, configured, compiled and run by a CTest case, so a template that produces a tree which
does not build is a red build rather than your afternoon.

A project declares its **kind**, and the kind decides what Studio offers:

| Kind | What you get |
|------|--------------|
| `CnaNative` | Scenes, entities, components, the Inspector, gizmos, prefabs, play mode, both viewports |
| `XnaCompatible` | Assets, importer settings, previews, renderer configuration and Play — and nothing else. Your game keeps its own `Initialize`/`LoadContent`/`Update`/`Draw` |

There is **no native file dialog**. The path field and the recent list open any project, and a file
browser is one of the things this product decided not to build.

## 2. Browse, import and manage assets

Put a file in `Assets/`. There is no import button: the **Content Browser** watches the directory,
and an asset gets a UUID that survives being renamed and moved. Cards carry cached thumbnails; the
list view is the same folder read a different way.

**Finding things.** A text search covers the name, the kind and the path, and *leaves the current
folder behind* — it looks at the whole project, because searching within one folder is how people
conclude an asset is gone when it is one folder over. The type filter narrows what you are
browsing, which is the opposite scope on purpose: a filter is a way of looking at where you are.

Both scale. A hundred thousand assets, searched or browsed, cost a screenful.

## 3. Create, open and edit scenes

Scenes and prefabs are documents. **Every mutation goes through the undo history** — there is no
path that edits the document directly, and that is enforced by a test rather than by a rule.

Tilemaps, layers and tags are here. So is format migration: a scene written by an older Studio is
upgraded on load, and one written by a *newer* Studio is refused with a message rather than
half-read.

## 4. The viewport, selection and gizmos

A 2D viewport and a 3D viewport, with camera navigation, translate/rotate/scale gizmos, and band
selection.

What is selected is outlined in both. A multi-selection also gets a box round the whole of it, and
a cross marks the point a rotation will turn about — which is the thing you need to know before
you press R.

## 5. Edit properties

The **Details** panel, driven by a descriptor model rather than by a switch on type. Object
properties, materials, lights and the scene's environment.

The label column is sized from the labels on screen, so a property called `x` sits next to its
value. A property that differs from its default is marked and can be reset, through the history
like everything else.

## 6. Save

Atomic writes — the file goes to a temporary and is renamed over the old one, so an interrupted
save leaves the previous version intact rather than half of both. Dirty tracking, autosave, and
recovery of a partially written file.

## 7. Play

**Play**, **Pause**, **Step**, **Stop** and **Restart**, against a real separate `cna-player`
process. That separation is the reason a crash in your game is a crash in your game: Studio reports
it, with the exit reason the operating system gave, and stays up.

The player's log is routed into the Output panel with its source marked. Assets reload live, and a
frame can be captured to a PNG.

## 8. Configure and run builds

The **Build** panel edits the project's target profiles — operating system, architecture, CNA
platform, CNA renderer, configuration and the optional CNA subsystems — with validation that knows
which combinations CNA will actually configure.

**Build** and **Clean Build** are two gestures. Clean runs your project's own `clean` target
between the configure and the build; it does not delete the directory.

Every command is displayed before it runs, and `Copy Commands` hands you the untruncated,
shell-quoted text. Studio drives *your* CMake and has no build system of its own.

A CNA option that is tri-state in CNA — `CNA_ENABLE_VIDEO` is `OFF`, `AUTO` or `ON` — is tri-state
here. `AUTO` uses FFmpeg where the build machine has it; `ON` requires it and fails the configure
without it, which is a thing a shipping project may well want to say.

## 9. Read the failure

A compiler error is a row naming the file, the line and the message — GCC, Clang, MSVC and CMake's
own configure errors. Activating the row opens that place in your editor.

The complete unparsed log is always one button away, whatever the parser made of it.

## 10. Recover from a crash

Autosave snapshots go to a per-user state directory. On reopening a project Studio finds the
snapshot that belongs to it and offers it; a snapshot is written atomically for the same reason a
save is.

The player reports its own crashes over the bridge, so a game that died tells you how.

## 11. Carry on in your own IDE

**Tools > Open Project in External Editor** and **Tools > Open Main Source in External Editor**,
using the editor named under **Preferences > Tools**. A full path or a name on the `PATH` —
`clion`, `code`, `vim`, `subl`, `kate` and their relatives are known well enough to be told which
line to go to.

If no editor is configured, the gesture is refused with a message naming the setting. Studio does
not guess: on Linux whatever opens a `.cpp` by default is as likely to be a text viewer as an IDE.

---

## What Studio does not do

Named so that their absence is a decision rather than something you are waiting for. None of these
is planned; the reasoning is [`ADR-001-SCOPE-REDUCTION.md`](ADR-001-SCOPE-REDUCTION.md).

- **It does not edit C++.** No code editor, no completion, no debugger. That is what step 11 is for.
- **No native file-browser dialogs** for New and Open Project.
- **No packaging, cooking or export pipeline.** Your project's own CMake already produces a runnable
  game, and CI proves it builds with Studio absent.
- **No installer for Studio itself.**
- **No shader graph**, and no node-based authoring of any kind.
- **No skeletal animation, particle, audio, terrain, physics or navigation authoring.** Sprite
  animation, audio preview and material and lighting editing are here; authoring tools for the rest
  are not.
- **No integrated profiler and no frame debugger.** RenderDoc and an ordinary profiler work on
  `cna-player`, which is an ordinary native process.
- **No C++ gameplay reflection**, no generated component boilerplate, no live property injection
  into a running game, no native code reload.
- **No plugin SDK** beyond the plugin host that already exists. Plugins can add panels, menus,
  component types and importers; there is no versioned public ABI.
- **No non-Latin text rendering, IME input, accessibility APIs or localisation.** A name in a
  non-Latin script renders as boxes. This one was cut on budget rather than on merit, and it is
  written down as such.

Something on that list can become active work — but only on the four-part test in
[`../plan.md`](../plan.md), which starts with a real, maintained CNA application hitting the need
repeatedly. "A professional editor has it" is not an argument, and neither is "it is in the
backlog".

---

## Keyboard

The chords follow the prototype's, which is what existing hands already know.

| | |
|---|---|
| Ctrl+N / Ctrl+Shift+N | New Scene / New Project |
| Ctrl+S / Ctrl+Shift+S | Save / Save All |
| Ctrl+Z / Ctrl+Y | Undo / Redo |
| Ctrl+B / Ctrl+Shift+B | Build / Clean Build |
| F5 / Ctrl+F5 / Shift+F5 | Play / Pause / Step |
| F12 | Capture the running game's frame |
| F2 | Rename whatever the Inspector is showing |
| F1 | About |

Every one of them is rebindable under **Preferences > Shortcuts**, and every command Studio has is
on a menu — a command reachable only by a chord is one nobody can discover, and a test refuses it.

---

## Running without a window

Studio's headless mode is not a lesser Studio: it opens projects, reads scenes, creates projects
from templates and exports them, with no GPU and no display.

```bash
cna-studio --list-templates
cna-studio --new-project=DIR --template=ID --project-name=NAME
cna-studio --project=FILE --headless --frames=N
cna-studio --project=FILE --export=DIR
cna-studio --ui-benchmark            # what a frame of UI costs, per panel
cna-studio --host-capabilities       # what Studio requires of a host renderer
```

`cna-studio --help` lists the rest.
