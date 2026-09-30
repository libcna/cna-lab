# C probes

Small standalone C programs that answer a question about CNA's own behaviour before Java is
written against it. They exist because some questions cannot honestly be answered from Java: a
segfault inside CNA is not a Java exception, and a JNI layer in between only makes the answer
harder to read.

Build one into CNA's shared probe directory against a staged C ABI artifact (each carries a
`PROVENANCE.txt`), never into `/tmp`:

```sh
ART=~/deps/cna-c-abi-0.35.0                 # HEADLESS; or ~/deps/cna-c-abi-0.35.0-opengles3-fx
OUT=../cna/build-probe/qual-probes
cc -std=c11 -Wall -Wextra -g -O0 -D_GNU_SOURCE -I"$ART/include" \
  -o "$OUT/java-cnb_model_roundtrip" tools/native-abi/probes/cnb_model_roundtrip.c \
  -L"$ART/lib" -lcna_c_api -lpthread -Wl,-rpath,"$ART/lib"
"$OUT/java-cnb_model_roundtrip"
```

A windowed artifact runs only inside CNA's private GPU runner, never on a desktop session:
`../cna/tools/platform/run_gpu_tests_private.sh --exec "$OUT/java-<probe>"`.

**Call a route before printing its outputs.** `printf("%s %d", name_of(route(&out)), out)`
leaves C free to read `out` before `route` writes it, and GCC does. Three findings
(JAVA-UPSTREAM-018, -020, -021) were exactly that; the probes below sequence every call first.

## Retired 2026-09-30

CNA ABI 0.30 removed `engine_layer.h`, and the probes that measured it went with it:
`chain_owned_pass`, `compute_and_storage`, `compute_compile_contract`, `cube_lut_refusal`,
`effect_pass_ownership`, `engine_layer_families`, `gpu_renderer_qualification`,
`instanced_draw_refusal`, `lent_effect_lifetime`, `lent_handles`, `lent_handle_use_after_lender`,
`light_probe_bake`, `pass_support_versus_behaviour`, `pbr_effect_material`,
`pipeline_scene_callbacks`, `pipeline_scene_callback_scene`, `shader_effect_cache` and
`transparent_draw_order`. Their measurements and findings (JAVA-UPSTREAM-005..013, -015) are in
Git history and `docs/backlog.json`.

## Re-measured against ABI 0.35.0 (2026-09-30)

| probe | result |
|---|---|
| `content_manager_model_teardown.c` | exits 0 in destroy, leave-to-manager and walk modes on HEADLESS and OPENGLES3 (JAVA-UPSTREAM-004 fixed) |
| `exit_with_live_graph.c` | `thread`, `thread-bare` and `main` all exit 0 on OPENGLES3 (JAVA-UPSTREAM-014 fixed, BINDFIX-041) |
| `camera_test_backend.c` | `PROBE_CASE=0` exits 0 on OPENGLES3 with the device layer (JAVA-UPSTREAM-019 fixed); sequenced, the wrong-size acquire writes `acquired=no` and `is_supported` answers yes (-020, -021 withdrawn) |
| `renderer_selection.c` | env mode reaches `main` with an absent renderer named (JAVA-UPSTREAM-017 fixed, BINDFIX-038); identity mode answers count 1, selection HEADLESS and latched after a device exists (-018 withdrawn) |
| `shader_effect_uniform_binding.c` | ported to SpriteBatch; paints the uniform in all three orders on OPENGLES3 (JAVA-UPSTREAM-016 fixed) |

The sections below are each probe's original record, kept for the measurements they carry.

## cnb_model_roundtrip.c

Builds a model with two bones, one part with real vertex and index bytes, and one mesh, encodes
it, parses it back and decodes it, then releases everything.

Written because JAVA-UPSTREAM-004 found `cna_content_manager_load_model` segfaulting during
teardown for any asset with a mesh part. The `.cnb` model family is a different code path, but
that was worth measuring rather than assuming before binding 33 routes against it. It prints
`PROBE OK` and exits zero on ABI 0.21.0.

It also earned its keep twice over: the first two runs failed, and taught the Java layer two
things CNA requires that no header comment states outright -- a part's declared
`vertex_stride * vertex_count` has to equal the bytes it holds, and the primitive topology of a
triangle list is 4.

## content_manager_model_teardown.c

`JAVA-UPSTREAM-004`, and the reason `CnaModel.Load` does not exist.

