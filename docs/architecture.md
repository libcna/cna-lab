# CNA-Swift Foundation-1 architecture

## Boundary

```text
strict Microsoft.Xna.Framework Swift facade
    -> internal ownership/runtime layer
    -> private typed dynamic-function table
    -> canonical CNA C ABI 0.7.0
    -> CNA
```

The package never imports a CNA C++ header, links a CNA C++ symbol, or uses
another language binding as a runtime. `CNAShim` contains only reviewed,
fixed-width C declarations needed to make C layouts and callbacks importable by
Swift. The compiler-backed native verifier compares that shim and the Swift
function table independently with canonical CNA C headers.

The strict XNA identity starts at `Microsoft.Xna.Framework`. Runtime helpers,
errors, loader types, native handles, callback pointers, ownership machinery,
and generation state remain outside that namespace and are internal except for
the support errors and BCL-mapping values a caller must be able to catch/use.

## Runtime state and generation

Every successful `Game` construction allocates one monotonically increasing
generation and captures its owner thread. The callback context retains the
`RuntimeState`; the state has only a weak reference back to `Game`. A
`GraphicsDeviceManager` also holds `Game` weakly, avoiding the old
Game-manager retain cycle. Shutdown invalidates the generation and clears the
game handle. A child from Game 1 therefore cannot operate against Game 2.

The native Game is `OWNED`. `GraphicsDevice` is `BORROWED` and valid only for
the current callback epoch. `GraphicsDeviceManager`, `Texture2D`, and
`SpriteBatch` are `OWNED` Game children. Swift value types are `MANAGED_VALUE`.
The loader and its immutable function table are `PROCESS_GLOBAL`.

Owned destruction is transactional: a handle is cleared only after CNA reports
successful destruction. Wrong-thread disposal throws and leaves the handle
available for owner-thread retry. `Game.Dispose` disposes registered children
in reverse order before destroying the Game. Explicit `Dispose` is
authoritative. `deinit` performs only safe best-effort cleanup when the owner
thread and generation are still valid; it never dispatches a knowingly unsafe
destroy from another thread.

## Callbacks and Swift errors

`CallbackContext` uses `Unmanaged.passRetained`, supplies its opaque pointer as
C `context`, and releases it exactly once after CNA can no longer call. Global
`@convention(c)` trampolines recover the state with `takeUnretainedValue`.
Every trampoline catches all Swift errors, stores the first error in runtime
state, returns CNA callback result 9, and returns normally through C. `Run`,
`RunOneFrame`, or `Dispose` rethrows the original Swift error only after native
control returns. CNA can repeat callback result 9 during cleanup after `Run`
already surfaced an error; the Game state records that fact and does not invent
a second error, while a new `UnloadContent` error is still surfaced.

The qualified tests cover errors from Initialize, LoadContent, Update, Draw,
and UnloadContent; 20 callback-error cycles; and Game recreation.

## Platform statement

Only Linux x86-64 is runtime-qualified. The Swift sources currently use a Linux
`dlopen` implementation and intentionally have no Apple platform declarations
in `Package.swift`. Apple, Windows, and Web/Wasm are future/unqualified.
HEADLESS and OPENGLES3 are the qualified renderers. HEADLESS executes the
native device, clear, texture, and SpriteBatch routes with no window; OPENGLES3
adds a real window (on a private display), cube-face storage and back-buffer
readback, which the suite asserts where the renderer provides them.
