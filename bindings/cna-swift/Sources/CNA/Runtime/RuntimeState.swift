// SPDX-License-Identifier: MIT

import CNAShim
import Foundation

internal enum RuntimeRegistry {
    private static let lock = NSLock()
    private static weak var callbackRuntime: RuntimeState?

    static func enter(_ runtime: RuntimeState) {
        lock.lock()
        callbackRuntime = runtime
        lock.unlock()
    }

    static func leave(_ runtime: RuntimeState) {
        lock.lock()
        if callbackRuntime === runtime { callbackRuntime = nil }
        lock.unlock()
    }

    static func current() throws -> RuntimeState {
        lock.lock()
        defer { lock.unlock() }
        guard let callbackRuntime else { throw CNAError.callbackOutsideGameLifecycle }
        return callbackRuntime
    }

    /// ContentManager is a managed XNA type and may be constructed outside a
    /// game callback. Native typed-loader acceleration is attached only when a
    /// callback runtime is actually present.
    static func currentIfAvailable() -> RuntimeState? {
        lock.lock()
        defer { lock.unlock() }
        return callbackRuntime
    }
}

internal final class RuntimeState {
    let functions: NativeFunctions
    let generation: UInt64
    let owner: OwnerThread
    weak var game: Microsoft.Xna.Framework.Game?

    private(set) var gameHandle: UInt64 = 0
    private(set) var isActive = true
    private(set) var callbackEpoch: UInt64 = 0
    private(set) var isInsideCallback = false
    private var callbackError: Error?
    private var children: [WeakRuntimeChild] = []

    // ------------------------------------------------------------------
    // Device-owned managed state.
    //
    // XNA's `GraphicsDevice` is one long-lived object per device, and its
    // `cachedBlendState`, its two `SamplerStateCollection`s and the values it
    // copies out of a state on assignment all live as long as the device does.
    // `set_BlendState`'s early-out is REFERENCE equality against that field.
    //
    // CNA's device handle cannot carry that: measured with
    // `build-probe/f42b_identity.c`, `cna_game_get_graphics_device` answers a
    // handle that is stable within one lifecycle callback and DIFFERENT in
    // every callback, so it is a per-callback capability token rather than the
    // device's identity. The Swift `GraphicsDevice` facade is built fresh on
    // every access to match, and anything cached on it would be lost.
    //
    // The object that does have the device's lifetime is the game, and this is
    // its runtime state. So the cache lives here and the facade reads and
    // writes through it. That is a divergence in where the storage physically
    // sits, which is not observable, decided by a measurement rather than by
    // convenience.
    var cachedBlendState: Microsoft.Xna.Framework.Graphics.BlendState?
    var cachedDepthStencilState: Microsoft.Xna.Framework.Graphics.DepthStencilState?
    var cachedRasterizerState: Microsoft.Xna.Framework.Graphics.RasterizerState?

    // `set_BlendState` copies these two out of the state it accepts, and
    // `set_DepthStencilState` copies the third. They are separate fields on
    // XNA's device, not reads through the cached state, so a later mutation of
    // a state object would not move them -- which is moot for a bound state
    // but is the shape being reproduced.
    // The device's own GraphicsProfile, read once from the device and held
    // here for the same reason every other device-owned value is: the handle
    // is a per-callback token and a facade cannot hold state across callbacks.
    //
    // XNA sets `_profileCapabilities` when the device is created and every
    // later reader is a field read, which is why `get_GraphicsProfile` is
    // `IL_NO_FAILURE_PATH` and the Swift property does not throw. Reading a
    // fallible native route from an infallible getter would be the mistake
    // `PresentationParameters` is deliberately absent to avoid, so the route
    // is called once, where a failure can still be reported, and the getter
    // reads the answer.
    var cachedGraphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile?
    /// The device's presentation parameters and adapter, read once at the
    /// same moment the profile is, because all three back accessors the CLR
    /// declares infallible.
    var cachedPresentationParameters:
        Microsoft.Xna.Framework.Graphics.PresentationParameters?
    var cachedAdapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?

    // The bound vertex-buffer bindings and index buffer, as MANAGED objects.
    //
    // XNA keeps `currentVertexBuffers` and `_currentIB` as fields and both
    // getters read them, so `GetVertexBuffers()` hands back the very objects
    // that were bound and `Indices` is a bare field read. CNA can say which
    // native handle is in a slot -- `cna_graphics_device_copy_vertex_buffers`
    // and `cna_graphics_device_get_index_buffer` both answer -- but it
    // deliberately publishes no route from a native object back to a handle,
    // and its own header prescribes the remedy: "cache what you bind and answer
    // from the cache". That is what XNA's DeviceResourceManager does, so the
    // cache is a reproduction rather than an invention.
    var cachedVertexBufferBindings:
        [Microsoft.Xna.Framework.Graphics.VertexBufferBinding] = []
    var cachedIndexBuffer: Microsoft.Xna.Framework.Graphics.IndexBuffer?

