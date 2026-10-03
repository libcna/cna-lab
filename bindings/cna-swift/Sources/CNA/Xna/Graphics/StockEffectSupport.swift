// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Graphics {
    /// The cached half of a stock effect, and the rule that decides what is
    /// cached at all.
    ///
    /// XNA's five stock effects are built the same way twice over. A property
    /// is one of exactly two things, and the accessor fallibility inventory
    /// reads the difference straight out of the IL:
    ///
    /// | | reads | writes | verdict |
    /// |---|---|---|---|
    /// | `World`, `Alpha`, `FogStart`, … | `ldfld`, a cached field | `stfld` plus a dirty bit | infallible both ways |
    /// | `SpecularColor`, `FogColor`, `Texture`, … | `param.GetValueX()` | `param.SetValue(…)` | fallible both ways |
    ///
    /// The first kind never touches the shader at set time: the value sits in
    /// a field until `OnApply` pushes the dirty ones. The second kind goes
    /// straight through. **That is why one projects as `{ get set }` and the
    /// other as `{ get throws }` plus a throwing writer** — and it is also why
    /// this type exists, because the first kind cannot write to CNA at set
    /// time either. A Swift setter cannot throw, and every CNA route can fail;
    /// deferring the write to `OnApply`, which *is* allowed to throw, is not a
    /// workaround but the same architecture XNA already has.
    ///
    /// The dirty mask is this projection's own, not XNA's `EffectDirtyFlags`:
    /// XNA's bits select shader permutations and recompute derived values,
    /// which is work CNA does. These bits answer one question — has this value
    /// been assigned since the last apply — so an untouched effect pushes
    /// nothing.
    internal struct StockEffectState {
        struct Dirty: OptionSet {
            let rawValue: UInt32
            static let world = Dirty(rawValue: 1 << 0)
            static let view = Dirty(rawValue: 1 << 1)
            static let projection = Dirty(rawValue: 1 << 2)
            static let diffuseColor = Dirty(rawValue: 1 << 3)
            static let emissiveColor = Dirty(rawValue: 1 << 4)
            static let alpha = Dirty(rawValue: 1 << 5)
            static let ambientLightColor = Dirty(rawValue: 1 << 6)
            static let lightingEnabled = Dirty(rawValue: 1 << 7)
            static let fogEnabled = Dirty(rawValue: 1 << 8)
            static let fogStart = Dirty(rawValue: 1 << 9)
            static let fogEnd = Dirty(rawValue: 1 << 10)
            static let vertexColorEnabled = Dirty(rawValue: 1 << 11)
            static let textureEnabled = Dirty(rawValue: 1 << 12)
            static let preferPerPixelLighting = Dirty(rawValue: 1 << 13)
            static let alphaFunction = Dirty(rawValue: 1 << 14)
            static let referenceAlpha = Dirty(rawValue: 1 << 15)
            static let weightsPerVertex = Dirty(rawValue: 1 << 16)
        }

        var dirty: Dirty = []

        var world = Microsoft.Xna.Framework.Matrix.Identity
        var view = Microsoft.Xna.Framework.Matrix.Identity
        var projection = Microsoft.Xna.Framework.Matrix.Identity
        var diffuseColor = Microsoft.Xna.Framework.Vector3.One
        var emissiveColor = Microsoft.Xna.Framework.Vector3.Zero
        var ambientLightColor = Microsoft.Xna.Framework.Vector3.Zero
        var alpha: Float = 1
        var lightingEnabled = false
        var fogEnabled = false
        var fogStart: Float = 0
        var fogEnd: Float = 1
        var vertexColorEnabled = false
        var textureEnabled = false
        var preferPerPixelLighting = false
        var alphaFunction = CompareFunction.Greater
        var referenceAlpha: Int32 = 0
        var weightsPerVertex: Int32 = 4

        mutating func mark(_ bit: Dirty) { dirty.insert(bit) }
    }

    /// Pushes a stock effect's dirty cached values onto its native effect.
    ///
    /// Called from each effect's `OnApply`, which is where XNA pushes its own
    /// dirty fields. Every route here is one CNA can refuse, and refusing is
    /// reported rather than swallowed — which is only possible because the
    /// write was deferred to a throwing member.
    internal static func flushStockEffectState(
        _ state: inout StockEffectState,
        handle: UInt64,
        runtime: RuntimeState,
        matrices: Bool,
        fog: Bool,
        lights: Bool,
        diffuse: ((CNASwift_Vector3) -> UInt32)?,
        emissive: ((CNASwift_Vector3) -> UInt32)?,
        alpha: ((Float) -> UInt32)?,
        vertexColor: ((UInt8) -> UInt32)?,
        texture: ((UInt8) -> UInt32)?,
        perPixel: ((UInt8) -> UInt32)?,
        alphaFunction: ((UInt32) -> UInt32)?,
        referenceAlpha: ((Int32) -> UInt32)?,
        weightsPerVertex: ((Int32) -> UInt32)?
    ) throws {
        let functions = runtime.functions
        func vector(_ value: Microsoft.Xna.Framework.Vector3) -> CNASwift_Vector3 {
            CNASwift_Vector3(x: value.X, y: value.Y, z: value.Z)
        }
        if matrices {
            if state.dirty.contains(.world) {
                try functions.check(
                    functions.effectMatricesSetWorld(
                        handle, nativeMatrix(from: state.world)),
                    operation: "cna_effect_matrices_set_world")
            }
            if state.dirty.contains(.view) {
                try functions.check(
                    functions.effectMatricesSetView(
                        handle, nativeMatrix(from: state.view)),
                    operation: "cna_effect_matrices_set_view")
            }
            if state.dirty.contains(.projection) {
                try functions.check(
                    functions.effectMatricesSetProjection(
                        handle, nativeMatrix(from: state.projection)),
                    operation: "cna_effect_matrices_set_projection")
            }
        }
        if fog {
            if state.dirty.contains(.fogEnabled) {
                try functions.check(
                    functions.effectFogSetEnabled(handle, state.fogEnabled ? 1 : 0),
                    operation: "cna_effect_fog_set_enabled")
            }
            if state.dirty.contains(.fogStart) {
                try functions.check(
                    functions.effectFogSetStart(handle, state.fogStart),
                    operation: "cna_effect_fog_set_start")
            }
            if state.dirty.contains(.fogEnd) {
                try functions.check(
                    functions.effectFogSetEnd(handle, state.fogEnd),
                    operation: "cna_effect_fog_set_end")
            }
        }
        if lights {
            if state.dirty.contains(.ambientLightColor) {
                try functions.check(
                    functions.effectLightsSetAmbientColor(
                        handle, vector(state.ambientLightColor)),
                    operation: "cna_effect_lights_set_ambient_color")
            }
            if state.dirty.contains(.lightingEnabled) {
                try functions.check(
                    functions.effectLightsSetEnabled(
                        handle, state.lightingEnabled ? 1 : 0),
                    operation: "cna_effect_lights_set_enabled")
            }
        }
        if let diffuse, state.dirty.contains(.diffuseColor) {
            try functions.check(
                diffuse(vector(state.diffuseColor)),
                operation: "the effect's set_diffuse_color")
        }
        if let emissive, state.dirty.contains(.emissiveColor) {
            try functions.check(
                emissive(vector(state.emissiveColor)),
                operation: "the effect's set_emissive_color")
        }
        if let alpha, state.dirty.contains(.alpha) {
            try functions.check(
                alpha(state.alpha),
                operation: "the effect's set_alpha")
        }
        if let vertexColor, state.dirty.contains(.vertexColorEnabled) {
            try functions.check(
                vertexColor(state.vertexColorEnabled ? 1 : 0),
                operation: "the effect's set_vertex_color_enabled")
        }
        if let texture, state.dirty.contains(.textureEnabled) {
            try functions.check(
                texture(state.textureEnabled ? 1 : 0),
                operation: "the effect's set_texture_enabled")
        }
        if let perPixel, state.dirty.contains(.preferPerPixelLighting) {
            try functions.check(
                perPixel(state.preferPerPixelLighting ? 1 : 0),
                operation: "the effect's set_prefer_per_pixel_lighting")
        }
        if let alphaFunction, state.dirty.contains(.alphaFunction) {
            try functions.check(
                alphaFunction(UInt32(state.alphaFunction.rawValue)),
                operation: "cna_alpha_test_effect_set_alpha_function")
        }
        if let referenceAlpha, state.dirty.contains(.referenceAlpha) {
            try functions.check(
                referenceAlpha(state.referenceAlpha),
                operation: "cna_alpha_test_effect_set_reference_alpha")
        }
        if let weightsPerVertex, state.dirty.contains(.weightsPerVertex) {
            try functions.check(
                weightsPerVertex(state.weightsPerVertex),
                operation: "cna_skinned_effect_set_weights_per_vertex")
        }
        state.dirty = []
    }

    /// `EffectHelpers.EnableDefaultLighting`, reproduced rather than forwarded.
    ///
    /// CNA publishes `cna_effect_lights_enable_default`, and it produces these
    /// exact ten vectors — measured in `build-probe/f69_default.c`, every
    /// component matching. It is still not called, and the reason is the
    /// authority rule: the values a caller sees have to come from XNA's IL,
    /// not from a native library that happens to agree today. The agreement is
    /// recorded as native evidence and nothing depends on it.
    ///
    /// Returns the ambient color the caller must assign, which is how XNA
    /// spells it: the helper returns a `Vector3` and each effect stores it
    /// through its own `AmbientLightColor` setter.
    @discardableResult
    internal static func enableDefaultLighting(
        _ light0: DirectionalLight,
        _ light1: DirectionalLight,
        _ light2: DirectionalLight
    ) throws -> Microsoft.Xna.Framework.Vector3 {
        try light0.SetDirection(
            Microsoft.Xna.Framework.Vector3(-0.5265408, -0.5735765, -0.6275069))
        try light0.SetDiffuseColor(
            Microsoft.Xna.Framework.Vector3(1, 0.9607844, 0.8078432))
        try light0.SetSpecularColor(
            Microsoft.Xna.Framework.Vector3(1, 0.9607844, 0.8078432))
        try light0.SetEnabled(true)

        try light1.SetDirection(
            Microsoft.Xna.Framework.Vector3(0.7198464, 0.3420201, 0.6040227))
        try light1.SetDiffuseColor(
            Microsoft.Xna.Framework.Vector3(0.9647059, 0.7607844, 0.4078432))
        try light1.SetSpecularColor(Microsoft.Xna.Framework.Vector3.Zero)
        try light1.SetEnabled(true)

        try light2.SetDirection(
            Microsoft.Xna.Framework.Vector3(0.4545195, -0.7660444, 0.4545195))
        try light2.SetDiffuseColor(
            Microsoft.Xna.Framework.Vector3(0.3231373, 0.3607844, 0.3937255))
        try light2.SetSpecularColor(
            Microsoft.Xna.Framework.Vector3(0.3231373, 0.3607844, 0.3937255))
        try light2.SetEnabled(true)

        return Microsoft.Xna.Framework.Vector3(0.05333332, 0.09882354, 0.1819608)
    }

    /// The three lights a stock effect that implements `IEffectLights` owns.
    ///
    /// Fetched once, because `cna_effect_lights_get_directional_light` hands
    /// back a fresh owned view on every call and `effect.DirectionalLight0 ===
    /// effect.DirectionalLight0` has to hold — XNA keeps three fields.
    internal static func stockEffectLights(
        handle: UInt64, runtime: RuntimeState
    ) throws -> (DirectionalLight, DirectionalLight, DirectionalLight) {
        func light(_ index: UInt32) throws -> DirectionalLight {
            var raw: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectLightsGetDirectionalLight(
                    handle, index, &raw),
                operation: "cna_effect_lights_get_directional_light")
            return try DirectionalLight(
                box: DirectionalLightBox(handle: raw, runtime: runtime))
        }
        return (try light(0), try light(1), try light(2))
    }

    /// `FrameworkResources.CantDisableLighting`, whose `{0}` is the effect's
    /// own type name.
    ///
    /// The two effects that use it are the two that implement
    /// `IEffectLights.LightingEnabled` explicitly, and each formats its own
    /// `GetType().Name` into it.
    internal static let cantDisableLightingMessage =
        "{0} does not support setting LightingEnabled to false."

    /// `FrameworkResources.SkinnedEffectMaxBones`, whose `{0}` is `MaxBones`.
    internal static let skinnedEffectMaxBonesMessage =
        "SkinnedEffect supports a maximum of {0} bones."
}

