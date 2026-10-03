// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    // The pinned metadata declares a public abstract interface with no base
    // interface and exactly four read/write properties, all abstract. It
    // declares no method and no conformer is required to exist.
    //
    // An abstract accessor has no body, so its fallibility is the fallibility
    // of the implementors a caller can actually be handed. All five registered
    // implementors -- AlphaTestEffect, BasicEffect, DualTextureEffect,
    // EnvironmentMapEffect and SkinnedEffect -- back three of the four
    // properties with plain fields:
    //
    //     set_FogEnabled: ldfld fogEnabled; beq; stfld; or dirty flags; ret
    //
    // `FogColor` is the exception. Every implementor forwards it straight to a
    // shader parameter:
    //
    //     get_FogColor: ldfld fogColorParam; callvirt EffectParameter::GetValueVector3()
    //     set_FogColor: ldfld fogColorParam; ldarg.1; callvirt EffectParameter::SetValue(Vector3)
    //
    // and both of those validate and throw -- `SetValue` opens with
    // `Elements.Count` and `newobj InvalidCastException; throw`, then checks
    // `ParameterClass` and `_columns` for an `InvalidOperationException`.
    // `FogColor` is therefore a fallible accessor pair, and the other three are
    // infallible; the accessor fallibility inventory records the exact chains.
    //
    // Swift forces the consequence rather than leaving it a preference. A
    // non-throwing `var FogColor: Vector3 { get }` requirement cannot be
    // witnessed by a throwing getter at all -- the compiler rejects the
    // conformance with "candidate throws, but protocol does not allow it" --
    // so a protocol that spelled it that way could never be conformed to by an
    // XNA-shaped effect. The reader is `{ get throws }`, which a non-throwing
    // witness still satisfies, and the writer cannot stay a `set` accessor for
    // two independent reasons: a Swift setter cannot throw, and the compiler
    // separately refuses any `set` beside a throwing getter. It becomes
    // `SetFogColor`, which is the projection of the CLR setter accessor and not
    // a new XNA member.
    //
    // `SetFogColor` is not `mutating`: the CLR setter mutates a class, and
    // every registered implementor is a class. That is the same basis on which
    // `IPackedVector.PackFromVector4` *is* mutating -- there the CLR operation
    // rewrites packed struct storage.
    public protocol IEffectFog {
        var FogEnabled: Bool { get set }

        var FogStart: Float { get set }

        var FogEnd: Float { get set }

        var FogColor: Microsoft.Xna.Framework.Vector3 { get throws }

        func SetFogColor(_ value: Microsoft.Xna.Framework.Vector3) throws
    }
}
