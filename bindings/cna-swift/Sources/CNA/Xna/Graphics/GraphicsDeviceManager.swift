// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework {
    /// The `Microsoft.Xna.Framework.GraphicsDeviceManager` projection.
    ///
    /// `open`, not `final`: XNA leaves the class derivable and its public
    /// `GraphicsDeviceManager(Game)` constructor is projected, so a consumer
    /// can genuinely derive from this one.
    ///
    /// It is the **producer** of `IGraphicsDeviceService`, which is what
    /// `Game.GraphicsDevice` and `DrawableGameComponent` resolve out of
    /// `Game.Services`. That producer is not fabricated: CNA's own manager
    /// owns the device, hands it out through
    /// `cna_graphics_device_manager_get_graphics_device`, and raises the four
    /// device events this service declares — the header says so in as many
    /// words: *"the four device events are also the canonical
    /// graphics-device-service events, so subscribing here is what a consumer
    /// of that service would observe."*
    ///
    /// ## There is only one container, and it is the managed one
    ///
    /// Registering into `Game.Services` creates no second lifecycle. CNA's
    /// native game owns the device and reads nothing from this container;
    /// `GameServiceContainer` is a CLR projection whose only consumers are
    /// `Game.GraphicsDevice`, `DrawableGameComponent` and whatever a caller
    /// looks up. So the registration adds a lookup XNA has and no ownership
    /// XNA does not.
    open class GraphicsDeviceManager: Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService,
                                      Microsoft.Xna.Framework.IGraphicsDeviceManager,
                                      RuntimeOwnedChild {
        private weak var game: Microsoft.Xna.Framework.Game?
        private let storage: NativeHandleStorage
        private let deviceCreatedSource = CNAEventSource<CNAEventArgs>()
        private let deviceDisposingSource = CNAEventSource<CNAEventArgs>()
        private let deviceResetSource = CNAEventSource<CNAEventArgs>()
        private let deviceResettingSource = CNAEventSource<CNAEventArgs>()
        private let disposedSource = CNAEventSource<CNAEventArgs>()
        private let preparingDeviceSettingsSource =
            CNAEventSource<Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs>()
        private var eventRegistrations: [UInt32: UInt64] = [:]
        private var eventBoxes: [Unmanaged<GraphicsDeviceManagerEventBox>] = []

        /// `DefaultBackBufferWidth`. `public static initonly int32`, assigned
        /// `0x320` by the class constructor. `initonly` is why this is a Swift
        /// `let` rather than a `var`.
        public static let DefaultBackBufferWidth: Int32 = 800

        /// `DefaultBackBufferHeight`. `public static initonly int32`, assigned
        /// `0x1e0` by the class constructor.
        public static let DefaultBackBufferHeight: Int32 = 480

        /// The exact `GraphicsDeviceManagerAlreadyPresent` message, read out of
        /// `Microsoft.Xna.Framework.Game.dll`'s own resource table.
        internal static let alreadyPresentMessage =
            "A graphics device manager is already registered.  The graphics "
            + "device manager cannot be changed once it is set."

        /// `GraphicsDeviceManager..ctor(Game game)`.
        ///
        /// The IL order is exact and every step of it is observable:
        ///
        /// ```text
        /// defaults: SynchronizeWithVerticalRetrace = true, depthStencilFormat = Depth24
        /// game == null                         -> ArgumentNullException("game", GameCannotBeNull)
        /// Services.GetService(IGraphicsDeviceManager) != null
        ///                                      -> ArgumentException(GraphicsDeviceManagerAlreadyPresent)
        /// Services.AddService(IGraphicsDeviceManager, this)
        /// Services.AddService(IGraphicsDeviceService, this)
        /// Window.ClientSizeChanged     += GameWindowClientSizeChanged
        /// Window.ScreenDeviceNameChanged += GameWindowScreenDeviceNameChanged
        /// ```
        ///
        /// The null-game branch is unreachable through a non-Optional Swift
        /// class parameter. The two window subscriptions are **not** projected
        /// and are not faked: `GameWindow` is not implemented, so there is no
        /// event to subscribe to; that is a recorded absence, not a silent one.
        public init(game: Microsoft.Xna.Framework.Game) throws {
            let runtime = game.runtime
            try runtime.validateGeneration(runtime.generation)
            try runtime.owner.validate("GraphicsDeviceManager.init")

            // The duplicate check happens BEFORE the native manager is
            // created, exactly as XNA checks before storing anything: a
            // refused second manager must leave no native object behind.
            if game.Services.GetService(Microsoft.Xna.Framework.IGraphicsDeviceManager.self) != nil {
                throw CNAArgumentException(
                    message: GraphicsDeviceManager.alreadyPresentMessage)
            }

            var handle: UInt64 = 0
            try runtime.functions.check(
                runtime.functions.graphicsManagerCreate(runtime.gameHandle, &handle),
                operation: "cna_graphics_device_manager_create"
            )
            self.game = game
            storage = NativeHandleStorage(
                handle: handle,
                typeName: "GraphicsDeviceManager",
                ownership: .owned,
                runtime: runtime,
                destroy: runtime.functions.graphicsManagerDestroy
            )
            runtime.register(self)
            try game.Services.AddService(
                Microsoft.Xna.Framework.IGraphicsDeviceManager.self, provider: self)
            try game.Services.AddService(
                Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService.self, provider: self)
            subscribeToManagerEvents(handle: handle)
        }

        /// `IGraphicsDeviceService.GraphicsDevice`.
        ///
        /// Optional and infallible, which the pinned evidence settles: the one
        /// registered implementor's accessor is a bare field read, and the
        /// field is null until `CreateDevice` and null again after `Dispose`.
        /// So "no device right now" is a normal result and not a failure.
        ///
        /// A CNA graphics device is callback-scoped, so outside a lifecycle
        /// callback there is no device this service can hand out and the
        /// answer is nil — which is the same state XNA reports before the loop
        /// has created one.
        public var GraphicsDevice: Microsoft.Xna.Framework.Graphics.GraphicsDevice? {
            guard let handle = try? storage.validatedHandle(
                "GraphicsDeviceManager.GraphicsDevice") else { return nil }
            // Defence in depth, and recorded as such: CNA refuses this route
            // outside a lifecycle callback on its own, with
            // CNA_RESULT_INVALID_STATE and a zero handle -- measured by
            // build-probe/f40_devicescope.c. Removing this guard therefore
            // changes nothing observable, which is why no mutation control
            // covers it; the guard states the scope rule in the binding rather
            // than relying on the host to keep enforcing it.
            guard storage.runtime.isInsideCallback else { return nil }
            var device: UInt64 = 0
            guard storage.runtime.functions.graphicsManagerGetGraphicsDevice(
                handle, &device) == 0, device != 0 else { return nil }
            // The facade caches the device's own GraphicsProfile the first
            // time one is built. This getter is `IGraphicsDeviceService`'s and
            // cannot throw, and it already answers nil for every other reason
            // the device is not available, so a profile that cannot be read
            // answers nil here too rather than inventing one.
            return try? Microsoft.Xna.Framework.Graphics.GraphicsDevice(
                borrowedHandle: device, runtime: storage.runtime)
        }

        /// `IGraphicsDeviceService.DeviceCreated`.
        public var DeviceCreated: CNAEvent<CNAEventArgs> { deviceCreatedSource.Event }

        /// `IGraphicsDeviceService.DeviceDisposing`.
        public var DeviceDisposing: CNAEvent<CNAEventArgs> { deviceDisposingSource.Event }

        /// `IGraphicsDeviceService.DeviceReset`.
        public var DeviceReset: CNAEvent<CNAEventArgs> { deviceResetSource.Event }

        /// `IGraphicsDeviceService.DeviceResetting`.
        public var DeviceResetting: CNAEvent<CNAEventArgs> { deviceResettingSource.Event }

        /// `GraphicsDeviceManager.Disposed`.
        ///
        /// Not an `IGraphicsDeviceService` member: the four above are the
        /// service's, and this one is the manager's own, raised at the end of
        /// `Dispose(true)`. It is a plain `EventHandler<EventArgs>` field with
        /// the ordinary `Delegate.Combine`/`Remove` accessors.
        public var Disposed: CNAEvent<CNAEventArgs> { disposedSource.Event }

        /// `GraphicsDeviceManager.PreparingDeviceSettings`.
        ///
        /// The one event whose payload a handler is meant to **change**: it
        /// carries the `GraphicsDeviceInformation` the manager is about to
        /// create a device from, and the manager reads that same object back
        /// afterwards.
        ///
        /// It is the only event on this type that is not
        /// `EventHandler<EventArgs>`, which is why it needs its own source.
        public var PreparingDeviceSettings:
            CNAEvent<Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs> {
            preparingDeviceSettingsSource.Event
        }

        /// `protected virtual void OnDeviceCreated(object sender, EventArgs args)`.
        ///
        /// ```text
        /// if (deviceCreated != null) deviceCreated(sender, args);
        /// ```
        ///
        /// `ldarg.1` then `ldarg.2`: the manager's raisers forward the
        /// **caller's** sender, where `Game.OnActivated` pushes `ldarg.0` and
        /// ignores its own `sender` parameter. Two raisers of the same shape
        /// in the same framework that do different things with the argument,
        /// so each is transcribed from its own IL.
        open func OnDeviceCreated(_ sender: Any?, args: CNAEventArgs) throws {
            try deviceCreatedSource.Raise(sender, args: args)
        }

/// `protected virtual void OnPreparingDeviceSettings(object sender,
        /// PreparingDeviceSettingsEventArgs args)`.
        ///
        /// Twenty-two bytes: the null test, then the invoke. The same
        /// caller's-sender forwarding as the raisers around it.
        open func OnPreparingDeviceSettings(
            _ sender: Any?,
            args: Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs
        ) throws {
            try preparingDeviceSettingsSource.Raise(sender, args: args)
        }

        /// `protected virtual GraphicsDeviceInformation FindBestDevice(bool
        /// anySuitableDevice)`.
        ///
        /// Eight bytes forwarding to `FindBestPlatformDevice`, which is 132:
        ///
        /// ```text
        /// found = new List<GraphicsDeviceInformation>()
        /// AddDevices(anySuitableDevice, found)
        /// if (found.Count == 0 && PreferMultiSampling) {
        ///     PreferMultiSampling = false          // and it STAYS false
        ///     AddDevices(anySuitableDevice, found)
        /// }
        /// if (found.Count == 0) throw NoSuitableGraphicsDeviceException(...)
        /// RankDevices(found)
        /// if (found.Count == 0) throw NoSuitableGraphicsDeviceException(...)
        /// return found[0]
        /// ```
        ///
        /// **The retry is destructive.** Turning multisampling off to find any
        /// device at all leaves `PreferMultiSampling` false afterwards; XNA
        /// never puts it back, and neither does this.
        ///
        /// **The emptiness test after `RankDevices` looks unreachable and is
        /// not.** Ranking only sorts — but it is `virtual`, so an override can
        /// empty the list, and the second test exists for that. Reproduced
        /// rather than dropped as dead.
        open func FindBestDevice(
            _ anySuitableDevice: Bool
        ) throws -> Microsoft.Xna.Framework.GraphicsDeviceInformation {
            let found = CNAList<Microsoft.Xna.Framework.GraphicsDeviceInformation>()
            try addDevices(anySuitableDevice, found)
            if found.Count == 0 && PreferMultiSampling {
                PreferMultiSampling = false
                try addDevices(anySuitableDevice, found)
            }
            if found.Count == 0 {
                throw Microsoft.Xna.Framework.Graphics
                    .NoSuitableGraphicsDeviceException(
                        message: GraphicsDeviceManager.noCompatibleDevicesMessage(
                            GraphicsProfile))
            }
            try RankDevices(found)
            if found.Count == 0 {
                throw Microsoft.Xna.Framework.Graphics
                    .NoSuitableGraphicsDeviceException(
                        message: GraphicsDeviceManager
                            .noCompatibleDevicesAfterRankingMessage)
            }
            return try found.Item(0)
        }

        /// `AddDevices(bool anySuitableDevice, List<GraphicsDeviceInformation>)`.
        ///
        /// Walks every adapter, skips those that do not support the requested
        /// profile, and offers each survivor's current display mode — plus, in
        /// full screen, every supported mode at least **640x480** (`0x280` by
        /// `0x1e0` in the IL).
        ///
        /// **`IsWindowOnAdapter` cannot be reproduced and is treated as true.**
        /// XNA's is `ScreenFromAdapter(adapter) == ScreenFromHandle(handle)`,
        /// two `System.Windows.Forms` calls with no counterpart here: CNA
        /// publishes no screen-to-adapter mapping at all. On a host reporting
        /// one adapter the window is necessarily on it, which is what makes the
        /// substitution safe rather than merely convenient — and it is written
        /// down because a multi-adapter host would need the real test and
        /// `anySuitableDevice` would stop being the only thing that matters.
        private func addDevices(
            _ anySuitableDevice: Bool,
            _ foundDevices: CNAList<Microsoft.Xna.Framework.GraphicsDeviceInformation>
        ) throws {
            // XNA reads `game.Window.Handle`; the manager holds the game
            // weakly, and a window exists only inside a callback.
            let handle = game?.Window?.Handle ?? 0
            guard let adapters = Microsoft.Xna.Framework.Graphics
                    .GraphicsAdapter.Adapters else { return }
            for index in 0 ..< adapters.Count {
                let adapter = try adapters.Item(index)
                guard adapter.IsProfileSupported(GraphicsProfile) else { continue }

                let base = Microsoft.Xna.Framework.GraphicsDeviceInformation()
                try base.SetAdapter(adapter)
                base.GraphicsProfile = GraphicsProfile
                base.PresentationParameters.DeviceWindowHandle = handle
                base.PresentationParameters.MultiSampleCount = 0
                base.PresentationParameters.IsFullScreen = IsFullScreen
                base.PresentationParameters.PresentationInterval =
                    SynchronizeWithVerticalRetrace ? .One : .Immediate

                if let current = adapter.CurrentDisplayMode {
                    try addDevice(adapter, current, base, foundDevices)
                }
                guard IsFullScreen, let modes = adapter.SupportedDisplayModes else {
                    continue
                }
                let cursor = modes.GetEnumerator()
                while let mode = try cursor.Next() {
                    guard mode.Width >= 640, mode.Height >= 480 else { continue }
                    try addDevice(adapter, mode, base, foundDevices)
                }
            }
        }

        /// The enumeration, reachable from the test suite.
        ///
        /// `AddDevices` is private in XNA and private here; `FindBestDevice`
        /// is the only public way in, and it ranks and picks before a test can
        /// see what was enumerated. This exposes the list itself to
        /// `@testable`, which is how the duplicate test above is observed at
        /// all.
        internal func testOnlyAddDevices(
            _ anySuitableDevice: Bool,
            _ foundDevices: CNAList<Microsoft.Xna.Framework.GraphicsDeviceInformation>
        ) throws {
            try addDevices(anySuitableDevice, foundDevices)
        }

        /// `AddDevices(GraphicsAdapter, DisplayMode, GraphicsDeviceInformation,
        /// List<GraphicsDeviceInformation>)`, 228 bytes.
        ///
        /// Clones the base, sizes the back buffer, negotiates a format, writes
        /// the three negotiated values back, and adds the candidate **only if
        /// the list holds no equal one** — which is what makes
        /// `GraphicsDeviceInformation.Equals` load-bearing rather than
        /// decorative.
        ///
        /// The negotiation's own return is discarded, because XNA pops it:
        /// whether the request was met exactly decides nothing, the three
        /// chosen values do.
        private func addDevice(
            _ adapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter,
            _ mode: Microsoft.Xna.Framework.Graphics.DisplayMode,
            _ baseDeviceInfo: Microsoft.Xna.Framework.GraphicsDeviceInformation,
            _ foundDevices: CNAList<Microsoft.Xna.Framework.GraphicsDeviceInformation>
        ) throws {
            let info = baseDeviceInfo.Clone()
            if IsFullScreen {
                info.PresentationParameters.BackBufferWidth = mode.Width
                info.PresentationParameters.BackBufferHeight = mode.Height
            } else {
                info.PresentationParameters.BackBufferWidth = PreferredBackBufferWidth
                info.PresentationParameters.BackBufferHeight = PreferredBackBufferHeight
            }

            var format = mode.Format
            var depth = PreferredDepthStencilFormat
            var samples: Int32 = PreferMultiSampling ? 16 : 0
            _ = try adapter.QueryBackBufferFormat(
                info.GraphicsProfile, format: mode.Format,
                depthFormat: PreferredDepthStencilFormat,
                multiSampleCount: PreferMultiSampling ? 16 : 0,
                selectedFormat: &format, selectedDepthFormat: &depth,
                selectedMultiSampleCount: &samples)
            info.PresentationParameters.BackBufferFormat = format
            info.PresentationParameters.DepthStencilFormat = depth
            info.PresentationParameters.MultiSampleCount = samples

            for existing in 0 ..< foundDevices.Count
            where try foundDevices.Item(existing).Equals(info) {
                return
            }
            foundDevices.Add(info)
        }

        /// `Resources.NoCompatibleDevices`, a one-argument format naming the
        /// profile that found nothing.
        internal static func noCompatibleDevicesMessage(
            _ profile: Microsoft.Xna.Framework.Graphics.GraphicsProfile
        ) -> String {
            noCompatibleDevices.replacingOccurrences(of: "{0}", with: "\(profile)")
        }

        internal static let noCompatibleDevices =
            "Could not find a Direct3D device that supports the XNA Framework {0} profile."
            + "\r\n\r\nVerify that a suitable graphics device is installed."
            + "\r\n\r\nMake sure the desktop is not locked, and that no other application is running in full screen mode."
            + "\r\n\r\nAvoid running under Remote Desktop or as a Windows service."
            + "\r\n\r\nCheck the display properties to make sure hardware acceleration is set to Full."

        /// `Resources.NoCompatibleDevicesAfterRanking`.
        internal static let noCompatibleDevicesAfterRankingMessage =
            "The process of ranking devices removed all compatible devices."

        /// `protected virtual void RankDevices(List<GraphicsDeviceInformation>
        /// foundDevices)`.
        ///
        /// Eight bytes forwarding to `RankDevicesPlatform`, which is thirteen:
        /// `foundDevices.Sort(new GraphicsDeviceInformationComparer(this))`.
        /// All of the behaviour is that comparer, and it is transcribed in
        /// `GraphicsDeviceInformationComparer` rather than approximated —
        /// which device a consumer ends up with is decided by this order and
        /// nothing else.
        ///
        /// **Sorted in place through the list's own surface.** `List<T>.Sort`
        /// is one of the thirty-six members `CNAList` does not project, and
        /// growing that subset for one caller would be the wrong way round, so
        /// the elements are read out, ordered, and written back. The observable
        /// effect is identical: the caller's list is reordered, not replaced.
        ///
        /// Ties keep no guaranteed order, in both. `List<T>.Sort` is an
        /// unstable introsort and Swift's `sort` is unstable too, so two
        /// candidates the comparer calls equal may come back in either order —
        /// which is XNA's contract rather than a gap here.
        open func RankDevices(
            _ foundDevices:
                CNAList<Microsoft.Xna.Framework.GraphicsDeviceInformation>
        ) throws {
            let count = foundDevices.Count
            guard count > 1 else { return }
            var items: [Microsoft.Xna.Framework.GraphicsDeviceInformation] = []
            items.reserveCapacity(Int(count))
            for index in 0 ..< count {
                items.append(try foundDevices.Item(index))
            }
            let comparer = Microsoft.Xna.Framework
                .GraphicsDeviceInformationComparer(self)
            items.sort { comparer.Compare($0, $1) < 0 }
            for (offset, item) in items.enumerated() {
                try foundDevices.SetItem(Int32(offset), item)
            }
        }

                /// `protected virtual bool CanResetDevice(GraphicsDeviceInformation
        /// newDeviceInfo)`.
        ///
        /// **Twenty-three bytes, and one comparison.** The whole rule is that
        /// the candidate's `GraphicsProfile` equals the current device's:
        ///
        /// ```text
        /// ldfld device; callvirt get_GraphicsProfile
        /// ldarg.1;      callvirt get_GraphicsProfile
        /// ceq; ret
        /// ```
        ///
        /// Nothing about back-buffer size, format or full screen enters it — a
        /// device can be reset into a different resolution but not into a
        /// different profile. That is narrower than the name suggests and is
        /// reproduced rather than widened.
        ///
        /// It is `throws` where XNA's is not, because reading the device's
        /// profile crosses the runtime boundary here; a manager with no device
        /// yet cannot answer at all, which XNA expresses as a
        /// `NullReferenceException` from `ldfld device`.
        open func CanResetDevice(
            _ newDeviceInfo: Microsoft.Xna.Framework.GraphicsDeviceInformation
        ) throws -> Bool {
            guard let device = GraphicsDevice else {
                throw CNANullReferenceException()
            }
            return device.GraphicsProfile == newDeviceInfo.GraphicsProfile
        }

                /// `protected virtual void OnDeviceDisposing(object sender, EventArgs args)`.
        open func OnDeviceDisposing(_ sender: Any?, args: CNAEventArgs) throws {
            try deviceDisposingSource.Raise(sender, args: args)
        }

        /// `protected virtual void OnDeviceReset(object sender, EventArgs args)`.
        open func OnDeviceReset(_ sender: Any?, args: CNAEventArgs) throws {
            try deviceResetSource.Raise(sender, args: args)
        }

        /// `protected virtual void OnDeviceResetting(object sender, EventArgs args)`.
        open func OnDeviceResetting(_ sender: Any?, args: CNAEventArgs) throws {
            try deviceResettingSource.Raise(sender, args: args)
        }

        // ------------------------------------------------------------------
        // The preferences.
        //
        // XNA keeps every one of these in a managed field and applies them at
        // `ChangeDevice`, which is what `ApplyChanges`, `ToggleFullScreen` and
        // `CreateDevice` all reach. That is not an implementation detail to
        // route around: it is the reason every getter is a bare `ldfld` and
        // therefore `IL_NO_FAILURE_PATH`. Reading them back from CNA would
        // make each one fallible and contradict the pinned verdict, the same
        // wall `GraphicsDevice.PresentationParameters` hit in Foundation 48.
        //
        // So the managed field is the value, and CNA is told at the same
        // moments XNA tells Direct3D. CNA's own header agrees with the
        // division: "Every preference route here records a request;
        // cna_graphics_device_manager_apply_changes is what acts on it."
        //
        // The two sides start in the same place, which is measured rather than
        // assumed. `build-probe/f51_manager_prefs.c` reads CNA's manager
        // before anything is applied and gets Reach, 800x480, Color, Depth24,
        // windowed, no multisampling, vsync on, orientation Default -- every
        // one of the eight the pinned `.ctor` sets, including the `ldc.i4.1`
        // that makes `SynchronizeWithVerticalRetrace` true.

        /// `graphicsProfile`. CLR default, which is `GraphicsProfile.Reach`.
        private var graphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile = .Reach

        /// `backBufferFormat`. CLR default, which is `SurfaceFormat.Color`.
        private var backBufferFormat: Microsoft.Xna.Framework.Graphics.SurfaceFormat = .Color

        /// `backBufferWidth`, assigned `DefaultBackBufferWidth` by the `.ctor`.
        private var backBufferWidth: Int32 = GraphicsDeviceManager.DefaultBackBufferWidth

        /// `backBufferHeight`, assigned `DefaultBackBufferHeight`.
        private var backBufferHeight: Int32 = GraphicsDeviceManager.DefaultBackBufferHeight

        /// `depthStencilFormat`, assigned `ldc.i4.2` — `DepthFormat.Depth24`.
        /// Not the CLR default, which would be `DepthFormat.None`.
        private var depthStencilFormat: Microsoft.Xna.Framework.Graphics.DepthFormat = .Depth24

        /// `isFullScreen`. CLR default false.
        private var isFullScreenPreference = false

        /// `preferMultiSampling`. CLR default false.
        private var preferMultiSamplingPreference = false

        /// `synchronizeWithVerticalRetrace`, assigned `ldc.i4.1` — true. It is
        /// the `.ctor`'s very first instruction.
        private var synchronizeWithVerticalRetracePreference = true

        /// `supportedOrientations`. CLR default, the empty `DisplayOrientation`.
        private var supportedOrientationsPreference: Microsoft.Xna.Framework.DisplayOrientation = []

        /// `isDeviceDirty`, set by every setter and read by `ApplyChanges`.
        private var isDeviceDirty = false

        /// `useResizedBackBuffer`, which only the two dimension setters clear.
        /// Nothing reads it yet: `GameWindow` is not projected, and XNA's own
        /// readers of this field are all inside `ChangeDevice`'s window
        /// negotiation. It is stored because the setters store it, and its
        /// absence of readers is recorded rather than hidden by leaving it out.
        private var useResizedBackBuffer = false

        /// `GraphicsDeviceManager.GraphicsProfile`.
        ///
        /// Every setter below is `stfld` followed by `isDeviceDirty = true`,
        /// and every getter is the matching `ldfld`. The setters are
        /// infallible, so they project as Swift `set` accessors — the two
        /// exceptions are `PreferredBackBufferWidth` and `Height`, whose CLR
        /// setters throw and therefore become writer methods.
        public var GraphicsProfile: Microsoft.Xna.Framework.Graphics.GraphicsProfile {
            get { graphicsProfile }
            set { graphicsProfile = newValue; isDeviceDirty = true }
        }

        /// `GraphicsDeviceManager.PreferredBackBufferFormat`.
        public var PreferredBackBufferFormat: Microsoft.Xna.Framework.Graphics.SurfaceFormat {
            get { backBufferFormat }
            set { backBufferFormat = newValue; isDeviceDirty = true }
        }

        /// `GraphicsDeviceManager.PreferredDepthStencilFormat`.
        public var PreferredDepthStencilFormat: Microsoft.Xna.Framework.Graphics.DepthFormat {
            get { depthStencilFormat }
            set { depthStencilFormat = newValue; isDeviceDirty = true }
        }

        /// `GraphicsDeviceManager.IsFullScreen`.
        public var IsFullScreen: Bool {
            get { isFullScreenPreference }
            set { isFullScreenPreference = newValue; isDeviceDirty = true }
        }

        /// `GraphicsDeviceManager.PreferMultiSampling`.
        public var PreferMultiSampling: Bool {
            get { preferMultiSamplingPreference }
            set { preferMultiSamplingPreference = newValue; isDeviceDirty = true }
        }

        /// `GraphicsDeviceManager.SynchronizeWithVerticalRetrace`.
        public var SynchronizeWithVerticalRetrace: Bool {
            get { synchronizeWithVerticalRetracePreference }
            set { synchronizeWithVerticalRetracePreference = newValue; isDeviceDirty = true }
        }

        /// `GraphicsDeviceManager.SupportedOrientations`.
        public var SupportedOrientations: Microsoft.Xna.Framework.DisplayOrientation {
            get { supportedOrientationsPreference }
            set { supportedOrientationsPreference = newValue; isDeviceDirty = true }
        }

        /// `GraphicsDeviceManager.PreferredBackBufferWidth`.
        ///
        /// The reader only; the CLR setter is fallible and Swift has no
        /// throwing setter, so it is `SetPreferredBackBufferWidth` below.
        public var PreferredBackBufferWidth: Int32 { backBufferWidth }

        /// `GraphicsDeviceManager.set_PreferredBackBufferWidth(Int32 value)`.
        ///
        /// ```text
        /// if (value <= 0)
        ///     throw new ArgumentOutOfRangeException(
        ///         "value", Resources.BackBufferDimMustBePositive);
        /// backBufferWidth = value;
        /// useResizedBackBuffer = false;
        /// isDeviceDirty = true;
        /// ```
        ///
        /// `bgt` against zero, so zero is rejected along with every negative.
        /// CNA would have taken either: its own setter "records whatever it is
        /// given, including a value that no adapter can present", and
        /// `build-probe/f51_manager_prefs.c` watched it accept `-5`. The
        /// validation is XNA's and belongs here.
        public func SetPreferredBackBufferWidth(_ value: Int32) throws {
            guard value > 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "value",
                    message: GraphicsDeviceManager.backBufferDimMustBePositiveMessage)
            }
            backBufferWidth = value
            useResizedBackBuffer = false
            isDeviceDirty = true
        }

        /// `GraphicsDeviceManager.PreferredBackBufferHeight`.
        public var PreferredBackBufferHeight: Int32 { backBufferHeight }

        /// `GraphicsDeviceManager.set_PreferredBackBufferHeight(Int32 value)`,
        /// which is `set_PreferredBackBufferWidth` with one field changed —
        /// including the shared `"value"` parameter name and the shared
        /// message.
        public func SetPreferredBackBufferHeight(_ value: Int32) throws {
            guard value > 0 else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "value",
                    message: GraphicsDeviceManager.backBufferDimMustBePositiveMessage)
            }
            backBufferHeight = value
            useResizedBackBuffer = false
            isDeviceDirty = true
        }

        /// The exact `BackBufferDimMustBePositive` message, read out of
        /// `Microsoft.Xna.Framework.Game.dll`'s own resource table. It names
        /// both dimensions whichever setter raised it.
        internal static let backBufferDimMustBePositiveMessage =
            "BackBufferWidth and BackBufferHeight must be greater than zero."

        /// `GraphicsDeviceManager.ToggleFullScreen()`.
        ///
        /// ```text
        /// this.IsFullScreen = !this.IsFullScreen;
        /// this.ChangeDevice(false);
        /// ```
        ///
        /// Written as XNA writes it — through the property, then a device
        /// change — rather than through CNA's own
        /// `cna_graphics_device_manager_toggle_full_screen`, which stays
        /// unbound. The two reach the same state: the probe toggles natively
        /// and then set-and-applies to the same values, and CNA reports the
        /// same `is_full_screen` either way. Going through the property is
        /// what keeps the managed field, which every getter reads, correct.
        public func ToggleFullScreen() throws {
            IsFullScreen = !IsFullScreen
            try changeDevice()
        }

        /// XNA's `ChangeDevice`, reduced to what this binding owns: hand CNA
        /// the recorded preferences and let it apply them.
        ///
        /// XNA's own body enumerates adapters, ranks candidate device
        /// configurations, and creates a Direct3D device. CNA does all of that
        /// and is not Direct3D, which is why the seven messages that body
        /// raises are recorded as `native-owned` in
        /// `tools/api_compat/recorded-message-absences.json` rather than
        /// invented here.
        private func changeDevice() throws {
            let handle = try storage.validatedHandle("GraphicsDeviceManager.ChangeDevice")
            try pushPreferences(handle: handle)
            try storage.runtime.functions.check(
                storage.runtime.functions.graphicsManagerApplyChanges(handle),
                operation: "cna_graphics_device_manager_apply_changes")
            isDeviceDirty = false
        }

        /// The nine recorded preferences, handed to CNA in one place so that
        /// `CreateDevice` and `ChangeDevice` cannot drift apart.
        private func pushPreferences(handle: UInt64) throws {
            let functions = storage.runtime.functions
            try functions.check(
                functions.graphicsManagerSetGraphicsProfile(
                    handle, UInt32(bitPattern: graphicsProfile.rawValue)),
                operation: "cna_graphics_device_manager_set_graphics_profile")
            try functions.check(
                functions.graphicsManagerSetPreferredBackBufferFormat(
                    handle, UInt32(bitPattern: backBufferFormat.rawValue)),
                operation: "cna_graphics_device_manager_set_preferred_back_buffer_format")
            try functions.check(
                functions.graphicsManagerSetPreferredDepthStencilFormat(
                    handle, UInt32(bitPattern: depthStencilFormat.rawValue)),
                operation: "cna_graphics_device_manager_set_preferred_depth_stencil_format")
            try functions.check(
                functions.graphicsManagerSetPreferredBackBufferWidth(handle, backBufferWidth),
                operation: "cna_graphics_device_manager_set_preferred_back_buffer_width")
            try functions.check(
                functions.graphicsManagerSetPreferredBackBufferHeight(handle, backBufferHeight),
                operation: "cna_graphics_device_manager_set_preferred_back_buffer_height")
            try functions.check(
                functions.graphicsManagerSetIsFullScreen(handle, isFullScreenPreference ? 1 : 0),
                operation: "cna_graphics_device_manager_set_is_full_screen")
            try functions.check(
                functions.graphicsManagerSetPreferMultiSampling(
                    handle, preferMultiSamplingPreference ? 1 : 0),
                operation: "cna_graphics_device_manager_set_prefer_multi_sampling")
            try functions.check(
                functions.graphicsManagerSetSynchronizeWithVerticalRetrace(
                    handle, synchronizeWithVerticalRetracePreference ? 1 : 0),
                operation: "cna_graphics_device_manager_set_synchronize_with_vertical_retrace")
            try functions.check(
                functions.graphicsManagerSetSupportedOrientations(
                    handle, UInt32(bitPattern: supportedOrientationsPreference.rawValue)),
                operation: "cna_graphics_device_manager_set_supported_orientations")
        }

        /// `IGraphicsDeviceManager.CreateDevice()`.
        ///
        /// `Game.RunGame` calls this on the registered manager before
        /// `Initialize`, which is what makes a device exist at all.
        public func CreateDevice() throws {
            let handle = try storage.validatedHandle("GraphicsDeviceManager.CreateDevice")
            // XNA's `IGraphicsDeviceManager.CreateDevice` reaches
            // `ChangeDevice(true)`, so the preferences recorded before the
            // first device exists are the ones it is created with. Pushing
            // them here is what makes a game that sets
            // `PreferredBackBufferWidth` in its constructor get that width.
            try pushPreferences(handle: handle)
            try storage.runtime.functions.check(
                storage.runtime.functions.graphicsManagerCreateDevice(handle),
                operation: "cna_graphics_device_manager_create_device"
            )
            isDeviceDirty = false
        }

        /// `IGraphicsDeviceManager.BeginDraw()`.
        public func BeginDraw() throws -> Bool {
            let handle = try storage.validatedHandle("GraphicsDeviceManager.BeginDraw")
            var shouldDraw: UInt8 = 0
            try storage.runtime.functions.check(
                storage.runtime.functions.graphicsManagerBeginDraw(handle, &shouldDraw),
                operation: "cna_graphics_device_manager_begin_draw"
            )
            return shouldDraw != 0
        }

        /// `IGraphicsDeviceManager.EndDraw()`.
        public func EndDraw() throws {
            let handle = try storage.validatedHandle("GraphicsDeviceManager.EndDraw")
            try storage.runtime.functions.check(
                storage.runtime.functions.graphicsManagerEndDraw(handle),
                operation: "cna_graphics_device_manager_end_draw"
            )
        }

        /// `GraphicsDeviceManager.ApplyChanges()`.
        ///
        /// ```text
        /// if (device != null && !isDeviceDirty) return;
        /// ChangeDevice(false);
        /// ```
        ///
        /// Twenty-five bytes, and the first fourteen are the short-circuit
        /// this projection did not have: with a device already made and no
        /// preference touched since, `ApplyChanges` does **nothing**. It used
        /// to call CNA unconditionally, which is a device reconfiguration on
        /// every call.
        ///
        /// `device != null` is `GraphicsDevice != nil` here, which is only
        /// true inside a lifecycle callback — so outside one the branch falls
        /// through to the device change, exactly as it does in XNA before the
        /// device exists.
        public func ApplyChanges() throws {
            if GraphicsDevice != nil, !isDeviceDirty { return }
            try changeDevice()
        }

        /// `IDisposable.Dispose()`, which is `Dispose(true)` and then
        /// `GC.SuppressFinalize(this)`. There is no finalizer to suppress in
        /// Swift, so the second instruction has no counterpart; `deinit` is
        /// the finalizer's projection and the idempotence below is what stands
        /// in for the suppression.
        public func Dispose() throws {
            try Dispose(true)
        }

        /// `protected virtual void Dispose(bool disposing)`.
        ///
        /// ```text
        /// if (!disposing) return;
        /// if (game != null) {
        ///     if (game.Services.GetService(IGraphicsDeviceService) == this)
        ///         game.Services.RemoveService(IGraphicsDeviceService);
        ///     game.Window.ClientSizeChanged      -= GameWindowClientSizeChanged;
        ///     game.Window.ScreenDeviceNameChanged -= GameWindowScreenDeviceNameChanged;
        ///     game.Window.OrientationChanged     -= GameWindowOrientationChanged;
        /// }
        /// if (device != null) { device.Dispose(); device = null; }
        /// if (Disposed != null) Disposed(this, EventArgs.Empty);
        /// ```
        ///
        /// Three things about that body are worth stating rather than leaving
        /// to be inferred.
        ///
        /// It removes `IGraphicsDeviceService` and **not**
        /// `IGraphicsDeviceManager`, and only when the registered service is
        /// this manager — a second manager cannot unregister the first.
        ///
        /// The three window unsubscriptions have no counterpart: `GameWindow`
        /// is not projected, so there was never a subscription to remove. The
        /// constructor records the same absence at the other end.
        ///
        /// `device.Dispose()` has none either. XNA's manager owns a device it
        /// created; CNA owns this one, hands it out per callback, and destroys
        /// it with the game. Disposing the facade would be disposing a
        /// borrowed token. The native manager's own destruction is what
        /// `storage.dispose` performs, and that is the honest counterpart.
        ///
        /// `Disposed` is raised last, after everything else, with `this` as
        /// the sender — `ldarg.0`, unlike the four device raisers above.
        open func Dispose(_ disposing: Bool) throws {
            guard disposing else { return }
            if let game {
                if (game.Services.GetService(
                        Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService.self)
                    as AnyObject?) === self {
                    game.Services.RemoveService(
                        Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService.self)
                }
            }
            releaseManagerEventSubscriptions()
            try storage.dispose(operation: "GraphicsDeviceManager.Dispose")
            try disposedSource.Raise(self, args: CNAEventArgs.Empty)
        }

        internal func nativeManagerEventFired(_ event: UInt32) {
            do {
                switch event {
                // Through the virtual raisers, not around them. XNA's own
                // device path calls `OnDeviceCreated` and its three
                // neighbours rather than touching the delegate fields, so a
                // subclass that overrides one sees the device events it
                // overrode for.
                case GraphicsDeviceManager.eventDeviceCreated:
                    try OnDeviceCreated(self, args: CNAEventArgs.Empty)
                case GraphicsDeviceManager.eventDeviceDisposing:
                    try OnDeviceDisposing(self, args: CNAEventArgs.Empty)
                case GraphicsDeviceManager.eventDeviceReset:
                    try OnDeviceReset(self, args: CNAEventArgs.Empty)
                case GraphicsDeviceManager.eventDeviceResetting:
                    try OnDeviceResetting(self, args: CNAEventArgs.Empty)
                default:
                    // CNA_GRAPHICS_DEVICE_MANAGER_EVENT_DISPOSED has no
                    // IGraphicsDeviceService counterpart and is deliberately
                    // not mapped onto one.
                    break
                }
            } catch {
                storage.runtime.storeCallbackError(error)
            }
        }

        internal static let eventDeviceCreated: UInt32 = 1
        internal static let eventDeviceDisposing: UInt32 = 2
        internal static let eventDeviceReset: UInt32 = 3
        internal static let eventDeviceResetting: UInt32 = 4

        /// The native manager handle, for the tests that check what CNA holds
        /// against what the managed fields say. Internal, and used only to
        /// observe: nothing in the projection reads a preference back.
        internal func nativeHandleForTests() throws -> UInt64 {
            try storage.validatedHandle("GraphicsDeviceManager test observation")
        }

        /// How many manager subscriptions are live, for the test that asserts
        /// they are real and are released.
        internal var managerEventRegistrationCountForTests: Int {
            eventRegistrations.count
        }

        private func subscribeToManagerEvents(handle: UInt64) {
            for event in [GraphicsDeviceManager.eventDeviceCreated,
                          GraphicsDeviceManager.eventDeviceDisposing,
                          GraphicsDeviceManager.eventDeviceReset,
                          GraphicsDeviceManager.eventDeviceResetting] {
                let box = Unmanaged.passRetained(
                    GraphicsDeviceManagerEventBox(manager: self, event: event))
                var registration: UInt64 = 0
                let result = storage.runtime.functions.graphicsManagerSubscribe(
                    handle, event, graphicsDeviceManagerEventCallback,
                    box.toOpaque(), &registration)
                if result == 0 {
                    eventRegistrations[event] = registration
                    eventBoxes.append(box)
                } else {
                    box.release()
                }
            }
        }

        private func releaseManagerEventSubscriptions() {
            for registration in eventRegistrations.values {
                _ = storage.runtime.functions.gameUnsubscribe(registration)
            }
            eventRegistrations.removeAll()
            for box in eventBoxes { box.release() }
            eventBoxes.removeAll()
        }

        internal var runtimeObjectIsDisposed: Bool { storage.isDisposed }
        internal func disposeFromParent() throws { try Dispose() }
        deinit { releaseManagerEventSubscriptions() }
    }
}

/// The rooted context CNA's manager-event subscriptions carry.
internal final class GraphicsDeviceManagerEventBox {
    weak var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    let event: UInt32
    init(manager: Microsoft.Xna.Framework.GraphicsDeviceManager, event: UInt32) {
        self.manager = manager
        self.event = event
    }
}

internal let graphicsDeviceManagerEventCallback: CNASwift_GameEventCallback = { context in
    guard let context else { return }
    let box = Unmanaged<GraphicsDeviceManagerEventBox>.fromOpaque(context)
        .takeUnretainedValue()
    box.manager?.nativeManagerEventFired(box.event)
}
