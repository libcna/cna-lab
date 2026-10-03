// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// `Microsoft.Xna.Framework.GamerServices.GamerServicesComponent`.
///
/// A `GameComponent` whose whole job is to pump the gamer-services dispatcher.
/// It is the first type in its namespace, and the only one XNA's contract
/// declares there.
final class Foundation98GamerServicesTests: XCTestCase {

    /// It is a `GameComponent` first: the base's own surface has to work, or
    /// adding one to a game's component collection would not.
    func testItIsAGameComponent() throws {
        let game = try GamerProbeGame()
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.ownerIsTheGame, "the component carries its game")
        XCTAssertEqual(game.enabledByDefault, true)
        XCTAssertEqual(game.updateOrderByDefault, 0)
    }

    /// `Initialize` and `Update` reach the dispatcher. Either can refuse on a
    /// host with no gamer services, and a refusal is recorded rather than
    /// hidden -- what must not happen is silence.
    func testInitializeAndUpdateReachTheDispatcher() throws {
        let game = try GamerProbeGame(pump: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.initializeRan, "Initialize returned or refused")
        XCTAssertTrue(game.updateRan, "Update returned or refused")
        // **Measured: both routes accept on this host.** That is asserted
        // rather than printed, because a route that started refusing is worth
        // failing on -- but it does NOT distinguish a component that pumps the
        // dispatcher from one that does nothing. Nothing can: the Windows
        // profile declares no GamerServicesDispatcher, so its
        // `get_is_initialized` has no consuming member and cannot be bound.
        // The two mutations that would have caught it are withdrawn with that
        // reason in place.
        XCTAssertEqual(game.initializeOutcome, "accepted")
        XCTAssertEqual(game.updateOutcome, "accepted")
    }

    /// **The `gameTime` is ignored, and that is XNA's own shape.**
    /// `GamerServicesComponent.Update` calls `GamerServicesDispatcher.Update()`
    /// with no argument at all, so a caller passing a different time sees the
    /// same behaviour -- which is asserted here rather than assumed.
    func testUpdateIgnoresTheGameTimeItIsGiven() throws {
        let game = try GamerProbeGame(pumpTwiceWithDifferentTimes: true)
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.firstUpdateOutcome, game.secondUpdateOutcome,
                       "two different game times, one behaviour")
    }
}

private final class GamerProbeGame: Microsoft.Xna.Framework.Game {
    let pump: Bool
    let pumpTwiceWithDifferentTimes: Bool

    var failure: Error?
    var pumpFailure: Error?
    var ownerIsTheGame = false
    var enabledByDefault: Bool?
    var updateOrderByDefault: Int32?
    var initializeRan = false
    var updateRan = false
    var initializeOutcome: String?
    var updateOutcome: String?
    var firstUpdateOutcome: String?
    var secondUpdateOutcome: String?

    init(pump: Bool = false, pumpTwiceWithDifferentTimes: Bool = false) throws {
        self.pump = pump
        self.pumpTwiceWithDifferentTimes = pumpTwiceWithDifferentTimes
        try super.init()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        typealias G = Microsoft.Xna.Framework.GamerServices
        do {
            let component = G.GamerServicesComponent(game: self)
            ownerIsTheGame = component.Game === self
            enabledByDefault = component.Enabled
            updateOrderByDefault = component.UpdateOrder

            func outcome(_ body: () throws -> Void) -> String {
                do { try body(); return "accepted" }
                catch { return "\(error)" }
            }

            if pumpTwiceWithDifferentTimes {
                firstUpdateOutcome = outcome {
                    try component.Update(Microsoft.Xna.Framework.GameTime())
                }
                secondUpdateOutcome = outcome { try component.Update(gameTime) }
                return
            }

            if pump {
                initializeOutcome = outcome { try component.Initialize() }
                initializeRan = true
                updateOutcome = outcome { try component.Update(gameTime) }
                updateRan = true
            }
        } catch {
            failure = error
        }
    }
}
