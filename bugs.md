# CNA findings and game-side workarounds

## Existing EasyGL vertex-buffer uploads during Update have no current context

Observed with CNA `next` at `1ca684199f9bbf56c522a4f0d4e13446d4bae7e1`, sharp-runtime `next` at `9e58c955d0449c50f5f4124c3e1fdd7b821125cf`, Linux desktop `OPENGLES3`, Mesa 25.0.7.

**Symptom:** after several chunk crossings, some recycled buffers still render their previous chunk data. Floors and ceilings disappear into the clear color and wallpaper appears on incorrect geometry. World generation and collision remain deterministic, so a fresh launch at the same position looks correct.

**Reproduction:** seed 12345, Level 0; fully load the neighborhoods centered at chunks `(0,0)`, `(-1,0)`, `(-2,0)`, then `(-2,1)` with the normal buffer pool. At position `(-77.5,75.5)`, looking toward positive Z, the western part of the room is corrupted. A normal 320 m controller route found it. Disabling buffer reuse removes it. Switching to `DynamicVertexBuffer` with `SetDataOptions::Discard` does not remove it.

**Cause:** `GraphicsDeviceManager::EndDraw` releases its frame context lease. `EasyGLRenderer::CreateVertexBuffer` calls `EnsureCallingThreadContext`, but `EasyGLVertexBufferRenderer::SetData` and `SetDataWithOptions` directly issue GL calls. `VertexBuffer` does not wrap those existing-resource writes in a context lease. A reused buffer written in `Update` therefore receives no GPU update when no context is current. New allocations happen to bind the context and can hide the problem. GPU read mapping in the failing upload sequence also returns null; a scoped renderer context makes the readback match the generated vertices and restores the exact screenshot.

**Game workaround:** `BackroomsGame::Stream` holds CNA's existing `GetRenderer().AcquireThreadContextLeaseEXT()` token through buffer retirement and uploads. Level transitions and final GPU resource cleanup use the same RAII scope. The bounded 48-buffer pool remains enabled. The game continues to use ordinary CNA vertex buffers and BasicEffect; no direct GL calls are used by the workaround.

**Engine follow-up:** have existing vertex/index-buffer operations acquire a device context lease, including their resource deletion paths, as framework texture operations already do. Add an EasyGL regression which writes an existing buffer between frame leases and then compares its GPU rendering. This repository does not modify CNA or sharp-runtime.
