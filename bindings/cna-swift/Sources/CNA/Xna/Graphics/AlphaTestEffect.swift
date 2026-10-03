// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.AlphaTestEffect` projection.
    ///
    /// It implements `IEffectFog` and `IEffectMatrices` and **not**
    /// `IEffectLights`: an alpha-tested sprite has no lighting model. CNA
    /// refuses the lighting routes on it for the same reason, in its own
    /// words — measured, `f69_lights.c`:
    ///
    /// ```text
    /// alphaTest get_directional_light(0) -> 6
    ///     "The Effect does not implement IEffectLights."
    /// ```
    ///
    /// so the absence is not merely unprojected, it is enforced on both sides.
    open class AlphaTestEffect: Effect, IEffectMatrices, IEffectFog {
        internal var state = StockEffectState()
        private var texture: Texture2D?

        /// `AlphaTestEffect(GraphicsDevice device)`.
        public init(device: GraphicsDevice) throws {
            let deviceHandle = try device.validatedHandle("AlphaTestEffect.init")
            let runtime = device.runtimeState
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.alphaTestEffectCreate(deviceHandle, &handle),
                operation: "cna_alpha_test_effect_create")
            super.init(handle: handle, runtime: runtime, device: device,
                       typeName: "AlphaTestEffect")
        }

        /// `protected AlphaTestEffect(AlphaTestEffect cloneSource)`.
        public init(cloneSource: AlphaTestEffect) throws {
            let source = try cloneSource.validatedHandle(
                "AlphaTestEffect.init(cloneSource:)")
            let runtime = cloneSource.nativeStorage.runtime
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectClone(source, &handle),
                operation: "cna_effect_clone")
            super.init(handle: handle, runtime: runtime,
                       device: cloneSource.GraphicsDevice,
                       typeName: "AlphaTestEffect")
            state = cloneSource.state
            texture = cloneSource.texture
        }

        open override func Clone() throws -> Effect {
            try AlphaTestEffect(cloneSource: self)
        }

        // MARK: - IEffectMatrices

        public var World: Microsoft.Xna.Framework.Matrix {
            get { state.world }
            set { state.world = newValue; state.mark(.world) }
        }

        public var View: Microsoft.Xna.Framework.Matrix {
            get { state.view }
            set { state.view = newValue; state.mark(.view) }
        }

        public var Projection: Microsoft.Xna.Framework.Matrix {
            get { state.projection }
            set { state.projection = newValue; state.mark(.projection) }
        }

        // MARK: - the field-backed state

        public var DiffuseColor: Microsoft.Xna.Framework.Vector3 {
            get { state.diffuseColor }
            set { state.diffuseColor = newValue; state.mark(.diffuseColor) }
        }

        public var Alpha: Float {
            get { state.alpha }
            set { state.alpha = newValue; state.mark(.alpha) }
        }

        public var VertexColorEnabled: Bool {
            get { state.vertexColorEnabled }
            set {
                state.vertexColorEnabled = newValue
                state.mark(.vertexColorEnabled)
            }
        }

        /// `AlphaTestEffect.AlphaFunction`.
        ///
        /// Field-backed and infallible on both sides: the comparison selects a
        /// shader permutation rather than writing a parameter, so XNA's setter
        /// is `stfld` plus a dirty bit. Only `Greater` and `Less` — and their
        /// negations — are meaningful to the stock shader, but the property
        /// accepts the whole enumeration exactly as XNA does.
        public var AlphaFunction: CompareFunction {
            get { state.alphaFunction }
            set { state.alphaFunction = newValue; state.mark(.alphaFunction) }
        }

        public var ReferenceAlpha: Int32 {
            get { state.referenceAlpha }
            set { state.referenceAlpha = newValue; state.mark(.referenceAlpha) }
        }

        // MARK: - the parameter-backed state

        public var Texture: Texture2D? {
            get throws {
                try trackedTexture("cna_alpha_test_effect_get_texture", texture) {
                    nativeStorage.runtime.functions.alphaTestEffectGetTexture($0, $1, $2)
                }
            }
        }

        public func SetTexture(_ value: Texture2D?) throws {
            let raw = try value?.validatedHandle("AlphaTestEffect.Texture") ?? 0
            try assignTexture("cna_alpha_test_effect_set_texture", raw) {
                nativeStorage.runtime.functions.alphaTestEffectSetTexture($0, $1)
            }
            texture = value
        }

        // MARK: - IEffectFog

        public var FogEnabled: Bool {
            get { state.fogEnabled }
            set { state.fogEnabled = newValue; state.mark(.fogEnabled) }
        }

        public var FogStart: Float {
            get { state.fogStart }
            set { state.fogStart = newValue; state.mark(.fogStart) }
        }

        public var FogEnd: Float {
            get { state.fogEnd }
            set { state.fogEnd = newValue; state.mark(.fogEnd) }
        }

        public var FogColor: Microsoft.Xna.Framework.Vector3 {
            get throws {
                try readVector("cna_effect_fog_get_color") {
                    nativeStorage.runtime.functions.effectFogGetColor($0, $1)
                }
            }
        }

        public func SetFogColor(_ value: Microsoft.Xna.Framework.Vector3) throws {
            try writeVector(value, "cna_effect_fog_set_color") {
                nativeStorage.runtime.functions.effectFogSetColor($0, $1)
            }
        }

        open override func OnApply() throws {
            let handle = try validatedHandle("AlphaTestEffect.OnApply")
            let functions = nativeStorage.runtime.functions
            try Microsoft.Xna.Framework.Graphics.flushStockEffectState(
                &state, handle: handle, runtime: nativeStorage.runtime,
                matrices: true, fog: true, lights: false,
                diffuse: { functions.alphaTestEffectSetDiffuseColor(handle, $0) },
                emissive: nil,
                alpha: { functions.alphaTestEffectSetAlpha(handle, $0) },
                vertexColor: { functions.alphaTestEffectSetVertexColorEnabled(handle, $0) },
                texture: nil,
                perPixel: nil,
                alphaFunction: { functions.alphaTestEffectSetAlphaFunction(handle, $0) },
                referenceAlpha: { functions.alphaTestEffectSetReferenceAlpha(handle, $0) },
                weightsPerVertex: nil)
        }
    }
}
