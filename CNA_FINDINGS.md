# CNA findings

What cna-killer found in CNA (`../cna`, branch `next` at `8dc7a7b99`, renderer OPENGLES3/EasyGL,
SDL3, private Weston + Xwayland on an AMD GPU, `SDL_AUDIO_DRIVER=dummy`), from seeds 1–5 at every
intensity, 2026-09-30. Every XNA rule cited was read from the XNA 4.0 assemblies in
`../xna4-decomp`.

## Status (CNA `next`, 2026-09-30)

| id | status |
|---|---|
| KF-1, KF-1a, KF-2, KF-16 | fixed, `1c04beed5`: the game thread holds the context for whole frames; resource operations lease it on any thread; `resources_` is guarded; no MSAA back buffer is built while the drawable is 0x0 |
| KF-3 | not reproduced after `1c04beed5` (seed 2 nightmare runs past tick 61); a real resize resets the scissor rectangle, the throw followed KF-2's failed reset |
| KF-4 | not a defect; comment corrected in `f2752e2a8` |
| KF-5 | fixed, `5c5a58cf4` |
| KF-6 | partly fixed, `8052300d2`: EasyGL reads the back buffer at its own size through the presentation rectangle (Letterbox, Stretch, display scale); the rest is a documented limitation (see the row) |
| KF-7, KF-8 | fixed, `4874c521d` |
| KF-9 | fixed, `d2020f892` |
| KF-10 | not a defect: cna-killer's own rectangle fit exactly in an odd-width back buffer; the check is corrected |
| KF-11 | kept deliberately, `d3273a9fe`; cna-killer accepts either outcome |
| KF-12 | fixed, `9335b38f0` |
| KF-13 | fixed, `ca2745cf5` |
| KF-14 | fixed, `a5c559786`, with a use-after-free the rejected add exposed |
| KF-15 | fixed, `d532a50f7` |
| KF-17 | fixed, `2a820f991` |

## The run ends

| id | finding | cause, as far as known | reproduce |
|---|---|---|---|
| KF-1 | A `Texture2D`, `VertexBuffer` or `RenderTarget2D` created on a worker thread fails with `GlContext::MakeCurrent failed: BadAccess`, and the main thread's GL context is unusable afterwards. XNA 4.0 allows creating resources on any thread. | Only `GraphicsDevice::Present` and `ContentReader` take the renderer's thread-context lease; a resource constructor calls `EnsureCallingThreadContext`, which makes the context current on the worker while the main thread still holds it. | `--seed=1 --intensity=high` (tick 24), `--only=WorkerThreadResources` |
| KF-1a | Two worker threads creating textures at the same moment also corrupt the heap: in one run of four the process aborted with glibc's `tcache_thread_shutdown(): unaligned tcache chunk detected` as a worker exited; the other three ended as KF-1. A single worker creating a mesh fails as KF-1 too. | Concurrent resource creation reaches renderer state without mutual exclusion (the lease again). | `--seed=17 --intensity=low --only=WorkerThreadResources,DrawMesh --max-ticks=4`, repeated |
| KF-2 | With multisampling on, a back buffer resize while the window is minimized throws `EasyGL: multisample backbuffer is incomplete`; afterwards `SetRenderTarget(nullptr)` cannot rebind the back buffer and `Present` throws `Cannot present while render targets are bound`. XNA treats `PreferMultiSampling` as a preference. | `CreateMsaaBuffers` is sized from the drawable, 0x0 while minimized (probable). | `--seed=2 --intensity=nightmare` (tick 9), `--seed=3` (tick 417) |
| KF-3 | `ApplyChanges` that shrinks the back buffer below the current `ScissorRectangle` throws `ArgumentException: The scissor rectangle must fit inside the active render surface`. XNA's `Reset` resets the scissor rectangle. | The stale rectangle is re-applied after the resize. | `--seed=2 --intensity=nightmare` (tick 61) |
| KF-16 | A `SetRenderTarget`, `Clear` or `ApplyChanges` issued from `Update` did nothing: EasyGL held the context only from `BeginDraw` to `EndDraw`, and a loading thread could take it in between. Also the real cause of KF-2 (`glCheckFramebufferStatus` returned 0 with no current context). | Found while fixing KF-2. | `easygl_update_time_gl_work` in CNA |

## Wrong data

