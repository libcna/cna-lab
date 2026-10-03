// SPDX-License-Identifier: MIT

import Foundation

extension Microsoft.Xna.Framework {
    /// `Microsoft.Xna.Framework.GameComponent`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.Game.dll` as a public **unsealed**
    /// class on `System.Object` implementing `IGameComponent`, `IUpdateable`
    /// and `System.IDisposable`. It is the base every ordinary XNA component
    /// derives from, and it is what the managed engine in `Game` was built
    /// for: adding one to `Game.Components` initializes it, updates it in
    /// `UpdateOrder` sequence while it is `Enabled`, and stops doing both when
    /// it is removed.
    ///
    /// Its own IL is entirely managed. Every member below is transcribed from
    /// it, and three details are the ones a plausible reimplementation gets
    /// wrong:
    ///
    /// - the constructor sets `enabled = true` **before** calling the base
    ///   constructor, so a component starts enabled and starts at
    ///   `UpdateOrder` zero;
    /// - both setters compare first and **raise nothing** when the value is
    ///   unchanged, so assigning the same order twice reorders the game's list
    ///   once, not twice;
    /// - `OnEnabledChanged` and `OnUpdateOrderChanged` ignore the `sender`
    ///   they are handed and raise the event with `this`. The parameter is
    ///   part of the signature and is not part of the behaviour.
    ///
    /// ## The parent reference is strong, and that is a cycle
    ///
    /// `Game` is a plain field read of the constructor's argument, so a
    /// component holds its game and `Game.Components` holds the component:
    /// a reference cycle. The CLR has the same cycle and a collector that
    /// does not care; Swift has neither. It is reproduced rather than weakened
    /// because the pinned inventory proves `Game` non-null, and a `weak`
    /// field would have had to answer nil — a state XNA cannot produce.
    ///
    /// `Dispose()` is what XNA gives a consumer to break it: it removes the
    /// component from `Game.Components`, which drops the game's half of the
    /// cycle. A component that is never disposed and whose game is dropped
    /// keeps both alive. That is stated rather than papered over, and it is
    /// the first projected type where the difference between a collector and
    /// reference counting is visible at all.
    ///
    /// CLR non-sealed maps to Swift `open`.
    open class GameComponent:
        Microsoft.Xna.Framework.IGameComponent,
        Microsoft.Xna.Framework.IUpdateable
    {
        private var enabled = true
        private var updateOrder: Int32 = 0
        private let owner: Microsoft.Xna.Framework.Game
        private var disposed = false

        // `lock (this)` in `Dispose(bool)`. The CLR uses the component itself
        // as the monitor; a private lock gives the same mutual exclusion
        // without publishing the object as one, which nothing in the projected
        // surface would let a caller use anyway.
        private let disposeLock = NSLock()

        private let enabledChangedSource = CNAEventSource<CNAEventArgs>()
        private let updateOrderChangedSource = CNAEventSource<CNAEventArgs>()
        private let disposedSource = CNAEventSource<CNAEventArgs>()

        /// `.ctor(Game game)`.
        ///
        /// `enabled = true; base..ctor(); game = game`. The CLR stores the
        /// argument without a null check; a non-Optional Swift parameter
        /// cannot be nil, so that path is unreachable rather than removed.
        public init(game: Microsoft.Xna.Framework.Game) {
            self.owner = game
        }

        /// `GameComponent.Game` — the stored field.
        public var Game: Microsoft.Xna.Framework.Game { owner }

        /// `GameComponent.Enabled`.
        ///
        /// `IUpdateable` declares it get-only; `GameComponent` adds the
        /// setter, which raises `EnabledChanged` through `OnEnabledChanged`
        /// **only when the value actually changes**.
        public var Enabled: Bool {
            get { enabled }
            set {
                guard enabled != newValue else { return }
                enabled = newValue
                try? OnEnabledChanged(self, args: CNAEventArgs.Empty)
            }
        }

        /// `GameComponent.UpdateOrder`.
        ///
        /// Same shape as `Enabled`: the setter raises through
        /// `OnUpdateOrderChanged` only on a real change. That event is what
        /// makes the game re-sort its updateable list.
        public var UpdateOrder: Int32 {
            get { updateOrder }
            set {
                guard updateOrder != newValue else { return }
                updateOrder = newValue
                try? OnUpdateOrderChanged(self, args: CNAEventArgs.Empty)
            }
        }

        /// `event EventHandler<EventArgs> EnabledChanged`.
        public var EnabledChanged: CNAEvent<CNAEventArgs> {
            enabledChangedSource.Event
        }

        /// `event EventHandler<EventArgs> UpdateOrderChanged`.
        public var UpdateOrderChanged: CNAEvent<CNAEventArgs> {
            updateOrderChangedSource.Event
        }

        /// `event EventHandler<EventArgs> Disposed`.
        public var Disposed: CNAEvent<CNAEventArgs> { disposedSource.Event }

        /// `public virtual void Initialize()`. The base body is `ret`.
        open func Initialize() throws {}

        /// `public virtual void Update(GameTime gameTime)`. The base body is
        /// `ret`: a component that overrides nothing does nothing, and the
        /// game still walks it.
        open func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {}

        /// `protected virtual void OnEnabledChanged(object sender, EventArgs args)`.
        ///
        /// The IL raises the event with **`this`** as the sender and the
        /// supplied `args`; the `sender` parameter is not used. Reproduced
        /// exactly, including that.
        open func OnEnabledChanged(_ sender: Any?, args: CNAEventArgs) throws {
            try enabledChangedSource.Raise(self, args: args)
        }

        /// `protected virtual void OnUpdateOrderChanged(object sender, EventArgs args)`.
        open func OnUpdateOrderChanged(
            _ sender: Any?, args: CNAEventArgs
        ) throws {
            try updateOrderChangedSource.Raise(self, args: args)
        }

        /// `public void Dispose()`.
        ///
        /// `Dispose(true); GC.SuppressFinalize(this)`. It is `virtual final`
        /// in the metadata — a sealed `IDisposable` implementation — so it is
        /// `final` here and the override point is `Dispose(_:)`.
        /// `SuppressFinalize` has no analogue: Swift has no finalizer to
        /// suppress, and inventing one would add lifetime behaviour XNA does
        /// not have.
        public final func Dispose() throws {
            try Dispose(true)
        }

        /// `protected virtual void Dispose(bool disposing)`.
        ///
        /// ```text
        /// if (!disposing) return
        /// lock (this) {
        ///     if (Game != null) Game.Components.Remove(this)
        ///     if (Disposed != null) Disposed(this, EventArgs.Empty)
        /// }
        /// ```
        ///
        /// Removing itself from `Game.Components` is what drives the game's
        /// `GameComponentRemoved` handler, so a disposed component stops being
        /// updated — and it is what breaks the parent cycle. The `Disposed`
        /// event is raised **after** the removal, so a handler observes a
        /// component the game has already let go of.
        ///
        /// `disposing: false` is the finalizer path and does nothing at all,
        /// which is transcribed rather than dropped.
        open func Dispose(_ disposing: Bool) throws {
            guard disposing else { return }
            disposeLock.lock()
            defer { disposeLock.unlock() }
            try Game.Components.Remove(self)
            try disposedSource.Raise(self, args: CNAEventArgs.Empty)
            disposed = true
        }
    }
}
