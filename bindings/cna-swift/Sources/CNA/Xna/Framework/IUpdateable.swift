// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    // Pinned in Microsoft.Xna.Framework.Game.dll as a public abstract
    // interface with no base interface and exactly five public identities:
    // `get_Enabled`, `get_UpdateOrder`, `Update`, and the `EnabledChanged` and
    // `UpdateOrderChanged` events. The IL declares `add_`/`remove_` accessor
    // pairs and no `raise_` accessor at all; those accessors are the CLR
    // encoding of an event, not XNA identities, and are not projected.
    //
    // Both properties are get-only in the pinned metadata: a conformer decides
    // how `Enabled` and `UpdateOrder` change and is responsible for raising the
    // matching event when they do. `Update` is an XNA lifecycle entry point, so
    // it takes the established `throws` language projection already used by
    // `Game.Update` and by `IGameComponent.Initialize`.
    //
    // No default implementation is supplied. XNA supplies none, and Swift needs
    // none: every requirement is satisfiable by an external conformer that owns
    // private `CNAEventSource` instances and publishes their `Event` views.
    public protocol IUpdateable {
        var Enabled: Bool { get }

        var UpdateOrder: Int32 { get }

        var EnabledChanged: CNAEvent<CNAEventArgs> { get }

        var UpdateOrderChanged: CNAEvent<CNAEventArgs> { get }

        func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws
    }
}