| id | finding | cause, as far as known |
|---|---|---|
| KF-4 | **Not a defect.** Data written with a full-level `SetData` through one `Texture2D` handle is not what a copy reads back: copies are copy-on-write by design (REMED-GFX-223), pinned by `UploadThroughOneHandleDoesNotReachAnother`. The copy constructor's comment claimed plain sharing; corrected in CNA `f2752e2a8`. | |
| KF-5 | A multisampled render target resolved while a scissor rectangle is active keeps stale pixels outside the rectangle. | `EasyGLRenderTargetRenderer::ResolveColorEXT` and `EasyGLRenderer::ResolveMsaa` call `glBlitFramebuffer`, which obeys the scissor test, without disabling it (the compiled-flip copy already does). |
| KF-6 | `GetBackBufferData` right after `Clear` returns `(0,0,0,0)` outside some rectangle. | Two causes. EasyGL read the window's drawable 1:1 at back-buffer coordinates, so any window not the back buffer's size (Letterbox bars, a display scale of 2) returned the wrong part -- fixed. The back buffer is still the window's surface, not a separate image as in XNA, and on Wayland that surface can differ from the size CNA reports: a window taller than the output (the compositor never grants it), a frame after minimize/restore or a fullscreen toggle, or a loading thread binding the surface between frames before a resize (Mesa fetches the next buffer then). Those remain; an offscreen back buffer, as the MSAA path has, would remove them. `--seed=1 --intensity=nightmare` (224x3043 back buffer), `--seed=2 --intensity=high --only=WorkerThreadResources,VerifyBackBuffer,ResizeBackBuffer` |
| KF-7 | `DynamicSoundEffectInstance.GetSampleDuration` truncates to whole milliseconds; XNA rounds (`TimeSpan.FromMilliseconds` on .NET Framework). 2236 bytes of stereo at 31184 Hz: XNA 18 ms, CNA 17 ms. | |
| KF-8 | `DynamicSoundEffectInstance.GetSampleSizeInBytes` returns sizes that are not whole sample frames: 83 ms mono at 40622 Hz is 6742 bytes in XNA, 6743 in CNA; 73 ms stereo at 33191 Hz is 9688 in XNA, 9691 in CNA. XNA: `(int)(ms * (rate / 1000f))` frames, plus `frames % channels`, times `BlockAlign`. | |
| KF-17 | `SoundEffect.FromStream` on a corrupted WAVE ends the process with SIGFPE (found after the fixes above, seed 1 nightmare, tick 68). | The "fmt " chunk declared zero channels; SDL_mixer divides by `channels * bits / 8` unchecked. |
| KF-9 | `ContentManager.ReadAsset<T>` on a corrupted XNB throws `std::bad_any_cast`; XNA throws `ContentLoadException`. | The root reader produced a different type and the result is `any_cast` unchecked. |

## Refusals XNA makes and CNA does not

| id | call | XNA |
|---|---|---|
| KF-10 | `GraphicsDevice.Viewport` reaching past the render target or back buffer (not a defect: `Viewport(w/2, 0, w/2+1, h)` fits exactly when `w` is odd) | `ArgumentException` |
| KF-11 | `SpriteBatch.End` after a texture drawn in the batch was disposed (CNA draws it; kept, since a C++ caller may as well destroy the texture) | `ObjectDisposedException` (the `Textures[0]` setter) |
| KF-12 | `SoundEffect` with a sample rate outside 8000–48000 Hz, three channels, an empty or misaligned buffer, or a loop region past the samples; `SoundEffectInstance.Volume`/`Pitch`/`Pan` out of range or NaN; `SoundEffect.MasterVolume` out of range or NaN; `SoundEffect.SpeedOfSound` of 0; `DynamicSoundEffectInstance` at 100 Hz; `SubmitBuffer` of partial frames; `DynamicSoundEffectInstance.IsLooped = true`; `CreateInstance` on a disposed `SoundEffect` | `ArgumentOutOfRangeException`, `ArgumentException`, `InvalidOperationException` or `ObjectDisposedException`, per `SoundEffect.FromBuffer`, the instance setters and `DynamicSoundEffectInstance` |

## Refusals with the wrong exception type

A ported `catch (ArgumentException)` or `catch (Exception)` does not catch these.

| id | call | CNA throws | XNA throws |
|---|---|---|---|
| KF-13 | any `ContentManager` load failure | `ContentLoadException` derived from `std::runtime_error` | `ContentLoadException : Exception` |
| KF-14 | `Game.Components.Add` of a component already in it | `std::invalid_argument` | `ArgumentException` |
| KF-15 | `Game.TargetElapsedTime` ≤ 0, `Game.InactiveSleepTime` < 0 | `std::out_of_range` | `ArgumentOutOfRangeException` |

## Checked and correct

In the same runs, about 5 000 verifications passed without a finding: texture `SetData`/`GetData`
in all 19 HiDef formats by level, rectangle and `startIndex`; render-target clears in every HiDef
render-target format, with MSAA and `PreserveContents`; multiple render targets; cube-map faces;
`Texture3D` boxes; vertex and index buffers at byte offsets; full-screen quads for blending,
viewport, culling, depth, stencil and colour write masks; sprite placement and orientation from
textures and render targets; PNG round trips; valid WAVE and XNB files; `GameComponent`s added to
a running game being initialized once. Decoders refused every corrupted image, WAVE file and XNB
without crashing. Resident memory stayed bounded.
