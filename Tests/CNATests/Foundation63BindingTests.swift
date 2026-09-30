// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class BindingProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((BindingProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (BindingProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 63: `SetVertexBuffer`, `SetVertexBuffers`, `GetVertexBuffers`
/// and `Indices`.
///
/// The binding half of the draw family. The draws themselves are **not** here:
/// CNA refuses every one of them with *"no effect has been applied"*
/// (`build-probe/f63_draw.c`), which is the same refusal XNA's `VerifyCanDraw`
/// makes with `CannotDrawNoShader`, so they wait for `Effect` rather than
/// shipping as members that always fail.
final class Foundation63BindingTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (BindingProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> BindingProbeGame {
        let game = try BindingProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    private func buffer(_ device: G.GraphicsDevice, count: Int32 = 4) throws
        -> G.VertexBuffer {
        try G.VertexBuffer(graphicsDevice: device,
                           vertexType: G.VertexPositionColor.self,
                           vertexCount: count, usage: .None)
    }

    /// What is bound is what comes back — the same object, not a copy.
    ///
    /// XNA hands back the managed bindings it stored. CNA can say which native
    /// handle is in a slot but publishes no route from a native object back to
    /// a handle, and its header prescribes caching what you bind; XNA's
    /// `DeviceResourceManager` does the same. So this asserts object identity,
    /// which is the thing a handle-keyed lookup could not give.
    func testWhatIsBoundComesBackByIdentity() throws {
        try requireNative()
        let game = try run { game, device in
            let first = try self.buffer(device)
            try device.SetVertexBuffer(first)
            let bindings = device.GetVertexBuffers()
            game.observations["count"] = "\(bindings.count)"
            game.observations["identity"] =
                "\(bindings.first?.VertexBuffer === first)"
            game.observations["offset"] = "\(bindings.first?.VertexOffset ?? -1)"
            game.observations["frequency"] =
                "\(bindings.first?.InstanceFrequency ?? -1)"
            try device.SetVertexBuffer(nil)
            game.observations["after unbind"] = "\(device.GetVertexBuffers().count)"
            try first.Dispose()
        }
        XCTAssertEqual(game.observations["count"], "1")
        XCTAssertEqual(game.observations["identity"], "true")
        XCTAssertEqual(game.observations["offset"], "0")
        XCTAssertEqual(game.observations["frequency"], "0")
        XCTAssertEqual(game.observations["after unbind"], "0")
    }

    /// The offset overload carries the offset, and the binding validates it
    /// against the buffer's own vertex count before the device sees it.
    func testTheOffsetOverloadValidatesThroughTheBinding() throws {
        try requireNative()
        let game = try run { game, device in
            let vertices = try self.buffer(device)
            try device.SetVertexBuffer(vertices, vertexOffset: 2)
            game.observations["offset"] =
                "\(device.GetVertexBuffers().first?.VertexOffset ?? -1)"

            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    CNAArgumentOutOfRangeException.argArgumentOutOfRangeMessage,
                    paramName: "vertexOffset"),
                paramName: "vertexOffset",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { try device.SetVertexBuffer(vertices, vertexOffset: 4) }

            // The refused call left the previous binding alone.
            game.observations["still bound"] =
                "\(device.GetVertexBuffers().first?.VertexOffset ?? -1)"
            try device.SetVertexBuffer(nil)
            try vertices.Dispose()
        }
        XCTAssertEqual(game.observations["offset"], "2")
        XCTAssertEqual(game.observations["still bound"], "2")
    }

    /// More streams than the profile allows is refused, naming the profile and
    /// the limit.
    ///
    /// Reach's `MaxVertexStreams` is 16, extracted from the assembly, so
    /// seventeen bindings is the first count XNA refuses. The check is managed
    /// and never reaches CNA.
    func testMoreStreamsThanTheProfileAllowsIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            let vertices = try self.buffer(device)
            let binding = try G.VertexBufferBinding(vertices)
            let seventeen = [G.VertexBufferBinding](repeating: binding, count: 17)
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile supports a maximum of 16 "
                    + "simultaneous vertex buffers.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) { try device.SetVertexBuffers(seventeen) }
            game.observations["limit"] =
                "\(device.profileCapabilities.maxVertexStreams)"
            try device.SetVertexBuffer(nil)
            try vertices.Dispose()
        }
    }

    /// A null array unbinds everything, exactly as a null single buffer does.
    func testANullArrayUnbindsEverything() throws {
        try requireNative()
        let game = try run { game, device in
            let vertices = try self.buffer(device)
            try device.SetVertexBuffer(vertices)
            game.observations["bound"] = "\(device.GetVertexBuffers().count)"
            try device.SetVertexBuffers(nil)
            game.observations["null"] = "\(device.GetVertexBuffers().count)"
            try device.SetVertexBuffer(vertices)
            try device.SetVertexBuffers([])
            game.observations["empty"] = "\(device.GetVertexBuffers().count)"
            try vertices.Dispose()
        }
        XCTAssertEqual(game.observations["bound"], "1")
        XCTAssertEqual(game.observations["null"], "0")
        XCTAssertEqual(game.observations["empty"], "0")
    }

    /// `GetVertexBuffers` returns a copy, so mutating the result binds nothing.
    func testTheReturnedArrayIsACopy() throws {
        try requireNative()
        let game = try run { game, device in
            let vertices = try self.buffer(device)
            try device.SetVertexBuffer(vertices)
            var copy = device.GetVertexBuffers()
            copy.removeAll()
            game.observations["after mutating the copy"] =
                "\(device.GetVertexBuffers().count)"
            try device.SetVertexBuffer(nil)
            try vertices.Dispose()
        }
        XCTAssertEqual(game.observations["after mutating the copy"], "1")
    }

    /// `Indices` reads back the object that was set, and setting the same one
    /// twice reaches no native route.
    func testIndicesRoundTripsByIdentity() throws {
        try requireNative()
        let game = try run { game, device in
            game.observations["initially"] = "\(device.Indices == nil)"
            let indices = try G.IndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 6, usage: .None)
            try device.SetIndices(indices)
            game.observations["identity"] = "\(device.Indices === indices)"
            // XNA's setter short-circuits on identity before touching the
            // device; doing it twice must be harmless either way.
            try device.SetIndices(indices)
            game.observations["twice"] = "\(device.Indices === indices)"
            try device.SetIndices(nil)
            game.observations["unbound"] = "\(device.Indices == nil)"
            try device.SetIndices(nil)
            game.observations["unbound twice"] = "\(device.Indices == nil)"
            try indices.Dispose()
        }
        XCTAssertEqual(game.observations["initially"], "true")
        XCTAssertEqual(game.observations["identity"], "true")
        XCTAssertEqual(game.observations["twice"], "true")
        XCTAssertEqual(game.observations["unbound"], "true")
        XCTAssertEqual(game.observations["unbound twice"], "true")
    }

    /// A disposed buffer is refused by both binders, on the CLR channel.
    func testADisposedBufferCannotBeBound() throws {
        try requireNative()
        _ = try run { game, device in
            let vertices = try self.buffer(device)
            try vertices.Dispose()
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object.\r\nObject name: 'VertexBuffer'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try device.SetVertexBuffer(vertices) }

            let indices = try G.IndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 6, usage: .None)
            try indices.Dispose()
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object.\r\nObject name: 'IndexBuffer'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try device.SetIndices(indices) }
            game.observations["checked"] = "yes"
        }
    }

    /// A binding survives the callback boundary, which is the reason the device
    /// identity is the runtime and not the facade.
    ///
    /// A facade is a per-callback capability token, so the `GraphicsDevice`
    /// object a buffer was created from is a *different object* from the one a
    /// later callback holds. Comparing facades would refuse every legitimate
    /// binding made in a later frame; comparing runtimes accepts it.
    func testABufferBindsInALaterCallbackThanItWasCreatedIn() throws {
        try requireNative()

        final class TwoCallbackGame: F.Game {
            var manager: F.GraphicsDeviceManager?
            var failure: Error?
            var boundInDraw = "not reached"
            private var buffer: G.VertexBuffer?

            override func LoadContent() throws {
                do {
                    guard let device = try GraphicsDevice else { return }
                    buffer = try G.VertexBuffer(
                        graphicsDevice: device,
                        vertexType: G.VertexPositionColor.self,
                        vertexCount: 3, usage: .None)
                } catch { failure = error }
            }

            override func Update(_ gameTime: F.GameTime) throws {
                do {
                    guard let device = try GraphicsDevice, let buffer else { return }
                    try device.SetVertexBuffer(buffer)
                    boundInDraw = "\(device.GetVertexBuffers().first?.VertexBuffer === buffer)"
                    try device.SetVertexBuffer(nil)
                } catch { failure = error }
                try Exit()
            }
        }

        let game = try TwoCallbackGame()
        game.manager = try F.GraphicsDeviceManager(game: game)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.boundInDraw, "true")
    }
}
