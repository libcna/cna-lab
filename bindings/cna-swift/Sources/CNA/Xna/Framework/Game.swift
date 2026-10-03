// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework {
    /// The XNA Game class projected with subclassable lifecycle methods. CNA's
    /// native loop owns frame scheduling and invokes every override through the
    /// reviewed C callback trampolines.
    open class Game {
        internal let runtime: RuntimeState
        private let callbackContext: CallbackContext
        private var disposed = false
        private var callbackFailureWasSurfaced = false

        // The managed mirrors of the host state XNA keeps in fields. See the
        // block by `IsActive` for why they are mirrors and not native reads.
        internal var mirroredIsMouseVisible = false
        internal var mirroredIsFixedTimeStep = true
        internal var mirroredTargetElapsedTime = Game.duration(fromTicks: 166_667)
        internal var mirroredInactiveSleepTime = Game.duration(fromTicks: 0)

        private let activatedSource = CNAEventSource<CNAEventArgs>()
        private let deactivatedSource = CNAEventSource<CNAEventArgs>()
        private let exitingSource = CNAEventSource<CNAEventArgs>()
        private let disposedSource = CNAEventSource<CNAEventArgs>()
        private var eventRegistrations: [UInt32: UInt64] = [:]
        private var hostEventBoxes: [Unmanaged<GameEventBox>] = []

        /// The exact `TargetElaspedCannotBeZero` message -- XNA's own spelling
        /// of "Elapsed" included -- read out of
        /// `Microsoft.Xna.Framework.Game.dll`'s resource table.
        internal static let targetElapsedCannotBeZeroMessage =
            "The target elapsed time must be greater than zero.  Specify a "
            + "non-zero positive value."

        /// The exact `InactiveSleepTimeCannotBeZero` message.
        internal static let inactiveSleepTimeCannotBeZeroMessage =
            "The inactive sleep time must be greater than or equal to zero.  "
            + "Specify zero or a positive value."

        /// A `System.TimeSpan` tick is 100 nanoseconds, and so is CNA's.
        internal static func duration(fromTicks ticks: Int64) -> Duration {
            Duration(
                secondsComponent: ticks / 10_000_000,
                attosecondsComponent: (ticks % 10_000_000) * 100_000_000_000)
        }

        internal static func ticks(from value: Duration) -> Int64 {
            let components = value.components
            return components.seconds * 10_000_000
                + components.attoseconds / 100_000_000_000
        }

        // ------------------------------------------------------------------
        // The managed component engine.
        //
        // `Game..ctor` allocates one `GameComponentCollection` and five
        // `List<T>`s and subscribes two private handlers to the collection's
        // events. Those handlers ARE the engine: `Components` on its own would
        // be a collection a consumer could add to and never see updated, which
        // is why Foundation 29 refused to ship it alone. All of it is managed
        // XNA behaviour -- the native host owns frame scheduling and nothing
        // else -- so all of it is projected here.
        //
        // The five lists are private state, not projections: `mscorlib`'s
        // `List<T>` is what the CLR uses for them, but nothing public exposes
        // them, so a Swift array is the same thing observed from outside.
        // ------------------------------------------------------------------

        private let gameComponents = GameComponentCollection()
        private var updateableComponents: [any IUpdateable] = []
        private var currentlyUpdatingComponents: [any IUpdateable] = []
        private var drawableComponents: [any IDrawable] = []
        private var currentlyDrawingComponents: [any IDrawable] = []
        private var notYetInitialized: [any IGameComponent] = []

        // `Game::inRun`. XNA sets it true between `BeginRun` and `EndRun`, and
        // `GameComponentAdded` branches on it: a component added before the
        // run is queued for initialization, one added during it is initialized
        // immediately. The CNA host issues both hooks, so the flag is real.
        private var inRun = false

        // Subscriptions the CLR expresses as `-=` on a delegate. Swift
        // closures have no identity, so the token the event returns is kept
        // beside the component it belongs to and matched back with the same
        // default equality every collection member uses.
        private var updateOrderSubscriptions:
            [(component: any IUpdateable, token: CNAEventSubscription)] = []
        private var drawOrderSubscriptions:
            [(component: any IDrawable, token: CNAEventSubscription)] = []

        /// `Game.LaunchParameters`.
        ///
        /// `Game..ctor` allocates exactly one and `get_LaunchParameters` is a
        /// bare field read, so this is the same object on every read and it
        /// carries this process's parsed arguments.
        public let LaunchParameters = Microsoft.Xna.Framework.LaunchParameters()

        /// `Game.Services`.
        ///
        /// `Game..ctor` allocates exactly one `GameServiceContainer` and
        /// `get_Services` is a bare field read, so this is the same object on
        /// every read. It is what `RunGame` asks for the
        /// `IGraphicsDeviceManager` before creating the device, and what
        /// `DrawableGameComponent` will ask for the graphics device service.
        ///
        /// Nothing registers anything into it here. XNA's own registration
        /// happens in `GraphicsDeviceManager..ctor`, which is a separate
        /// member; an empty container is what a `Game` with no manager has in
        /// XNA too.
        public let Services = GameServiceContainer()

        /// `Game.Components`.
        ///
        /// `get_Components` is a bare field read of the single collection
        /// `Game..ctor` allocates, so this is the same object on every read.
        /// Adding to it drives the engine below through the collection's own
        /// events, exactly as it does in XNA.
        public var Components: GameComponentCollection { gameComponents }

        public init() throws {
            let functions = try NativeFunctions.load()
            let runtime = RuntimeState(functions: functions)
            self.runtime = runtime
            callbackContext = CallbackContext(runtime: runtime)

            var callbacks = CNASwift_GameCallbacks()
            callbacks.struct_size = UInt32(MemoryLayout<CNASwift_GameCallbacks>.size)
            callbacks.struct_version = 1
            callbacks.load_content = gameLoadContentCallback
            callbacks.update = gameUpdateCallback
            callbacks.draw = gameDrawCallback
            callbacks.unload_content = gameUnloadContentCallback
            callbacks.exiting = gameExitingCallback
            callbacks.context = callbackContext.pointer

            var handle: UInt64 = 0
            let createResult = withUnsafePointer(to: &callbacks) { callbacksPointer -> UInt32 in
                var createInfo = CNASwift_GameCreateInfo()
                createInfo.struct_size = UInt32(MemoryLayout<CNASwift_GameCreateInfo>.size)
                createInfo.struct_version = 1
                createInfo.is_fixed_time_step = 1
                createInfo.target_elapsed_time_ticks = 166_667
                createInfo.window_title = CNASwift_StringView(data: nil, byte_length: 0)
                createInfo.callbacks = callbacksPointer
                return withUnsafePointer(to: &createInfo) {
                    functions.gameCreate($0, &handle)
                }
            }
            do {
                try functions.check(createResult, operation: "cna_game_create")
            } catch {
                callbackContext.releaseAfterNativeStopsCalling()
                throw error
            }
            runtime.installGameHandle(handle)

            var hooks = CNASwift_GameFrameHooks()
            hooks.struct_size = UInt32(MemoryLayout<CNASwift_GameFrameHooks>.size)
            hooks.struct_version = 1
            hooks.initialize = gameInitializeCallback
            hooks.begin_run = gameBeginRunCallback
            hooks.end_run = gameEndRunCallback
            hooks.begin_draw = gameBeginDrawCallback
            hooks.end_draw = gameEndDrawCallback
            hooks.context = callbackContext.pointer
            let hookResult = withUnsafePointer(to: &hooks) {
                functions.gameSetFrameHooks(handle, $0)
            }
            do {
                try functions.check(hookResult, operation: "cna_game_set_frame_hooks_ext")
            } catch {
                _ = functions.gameDestroy(handle)
                runtime.invalidateAfterNativeShutdown()
                callbackContext.releaseAfterNativeStopsCalling()
                throw error
            }
            runtime.game = self

            // Seed the managed mirrors from the host, and subscribe to the
            // four host events. The seeds are what `Game..ctor` sets in XNA:
            // the create info carried the same values, so this reads back what
            // the host actually took rather than repeating what was sent.
            var active: UInt8 = 0
            if functions.gameGetIsActive(handle, &active) == 0 {
                IsActive = active != 0
            }
            var mouseVisible: UInt8 = 0
            if functions.gameGetIsMouseVisible(handle, &mouseVisible) == 0 {
                mirroredIsMouseVisible = mouseVisible != 0
            }
            var fixedStep: UInt8 = 0
            if functions.gameGetIsFixedTimeStep(handle, &fixedStep) == 0 {
                mirroredIsFixedTimeStep = fixedStep != 0
            }
            var targetTicks: Int64 = 0
            if functions.gameGetTargetElapsedTimeTicks(handle, &targetTicks) == 0 {
                mirroredTargetElapsedTime = Game.duration(fromTicks: targetTicks)
            }
            var sleepTicks: Int64 = 0
            if functions.gameGetInactiveSleepTimeTicks(handle, &sleepTicks) == 0 {
                mirroredInactiveSleepTime = Game.duration(fromTicks: sleepTicks)
            }
            subscribeToHostEvents(handle: handle)

            // `Game..ctor` subscribes these two immediately after allocating
            // the collection. `self` is captured weakly: the CLR delegate
            // holds a strong reference and its cycle is a garbage collector's
            // problem, while here the collection is owned BY the game, so a
            // strong capture would keep every game alive forever.
            _ = gameComponents.ComponentAdded.Add { [weak self] _, args in
                try self?.gameComponentAdded(args.GameComponent)
            }
            _ = gameComponents.ComponentRemoved.Add { [weak self] _, args in
                self?.gameComponentRemoved(args.GameComponent)
            }
        }

        deinit {
            guard !disposed, runtime.owner.isCurrent else { return }
            try? Dispose()
        }

        /// The first step of XNA's `RunGame`, before `Initialize`:
        ///
        /// ```text
        /// graphicsDeviceManager = Services.GetService(typeof(IGraphicsDeviceManager))
        ///                         as IGraphicsDeviceManager;
        /// if (graphicsDeviceManager != null) graphicsDeviceManager.CreateDevice();
        /// ```
        ///
        /// This is the moment a manager's recorded preferences -- a profile or
        /// a back-buffer size set in the game's constructor -- reach the
        /// device. Without it they reached CNA only on an explicit
        /// `ApplyChanges`, so a constructor-time `GraphicsProfile = .HiDef`
        /// ran the whole game under Reach.
        private func createDeviceThroughTheManager() throws {
            if let manager = Services.GetService(IGraphicsDeviceManager.self)
                as? IGraphicsDeviceManager {
                try manager.CreateDevice()
            }
        }

        public func Run() throws {
            let handle = try validatedHandle("Game.Run")
            try createDeviceThroughTheManager()
            runtime.clearCallbackError()
            callbackFailureWasSurfaced = false
            let result = runtime.functions.gameRun(handle)
            if let callbackError = runtime.takeCallbackError() {
                callbackFailureWasSurfaced = true
                throw callbackError
            }
            try runtime.functions.check(result, operation: "cna_game_run")
        }

        /// The native frame runs the whole `RunGame` sequence -- `Initialize`
        /// through `UnloadContent` -- so it starts where `Run` does.
        public func RunOneFrame() throws {
            let handle = try validatedHandle("Game.RunOneFrame")
            try createDeviceThroughTheManager()
            runtime.clearCallbackError()
            callbackFailureWasSurfaced = false
            let result = runtime.functions.gameRunOneFrame(handle)
            if let callbackError = runtime.takeCallbackError() {
                callbackFailureWasSurfaced = true
                throw callbackError
            }
            try runtime.functions.check(result, operation: "cna_game_run_one_frame")
        }

/// `Game.Window`.
        ///
        /// Infallible — `ldarg.0; ldfld window; ret` — so it reads a field
        /// here too. The window facade is built the first time it is asked
        /// for, inside a lifecycle callback, and kept for the runtime's
        /// generation: building it needs the game handle, and reading it must
        /// not throw.
        ///
        /// **Optional, and nil outside a callback.** XNA's is never null once
        /// the game is constructed; this cannot build one without a runtime,
        /// and a getter that may not throw has nowhere else to put that. The
        /// same divergence `GraphicsAdapter.Adapters` carries, for the same
        /// reason.
        public var Window: Microsoft.Xna.Framework.GameWindow? {
            if let cachedWindow, cachedWindowGeneration == currentGeneration {
                return cachedWindow
            }
            guard let runtime = try? RuntimeRegistry.current(),
                  let window = try? Microsoft.Xna.Framework.GameWindow(runtime: runtime)
            else { return nil }
            cachedWindow = window
            cachedWindowGeneration = runtime.generation
            return window
        }

        private var currentGeneration: UInt64? {
            (try? RuntimeRegistry.current())?.generation
        }

        private var cachedWindow: Microsoft.Xna.Framework.GameWindow?
        private var cachedWindowGeneration: UInt64?
        private var cachedContent: Microsoft.Xna.Framework.Content.ContentManager?
        private var cachedContentGeneration: UInt64?

        /// `Game.Content`.
        ///
        /// **Non-Optional, and it traps when there is no runtime.** The
        /// return's nullability is proven non-null -- XNA builds the manager in
        /// the constructor and the getter is a field read -- so Optional would
        /// invent a state XNA has no way to produce. The getter is also
        /// infallible, so it cannot report the missing runtime as an error.
        /// That leaves a trap, which is the same answer `DefaultAdapter`
        /// reached from the same two facts, and for the same reason: it is the
        /// honest Swift analogue of an unhandled CLR exception.
        ///
        /// The divergence is *when* the manager is built. XNA builds it in the
        /// constructor; here it needs a graphics device, which does not exist
        /// until the runtime is up, so it is built on first read inside the
        /// lifecycle instead.
        public var Content: Microsoft.Xna.Framework.Content.ContentManager {
            if let cachedContent, cachedContentGeneration == currentGeneration {
                return cachedContent
            }
            guard let runtime = try? RuntimeRegistry.current(),
                  let manager = try? Microsoft.Xna.Framework.Content.ContentManager(
                      serviceProvider: Services)
            else {
                preconditionFailure(
                    "Game.Content was read before the runtime existed, so no "
                    + "ContentManager could be built. XNA builds one in the "
                    + "Game constructor; this binding can only build one once a "
                    + "graphics device is available, because cna_content_manager_create "
                    + "takes one. Read it from inside a Game callback.")
            }
            cachedContent = manager
            cachedContentGeneration = runtime.generation
            return manager
        }

        /// `Game.set_Content`, whose IL throws `ArgumentNullException` directly
        /// -- so it projects as a throwing writer method rather than a settable
        /// property.
        ///
        /// **The refusal it exists for cannot happen here.** A writer takes the
        /// property's own type, and `Content` is proven non-null, so the value
        /// is non-Optional and Swift has no way to spell the null this setter
        /// tests for. The `throws` is kept because the setter is fallible and
        /// the projection is measured against that fact; what changed is that
        /// the type system now refuses what XNA had to refuse at run time.
        public func SetContent(
            _ value: Microsoft.Xna.Framework.Content.ContentManager
        ) throws {
            cachedContent = value
            cachedContentGeneration = currentGeneration
        }

        public func Exit() throws {
            let handle = try validatedHandle("Game.Exit")
            try runtime.functions.check(
                runtime.functions.gameRequestExit(handle),
                operation: "cna_game_request_exit"
            )
        }

        /// `Game.Dispose()`.
        ///
        /// `virtual final` in the metadata -- the sealed `IDisposable`
        /// implementation -- so `final` here. The body is `Dispose(true)`
        /// followed by `GC.SuppressFinalize(this)`, which Swift has no
        /// counterpart for.
        public final func Dispose() throws {
            try Dispose(true)
        }

        /// `protected virtual void Dispose(bool disposing)`.
        ///
        /// The one override point. XNA's body, when disposing, snapshots
        /// `Components` into an array, disposes every `IDisposable` in it,
        /// disposes `content`, unhooks the graphics device manager and then
        /// raises `Disposed`. The native teardown is this host's counterpart
        /// to the last of those, and it happens after the children are gone
        /// and before `Disposed` is raised -- so a handler sees a game whose
        /// children are already released, which is the order XNA produces.
        ///
        /// Swift has no `protected`, so this is public.
        open func Dispose(_ disposing: Bool) throws {
            guard disposing else { return }
            if disposed { return }
            try runtime.owner.validate("Game.Dispose")
            try runtime.disposeChildren()
            releaseHostEventSubscriptions()
            runtime.clearCallbackError()
            let handle = try validatedHandle("Game.Dispose")
            let result = runtime.functions.gameDestroy(handle)
            if result != 0 && result != 9 {
                try runtime.functions.check(result, operation: "cna_game_destroy")
            }
            disposed = true
            runtime.invalidateAfterNativeShutdown()
            callbackContext.releaseAfterNativeStopsCalling()
            try disposedSource.Raise(self, args: CNAEventArgs.Empty)
            if let callbackError = runtime.takeCallbackError() { throw callbackError }
            if result == 9 && !callbackFailureWasSurfaced {
                throw CNAError.nativeFailure(
                    operation: "cna_game_destroy callback",
                    result: result,
                    message: "native shutdown callback failed without a stored Swift Error"
                )
            }
            callbackFailureWasSurfaced = false
        }

        /// `Game.GraphicsDevice`.
        ///
        /// The IL resolves `IGraphicsDeviceService` out of `Services` -- caching
        /// it in a field on the first successful lookup -- raises
        /// `InvalidOperationException(NoGraphicsDeviceService)` when there is
        /// none, and otherwise returns `service.GraphicsDevice`, which is
        /// itself nullable. So the getter is fallible AND its result is
        /// Optional, and both halves are projected.
        ///
        /// Before Foundation 40 this borrowed the device straight from the
        /// host, which skipped the container entirely; a game with no
        /// `GraphicsDeviceManager` got a device where XNA raises.
        public var GraphicsDevice: Graphics.GraphicsDevice? {
            get throws {
                if graphicsDeviceService == nil {
                    graphicsDeviceService = Services.GetService(
                        Graphics.IGraphicsDeviceService.self)
                        as? Graphics.IGraphicsDeviceService
                }
                guard let graphicsDeviceService else {
                    throw CNAInvalidOperationException(
                        message: Game.noGraphicsDeviceServiceMessage)
                }
                return graphicsDeviceService.GraphicsDevice
            }
        }

        /// `Game.graphicsDeviceService`, cached on the first successful lookup
        /// exactly as the CLR field is.
        private var graphicsDeviceService: Graphics.IGraphicsDeviceService?

        /// The exact `NoGraphicsDeviceService` message, read out of
        /// `Microsoft.Xna.Framework.Game.dll`'s own resource table.
        internal static let noGraphicsDeviceServiceMessage =
            "This property requires a graphics device service in the game "
            + "service container."

        // ------------------------------------------------------------------
        // Timing and host state.
        //
        // Every getter in this block is `IL_NO_FAILURE_PATH` in the pinned
        // accessor verdicts, so none of them may throw -- and a native round
        // trip always can, on the owner thread or a stale generation. Each is
        // therefore a managed mirror, which is exactly what XNA has: a field.
        // The mirrors are seeded from the host at construction and updated
        // only when a write to the host actually succeeded, so the getter
        // reports what holds rather than what was asked for.
        // ------------------------------------------------------------------

        /// `Game.IsActive`.
        ///
        /// XNA reads `isActive` -- a field the host updates -- and ANDs it
        /// with `!Guide.IsVisible` when GamerServices is initialized.
        /// GamerServices is outside the selected profile and is never
        /// initialized, so the second half is constant true and the value is
        /// the field. Here the field is seeded from `cna_game_get_is_active`
        /// at construction and updated by CNA's own Activated/Deactivated
        /// notifications, which is the same mechanism.
        public private(set) var IsActive: Bool = false

        /// `Game.IsMouseVisible`.
        ///
        /// The CLR setter stores the field and then, **if the window exists**,
        /// forwards to `GameWindow.set_IsMouseVisible`. Both accessors are
        /// infallible, so the Swift setter cannot throw; it writes through to
        /// the host and leaves the mirror unchanged when the host refuses, so
        /// the getter never reports a state the host does not hold.
        public var IsMouseVisible: Bool {
            get { mirroredIsMouseVisible }
            set {
                guard let handle = try? validatedHandle("Game.IsMouseVisible") else { return }
                guard runtime.functions.gameSetIsMouseVisible(handle, newValue ? 1 : 0) == 0
                else { return }
                mirroredIsMouseVisible = newValue
            }
        }

        /// `Game.IsFixedTimeStep`.
        ///
        /// The CLR setter stores the field and calls nothing: in XNA that
        /// field *is* what `Game.Tick` reads. Here the host owns the loop, so
        /// the write has to reach it or the property would be inert -- a
        /// recorded divergence in mechanism and not in observable behaviour.
        public var IsFixedTimeStep: Bool {
            get { mirroredIsFixedTimeStep }
            set {
                guard let handle = try? validatedHandle("Game.IsFixedTimeStep") else { return }
                guard runtime.functions.gameSetIsFixedTimeStep(handle, newValue ? 1 : 0) == 0
                else { return }
                mirroredIsFixedTimeStep = newValue
            }
        }

        /// `Game.TargetElapsedTime` — the getter.
        public var TargetElapsedTime: Duration { mirroredTargetElapsedTime }

        /// `Game.TargetElapsedTime` — the setter.
        ///
        /// Projected as a writer method because the CLR setter is fallible:
        /// `value <= TimeSpan.Zero` raises
        /// `ArgumentOutOfRangeException("value", TargetElaspedCannotBeZero)`,
        /// and Swift has no throwing property setter. The comparison is
        /// `op_LessThanOrEqual`, so zero itself is refused.
        public func SetTargetElapsedTime(_ value: Duration) throws {
            guard value > .zero else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "value",
                    message: Game.targetElapsedCannotBeZeroMessage)
            }
            let handle = try validatedHandle("Game.TargetElapsedTime")
            try runtime.functions.check(
                runtime.functions.gameSetTargetElapsedTimeTicks(handle, Game.ticks(from: value)),
                operation: "cna_game_set_target_elapsed_time_ticks"
            )
            mirroredTargetElapsedTime = value
        }

        /// `Game.InactiveSleepTime` — the getter.
        public var InactiveSleepTime: Duration { mirroredInactiveSleepTime }

        /// `Game.InactiveSleepTime` — the setter.
        ///
        /// `value < TimeSpan.Zero` raises
        /// `ArgumentOutOfRangeException("value", InactiveSleepTimeCannotBeZero)`.
        /// The comparison is `op_LessThan`, so zero is **accepted** here where
        /// `TargetElapsedTime` refuses it -- the two messages read alike and
        /// the two conditions do not.
        public func SetInactiveSleepTime(_ value: Duration) throws {
            guard value >= .zero else {
                throw CNAArgumentOutOfRangeException(
                    paramName: "value",
                    message: Game.inactiveSleepTimeCannotBeZeroMessage)
            }
            let handle = try validatedHandle("Game.InactiveSleepTime")
            try runtime.functions.check(
                runtime.functions.gameSetInactiveSleepTimeTicks(handle, Game.ticks(from: value)),
                operation: "cna_game_set_inactive_sleep_time_ticks"
            )
            mirroredInactiveSleepTime = value
        }

        /// `Game.Tick()`.
        ///
        /// XNA runs the whole fixed/variable-step body here. The host owns
        /// that loop, so this is its single route; the sequence it produces is
        /// pinned by
        /// `NativeLifecycleTests.testHostGameTimeSequenceIsMeasuredNotAssumed`.
        public func Tick() throws {
            let handle = try validatedHandle("Game.Tick")
            try runtime.functions.check(
                runtime.functions.gameTick(handle), operation: "cna_game_tick")
            if let callbackError = runtime.takeCallbackError() { throw callbackError }
        }

        /// `Game.SuppressDraw()`.
        ///
        /// The CLR body is one field store that `Tick` reads. The host reads
        /// its own flag, so the call has to reach it.
        public func SuppressDraw() throws {
            let handle = try validatedHandle("Game.SuppressDraw")
            try runtime.functions.check(
                runtime.functions.gameSuppressDraw(handle),
                operation: "cna_game_suppress_draw")
        }

        /// `Game.ResetElapsedTime()`.
        ///
        /// The CLR body sets `forceElapsedTimeToZero`, clears
        /// `drawRunningSlowly` and pushes both running-slowly counters to
        /// `Int32.MaxValue` -- all state the host owns here.
        public func ResetElapsedTime() throws {
            let handle = try validatedHandle("Game.ResetElapsedTime")
            try runtime.functions.check(
                runtime.functions.gameResetElapsedTime(handle),
                operation: "cna_game_reset_elapsed_time")
        }

        /// `Game.ShowMissingRequirementMessage(Exception)`.
        ///
        /// The CLR body is `host != null ? host.ShowMissingRequirementMessage(e)
        /// : false`, and `GameHost`'s own base returns `false`. Only
        /// `WindowsGameHost` overrides it, with a `System.Windows.Forms`
        /// message box. CNA is the host here and shows no message, so `false`
        /// is this host's answer and not a stand-in for one -- a caller that
        /// gets `false` learns exactly what XNA tells it: the message was not
        /// shown, so rethrow.
        open func ShowMissingRequirementMessage(_ exception: CNAException) -> Bool {
            false
        }

        // ------------------------------------------------------------------
        // The four host events.
        //
        // CNA_GAME_EVENT_ACTIVATED, _DEACTIVATED, _EXITING and _DISPOSED are
        // real host notifications, subscribed at construction and released at
        // disposal. The callback carries only the context, so one box is
        // registered per event and each box carries the identity it was
        // registered for.
        //
        // The raise sites are XNA's own: a host notification calls the
        // corresponding `On…` method, which is what raises the event -- so an
        // override that does not call `super` suppresses the event exactly as
        // it does in XNA, and `Exiting` is raised from `OnExiting` and not
        // from the teardown path. That distinction matters: mapping a CNA
        // teardown notification straight onto `Exiting` would fire it where
        // XNA does not.
        // ------------------------------------------------------------------

        internal func nativeGameEventFired(_ event: UInt32) {
            do {
                switch event {
                case Game.eventActivated:
                    IsActive = true
                    try OnActivated(self, args: CNAEventArgs.Empty)
                case Game.eventDeactivated:
                    IsActive = false
                    try OnDeactivated(self, args: CNAEventArgs.Empty)
                case Game.eventExiting:
                    try OnExiting(self, args: CNAEventArgs.Empty)
                case Game.eventDisposed:
                    try disposedSource.Raise(self, args: CNAEventArgs.Empty)
                default:
                    break
                }
            } catch {
                runtime.storeCallbackError(error)
            }
        }

        /// How many host event subscriptions are live, for the test that
        /// asserts they are real and are released.
        internal var hostEventRegistrationCountForTests: Int { eventRegistrations.count }

        internal static let eventActivated: UInt32 = 0
        internal static let eventDeactivated: UInt32 = 1
        internal static let eventDisposed: UInt32 = 2
        internal static let eventExiting: UInt32 = 3

        private func subscribeToHostEvents(handle: UInt64) {
            for event in [Game.eventActivated, Game.eventDeactivated,
                          Game.eventExiting, Game.eventDisposed] {
                let box = Unmanaged.passRetained(GameEventBox(game: self, event: event))
                var registration: UInt64 = 0
                let result = runtime.functions.gameSubscribe(
                    handle, event, gameEventCallback, box.toOpaque(), &registration)
                if result == 0 {
                    eventRegistrations[event] = registration
                    hostEventBoxes.append(box)
                } else {
                    box.release()
                }
            }
        }

        internal func releaseHostEventSubscriptions() {
            for registration in eventRegistrations.values {
                _ = runtime.functions.gameUnsubscribe(registration)
            }
            eventRegistrations.removeAll()
            for box in hostEventBoxes { box.release() }
            hostEventBoxes.removeAll()
        }

        /// `Game.Activated`.
        public var Activated: CNAEvent<CNAEventArgs> { activatedSource.Event }

        /// `Game.Deactivated`.
        public var Deactivated: CNAEvent<CNAEventArgs> { deactivatedSource.Event }

        /// `Game.Exiting`.
        public var Exiting: CNAEvent<CNAEventArgs> { exitingSource.Event }

        /// `Game.Disposed`.
        public var Disposed: CNAEvent<CNAEventArgs> { disposedSource.Event }

        /// `protected virtual void OnActivated(object sender, EventArgs args)`.
        ///
        /// The IL raises the handler with **`this`** as the sender and the
        /// `args` parameter as the argument -- `ldarg.0` then `ldarg.2`. The
        /// `sender` parameter is declared and never used, which is XNA's own
        /// quirk and is reproduced rather than corrected.
        open func OnActivated(_ sender: Any?, args: CNAEventArgs) throws {
            try activatedSource.Raise(self, args: args)
        }

        /// `protected virtual void OnDeactivated(object sender, EventArgs args)`.
        /// The same shape, and the same ignored `sender`.
        open func OnDeactivated(_ sender: Any?, args: CNAEventArgs) throws {
            try deactivatedSource.Raise(self, args: args)
        }

        /// `protected virtual void Initialize()`.
        ///
        /// The IL is
        ///
        /// ```text
        /// HookDeviceEvents()
        /// while (notYetInitialized.Count != 0) {
        ///     notYetInitialized[0].Initialize()
        ///     notYetInitialized.RemoveAt(0)
        /// }
        /// if (graphicsDeviceService != null && GraphicsDevice != null)
        ///     LoadContent()
        /// ```
        ///
        /// Two things about the transcription:
        ///
        /// - the drain takes index **0** each time and removes it, so a
        ///   component whose own `Initialize` adds another component sees that
        ///   one initialized too, in the same pass. Iterating a snapshot would
        ///   not;
        /// - `LoadContent()` is **not** called here. The CNA host issues that
        ///   call as its own `load_content` callback, immediately after this
        ///   one -- measured, not assumed, by
        ///   `testNativeCallbackOrderIsMeasuredNotAssumed`. The two are the
        ///   same XNA lifecycle occurrence; calling it here as well would give
        ///   a consumer two invocations for one, and assigning it to the one
        ///   callback path is what keeps the invariant.
        ///
        /// `HookDeviceEvents` is absent because the device events it hooks are
        /// not projected; nothing is invented in its place. Two consequences
        /// of that absence are recorded rather than half-reproduced, because
        /// closing either needs `IGraphicsDeviceService`:
        ///
        /// - XNA calls `LoadContent` only `if (graphicsDeviceService != null
        ///   && GraphicsDevice != null)`. Here the host decides, so a game
        ///   with no `GraphicsDeviceManager` still receives `LoadContent`;
        /// - XNA's `LoadContent` fires from **inside** this base body, so an
        ///   override that does not call `super.Initialize()` never receives
        ///   it. Here it arrives on its own callback and so always does.
        ///
        /// Gating the callback on this body having run would fix the second
        /// and would also suppress the device-reset reload XNA issues through
        /// the very handlers `HookDeviceEvents` installs -- behaviour this
        /// host has not been measured for. A guard that trades a known
        /// divergence for an unmeasured one is not an improvement.
        open func Initialize() throws {
            while !notYetInitialized.isEmpty {
                let component = notYetInitialized[0]
                try component.Initialize()
                // Guard against a handler having already drained it.
                if !notYetInitialized.isEmpty {
                    notYetInitialized.remove(at: 0)
                }
            }
        }

        open func LoadContent() throws {}
        open func UnloadContent() throws {}

        /// `protected virtual void Update(GameTime gameTime)`.
        ///
        /// ```text
        /// for (i = 0; i < updateableComponents.Count; i++)
        ///     currentlyUpdatingComponents.Add(updateableComponents[i])
        /// for (j = 0; j < currentlyUpdatingComponents.Count; j++) {
        ///     var u = currentlyUpdatingComponents[j]
        ///     if (u.Enabled) u.Update(gameTime)
        /// }
        /// currentlyUpdatingComponents.Clear()
        /// ```
        ///
        /// The copy is what makes mutation during traversal safe: a component
        /// that adds or removes another during its own `Update` changes
        /// `updateableComponents`, not the list being walked, so the current
        /// pass runs over exactly the components that were present when it
        /// began. `Enabled` is read per component at the moment it is reached,
        /// so a component disabled by an earlier one in the same pass is
        /// skipped.
        open func Update(_ gameTime: GameTime) throws {
            for component in updateableComponents {
                currentlyUpdatingComponents.append(component)
            }
            var index = 0
            while index < currentlyUpdatingComponents.count {
                let component = currentlyUpdatingComponents[index]
                if component.Enabled { try component.Update(gameTime) }
                index += 1
            }
            currentlyUpdatingComponents.removeAll(keepingCapacity: true)
        }

        /// `protected virtual void Draw(GameTime gameTime)`.
        ///
        /// The same shape as `Update`, over `drawableComponents` and guarded
        /// by `Visible`.
        open func Draw(_ gameTime: GameTime) throws {
            for component in drawableComponents {
                currentlyDrawingComponents.append(component)
            }
            var index = 0
            while index < currentlyDrawingComponents.count {
                let component = currentlyDrawingComponents[index]
                if component.Visible { try component.Draw(gameTime) }
                index += 1
            }
            currentlyDrawingComponents.removeAll(keepingCapacity: true)
        }

        /// `protected virtual void BeginRun()`.
        ///
        /// XNA's own body is empty. `inRun` is set by `RunGame` immediately
        /// **before** this call and cleared immediately after `EndRun`, and
        /// the CNA host issues both hooks at the same points -- measured, not
        /// assumed. Setting the flag here rather than in an invented place is
        /// what makes `GameComponentAdded`'s `inRun` branch real.
        open func BeginRun() throws {
            inRun = true
        }

        /// `protected virtual void EndRun()`. XNA's own body is empty.
        open func EndRun() throws {
            inRun = false
        }
        open func BeginDraw() throws -> Bool { true }
        open func EndDraw() throws {}
        /// `protected virtual void OnExiting(object sender, EventArgs args)`.
        /// The same shape as the other two, and the same ignored `sender`.
        open func OnExiting(_ sender: Any?, args: CNAEventArgs) throws {
            try exitingSource.Raise(self, args: args)
        }

        // ------------------------------------------------------------------
        // `Game::GameComponentAdded` and `Game::GameComponentRemoved`.
        // ------------------------------------------------------------------

        /// ```text
        /// if (inRun) e.GameComponent.Initialize()
        /// else       notYetInitialized.Add(e.GameComponent)
        /// if (e.GameComponent is IUpdateable u) insert sorted, subscribe
        /// if (e.GameComponent is IDrawable d)   insert sorted, subscribe
        /// ```
        private func gameComponentAdded(
            _ component: any IGameComponent
        ) throws {
            if inRun {
                try component.Initialize()
            } else {
                notYetInitialized.append(component)
            }
            if let updateable = component as? any IUpdateable {
                insertUpdateable(updateable)
            }
            if let drawable = component as? any IDrawable {
                insertDrawable(drawable)
            }
        }

        /// ```text
        /// if (!inRun) notYetInitialized.Remove(e.GameComponent)
        /// if (u != null) { updateableComponents.Remove(u); u.UpdateOrderChanged -= h }
        /// if (d != null) { drawableComponents.Remove(d);   d.DrawOrderChanged  -= h }
        /// ```
        ///
        /// The `notYetInitialized` removal happens only when the game is not
        /// running, which is the same condition that put it there.
        private func gameComponentRemoved(_ component: any IGameComponent) {
            if !inRun {
                if let index = notYetInitialized.firstIndex(where: {
                    cnaDefaultEquals($0, component)
                }) {
                    notYetInitialized.remove(at: index)
                }
            }
            if let updateable = component as? any IUpdateable {
                removeUpdateable(updateable)
            }
            if let drawable = component as? any IDrawable {
                removeDrawable(drawable)
            }
        }

        /// The sorted insertion `GameComponentAdded` and
        /// `UpdateableUpdateOrderChanged` share.
        ///
        /// ```text
        /// i = updateableComponents.BinarySearch(u, UpdateOrderComparer.Default)
        /// if (i >= 0) return                       // already present
        /// i = ~i
        /// while (i < Count && list[i].UpdateOrder == u.UpdateOrder) i++
        /// updateableComponents.Insert(i, u)
        /// u.UpdateOrderChanged += UpdateableUpdateOrderChanged
        /// ```
        ///
        /// `UpdateOrderComparer.Compare` never returns 0 for two DISTINCT
        /// objects -- it returns 0 only when `x.Equals(y)` -- so the binary
        /// search finds a non-negative index only for a component that is
        /// already in the list, and the tie scan then places a new
        /// equal-ordered component **after** every existing one. That is what
        /// makes the ordering stable, and it is why a comparer that merely
        /// compared `UpdateOrder` would have been wrong.
        private func insertUpdateable(_ component: any IUpdateable) {
            guard !updateableComponents.contains(where: {
                cnaDefaultEquals($0, component)
            }) else { return }
            var index = updateableComponents.firstIndex {
                $0.UpdateOrder > component.UpdateOrder
            } ?? updateableComponents.count
            while index < updateableComponents.count,
                  updateableComponents[index].UpdateOrder == component.UpdateOrder {
                index += 1
            }
            updateableComponents.insert(component, at: index)
            let token = component.UpdateOrderChanged.Add { [weak self] sender, _ in
                guard let self, let changed = sender as? any IUpdateable else { return }
                self.removeUpdateable(changed, keepingSubscription: true)
                self.reinsertUpdateable(changed)
            }
            updateOrderSubscriptions.append((component, token))
        }

        private func insertDrawable(_ component: any IDrawable) {
            guard !drawableComponents.contains(where: {
                cnaDefaultEquals($0, component)
            }) else { return }
            var index = drawableComponents.firstIndex {
                $0.DrawOrder > component.DrawOrder
            } ?? drawableComponents.count
            while index < drawableComponents.count,
                  drawableComponents[index].DrawOrder == component.DrawOrder {
                index += 1
            }
            drawableComponents.insert(component, at: index)
            let token = component.DrawOrderChanged.Add { [weak self] sender, _ in
                guard let self, let changed = sender as? any IDrawable else { return }
                self.removeDrawable(changed, keepingSubscription: true)
                self.reinsertDrawable(changed)
            }
            drawOrderSubscriptions.append((component, token))
        }

        /// `Game::UpdateableUpdateOrderChanged` -- remove, then re-insert at
        /// the position the new order gives, WITHOUT resubscribing: the CLR
        /// handler does not touch the delegate.
        private func reinsertUpdateable(_ component: any IUpdateable) {
            var index = updateableComponents.firstIndex {
                $0.UpdateOrder > component.UpdateOrder
            } ?? updateableComponents.count
            while index < updateableComponents.count,
                  updateableComponents[index].UpdateOrder == component.UpdateOrder {
                index += 1
            }
            updateableComponents.insert(component, at: index)
        }

        private func reinsertDrawable(_ component: any IDrawable) {
            var index = drawableComponents.firstIndex {
                $0.DrawOrder > component.DrawOrder
            } ?? drawableComponents.count
            while index < drawableComponents.count,
                  drawableComponents[index].DrawOrder == component.DrawOrder {
                index += 1
            }
            drawableComponents.insert(component, at: index)
        }

        private func removeUpdateable(
            _ component: any IUpdateable, keepingSubscription: Bool = false
        ) {
            if let index = updateableComponents.firstIndex(where: {
                cnaDefaultEquals($0, component)
            }) {
                updateableComponents.remove(at: index)
            }
            guard !keepingSubscription else { return }
            if let index = updateOrderSubscriptions.firstIndex(where: {
                cnaDefaultEquals($0.component, component)
            }) {
                component.UpdateOrderChanged.Remove(
                    updateOrderSubscriptions[index].token)
                updateOrderSubscriptions.remove(at: index)
            }
        }

        private func removeDrawable(
            _ component: any IDrawable, keepingSubscription: Bool = false
        ) {
            if let index = drawableComponents.firstIndex(where: {
                cnaDefaultEquals($0, component)
            }) {
                drawableComponents.remove(at: index)
            }
            guard !keepingSubscription else { return }
            if let index = drawOrderSubscriptions.firstIndex(where: {
                cnaDefaultEquals($0.component, component)
            }) {
                component.DrawOrderChanged.Remove(
                    drawOrderSubscriptions[index].token)
                drawOrderSubscriptions.remove(at: index)
            }
        }

        private func validatedHandle(_ operation: String) throws -> UInt64 {
            guard !disposed, runtime.gameHandle != 0 else { throw CNAError.disposedObject("Game") }
            try runtime.validateGeneration(runtime.generation)
            try runtime.owner.validate(operation)
            return runtime.gameHandle
        }
    }
}

/// The rooted context CNA's game-event subscriptions carry.
///
/// One box serves all four events; the event identity is not passed to the
/// callback, so a separate box is registered per event and each carries the
/// identity it was registered for. The game is held **weakly**, so a
/// subscription cannot keep it alive.
internal final class GameEventBox {
    weak var game: Microsoft.Xna.Framework.Game?
    let event: UInt32
    init(game: Microsoft.Xna.Framework.Game, event: UInt32) {
        self.game = game
        self.event = event
    }
}

internal let gameEventCallback: CNASwift_GameEventCallback = { context in
    guard let context else { return }
    let box = Unmanaged<GameEventBox>.fromOpaque(context).takeUnretainedValue()
    box.game?.nativeGameEventFired(box.event)
}
