// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.Effect` projection.
    ///
    /// `open`, not `final`: XNA leaves it derivable and the five stock effects
    /// derive from it. It is a `GraphicsResource`, so `Name`, `Tag`,
    /// `IsDisposed`, `GraphicsDevice`, `Dispose()` and `Disposing` all come
    /// from the base — the eight members declared here are the two
    /// constructors, `Clone`, `OnApply`, `Dispose(Bool)` and the three
    /// collection properties.
    ///
    /// **This is what unblocked the draw family.** Foundation 63 withheld every
    /// draw because CNA answers *"no effect has been applied"* until one is,
    /// which is also XNA's `CannotDrawNoShader`. `build-probe/f67_effect.c`
    /// measures the other half: after `cna_effect_apply` — or
    /// `cna_effect_pass_apply` — the same `cna_graphics_device_draw_primitives`
    /// returns 0.
    ///
    /// `cna_effect_create_empty` is what the header calls "the minimal concrete
    /// adapter for the native abstract Effect base class": no parameters, one
    /// technique, one pass, and a native type name that is literally
    /// `Microsoft.Xna.Framework.Graphics.Effect`. That is the effect a
    /// consumer gets from the byte-array constructor on a host with no shader
    /// compiler, and `IsCompiled` is false there — measured, not assumed.
    open class Effect: GraphicsResource {
        private var parameters: EffectParameterCollection?
        private var techniques: EffectTechniqueCollection?
        private var currentTechnique: EffectTechnique?

        internal init(
            handle: UInt64, runtime: RuntimeState, device: GraphicsDevice?,
            typeName: String
        ) {
            super.init(
                storage: NativeHandleStorage(
                    handle: handle, typeName: typeName, ownership: .owned,
                    runtime: runtime, destroy: runtime.functions.effectDestroy),
                device: device)
            runtime.register(self)
        }

        /// `Effect(GraphicsDevice graphicsDevice, Byte[] effectCode)`.
        ///
        /// `CreateEffectFromCode`'s first two tests are pure managed
        /// arithmetic and are reproduced here:
        ///
        /// ```text
        /// if (effectCode == null || effectCode.Length == 0)
        ///     throw new ArgumentNullException("effectCode", NullNotAllowed);
        /// if (effectCode.Length % 4 != 0)
        ///     throw new ArgumentException(
        ///         Format(ArrayMultipleFour, "effectCode"), "effectCode");
        /// ```
        ///
        /// An **empty** array is a null one, which is the same shape
        /// `Texture2D.CopyData` and `VertexBuffer.CopyData` have.
        ///
        /// Everything after that reads the compiled effect's own header —
        /// `MustUserShaderCode` when the bytes are not a compiled effect, and
        /// `ProfileVertexShaderModel`/`ProfilePixelShaderModel` when the shader
        /// models inside exceed the profile's. Those three are recorded as
        /// **native-owned**: they are conclusions about bytecode this binding
        /// does not parse, and `cna_effect_create_compiled` reaches its own
        /// verdict about the same bytes.
        public convenience init(
            graphicsDevice: GraphicsDevice, effectCode: [UInt8]
        ) throws {
            guard !effectCode.isEmpty else {
                throw CNAArgumentNullException(
                    paramName: "effectCode",
                    message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                        .nullNotAllowedMessage)
            }
            guard effectCode.count % 4 == 0 else {
                throw CNAArgumentException(
                    message: Effect.arrayMultipleFourMessage
                        .replacingOccurrences(of: "{0}", with: "effectCode"),
                    paramName: "effectCode")
            }
            let deviceHandle = try graphicsDevice.validatedHandle("Effect.init")
            let runtime = graphicsDevice.runtimeState
            var handle: UInt64 = 0
            try runtime.functions.check(
                effectCode.withUnsafeBufferPointer {
                    runtime.functions.effectCreateCompiled(
                        deviceHandle, $0.baseAddress, UInt64($0.count), &handle)
                },
                operation: "cna_effect_create_compiled")
            self.init(handle: handle, runtime: runtime, device: graphicsDevice,
                      typeName: "Effect")
        }

        /// `Effect(Effect cloneSource)` — the protected clone constructor every
        /// derived effect calls.
        public convenience init(cloneSource: Effect) throws {
            let source = try cloneSource.validatedHandle("Effect.init(cloneSource:)")
            let runtime = cloneSource.nativeStorage.runtime
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectClone(source, &handle),
                operation: "cna_effect_clone")
            self.init(handle: handle, runtime: runtime,
                      device: cloneSource.GraphicsDevice, typeName: "Effect")
        }

        /// The `Effect` a device can make with no shader bytes at all, which is
        /// what CNA's `create_empty` builds. Not an XNA constructor — XNA has
        /// no parameterless `Effect` — so it is internal, and it exists because
        /// it is the only effect this host can actually produce and therefore
        /// the only one the tests can exercise.
        internal static func empty(graphicsDevice: GraphicsDevice) throws -> Effect {
            let deviceHandle = try graphicsDevice.validatedHandle("Effect.empty")
            let runtime = graphicsDevice.runtimeState
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectCreateEmpty(deviceHandle, &handle),
                operation: "cna_effect_create_empty")
            return Effect(handle: handle, runtime: runtime,
                          device: graphicsDevice, typeName: "Effect")
        }

        /// `Effect.Parameters`.
        ///
        /// Built once and cached, so `effect.Parameters === effect.Parameters`
        /// and `effect.Parameters[0] === effect.Parameters[0]`. XNA's getter
        /// reads a field the constructor filled; CNA hands back a fresh owned
        /// collection view on every call, and two objects for one collection is
        /// what the cache prevents.
        public var Parameters: EffectParameterCollection? {
            if let parameters { return parameters }
            var handle: UInt64 = 0
            let built: EffectParameterCollection
            if let owner = try? validatedHandle("Effect.Parameters"),
               nativeStorage.runtime.functions.effectGetParameters(owner, &handle) == 0 {
                built = EffectParameterCollection(
                    handle: handle, runtime: nativeStorage.runtime)
            } else {
                built = EffectParameterCollection(
                    handle: 0, runtime: nativeStorage.runtime)
            }
            parameters = built
            return built
        }

        /// `Effect.Techniques`.
        public var Techniques: EffectTechniqueCollection? {
            if let techniques { return techniques }
            var handle: UInt64 = 0
            let built: EffectTechniqueCollection
            if let owner = try? validatedHandle("Effect.Techniques"),
               nativeStorage.runtime.functions.effectGetTechniques(owner, &handle) == 0 {
                built = EffectTechniqueCollection(
                    handle: handle, runtime: nativeStorage.runtime, owner: self)
            } else {
                built = EffectTechniqueCollection(
                    handle: 0, runtime: nativeStorage.runtime, owner: self)
            }
            techniques = built
            return built
        }

        /// `Effect.CurrentTechnique`.
        ///
        /// The getter is a field read in XNA and must not throw. The **setter**
        /// is `IL_DIRECT_THROW` — it refuses a technique belonging to another
        /// effect — so it is a throwing writer method rather than a Swift
        /// setter, which is the same shape `GraphicsDevice.Indices` has.
        ///
        /// The value is resolved back through `Techniques` so that
        /// `effect.CurrentTechnique === effect.Techniques[0]` holds, which is
        /// what XNA's shared `List<T>` gives and a second owned view would not.
        public var CurrentTechnique: EffectTechnique? {
            if let currentTechnique { return currentTechnique }
            var handle: UInt64 = 0
            guard let owner = try? validatedHandle("Effect.CurrentTechnique"),
                  nativeStorage.runtime.functions.effectGetCurrentTechnique(
                    owner, &handle) == 0, handle != 0 else { return nil }
            // The handle CNA answers is a fresh view; the object a caller must
            // see is the one `Techniques` already produced. Matching by name is
            // what XNA's own `CurrentTechnique` field identity amounts to for a
            // consumer, and it is the only correspondence this ABI exposes.
            let collection = Techniques
            let name = EffectTechnique(
                handle: handle, runtime: nativeStorage.runtime, owner: self).Name
            let resolved = collection?[name] ?? collection?[Int32(0)]
            currentTechnique = resolved
            return resolved
        }

        /// `set_CurrentTechnique`.
        ///
        /// ```text
        /// Helpers.CheckDisposed(this, pComPtr);
        /// if (value == null) throw new ArgumentNullException("value", NullNotAllowed);
        /// ```
        public func SetCurrentTechnique(_ value: EffectTechnique?) throws {
            let owner = try validatedHandle("Effect.CurrentTechnique")
            guard let value else {
                throw CNAArgumentNullException(
                    paramName: "value",
                    message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                        .nullNotAllowedMessage)
            }
            let technique = try value.box.validated("Effect.CurrentTechnique")
            try nativeStorage.runtime.functions.check(
                nativeStorage.runtime.functions.effectSetCurrentTechnique(
                    owner, technique),
                operation: "cna_effect_set_current_technique")
            currentTechnique = value
        }

        /// `Effect.Clone()`.
        ///
        /// `virtual` in XNA and overridden by every stock effect to return its
        /// own type, so `open` here. The base returns a new `Effect` over
        /// `cna_effect_clone`, which the header says produces "an independent
        /// native clone of the same concrete native type".
        open func Clone() throws -> Effect {
            try Effect(cloneSource: self)
        }

        /// `Effect.OnApply()`.
        ///
        /// `protected virtual` and **empty** in XNA — three bytes of IL, `nop;
        /// ret` — existing solely so a derived effect can push its own state
        /// before a pass applies. Swift has no `protected`, so it is `open`.
        /// It does nothing here for the same reason it does nothing there.
        open func OnApply() throws {}

        /// Applies the effect to its device, which is what makes a draw legal.
        ///
        /// Not an XNA member: XNA applies through `EffectPass.Apply`, and this
        /// is `cna_effect_apply`, the whole-effect route. It is internal so the
        /// public surface stays XNA's, and it exists because it is the route
        /// the draw evidence was measured against.
        ///
        /// It does **not** call `OnApply`. `EffectPass.Apply` does, and putting
        /// it here as well would fire the derived hook twice for one
        /// application — the first draft did exactly that.
        internal func apply() throws {
            try nativeStorage.runtime.functions.check(
                nativeStorage.runtime.functions.effectApply(
                    try validatedHandle("Effect.apply")),
                operation: "cna_effect_apply")
        }

        /// `FrameworkResources.ArrayMultipleFour`, whose `{0}` is the
        /// parameter name — the message names the array and the exception
        /// names it again as `ParamName`.
        internal static let arrayMultipleFourMessage =
            "The array {0} must have a length that is a multiple of four."

        /// `protected override void Dispose(bool)`.
        open override func Dispose(_ disposing: Bool) throws {
            try super.Dispose(disposing)
        }
    }
}
