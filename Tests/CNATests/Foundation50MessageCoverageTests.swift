// SPDX-License-Identifier: MIT

import Foundation
import XCTest
import CNAShim
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private let invalidOperationHResult = Int32(bitPattern: 0x8013_1509)

/// A drawable that records nothing; it exists only to be asked for a device.
private final class BareDrawable: F.DrawableGameComponent {}

/// A game whose body runs inside `LoadContent`, where a device exists.
private final class BatchProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((BatchProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (BatchProbeGame, G.GraphicsDevice) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try F.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        do {
            guard let device = try GraphicsDevice else {
                throw CNAError.producerInvariant("no device")
            }
            try body?(self, device)
        } catch {
            failure = error
        }
        try Exit()
    }

    override func Update(_ gameTime: F.GameTime) throws { try Exit() }
}

/// Foundation 50: the messages implemented members raise.
///
/// `SpriteBatch`'s begin/end rule was enforced only by CNA, which answers
/// `CNA_RESULT_INVALID_STATE` — a runtime-channel failure for a CLR-channel
/// mistake. `DrawableGameComponent.GraphicsDevice` raised its sibling's
/// message.
final class Foundation50MessageCoverageTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (BatchProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> BatchProbeGame {
        let game = try BatchProbeGame(body)
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
    // SpriteBatch's begin/end rule.
    // ------------------------------------------------------------------

    /// A second `Begin` inside a pair is an XNA rule violation, so it raises
    /// XNA's exception with XNA's message — not the native refusal that also
    /// stands behind it.
    func testASecondBeginRaisesXnasOwnException() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            try batch.Begin()
            do {
                try batch.Begin()
                game.observations["threw"] = "nothing"
            } catch let error as CNAException {
                game.observations["threw"] = String(describing: Swift.type(of: error))
                game.observations["message"] = error.Message
                game.observations["hResult"] = String(error.HResult)
            }
            try batch.End()
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["threw"], "CNAInvalidOperationException")
        XCTAssertEqual(
            game.observations["message"],
            "Begin cannot be called again until End has been successfully called.")
        XCTAssertEqual(game.observations["hResult"], String(invalidOperationHResult))
    }

    /// `End` outside a pair, likewise.
    func testEndWithoutBeginRaisesXnasOwnException() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            do {
                try batch.End()
                game.observations["threw"] = "nothing"
            } catch let error as CNAException {
                game.observations["threw"] = String(describing: Swift.type(of: error))
                game.observations["message"] = error.Message
            }
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["threw"], "CNAInvalidOperationException")
        XCTAssertEqual(
            game.observations["message"],
            "Begin must be called successfully before End can be called.")
    }

    /// `Draw` outside a pair raises the third message, which is a different
    /// string from the other two and says "a Draw".
    func testDrawWithoutBeginRaisesItsOwnMessage() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let texture = try G.RenderTarget2D(
                graphicsDevice: device, width: 8, height: 8)
            do {
                try batch.Draw(texture, position: .Zero, color: .White)
                game.observations["threw"] = "nothing"
            } catch let error as CNAException {
                game.observations["threw"] = String(describing: Swift.type(of: error))
                game.observations["message"] = error.Message
            }
            try texture.Dispose()
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["threw"], "CNAInvalidOperationException")
        XCTAssertEqual(
            game.observations["message"],
            "Begin must be called successfully before a Draw can be called.")
    }

    /// The three messages are three different strings. XNA has three keys and
    /// this projection used one message at two sites until Foundation 50, so
    /// the distinctness is asserted rather than assumed.
    func testTheThreeBatchMessagesAreDistinct() {
        let all = Set([
            endMustBeCalledBeforeBeginMessage,
            beginMustBeCalledBeforeEndMessage,
            beginMustBeCalledBeforeDrawMessage,
        ])
        XCTAssertEqual(all.count, 3)
    }

    /// The flag closes as well as opens: a completed pair leaves the batch
    /// ready for the next one, and a `Draw` inside the second pair is fine.
    func testAClosedPairCanBeReopened() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let texture = try G.RenderTarget2D(
                graphicsDevice: device, width: 8, height: 8)
            try batch.Begin()
            try batch.End()
            try batch.Begin()
            try batch.Draw(texture, position: .Zero, color: .White)
            try batch.End()
            game.observations["completed"] = "true"
            try texture.Dispose()
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["completed"], "true")
    }

    /// CNA enforces the same sequence itself. The projection's flag is not
    /// what prevents the bad call — it is what makes the failure XNA's rather
    /// than a native result code, so this pins that the native route still
    /// refuses underneath.
    func testTheNativeRouteRefusesTheSameSequence() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let handle = try batch.validatedHandle("test")
            let runtime = batch.nativeStorage.runtime
            // Through the route the projection itself uses. The plain
            // `cna_sprite_batch_begin` was unbound in Foundation 54 once no
            // member consumed it, and a test is not a member.
            var blend = G.BlendState.AlphaBlend.nativeDescriptor()
            var sampler = G.SamplerState.LinearClamp.nativeDescriptor()
            var depth = G.DepthStencilState.None.nativeDescriptor()
            var raster = G.RasterizerState.CullCounterClockwise.nativeDescriptor()
            func begin() -> UInt32 {
                runtime.functions.spriteBatchBeginWithStates(
                    handle, G.SpriteSortMode.Deferred.rawValue,
                    &blend, &sampler, &depth, &raster)
            }
            game.observations["firstBegin"] = String(begin())
            game.observations["secondBegin"] = String(begin())
            game.observations["end"] = String(runtime.functions.spriteBatchEnd(handle))
            game.observations["secondEnd"] = String(runtime.functions.spriteBatchEnd(handle))
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["firstBegin"], "0")
        XCTAssertNotEqual(game.observations["secondBegin"], "0",
                          "CNA refuses a second begin on its own")
        XCTAssertEqual(game.observations["end"], "0")
        XCTAssertNotEqual(game.observations["secondEnd"], "0",
                          "and an end outside an interval")
    }

    /// XNA's begin/end checks come **first**: `Begin`, `End` and
    /// `InternalDraw` open with the flag test and none of them calls
    /// `Helpers.CheckDisposed` at all. The order is observable on a disposed
    /// batch, which is why it is projected rather than approximated.
    func testTheRuleIsCheckedBeforeTheHandle() throws {
        try requireNative()
        let game = try run { game, device in
            let opened = try G.SpriteBatch(graphicsDevice: device)
            try opened.Begin()
            try opened.Dispose()
            do {
                try opened.Begin()
                game.observations["disposedInsideAPair"] = "accepted"
            } catch let error as CNAException {
                game.observations["disposedInsideAPair"] = error.Message
            }

            let closed = try G.SpriteBatch(graphicsDevice: device)
            try closed.Dispose()
            do {
                try closed.End()
                game.observations["disposedOutsideAPair"] = "accepted"
            } catch let error as CNAException {
                game.observations["disposedOutsideAPair"] = error.Message
            }
        }
        XCTAssertEqual(
            game.observations["disposedInsideAPair"],
            "Begin cannot be called again until End has been successfully called.",
            "the pair rule is decided before the handle is looked at")
        XCTAssertEqual(
            game.observations["disposedOutsideAPair"],
            "Begin must be called successfully before End can be called.")
    }

    // ------------------------------------------------------------------
    // DrawableGameComponent's two messages.
    // ------------------------------------------------------------------

    /// `GraphicsDevice` before `Initialize` raises
    /// `PropertyCannotBeCalledBeforeInitialize` — **not** the
    /// `MissingGraphicsDeviceService` its sibling raises.
    func testTheDeviceGetterRaisesItsOwnMessage() throws {
        try requireNative()
        let game = try F.Game()
        defer { try? game.Dispose() }
        let component = BareDrawable(game: game)
        assertProjected(
            CNAInvalidOperationException.self,
            message: "The GraphicsDevice property cannot be used before "
                + "Initialize has been called.",
            hResult: invalidOperationHResult
        ) {
            _ = try component.GraphicsDevice
        }
    }

    /// All three of this family's messages are distinct strings: the getter's,
    /// `Initialize`'s, and `Game.GraphicsDevice`'s.
    func testTheThreeDeviceServiceMessagesAreDistinct() {
        let all = Set([
            F.DrawableGameComponent.propertyCannotBeCalledBeforeInitializeMessage,
            F.DrawableGameComponent.missingGraphicsDeviceServiceMessage,
            F.Game.noGraphicsDeviceServiceMessage,
        ])
        XCTAssertEqual(all.count, 3, "XNA has three keys here, not one")
    }
}
