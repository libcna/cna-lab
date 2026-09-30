// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

/// A game whose body runs inside `LoadContent`, where a device exists.
private final class PreferenceProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((PreferenceProbeGame) throws -> Void)?

    init(_ body: @escaping (PreferenceProbeGame) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try F.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        do { try body?(self) } catch { failure = error }
        try Exit()
    }

    override func Update(_ gameTime: F.GameTime) throws { try Exit() }
}

/// Foundation 51: `GraphicsDeviceManager`'s nine preferences, the two that
/// validate, and the short-circuit `ApplyChanges` did not have.
final class Foundation51ManagerPreferencesTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (PreferenceProbeGame) throws -> Void
    ) throws -> PreferenceProbeGame {
        let game = try PreferenceProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    private let dimensionMessage =
        "BackBufferWidth and BackBufferHeight must be greater than zero."

    // ------------------------------------------------------------------
    // The constructor's defaults.
    // ------------------------------------------------------------------

    /// Every default is the pinned `.ctor`'s, not a remembered one. Two are
    /// not the CLR zero: `SynchronizeWithVerticalRetrace` is `ldc.i4.1` and
    /// `PreferredDepthStencilFormat` is `ldc.i4.2`.
    func testTheDefaultsAreTheConstructorsOwn() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        XCTAssertEqual(manager.PreferredBackBufferWidth, 800)
        XCTAssertEqual(manager.PreferredBackBufferHeight, 480)
        XCTAssertEqual(manager.PreferredBackBufferWidth,
                       F.GraphicsDeviceManager.DefaultBackBufferWidth)
        XCTAssertEqual(manager.PreferredBackBufferHeight,
                       F.GraphicsDeviceManager.DefaultBackBufferHeight)
        XCTAssertEqual(manager.GraphicsProfile, .Reach)
        XCTAssertEqual(manager.PreferredBackBufferFormat, .Color)
        XCTAssertEqual(manager.PreferredDepthStencilFormat, .Depth24,
                       "ldc.i4.2, not the CLR default DepthFormat.None")
        XCTAssertFalse(manager.IsFullScreen)
        XCTAssertFalse(manager.PreferMultiSampling)
        XCTAssertTrue(manager.SynchronizeWithVerticalRetrace,
                      "ldc.i4.1, the constructor's first instruction")
        XCTAssertEqual(manager.SupportedOrientations, [])
    }

    /// CNA's own manager starts in the same nine places, which is what makes
    /// the managed fields and the native manager agree before anything is
    /// applied. Measured one layer down by `build-probe/f51_manager_prefs.c`,
    /// which reads CNA's preference getters directly and finds Reach,
    /// 800x480, Color, Depth24, windowed, no multisampling, vsync on and
    /// orientation Default. Those getters are deliberately **not** bound —
    /// no projected member reads a preference back from CNA, and a route no
    /// member consumes is what the native boundary refuses — so the agreement
    /// is pinned there rather than here, and what this asserts is the
    /// consequence: an untouched manager applies its defaults and the device
    /// comes back with them.
    func testAnUntouchedManagerAppliesTheConstructorsDefaults() throws {
        try requireNative()
        let game = try run { game in
            guard let manager = game.manager, let device = try game.GraphicsDevice
            else { return }
            try manager.ApplyChanges()
            let native = try device.nativePresentationParameters()
            game.observations["width"] = String(native.back_buffer_width)
            game.observations["height"] = String(native.back_buffer_height)
            game.observations["depth"] = String(native.depth_stencil_format)
        }
        XCTAssertEqual(game.observations["width"], "800")
        XCTAssertEqual(game.observations["height"], "480")
        XCTAssertEqual(
            game.observations["depth"],
            String(G.DepthFormat.Depth24.rawValue),
            "the constructor's ldc.i4.2 survives the round trip")
    }

    // ------------------------------------------------------------------
    // The two setters that validate.
    // ------------------------------------------------------------------

    /// `bgt` against zero: zero is rejected with every negative, and the
    /// stored value does not move.
    func testANonPositiveDimensionIsRejected() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        for bad in [Int32(0), -1, Int32.min] {
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: dimensionMessage + "\r\nParameter name: value",
                paramName: "value",
                hResult: Int32(bitPattern: 0x8013_1502)
            ) {
                try manager.SetPreferredBackBufferWidth(bad)
            }
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: dimensionMessage + "\r\nParameter name: value",
                paramName: "value",
                hResult: Int32(bitPattern: 0x8013_1502)
            ) {
                try manager.SetPreferredBackBufferHeight(bad)
            }
        }
        XCTAssertEqual(manager.PreferredBackBufferWidth, 800, "unchanged by a refusal")
        XCTAssertEqual(manager.PreferredBackBufferHeight, 480)
    }

    /// One is the smallest accepted value; the message names both dimensions
    /// whichever setter raised it.
    func testOneIsAccepted() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        try manager.SetPreferredBackBufferWidth(1)
        try manager.SetPreferredBackBufferHeight(1)
        XCTAssertEqual(manager.PreferredBackBufferWidth, 1)
        XCTAssertEqual(manager.PreferredBackBufferHeight, 1)
    }

    // ------------------------------------------------------------------
    // Every preference stores and reads back.
    // ------------------------------------------------------------------

    func testEveryPreferenceRoundTrips() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let manager = try F.GraphicsDeviceManager(game: game)
        manager.GraphicsProfile = .HiDef
        manager.PreferredBackBufferFormat = .Bgr565
        manager.PreferredDepthStencilFormat = .Depth24Stencil8
        manager.IsFullScreen = true
        manager.PreferMultiSampling = true
        manager.SynchronizeWithVerticalRetrace = false
        manager.SupportedOrientations = [.Portrait, .LandscapeLeft]
        try manager.SetPreferredBackBufferWidth(640)
        try manager.SetPreferredBackBufferHeight(360)

        XCTAssertEqual(manager.GraphicsProfile, .HiDef)
        XCTAssertEqual(manager.PreferredBackBufferFormat, .Bgr565)
        XCTAssertEqual(manager.PreferredDepthStencilFormat, .Depth24Stencil8)
        XCTAssertTrue(manager.IsFullScreen)
        XCTAssertTrue(manager.PreferMultiSampling)
        XCTAssertFalse(manager.SynchronizeWithVerticalRetrace)
        XCTAssertEqual(manager.SupportedOrientations, [.Portrait, .LandscapeLeft])
        XCTAssertEqual(manager.PreferredBackBufferWidth, 640)
        XCTAssertEqual(manager.PreferredBackBufferHeight, 360)
    }

    // ------------------------------------------------------------------
    // ApplyChanges, and the short-circuit it did not have.
    // ------------------------------------------------------------------

    /// A recorded preference reaches the device through `ApplyChanges`.
    func testApplyChangesReachesTheDevice() throws {
        try requireNative()
        let game = try run { game in
            guard let manager = game.manager else { return }
            try manager.SetPreferredBackBufferWidth(640)
            try manager.SetPreferredBackBufferHeight(360)
            try manager.ApplyChanges()
            guard let device = try game.GraphicsDevice else { return }
            let native = try device.nativePresentationParameters()
            game.observations["width"] = String(native.back_buffer_width)
            game.observations["height"] = String(native.back_buffer_height)
        }
        XCTAssertEqual(game.observations["width"], "640")
        XCTAssertEqual(game.observations["height"], "360")
    }

    /// With a device and nothing dirty, `ApplyChanges` does nothing at all.
    ///
    /// "Did nothing" is observed by leaving a value where only a push would
    /// overwrite it. CNA's preference setters *record* a request — the device
    /// does not move until something applies it — so the test writes 512 into
    /// the native preference behind the projection's back, calls the clean
    /// `ApplyChanges`, and then applies natively. If the clean call had
    /// pushed, it would have written the managed 800 over the 512 and the
    /// device would come back 800; because it short-circuits, the 512 is
    /// still there to be applied.
    func testApplyChangesShortCircuitsWhenNothingIsDirty() throws {
        try requireNative()
        let game = try run { game in
            guard let manager = game.manager, let device = try game.GraphicsDevice
            else { return }
            let handle = try manager.nativeHandleForTests()
            let functions = try RuntimeRegistry.current().functions

            try manager.ApplyChanges()          // clears any startup dirt
            _ = functions.graphicsManagerSetPreferredBackBufferWidth(handle, 512)
            try manager.ApplyChanges()          // not dirty: must not push
            _ = functions.graphicsManagerApplyChanges(handle)
            game.observations["clean"] =
                String(try device.nativePresentationParameters().back_buffer_width)

            _ = functions.graphicsManagerSetPreferredBackBufferWidth(handle, 512)
            manager.PreferMultiSampling = false   // any setter marks it dirty
            try manager.ApplyChanges()          // dirty: pushes 800 over the 512
            game.observations["dirty"] =
                String(try device.nativePresentationParameters().back_buffer_width)
        }
        XCTAssertEqual(game.observations["clean"], "512",
                       "a clean ApplyChanges must not overwrite the native preference")
        XCTAssertEqual(game.observations["dirty"], "800",
                       "a dirty one pushes every managed preference over it")
    }

    /// `ToggleFullScreen` flips the property **and** changes the device,
    /// which is what its two IL instructions do:
    ///
    /// ```text
    /// this.IsFullScreen = !this.IsFullScreen;
    /// this.ChangeDevice(false);
    /// ```
    ///
    /// The device change is observed the same way the short-circuit is: a
    /// native preference written behind the projection's back survives a call
    /// that does not push and is overwritten by one that does.
    func testToggleFullScreenFlipsAndChangesTheDevice() throws {
        try requireNative()
        let game = try run { game in
            guard let manager = game.manager, let device = try game.GraphicsDevice
            else { return }
            let handle = try manager.nativeHandleForTests()
            let functions = try RuntimeRegistry.current().functions

            game.observations["before"] = String(manager.IsFullScreen)
            _ = functions.graphicsManagerSetPreferredBackBufferWidth(handle, 512)
            try manager.ToggleFullScreen()
            game.observations["after"] = String(manager.IsFullScreen)
            // The device does not move until something applies, so reading its
            // width straight after the toggle cannot tell a toggle that pushed
            // from one that did not: both leave it at 800. Applying natively
            // afterwards asks which preference is standing — the managed 800
            // the toggle should have pushed, or the 512 left behind its back.
            _ = functions.graphicsManagerApplyChanges(handle)
            game.observations["width"] =
                String(try device.nativePresentationParameters().back_buffer_width)

            try manager.ToggleFullScreen()
            game.observations["back"] = String(manager.IsFullScreen)
        }
        XCTAssertEqual(game.observations["before"], "false")
        XCTAssertEqual(game.observations["after"], "true")
        XCTAssertEqual(game.observations["back"], "false")
        XCTAssertEqual(
            game.observations["width"], "800",
            "the toggle pushed every managed preference and applied them, "
            + "overwriting the 512 written behind its back")
    }
}
