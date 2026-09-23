# Renderers and platforms, for contributors

`plan.md` `CORE-09` (`STUDIO-29006`).

What the model *is* — two axes, fifty renderer identities, a runtime capability profile — is
[`ARCHITECTURE.md`](ARCHITECTURE.md) §2 to §4, and this document does not repeat it. What is here
is the part a contributor needs: **where each piece of it lives, what is allowed to know what, and
which guard fails when you get it wrong.**

---

## The one rule

> **Nothing outside the catalogue names a renderer.**

No `if (renderer == "vulkan")`, no list of backends in a panel, no switch on a platform name in the
build planner. A renderer identity is data that CNA publishes and Studio classifies, and code that
hard-codes one is code that stops being true the next time CNA adds a backend — silently, because
an unknown renderer looks exactly like a renderer the user did not pick.

`NoStudioCodeHardCodesARendererName`, in `ArchitectureGuardTests.cpp`, refuses the comparison
anywhere but in the catalogue itself. It is
worth knowing that the guard exists before you write the line rather than after.

---

## Where it lives

| Piece | File | Answers |
|-------|------|---------|
| The catalogue | `CNA/Studio/Project/RendererCatalog.hpp` | Which identities exist, which family implements each, which are available on which operating system, and what a legacy name migrates to |
| The target profile | `CNA/Studio/Project/TargetProfile.hpp` | What *one build of a game* is: OS, architecture, CNA platform, CNA renderer, configuration, feature options — and whether that combination is buildable |
| The host contract | `CNA/Studio/Project/StudioHostRequirements.hpp` | What Studio requires of the renderer **it** runs on, which is a different question from what a game requires |
| The capability bridge | `CNA/Studio/Viewport/CnaCapabilityBridge.hpp` | Turning CNA's runtime `RendererCapabilityProfile` into the answers above. The only module that links CNA |
| The build commands | `CNA/Studio/Project/Cpp/CppToolchain.hpp` | Turning a profile into `-DCNA_GRAPHICS_RENDERER=…` and the rest |

The separation that matters most is the third row from the fourth. **The renderer Studio draws its
own UI with and the renderer a game targets are unrelated**, and conflating them is the mistake
`docs/UI-RENDER-PATH.md` exists to name: a user on a machine that cannot run Vulkan can still build
a Vulkan game, and a Studio that refused would be wrong.

---

## Adding a renderer identity

CNA adds one; Studio finds out. In order:

1. **Do nothing first.** `TheRendererCatalogueClassifiesEveryAuditedCnaRenderer` fails,
   naming the identity — and `TheRendererCatalogueClaimsNothingCnaDoesNotHave` fails from the
   other direction if you ever classify one CNA does not have. That failure *is* the notification, and it is why Studio does not keep a
   copy of CNA's list that somebody has to remember to update.
2. **Classify it in the catalogue** — its family, and the operating systems CNA will configure it
   on. The gate table is transcribed from `cmake/RendererSelection.cmake`, and
   `TheRendererGateTranscriptionStillMatchesCnasOwnConfigureRules` reads CNA's own file and
   checks the transcription against it, on a build that has a CNA checkout to read.
3. **Nothing else.** The Build panel, the validation, the diagnostics and the export all read the
   catalogue. If a fourth place needs editing, that place is the bug.

## Renaming one

CNA renamed `D3D11` to `DIRECTX11`, and `EASYGL` stopped being a renderer at all — it became a
family serving five GL profiles. Projects written before that name a renderer that cannot be built.

The catalogue carries a **legacy alias table**, and a profile naming an old identity is migrated on
load *and told so*. Migrating in silence would change which renderer somebody's game ships on
without their knowing, which is worse than refusing.

---

## Platforms

`CNA_PLATFORM` is the other axis, and CNA rejects invalid combinations at configure time rather
than at run time. Implemented today: `SDL3` (default), `SDL2`, `HEADLESS`, `TERMINAL` (POSIX only).
Reserved and a hard configure error: `SDL12`, `WIN32`, `EMSCRIPTEN`.

Studio models them separately everywhere — a profile, the Build panel, the validation and the
capability report — because they *are* separate, and one flat "backend" field is exactly the model
the prototype had and current CNA does not.

---

## Capabilities: four answers, not two

`RendererFeature` is answered `Supported`, `Restricted`, `Unsupported` or `Unknown`.

**`Unknown` is not `Unsupported`.** It means the renderer has not classified the feature, and
Studio treats it as not-satisfied for a *required* feature and says so. A tool that starts and then
fails to draw is worse than one that refuses with a reason — and a tool that refuses because it
read a silence as a no would be worse than both, which is why the distinction is carried rather
than flattened.

`cna-studio --host-capabilities` prints what Studio requires and, on a build with a device, what
this machine answers.

---

## What to run before you push

```bash
ctest --test-dir build -R "Renderer|Capabilit|TargetProfile"
```

On a CNA-backed build, `TheRendererGateTranscriptionStillMatchesCnasOwnConfigureRules` and the capability-bridge
cases are the ones that can only be true with a checkout to read; on a dependency-free build they skip rather
than pass vacuously, which is a distinction the failure messages make explicitly.
