// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.EnvironmentMapEffect` projection.
    ///
    /// It implements `IEffectLights` but declares **no public
    /// `LightingEnabled`**, because its lighting cannot be turned off:
    ///
    /// ```text
    /// EnvironmentMapEffect::IEffectLights.get_LightingEnabled: ldc.i4.1; ret
    /// EnvironmentMapEffect::IEffectLights.set_LightingEnabled:
    ///     if (!value) throw new NotSupportedException(
    ///         Format(CantDisableLighting, GetType().Name));
    /// ```
    ///
    /// CNA reaches the same verdict independently, and refuses the same call:
    /// *"This stock effect's lighting cannot be disabled."* — measured,
    /// `f69_lights.c`. The managed message is the one raised, because CNA is
    /// not the authority for what XNA throws.
    ///
    /// It also declares no `SpecularColor`, `SpecularPower`,
    /// `PreferPerPixelLighting` or `VertexColorEnabled`: an environment map
    /// supplies its own highlight through `EnvironmentMapSpecular` and
    /// `FresnelFactor`.
    open class EnvironmentMapEffect: Effect, IEffectMatrices, IEffectFog, IEffectLights {
        internal var state = StockEffectState()
        private var light0: DirectionalLight!
        private var light1: DirectionalLight!
        private var light2: DirectionalLight!
        private var texture: Texture2D?
        private var environmentMap: TextureCube?

        /// `EnvironmentMapEffect(GraphicsDevice device)`.
        public init(device: GraphicsDevice) throws {
            let deviceHandle = try device.validatedHandle("EnvironmentMapEffect.init")
            let runtime = device.runtimeState
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.environmentMapEffectCreate(deviceHandle, &handle),
                operation: "cna_environment_map_effect_create")
            super.init(handle: handle, runtime: runtime, device: device,
                       typeName: "EnvironmentMapEffect")
            (light0, light1, light2) = try Microsoft.Xna.Framework.Graphics
                .stockEffectLights(handle: handle, runtime: runtime)
        }

        /// `protected EnvironmentMapEffect(EnvironmentMapEffect cloneSource)`.
        public init(cloneSource: EnvironmentMapEffect) throws {
            let source = try cloneSource.validatedHandle(
                "EnvironmentMapEffect.init(cloneSource:)")
            let runtime = cloneSource.nativeStorage.runtime
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectClone(source, &handle),
                operation: "cna_effect_clone")
            super.init(handle: handle, runtime: runtime,
                       device: cloneSource.GraphicsDevice,
                       typeName: "EnvironmentMapEffect")
            state = cloneSource.state
            texture = cloneSource.texture
            environmentMap = cloneSource.environmentMap
            (light0, light1, light2) = try Microsoft.Xna.Framework.Graphics
                .stockEffectLights(handle: handle, runtime: runtime)
        }

        open override func Clone() throws -> Effect {
            try EnvironmentMapEffect(cloneSource: self)
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

        public var EmissiveColor: Microsoft.Xna.Framework.Vector3 {
            get { state.emissiveColor }
            set { state.emissiveColor = newValue; state.mark(.emissiveColor) }
        }

        public var Alpha: Float {
            get { state.alpha }
            set { state.alpha = newValue; state.mark(.alpha) }
        }

        // MARK: - the parameter-backed state

        public var Texture: Texture2D? {
            get throws {
                try trackedTexture("cna_environment_map_effect_get_texture", texture) {
                    nativeStorage.runtime.functions.environmentMapEffectGetTexture($0, $1, $2)
                }
            }
        }

        public func SetTexture(_ value: Texture2D?) throws {
            let raw = try value?.validatedHandle("EnvironmentMapEffect.Texture") ?? 0
            try assignTexture("cna_environment_map_effect_set_texture", raw) {
                nativeStorage.runtime.functions.environmentMapEffectSetTexture($0, $1)
            }
            texture = value
        }

        /// `EnvironmentMapEffect.EnvironmentMap` — the only `TextureCube`
        /// property in the stock set, and pinned fallible through
        /// `EffectParameter.GetValueTextureCube`.
        public var EnvironmentMap: TextureCube? {
            get throws {
                try trackedTexture("cna_environment_map_effect_get_environment_map", environmentMap) {
                    nativeStorage.runtime.functions.environmentMapEffectGetEnvironmentMap($0, $1, $2)
                }
            }
        }

        public func SetEnvironmentMap(_ value: TextureCube?) throws {
            let raw = try value?.validatedHandle("EnvironmentMapEffect.EnvironmentMap") ?? 0
            try assignTexture("cna_environment_map_effect_set_environment_map", raw) {
                nativeStorage.runtime.functions.environmentMapEffectSetEnvironmentMap($0, $1)
            }
            environmentMap = value
        }

        public var EnvironmentMapAmount: Float {
            get throws {
                try readSingle("cna_environment_map_effect_get_amount") {
                    nativeStorage.runtime.functions.environmentMapEffectGetAmount($0, $1)
                }
            }
        }

        public func SetEnvironmentMapAmount(_ value: Float) throws {
            try writeSingle(value, "cna_environment_map_effect_set_amount") {
                nativeStorage.runtime.functions.environmentMapEffectSetAmount($0, $1)
            }
        }

        public var EnvironmentMapSpecular: Microsoft.Xna.Framework.Vector3 {
            get throws {
                try readVector("cna_environment_map_effect_get_specular") {
                    nativeStorage.runtime.functions.environmentMapEffectGetSpecular($0, $1)
                }
            }
        }

        public func SetEnvironmentMapSpecular(
            _ value: Microsoft.Xna.Framework.Vector3
        ) throws {
            try writeVector(value, "cna_environment_map_effect_set_specular") {
                nativeStorage.runtime.functions.environmentMapEffectSetSpecular($0, $1)
            }
        }

        public var FresnelFactor: Float {
            get throws {
                try readSingle("cna_environment_map_effect_get_fresnel_factor") {
                    nativeStorage.runtime.functions.environmentMapEffectGetFresnelFactor($0, $1)
                }
            }
        }

        public func SetFresnelFactor(_ value: Float) throws {
            try writeSingle(value, "cna_environment_map_effect_set_fresnel_factor") {
                nativeStorage.runtime.functions.environmentMapEffectSetFresnelFactor($0, $1)
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

        // MARK: - IEffectLights

        public var DirectionalLight0: DirectionalLight? { light0 }
        public var DirectionalLight1: DirectionalLight? { light1 }
        public var DirectionalLight2: DirectionalLight? { light2 }

        public var AmbientLightColor: Microsoft.Xna.Framework.Vector3 {
            get { state.ambientLightColor }
            set {
                state.ambientLightColor = newValue
                state.mark(.ambientLightColor)
            }
        }

        /// `IEffectLights.LightingEnabled`, implemented explicitly in the IL.
        ///
        /// Two instructions: `ldc.i4.1; ret`. It is not a stored value and
        /// never becomes one.
        public var LightingEnabled: Bool { true }

        /// `IEffectLights.set_LightingEnabled`.
        ///
        /// `true` returns silently — the whole body is one `brtrue` past the
        /// throw. `false` is refused with the effect's own type name formatted
        /// into `CantDisableLighting`.
        public func SetLightingEnabled(_ value: Bool) throws {
            guard !value else { return }
            throw CNANotSupportedException(
                message: Microsoft.Xna.Framework.Graphics
                    .cantDisableLightingMessage
                    .replacingOccurrences(of: "{0}", with: "EnvironmentMapEffect"))
        }

        /// `EnvironmentMapEffect.EnableDefaultLighting()`.
        ///
        /// It does **not** assign `LightingEnabled` first, because there is
        /// nothing to assign: this effect's lighting is already on.
        public func EnableDefaultLighting() throws {
            AmbientLightColor = try Microsoft.Xna.Framework.Graphics
                .enableDefaultLighting(light0, light1, light2)
        }

        open override func OnApply() throws {
            let handle = try validatedHandle("EnvironmentMapEffect.OnApply")
            let functions = nativeStorage.runtime.functions
            try Microsoft.Xna.Framework.Graphics.flushStockEffectState(
                &state, handle: handle, runtime: nativeStorage.runtime,
                matrices: true, fog: true, lights: true,
                diffuse: { functions.environmentMapEffectSetDiffuseColor(handle, $0) },
                emissive: { functions.environmentMapEffectSetEmissiveColor(handle, $0) },
                alpha: { functions.environmentMapEffectSetAlpha(handle, $0) },
                vertexColor: nil,
                texture: nil,
                perPixel: nil,
                alphaFunction: nil,
                referenceAlpha: nil,
                weightsPerVertex: nil)
        }
    }
}
