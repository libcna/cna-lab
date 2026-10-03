import XCTest
@testable import CNA

/// `Microsoft.Xna.Framework.Input.Mouse`.
///
/// Three members, none of which has a managed half: every one is Win32 in
/// XNA's IL. What is asserted is therefore what this host can observe — and on
/// HEADLESS the logical cursor position turns out to be observable, so the
/// milestone is not as thin as Foundation 72's was.
final class Foundation74MouseTests: XCTestCase {
    private typealias M = Microsoft.Xna.Framework.Input.Mouse

    /// All three members take the runtime's handle the way `Keyboard.GetState`
    /// does, so all three refuse outside a lifecycle callback. XNA's are
    /// genuinely static and answer anywhere; this is the binding declining to
    /// invent a cursor it cannot read.
    func testEveryMemberRefusesOutsideTheLifecycle() {
        func expectOutside(_ body: () throws -> Void, _ what: String) {
            XCTAssertThrowsError(try body(), what) { error in
                guard case CNAError.callbackOutsideGameLifecycle = error else {
                    return XCTFail("\(what): expected callbackOutsideGameLifecycle, got \(error)")
                }
            }
        }
        expectOutside({ _ = try M.GetState() }, "GetState")
        expectOutside({ try M.SetPosition(1, y: 2) }, "SetPosition")

        // WindowHandle is deliberately NOT in that list. Both its
        // accessors are infallible in the CLR -- the getter is
        // `ldsfld hHookedHandle; ret` -- so it is a stored value here too
        // and reads anywhere, exactly as XNA's does.
        XCTAssertEqual(M.WindowHandle, 0,
                       "an unset handle reads zero without a runtime")
    }

