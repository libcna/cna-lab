// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// `Microsoft.Xna.Framework.Input.Touch.TouchPanel`.
///
/// The last Input type. This host has no touch device, which is the honest
/// expectation and is what most of these assertions are about: a panel that is
/// not connected still answers, and every setting still round-trips.
final class Foundation99TouchPanelTests: XCTestCase {

    func testTheCapabilitiesReportNoTouchDevice() throws {
        let game = try TouchProbeGame()
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        if let refusal = game.readFailure { throw refusal }
        XCTAssertEqual(game.isConnected, false, "no touch screen on this host")
        XCTAssertGreaterThanOrEqual(game.maximumTouchCount ?? -1, 0)
        XCTAssertEqual(game.touchCount, 0, "and therefore no touches")
        XCTAssertEqual(game.isGestureAvailable, false)
    }

    /// **Three settings are properties, not writer methods**, because both
    /// their accessors are `IL_NO_FAILURE_PATH` -- the only members in this
    /// binding shaped that way. Each round-trips through the runtime.
    func testTheThreeInfallibleSettingsRoundTrip() throws {
        let game = try TouchProbeGame(exerciseSettings: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.widthAfter, 640)
        XCTAssertEqual(game.heightAfter, 480)
        XCTAssertEqual(game.windowHandleAfter, 1234)
        XCTAssertNil(game.pushFailureAfterSet,
                     "a setting the panel accepted leaves no kept failure")
    }

    /// The two fallible writers refuse or accept through the runtime, and the
    /// gesture mask round-trips.
    func testTheTwoWritersReachTheRuntime() throws {
        let game = try TouchProbeGame(exerciseWriters: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        if let refusal = game.writerFailure { throw refusal }
        XCTAssertEqual(game.gesturesAfter?.rawValue,
                       Microsoft.Xna.Framework.Input.Touch.GestureType.Tap.rawValue
                       | Microsoft.Xna.Framework.Input.Touch.GestureType.Flick.rawValue)
        XCTAssertEqual(game.orientationAfter?.rawValue,
                       Microsoft.Xna.Framework.DisplayOrientation.LandscapeLeft.rawValue)
    }

    /// **Read outside a runtime, the infallible getters answer their stored
    /// value rather than refusing** -- they have no way to refuse. This is the
    /// only place that fallback is observable, because inside the lifecycle
    /// the runtime always answers.
    func testTheInfallibleGettersFallBackOutsideTheLifecycle() {
        typealias T = Microsoft.Xna.Framework.Input.Touch.TouchPanel
        // No game is running here: every one of these must return, not throw.
        _ = T.DisplayWidth
        _ = T.DisplayHeight
        _ = T.WindowHandle
        _ = T.EnabledGestures
        _ = T.DisplayOrientation
        XCTAssertTrue(true, "none of the five refused")
    }
}

private final class TouchProbeGame: Microsoft.Xna.Framework.Game {
    let exerciseSettings: Bool
    let exerciseWriters: Bool

    var failure: Error?
    var readFailure: Error?
    var writerFailure: Error?
    var isConnected: Bool?
    var maximumTouchCount: Int32?
    var touchCount: Int32?
    var isGestureAvailable: Bool?
    var widthAfter: Int32?
    var heightAfter: Int32?
    var windowHandleAfter: Int?
    var pushFailureAfterSet: Error?
    var gesturesAfter: Microsoft.Xna.Framework.Input.Touch.GestureType?
    var orientationAfter: Microsoft.Xna.Framework.DisplayOrientation?

    init(exerciseSettings: Bool = false, exerciseWriters: Bool = false) throws {
        self.exerciseSettings = exerciseSettings
        self.exerciseWriters = exerciseWriters
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        typealias T = Microsoft.Xna.Framework.Input.Touch.TouchPanel
        do {
            if exerciseSettings {
                T.DisplayWidth = 640
                T.DisplayHeight = 480
                T.WindowHandle = 1234
                widthAfter = T.DisplayWidth
                heightAfter = T.DisplayHeight
                windowHandleAfter = T.WindowHandle
                pushFailureAfterSet = T.lastPushFailure
                return
            }

            if exerciseWriters {
                do {
                    let mask = Microsoft.Xna.Framework.Input.Touch.GestureType(
                        rawValue:
                            Microsoft.Xna.Framework.Input.Touch.GestureType.Tap.rawValue
                            | Microsoft.Xna.Framework.Input.Touch.GestureType.Flick.rawValue)
                    try T.SetEnabledGestures(mask)
                    gesturesAfter = T.EnabledGestures
                    try T.SetDisplayOrientation(.LandscapeLeft)
                    orientationAfter = T.DisplayOrientation
                } catch { writerFailure = error }
                return
            }

            do {
                let capabilities = try T.GetCapabilities()
                isConnected = capabilities.IsConnected
                maximumTouchCount = capabilities.MaximumTouchCount
                touchCount = try T.GetState().Count
                isGestureAvailable = try T.IsGestureAvailable
            } catch { readFailure = error }
        } catch {
            failure = error
        }
    }
}
