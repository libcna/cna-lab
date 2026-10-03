// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    /// The `Microsoft.Xna.Framework.DrawableGameComponent` projection.
    ///
    /// Blocked since Foundation 24 on one thing: `Initialize` resolves
    /// `IGraphicsDeviceService` out of `Game.Services`, and nothing produced
    /// that service. Foundation 40 supplied the producer XNA itself supplies —
    /// `GraphicsDeviceManager`, which registers itself in its own constructor —
    /// so the resolution is real and the `InvalidOperationException` a game
    /// with no manager gets is the one XNA raises, not a stand-in.
    ///
    /// CLR non-sealed maps to Swift `open`.
    open class DrawableGameComponent: Microsoft.Xna.Framework.GameComponent,
                                      Microsoft.Xna.Framework.IDrawable {
        private var visible = true
        private var drawOrder: Int32 = 0
        private var initialized = false
        private var deviceService: Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService?
        private var deviceSubscriptions: [CNAEventSubscription] = []
        private let visibleChangedSource = CNAEventSource<CNAEventArgs>()
        private let drawOrderChangedSource = CNAEventSource<CNAEventArgs>()

        /// The exact `MissingGraphicsDeviceService` message, read out of
        /// `Microsoft.Xna.Framework.Game.dll`'s own resource table. It is a
        /// **different** string from `Game.GraphicsDevice`'s
        /// `NoGraphicsDeviceService`, and the two are not interchangeable.
        /// The exact `PropertyCannotBeCalledBeforeInitialize` message, read
        /// out of `Microsoft.Xna.Framework.Game.dll`'s own resource table.
        /// `GraphicsDevice` raises this one; `Initialize` raises
        /// `MissingGraphicsDeviceService`. Three keys, three meanings.
        internal static let propertyCannotBeCalledBeforeInitializeMessage =
            "The GraphicsDevice property cannot be used before Initialize "
            + "has been called."

        internal static let missingGraphicsDeviceServiceMessage =
            "Drawable components require a graphics device service in the game "
            + "service container."

        /// `DrawableGameComponent..ctor(Game game)`.
        public override init(game: Microsoft.Xna.Framework.Game) {
            super.init(game: game)
        }

        /// `IDrawable.Visible`.
        ///
        /// The setter compares first and raises `VisibleChanged` through
        /// `OnVisibleChanged` **only when the value actually changed** — a
        /// write of the same value stores nothing and raises nothing.
        public var Visible: Bool {
            get { visible }
            set {
                guard visible != newValue else { return }
                visible = newValue
                try? OnVisibleChanged(self, args: CNAEventArgs.Empty)
            }
        }

        /// `IDrawable.DrawOrder`. The same compare-then-raise shape.
        public var DrawOrder: Int32 {
            get { drawOrder }
            set {
                guard drawOrder != newValue else { return }
                drawOrder = newValue
                try? OnDrawOrderChanged(self, args: CNAEventArgs.Empty)
            }
        }

        /// `IDrawable.VisibleChanged`.
        public var VisibleChanged: CNAEvent<CNAEventArgs> { visibleChangedSource.Event }

        /// `IDrawable.DrawOrderChanged`.
        public var DrawOrderChanged: CNAEvent<CNAEventArgs> {
            drawOrderChangedSource.Event
        }

        /// `DrawableGameComponent.GraphicsDevice`.
        ///
        /// The IL raises `InvalidOperationException(MissingGraphicsDeviceService)`
        /// when `Initialize` has not resolved a service, and otherwise returns
        /// `deviceService.GraphicsDevice` — which is itself Optional. So the
        /// getter is fallible and its result is Optional, exactly as
        /// `Game.GraphicsDevice` is.
        public var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? {
            get throws {
                guard let deviceService else {
                    // `PropertyCannotBeCalledBeforeInitialize`, not
                    // `MissingGraphicsDeviceService`. The type raises exactly
                    // two messages and this projection used the same one at
                    // both sites until Foundation 50; the pinned IL gives
                    // `get_GraphicsDevice` this key and `Initialize` the
                    // other, and they say different things to a user.
                    throw CNAInvalidOperationException(
                        message: DrawableGameComponent
                            .propertyCannotBeCalledBeforeInitializeMessage)
                }
                return deviceService.GraphicsDevice
            }
        }

        /// `DrawableGameComponent.Initialize()`.
        ///
        /// The IL, in order:
        ///
        /// ```text
        /// base.Initialize()
        /// if (initialized) return
        /// deviceService = Game.Services.GetService(IGraphicsDeviceService) as …
        /// if (deviceService == null) throw InvalidOperationException(MissingGraphicsDeviceService)
        /// deviceService.DeviceCreated   += DeviceCreated     // -> LoadContent()
        /// deviceService.DeviceResetting += DeviceResetting   // -> nothing
        /// deviceService.DeviceReset     += DeviceReset       // -> nothing
        /// deviceService.DeviceDisposing += DeviceDisposing   // -> UnloadContent()
        /// if (deviceService.GraphicsDevice != null) LoadContent()
        /// initialized = true
        /// ```
        ///
        /// The `initialized` guard is a **one-time** guard on the resolution
        /// and subscription, not on `LoadContent`: the `DeviceCreated`
        /// subscription is what reloads content after a device reset, and it
        /// stays subscribed. Guarding the reload as well would fix startup and
        /// break every subsequent reset, which is the exact trap this
        /// milestone was told to avoid.
        open override func Initialize() throws {
            try super.Initialize()
            guard !initialized else { return }
            deviceService = Game.Services.GetService(
                Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService.self)
                as? Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService
            guard let deviceService else {
                throw CNAInvalidOperationException(
                    message: DrawableGameComponent.missingGraphicsDeviceServiceMessage)
            }
            deviceSubscriptions.append(deviceService.DeviceCreated.Add {
                [weak self] _, _ in try self?.LoadContent()
            })
            // `DeviceResetting` and `DeviceReset` have empty bodies in the IL.
            // They are subscribed anyway, because the subscription itself is
            // observable through the service and dropping it would change what
            // a service sees; the handlers do nothing, as XNA's do.
            deviceSubscriptions.append(deviceService.DeviceResetting.Add { _, _ in })
            deviceSubscriptions.append(deviceService.DeviceReset.Add { _, _ in })
            deviceSubscriptions.append(deviceService.DeviceDisposing.Add {
                [weak self] _, _ in try self?.UnloadContent()
            })
            if deviceService.GraphicsDevice != nil {
                try LoadContent()
            }
            initialized = true
        }

        /// `protected virtual void LoadContent()` — an empty body in the IL.
        open func LoadContent() throws {}

        /// `protected virtual void UnloadContent()` — an empty body in the IL.
        open func UnloadContent() throws {}

        /// `IDrawable.Draw` — `virtual` with an empty body.
        open func Draw(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {}

        /// `protected virtual void OnVisibleChanged(object sender, EventArgs args)`.
        ///
        /// Raises with the component as sender, like every other `On…` raise
        /// site in this assembly: the setter passes `this` and the handler is
        /// invoked with `this`.
        open func OnVisibleChanged(_ sender: Any?, args: CNAEventArgs) throws {
            try visibleChangedSource.Raise(self, args: args)
        }

        /// `protected virtual void OnDrawOrderChanged(object sender, EventArgs args)`.
        open func OnDrawOrderChanged(_ sender: Any?, args: CNAEventArgs) throws {
            try drawOrderChangedSource.Raise(self, args: args)
        }

        /// `protected override void Dispose(bool disposing)`.
        ///
        /// The IL calls `UnloadContent()` first, then removes all four device
        /// handlers, and only then runs the base's own disposal.
        open override func Dispose(_ disposing: Bool) throws {
            guard disposing else { return }
            try UnloadContent()
            if let deviceService {
                deviceService.DeviceCreated.Remove(deviceSubscriptions[0])
                deviceService.DeviceResetting.Remove(deviceSubscriptions[1])
                deviceService.DeviceReset.Remove(deviceSubscriptions[2])
                deviceService.DeviceDisposing.Remove(deviceSubscriptions[3])
                deviceSubscriptions.removeAll()
            }
            try super.Dispose(disposing)
        }
    }
}
