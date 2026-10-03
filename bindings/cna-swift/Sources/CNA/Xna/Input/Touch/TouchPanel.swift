// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Input.Touch {

    /// `Microsoft.Xna.Framework.Input.Touch.TouchPanel`.
    ///
    /// The last Input type, and entirely static as XNA's is. Every supporting
    /// type it answers -- `TouchPanelCapabilities`, `TouchCollection`,
    /// `GestureSample` -- was projected before it, so this milestone is the
    /// panel alone.
    ///
    /// **Three properties are settable and three are not, and the accessor
    /// table draws the line in an unusual place.** `WindowHandle`,
    /// `DisplayWidth` and `DisplayHeight` have setters that are
    /// `IL_NO_FAILURE_PATH`, so they stay *settable properties* rather than
    /// becoming writer methods -- the only members in this binding that do.
    /// `EnabledGestures`' setter is `IL_DIRECT_THROW` and
    /// `DisplayOrientation`'s is `IL_REACHABLE_THROW`, so both are writers.
    public final class TouchPanel {

        internal init() {}

        /// `TouchPanel.GetCapabilities()`.
        public static func GetCapabilities() throws -> TouchPanelCapabilities {
            let rt = try RuntimeRegistry.current()
            var native = CNASwift_TouchCapabilities()
            native.struct_size = UInt32(MemoryLayout<CNASwift_TouchCapabilities>.size)
            native.struct_version = 1
            try rt.functions.check(
                withUnsafeMutablePointer(to: &native) {
                    rt.functions.touchGetCapabilities(rt.gameHandle, $0)
                },
                operation: "cna_touch_get_capabilities")
            return TouchPanelCapabilities(
                isConnected: native.is_connected != 0,
                maximumTouchCount: Int32(bitPattern: native.maximum_touch_count))
        }

        /// `TouchPanel.GetState()`.
        ///
        /// The native state carries a fixed array of eight locations and a
        /// count; only the counted prefix is read, because the rest is
        /// whatever the last frame left there.
        public static func GetState() throws -> TouchCollection {
            let rt = try RuntimeRegistry.current()
            var native = CNASwift_TouchState()
            native.struct_size = UInt32(MemoryLayout<CNASwift_TouchState>.size)
            native.struct_version = 1
            try rt.functions.check(
                withUnsafeMutablePointer(to: &native) {
                    rt.functions.touchGetState(rt.gameHandle, $0)
                },
                operation: "cna_touch_get_state")
            let all = withUnsafeBytes(of: native.touches) {
                Array($0.bindMemory(to: CNASwift_TouchLocation.self))
            }
            var locations: [TouchLocation] = []
            for raw in all.prefix(Int(native.touch_count)) {
                guard let state = TouchLocationState(
                    rawValue: Int32(bitPattern: raw.state)) else {
                    throw CNAError.nativeFailure(
                        operation: "TouchPanel.GetState", result: 1,
                        message: "native touch state \(raw.state) is not an XNA TouchLocationState")
                }
                locations.append(TouchLocation(
                    raw.id, state,
                    Microsoft.Xna.Framework.Vector2(raw.position.x, raw.position.y)))
            }
            return try TouchCollection(locations)
        }

        /// `TouchPanel.ReadGesture()`.
        public static func ReadGesture() throws -> GestureSample {
            let rt = try RuntimeRegistry.current()
            var native = CNASwift_GestureSample()
            native.struct_size = UInt32(MemoryLayout<CNASwift_GestureSample>.size)
            native.struct_version = 1
            try rt.functions.check(
                withUnsafeMutablePointer(to: &native) {
                    rt.functions.touchPanelReadGesture(rt.gameHandle, $0)
                },
                operation: "cna_touch_panel_read_gesture")
            return GestureSample(
                GestureType(rawValue: Int32(bitPattern: native.gesture_type)),
                Microsoft.Xna.Framework.Audio.SoundEffect
                    .duration(fromTicks: native.timestamp_ticks),
                Microsoft.Xna.Framework.Vector2(native.position.x, native.position.y),
                Microsoft.Xna.Framework.Vector2(native.position2.x, native.position2.y),
                Microsoft.Xna.Framework.Vector2(native.delta.x, native.delta.y),
                Microsoft.Xna.Framework.Vector2(native.delta2.x, native.delta2.y))
        }

        /// `TouchPanel.IsGestureAvailable`, `IL_DIRECT_THROW`.
        public static var IsGestureAvailable: Bool {
            get throws {
                let rt = try RuntimeRegistry.current()
                var value: UInt8 = 0
                try rt.functions.check(
                    rt.functions.touchPanelGetIsGestureAvailable(rt.gameHandle, &value),
                    operation: "cna_touch_panel_get_is_gesture_available")
                return value != 0
            }
        }

        /// `TouchPanel.EnabledGestures`, whose getter cannot fail.
        public static var EnabledGestures: GestureType {
            guard let rt = try? RuntimeRegistry.current() else { return GestureType(rawValue: 0) }
            var raw: UInt32 = 0
            guard rt.functions.touchPanelGetEnabledGestures(
                rt.gameHandle, &raw) == 0 else { return GestureType(rawValue: 0) }
            return GestureType(rawValue: Int32(bitPattern: raw))
        }

        public static func SetEnabledGestures(_ value: GestureType) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.touchPanelSetEnabledGestures(
                    rt.gameHandle, UInt32(bitPattern: value.rawValue)),
                operation: "cna_touch_panel_set_enabled_gestures")
        }

        /// `TouchPanel.DisplayOrientation`, whose setter is `IL_REACHABLE_THROW`.
        public static var DisplayOrientation: Microsoft.Xna.Framework.DisplayOrientation {
            guard let rt = try? RuntimeRegistry.current() else { return .Default }
            var raw: UInt32 = 0
            // `DisplayOrientation` is an OptionSet, not an enum: every bit
            // pattern is a legal value, so there is nothing to reject.
            guard rt.functions.touchPanelGetDisplayOrientation(
                rt.gameHandle, &raw) == 0 else { return .Default }
            return Microsoft.Xna.Framework.DisplayOrientation(
                rawValue: Int32(bitPattern: raw))
        }

        public static func SetDisplayOrientation(
            _ value: Microsoft.Xna.Framework.DisplayOrientation
        ) throws {
            let rt = try RuntimeRegistry.current()
            try rt.functions.check(
                rt.functions.touchPanelSetDisplayOrientation(
                    rt.gameHandle, UInt32(bitPattern: value.rawValue)),
                operation: "cna_touch_panel_set_display_orientation")
        }

        /// `TouchPanel.WindowHandle`.
        ///
        /// **A settable property with a deferred write**, because BOTH its
        /// accessors are `IL_NO_FAILURE_PATH` and neither may refuse. The value
        /// is stored and pushed to CNA on the same call; a push that fails is
        /// kept in `lastPushFailure` rather than thrown, since the setter has
        /// no way to report it -- which is the shape `GraphicsAdapter`'s device
        /// preferences already use.
        public static var WindowHandle: Int {
            get {
                guard let rt = try? RuntimeRegistry.current() else { return storedWindowHandle }
                var value: UInt64 = 0
                guard rt.functions.touchPanelGetWindowHandle(
                    rt.gameHandle, &value) == 0 else { return storedWindowHandle }
                return Int(bitPattern: UInt(value))
            }
            set {
                storedWindowHandle = newValue
                push { rt in
                    rt.functions.touchPanelSetWindowHandle(
                        rt.gameHandle, UInt64(UInt(bitPattern: newValue)))
                }
            }
        }

        /// `TouchPanel.DisplayWidth`, settable and infallible in both
        /// directions.
        public static var DisplayWidth: Int32 {
            get { readInt32OrStored(stored: storedDisplayWidth, width: true) }
            set {
                storedDisplayWidth = newValue
                push { rt in
                    rt.functions.touchPanelSetDisplayWidth(rt.gameHandle, newValue)
                }
            }
        }

        /// `TouchPanel.DisplayHeight`.
        public static var DisplayHeight: Int32 {
            get { readInt32OrStored(stored: storedDisplayHeight, width: false) }
            set {
                storedDisplayHeight = newValue
                push { rt in
                    rt.functions.touchPanelSetDisplayHeight(rt.gameHandle, newValue)
                }
            }
        }

        /// The error an infallible setter could not raise, kept so a test can
        /// assert the divergence is real rather than described.
        internal private(set) static var lastPushFailure: Error?

        private static var storedWindowHandle = 0
        private static var storedDisplayWidth: Int32 = 0
        private static var storedDisplayHeight: Int32 = 0

        /// The two Int32 reads share a body, and the route is chosen by a flag
        /// rather than passed in -- a `@convention(c)` pointer behind a Swift
        /// closure parameter crashes this toolchain.
        private static func readInt32OrStored(stored: Int32, width: Bool) -> Int32 {
            guard let rt = try? RuntimeRegistry.current() else { return stored }
            var value: Int32 = 0
            let result = width
                ? rt.functions.touchPanelGetDisplayWidth(rt.gameHandle, &value)
                : rt.functions.touchPanelGetDisplayHeight(rt.gameHandle, &value)
            guard result == 0 else { return stored }
            return value
        }

        private static func push(_ body: (RuntimeState) -> UInt32) {
            guard let rt = try? RuntimeRegistry.current() else {
                lastPushFailure = CNAError.streamFailure(
                    "no runtime to push a touch-panel setting to")
                return
            }
            let result = body(rt)
            lastPushFailure = result == 0 ? nil : CNAError.nativeFailure(
                operation: "TouchPanel setter", result: result,
                message: "the panel refused a setting an infallible setter "
                    + "cannot report")
        }
    }
}
