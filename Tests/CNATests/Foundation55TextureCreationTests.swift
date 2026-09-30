// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class TextureProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((TextureProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (TextureProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 55: the two `Texture2D` constructors and the validation that
/// precedes them.
final class Foundation55TextureCreationTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (TextureProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> TextureProbeGame {
        let game = try TextureProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    private let zeroSizeMessage = "Resource size must be greater than zero."

    /// The three-parameter constructor is the five-parameter one with two
    /// literals: `mipMap` false and `format` `SurfaceFormat.Color`, both
    /// `ldc.i4.0` in the forwarding IL.
    func testTheShortConstructorIsTheLongOneWithTwoLiterals() throws {
        try requireNative()
        let game = try run { game, device in
            let short = try G.Texture2D(graphicsDevice: device, width: 8, height: 4)
            let long = try G.Texture2D(graphicsDevice: device, width: 8, height: 4,
                                       mipMap: false, format: .Color)
            game.observations["width"] = String(short.Width)
            game.observations["height"] = String(short.Height)
            game.observations["format"] = String(describing: short.Format)
            game.observations["levels"] = String(short.LevelCount)
            game.observations["sameFormat"] =
                String(short.Format == long.Format)
            game.observations["sameLevels"] =
                String(short.LevelCount == long.LevelCount)
            game.observations["bounds"] = String(describing: short.Bounds)
            try short.Dispose()
            try long.Dispose()
        }
        XCTAssertEqual(game.observations["width"], "8")
        XCTAssertEqual(game.observations["height"], "4")
        XCTAssertEqual(game.observations["format"], "Color")
        XCTAssertEqual(game.observations["sameFormat"], "true")
        XCTAssertEqual(game.observations["sameLevels"], "true")
    }

    /// A non-positive dimension is refused before anything native is asked,
    /// with the width tested first — so a texture invalid in both dimensions
    /// blames the width.
    func testANonPositiveDimensionIsRefused() throws {
        try requireNative()
        let game = try run { game, device in
            func attempt(_ w: Int32, _ h: Int32) -> String {
                do {
                    _ = try G.Texture2D(graphicsDevice: device, width: w, height: h)
                    return "accepted"
                } catch let error as CNAArgumentOutOfRangeException {
                    return error.ParamName ?? "no name"
                } catch {
                    return "other"
                }
            }
            game.observations["zeroWidth"] = attempt(0, 4)
            game.observations["negativeWidth"] = attempt(-1, 4)
            game.observations["zeroHeight"] = attempt(4, 0)
            game.observations["bothInvalid"] = attempt(0, 0)
            do {
                _ = try G.Texture2D(graphicsDevice: device, width: 0, height: 4)
            } catch let error as CNAArgumentOutOfRangeException {
                game.observations["message"] = error.Message
            }
        }
        XCTAssertEqual(game.observations["zeroWidth"], "width")
        XCTAssertEqual(game.observations["negativeWidth"], "width")
        XCTAssertEqual(game.observations["zeroHeight"], "height")
        XCTAssertEqual(game.observations["bothInvalid"], "width",
                       "the width is tested first")
        XCTAssertEqual(
            game.observations["message"],
            zeroSizeMessage + "\r\nParameter name: width")
    }

    /// The granted values come back from CNA, not from the request, exactly as
    /// `RenderTarget2D` reads its own grants.
    func testTheGrantedValuesAreReadBackFromCna() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(
                graphicsDevice: device, width: 16, height: 16,
                mipMap: true, format: .Color)
            // CNA grants the requested width and height exactly -- 7x3 and
            // 5000x5000 alike, measured in `build-probe/f55_grants.c` -- so
            // the dimensions cannot tell a projection that reports the grant
            // from one that reports the request. The level count can: a mipped
            // 16x16 comes back with five levels, and nothing in the request
            // says five.
            game.observations["levels"] = String(texture.LevelCount)
            game.observations["width"] = String(texture.Width)
            game.observations["isTexture"] = String((texture as Any) is G.Texture)
            game.observations["isResource"] = String((texture as Any) is G.GraphicsResource)
            try texture.Dispose()
            game.observations["disposed"] = String(texture.IsDisposed)
        }
        XCTAssertEqual(game.observations["width"], "16")
        XCTAssertEqual(game.observations["isTexture"], "true")
        XCTAssertEqual(game.observations["isResource"], "true")
        XCTAssertEqual(game.observations["disposed"], "true")
        XCTAssertEqual(game.observations["levels"], "5",
                       "log2(16) + 1 levels, granted by CNA and not asked for")
    }

    /// A created texture is a real sprite source: it draws through the batch
    /// like any other, which is what a texture is for.
    func testACreatedTextureCanBeDrawn() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 8, height: 8)
            let batch = try G.SpriteBatch(graphicsDevice: device)
            try batch.Begin()
            try batch.Draw(texture, position: .Zero, color: .White)
            try batch.Draw(texture, destinationRectangle: F.Rectangle(0, 0, 16, 16),
                           color: .White)
            try batch.End()
            game.observations["drawn"] = "true"
            try batch.Dispose()
            try texture.Dispose()
        }
        XCTAssertEqual(game.observations["drawn"], "true")
    }
}