Loading a Model through CNA's own content manager and destroying it segfaults inside
`PartResource::~PartResource` for any asset whose meshes have parts -- which is every real model.
CNA's own content fixtures are models with one bone and no meshes, which is why the path is
uncovered upstream.

It is a source probe rather than a note because *"still broken"* is a measurement that has to be
retaken against each CNA this repository qualifies against, and a segfault is not something a Java
test can report. Retaken on 2026-08-31 against ABI 0.21.0, on HEADLESS and on OPENGL33:

```text
game_create=0  get_graphics_device=0  content_manager_create=0
load_model=0 model=4294967303
destroying
<SIGABRT>
```

The model loads. `cna_model_destroy` does not return.

## exit_with_live_graph.c

`JAVA-UPSTREAM-014`. What happens when a process exits with a CNA game still alive?

A game that is never destroyed is not a hypothetical: a JVM exiting on an unhandled exception, a
`System.exit`, or simply a program that lets the operating system reclaim everything all leave the
native graph standing. CNA-Java has had a subprocess test for exactly that since the ownership
graph existed, and on HEADLESS the process exits zero.

On the EasyGL renderers it aborts -- `terminate called without an active exception`, SIGABRT --
and this reproduces it with no Java anywhere, one case per process because the interesting
outcomes are crashes:

| case | HEADLESS | SOFTWARE | OPENGL4 | OPENGLES3 | OPENGL33 |
|---|---|---|---|---|---|
| a live game | 0 | 0 | 0 | 0 | 0 |
| a live device manager | 0 | 0 | 0 | 0 | 0 |
| a frame run | 0 | 0 | 0 | 0 | 0 |
| a live static or dynamic vertex buffer | 0 | 0 | 0 | 0 | 0 |
| all of it on a thread that ends first | 0 | 0 | 0 | **134** | **134** |
| that thread with **no** buffer | 0 | 0 | 0 | 0 | 0 |

**Both conditions are necessary and neither alone does it.** A buffer alive at exit is fine; a
thread that ends before the process is fine; the two together abort.

The second condition is what makes this reach every Java program rather than an unusual one. The
`java` launcher runs `main` on a thread it creates, not on the process's initial thread, so the
thread that made the game and its GL context has already ended by the time the process exits --
without anyone choosing that. Narrowing from the Java side agreed exactly: a plain game with a
device and a frame exits cleanly, and adding one `VertexBuffer` -- static or dynamic, bound or
not, with a listener or without -- aborts.

## camera_test_backend.c

Can a camera be qualified on a machine with no camera?

Fifteen camera routes were unbound behind the reason "camera ... are CNA device extensions beyond
XNA 4.0", which says why they are an extension and not why they are absent. The assumption behind
leaving them was that a webcam API needs a webcam. It does not:
`cna_camera_create_with_test_backend_ext` builds one whose frames the caller supplies and whose
state the caller drives, so every route can be exercised and the pixels compared.

**A frame goes in and comes back out, texel for texel.** A 4x3 frame whose channels are functions
of the texel index, set through `cna_camera_set_test_frame_ext`, read back out of a `Texture2D`
after `cna_camera_try_acquire_frame_ext` -- first texel `0,255,0` in and `0,255,0` out, last
`187,68,33` and `187,68,33`. The frame is kept rather than consumed: acquiring twice gives it
twice.

**The size check is exactly right, and the first reading of it was not.** The header says a
texture whose size does not match the frame is refused, and against one fixed 2x2 texture:

| Frame | acquired | wrote into the texture |
|---|---|---|
| 2x2 | yes | yes |
| 3x2, 2x3, 3x3, 4x3, 4x4, 8x8, 16x16 | no | no |

The first version of this probe reported that a 4x3 frame was acquired into a 2x2 texture. It was
not: the probe had written `CNA_Bool acquired = CNA_TRUE` before a call that does not write on its
"nothing to give" path, and read its own initialiser back. Every out-parameter here is poisoned
with `0xAB` first, so "untouched" is a visible third answer -- which is how the next finding was
found rather than missed.

**Three defects, and the family is not bound because of the first.**

`JAVA-UPSTREAM-019`: **destroying a camera after a longer session segfaults the process.**
Reproducible three runs of three, on HEADLESS and OPENGL33 alike:

```sh
PROBE_CASE=0 ./camera_test_backend                # SIGSEGV
PROBE_CASE=0 PROBE_SKIP=camera ./camera_test_backend   # exit 0, game destroy SUCCESS
```

