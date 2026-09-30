// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.BasicEffect` projection.
    ///
    /// `open`, not `final`: the pinned metadata leaves all five stock effects
    /// derivable. It is an `Effect`, so `Name`, `Tag`, `IsDisposed`,
    /// `GraphicsDevice`, `Parameters`, `Techniques`, `CurrentTechnique` and
    /// `Dispose` all come from the base.
    ///
    /// **Nothing in this type reads a shader parameter.** Under CNA 0.21 there
    /// were none to read -- `build-probe/f69_lights.c` measured a native
    /// `BasicEffect` publishing `parameter count -> 0`, where `AlphaTestEffect`
    /// published six and `SkinnedEffect` twelve (SW-40). CNA 0.35 publishes
    /// XNA's 21, and they reach `Parameters` through the base; the state this
    /// type exposes still lives behind the named CNA routes, and the defaults
    /// CNA gives a fresh one are XNA's own:
    ///
    /// ```text
    /// light0 enabled=1 dir=(0 -1 0) diff=(1 1 1) spec=(0 0 0)
    /// lights_get_enabled=0   ambient=(0 0 0)
    /// ```
    ///
    /// which is exactly what the constructor's IL sets — `DirectionalLight0.Enabled
    /// = true`, `SpecularColor = One`, `SpecularPower = 16`, everything else
    /// its field default.
    open class BasicEffect: Effect, IEffectMatrices, IEffectFog, IEffectLights {
        internal var state = StockEffectState()
        private var light0: DirectionalLight!
        private var light1: DirectionalLight!
        private var light2: DirectionalLight!
        /// The `Texture2D` a caller last assigned, kept so `Texture` answers
        /// the same object it was given. CNA hands back a raw texture handle,
        /// and a handle is not an identity.
        private var texture: Texture2D?

        /// `BasicEffect(GraphicsDevice device)`.
        public init(device: GraphicsDevice) throws {
            let deviceHandle = try device.validatedHandle("BasicEffect.init")
            let runtime = device.runtimeState
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.basicEffectCreate(deviceHandle, &handle),
                operation: "cna_basic_effect_create")
            super.init(handle: handle, runtime: runtime, device: device,
                       typeName: "BasicEffect")
            (light0, light1, light2) = try Microsoft.Xna.Framework.Graphics
                .stockEffectLights(handle: handle, runtime: runtime)
        }

        /// `protected BasicEffect(BasicEffect cloneSource)`.
        public init(cloneSource: BasicEffect) throws {
            let source = try cloneSource.validatedHandle("BasicEffect.init(cloneSource:)")
            let runtime = cloneSource.nativeStorage.runtime
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectClone(source, &handle),
                operation: "cna_effect_clone")
            super.init(handle: handle, runtime: runtime,
                       device: cloneSource.GraphicsDevice, typeName: "BasicEffect")
            state = cloneSource.state
            texture = cloneSource.texture
            (light0, light1, light2) = try Microsoft.Xna.Framework.Graphics
                .stockEffectLights(handle: handle, runtime: runtime)
        }

        /// `BasicEffect.Clone()` — `virtual` in `Effect` and overridden here to
        /// return a `BasicEffect`, through the protected clone constructor.
        open override func Clone() throws -> Effect {
            try BasicEffect(cloneSource: self)
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

        // MARK: - the field-backed material state

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

        public var PreferPerPixelLighting: Bool {
            get { state.preferPerPixelLighting }
            set {
                state.preferPerPixelLighting = newValue
                state.mark(.preferPerPixelLighting)
            }
        }

        public var TextureEnabled: Bool {
            get { state.textureEnabled }
            set { state.textureEnabled = newValue; state.mark(.textureEnabled) }
        }

        public var VertexColorEnabled: Bool {
            get { state.vertexColorEnabled }
            set {
                state.vertexColorEnabled = newValue
                state.mark(.vertexColorEnabled)
            }
        }

        // MARK: - the parameter-backed material state
        //
        // `SpecularColor` and `SpecularPower` do NOT go through the dirty
        // mask, and the IL is why: `get_SpecularColor` is
        // `specularColorParam.GetValueVector3()` and its setter is
        // `specularColorParam.SetValue(value)`. Both are pinned
        // `IL_REACHABLE_THROW`, so both reach CNA immediately.

        public var SpecularColor: Microsoft.Xna.Framework.Vector3 {
            get throws {
                try readVector("cna_basic_effect_get_specular_color") {
                    nativeStorage.runtime.functions.basicEffectGetSpecularColor($0, $1)
                }
            }
        }

        public func SetSpecularColor(
            _ value: Microsoft.Xna.Framework.Vector3
        ) throws {
            try writeVector(value, "cna_basic_effect_set_specular_color") {
                nativeStorage.runtime.functions.basicEffectSetSpecularColor($0, $1)
            }
        }

        public var SpecularPower: Float {
            get throws {
                let handle = try validatedHandle("BasicEffect.SpecularPower")
                var value: Float = 0
                try nativeStorage.runtime.functions.check(
                    nativeStorage.runtime.functions.basicEffectGetSpecularPower(
                        handle, &value),
                    operation: "cna_basic_effect_get_specular_power")
                return value
            }
        }

        public func SetSpecularPower(_ value: Float) throws {
            let handle = try validatedHandle("BasicEffect.SpecularPower")
            try nativeStorage.runtime.functions.check(
                nativeStorage.runtime.functions.basicEffectSetSpecularPower(
                    handle, value),
                operation: "cna_basic_effect_set_specular_power")
        }

        public var Texture: Texture2D? {
            get throws {
                try trackedTexture("cna_basic_effect_get_texture", texture) {
                    nativeStorage.runtime.functions.basicEffectGetTexture($0, $1, $2)
                }
            }
        }

        public func SetTexture(_ value: Texture2D?) throws {
            let raw = try value?.validatedHandle("BasicEffect.Texture") ?? 0
            try assignTexture("cna_basic_effect_set_texture", raw) {
                nativeStorage.runtime.functions.basicEffectSetTexture($0, $1)
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

        /// `BasicEffect.LightingEnabled` — a public property here, with a
        /// field-backed setter that cannot fail. The two effects that refuse
        /// to disable lighting do not declare it at all.
        public var LightingEnabled: Bool {
            get { state.lightingEnabled }
            set {
                state.lightingEnabled = newValue
                state.mark(.lightingEnabled)
            }
        }

        /// The `IEffectLights.LightingEnabled` writer.
        ///
        /// Not a second XNA member: it is the same CLR `set_LightingEnabled`
        /// accessor the property's `set` already projects, spelled again
        /// because Swift needs a *method* to witness the interface's throwing
        /// writer requirement — and the requirement throws because
        /// `EnvironmentMapEffect` and `SkinnedEffect` implement it with a
        /// `NotSupportedException`. This one cannot fail, and says so by not
        /// being `throws`.
        public func SetLightingEnabled(_ value: Bool) {
            LightingEnabled = value
        }

        /// `BasicEffect.EnableDefaultLighting()`.
        ///
        /// ```text
        /// this.LightingEnabled = true;
        /// this.AmbientLightColor =
        ///     EffectHelpers.EnableDefaultLighting(light0, light1, light2);
        /// ```
        public func EnableDefaultLighting() throws {
            LightingEnabled = true
            AmbientLightColor = try Microsoft.Xna.Framework.Graphics
                .enableDefaultLighting(light0, light1, light2)
        }

        // MARK: - OnApply

        /// `protected internal override void OnApply()`.
        ///
        /// XNA recomputes its derived values and pushes the dirty ones here.
        /// This pushes the dirty ones, which is the half CNA does not already
        /// own — and it is the only place the field-backed setters' values can
        /// reach the device, because a Swift setter cannot report a refusal.
        open override func OnApply() throws {
            let handle = try validatedHandle("BasicEffect.OnApply")
            let functions = nativeStorage.runtime.functions
            try Microsoft.Xna.Framework.Graphics.flushStockEffectState(
                &state, handle: handle, runtime: nativeStorage.runtime,
                matrices: true, fog: true, lights: true,
                diffuse: { functions.basicEffectSetDiffuseColor(handle, $0) },
                emissive: { functions.basicEffectSetEmissiveColor(handle, $0) },
                alpha: { functions.basicEffectSetAlpha(handle, $0) },
                vertexColor: { functions.basicEffectSetVertexColorEnabled(handle, $0) },
                texture: { functions.basicEffectSetTextureEnabled(handle, $0) },
                perPixel: { functions.basicEffectSetPreferPerPixelLighting(handle, $0) },
                alphaFunction: nil,
                referenceAlpha: nil,
                weightsPerVertex: nil)
        }
    }
}
