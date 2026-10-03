// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.GamerServices {

    /// `Microsoft.Xna.Framework.GamerServices.GamerServicesComponent`.
    ///
    /// A `GameComponent` whose whole job is to pump the gamer-services
    /// dispatcher: XNA's `Initialize` calls
    /// `GamerServicesDispatcher.Initialize(Game)` and its `Update` calls
    /// `GamerServicesDispatcher.Update()`, and this does exactly those two
    /// things through the routes of the same names.
    ///
    /// **The component handle CNA offers is deliberately not used.**
    /// `cna_gamer_services_component_create` makes a *canonical* component
    /// whose initialize and update belong to the runtime -- which is the right
    /// answer for a C caller building a component set, and the wrong one here:
    /// this type IS the component, XNA's own two calls are what it performs,
    /// and a consumer overriding `Update` must be able to decide whether the
    /// base runs. A route with no consuming member is not bound, so it is not.
    public class GamerServicesComponent: Microsoft.Xna.Framework.GameComponent {

        /// `.ctor(Game game)`.
        public override init(game: Microsoft.Xna.Framework.Game) {
            super.init(game: game)
        }

        /// `GamerServicesComponent.Initialize()`.
        ///
        /// XNA's body is `GamerServicesDispatcher.Initialize(Game)`; the
        /// dispatcher takes the game, and here the runtime holds it.
        public override func Initialize() throws {
            let runtime = try RuntimeRegistry.current()
            try runtime.functions.check(
                runtime.functions.gamerServicesDispatcherInitialize(
                    runtime.gameHandle),
                operation: "cna_gamer_services_dispatcher_initialize")
        }

        /// `GamerServicesComponent.Update(GameTime gameTime)`.
        ///
        /// XNA's body is `GamerServicesDispatcher.Update()` and it ignores the
        /// `gameTime` it is handed -- so this does too, rather than inventing a
        /// use for it.
        public override func Update(
            _ gameTime: Microsoft.Xna.Framework.GameTime
        ) throws {
            let runtime = try RuntimeRegistry.current()
            try runtime.functions.check(
                runtime.functions.gamerServicesDispatcherUpdate(),
                operation: "cna_gamer_services_dispatcher_update")
        }
    }
}