    /// `currentRenderTargetBindings` and `currentRenderTargetCount`, as one
    /// array: the count is the array's own.
    ///
    /// The same reproduction, for the same reason. CNA answers which native
    /// handle occupies each slot -- `cna_graphics_device_copy_render_targets`
    /// hands back the handle, the array slice and the face -- but publishes no
    /// route from a native object back to a handle, so what was BOUND can only
    /// come from a cache. `GetRenderTargets()` must hand back the objects, so
    /// this is where they live.
    var cachedRenderTargetBindings:
        [Microsoft.Xna.Framework.Graphics.RenderTargetBinding] = []

    var cachedBlendFactor = Microsoft.Xna.Framework.Color.White
    var cachedMultiSampleMask: Int32 = -1
    var cachedReferenceStencil: Int32 = 0

    // XNA re-applies when the flag is set even if the same instance is
    // assigned again. `set_RasterizerState` has NO such flag: an identical
    // instance is always a no-op there, with no escape hatch.
    //
    // **Both flags are permanently false today**, because the two things that
    // set them in XNA -- a device reset and an effect pass ending -- are not
    // projected yet. They are transcribed rather than omitted so that the
    // setters are the algorithm XNA runs and not a simplification of it, but
    // nothing in this binding can currently observe the difference between
    // having a flag and not having one. That is why the mutation aimed at the
    // rasterizer's *absence* of a flag was withdrawn as unfalsifiable rather
    // than left in the harness claiming coverage it does not have: see
    // `docs/foundation-45-device-state-evidence.md`.
    var blendStateDirty = false
    var depthStencilStateDirty = false


    // XNA builds both collections in the GraphicsDevice constructor, with
    // `ProfileCapabilities.MaxSamplers` (16 in both profiles) and
    // `MaxVertexSamplers` (0 under Reach, 4 under HiDef) slots and offsets 0
    // and 0x101. They are created lazily here because `RuntimeState` exists
    // before any device does, and creating them eagerly would claim a lifetime
    // this binding cannot justify.
    private var pixelSamplerStates: Microsoft.Xna.Framework.Graphics.SamplerStateCollection?
    private var vertexSamplerStates: Microsoft.Xna.Framework.Graphics.SamplerStateCollection?
    private var pixelTextures: Microsoft.Xna.Framework.Graphics.TextureCollection?
    private var vertexTextures: Microsoft.Xna.Framework.Graphics.TextureCollection?

    /// How many slots each of the four collections has, **from the device's own
    /// profile**: `MaxSamplers` and `MaxVertexSamplers`, which are 16 and 0
    /// under Reach and 16 and 4 under HiDef.
    ///
    /// **A recorded divergence that Foundation 66 resolved.** Until then all
    /// four were 16 long, because the vertex collections were built before this
    /// binding could ask what profile the device was — the note here said "this
    /// binding has no profile selection", and Foundation 62 made that untrue.
    /// A 16-slot vertex collection accepted fifteen indices XNA raises
    /// `ArgumentOutOfRangeException` for, on a profile that has no vertex
    /// samplers at all, and the bounds check is observable.
    ///
    /// CNA's own limit is unchanged and is the wider of the two: 16 slots on
    /// **both** stages, `CNA_TEXTURE_COLLECTION_MAX_TEXTURES`, with 16 and
    /// above refused (`build-probe/f42_states.c`, `build-probe/f66_slots.c`).
    /// XNA's profile is the narrower rule and is the one that decides.
    func samplerSlotCount(vertex: Bool) -> Int {
        let capabilities = Microsoft.Xna.Framework.Graphics.ProfileCapabilities
            .table(for: cachedGraphicsProfile ?? .Reach)
        return Int(vertex ? capabilities.maxVertexSamplers : capabilities.maxSamplers)
    }

    func samplerStates(
        for device: Microsoft.Xna.Framework.Graphics.GraphicsDevice, vertex: Bool
    ) -> Microsoft.Xna.Framework.Graphics.SamplerStateCollection {
        if vertex {
            if let existing = vertexSamplerStates {
                existing.rebind(to: device)
                return existing
            }
            let created = Microsoft.Xna.Framework.Graphics.SamplerStateCollection(
                device: device, samplerOffset: 0x101, stage: 1,
                count: samplerSlotCount(vertex: true))
            vertexSamplerStates = created
            return created
        }
        if let existing = pixelSamplerStates {
            existing.rebind(to: device)
            return existing
        }
        let created = Microsoft.Xna.Framework.Graphics.SamplerStateCollection(
            device: device, samplerOffset: 0, stage: 0,
            count: samplerSlotCount(vertex: false))
        pixelSamplerStates = created
        return created
    }

