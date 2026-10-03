// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.DualTextureEffect` projection.
    ///
    /// `AlphaTestEffect`'s shape with the alpha test replaced by a second
    /// texture, and the same two interfaces: no lighting, so no
    /// `IEffectLights`. Its two textures are the only members that separate it
    /// from `AlphaTestEffect`, and both are parameter-backed and fallible.
    open class DualTextureEffect: Effect, IEffectMatrices, IEffectFog {
        internal var state = StockEffectState()
        private var texture: Texture2D?
        private var texture2: Texture2D?

        /// `DualTextureEffect(GraphicsDevice device)`.
        public init(device: GraphicsDevice) throws {
            let deviceHandle = try device.validatedHandle("DualTextureEffect.init")
            let runtime = device.runtimeState
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.dualTextureEffectCreate(deviceHandle, &handle),
                operation: "cna_dual_texture_effect_create")
            super.init(handle: handle, runtime: runtime, device: device,
                       typeName: "DualTextureEffect")
        }

        /// `protected DualTextureEffect(DualTextureEffect cloneSource)`.
        public init(cloneSource: DualTextureEffect) throws {
            let source = try cloneSource.validatedHandle(
                "DualTextureEffect.init(cloneSource:)")
            let runtime = cloneSource.nativeStorage.runtime
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectClone(source, &handle),
                operation: "cna_effect_clone")
            super.init(handle: handle, runtime: runtime,
                       device: cloneSource.GraphicsDevice,
                       typeName: "DualTextureEffect")
            state = cloneSource.state
            texture = cloneSource.texture
            texture2 = cloneSource.texture2
        }

        open override func Clone() throws -> Effect {
            try DualTextureEffect(cloneSource: self)
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

        // MARK: - the two parameter-backed textures
        //
        // XNA declares two properties; CNA publishes ONE route pair with a
        // `texture_index` of zero or one. Both spellings name the same two
        // layers, so `Texture` is layer zero and `Texture2` is layer one, and
        // the index is a named constant rather than a literal at four call
        // sites — a swapped index would compile, run, and quietly write the
        // other layer.

        private static let firstLayer: UInt32 = 0
        private static let secondLayer: UInt32 = 1

        public var Texture: Texture2D? {
            get throws { try layer(DualTextureEffect.firstLayer, texture, "Texture") }
        }

        public func SetTexture(_ value: Texture2D?) throws {
            try setLayer(DualTextureEffect.firstLayer, value, "Texture")
            texture = value
        }

        public var Texture2: Texture2D? {
            get throws { try layer(DualTextureEffect.secondLayer, texture2, "Texture2") }
        }

        public func SetTexture2(_ value: Texture2D?) throws {
            try setLayer(DualTextureEffect.secondLayer, value, "Texture2")
            texture2 = value
        }

        private func layer(
            _ index: UInt32, _ tracked: Texture2D?, _ name: String
        ) throws -> Texture2D? {
            try trackedTexture("cna_dual_texture_effect_get_texture", tracked) {
                nativeStorage.runtime.functions.dualTextureEffectGetTexture(
                    $0, index, $1, $2)
            }
        }

        private func setLayer(
            _ index: UInt32, _ value: Texture2D?, _ name: String
        ) throws {
            let raw = try value?.validatedHandle("DualTextureEffect.\(name)") ?? 0
            try assignTexture("cna_dual_texture_effect_set_texture", raw) {
                nativeStorage.runtime.functions.dualTextureEffectSetTexture(
                    $0, index, $1)
            }
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
            let handle = try validatedHandle("DualTextureEffect.OnApply")
            let functions = nativeStorage.runtime.functions
            try Microsoft.Xna.Framework.Graphics.flushStockEffectState(
                &state, handle: handle, runtime: nativeStorage.runtime,
                matrices: true, fog: true, lights: false,
                diffuse: { functions.dualTextureEffectSetDiffuseColor(handle, $0) },
                emissive: nil,
                alpha: { functions.dualTextureEffectSetAlpha(handle, $0) },
                vertexColor: { functions.dualTextureEffectSetVertexColorEnabled(handle, $0) },
                texture: nil,
                perPixel: nil,
                alphaFunction: nil,
                referenceAlpha: nil,
                weightsPerVertex: nil)
        }
    }
}
