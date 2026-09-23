# Getting started with CNA Studio

From a clone to a running game, through the workflow CNA Studio exists for.
`plan.md` `CORE-09` (`STUDIO-33001`).

**What you end up with is an ordinary CNA project.** A directory with a `CMakeLists.txt`, some C++
and some assets, which builds and runs with Studio uninstalled. That is not a footnote — it is the
one rule the whole product is shaped by, and the last step here is to prove it on your own machine.

---

## Before you start

| You need | Why |
|----------|-----|
| A C++23 compiler — GCC 13+, Clang 16+ or MSVC 19.38+ | Studio and CNA are both C++23 |
| CMake ≥ 3.20 | Studio drives *your project's* CMake; it never generates a build system of its own |
| A [CNA](https://github.com/libcna/cna) checkout | Only to **build a game**. Studio itself builds without one |

You do not need a GPU, a display, or any of CNA's sibling checkouts to build and run Studio. That
is deliberate: the default build has no external dependencies at all.

---

## 1. Build Studio

```bash
git clone https://github.com/libcna/cna-studio
cd cna-studio
cmake -S . -B build -G Ninja
cmake --build build -j4
```

Check it works:

```bash
./build/cna-studio --version
./build/cna-studio --list-templates
```

If `--list-templates` prints four templates, you have a working Studio.

> **This build has no viewport.** The CNA-backed viewport is opt-in, because it needs a CNA
> checkout; see [Building with the CNA viewport](../README.md#building-with-the-cna-viewport).
> Everything below works without it — including creating a project and building the game.

---

## 2. Create a project

From the Project Hub, or — which is the same code — from the command line:

```bash
./build/cna-studio --new-project=/tmp/MyGame --template=basic-sample --project-name="My Game"
```

```
/tmp/MyGame/
├── My Game.cnaproject     the project file Studio opens
├── CMakeLists.txt         yours; Studio drives it and never rewrites it
├── Source/Main.cpp        your game's entry point
├── Assets/                what the importers watch
├── Scenes/                what the editor edits
└── Runtime/               the scene loader the template brought with it
```

The four templates differ in what they start you with, not in what they let you do:

| Template | Starts you with |
|----------|-----------------|
| `basic-sample` | A scene with something in it. The one to pick first |
| `empty-2d` | A 2D project with an empty scene |
| `empty-3d` | A 3D project with an empty scene |
| `xna-compatible` | A hand-written `Initialize`/`LoadContent`/`Update`/`Draw` game, with no scene model at all |

---

## 3. Open it

```bash
./build/cna-studio --project="/tmp/MyGame/My Game.cnaproject"
```

Or headless, which is what a test does and what you can do over SSH:

```bash
./build/cna-studio --project="/tmp/MyGame/My Game.cnaproject" --headless --frames=2
```

```
[info] Opened project 'My Game' (CnaNative) at /tmp/MyGame
[info] Assets: 4 found, 0 new, 0 moved, 0 missing
[info] Opened scene 'Level01' with 5 entities
```

---

## 4. Do some work

The eleven steps and what they are each for are the [user guide](USER-GUIDE.md). The shortest path
to seeing something happen:

1. **Put a PNG in `Assets/`.** The Content Browser picks it up; there is no import button to find.
2. **Drag it into the viewport.** That makes an entity with a sprite on it.
3. **Move it** with the translate gizmo, or type numbers into the Details panel.
4. **Ctrl+S.**

Every one of those goes through the undo history, so Ctrl+Z reaches all of it.

---

## 5. Build the game

**Build > Build** (Ctrl+B), or `Clean Build` when you suspect the build tree rather than the code.
Studio runs your project's own CMake and shows you the exact commands first; `Copy Commands` puts
them on the clipboard so you can run them yourself.

A compiler error becomes a row naming the file, the line and the message. Activating the row opens
that place in the editor you set under **Preferences > Tools > External editor** — `clion`, `code`,
`vim`, or a full path. Set it before you need it; Studio refuses the gesture with a message rather
than guessing which editor you meant.

---

## 6. Play it

**Play > Play** (F5). The game runs in a **separate process** (`cna-player`), so a crash in your
game is a crash in your game: Studio reports it and stays up. Pause, Step and Stop do what they
say, and the log routes the player's output into the Output panel with its source marked.

---

## 7. Prove it does not need Studio

The step that matters most, and the one you should do once so that you believe it:

```bash
cmake -S /tmp/MyGame -B /tmp/MyGame/build -DCNA_ROOT=/path/to/cna -DCNA_GRAPHICS_RENDERER=SOFTWARE
cmake --build /tmp/MyGame/build -j4
cd /tmp/MyGame/build && SDL_VIDEODRIVER=dummy ./My_Game --frames=10
```

```
My_Game: loaded 5 entities, drew 2 sprites
```

No Studio anywhere in that. CI does exactly this for every template on every change
(`ctest -R CnaStudioTemplateBuilds`), because the one failure mode this rule has is the one that
looks fine until somebody tries it.

---

## 8. Carry on in your own editor

**Tools > Open Project in External Editor** hands the whole directory to CLion, VS Code or whatever
you configured. **Tools > Open Main Source in External Editor** opens `Source/Main.cpp`.

This is not an escape hatch — it is the point. Studio does the visual half; the C++ is written
where you write C++.

---

## Where to go next

| | |
|---|---|
| What each of the eleven steps does, and what Studio deliberately does not do | [`USER-GUIDE.md`](USER-GUIDE.md) |
| How it is built, and why | [`ARCHITECTURE.md`](ARCHITECTURE.md) |
| Which renderers and platforms a target can name | [`RENDERERS-AND-PLATFORMS.md`](RENDERERS-AND-PLATFORMS.md) |
| What is and is not being worked on | [`../plan.md`](../plan.md) |
