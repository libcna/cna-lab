// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Input {

    /// `Microsoft.Xna.Framework.Input.Mouse`, a sealed static class with three
    /// members and **no managed half at all**.
    ///
    /// Every one of them is Win32 in the IL. `GetState` is `GetCursorPos`, then
    /// `ScreenToClient` when a window is hooked, then `GetAsyncKeyState` once
    /// per button; `SetPosition` is `ClientToScreen` then `SetCursorPos`; and
    /// `WindowHandle` is a raw `HWND` field. There is no decision in any of
    /// them this binding could reproduce, so all three forward, and what they
    /// promise is deliberately narrow.
    ///
    /// `MouseState` and `ButtonState` already stand, so this is the last piece
    /// of its family rather than the first.
    public final class Mouse {

        /// `CNA_MOUSE_BUTTON_*`, pinned by `_Static_assert`s in the ABI probe.
        ///
        /// The bit order is **not** the order XNA's `MouseState` constructor
        /// takes its buttons in — CNA orders them left, middle, right while the
        /// constructor takes left, middle, right *after* the scroll wheel and
        /// the property list reads left, right, middle. A swap here would
        /// compile, run, and report a right-click as a middle-click, so each
        /// bit is a named constant used once.
        private static let buttonLeft: UInt32 = 1 << 0
        private static let buttonMiddle: UInt32 = 1 << 1
        private static let buttonRight: UInt32 = 1 << 2
        private static let buttonX1: UInt32 = 1 << 3
        private static let buttonX2: UInt32 = 1 << 4

        private static func button(
            _ mask: UInt32, _ bit: UInt32
        ) -> Microsoft.Xna.Framework.Input.ButtonState {
            (mask & bit) != 0 ? .Pressed : .Released
        }

        /// `Mouse.GetState()`.
        ///
        /// XNA reads the cursor and every button through Win32 at the call
        /// site; CNA captures the same snapshot in one route, which its header
        /// requires to run on the game creation thread. The handle comes from
        /// `RuntimeRegistry.current()`, the way every other static XNA input
        /// member takes it, so outside a lifecycle callback this refuses where
        /// XNA would answer.
        ///
        /// **`CNA_MouseState` carries a horizontal wheel and XNA has nowhere to
        /// put it.** `MouseState`'s eight fields include one scroll value, so
        /// the horizontal one is read and dropped rather than folded into the
        /// vertical — folding it would invent a number XNA never reports.
        public static func GetState() throws -> Microsoft.Xna.Framework.Input.MouseState {
            let runtime = try RuntimeRegistry.current()
            try Mouse.pushWindowHandle(runtime)
            var state = CNASwift_MouseState()
            state.struct_size = UInt32(MemoryLayout<CNASwift_MouseState>.size)
            state.struct_version = 1
            try runtime.functions.check(
                runtime.functions.mouseGetState(runtime.gameHandle, &state),
                operation: "cna_mouse_get_state"
            )
            return Mouse.projectedState(
                x: state.x, y: state.y,
                scrollWheel: state.scroll_wheel,
                horizontalScrollWheel: state.horizontal_scroll_wheel,
                pressed: state.pressed_buttons)
        }

        /// The whole translation from a captured `CNA_MouseState` to XNA's
        /// value, as a function of its numbers.
        ///
        /// **Extracted so it can be tested at all.** On this host every button
        /// reads `Released` and both wheels read zero, so a transposed button
        /// bit and a wheel read from the wrong field are both invisible through
        /// `GetState` -- two mutations survived it and neither was a missing
        /// assertion. Same remedy as Foundation 73's `volumeAspectExtremes` and
        /// Foundation 68's `arrayHolds`: when a mapping cannot be reached with
        /// distinguishing inputs, make it a function and give it some.
        ///
        /// The pinned constructor order is `(x, y, scrollWheel, left, MIDDLE,
        /// right, x1, x2)` -- `middleButton` precedes `rightButton`, which is
        /// not the order the property list suggests.
        ///
        /// `horizontalScrollWheel` is taken and **dropped**. XNA's `MouseState`
        /// has one scroll value and no place for a second; folding it into the
        /// vertical would invent a number XNA never reports. It is a parameter
        /// rather than an omission so that reading the wrong one is a visible
        /// mistake here.
        internal static func projectedState(
            x: Int32,
            y: Int32,
            scrollWheel: Int32,
            horizontalScrollWheel: Int32,
            pressed: UInt32
        ) -> Microsoft.Xna.Framework.Input.MouseState {
            _ = horizontalScrollWheel
            return Microsoft.Xna.Framework.Input.MouseState(
                x,
                y,
                scrollWheel,
                button(pressed, Mouse.buttonLeft),
                button(pressed, Mouse.buttonMiddle),
                button(pressed, Mouse.buttonRight),
                button(pressed, Mouse.buttonX1),
                button(pressed, Mouse.buttonX2)
            )
        }

        /// `Mouse.SetPosition(Int32 x, Int32 y)`.
        ///
        /// **The coordinates are client-relative when a window is hooked.**
        /// XNA's IL builds a `POINT`, calls `ClientToScreen` through the hooked
        /// `HWND` and only then `SetCursorPos` — so the same pair means
        /// different screen positions depending on `WindowHandle`. CNA's route
        /// is documented as moving the cursor *"to a position inside the bound
        /// window"*, which is the same frame of reference.
        ///
        /// **XNA discards both results.** `ClientToScreen` and `SetCursorPos`
        /// each return a status and the IL `pop`s both, so a move that cannot
        /// happen is silent. CNA says the same from its side: *"The canonical
        /// operation returns nothing, so a request no backend can satisfy
        /// succeeds and changes nothing."* This member is still `throws`,
        /// because reaching the route at all can fail -- there may be no
        /// runtime -- but a refused *move* is not a failure and is not reported
        /// as one.
        public static func SetPosition(_ x: Int32, y: Int32) throws {
            let runtime = try RuntimeRegistry.current()
            try Mouse.pushWindowHandle(runtime)
            try runtime.functions.check(
                runtime.functions.mouseSetPosition(runtime.gameHandle, x, y),
                operation: "cna_mouse_set_position"
            )
        }

        /// `Mouse.WindowHandle`, XNA's `hHookedHandle`.
        ///
        /// **Infallible on both accessors, because XNA's are.** `get` is
        /// `ldsfld hHookedHandle; ret` -- it reads a static field and never
        /// asks the platform anything -- and `set` is `stsfld` followed by
        /// handing the value to the `WindowMessageHooker`. The pinned
        /// fallibility measurement says so, and the verifier refuses a throwing
        /// reader here.
        ///
        /// So this is a stored value, exactly as XNA's is, and reading it needs
        /// no runtime: outside a lifecycle callback it answers what was last
        /// set, where a route-backed getter would have refused.
        ///
        /// `System.IntPtr`, projected as `Int` the way
        /// `GraphicsDevice.Present(overrideWindowHandle:)` already projects it.
        /// The value is opaque on both sides -- CNA's header says it *"is an
        /// opaque native pointer-sized identifier, not a `CNA_Handle`, and the
        /// C API never dereferences it"* -- and **zero means no window is
        /// bound**, the same thing XNA's null `HWND` means and what its
        /// `GetState` tests before calling `ScreenToClient`.
        public static var WindowHandle: Int {
            get { storedWindowHandle }
            set {
                storedWindowHandle = newValue
                windowHandleNeedsPush = true
            }
        }

        /// XNA's `hHookedHandle`, which starts null.
        private static var storedWindowHandle = 0
        private static var windowHandleNeedsPush = false

        /// The deferred half of the setter.
        ///
        /// A Swift setter cannot throw and every CNA route can fail, so the
        /// binding a `set` records is pushed at the next member that already
        /// throws -- the same architecture the infallible XNA setters elsewhere
        /// use. XNA hands the handle to its hooker inside the setter; here that
        /// happens at the next `GetState` or `SetPosition`, which are the only
        /// members whose behaviour the binding changes.
        private static func pushWindowHandle(_ runtime: RuntimeState) throws {
            guard windowHandleNeedsPush else { return }
            try runtime.functions.check(
                runtime.functions.mouseSetWindowHandle(
                    runtime.gameHandle, UInt64(UInt(bitPattern: storedWindowHandle))),
                operation: "cna_mouse_set_window_handle"
            )
            windowHandleNeedsPush = false
        }
    }
}
