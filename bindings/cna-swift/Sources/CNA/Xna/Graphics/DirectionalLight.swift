// SPDX-License-Identifier: MIT

import CNAShim

/// One owned native `CNA_DirectionalLightHandle`, released exactly once.
///
/// A nested light is a *view* on its effect: `cna_effect_lights_get_directional_light`
/// hands back a fresh owned handle on every call, which is why a light is
/// fetched once and kept. Measured, `build-probe/f69_lights.c`:
///
/// ```text
/// get_directional_light(0) twice -> 0/0  handles 8589934596 4294967301  same=0
/// ```
internal final class DirectionalLightBox {
    internal private(set) var handle: UInt64
    internal let runtime: RuntimeState

    init(handle: UInt64, runtime: RuntimeState) {
        self.handle = handle
        self.runtime = runtime
    }

    func release() {
        guard handle != 0 else { return }
        _ = runtime.functions.directionalLightDestroy(handle)
        handle = 0
    }

    func validated(_ operation: String) throws -> UInt64 {
        guard handle != 0 else {
            throw CNAError.producerInvariant(
                "\(operation) on a released directional light")
        }
        return handle
    }

    deinit { release() }
}

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.DirectionalLight` projection.
    ///
    /// `sealed` in the IL, so `final` here. It is **not** a `GraphicsResource`
    /// and not `IDisposable`: it derives from `System.Object` and holds three
    /// `EffectParameter` references and four cached values.
    ///
    /// ```text
    /// .field private class EffectParameter directionParam
    /// .field private class EffectParameter diffuseColorParam
    /// .field private class EffectParameter specularColorParam
    /// .field private bool enabled
    /// .field private Vector3 cachedDirection
    /// .field private Vector3 cachedDiffuseColor
    /// .field private Vector3 cachedSpecularColor
    /// ```
    ///
    /// **Every getter reads a cache**, never a parameter — all four are
    /// `ldfld; ret` and all four are pinned `IL_NO_FAILURE_PATH`. Every setter
    /// writes a parameter and is pinned fallible through
    /// `EffectParameter.SetValue`. That asymmetry is the whole shape of this
    /// type, and it is why the four properties project as `{ get }` plus a
    /// throwing `Set…`.
    ///
    /// **Every parameter reference is null-tolerant.** Each setter guards with
    /// `brfalse.s` before writing, so a light built with three nulls is a
    /// well-defined value holder that writes nothing. The projection keeps
    /// that, because the public constructor accepts nulls.
    public final class DirectionalLight {
        private let directionParam: EffectParameter?
        private let diffuseColorParam: EffectParameter?
        private let specularColorParam: EffectParameter?
        /// The native light a stock effect's light is a view on. `nil` for one
        /// a caller built through the public constructor: CNA's
        /// `cna_directional_light_create` makes a light with no parameters and
        /// no effect, which is a *different* object from the one XNA's
        /// constructor makes over three caller-supplied parameters, so it is
        /// not substituted for it.
        private let box: DirectionalLightBox?

        private var enabled: Bool = false
        private var cachedDirection = Microsoft.Xna.Framework.Vector3.Zero
        private var cachedDiffuseColor = Microsoft.Xna.Framework.Vector3.Zero
        private var cachedSpecularColor = Microsoft.Xna.Framework.Vector3.Zero

        /// `DirectionalLight(EffectParameter, EffectParameter, EffectParameter,
        /// DirectionalLight)`.
        ///
        /// With a clone source the four cached values are copied **directly**,
        /// field to field, so no parameter is written. Without one the three
        /// setters run, which is not the same thing: `Direction` writes its
        /// parameter, and `DiffuseColor` and `SpecularColor` do not, because
        /// `enabled` is still false.
        ///
        /// ```text
        /// IL_0054: this.Direction      = Vector3.Down
        /// IL_005f: this.DiffuseColor   = Vector3.One
        /// IL_006a: this.SpecularColor  = Vector3.Zero
        /// ```
        public init(
            directionParameter: EffectParameter?,
            diffuseColorParameter: EffectParameter?,
            specularColorParameter: EffectParameter?,
            cloneSource: DirectionalLight?
        ) throws {
            directionParam = directionParameter
            diffuseColorParam = diffuseColorParameter
            specularColorParam = specularColorParameter
            box = nil
            if let cloneSource {
                enabled = cloneSource.enabled
                cachedDirection = cloneSource.cachedDirection
                cachedDiffuseColor = cloneSource.cachedDiffuseColor
                cachedSpecularColor = cloneSource.cachedSpecularColor
                return
            }
            try SetDirection(Microsoft.Xna.Framework.Vector3.Down)
            try SetDiffuseColor(Microsoft.Xna.Framework.Vector3.One)
            try SetSpecularColor(Microsoft.Xna.Framework.Vector3.Zero)
        }

        /// A stock effect's own light, seeded from the native light it views.
        ///
        /// Not an XNA constructor: XNA's stock effects build their lights from
        /// their own `EffectParameter`s, and on this artifact a `BasicEffect`
        /// publishes **no parameters at all** — measured, `f69_lights.c`,
        /// `parameter count -> 0 0`, where the other four publish five to
        /// twelve. So the effect's light is the native one, and the four
        /// cached values are read from it once so the getters stay infallible.
        internal init(box: DirectionalLightBox) throws {
            directionParam = nil
            diffuseColorParam = nil
            specularColorParam = nil
            self.box = box
            let handle = try box.validated("DirectionalLight.init")
            let functions = box.runtime.functions
            var isEnabled: UInt8 = 0
            try functions.check(
                functions.directionalLightGetEnabled(handle, &isEnabled),
                operation: "cna_directional_light_get_enabled")
            enabled = isEnabled != 0
            cachedDirection = try DirectionalLight.readVector(
                box, "cna_directional_light_get_direction") {
                functions.directionalLightGetDirection(handle, $0)
            }
            cachedDiffuseColor = try DirectionalLight.readVector(
                box, "cna_directional_light_get_diffuse_color") {
                functions.directionalLightGetDiffuseColor(handle, $0)
            }
            cachedSpecularColor = try DirectionalLight.readVector(
                box, "cna_directional_light_get_specular_color") {
                functions.directionalLightGetSpecularColor(handle, $0)
            }
        }

        private static func readVector(
            _ box: DirectionalLightBox, _ operation: String,
            _ route: (UnsafeMutablePointer<CNASwift_Vector3>?) -> UInt32
        ) throws -> Microsoft.Xna.Framework.Vector3 {
            var value = CNASwift_Vector3()
            try box.runtime.functions.check(route(&value), operation: operation)
            return Microsoft.Xna.Framework.Vector3(value.x, value.y, value.z)
        }

        /// `DirectionalLight.Enabled` — `ldfld enabled; ret`.
        public var Enabled: Bool { enabled }

        /// `set_Enabled`.
        ///
        /// Three things happen here that a plainer setter would not do:
        ///
        /// ```text
        /// IL_0000: if (this.enabled == value) return;      // a no-op, exactly
        /// IL_0016: if (enabled) { diffuseParam  = cachedDiffuseColor
        ///                         specularParam = cachedSpecularColor }
        /// IL_004b: else        { diffuseParam  = Vector3.Zero
        ///                         specularParam = Vector3.Zero }
        /// ```
        ///
        /// Disabling a light does not clear what the light *remembers* — the
        /// two caches are untouched, so `DiffuseColor` still answers what it
        /// answered before — it clears what the **shader** sees. Re-enabling
        /// pushes the remembered values back. `directionParam` is never
        /// touched by either branch.
        public func SetEnabled(_ value: Bool) throws {
            guard enabled != value else { return }
            enabled = value
            if enabled {
                try diffuseColorParam?.SetValue(cachedDiffuseColor)
                try specularColorParam?.SetValue(cachedSpecularColor)
            } else {
                try diffuseColorParam?.SetValue(
                    Microsoft.Xna.Framework.Vector3.Zero)
                try specularColorParam?.SetValue(
                    Microsoft.Xna.Framework.Vector3.Zero)
            }
            try pushEnabled()
        }

        /// `DirectionalLight.Direction` — `ldfld cachedDirection; ret`.
        public var Direction: Microsoft.Xna.Framework.Vector3 { cachedDirection }

        /// `set_Direction`: writes the parameter **unconditionally**, then
        /// caches. The only one of the three that does not consult `enabled`.
        public func SetDirection(_ value: Microsoft.Xna.Framework.Vector3) throws {
            try directionParam?.SetValue(value)
            cachedDirection = value
            try push(value, "cna_directional_light_set_direction") {
                box?.runtime.functions.directionalLightSetDirection($0, $1) ?? 0
            }
        }

        /// `DirectionalLight.DiffuseColor` — `ldfld cachedDiffuseColor; ret`.
        public var DiffuseColor: Microsoft.Xna.Framework.Vector3 { cachedDiffuseColor }

        /// `set_DiffuseColor`: writes the parameter **only while enabled**,
        /// then caches either way.
        public func SetDiffuseColor(_ value: Microsoft.Xna.Framework.Vector3) throws {
            if enabled { try diffuseColorParam?.SetValue(value) }
            cachedDiffuseColor = value
            try push(value, "cna_directional_light_set_diffuse_color") {
                box?.runtime.functions.directionalLightSetDiffuseColor($0, $1) ?? 0
            }
        }

        /// `DirectionalLight.SpecularColor` — `ldfld cachedSpecularColor; ret`.
        public var SpecularColor: Microsoft.Xna.Framework.Vector3 { cachedSpecularColor }

        /// `set_SpecularColor`: the same shape as `SetDiffuseColor`.
        public func SetSpecularColor(_ value: Microsoft.Xna.Framework.Vector3) throws {
            if enabled { try specularColorParam?.SetValue(value) }
            cachedSpecularColor = value
            try push(value, "cna_directional_light_set_specular_color") {
                box?.runtime.functions.directionalLightSetSpecularColor($0, $1) ?? 0
            }
        }

        /// Mirrors a cached value onto the native light a stock effect's light
        /// views, so the effect the caller is about to apply sees it.
        ///
        /// A caller-built light has no native counterpart and this does
        /// nothing, which is why the cache — not the native light — is what
        /// every getter reads.
        private func push(
            _ value: Microsoft.Xna.Framework.Vector3,
            _ operation: String,
            _ route: (UInt64, CNASwift_Vector3) -> UInt32
        ) throws {
            guard let box else { return }
            let handle = try box.validated(operation)
            try box.runtime.functions.check(
                route(handle, CNASwift_Vector3(x: value.X, y: value.Y, z: value.Z)),
                operation: operation)
        }

        private func pushEnabled() throws {
            guard let box else { return }
            let handle = try box.validated("cna_directional_light_set_enabled")
            try box.runtime.functions.check(
                box.runtime.functions.directionalLightSetEnabled(
                    handle, enabled ? 1 : 0),
                operation: "cna_directional_light_set_enabled")
        }
    }
}
