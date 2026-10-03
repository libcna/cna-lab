// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    // Pinned in Microsoft.Xna.Framework.Game.dll as a public abstract
    // interface with no base interface and exactly one abstract method.
    // `Initialize` is an XNA lifecycle entry point, so it takes the
    // established `throws` language projection used by every `Game` lifecycle
    // member. No conformer is required to exist and none is supplied.
    public protocol IGameComponent {
        func Initialize() throws
    }
}
