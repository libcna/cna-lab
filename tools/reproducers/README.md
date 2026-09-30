# C reproducers for the upstream findings

Each file here is a self-contained C program against CNA's public headers, with
no Rust in the process. That is the point: a finding measured only through a
binding invites the answer "then fix your binding", and these remove it.

They live in the repository rather than in `build-probe/` -- which is
gitignored -- because `docs/upstream-findings.md` cites them, and a finding
whose reproducer is not in the tree is a finding the next person has to rebuild
from prose. Build them *into* `build-probe/`, which is where every throwaway
binary in this repository goes:

```sh
ART=~/deps/cna-c-abi-0.35.0        # a staged CNA C ABI artifact (lib/ + include/)
cc -O0 -g -D_GNU_SOURCE -pthread tools/reproducers/<file>.c \
  -I$ART/include -L$ART/lib -lcna_c_api -Wl,-rpath,$ART/lib \
  -o build-probe/<file>
./build-probe/<file> [args]
```

The HEADLESS artifact is the default, because its renderer makes a
`GraphicsDevice` without a window. GL-family probes run inside CNA's private
GPU display (`tools/platform/run_gpu_tests_private.sh --exec`), never on a
live desktop.

**Status at ABI 0.35.0 (2026-09-30):** every finding these reproducers were
written for is fixed upstream except `RUST-UPSTREAM-027` (sample math); see the
status table at the top of `docs/upstream-findings.md`. The programs are kept
as regression probes.

| File | Finding | What it shows |
|---|---|---|
| `ext015g_model_ownership.c` | — | that a `models.h` view handle is independently counted: it keeps answering, name and all, after `cna_model_destroy`. This is why `ModelBoneView` and `ModelMeshView` carry no lifetime parameter. |
| `ext015g_load_model_destroy.c` | `RUST-UPSTREAM-021` | destroying a content-loaded model with a mesh part faults. Takes a content root and an asset name. |
| `ext015g_handbuilt_mesh.c` | `RUST-UPSTREAM-021` | the control: the same shape built by hand destroys cleanly, which is what makes *content-loaded* the answer. |
| `ext015g_manager_teardown.c` | `RUST-UPSTREAM-021` | that leaking the model handle does not avoid the fault; it moves it to process exit. |
| `census030_effect_technique_owner.c` | `RUST-UPSTREAM-030` | that a technique added to an effect's own collection could not be selected as its current technique. |
| `census031_avatar_loader_exit.c` | `RUST-UPSTREAM-031` | that exiting while the avatar loader is still assembling a model crashed or hung in static destruction. Takes a count of extra loads. |
| `census002_packet_truncation.c` | `RUST-UPSTREAM-028` | that a packet larger than the caller's buffer is silently cut and reported as a success -- `out_received` is the buffer, not the packet -- and that the `PacketReader` overload delivers all 5,000 bytes while reporting that it received none. Takes no arguments. |

## `ext015h_concurrent_device_create.c` — RUST-UPSTREAM-023

Six threads call `cna_graphics_device_create` at once. On a GL renderer about
one run in five dies with `SIGSEGV` or glibc's "double free or corruption";
serialising the create call alone removes it.

```sh
ART=~/deps/cna-c-abi-0.35.0-opengles3-fx
cc -O0 -g -pthread tools/reproducers/ext015h_concurrent_device_create.c \
   -I$ART/include -L$ART/lib -lcna_c_api -Wl,-rpath,$ART/lib \
   -o build-probe/ext015h_concurrent_device_create
for i in $(seq 1 40); do ./build-probe/ext015h_concurrent_device_create >/dev/null 2>&1 || echo abort; done
```

Environment knobs, all off by default: `REPRO_THREADS=<1..6>`,
`REPRO_NO_DESTROY=1` (leak the handles), `REPRO_SERIALIZE_CREATE=1`,
`REPRO_SERIALIZE_DESTROY=1`. A run needs no display; it is the GL renderer
being *built* that matters, not whether it ends up with a surface.