extension Microsoft.Xna.Framework.Graphics.Effect {
    /// One parameter-backed `Vector3` read, for the accessors XNA spells as
    /// `param.GetValueVector3()`.
    ///
    /// The route is invoked INSIDE a Swift closure rather than passed as a
    /// value. Handing a `@convention(c)` function to a parameter of Swift
    /// function type makes the compiler build a re-abstraction thunk, and
    /// Swift 6.0.3 asserts rather than building one:
    ///
    /// ```text
    /// SILGenPoly.cpp:5539: Assertion `expectedType->getLanguage() ==
    ///     fn.getType()...->getLanguage() && "bridging in re-abstraction thunk?"'
    /// ```
    ///
    /// Calling through the closure keeps the C pointer on one side of the
    /// boundary, which is also the clearer thing to read: the call site names
    /// its route and its operation string together.
    internal func readVector(
        _ operation: String,
        _ call: (UInt64, UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
    ) throws -> Microsoft.Xna.Framework.Vector3 {
        let handle = try validatedHandle(operation)
        var value = CNASwift_Vector3()
        try nativeStorage.runtime.functions.check(
            call(handle, &value), operation: operation)
        return Microsoft.Xna.Framework.Vector3(value.x, value.y, value.z)
    }

    internal func writeVector(
        _ value: Microsoft.Xna.Framework.Vector3,
        _ operation: String,
        _ call: (UInt64, CNASwift_Vector3) -> UInt32
    ) throws {
        let handle = try validatedHandle(operation)
        try nativeStorage.runtime.functions.check(
            call(handle, CNASwift_Vector3(x: value.X, y: value.Y, z: value.Z)),
            operation: operation)
    }

    internal func readSingle(
        _ operation: String,
        _ call: (UInt64, UnsafeMutablePointer<Float>?) -> UInt32
    ) throws -> Float {
        let handle = try validatedHandle(operation)
        var value: Float = 0
        try nativeStorage.runtime.functions.check(
            call(handle, &value), operation: operation)
        return value
    }

    internal func writeSingle(
        _ value: Float,
        _ operation: String,
        _ call: (UInt64, Float) -> UInt32
    ) throws {
        let handle = try validatedHandle(operation)
        try nativeStorage.runtime.functions.check(
            call(handle, value), operation: operation)
    }

    /// The `has`/`handle` pair every stock effect's texture getter answers.
    ///
    /// The tracked Swift object is what comes back, because CNA reports a raw
    /// handle and a handle is not an identity: `effect.Texture === theTexture`
    /// has to hold for the object the caller assigned. The native call is
    /// still made, and is what makes the getter fallible and what reports a
    /// disposed effect.
    internal func trackedTexture<T: AnyObject>(
        _ operation: String, _ tracked: T?,
        _ call: (UInt64, UnsafeMutablePointer<UInt8>?, UnsafeMutablePointer<UInt64>?) -> UInt32
    ) throws -> T? {
        let handle = try validatedHandle(operation)
        var present: UInt8 = 0
        var raw: UInt64 = 0
        try nativeStorage.runtime.functions.check(
            call(handle, &present, &raw), operation: operation)
        return present != 0 ? tracked : nil
    }

    internal func assignTexture(
        _ operation: String, _ raw: UInt64,
        _ call: (UInt64, UInt64) -> UInt32
    ) throws {
        let handle = try validatedHandle(operation)
        try nativeStorage.runtime.functions.check(
            call(handle, raw), operation: operation)
    }
}
