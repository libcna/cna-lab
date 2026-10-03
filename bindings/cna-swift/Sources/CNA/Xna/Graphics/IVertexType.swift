// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.IVertexType` projection.
    ///
    /// One member, a get-only `VertexDeclaration`. Its recorded accessor
    /// verdict is `IL_ABSTRACT_DECLARATION` with `fallible: false` — an
    /// interface declaration has no body to fail in — so the requirement is a
    /// plain non-throwing reader, and every registered implementor witnesses it
    /// with a field read.
    ///
    /// XNA's four vertex structs implement it **explicitly**: each declares
    /// `private newslot specialname virtual final ... IVertexType.get_VertexDeclaration`
    /// with an `.override`. Swift has no private conformance, so the witness is
    /// public — the single widening this projection makes for an explicit
    /// interface implementation, recorded in `protocolWitnessMemberProjections`
    /// exactly as the packed-vector family's are.
    public protocol IVertexType {
        var VertexDeclaration: Microsoft.Xna.Framework.Graphics.VertexDeclaration { get }
    }
}
