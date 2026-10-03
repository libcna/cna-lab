// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias G = Microsoft.Xna.Framework.Graphics
private typealias F = Microsoft.Xna.Framework

/// A game that runs the viewport/scissor slice inside `LoadContent`.
private final class BoundsProbeGame: Microsoft.Xna.Framework.Game {
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((BoundsProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (BoundsProbeGame, G.GraphicsDevice) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        do {
            guard let device = try GraphicsDevice else {
                throw CNAError.producerInvariant(
                    "the registered graphics device service produced no device")
            }
            try body?(self, device)
        } catch {
            failure = error
        }
        try Exit()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        try Exit()
    }
}

/// Foundation 49: the validation `set_Viewport` and `set_ScissorRectangle`
/// perform, which Foundation 47 projected without.
final class Foundation49ViewportScissorTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (BoundsProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> BoundsProbeGame {
        let game = try BoundsProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    /// Runs one closure per case and records, for each, whether it raised the
    /// pinned `ArgumentException` — so a case that throws the *wrong* error is
    /// as visible as one that throws nothing.
    private func outcomes(
        _ cases: [String: (G.GraphicsDevice) throws -> Void]
    ) throws -> [String: String] {
        let game = try run { game, device in
            for (name, body) in cases.sorted(by: { $0.key < $1.key }) {
                do {
                    try body(device)
                    game.observations[name] = "accepted"
                } catch let error as CNAArgumentException {
                    game.observations[name] = "ArgumentException|\(error.Message)"
                } catch {
                    game.observations[name] = "other|\(error)"
                }
            }
        }
        return game.observations
    }

    private let viewportMessage =
        "The viewport is invalid. The viewport cannot be larger than or "
        + "outside of the current render target bounds. The MinDepth and "
        + "MaxDepth must be between 0 and 1."
    private let scissorMessage =
        "The scissor rectangle is invalid. The scissor rectangle cannot be "
        + "larger than or outside of the current render target bounds."

    private func assertRejected(
        _ observations: [String: String], _ name: String, _ message: String,
        file: StaticString = #filePath, line: UInt = #line
    ) {
        // `ArgumentException.Message` composes the message, `\r\n` and
        // "Parameter name: <name>" — the .NET Framework 4.0 form the pinned
        // BCL IL carries, not .NET Core's "(Parameter '<name>')".
        XCTAssertEqual(
            observations[name],
            "ArgumentException|" + message + "\r\nParameter name: value",
            name, file: file, line: line)
    }

    // ------------------------------------------------------------------
    // set_Viewport: five branches, one message.
    // ------------------------------------------------------------------

    /// The backbuffer is 800x480, so a viewport that covers it exactly is the
    /// legal case every rejection below is measured against.
    func testTheFullBackbufferViewportIsAccepted() throws {
        try requireNative()
        let seen = try outcomes([
            "full": { try $0.SetViewport(G.Viewport(0, 0, 800, 480)) },
            "inset": { try $0.SetViewport(G.Viewport(10, 20, 100, 50)) },
            "emptyDepthRange": {
                var v = G.Viewport(0, 0, 800, 480)
                v.MinDepth = 0.5
                v.MaxDepth = 0.5      // bge.un accepts equality
                try $0.SetViewport(v)
            },
        ])
        XCTAssertEqual(seen["full"], "accepted")
        XCTAssertEqual(seen["inset"], "accepted")
        XCTAssertEqual(seen["emptyDepthRange"], "accepted")
    }

    /// A negative origin and a non-positive extent. The IL uses `blt` on the
    /// origin and `ble` on the extent, so zero is legal for one and not the
    /// other — a distinction no prose description of "invalid" would keep.
    func testTheOriginAndExtentBranches() throws {
        try requireNative()
        let seen = try outcomes([
            "negativeX": { try $0.SetViewport(G.Viewport(-1, 0, 10, 10)) },
            "negativeY": { try $0.SetViewport(G.Viewport(0, -1, 10, 10)) },
            "zeroX": { try $0.SetViewport(G.Viewport(0, 0, 10, 10)) },
            "zeroWidth": { try $0.SetViewport(G.Viewport(0, 0, 0, 10)) },
            "zeroHeight": { try $0.SetViewport(G.Viewport(0, 0, 10, 0)) },
            "negativeWidth": { try $0.SetViewport(G.Viewport(0, 0, -1, 10)) },
        ])
        for name in ["negativeX", "negativeY", "zeroWidth", "zeroHeight", "negativeWidth"] {
            assertRejected(seen, name, viewportMessage)
        }
        XCTAssertEqual(seen["zeroX"], "accepted", "X = 0 is `blt`, not `ble`")
    }

    /// The bounds branch, against the backbuffer.
    func testAViewportPastTheBackbufferBoundsIsRejected() throws {
        try requireNative()
        let seen = try outcomes([
            "exactWidth": { try $0.SetViewport(G.Viewport(0, 0, 800, 480)) },
            "oneTooWide": { try $0.SetViewport(G.Viewport(0, 0, 801, 480)) },
            "oneTooTall": { try $0.SetViewport(G.Viewport(0, 0, 800, 481)) },
            "offsetPastRight": { try $0.SetViewport(G.Viewport(1, 0, 800, 480)) },
            "offsetPastBottom": { try $0.SetViewport(G.Viewport(0, 1, 800, 480)) },
        ])
        XCTAssertEqual(seen["exactWidth"], "accepted")
        for name in ["oneTooWide", "oneTooTall", "offsetPastRight", "offsetPastBottom"] {
            assertRejected(seen, name, viewportMessage)
        }
    }

    /// The depth branches, including the one only `bge.un` gets right: a NaN
    /// depth is rejected, where an ordered `MaxDepth < MinDepth` would accept
    /// it.
    func testTheDepthBranches() throws {
        try requireNative()
        func viewport(_ minDepth: Float, _ maxDepth: Float) -> G.Viewport {
            var v = G.Viewport(0, 0, 800, 480)
            v.MinDepth = minDepth
            v.MaxDepth = maxDepth
            return v
        }
        let seen = try outcomes([
            "minBelowZero": { try $0.SetViewport(viewport(-0.1, 1)) },
            "minAboveOne": { try $0.SetViewport(viewport(1.1, 1)) },
            "maxBelowZero": { try $0.SetViewport(viewport(0, -0.1)) },
            "maxAboveOne": { try $0.SetViewport(viewport(0, 1.1)) },
            "inverted": { try $0.SetViewport(viewport(0.8, 0.2)) },
            "minIsNaN": { try $0.SetViewport(viewport(.nan, 1)) },
            "maxIsNaN": { try $0.SetViewport(viewport(0, .nan)) },
            "boundaries": { try $0.SetViewport(viewport(0, 1)) },
        ])
        for name in ["minBelowZero", "minAboveOne", "maxBelowZero", "maxAboveOne",
                     "inverted", "minIsNaN", "maxIsNaN"] {
            assertRejected(seen, name, viewportMessage)
        }
        XCTAssertEqual(seen["boundaries"], "accepted", "0 and 1 are inclusive")
    }

    /// The bounds come from the **bound render target**, not the backbuffer:
    /// the same viewport is legal against one and rejected against the other.
    func testABoundRenderTargetSuppliesTheBounds() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 64, height: 64)
            try device.SetRenderTarget(target)
            do {
                try device.SetViewport(G.Viewport(0, 0, 800, 480))
                game.observations["backbufferSizedOnTarget"] = "accepted"
            } catch is CNAArgumentException {
                game.observations["backbufferSizedOnTarget"] = "rejected"
            }
            do {
                try device.SetViewport(G.Viewport(0, 0, 64, 64))
                game.observations["targetSizedOnTarget"] = "accepted"
            } catch is CNAArgumentException {
                game.observations["targetSizedOnTarget"] = "rejected"
            }
            try device.SetRenderTarget(nil)
            do {
                try device.SetViewport(G.Viewport(0, 0, 800, 480))
                game.observations["backbufferSizedOffTarget"] = "accepted"
            } catch is CNAArgumentException {
                game.observations["backbufferSizedOffTarget"] = "rejected"
            }
            try target.Dispose()
        }
        XCTAssertEqual(game.observations["backbufferSizedOnTarget"], "rejected")
        XCTAssertEqual(game.observations["targetSizedOnTarget"], "accepted")
        XCTAssertEqual(game.observations["backbufferSizedOffTarget"], "accepted")
    }

    // ------------------------------------------------------------------
    // set_ScissorRectangle.
    // ------------------------------------------------------------------

    /// A negative field is rejected; an empty rectangle is not. `blt` against
    /// zero, where the viewport's extent test is `ble`.
    func testTheScissorNegativeAndEmptyCases() throws {
        try requireNative()
        let seen = try outcomes([
            "empty": { try $0.SetScissorRectangle(F.Rectangle(0, 0, 0, 0)) },
            "full": { try $0.SetScissorRectangle(F.Rectangle(0, 0, 800, 480)) },
            "negativeX": { try $0.SetScissorRectangle(F.Rectangle(-1, 0, 10, 10)) },
            "negativeY": { try $0.SetScissorRectangle(F.Rectangle(0, -1, 10, 10)) },
            "negativeWidth": { try $0.SetScissorRectangle(F.Rectangle(0, 0, -1, 10)) },
            "negativeHeight": { try $0.SetScissorRectangle(F.Rectangle(0, 0, 10, -1)) },
        ])
        XCTAssertEqual(seen["empty"], "accepted", "an empty scissor rectangle is legal")
        XCTAssertEqual(seen["full"], "accepted")
        for name in ["negativeX", "negativeY", "negativeWidth", "negativeHeight"] {
            assertRejected(seen, name, scissorMessage)
        }
    }

    /// The edge tests, which are on the converted D3DRECT edges rather than on
    /// the extents.
    func testAScissorRectanglePastTheBoundsIsRejected() throws {
        try requireNative()
        let seen = try outcomes([
            "exact": { try $0.SetScissorRectangle(F.Rectangle(0, 0, 800, 480)) },
            "rightPast": { try $0.SetScissorRectangle(F.Rectangle(1, 0, 800, 480)) },
            "bottomPast": { try $0.SetScissorRectangle(F.Rectangle(0, 1, 800, 480)) },
            "originPastRight": { try $0.SetScissorRectangle(F.Rectangle(801, 0, 0, 0)) },
            "originPastBottom": { try $0.SetScissorRectangle(F.Rectangle(0, 481, 0, 0)) },
            "originOnEdge": { try $0.SetScissorRectangle(F.Rectangle(800, 480, 0, 0)) },
        ])
        XCTAssertEqual(seen["exact"], "accepted")
        XCTAssertEqual(seen["originOnEdge"], "accepted", "the comparison is `bgt`")
        for name in ["rightPast", "bottomPast", "originPastRight", "originPastBottom"] {
            assertRejected(seen, name, scissorMessage)
        }
    }

    /// The pair of tests that only an int32 overflow can reach, and the reason
    /// the arithmetic is `&+` and `&-`.
    ///
    /// `X = 2, Width = Int32.max` wraps the right edge to a large negative
    /// number, which slips past the edge test — and wraps back to `Int32.max`
    /// when the width is recovered, which this catches. A checked `+` would
    /// have trapped instead of rejecting.
    func testTheScissorOverflowBranchIsReachedAndRejects() throws {
        try requireNative()
        let seen = try outcomes([
            "widthOverflows": {
                try $0.SetScissorRectangle(F.Rectangle(2, 0, Int32.max, 0))
            },
            "heightOverflows": {
                try $0.SetScissorRectangle(F.Rectangle(0, 2, 0, Int32.max))
            },
        ])
        assertRejected(seen, "widthOverflows", scissorMessage)
        assertRejected(seen, "heightOverflows", scissorMessage)
    }

    /// The scissor rectangle reads the same bounds source as the viewport.
    func testABoundRenderTargetSuppliesTheScissorBounds() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 64, height: 64)
            try device.SetRenderTarget(target)
            do {
                try device.SetScissorRectangle(F.Rectangle(0, 0, 800, 480))
                game.observations["backbufferSized"] = "accepted"
            } catch is CNAArgumentException {
                game.observations["backbufferSized"] = "rejected"
            }
            do {
                try device.SetScissorRectangle(F.Rectangle(0, 0, 64, 64))
                game.observations["targetSized"] = "accepted"
            } catch is CNAArgumentException {
                game.observations["targetSized"] = "rejected"
            }
            try device.SetRenderTarget(nil)
            try target.Dispose()
        }
        XCTAssertEqual(game.observations["backbufferSized"], "rejected")
        XCTAssertEqual(game.observations["targetSized"], "accepted")
    }
}
