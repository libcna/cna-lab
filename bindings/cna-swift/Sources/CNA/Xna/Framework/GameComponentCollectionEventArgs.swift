// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    // Pinned in Microsoft.Xna.Framework.Game.dll as a public, non-sealed class
    // extending `System.EventArgs`, with exactly two public identities: the
    // public constructor `.ctor(IGameComponent gameComponent)` and the get-only
    // property `GameComponent`. The constructor chains
    // `System.EventArgs::.ctor()` and stores its argument verbatim; the getter
    // returns that field. Non-sealed with a public constructor maps to `open`.
    //
    // The IL stores the argument without a null check, so XNA accepts a null
    // component here and the projection accordingly does not add one. The
    // parameter is non-optional because `IGameComponent` is a Swift protocol
    // existential and the pinned signature has no `[AllowNull]`-style
    // annotation to project from.
    //
    // XNA's producer is `GameComponentCollection`, whose base is
    // `System.Collections.ObjectModel.Collection<IGameComponent>` — an
    // unmapped BCL collection base — so that type stays deferred. This event
    // argument is independently complete: nothing it declares depends on the
    // collection.
    open class GameComponentCollectionEventArgs: CNAEventArgs {
        private let gameComponent: Microsoft.Xna.Framework.IGameComponent

        public init(gameComponent: Microsoft.Xna.Framework.IGameComponent) {
            self.gameComponent = gameComponent
            super.init()
        }

        public var GameComponent: Microsoft.Xna.Framework.IGameComponent {
            gameComponent
        }
    }
}
