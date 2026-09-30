// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class DynamicProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((DynamicProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (DynamicProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 61: `DynamicVertexBuffer` and `DynamicIndexBuffer`.
///
/// The contents round-trip exactly as the static buffers' do. What cannot be
/// asserted is the `ContentLost` event *firing*: HEADLESS is one of the
/// renderer families CNA documents as unable to lose a device, so the
/// subscription is made, held and released, and the flag stays false. That is
/// the same shape `RenderTarget2D.ContentLost` records, and it is asserted as
/// a subscription rather than as an event.
final class Foundation61DynamicBufferTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (DynamicProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> DynamicProbeGame {
        let game = try DynamicProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    private func vertices(_ values: [(Float, UInt8)]) -> [G.VertexPositionColor] {
        values.map {
            G.VertexPositionColor(
                F.Vector3($0.0, $0.0, $0.0),
                F.Color(Int32($0.1), Int32($0.1), Int32($0.1), 255))
        }
    }

    private func describe(_ data: [G.VertexPositionColor]) -> String {
        data.map { "\($0.Position.X),\($0.Color.R)" }.joined(separator: " ")
    }

    /// A dynamic buffer is a `VertexBuffer`, and everything the base does still
    /// works through it.
    func testADynamicVertexBufferIsAVertexBuffer() throws {
        try requireNative()
        let written = vertices([(1, 10), (2, 20), (3, 30), (4, 40)])
        let game = try run { game, device in
            let buffer = try G.DynamicVertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            let asVertexBuffer: G.VertexBuffer = buffer
            game.observations["is a vertex buffer"] =
            "\(asVertexBuffer === buffer)"
            game.observations["count"] = "\(buffer.VertexCount)"
            game.observations["declaration"] =
                "\(buffer.VertexDeclaration === G.VertexPositionColor.VertexDeclaration)"
            try buffer.SetData(written)
            var read = [G.VertexPositionColor](
                repeating: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.Transparent),
                count: 4)
            try buffer.GetData(&read)
            game.observations["read"] = self.describe(read)
            game.observations["content lost"] = "\(buffer.IsContentLost)"
            game.observations["subscribed"] = "\(buffer.contentLostRegistration != 0)"
            try buffer.Dispose()
            game.observations["released"] = "\(buffer.contentLostRegistration == 0)"
        }
        XCTAssertEqual(game.observations["is a vertex buffer"], "true")
        XCTAssertEqual(game.observations["count"], "4")
        XCTAssertEqual(game.observations["declaration"], "true")
        XCTAssertEqual(game.observations["read"], describe(written))
        XCTAssertEqual(game.observations["content lost"], "false")
        XCTAssertEqual(game.observations["subscribed"], "true")
        XCTAssertEqual(game.observations["released"], "true")
    }

    /// Every `SetDataOptions` value reaches the buffer and writes what it was
    /// given.
    ///
    /// The flag is a driver hint; what is asserted is that none of the three
    /// changes the bytes, and that all three are accepted where XNA accepts
    /// them. A projection that sent an option-taking upload to a *static*
    /// buffer would fail outright — CNA answers `NOT_SUPPORTED` for every
    /// option value there, `None` included.
    func testEveryOptionWritesWhatItWasGiven() throws {
        try requireNative()
        let game = try run { game, device in
            let buffer = try G.DynamicVertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 2, usage: .None)
            for (name, option) in [("none", G.SetDataOptions.None),
                                   ("discard", G.SetDataOptions.Discard),
                                   ("nooverwrite", G.SetDataOptions.NoOverwrite)] {
                let written = self.vertices([(9, 90), (8, 80)])
                try buffer.SetData(written, startIndex: 0, elementCount: 2,
                                   options: option)
                var read = [G.VertexPositionColor](
                    repeating: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.Transparent),
                    count: 2)
                try buffer.GetData(&read)
                game.observations[name] = self.describe(read)
            }
            try buffer.Dispose()

            // The declaration-taking constructor as well, because it is a
            // separate `dynamic: true` and a projection that got only one of
            // them right would still refuse every option here.
            let declared = try G.DynamicVertexBuffer(
                graphicsDevice: device,
                vertexDeclaration: G.VertexPositionColor.VertexDeclaration,
                vertexCount: 2, usage: .None)
            try declared.SetData(self.vertices([(5, 50), (6, 60)]),
                                 startIndex: 0, elementCount: 2, options: .Discard)
            var read = [G.VertexPositionColor](
                repeating: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.Transparent),
                count: 2)
            try declared.GetData(&read)
            game.observations["declared"] = self.describe(read)
            try declared.Dispose()
        }
        XCTAssertEqual(game.observations["declared"], "5.0,50 6.0,60")
        XCTAssertEqual(game.observations["none"], "9.0,90 8.0,80")
        XCTAssertEqual(game.observations["discard"], "9.0,90 8.0,80")
        XCTAssertEqual(game.observations["nooverwrite"], "9.0,90 8.0,80")
    }

    /// The six-argument overload carries the byte offset through, options and all.
    func testTheOffsetOverloadStillWindowsTheBuffer() throws {
        try requireNative()
        let game = try run { game, device in
            let buffer = try G.DynamicVertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 3, usage: .None)
            try buffer.SetData(self.vertices([(1, 11), (2, 22), (3, 33)]))
            try buffer.SetData(16, data: self.vertices([(7, 77)]), startIndex: 0,
                               elementCount: 1, vertexStride: 0, options: .Discard)
            var read = [G.VertexPositionColor](
                repeating: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.Transparent),
                count: 3)
            try buffer.GetData(&read)
            game.observations["read"] = self.describe(read)
            try buffer.Dispose()
        }
        XCTAssertEqual(game.observations["read"], "1.0,11 7.0,77 3.0,33")
    }

    /// The index side, at both widths and through both option overloads.
    func testDynamicIndexBufferRoundTripsWithOptions() throws {
        try requireNative()
        let game = try run { game, device in
            let buffer = try G.DynamicIndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 6, usage: .None)
            // Typed as the base, which only compiles because it derives
            // from it. The recorded fact is the one that can differ.
            let asIndexBuffer: G.IndexBuffer = buffer
            game.observations["is an index buffer"] =
            "\(asIndexBuffer === buffer)"
            try buffer.SetData([Int16(1), 2, 3, 4, 5, 6], startIndex: 0,
                               elementCount: 6, options: .Discard)
            var read = [Int16](repeating: 0, count: 6)
            try buffer.GetData(&read)
            game.observations["whole"] = read.map(String.init).joined(separator: " ")

            try buffer.SetData(4, data: [Int16(77), 88], startIndex: 0,
                               elementCount: 2, options: .NoOverwrite)
            try buffer.GetData(&read)
            game.observations["window"] = read.map(String.init).joined(separator: " ")
            game.observations["content lost"] = "\(buffer.IsContentLost)"
            game.observations["subscribed"] = "\(buffer.contentLostRegistration != 0)"
            try buffer.Dispose()
            game.observations["released"] = "\(buffer.contentLostRegistration == 0)"

            // typeof(ushort), because Reach refuses 32-bit indices -- the
            // capability table says so and Foundation60BufferTests asserts the
            // refusal itself.
            let byType = try G.DynamicIndexBuffer(
                graphicsDevice: device, indexType: UInt16.self,
                indexCount: 3, usage: .None)
            game.observations["by type"] =
                "\(byType.IndexElementSize == G.IndexElementSize.SixteenBits)"
            try byType.Dispose()
        }
        XCTAssertEqual(game.observations["is an index buffer"], "true")
        XCTAssertEqual(game.observations["whole"], "1 2 3 4 5 6")
        XCTAssertEqual(game.observations["window"], "1 2 77 88 5 6")
        XCTAssertEqual(game.observations["content lost"], "false")
        XCTAssertEqual(game.observations["subscribed"], "true")
        XCTAssertEqual(game.observations["released"], "true")
        XCTAssertEqual(game.observations["by type"], "true")
    }

    /// Every XNA validation the base performs still runs through the dynamic
    /// overloads.
    func testTheDynamicOverloadsValidateLikeTheBase() throws {
        try requireNative()
        _ = try run { game, device in
            let buffer = try G.DynamicVertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 2, usage: .None)
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    G.GraphicsDevice.nullNotAllowedMessage, paramName: "data"),
                paramName: "data",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) {
                try buffer.SetData([G.VertexPositionColor](), startIndex: 0,
                                   elementCount: 0, options: .Discard)
            }
            assertProjected(
                CNAInvalidOperationException.self,
                message: G.BufferResources.resourceDataMustBeCorrectSize,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) {
                try buffer.SetData(self.vertices([(1, 1), (2, 2), (3, 3)]),
                                   startIndex: 0, elementCount: 3, options: .None)
            }
            game.observations["checked"] = "yes"
            try buffer.Dispose()
        }
    }

    /// Disposal releases the native subscription before the buffer, on both
    /// paths, so no callback can reach a released box.
    func testDisposalReleasesTheSubscriptionOnBothPaths() throws {
        try requireNative()
        let game = try run { game, device in
            let quiet = try G.DynamicVertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 2, usage: .None)
            var quietCount = 0
            _ = quiet.Disposing.Add { _, _ in quietCount += 1 }
            try quiet.Dispose(false)
            game.observations["finalizer"] =
                "\(quietCount) \(quiet.IsDisposed) \(quiet.contentLostRegistration == 0)"

            let loud = try G.DynamicIndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 2, usage: .None)
            var loudCount = 0
            _ = loud.Disposing.Add { _, _ in loudCount += 1 }
            try loud.Dispose(true)
            game.observations["disposing"] =
                "\(loudCount) \(loud.IsDisposed) \(loud.contentLostRegistration == 0)"
        }
        XCTAssertEqual(game.observations["finalizer"], "0 true true")
        XCTAssertEqual(game.observations["disposing"], "1 true true")
    }
}
