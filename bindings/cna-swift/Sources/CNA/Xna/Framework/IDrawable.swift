// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    // Pinned in Microsoft.Xna.Framework.Game.dll as a public abstract
    // interface with no base interface and exactly five public identities:
    // `get_Visible`, `get_DrawOrder`, `Draw`, and the `VisibleChanged` and
    // `DrawOrderChanged` events. As with `IUpdateable`, the IL declares
    // `add_`/`remove_` accessor pairs and no `raise_` accessor, and those
    // accessors are not XNA identities.
    //
    // `IDrawable` does **not** extend `IUpdateable` in the pinned metadata —
    // the two interfaces are independent, and `DrawableGameComponent` is what
    // implements both. Nothing here conforms to either.
    public protocol IDrawable {
        var Visible: Bool { get }

        var DrawOrder: Int32 { get }

        var VisibleChanged: CNAEvent<CNAEventArgs> { get }

        var DrawOrderChanged: CNAEvent<CNAEventArgs> { get }

        func Draw(_ gameTime: Microsoft.Xna.Framework.GameTime) throws
    }
}
