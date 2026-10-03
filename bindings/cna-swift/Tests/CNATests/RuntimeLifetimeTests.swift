// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

/// Does the runtime actually die when the game does?
///
/// A `GraphicsResource` holds its `GraphicsDevice` facade strongly and the
/// facade holds the `RuntimeState` strongly, so **any** cache on the runtime
/// that holds a resource closes a cycle: `RuntimeState -> resource -> facade ->
/// RuntimeState`. Foundation 63 added the first one — the bound vertex-buffer
/// bindings — and nothing caught it: binding a single buffer kept the runtime,
/// every native handle it tracks and every registered child alive past
/// `Game.Dispose` for the life of the process.
///
/// A retain cycle is invisible to AddressSanitizer (nothing is unreachable, so
/// nothing leaks in its sense) and to every gate this project has. These tests
/// are what asks. They are deliberately written as *controls plus the case*:
/// the two controls prove the harness can observe a released runtime at all, so
/// a green case is not green because the measurement is broken.
final class RuntimeLifetimeTests: XCTestCase {
    /// The case: a bound resource must not outlive `Game.Dispose`.
    func testRuntimeStateIsReleasedAfterBindingABuffer() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("needs the native library")
        }
        weak var weakRuntime: RuntimeState?

        final class Probe: F.Game {
            var manager: F.GraphicsDeviceManager?
            var failure: Error?
            var captured: RuntimeState?
            override func LoadContent() throws {
                do {
                    guard let device = try GraphicsDevice else { return }
                    captured = device.runtimeState
                    let vb = try G.VertexBuffer(
                        graphicsDevice: device,
                        vertexType: G.VertexPositionColor.self,
                        vertexCount: 3, usage: .None)
                    try device.SetVertexBuffer(vb)
                } catch { failure = error }
                try Exit()
            }
            override func Update(_ gameTime: F.GameTime) throws { try Exit() }
        }

        do {
            let game = try Probe()
            game.manager = try F.GraphicsDeviceManager(game: game)
            try game.Run()
            weakRuntime = game.captured
            game.captured = nil
            try game.Dispose()
            if let failure = game.failure { throw failure }
        }
        XCTAssertNil(
            weakRuntime,
            "RuntimeState outlived the game: a bound resource closes a retain "
            + "cycle. RuntimeState.releaseBoundResources() is what breaks it.")
    }

    /// The control: the same game with NOTHING bound. If this also leaks, the
    /// cycle is not the binding cache's.
    func testRuntimeStateIsReleasedWithNothingBound() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("needs the native library")
        }
        weak var weakRuntime: RuntimeState?

        final class Bare: F.Game {
            var manager: F.GraphicsDeviceManager?
            var captured: RuntimeState?
            override func LoadContent() throws {
                captured = (try GraphicsDevice)?.runtimeState
                try Exit()
            }
            override func Update(_ gameTime: F.GameTime) throws { try Exit() }
        }
        do {
            let game = try Bare()
            game.manager = try F.GraphicsDeviceManager(game: game)
            try game.Run()
            weakRuntime = game.captured
            game.captured = nil
            try game.Dispose()
        }
        XCTAssertNil(weakRuntime, "RuntimeState outlived a game that bound nothing")
    }

    /// And with a buffer CREATED but never bound, which separates "the resource
    /// registry retains it" from "the binding cache does".
    func testRuntimeStateIsReleasedWithAnUnboundBuffer() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("needs the native library")
        }
        weak var weakRuntime: RuntimeState?

        final class Created: F.Game {
            var manager: F.GraphicsDeviceManager?
            var failure: Error?
            var captured: RuntimeState?
            override func LoadContent() throws {
                do {
                    guard let device = try GraphicsDevice else { return }
                    captured = device.runtimeState
                    _ = try G.VertexBuffer(
                        graphicsDevice: device,
                        vertexType: G.VertexPositionColor.self,
                        vertexCount: 3, usage: .None)
                } catch { failure = error }
                try Exit()
            }
            override func Update(_ gameTime: F.GameTime) throws { try Exit() }
        }
        do {
            let game = try Created()
            game.manager = try F.GraphicsDeviceManager(game: game)
            try game.Run()
            weakRuntime = game.captured
            game.captured = nil
            try game.Dispose()
            if let failure = game.failure { throw failure }
        }
        XCTAssertNil(weakRuntime, "RuntimeState outlived a game with an unbound buffer")
    }
}
