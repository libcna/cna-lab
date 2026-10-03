// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.SkinnedEffect` projection.
    ///
    /// The only stock effect with a public field — `MaxBones = 72` — and the
    /// only one whose state includes an array. Like `EnvironmentMapEffect` it
    /// implements `IEffectLights.LightingEnabled` explicitly and refuses to
    /// turn lighting off, with the same two IL bodies.
    ///
    /// It declares no `VertexColorEnabled`. CNA publishes
    /// `cna_skinned_effect_{get,set}_vertex_color_enabled` and the header calls
    /// it "the CNA extension that enables per-vertex color on SkinnedEffect" —
    /// an extension, by its own description, so the pair has no consuming
    /// member here and is not bound.
    open class SkinnedEffect: Effect, IEffectMatrices, IEffectFog, IEffectLights {
        /// `public const int MaxBones = 72`.
        ///
        /// CNA enforces the identical bound from its own side: a
        /// `set_bone_transforms` of 73 matrices is refused with *"The
        /// SkinnedEffect bone-transform array must contain one through 72
        /// matrices."*
        public static let MaxBones: Int32 = 72

        internal var state = StockEffectState()
        private var light0: DirectionalLight!
        private var light1: DirectionalLight!
        private var light2: DirectionalLight!
        private var texture: Texture2D?

        /// `SkinnedEffect(GraphicsDevice device)`.
        public init(device: GraphicsDevice) throws {
            let deviceHandle = try device.validatedHandle("SkinnedEffect.init")
            let runtime = device.runtimeState
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.skinnedEffectCreate(deviceHandle, &handle),
                operation: "cna_skinned_effect_create")
            super.init(handle: handle, runtime: runtime, device: device,
                       typeName: "SkinnedEffect")
            (light0, light1, light2) = try Microsoft.Xna.Framework.Graphics
                .stockEffectLights(handle: handle, runtime: runtime)
        }

        /// `protected SkinnedEffect(SkinnedEffect cloneSource)`.
        public init(cloneSource: SkinnedEffect) throws {
            let source = try cloneSource.validatedHandle(
                "SkinnedEffect.init(cloneSource:)")
            let runtime = cloneSource.nativeStorage.runtime
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectClone(source, &handle),
                operation: "cna_effect_clone")
            super.init(handle: handle, runtime: runtime,
                       device: cloneSource.GraphicsDevice,
                       typeName: "SkinnedEffect")
            state = cloneSource.state
            texture = cloneSource.texture
            (light0, light1, light2) = try Microsoft.Xna.Framework.Graphics
                .stockEffectLights(handle: handle, runtime: runtime)
        }

        open override func Clone() throws -> Effect {
            try SkinnedEffect(cloneSource: self)
        }

        // MARK: - the bone transforms

        /// `SkinnedEffect.SetBoneTransforms(Matrix[] boneTransforms)`.
        ///
        /// ```text
        /// if (boneTransforms == null || boneTransforms.Length == 0)
        ///     throw new ArgumentNullException("boneTransforms", NullNotAllowed);
        /// if (boneTransforms.Length > 72)
        ///     throw new ArgumentException(Format(SkinnedEffectMaxBones, 72));
        /// bonesParam.SetValue(boneTransforms);
        /// ```
        ///
        /// The second exception carries **no `ParamName`** — it is
        /// `ArgumentException(string)`, one argument — while the first names
        /// the parameter. Two adjacent tests on the same argument, reported
        /// two different ways, and reproducing one shape for both would be
        /// wrong in one of them.
        public func SetBoneTransforms(
            _ boneTransforms: [Microsoft.Xna.Framework.Matrix]
        ) throws {
            guard !boneTransforms.isEmpty else {
                throw CNAArgumentNullException(
                    paramName: "boneTransforms",
                    message: Microsoft.Xna.Framework.Graphics.GraphicsDevice
                        .nullNotAllowedMessage)
            }
            guard boneTransforms.count <= Int(SkinnedEffect.MaxBones) else {
                throw CNAArgumentException(
                    message: Microsoft.Xna.Framework.Graphics
                        .skinnedEffectMaxBonesMessage
                        .replacingOccurrences(
                            of: "{0}", with: String(SkinnedEffect.MaxBones)))
            }
            let handle = try validatedHandle("SkinnedEffect.SetBoneTransforms")
            let native = boneTransforms.map {
                Microsoft.Xna.Framework.Graphics.nativeMatrix(from: $0)
            }
            try native.withUnsafeBufferPointer { buffer in
                try nativeStorage.runtime.functions.check(
                    nativeStorage.runtime.functions.skinnedEffectSetBoneTransforms(
                        handle, buffer.baseAddress, UInt64(buffer.count)),
                    operation: "cna_skinned_effect_set_bone_transforms")
            }
        }

        /// `SkinnedEffect.GetBoneTransforms(int count)`.
        ///
        /// ```text
        /// if (count <= 0)  throw new ArgumentOutOfRangeException("count");
        /// if (count > 72)  throw new ArgumentOutOfRangeException(
        ///                      "count", Format(SkinnedEffectMaxBones, 72));
        /// ... foreach (ref m in result) m.M44 = 1;
        /// ```
        ///
        /// Both refusals are `ArgumentOutOfRangeException("count")` and only
        /// the second carries a message — the first is the bare one-argument
        /// constructor.
        ///
        /// **`M44` is restored to 1 on every returned matrix.** XNA stores a
        /// bone as three rows, so the fourth column comes back from the
        /// parameter as whatever the packed form left there; the loop at
        /// `IL_0051` writes the one that makes it an affine transform again.
        ///
        /// **No test can observe that loop here, and the reason is measured.**
        /// `build-probe/f69_bones.c` writes a bone with `m44 = 0` and a
        /// fourth column of `(7, 8, 9)`, and CNA answers `m44 = 1` with the
        /// column intact — it packs the bone the same way XNA does and
        /// restores the same diagonal on the way out. So the mutation that
        /// deletes this line cannot be made to fail, and it was withdrawn
        /// rather than scored. The line stays because it is what the IL does
        /// and because a CNA that stopped normalising would need it.
        public func GetBoneTransforms(
            _ count: Int32
        ) throws -> [Microsoft.Xna.Framework.Matrix] {
            guard count > 0 else {
                throw CNAArgumentOutOfRangeException(paramName: "count")
            }
            guard count <= SkinnedEffect.MaxBones else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "count",
                    message: Microsoft.Xna.Framework.Graphics
                        .skinnedEffectMaxBonesMessage
                        .replacingOccurrences(
                            of: "{0}", with: String(SkinnedEffect.MaxBones)))
            }
            let handle = try validatedHandle("SkinnedEffect.GetBoneTransforms")
            var buffer = [CNASwift_Matrix](
                repeating: CNASwift_Matrix(), count: Int(count))
            var written: UInt64 = 0
            try buffer.withUnsafeMutableBufferPointer { destination in
                try nativeStorage.runtime.functions.check(
                    nativeStorage.runtime.functions.skinnedEffectCopyBoneTransforms(
                        handle, UInt64(count), destination.baseAddress,
                        UInt64(destination.count), &written),
                    operation: "cna_skinned_effect_copy_bone_transforms")
            }
            return buffer.prefix(Int(written)).map {
                var matrix = Microsoft.Xna.Framework.Graphics.matrix(from: $0)
                matrix.M44 = 1
                return matrix
            }
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

        public var PreferPerPixelLighting: Bool {
            get { state.preferPerPixelLighting }
            set {
                state.preferPerPixelLighting = newValue
                state.mark(.preferPerPixelLighting)
            }
        }

        /// `SkinnedEffect.WeightsPerVertex`.
        ///
        /// The getter is `ldfld`, so infallible; the setter is the project's
        /// first `IL_DIRECT_THROW` accessor in this milestone — three `beq`
        /// tests and then the throw, before any field is written:
        ///
        /// ```text
        /// if (value != 1 && value != 2 && value != 4)
        ///     throw new ArgumentOutOfRangeException(
        ///         "value", SkinnedEffectWeightsPerVertex);
        /// ```
        ///
        /// The parameter name really is `"value"` — the compiler-generated
        /// name of a setter's argument — not the property's name.
        public var WeightsPerVertex: Int32 { state.weightsPerVertex }

        public func SetWeightsPerVertex(_ value: Int32) throws {
            guard value == 1 || value == 2 || value == 4 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "value",
                    message: SkinnedEffect.weightsPerVertexMessage)
            }
            state.weightsPerVertex = value
            state.mark(.weightsPerVertex)
        }

        /// `FrameworkResources.SkinnedEffectWeightsPerVertex`.
        internal static let weightsPerVertexMessage =
            "SkinnedEffect.WeightsPerVertex must be 1, 2, or 4."

        // MARK: - the parameter-backed state

        public var SpecularColor: Microsoft.Xna.Framework.Vector3 {
            get throws {
                try readVector("cna_skinned_effect_get_specular_color") {
                    nativeStorage.runtime.functions.skinnedEffectGetSpecularColor($0, $1)
                }
            }
        }

        public func SetSpecularColor(
            _ value: Microsoft.Xna.Framework.Vector3
        ) throws {
            try writeVector(value, "cna_skinned_effect_set_specular_color") {
                nativeStorage.runtime.functions.skinnedEffectSetSpecularColor($0, $1)
            }
        }

        public var SpecularPower: Float {
            get throws {
                try readSingle("cna_skinned_effect_get_specular_power") {
                    nativeStorage.runtime.functions.skinnedEffectGetSpecularPower($0, $1)
                }
            }
        }

        public func SetSpecularPower(_ value: Float) throws {
            try writeSingle(value, "cna_skinned_effect_set_specular_power") {
                nativeStorage.runtime.functions.skinnedEffectSetSpecularPower($0, $1)
            }
        }

        public var Texture: Texture2D? {
            get throws {
                try trackedTexture("cna_skinned_effect_get_texture", texture) {
                    nativeStorage.runtime.functions.skinnedEffectGetTexture($0, $1, $2)
                }
            }
        }

        public func SetTexture(_ value: Texture2D?) throws {
            let raw = try value?.validatedHandle("SkinnedEffect.Texture") ?? 0
            try assignTexture("cna_skinned_effect_set_texture", raw) {
                nativeStorage.runtime.functions.skinnedEffectSetTexture($0, $1)
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

        /// `IEffectLights.LightingEnabled` — `ldc.i4.1; ret`, exactly as
        /// `EnvironmentMapEffect`'s.
        public var LightingEnabled: Bool { true }

        /// `IEffectLights.set_LightingEnabled` — the same body as
        /// `EnvironmentMapEffect`'s, formatting **this** type's name.
        public func SetLightingEnabled(_ value: Bool) throws {
            guard !value else { return }
            throw CNANotSupportedException(
                message: Microsoft.Xna.Framework.Graphics
                    .cantDisableLightingMessage
                    .replacingOccurrences(of: "{0}", with: "SkinnedEffect"))
        }

        public func EnableDefaultLighting() throws {
            AmbientLightColor = try Microsoft.Xna.Framework.Graphics
                .enableDefaultLighting(light0, light1, light2)
        }

        open override func OnApply() throws {
            let handle = try validatedHandle("SkinnedEffect.OnApply")
            let functions = nativeStorage.runtime.functions
            try Microsoft.Xna.Framework.Graphics.flushStockEffectState(
                &state, handle: handle, runtime: nativeStorage.runtime,
                matrices: true, fog: true, lights: true,
                diffuse: { functions.skinnedEffectSetDiffuseColor(handle, $0) },
                emissive: { functions.skinnedEffectSetEmissiveColor(handle, $0) },
                alpha: { functions.skinnedEffectSetAlpha(handle, $0) },
                vertexColor: nil,
                texture: nil,
                perPixel: { functions.skinnedEffectSetPreferPerPixelLighting(handle, $0) },
                alphaFunction: nil,
                referenceAlpha: nil,
                weightsPerVertex: { functions.skinnedEffectSetWeightsPerVertex(handle, $0) })
        }
    }
}
