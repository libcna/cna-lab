import XCTest
@testable import CNA

/// `GraphicsDevice`'s presentation and identity members.
///
/// Foundation 60 measured `Present` and `Reset` as accepted and the entry read
/// "not blocked, and now ordinary work" for fourteen milestones. These are that
/// work, plus the three snapshot accessors `GraphicsAdapter` unblocked.
final class Foundation78DevicePresentationTests: XCTestCase {

    func testTheSnapshotAccessorsAnswerWithoutThrowing() throws {
        let game = try DeviceProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }

        // Infallible in the CLR, so infallible here: read without `try`.
        let parameters = try XCTUnwrap(game.parameters)
        XCTAssertGreaterThan(parameters.BackBufferWidth, 0)
        XCTAssertGreaterThan(parameters.BackBufferHeight, 0)
        XCTAssertNotNil(game.adapter, "the device records the adapter it uses")
        XCTAssertTrue(game.adapter === game.adapterAgain,
                      "and records it once, not per read")

        // The one fallible accessor of the group.
        XCTAssertNotNil(game.displayMode)
    }

    /// A returning `Present` means the device took the request, and nothing
    /// more: this host has no display, which is the bound Foundation 68's
    /// draws live inside.
    func testPresentAndResetAreAccepted() throws {
        let game = try DeviceProbeGame(exercise: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.presented, true)
        XCTAssertEqual(game.reset, true)
    }

    /// Resetting with new parameters replaces what the property reports, and
    /// only after the reset is accepted.
    /// The device **records** the adapter it was reset with, and the reset
    /// **carries that adapter's index** to the route.
    ///
    /// Both need a second adapter to be observable at all, and this host
    /// reports one — so the test builds a distinct adapter through the
    /// internal initializer. Two mutations survived without it: one replacing
    /// the recorded adapter with a fresh lookup, one passing null where the
    /// index belongs. With one adapter neither changes an answer; with an
    /// index the runtime does not have, both do.
    func testTheResetCarriesTheAdapterAndItsIndex() throws {
        let game = try DeviceProbeGame(resetWithForeignAdapter: true)
        try game.Run()
        if let failure = game.failure { throw failure }

        // An index no adapter has must be refused, which is only possible if
        // the index actually reaches the route.
        XCTAssertNotNil(game.foreignResetFailure,
                        "a reset naming adapter 99 must be refused")

        // And a reset that IS accepted records the adapter it was given.
        XCTAssertTrue(game.adapterAfterReset === game.passedAdapter,
                      "the device reports the adapter the reset carried")
    }

    func testResetWithParametersReplacesTheReportedOnes() throws {
        let game = try DeviceProbeGame(resetWithParameters: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.widthAfterReset, 320,
                       "the parameters the reset accepted are the ones reported")
    }
}

private final class DeviceProbeGame: Microsoft.Xna.Framework.Game {
    let exercise: Bool
    let resetWithParameters: Bool
    let resetWithForeignAdapter: Bool
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var failure: Error?
    var parameters: Microsoft.Xna.Framework.Graphics.PresentationParameters?
    var adapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?
    var adapterAgain: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?
    var displayMode: Microsoft.Xna.Framework.Graphics.DisplayMode?
    var presented: Bool?
    var reset: Bool?
    var widthAfterReset: Int32?
    var foreignResetFailure: Error?
    var passedAdapter: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?
    var adapterAfterReset: Microsoft.Xna.Framework.Graphics.GraphicsAdapter?

    init(exercise: Bool = false, resetWithParameters: Bool = false,
         resetWithForeignAdapter: Bool = false) throws {
        self.exercise = exercise
        self.resetWithParameters = resetWithParameters
        self.resetWithForeignAdapter = resetWithForeignAdapter
        try super.init()
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        defer { try? Exit() }
        do {
            guard let device = try GraphicsDevice else { return }
            parameters = device.PresentationParameters
            adapter = device.Adapter
            adapterAgain = device.Adapter
            displayMode = try device.DisplayMode

            if exercise {
                try device.Present()
                presented = true
                try device.Reset()
                reset = true
            }
            if resetWithParameters, let existing = device.PresentationParameters {
                let replacement = Microsoft.Xna.Framework.Graphics
                    .PresentationParameters()
                replacement.BackBufferWidth = 320
                replacement.BackBufferHeight = existing.BackBufferHeight
                replacement.BackBufferFormat = existing.BackBufferFormat
                replacement.DepthStencilFormat = existing.DepthStencilFormat
                try device.Reset(replacement)
                widthAfterReset = device.PresentationParameters?.BackBufferWidth
            }
            if resetWithForeignAdapter, let existing = device.PresentationParameters {
                // An adapter whose index the runtime does not have. Building it
                // needs the internal initializer, which is why this test is in
                // the @testable suite rather than the archive canary.
                let foreign = Microsoft.Xna.Framework.Graphics.GraphicsAdapter(
                    adapterIndex: 99, deviceName: "absent", description: "absent",
                    vendorId: 0, deviceId: 0, revision: 0, subSystemId: 0,
                    isDefaultAdapter: false, isWideScreen: false,
                    currentDisplayMode: nil, supportedDisplayModes: nil,
                    profileSupport: [:])
                do {
                    try device.Reset(existing, graphicsAdapter: foreign)
                } catch {
                    foreignResetFailure = error
                }
                // Now one the runtime does have, and the device must report it.
                let real = Microsoft.Xna.Framework.Graphics.GraphicsAdapter(
                    adapterIndex: 0, deviceName: "recorded", description: "recorded",
                    vendorId: 1, deviceId: 1, revision: 0, subSystemId: 0,
                    isDefaultAdapter: true, isWideScreen: false,
                    currentDisplayMode: nil, supportedDisplayModes: nil,
                    profileSupport: [:])
                passedAdapter = real
                try device.Reset(existing, graphicsAdapter: real)
                adapterAfterReset = device.Adapter
            }
        } catch {
            failure = error
        }
    }
}
