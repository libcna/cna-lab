// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    // The pinned metadata declares a public abstract interface with no base
    // interface, one method and five properties, all abstract. Three of the
    // properties are getter-only.
    //
    // `LightingEnabled` is the reason this protocol does not read like
    // `IEffectMatrices`. An abstract accessor has no body, so it is fallible
    // exactly when a registered implementor is -- and two of the three
    // implementors are:
    //
    //     EnvironmentMapEffect::IEffectLights.get_LightingEnabled:
    //         ldc.i4.1; ret
    //     EnvironmentMapEffect::IEffectLights.set_LightingEnabled:
    //         ldarg.1; brtrue.s IL_0032
    //         ... newobj NotSupportedException(Format(CantDisableLighting,
    //                                                 GetType().Name)); throw
    //         IL_0032: ret
    //
    // SkinnedEffect's pair is byte-for-byte the same. Both are EXPLICIT
    // interface implementations, which is why neither type declares a public
    // `LightingEnabled` at all: their lighting is always on, and asking to
    // turn it off is refused rather than ignored.
    //
    // BasicEffect is the third implementor and implements the member
    // implicitly, with an infallible field-backed setter. So the interface
    // requirement is fallible while one of its implementors' own accessors is
    // not -- the reader is `{ get }` and the writer is a throwing method,
    // which a non-throwing witness still satisfies.
    //
    // `EnableDefaultLighting` throws for a plainer reason: it drives the three
    // lights' setters, and `DirectionalLight`'s setters write shader
    // parameters.
    public protocol IEffectLights {
        func EnableDefaultLighting() throws

        // The pinned return-nullability reference records all three as
        // normally-null-returning: XNA holds them in fields that only
        // `CacheEffectParameters` fills, so a derived effect that has not run
        // it yet answers null. Optional, therefore, even though every effect
        // projected here fills them in its constructor.
        var DirectionalLight0: DirectionalLight? { get }

        var DirectionalLight1: DirectionalLight? { get }

        var DirectionalLight2: DirectionalLight? { get }

        var AmbientLightColor: Microsoft.Xna.Framework.Vector3 { get set }

        var LightingEnabled: Bool { get }

        func SetLightingEnabled(_ value: Bool) throws
    }
}
