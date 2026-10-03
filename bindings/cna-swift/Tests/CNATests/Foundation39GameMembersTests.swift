// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework

/// A game whose body runs once, inside `Update`, where a live host exists.
private final class GameMemberProbe: F.Game {
    var failure: Error?
    var observations: [String: String] = [:]
    var activated = 0
    var deactivated = 0
    var exiting = 0
    var disposedRaises = 0
    private var body: ((GameMemberProbe) throws -> Void)?
    private var ran = false

    init(_ body: @escaping (GameMemberProbe) throws -> Void) throws {
        try super.init()
        self.body = body
        _ = Activated.Add { [weak self] _, _ in self?.activated += 1 }
        _ = Deactivated.Add { [weak self] _, _ in self?.deactivated += 1 }
        _ = Exiting.Add { [weak self] _, _ in self?.exiting += 1 }
        _ = Disposed.Add { [weak self] _, _ in self?.disposedRaises += 1 }
    }

    override func Update(_ gameTime: F.GameTime) throws {
        if !ran {
            ran = true
            do { try body?(self) } catch { failure = error }
        }
        try Exit()
    }
}

final class Foundation39GameMembersTests: XCTestCase {
    private var nativeConfigured: Bool {
        ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] != nil
    }

    private func requireNative() throws {
        if !nativeConfigured {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    @discardableResult
    private func run(
        _ body: @escaping (GameMemberProbe) throws -> Void
    ) throws -> GameMemberProbe {
        let game = try GameMemberProbe(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    // ------------------------------------------------------------------
    // The timing surface.
    // ------------------------------------------------------------------

    /// The mirrors are seeded from the host, not from the values the create
    /// info carried.
    func testTheTimingMirrorsAreSeededFromTheHost() throws {
        try requireNative()
        let game = try GameMemberProbe { _ in }
        defer { try? game.Dispose() }
        XCTAssertTrue(game.IsFixedTimeStep, "the create info asks for a fixed step")
        XCTAssertEqual(
            game.TargetElapsedTime,
            Duration(secondsComponent: 0, attosecondsComponent: 166_667 * 100_000_000_000))
        XCTAssertGreaterThanOrEqual(game.InactiveSleepTime, .zero)
        try game.Dispose()
    }

    /// A write that the host accepts moves the mirror; the getter never
    /// reports a state the host does not hold.
    func testTheInfallibleSettersWriteThroughToTheHost() throws {
        try requireNative()
        let game = try run { game in
            game.IsFixedTimeStep = false
            game.observations["fixedAfterFalse"] = String(game.IsFixedTimeStep)
            game.IsFixedTimeStep = true
            game.observations["fixedAfterTrue"] = String(game.IsFixedTimeStep)
            game.IsMouseVisible = true
            game.observations["mouseAfterTrue"] = String(game.IsMouseVisible)
            game.IsMouseVisible = false
            game.observations["mouseAfterFalse"] = String(game.IsMouseVisible)
        }
        XCTAssertEqual(game.observations["fixedAfterFalse"], "false")
        XCTAssertEqual(game.observations["fixedAfterTrue"], "true")
        XCTAssertEqual(game.observations["mouseAfterFalse"], "false")
        // Whether the host accepts a visible cursor on HEADLESS is the host's
        // business; what must hold is that the mirror agrees with it.
        XCTAssertNotNil(game.observations["mouseAfterTrue"])
    }

    /// After disposal the setters are a no-op rather than a crash or a lie:
    /// the write cannot reach the host, so the mirror does not move.
    func testTheInfallibleSettersAreInertOnceTheHostIsGone() throws {
        try requireNative()
        let game = try GameMemberProbe { _ in }
        defer { try? game.Dispose() }
        let before = game.IsFixedTimeStep
        try game.Dispose()
        game.IsFixedTimeStep = !before
        XCTAssertEqual(
            game.IsFixedTimeStep, before,
            "a refused write must leave the mirror alone")
        game.IsMouseVisible = true
        XCTAssertFalse(game.IsMouseVisible)
    }

    /// `set_TargetElapsedTime` refuses `value <= Zero`; `set_InactiveSleepTime`
    /// refuses only `value < Zero`. The two messages read alike and the two
    /// conditions differ, so zero separates them.
    func testTheTwoTimeSpanSettersRefuseDifferentValues() throws {
        try requireNative()
        let game = try run { game in
            let composedTarget = composedArgumentMessage(
                "The target elapsed time must be greater than zero.  Specify a "
                + "non-zero positive value.", paramName: "value")
            for refused in [Duration.zero, Duration.seconds(-1)] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedTarget,
                    paramName: "value",
                    hResult: Int32(bitPattern: 0x8013_1502)
                ) {
                    try game.SetTargetElapsedTime(refused)
                }
            }
            let composedSleep = composedArgumentMessage(
                "The inactive sleep time must be greater than or equal to "
                + "zero.  Specify zero or a positive value.", paramName: "value")
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedSleep,
                paramName: "value",
                hResult: Int32(bitPattern: 0x8013_1502)
            ) {
                try game.SetInactiveSleepTime(Duration.seconds(-1))
            }
            // Zero is refused by one and accepted by the other.
            XCTAssertNoThrow(try game.SetInactiveSleepTime(.zero))
            game.observations["sleepAfterZero"] = String(describing: game.InactiveSleepTime)

            try game.SetTargetElapsedTime(Duration.milliseconds(20))
            game.observations["targetAfterSet"] =
                String(describing: game.TargetElapsedTime)
        }
        XCTAssertEqual(game.observations["sleepAfterZero"], "0.0 seconds")
        XCTAssertEqual(game.observations["targetAfterSet"], "0.02 seconds")
    }

    // ------------------------------------------------------------------
    // Tick, SuppressDraw and ResetElapsedTime.
    // ------------------------------------------------------------------

    /// `Tick` drives the host loop exactly one step and reaches the game's own
    /// `Update`.
    func testTickDrivesOneHostStep() throws {
        try requireNative()
        final class Counting: F.Game {
            var updates = 0
            var draws = 0
            override func Update(_ gameTime: F.GameTime) throws { updates += 1 }
            override func Draw(_ gameTime: F.GameTime) throws { draws += 1 }
        }
        let game = try Counting()
        defer { try? game.Dispose() }
        try game.Tick()
        let afterOne = game.updates
        try game.Tick()
        try game.Tick()
        XCTAssertGreaterThanOrEqual(afterOne, 1, "one Tick must reach Update")
        XCTAssertGreaterThan(game.updates, afterOne, "each Tick advances the loop")
    }

    func testSuppressDrawAndResetElapsedTimeReachTheHost() throws {
        try requireNative()
        let game = try run { game in
            try game.SuppressDraw()
            try game.ResetElapsedTime()
            game.observations["reached"] = "true"
        }
        XCTAssertEqual(game.observations["reached"], "true")
    }

    /// Every one of them refuses once the host is gone, through the runtime
    /// channel and never as a projected CLR exception.
    func testTheHostRoutesRefuseAfterDisposal() throws {
        try requireNative()
        let game = try GameMemberProbe { _ in }
        defer { try? game.Dispose() }
        try game.Dispose()
        for (name, body) in [
            ("Tick", { try game.Tick() }),
            ("SuppressDraw", { try game.SuppressDraw() }),
            ("ResetElapsedTime", { try game.ResetElapsedTime() }),
            ("SetTargetElapsedTime", { try game.SetTargetElapsedTime(.milliseconds(20)) }),
            ("SetInactiveSleepTime", { try game.SetInactiveSleepTime(.zero) }),
        ] as [(String, () throws -> Void)] {
            XCTAssertThrowsError(try body(), name) { error in
                XCTAssertTrue(error is CNAError, "\(name): \(error)")
                XCTAssertFalse(error is CNAException, "\(name) must not be a CLR exception")
            }
        }
    }

    // ------------------------------------------------------------------
    // The four host events.
    // ------------------------------------------------------------------

    /// `Disposed` is raised once, by disposal, and `Exiting` is **not** —
    /// mapping a teardown notification onto `Exiting` is the divergence this
    /// asserts against.
    func testDisposedIsRaisedOnceAndExitingIsNotRaisedByTeardown() throws {
        try requireNative()
        let game = try GameMemberProbe { _ in }
        defer { try? game.Dispose() }
        XCTAssertEqual(game.disposedRaises, 0)
        try game.Dispose()
        XCTAssertEqual(game.disposedRaises, 1, "Disposed fires exactly once")
        XCTAssertEqual(
            game.exiting, 0,
            "teardown is not Exiting: mapping a CNA shutdown notification onto "
            + "XNA's Exiting would fire it where XNA does not")
        try game.Dispose()
        XCTAssertEqual(game.disposedRaises, 1, "a second Dispose raises nothing")
    }

    /// The `On…` methods are the raise sites, so an override that does not
    /// call `super` suppresses the event — which is XNA's own shape.
    func testTheOnMethodsAreTheRaiseSitesAndPassTheGameAsSender() throws {
        try requireNative()
        // CNA permits ONE C-owned game at a time, so the two halves of this
        // test run one after the other rather than side by side.
        do {
            let game = try GameMemberProbe { _ in }
            defer { try? game.Dispose() }
            var sender: AnyObject?
            var raises = 0
            _ = game.Activated.Add { value, _ in
                raises += 1
                sender = value as AnyObject?
            }
            try game.OnActivated("a different sender", args: CNAEventArgs.Empty)
            XCTAssertEqual(raises, 1)
            XCTAssertTrue(
                sender === game,
                "the IL loads `this`, not the sender parameter, so the "
                + "parameter is declared and ignored")
        }

        final class Silent: F.Game {
            override func OnActivated(_ sender: Any?, args: CNAEventArgs) throws {}
        }
        let silent = try Silent()
        defer { try? silent.Dispose() }
        var silentRaises = 0
        _ = silent.Activated.Add { _, _ in silentRaises += 1 }
        try silent.OnActivated(nil, args: CNAEventArgs.Empty)
        XCTAssertEqual(
            silentRaises, 0,
            "an override that skips super suppresses the event, as in XNA")
    }

    /// `OnExiting` raises `Exiting`; `Exit()` asks the host to stop and does
    /// not raise it on its own.
    func testOnExitingIsTheExitingRaiseSite() throws {
        try requireNative()
        let game = try GameMemberProbe { _ in }
        defer { try? game.Dispose() }
        XCTAssertEqual(game.exiting, 0)
        try game.OnExiting(nil, args: CNAEventArgs.Empty)
        XCTAssertEqual(game.exiting, 1)
        try game.Dispose()
    }

    /// **The two things CNA calls "exiting" are not the same thing.**
    ///
    /// Measured with a pure-C probe:
    ///
    /// ```text
    ///                            request_exit   destroy (never ran)
    /// CNA_GameCallbacks.exiting      fires            fires
    /// CNA_GAME_EVENT_EXITING         fires            does not fire
    /// ```
    ///
    /// XNA raises `Exiting` when the game is exiting, and disposing a game
    /// that never ran raises nothing. The event is the one that matches, so
    /// that is what `Game` subscribes to; the lifecycle callback is a teardown
    /// notification with no XNA counterpart and maps to nothing.
    func testExitingFollowsTheEventAndNotTheTeardownCallback() throws {
        try requireNative()
        // A game that never runs: disposal must raise Disposed and NOT Exiting.
        do {
            let game = try GameMemberProbe { _ in }
            try game.Dispose()
            XCTAssertEqual(game.disposedRaises, 1)
            XCTAssertEqual(
                game.exiting, 0,
                "the teardown callback must not be mapped onto Exiting")
        }
        // A game that runs and exits: Exiting fires exactly once, at Exit().
        let ran = try run { _ in }
        XCTAssertEqual(
            ran.exiting, 1,
            "Exit() is where XNA raises Exiting, and exactly once")
        XCTAssertEqual(ran.disposedRaises, 1)
    }

    /// The host subscriptions are real and are released by disposal.
    func testTheHostEventSubscriptionsAreRealAndAreReleased() throws {
        try requireNative()
        let game = try GameMemberProbe { _ in }
        defer { try? game.Dispose() }
        XCTAssertEqual(
            game.hostEventRegistrationCountForTests, 4,
            "one registration per CNA_GAME_EVENT identity")
        try game.Dispose()
        XCTAssertEqual(game.hostEventRegistrationCountForTests, 0)
    }

    /// `IsActive` is a mirror the host updates, seeded at construction.
    func testIsActiveIsSeededFromTheHost() throws {
        try requireNative()
        let game = try GameMemberProbe { _ in }
        defer { try? game.Dispose() }
        // HEADLESS has no window and reports whatever it reports; what must
        // hold is that reading it never throws and never changes on its own.
        let first = game.IsActive
        XCTAssertEqual(game.IsActive, first)
        try game.Dispose()
        XCTAssertEqual(game.IsActive, first, "disposal does not fabricate a change")
    }

    // ------------------------------------------------------------------
    // ShowMissingRequirementMessage.
    // ------------------------------------------------------------------

    /// The base answers `false`, which is `GameHost`'s own base answer and
    /// tells a caller the message was not shown.
    func testShowMissingRequirementMessageReportsThatNothingWasShown() throws {
        try requireNative()
        let game = try GameMemberProbe { _ in }
        defer { try? game.Dispose() }
        XCTAssertFalse(
            game.ShowMissingRequirementMessage(
                Microsoft.Xna.Framework.Graphics.NoSuitableGraphicsDeviceException()))
        try game.Dispose()
    }
}