One `cna_camera_destroy` fewer and the run is clean, so the call is named. It resists reduction
and the reduced modes are kept anyway, because ruling out the obvious guesses is most of what a
reader needs next: `minimal` (two cameras created and destroyed), `minimal-acquire` (one camera
used then destroyed) and `minimal-two-used` (a second camera created, used and destroyed while
the first lives) are all clean.

`JAVA-UPSTREAM-020`: **`try_acquire_frame_ext` returns SUCCESS without writing `out_acquired`** on
the paths where it acquires nothing, though the header says no frame ready is an ordinary
`CNA_FALSE`. Harmless to a Java binding, whose `boolean[1]` is zero-initialised, and not harmless
to the C callers the ABI exists for.

`JAVA-UPSTREAM-021`: **`cna_camera_create` succeeds where `is_supported` says no** and
`get_count_ext` says zero -- the fourth capability query in this backlog that does not predict the
behaviour behind it. And `set_test_state_ext` refuses `NOT_SUPPORTED` and `CLOSED`, the first of
which is the state a test-backend camera is born in, so the setter cannot restore the object's own
initial state.

So the family stays unbound with a measured reason rather than an assumed one. Binding a route
that can kill a game is worse than leaving the family out, and the qualification that would have
justified binding it is what found the crash.

## renderer_selection.c

Which renderers does this build have, and what happens when a caller asks for one it does not?

The second half of that question was answered by accident: a qualification sweep named
`OPENGLES2`, a renderer this library was configured without, and the JVM printed
`terminate called after throwing an instance of 'System::InvalidOperationException'` and died with
signal 6. The message was a good one. The delivery was a process abort across a C ABI whose whole
contract is that failures come back as a `CNA_Result`.

**The abort is at library load, not at first call.** Run this probe with
`CNA_GRAPHICS_RENDERER=OPENGLES2` and nothing of it runs -- not `main`, not a
`__attribute__((constructor))` of the program itself. A trivial program that links the library but
references no symbol from it survives, because `--as-needed` drops the `DT_NEEDED` entry and the
library is never loaded at all; add one reference and it dies. So there is no point at which a
caller could guard it, and in a JVM `System.loadLibrary` never returns. Recorded as
`JAVA-UPSTREAM-017`.

**The API path is safe, and is the whole reason to bind this family.** Every setter refuses
cleanly:

| Call | result |
|---|---|
| `set_preferred_ext(OPENGLES2)` -- defined, not in this build | `INVALID_STATE` |
| `set_preferred_by_name_ext("OPENGLES2")` | `INVALID_STATE` |
| `set_preferred_by_name_ext("NOT_A_RENDERER")` | `INVALID_ARGUMENT` |
| `set_preferred_ext(9999)` -- outside the table | `INVALID_ARGUMENT` |
| `set_preferred_by_name_ext("HEADLESS")` | SUCCESS |
| `set_fallback_chain_ext(chain, 1)` with an undefined identity | `INVALID_ARGUMENT` |
| `set_fallback_chain_ext(NULL, 1)` | `INVALID_ARGUMENT` |
| `set_fallback_chain_ext(NULL, 0)` | SUCCESS |
| any setter once a renderer exists | `INVALID_STATE` |

`try_parse_name_ext` is case-insensitive, treats an unrecognised name as an answer rather than a
failure, and parses names for renderers this build does not have -- which is right: parsing a name
and having the renderer are different questions.

**Creating a device resets three query routes.** `JAVA-UPSTREAM-018`. `identity` mode asks them
on a process that touches the selection not at all, before and after one `GraphicsDevice`:

| Route | before a device | after a device |
|---|---|---|
| `get_available_count_ext` | SUCCESS, **5** | SUCCESS, **0** -- while `copy_available_ext` still says 5 |
| `get_selected_ext` | SUCCESS, **the renderer the run asked for** | SUCCESS, **`UNKNOWN`** |
| `get_is_latched_ext` | SUCCESS, not latched | SUCCESS, **still not latched** -- it never reports the state it exists to report |
| `get_active_ext` | `INVALID_STATE`, correctly | SUCCESS, **the running renderer** -- correct |
| `copy_available_ext` | 5 identities | 5 identities -- correct |

