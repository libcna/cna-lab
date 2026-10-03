// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.EffectMaterial` projection.
    ///
    /// The smallest type in the stock set: one public constructor and nothing
    /// else. Its whole body is eight bytes of IL —
    ///
    /// ```text
    /// IL_0000: ldarg.0; ldarg.1
    /// IL_0002: call instance void Effect::.ctor(Effect)
    /// IL_0007: ret
    /// ```
    ///
    /// — so it adds no state and no behaviour to `Effect`. What it adds is a
    /// **type**: the content pipeline writes an `EffectMaterial` where a model
    /// mesh part's effect was authored as a material, and `Model` reads the
    /// type back to know which it is.
    ///
    /// Its clone source is a plain `Effect`, not an `EffectMaterial`, which is
    /// what lets the pipeline build one over any loaded effect.
    open class EffectMaterial: Effect {
        /// `EffectMaterial(Effect cloneSource)`.
        public init(cloneSource: Effect) throws {
            let source = try cloneSource.validatedHandle(
                "EffectMaterial.init(cloneSource:)")
            let runtime = cloneSource.nativeStorage.runtime
            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.effectMaterialCreate(source, &handle),
                operation: "cna_effect_material_create")
            super.init(handle: handle, runtime: runtime,
                       device: cloneSource.GraphicsDevice,
                       typeName: "EffectMaterial")
        }
    }
}
