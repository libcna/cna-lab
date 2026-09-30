# CNA findings

What cna-killer found in CNA (`../cna`, branch `next` at `8dc7a7b99`, renderer OPENGLES3/EasyGL,
SDL3, private Weston + Xwayland on an AMD GPU, `SDL_AUDIO_DRIVER=dummy`), from seeds 1–5 at every
intensity, 2026-09-30. Every XNA rule cited was read from the XNA 4.0 assemblies in
`../xna4-decomp`. Status of all rows: **open**.

## The run ends

| id | finding | cause, as far as known | reproduce |
|---|---|---|---|
| KF-1 | A `Texture2D`, `VertexBuffer` or `RenderTarget2D` created on a worker thread fails with `GlContext::MakeCurrent failed: BadAccess`, and the main thread's GL context is unusable afterwards. XNA 4.0 allows creating resources on any thread. | Only `GraphicsDevice::Present` and `ContentReader` take the renderer's thread-context lease; a resource constructor calls `EnsureCallingThreadContext`, which makes the context current on the worker while the main thread still holds it. | `--seed=1 --intensity=high` (tick 24), `--only=WorkerThreadResources` |
| KF-2 | With multisampling on, a back buffer resize while the window is minimized throws `EasyGL: multisample backbuffer is incomplete`; afterwards `SetRenderTarget(nullptr)` cannot rebind the back buffer and `Present` throws `Cannot present while render targets are bound`. XNA treats `PreferMultiSampling` as a preference. | `CreateMsaaBuffers` is sized from the drawable, 0x0 while minimized (probable). | `--seed=2 --intensity=nightmare` (tick 9), `--seed=3` (tick 417) |
| KF-3 | `ApplyChanges` that shrinks the back buffer below the current `ScissorRectangle` throws `ArgumentException: The scissor rectangle must fit inside the active render surface`. XNA's `Reset` resets the scissor rectangle. | The stale rectangle is re-applied after the resize. | `--seed=2 --intensity=nightmare` (tick 61) |

## Wrong data

| id | finding | cause, as far as known |
|---|---|---|
| KF-4 | Data written with `SetData` through one `Texture2D` handle is not what `GetData` returns through a copy of it, although a copy "shares the underlying texture resource". | Each copy holds its own `cpuPixels_` pointer; `SetData` replaces the writer's pointer instead of updating the shared cache. |
| KF-5 | A multisampled render target resolved while a scissor rectangle is active keeps stale pixels outside the rectangle. | `EasyGLRenderTargetRenderer::ResolveColorEXT` and `EasyGLRenderer::ResolveMsaa` call `glBlitFramebuffer`, which obeys the scissor test, without disabling it (the compiled-flip copy already does). |
| KF-6 | `GetBackBufferData` right after `Clear` returns `(0,0,0,0)` outside some rectangle. | Probably KF-5: with a multisampled back buffer the read first resolves through `ResolveMsaa`. Recheck after KF-5. |
| KF-7 | `DynamicSoundEffectInstance.GetSampleDuration` truncates to whole milliseconds; XNA rounds (`TimeSpan.FromMilliseconds` on .NET Framework). 2236 bytes of stereo at 31184 Hz: XNA 18 ms, CNA 17 ms. | |
| KF-8 | `DynamicSoundEffectInstance.GetSampleSizeInBytes` returns sizes that are not whole sample frames: 83 ms mono at 40622 Hz is 6742 bytes in XNA, 6743 in CNA; 73 ms stereo at 33191 Hz is 9688 in XNA, 9691 in CNA. XNA: `(int)(ms * (rate / 1000f))` frames, plus `frames % channels`, times `BlockAlign`. | |
| KF-9 | `ContentManager.ReadAsset<T>` on a corrupted XNB throws `std::bad_any_cast`; XNA throws `ContentLoadException`. | The root reader produced a different type and the result is `any_cast` unchecked. |

## Refusals XNA makes and CNA does not

| id | call | XNA |
|---|---|---|
| KF-10 | `GraphicsDevice.Viewport` reaching past the render target or back buffer | `ArgumentException` |
| KF-11 | `SpriteBatch.End` after a texture drawn in the batch was disposed | `ObjectDisposedException` (the `Textures[0]` setter) |
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
