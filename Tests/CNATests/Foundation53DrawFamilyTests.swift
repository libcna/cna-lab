// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class DrawProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((DrawProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (DrawProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 53: the seven `SpriteBatch.Draw` overloads, and the split
/// between the two command shapes that carries them.
final class Foundation53DrawFamilyTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (DrawProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> DrawProbeGame {
        let game = try DrawProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    /// All seven overloads execute inside a begin/end pair, on both command
    /// shapes: the position family through `cna_sprite_batch_submit_scaled_many`
    /// and the destination family through `cna_sprite_batch_submit_many`.
    func testEveryDrawOverloadExecutes() throws {
        try requireNative()
        _ = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let texture = try G.RenderTarget2D(
                graphicsDevice: device, width: 16, height: 16)
            let source = F.Rectangle(0, 0, 8, 8)
            let destination = F.Rectangle(4, 4, 32, 32)
            try batch.Begin()
            try batch.Draw(texture, position: .Zero, color: .White)
            try batch.Draw(texture, position: .Zero, sourceRectangle: source,
                           color: .White)
            try batch.Draw(texture, position: .Zero, sourceRectangle: nil,
                           color: .White, rotation: 0, origin: .Zero,
                           scale: Float(2), effects: .None, layerDepth: 0)
            try batch.Draw(texture, position: .Zero, sourceRectangle: source,
                           color: .White, rotation: 0.5, origin: .Zero,
                           scale: F.Vector2(2, 3), effects: .FlipHorizontally,
                           layerDepth: 0.25)
            try batch.Draw(texture, destinationRectangle: destination, color: .White)
            try batch.Draw(texture, destinationRectangle: destination,
                           sourceRectangle: source, color: .White)
            try batch.Draw(texture, destinationRectangle: destination,
                           sourceRectangle: source, color: .White, rotation: 0.5,
                           origin: .Zero, effects: .FlipVertically, layerDepth: 0.5)
            try batch.End()
            try texture.Dispose()
            try batch.Dispose()
        }
    }

    /// The destination family has no scale parameter: the rectangle is the
    /// size. A projection that folded it into the scaled command would have to
    /// invent a scale from a source size, and that is exactly what the two
    /// command shapes exist to avoid — so this pins that a destination draw
    /// works with **no** source rectangle at all, where a fabricated scale
    /// would have had nothing to divide by.
    func testADestinationDrawNeedsNoSourceRectangle() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let texture = try G.RenderTarget2D(
                graphicsDevice: device, width: 16, height: 16)
            try batch.Begin()
            try batch.Draw(texture, destinationRectangle: F.Rectangle(0, 0, 100, 40),
                           color: .White)
            try batch.End()
            game.observations["completed"] = "true"
            try texture.Dispose()
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["completed"], "true")
    }

    /// Every overload is guarded by the begin/end rule, including the new
    /// destination family, and every one raises XNA's message rather than a
    /// native result code.
    func testEveryOverloadIsGuardedByTheBeginEndRule() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let texture = try G.RenderTarget2D(
                graphicsDevice: device, width: 16, height: 16)
            var messages: Set<String> = []
            func record(_ body: () throws -> Void) {
                do { try body(); messages.insert("accepted") }
                catch let error as CNAInvalidOperationException {
                    messages.insert(error.Message)
                } catch { messages.insert("other: \(error)") }
            }
            record { try batch.Draw(texture, position: .Zero, color: .White) }
            record { try batch.Draw(texture, position: .Zero,
                                    sourceRectangle: nil, color: .White) }
            record { try batch.Draw(texture,
                                    destinationRectangle: F.Rectangle(0, 0, 4, 4),
                                    color: .White) }
            record { try batch.Draw(texture,
                                    destinationRectangle: F.Rectangle(0, 0, 4, 4),
                                    sourceRectangle: nil, color: .White,
                                    rotation: 0, origin: .Zero, effects: .None,
                                    layerDepth: 0) }
            game.observations["distinct"] = String(messages.count)
            game.observations["message"] = messages.first ?? ""
            try texture.Dispose()
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["distinct"], "1",
                       "every overload answers the same way outside a pair")
        XCTAssertEqual(
            game.observations["message"],
            "Begin must be called successfully before a Draw can be called.")
    }
}