    func testTheCursorPositionRoundTripsThroughTheRuntime() throws {
        let game = try MouseProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.movedTo?.x, 40, "SetPosition's x is what GetState reports")
        XCTAssertEqual(game.movedTo?.y, 50, "SetPosition's y is what GetState reports")
        XCTAssertEqual(game.negativeTo?.x, -5, "a negative position is accepted, not clamped")
        XCTAssertEqual(game.negativeTo?.y, -5)
    }

    /// Every button reads `Released` on this host, and the assertion is that
    /// each one is read from **its own bit**. The bit order is not the order
    /// `MouseState`'s constructor takes its buttons in, so a transposition
    /// would compile and report a right-click as a middle-click.
    func testEveryButtonIsReleasedOnAHeadlessHost() throws {
        let game = try MouseProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        let state = try XCTUnwrap(game.observed)
        XCTAssertEqual(state.LeftButton, .Released)
        XCTAssertEqual(state.MiddleButton, .Released)
        XCTAssertEqual(state.RightButton, .Released)
        XCTAssertEqual(state.XButton1, .Released)
        XCTAssertEqual(state.XButton2, .Released)
        XCTAssertEqual(state.ScrollWheelValue, 0)
    }

    /// The handle round-trips, and zero unbinds.
    ///
    /// **Zero is also what an unbound handle must read as, and the route does
    /// not write it.** Measured in `build-probe/f74_mouse.c`: with no window
    /// bound, `cna_mouse_get_window_handle` returns success and leaves the
    /// caller's buffer UNTOUCHED — a probe that seeded it with 123 read 123
    /// back — although its own header says the value "is zero when none is
    /// bound". The projection zero-initialises before the call for exactly
    /// that reason, and this test would read the previous handle rather than
    /// zero if it stopped.
    /// The handle is a stored value, and what it is set to is pushed at the
    /// next throwing member.
    ///
    /// XNA hands the handle to its `WindowMessageHooker` inside the setter. A
    /// Swift setter cannot throw and every CNA route can fail, so the push is
    /// deferred to `GetState`/`SetPosition` -- and the test that it happened is
    /// that those still succeed with a non-zero handle recorded.
    /// Each button comes from **its own bit**, and the scroll value from the
    /// **vertical** wheel.
    ///
    /// Tested directly on the translation, because it cannot be reached with
    /// distinguishing inputs: on this host every button reads `Released` and
    /// both wheels read zero, so a transposed pair and a wheel read from the
    /// wrong field are equally invisible through `GetState`. Two mutations
    /// survived exactly that, and these are the inputs that fail them.
    ///
    /// (It was then lost once to a careless edit of this file and both
    /// mutations came back SURVIVED, which is the harness earning its keep for
    /// a second time in one milestone.)
    func testTheStateTranslationReadsEachBitAndTheVerticalWheel() {
        typealias B = Microsoft.Xna.Framework.Input.ButtonState
        func state(_ pressed: UInt32, wheel: Int32 = 7, horizontal: Int32 = -99)
            -> Microsoft.Xna.Framework.Input.MouseState {
            M.projectedState(x: 1, y: 2, scrollWheel: wheel,
                             horizontalScrollWheel: horizontal, pressed: pressed)
        }
        // CNA orders the bits left, middle, right; XNA's constructor takes
        // them left, MIDDLE, right after the wheel. Each is checked alone so a
        // transposition cannot hide behind a neighbour.
        let cases: [(UInt32, String, (Microsoft.Xna.Framework.Input.MouseState) -> B)] = [
            (1 << 0, "left", { $0.LeftButton }),
            (1 << 1, "middle", { $0.MiddleButton }),
            (1 << 2, "right", { $0.RightButton }),
            (1 << 3, "x1", { $0.XButton1 }),
            (1 << 4, "x2", { $0.XButton2 }),
        ]
        for (bit, name, read) in cases {
            let only = state(bit)
            XCTAssertEqual(read(only), .Pressed, "\(name) must come from bit \(bit)")
            for (otherBit, otherName, otherRead) in cases where otherBit != bit {
                XCTAssertEqual(otherRead(only), .Released,
                               "\(name) pressed must leave \(otherName) released")
            }
        }
        // The single scroll value is the VERTICAL wheel. XNA has nowhere to
        // put the horizontal one and this projection reports it nowhere.
        XCTAssertEqual(state(0, wheel: 7, horizontal: -99).ScrollWheelValue, 7,
                       "ScrollWheelValue is the vertical wheel, never the horizontal")
    }

    func testTheWindowHandleIsStoredAndPushedAtTheNextThrowingMember() throws {
        typealias Mouse = Microsoft.Xna.Framework.Input.Mouse
        Mouse.WindowHandle = 0x1234
        XCTAssertEqual(Mouse.WindowHandle, 0x1234,
                       "the setter stores, with no runtime in sight")
        let game = try MouseWindowProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.acceptedWithHandle, true,
                       "the recorded handle is pushed and accepted")
        Mouse.WindowHandle = 0
        XCTAssertEqual(Mouse.WindowHandle, 0, "zero unbinds")
    }

}

/// Cursor and buttons only.
///
/// **It must not touch the window handle**, and that is not tidiness. The
/// binding of a window outlives the `Game` that bound it, so a probe that binds
/// one leaves every later test reading a handle CNA has written at least once
/// -- which is exactly the condition under which the unbound-read behaviour
/// below stops being observable. One draft shared a single probe across all
/// three tests and the mutation for it survived.
private final class MouseProbeGame: Microsoft.Xna.Framework.Game {
    var failure: Error?
    var observed: Microsoft.Xna.Framework.Input.MouseState?
    var movedTo: (x: Int32, y: Int32)?
    var negativeTo: (x: Int32, y: Int32)?

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        // Exit is driven by reaching the end of this callback, never by a
        // counter the work advances -- Foundation 72's probe hung for exactly
        // that reason.
        defer { try? Exit() }
        do {
            typealias M = Microsoft.Xna.Framework.Input.Mouse
            try M.SetPosition(40, y: 50)
            let moved = try M.GetState()
            movedTo = (moved.X, moved.Y)
            observed = moved

            try M.SetPosition(-5, y: -5)
            let negative = try M.GetState()
            negativeTo = (negative.X, negative.Y)
        } catch {
            failure = error
        }
    }
}

/// The only probe in this file that binds a window.
private final class MouseWindowProbeGame: Microsoft.Xna.Framework.Game {
    var failure: Error?
    var acceptedWithHandle: Bool?

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        do {
            // Reaching the route at all pushes whatever handle was recorded.
            _ = try Microsoft.Xna.Framework.Input.Mouse.GetState()
            acceptedWithHandle = true
        } catch {
            failure = error
        }
    }
}
