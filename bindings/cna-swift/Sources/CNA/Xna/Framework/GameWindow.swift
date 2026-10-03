// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework {

    /// `Microsoft.Xna.Framework.GameWindow`.
    ///
    /// **A snapshot, for the third time and the same reason.** All six of its
    /// accessors are pinned `IL_NO_FAILURE_PATH`, so not one Swift getter may
    /// throw, while every `cna_game_window_*` route needs the game handle and
    /// can fail. The values are read once when the window facade is built and
    /// every property afterwards reads a field — the shape `GraphicsAdapter`
    /// and `GraphicsDevice` were forced into before it.
    ///
    /// ## Nothing here opens a window
    ///
    /// This runs under the project's no-visible-window rule, and none of the
    /// bound routes creates one: they read a title, a client rectangle, an
    /// orientation and a native handle that the HEADLESS renderer has already
    /// decided. `cna_game_window_minimize_ext`, `restore_ext` and
    /// `set_is_borderless_ext` are **not bound** — XNA has no such members, and
    /// a route with no consuming member is not bound.
    public class GameWindow {

        internal let runtime: RuntimeState
        private var storedTitle: String
        private var storedScreenDeviceName: String
        private var storedHandle: Int
        private var storedClientBounds: Microsoft.Xna.Framework.Rectangle
        private var storedAllowUserResizing: Bool
        private var storedOrientation: Microsoft.Xna.Framework.DisplayOrientation

        private let screenDeviceNameChangedSource = CNAEventSource<CNAEventArgs>()
        private let clientSizeChangedSource = CNAEventSource<CNAEventArgs>()
        private let orientationChangedSource = CNAEventSource<CNAEventArgs>()

        internal init(runtime: RuntimeState) throws {
            self.runtime = runtime
            let handle = runtime.gameHandle
            let functions = runtime.functions

            storedTitle = try GameWindow.copyString(
                runtime, handle,
                functions.gameWindowGetTitleSize, functions.gameWindowCopyTitle,
                "cna_game_window_get_title_size", "cna_game_window_copy_title")
            storedScreenDeviceName = try GameWindow.copyString(
                runtime, handle,
                functions.gameWindowGetScreenDeviceNameSize,
                functions.gameWindowCopyScreenDeviceName,
                "cna_game_window_get_screen_device_name_size",
                "cna_game_window_copy_screen_device_name")

            var native: UInt64 = 0
            try functions.check(functions.gameWindowGetNativeHandle(handle, &native),
                                operation: "cna_game_window_get_native_handle_ext")
            storedHandle = Int(bitPattern: UInt(native))

            var bounds = CNASwift_Rectangle()
            try functions.check(functions.gameWindowGetClientBounds(handle, &bounds),
                                operation: "cna_game_window_get_client_bounds")
            storedClientBounds = Microsoft.Xna.Framework.Rectangle(
                bounds.x, bounds.y, bounds.width, bounds.height)

            var allowed: UInt8 = 0
            try functions.check(
                functions.gameWindowGetAllowUserResizing(handle, &allowed),
                operation: "cna_game_window_get_allow_user_resizing")
            storedAllowUserResizing = allowed != 0

            var orientation: UInt32 = 0
            try functions.check(
                functions.gameWindowGetCurrentOrientation(handle, &orientation),
                operation: "cna_game_window_get_current_orientation")
            storedOrientation = Microsoft.Xna.Framework.DisplayOrientation(
                rawValue: Int32(bitPattern: orientation))
        }

        // MARK: - The six snapshot accessors

        /// `GameWindow.Title`.
        ///
        /// The getter reads the stored string, as XNA's `ldfld title` does.
        /// **The setter is fallible**, so it is the writer method below rather
        /// than a Swift `set`.
        public var Title: String? { storedTitle }

        /// `GameWindow.set_Title(String value)` **and** the protected
        /// `SetTitle(String)` hook, which are one member here.
        ///
        /// XNA declares both, and the accessor rule derives the property
        /// writer's name as `Set` + `Title` — which is the hook's own name. The
        /// two cannot be overloaded apart in Swift: a `String` literal binds to
        /// the non-Optional one, and that is how the collision was found rather
        /// than reasoned about. So they are the same member, and
        /// `mapping-rules.json` records the parameter as Optional to make the
        /// two expected identities one.
        ///
        /// It does everything the setter's fifty-three bytes do:
        ///
        /// ```text
        /// if (value == null) throw ArgumentNullException("value", TitleCannotBeNull);
        /// if (title != value) { title = value; SetTitle(value); }
        /// ```
        ///
        /// **The platform is reached only when the value changes** — setting
        /// the same title twice pushes once.
        ///
        /// The one divergence, stated: a subclass overriding this replaces the
        /// whole setter, where an XNA override replaces only the platform half
        /// and never sees a null or an unchanged value.
        open func SetTitle(_ value: String?) throws {
            guard let value else {
                throw CNAArgumentNullException(
                    paramName: "value", message: GameWindow.titleCannotBeNullMessage)
            }
            guard storedTitle != value else { return }
            try pushPendingState()
            storedTitle = value
            var utf8 = Array(value.utf8)
            try runtime.functions.check(
                GameWindow.withStringView(&utf8) { view in
                    runtime.functions.gameSetWindowTitle(runtime.gameHandle, view)
                },
                operation: "cna_game_set_window_title")
        }

        /// `GameWindow.Handle`, the native window identifier.
        ///
        /// `System.IntPtr`, projected as `Int` the way every other opaque
        /// native handle in this binding is. Zero when the host has no window,
        /// which is what a HEADLESS renderer reports.
        public var Handle: Int { storedHandle }

        public var ClientBounds: Microsoft.Xna.Framework.Rectangle {
            storedClientBounds
        }

        public var ScreenDeviceName: String? { storedScreenDeviceName }

        public var CurrentOrientation: Microsoft.Xna.Framework.DisplayOrientation {
            storedOrientation
        }

        /// `GameWindow.AllowUserResizing`.
        ///
        /// Both accessors are infallible in the CLR, and the setter's effect
        /// crosses the boundary — so the value is stored and pushed at the
        /// throwing writer, the deferred-write shape `Mouse.WindowHandle`
        /// established.
        public var AllowUserResizing: Bool {
            get { storedAllowUserResizing }
            set {
                storedAllowUserResizing = newValue
                resizingNeedsPush = true
            }
        }

        private var resizingNeedsPush = false

        /// The deferred half of `AllowUserResizing`.
        ///
        /// Both its accessors are infallible in the CLR, so the Swift setter
        /// cannot throw — and the value has to reach the runtime somewhere.
        /// It is pushed at the next member that already throws, which is the
        /// screen-device-change pair and the title hook. Same architecture as
        /// `Mouse.WindowHandle` and `GraphicsAdapter`'s device preferences.
        private func pushPendingState() throws {
            guard resizingNeedsPush else { return }
            try runtime.functions.check(
                runtime.functions.gameWindowSetAllowUserResizing(
                    runtime.gameHandle, storedAllowUserResizing ? 1 : 0),
                operation: "cna_game_window_set_allow_user_resizing")
            resizingNeedsPush = false
        }

        // MARK: - The screen-device change pair

        public func BeginScreenDeviceChange(_ willBeFullScreen: Bool) throws {
            try pushPendingState()
            try runtime.functions.check(
                runtime.functions.gameWindowBeginScreenDeviceChange(
                    runtime.gameHandle, willBeFullScreen ? 1 : 0),
                operation: "cna_game_window_begin_screen_device_change")
        }

        /// `EndScreenDeviceChange(String, Int32, Int32)`.
        public func EndScreenDeviceChange(
            _ screenDeviceName: String,
            clientWidth: Int32,
            clientHeight: Int32
        ) throws {
            var utf8 = Array(screenDeviceName.utf8)
            try runtime.functions.check(
                GameWindow.withStringView(&utf8) { view in
                    runtime.functions.gameWindowEndScreenDeviceChange(
                        runtime.gameHandle, view, clientWidth, clientHeight)
                },
                operation: "cna_game_window_end_screen_device_change")
            storedScreenDeviceName = screenDeviceName
        }

        /// `EndScreenDeviceChange(String)`.
        ///
        /// Thirty bytes: it reads its **own** `ClientBounds` for the width and
        /// height and forwards. So the one-argument overload keeps the current
        /// size rather than defaulting to anything.
        public func EndScreenDeviceChange(_ screenDeviceName: String) throws {
            try EndScreenDeviceChange(
                screenDeviceName,
                clientWidth: ClientBounds.Width,
                clientHeight: ClientBounds.Height)
        }

        /// `GameWindow.SetSupportedOrientations(DisplayOrientation)`.
        ///
        /// **Not forwarded anywhere, and that is measured rather than lazy.**
        /// CNA publishes no orientation-setting route: `runtime_window.h` has
        /// `get_current_orientation` and nothing that writes one. XNA's own
        /// member is `protected abstract` and the Windows implementation is a
        /// no-op, because a desktop window has one orientation. Recording the
        /// request keeps the member honest without inventing an effect.
        open func SetSupportedOrientations(
            _ orientations: Microsoft.Xna.Framework.DisplayOrientation
        ) throws {
            requestedOrientations = orientations
        }

        /// What the last `SetSupportedOrientations` asked for. Nothing reads it
        /// but a test; it exists so the member stores rather than discards.
        internal private(set) var requestedOrientations:
            Microsoft.Xna.Framework.DisplayOrientation = .Default

        // MARK: - The three events and their raisers

        public var ScreenDeviceNameChanged: CNAEvent<CNAEventArgs> {
            screenDeviceNameChangedSource.Event
        }
        public var ClientSizeChanged: CNAEvent<CNAEventArgs> {
            clientSizeChangedSource.Event
        }
        public var OrientationChanged: CNAEvent<CNAEventArgs> {
            orientationChangedSource.Event
        }

        open func OnActivated() throws {}
        open func OnDeactivated() throws {}
        open func OnPaint() throws {}

        open func OnScreenDeviceNameChanged() throws {
            try screenDeviceNameChangedSource.Raise(self, args: CNAEventArgs.Empty)
        }
        open func OnClientSizeChanged() throws {
            try clientSizeChangedSource.Raise(self, args: CNAEventArgs.Empty)
        }
        open func OnOrientationChanged() throws {
            try orientationChangedSource.Raise(self, args: CNAEventArgs.Empty)
        }

        /// `Resources.TitleCannotBeNull`.
        ///
        /// Read out of the registered assembly's string table, not written from
        /// memory: the first draft of this file invented "GameWindow title
        /// cannot be null." and nothing caught it, because a message this
        /// binding has not ADMITTED is a message the resource-string check does
        /// not compare. Admitting it is what made the difference visible.
        ///
        /// Note the two spaces after the first sentence — they are Microsoft's.
        internal static let titleCannotBeNullMessage =
            "The title name cannot be null.  Use an empty string instead."

        private static func withStringView(
            _ utf8: inout [UInt8], _ body: (CNASwift_StringView) -> UInt32
        ) -> UInt32 {
            utf8.withUnsafeMutableBufferPointer { buffer -> UInt32 in
                var view = CNASwift_StringView()
                view.byte_length = UInt64(buffer.count)
                guard let base = buffer.baseAddress else { return body(view) }
                return base.withMemoryRebound(to: CChar.self, capacity: buffer.count) {
                    view.data = UnsafePointer($0)
                    return body(view)
                }
            }
        }

        private static func copyString(
            _ runtime: RuntimeState, _ handle: UInt64,
            _ size: (UInt64, UnsafeMutablePointer<UInt64>?) -> UInt32,
            _ copy: (UInt64, UnsafeMutablePointer<CChar>?, UInt64,
                     UnsafeMutablePointer<UInt64>?) -> UInt32,
            _ sizeOperation: String, _ copyOperation: String
        ) throws -> String {
            var byteCount: UInt64 = 0
            try runtime.functions.check(size(handle, &byteCount),
                                        operation: sizeOperation)
            guard byteCount > 0 else { return "" }
            var bytes = [CChar](repeating: 0, count: Int(byteCount) + 1)
            var written: UInt64 = 0
            try runtime.functions.check(
                bytes.withUnsafeMutableBufferPointer { buffer in
                    copy(handle, buffer.baseAddress, byteCount, &written)
                },
                operation: copyOperation)
            return String(decoding: bytes.prefix(Int(written))
                            .map { UInt8(bitPattern: $0) }, as: UTF8.self)
        }
    }
}