**This corrects an earlier reading of the same family, and the correction is the more useful
finding.** Asked from `main`, which calls `set_preferred_by_name` and
`reset_selection_for_tests_ext` before it gets there, every one of these looked broken and the
conclusion drawn was "write-only". They were being asked after the probe had rearranged the state
they report. Measured without that, they are correct until a device exists and three of them are
reset by creating one -- a narrower defect with a specific trigger, rather than five unrelated
ones. A probe that mutates what it is about to measure will always find something.

**Three routes named `current` report the compile-time default, not the running renderer.** On a
build configured `CNA_GRAPHICS_RENDERER=HEADLESS` with five renderers compiled in, running under
`CNA_GRAPHICS_RENDERER=OPENGL33`:

| Route | answers |
|---|---|
| `cna_graphics_renderer_copy_current_name` | `"HEADLESS"` -- and its declaration does say it matches the build option, so this one is honest |
| `cna_graphics_renderer_get_current_type` | `HEADLESS` |
| `cna_graphics_backend_get_current_category` | `Diagnostic`, HEADLESS's category |
| `cna_graphics_backend_get_current_maturity` | `Supported`, HEADLESS's maturity |
| `cna_graphics_renderer_get_active_ext` | **`OPENGL33`** |
| `cna_graphics_device_copy_renderer_name` | **`"OPENGL33"`** |

The two that are right are the two that ask about something real -- the selection that happened,
and the device in hand. The word "current" in the other three means "the one this build was
configured with", which on a single-renderer build is the same thing and on this one is not.

**Per-identity classification works for every identity, compiled in or not:**

| Identity | category | maturity |
|---|---|---|
| HEADLESS | Diagnostic | Supported |
| SOFTWARE | Software | Experimental |
| OPENGL33, OPENGLES3, OPENGLES2, VULKAN | Native | Production |
| OPENGL4 | Native | Supported |
| STUB | Diagnostic | Supported |
| UNKNOWN | `INVALID_ARGUMENT` | `INVALID_ARGUMENT` |

The enumeration itself is sound: `copy_available_ext` supports the zero-capacity probe, writes no
partial result when the buffer is one element short, and lists `HEADLESS OPENGLES3 OPENGL33
OPENGL4 SOFTWARE` -- which is what the `CNA_GRAPHICS_RENDERERS` cache entry says, read at runtime
rather than out of `CMakeCache.txt`. `get_is_available_ext` agrees with it identity by identity
and refuses `UNKNOWN` and out-of-range values. The four fallback reasons name themselves
`NotCompiledIn`, `ProbeUnavailable`, `InitializationFailed` and `WindowKindConflict`.

So CNA-Java binds eighteen of the family's twenty-two routes, including `get_active_ext`, and
`GraphicsRenderer.available()` sizes its buffer with the zero-capacity probe rather than the count
route -- which is what keeps it working after a device exists.

## shader_effect_uniform_binding.c

`JAVA-UPSTREAM-016`. When does a uniform set on a `ShaderEffect` actually reach the shader?

`cna_shader_effect_set_uniform_*` answers `SUCCESS` whatever else is going on, and on CNA's EasyGL
renderer the value is silently discarded unless the effect's own GL program happens to be the
current one. `EasyGLEffectRenderer::SetUniformFloat` and its eight siblings ask for a uniform
location and write to it without binding first, and `glUniform*` writes to whichever program is
current.

A fragment shader that writes nothing but a uniform, drawn into a render target and read back:

| renderer | uniform then apply | apply then uniform |
|---|---|---|
| HEADLESS | readback refused | readback refused |
| SOFTWARE | 0,255,0,255 (the source, unshaded) | 0,255,0,255 |
| OPENGL4 | **0,0,0,255** | **255,0,0,255** |
| OPENGLES3 | **0,0,0,0** | **255,0,0,255** |
| OPENGL33 | **0,0,0,0** | **255,0,0,255** |

The same renderer does it correctly one file over: `EasyGLComputeShaderRenderer::SetUniformInt`
opens with `program_.use()`. Two uniform setters in one renderer, one of which works from a cold
start and one of which does not -- which is what makes this a defect rather than a contract. CNA's
own passes are unaffected because they apply their effect as part of drawing; a game reaching the
routes directly is not, and `ShaderEffect.apply()` is where that is written down.

It also settles what a custom full-screen shader has to look like. The vertex program must match
what the pass feeds it -- `aPos`, `aTexCoord` and `aColor` at locations nought, one and two, plus a
`projection` uniform -- which is the eight lines every lens pass inside CNA shares. A shader that
names its attributes anything else compiles, reports itself valid, and draws nothing.