    /// `pTextureCollection` / `pVertexTextureCollection`, built the same way
    /// and for the same reasons.
    func textures(
        for device: Microsoft.Xna.Framework.Graphics.GraphicsDevice, vertex: Bool
    ) -> Microsoft.Xna.Framework.Graphics.TextureCollection {
        if vertex {
            if let existing = vertexTextures {
                existing.rebind(to: device)
                return existing
            }
            let created = Microsoft.Xna.Framework.Graphics.TextureCollection(
                device: device, textureOffset: 0x101, stage: 1,
                count: samplerSlotCount(vertex: true))
            vertexTextures = created
            return created
        }
        if let existing = pixelTextures {
            existing.rebind(to: device)
            return existing
        }
        let created = Microsoft.Xna.Framework.Graphics.TextureCollection(
            device: device, textureOffset: 0, stage: 0,
            count: samplerSlotCount(vertex: false))
        pixelTextures = created
        return created
    }

    /// Every texture collection that exists, for the `ResourceInUse` scan and
    /// for dropping a disposed texture out of the managed cache.
    var liveTextureCollections: [Microsoft.Xna.Framework.Graphics.TextureCollection] {
        [pixelTextures, vertexTextures].compactMap { $0 }
    }

    init(functions: NativeFunctions) {
        self.functions = functions
        generation = GenerationSequence.next()
        owner = OwnerThread()
    }

    func installGameHandle(_ handle: UInt64) { gameHandle = handle }

    func validateGeneration(_ expected: UInt64) throws {
        guard isActive, expected == generation else {
            throw CNAError.staleRuntimeGeneration(expected: expected, actual: isActive ? generation : nil)
        }
    }

    func enterCallback(nativeGame: UInt64) throws {
        try owner.validate("Game callback")
        try validateGeneration(generation)
        guard nativeGame == gameHandle else {
            throw CNAError.nativeFailure(operation: "Game callback handle validation", result: 2, message: "native callback supplied another Game handle")
        }
        callbackEpoch &+= 1
        if callbackEpoch == 0 { callbackEpoch = 1 }
        isInsideCallback = true
        RuntimeRegistry.enter(self)
    }

    func leaveCallback() {
        isInsideCallback = false
        RuntimeRegistry.leave(self)
    }

    func validateBorrowed(epoch: UInt64, operation: String) throws {
        try validateGeneration(generation)
        try owner.validate(operation)
        guard isInsideCallback, callbackEpoch == epoch else {
            throw CNAError.callbackOutsideGameLifecycle
        }
    }

    func gameTime(from pointer: UnsafePointer<CNASwift_GameTime>?) throws -> Microsoft.Xna.Framework.GameTime {
        guard let value = pointer?.pointee else {
            throw CNAError.nativeFailure(operation: "GameTime callback", result: 12, message: "native update/draw omitted GameTime")
        }
        return Microsoft.Xna.Framework.GameTime(
            totalTicks: value.total_game_time_ticks,
            elapsedTicks: value.elapsed_game_time_ticks,
            runningSlowly: value.is_running_slowly != 0
        )
    }

    func storeCallbackError(_ error: Error) {
        if callbackError == nil { callbackError = error }
    }

    func clearCallbackError() { callbackError = nil }

    func takeCallbackError() -> Error? {
        defer { callbackError = nil }
        return callbackError
    }

    func register(_ child: RuntimeOwnedChild) {
        children.removeAll { $0.value == nil }
        children.append(WeakRuntimeChild(child))
    }

    func disposeChildren() throws {
        var firstError: Error?
        for weakChild in children.reversed() {
            guard let child = weakChild.value, !child.runtimeObjectIsDisposed else { continue }
            do { try child.disposeFromParent() } catch { if firstError == nil { firstError = error } }
        }
        children.removeAll { $0.value == nil || $0.value?.runtimeObjectIsDisposed == true }
        if let firstError { throw firstError }
    }

    func invalidateAfterNativeShutdown() {
        isActive = false
        gameHandle = 0
        isInsideCallback = false
        releaseBoundResources()
        RuntimeRegistry.leave(self)
    }

    /// Releases every cache that holds a `GraphicsResource` strongly.
    ///
    /// **This closes a retain cycle, and without it the whole runtime leaks.**
    /// A `GraphicsResource` holds its `GraphicsDevice` facade strongly and the
    /// facade holds this object strongly, so a cache here that holds a resource
    /// closes `RuntimeState -> resource -> facade -> RuntimeState`. Foundation
    /// 63 added the first such cache and nothing caught it: binding one vertex
    /// buffer kept the runtime, every handle it tracks and every registered
    /// child alive for the life of the process, past `Game.Dispose`. ASan does
    /// not report a retain cycle and no test asked.
    ///
    /// Doing it here also reproduces XNA rather than merely patching Swift:
    /// `GraphicsDevice.Dispose` releases `currentVertexBuffers`, `_currentIB`
    /// and `currentRenderTargetBindings` along with the device, and after this
    /// point there is no device for anything to be bound to.
    ///
    /// `SamplerStateCollection` is not in the list because it already holds its
    /// device weakly, for this reason, since Foundation 46.
    private func releaseBoundResources() {
        cachedVertexBufferBindings = []
        cachedIndexBuffer = nil
        cachedRenderTargetBindings = []
        pixelTextures = nil
        vertexTextures = nil
        cachedBlendState = nil
        cachedDepthStencilState = nil
        cachedRasterizerState = nil
    }
}
