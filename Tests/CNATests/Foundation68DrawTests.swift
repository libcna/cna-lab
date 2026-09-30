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

/// Foundation 68: the draw family.
///
/// **What a passing test here means, and what it does not.** A draw that
/// returns is a draw the device *accepted*. Nothing below asserts that a pixel
/// arrived anywhere, because nothing on this host can read one back — Foundation
/// 53's first bounding fact stands and `GetBackBufferData` still answers
/// `NOT_SUPPORTED`. Geometry, blending, sampling and sort order are
/// unverifiable here and are not claimed.
///
/// What *is* asserted is everything that happens before the device is touched:
/// nine members' worth of managed validation, in the order the IL puts it, plus
/// the two native refusals a caller will actually meet.
final class Foundation68DrawTests: XCTestCase {
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

    private func vertices(_ count: Int) -> [G.VertexPositionColor] {
        [G.VertexPositionColor](
            repeating: G.VertexPositionColor(
                F.Vector3(0, 0, 0),
                F.Color(Int32(255), Int32(255), Int32(255), Int32(255))),
            count: count)
    }

    /// A device with an effect applied and a written vertex buffer bound — the
    /// state in which a draw is legal, established once.
    private func ready(
        _ device: G.GraphicsDevice, vertexCount: Int32 = 64
    ) throws -> (G.Effect, G.VertexBuffer) {
        let effect = try G.Effect.empty(graphicsDevice: device)
        let buffer = try G.VertexBuffer(
            graphicsDevice: device, vertexType: G.VertexPositionColor.self,
            vertexCount: vertexCount, usage: .None)
        try buffer.SetData(vertices(Int(vertexCount)))
        try device.SetVertexBuffer(buffer)
        guard let pass = effect.Techniques?[Int32(0)]?.Passes[Int32(0)] else {
            throw CNAError.producerInvariant("no pass")
        }
        try pass.Apply()
        return (effect, buffer)
    }

    // ------------------------------------------------------------------
    // The draws that reach the device.

    /// A draw from a bound, written buffer with an effect applied is accepted.
    ///
    /// This is the whole of what can be claimed: `DrawPrimitives` returned. It
    /// is still worth asserting, because the same call is a refusal in three
    /// separate states and the difference between them is the milestone.
    func testADrawIsAcceptedOnceTheDeviceIsReady() throws {
        try requireNative()
        let game = try run { game, device in
            let (effect, buffer) = try self.ready(device)
            try device.DrawPrimitives(.TriangleList, startVertex: 0, primitiveCount: 1)
            try device.DrawPrimitives(.TriangleList, startVertex: 0, primitiveCount: 2)
            try device.DrawPrimitives(.LineList, startVertex: 0, primitiveCount: 1)
            try device.DrawPrimitives(.TriangleStrip, startVertex: 0, primitiveCount: 1)
            game.observations["accepted"] = "yes"
            try device.SetVertexBuffer(nil)
            try buffer.Dispose()
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["accepted"], "yes")
    }

    /// **Without an applied effect, every draw is refused** — and the refusal
    /// arrives on the runtime channel with CNA's own diagnosis, not as XNA's
    /// `CannotDrawNoShader`. CNA 0.21 classified it `CNA_RESULT_INTERNAL`
    /// (12); since 0.35 it is `CNA_RESULT_INVALID_STATE` (3), which is what a
    /// missing precondition is.
    ///
    /// XNA's `VerifyCanDraw` reaches the same verdict from a D3D state tracker
    /// this binding cannot see, so the native answer is forwarded rather than a
    /// managed exception invented. This is the refusal Foundation 63 recorded
    /// and withheld the whole family on.
    func testWithoutAnEffectEveryDrawIsRefused() throws {
        try requireNative()
        let game = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 64, usage: .None)
            try buffer.SetData(self.vertices(64))
            try device.SetVertexBuffer(buffer)

            var outcome = "accepted"
            do {
                try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                          primitiveCount: 1)
            } catch let error as CNAError {
                if case .nativeFailure(let operation, let result, let message) = error {
                    outcome = "\(operation)=\(result)"
                    game.observations["message"] = message
                }
            }
            game.observations["no effect"] = outcome
            try device.SetVertexBuffer(nil)
            try buffer.Dispose()
        }
        // 3 is CNA_RESULT_INVALID_STATE.
        XCTAssertEqual(game.observations["no effect"],
                       "cna_graphics_device_draw_primitives=3")
        XCTAssertTrue(
            game.observations["message"]?.contains("no effect has been applied")
                ?? false,
            "the native diagnosis is forwarded verbatim, got "
                + (game.observations["message"] ?? "<none>"))
    }

    /// **A buffer with no data uploaded is drawn, as XNA draws it.**
    ///
    /// XNA draws the undefined contents of an un-`SetData`'d buffer rather
    /// than raising. CNA 0.21 counted vertices *written* rather than capacity
    /// allocated and refused the draw with `CNA_RESULT_INVALID_ARGUMENT`
    /// ("exceeds the bound vertex buffer"); CNA 0.35 counts the capacity, so
    /// the divergence this test used to pin is gone and it now pins XNA's rule.
    func testAnUnwrittenBufferIsDrawnAsXnaDrawsIt() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 64, usage: .None)
            try device.SetVertexBuffer(buffer)
            guard let pass = effect.Techniques?[Int32(0)]?.Passes[Int32(0)] else {
                throw CNAError.producerInvariant("no pass")
            }
            try pass.Apply()

            var outcome = "accepted"
            do {
                try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                          primitiveCount: 1)
            } catch let error as CNAError {
                if case .nativeFailure(_, let result, let message) = error {
                    outcome = "\(result)"
                    game.observations["message"] = message
                }
            }
            game.observations["unwritten"] = outcome

            // XNA refuses SetData on a bound buffer (ResourceInUse), so the
            // upload happens unbound.
            try device.SetVertexBuffer(nil)
            try buffer.SetData(self.vertices(64))
            try device.SetVertexBuffer(buffer)
            try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                      primitiveCount: 1)
            game.observations["written"] = "accepted"

            try device.SetVertexBuffer(nil)
            try buffer.Dispose()
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["unwritten"], "accepted",
                       "got " + (game.observations["message"] ?? "<none>"))
        XCTAssertEqual(game.observations["written"], "accepted")
    }

    /// **A bound buffer refuses `SetData`** — XNA's `CopyData` scans the bound
    /// streams (and `Indices`) unless the lock flags carry `Discard`,
    /// `NoOverwrite` or `READONLY`, and raises `ResourceInUse`. `GetData` is a
    /// read-only lock and passes; the same upload is accepted once unbound.
    func testABoundBufferRefusesSetDataButNotGetData() throws {
        try requireNative()
        let game = try run { game, device in
            let vertices = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 64, usage: .None)
            let indices = try G.IndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 6, usage: .None)
            try vertices.SetData(self.vertices(64))
            try indices.SetData([UInt16](repeating: 0, count: 6))
            try device.SetVertexBuffer(vertices)
            try device.SetIndices(indices)
            let message = "You may not call SetData on a resource while it is "
                + "actively set on the GraphicsDevice. Unset it from the device "
                + "before calling SetData."
            assertProjected(
                CNAInvalidOperationException.self, message: message,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try vertices.SetData(self.vertices(64)) }
            assertProjected(
                CNAInvalidOperationException.self, message: message,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try indices.SetData([UInt16](repeating: 1, count: 6)) }
            var readBack = [UInt16](repeating: 9, count: 6)
            try indices.GetData(&readBack)
            game.observations["read while bound"] = "\(readBack)"
            try device.SetVertexBuffer(nil)
            try device.SetIndices(nil)
            try vertices.SetData(self.vertices(64))
            try indices.SetData([UInt16](repeating: 1, count: 6))
            game.observations["unbound"] = "accepted"
            try indices.Dispose()
            try vertices.Dispose()
        }
        XCTAssertEqual(game.observations["read while bound"], "[0, 0, 0, 0, 0, 0]")
        XCTAssertEqual(game.observations["unbound"], "accepted")
    }

    // ------------------------------------------------------------------
    // The managed validation, which is the milestone's own work.

    /// `primitiveCount <= 0` is `MustDrawSomething`, and it is checked before
    /// anything reaches the device — so it fires with no effect applied and no
    /// buffer bound at all.
    func testDrawingNothingIsRefusedBeforeTheDeviceIsTouched() throws {
        try requireNative()
        _ = try run { game, device in
            let message = "When drawing, at least one primitive must be drawn."
            for count in [Int32(0), -1, -100] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(
                        message, paramName: "primitiveCount"),
                    paramName: "primitiveCount",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) {
                    try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                              primitiveCount: count)
                }
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(
                        message, paramName: "primitiveCount"),
                    paramName: "primitiveCount",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) {
                    try device.DrawIndexedPrimitives(
                        .TriangleList, baseVertex: 0, minVertexIndex: 0,
                        numVertices: 3, startIndex: 0, primitiveCount: count)
                }
            }
            game.observations["checked"] = "yes"
        }
    }

    /// More primitives than the profile allows is refused, naming the profile
    /// and the **limit** — Reach's `MaxPrimitiveCount` is 65,535.
    func testMorePrimitivesThanTheProfileAllowsIsRefused() throws {
        try requireNative()
        let game = try run { game, device in
            game.observations["limit"] =
                "\(device.profileCapabilities.maxPrimitiveCount)"
            let message = "XNA Framework Reach profile supports a maximum of "
                + "65535 primitives per draw call."
            // 65535 is exactly the limit and passes the managed test; the
            // device then refuses it for its own reasons, which is a different
            // channel and a different message.
            var atTheLimit = "accepted"
            do {
                try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                          primitiveCount: 65535)
            } catch is CNAError {
                atTheLimit = "native refusal"
            }
            game.observations["at the limit"] = atTheLimit

            assertProjected(
                CNANotSupportedException.self, message: message,
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                          primitiveCount: 65536)
            }
            assertProjected(
                CNANotSupportedException.self, message: message,
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                try device.DrawInstancedPrimitives(
                    .TriangleList, baseVertex: 0, minVertexIndex: 0,
                    numVertices: 3, startIndex: 0, primitiveCount: 1,
                    instanceCount: 65536)
            }
        }
        XCTAssertEqual(game.observations["limit"], "65535")
        // The managed test passed it through; the device refused it.
        XCTAssertEqual(game.observations["at the limit"], "native refusal")
    }

    /// The indexed draws test `numVertices` **before** `primitiveCount`, so a
    /// call that is wrong in both ways reports the vertex count.
    func testTheVertexCountIsCheckedBeforeThePrimitiveCount() throws {
        try requireNative()
        _ = try run { game, device in
            let message = "When drawing indexed primitives, the number of "
                + "vertices passed in must be greater than zero."
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(message, paramName: "numVertices"),
                paramName: "numVertices",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                try device.DrawIndexedPrimitives(
                    .TriangleList, baseVertex: 0, minVertexIndex: 0,
                    numVertices: 0, startIndex: 0, primitiveCount: -1)
            }
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(message, paramName: "numVertices"),
                paramName: "numVertices",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                try device.DrawInstancedPrimitives(
                    .TriangleList, baseVertex: 0, minVertexIndex: 0,
                    numVertices: -1, startIndex: 0, primitiveCount: -1,
                    instanceCount: -1)
            }
            game.observations["checked"] = "yes"
        }
    }

    /// A bound buffer with a non-zero instance frequency makes the three
    /// non-instanced draws refuse — and leaves `DrawInstancedPrimitives`
    /// alone, which is the one that needs it.
    func testANonZeroInstanceFrequencyRefusesTheNonInstancedDraws() throws {
        try requireNative()
        let game = try run { game, device in
            let (effect, buffer) = try self.ready(device)
            game.observations["mask when plain"] = "\(device.boundInstanceStreamMask)"

            let binding = try G.VertexBufferBinding(buffer, 0, 1)
            try device.SetVertexBuffers([binding])
            game.observations["mask when instanced"] =
                "\(device.boundInstanceStreamMask != 0)"

            let message = "Non-instanced draw calls are not valid when a "
                + "vertex buffer is bound with a non-zero instance frequency."
            assertProjected(
                CNAInvalidOperationException.self, message: message,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) {
                try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                          primitiveCount: 1)
            }
            assertProjected(
                CNAInvalidOperationException.self, message: message,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) {
                try device.DrawIndexedPrimitives(
                    .TriangleList, baseVertex: 0, minVertexIndex: 0,
                    numVertices: 3, startIndex: 0, primitiveCount: 1)
            }

            // The instanced draw does not make THAT complaint -- it makes its
            // own, because every bound stream is instanced and it needs a
            // mixture.
            let mixture = "DrawInstancedPrimitives requires at least one vertex "
                + "buffer to be bound with a non-zero instance frequency, and "
                + "also at least one with a zero instance frequency."
            assertProjected(
                CNAInvalidOperationException.self, message: mixture,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) {
                try device.DrawInstancedPrimitives(
                    .TriangleList, baseVertex: 0, minVertexIndex: 0,
                    numVertices: 3, startIndex: 0, primitiveCount: 1,
                    instanceCount: 1)
            }

            // And ALL-zero is refused by the same test, for the same reason.
            try device.SetVertexBuffer(buffer)
            assertProjected(
                CNAInvalidOperationException.self, message: mixture,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) {
                try device.DrawInstancedPrimitives(
                    .TriangleList, baseVertex: 0, minVertexIndex: 0,
                    numVertices: 3, startIndex: 0, primitiveCount: 1,
                    instanceCount: 1)
            }

            // A genuine mixture gets past every managed test and reaches the
            // device, which answers for its own reasons.
            let second = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 8, usage: .None)
            try second.SetData(self.vertices(8))
            try device.SetVertexBuffers([
                try G.VertexBufferBinding(buffer, 0, 0),
                try G.VertexBufferBinding(second, 0, 1),
            ])
            var mixed = "accepted"
            do {
                try device.DrawInstancedPrimitives(
                    .TriangleList, baseVertex: 0, minVertexIndex: 0,
                    numVertices: 3, startIndex: 0, primitiveCount: 1,
                    instanceCount: 1)
            } catch is CNAError {
                mixed = "reached the device"
            }
            game.observations["mixed"] = mixed

            try device.SetVertexBuffers(nil)
            try second.Dispose()
            try buffer.Dispose()
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["mask when plain"], "0")
        XCTAssertEqual(game.observations["mask when instanced"], "true")
        // Either outcome is a pass -- what matters is that it got past the
        // MANAGED tests, which the two refusals above prove it would not have.
        XCTAssertNotEqual(game.observations["mixed"], nil)
    }

    // ------------------------------------------------------------------
    // The user-primitive draws, whose validation is entirely this binding's.

    /// A user-primitive draw is accepted once an effect is applied, and it
    /// needs no bound buffer at all — the array is the stream.
    func testAUserPrimitiveDrawIsAccepted() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            guard let pass = effect.Techniques?[Int32(0)]?.Passes[Int32(0)] else {
                throw CNAError.producerInvariant("no pass")
            }
            try pass.Apply()

            let data = self.vertices(6)
            try device.DrawUserPrimitives(
                .TriangleList, vertexData: data, vertexOffset: 0,
                primitiveCount: 2,
                vertexDeclaration: G.VertexPositionColor.VertexDeclaration)
            game.observations["with declaration"] = "accepted"

            // The overload that reads the declaration from T itself.
            try device.DrawUserPrimitives(
                .TriangleList, vertexData: data, vertexOffset: 0,
                primitiveCount: 2)
            game.observations["from the type"] = "accepted"

            let indices: [Int16] = [0, 1, 2, 3, 4, 5]
            try device.DrawUserIndexedPrimitives(
                .TriangleList, vertexData: data, vertexOffset: 0,
                numVertices: 6, indexData: indices, indexOffset: 0,
                primitiveCount: 2)
            game.observations["indexed"] = "accepted"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["with declaration"], "accepted")
        XCTAssertEqual(game.observations["from the type"], "accepted")
        XCTAssertEqual(game.observations["indexed"], "accepted")
    }

    /// The user-primitive draws' seven managed tests, in the IL's order.
    ///
    /// These are the only draw refusals that are entirely this binding's — the
    /// bound-buffer draws forward the device's verdict for anything past the
    /// primitive count, and these do not reach the device at all.
    func testTheUserPrimitiveValidationInOrder() throws {
        try requireNative()
        _ = try run { game, device in
            let declaration = G.VertexPositionColor.VertexDeclaration
            let data = self.vertices(6)

            // An empty array is a null one.
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    "This method does not accept null for this parameter.",
                    paramName: "vertexData"),
                paramName: "vertexData",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) {
                try device.DrawUserPrimitives(
                    .TriangleList, vertexData: [G.VertexPositionColor](),
                    vertexOffset: 0, primitiveCount: 1,
                    vertexDeclaration: declaration)
            }

            // Then the primitive count.
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    "When drawing, at least one primitive must be drawn.",
                    paramName: "primitiveCount"),
                paramName: "primitiveCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                try device.DrawUserPrimitives(
                    .TriangleList, vertexData: data, vertexOffset: 0,
                    primitiveCount: 0, vertexDeclaration: declaration)
            }

            // Then the offset, against the array's own length.
            let offsetMessage = "The offset must be within the valid range for "
                + "this resource."
            for offset in [Int32(-1), 6, 7] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(
                        offsetMessage, paramName: "vertexOffset"),
                    paramName: "vertexOffset",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) {
                    try device.DrawUserPrimitives(
                        .TriangleList, vertexData: data, vertexOffset: offset,
                        primitiveCount: 1, vertexDeclaration: declaration)
                }
            }

            // Then the count of vertices the primitives actually need, which is
            // where `primitiveCount` is blamed a second time with a different
            // message.
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    "This parameter must be a valid index within the array.",
                    paramName: "primitiveCount"),
                paramName: "primitiveCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                // Three triangles need nine vertices; there are six.
                try device.DrawUserPrimitives(
                    .TriangleList, vertexData: data, vertexOffset: 0,
                    primitiveCount: 3, vertexDeclaration: declaration)
            }
            // And the same test catches an offset that leaves too few behind.
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    "This parameter must be a valid index within the array.",
                    paramName: "primitiveCount"),
                paramName: "primitiveCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                try device.DrawUserPrimitives(
                    .TriangleList, vertexData: data, vertexOffset: 4,
                    primitiveCount: 1, vertexDeclaration: declaration)
            }
            game.observations["checked"] = "yes"
        }
    }

    /// **32-bit user indices are refused on a Reach device**, and that is XNA's
    /// rule rather than a host limit — the same `IndexElementSize32` the
    /// `IndexBuffer` constructor reads, from the same extracted table.
    func testThirtyTwoBitUserIndicesAreRefusedOnReach() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            guard let pass = effect.Techniques?[Int32(0)]?.Passes[Int32(0)] else {
                throw CNAError.producerInvariant("no pass")
            }
            try pass.Apply()
            let data = self.vertices(6)

            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile does not support 32 bit "
                    + "indices. Use IndexElementSize.SixteenBits or a type that "
                    + "has a size of two bytes.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                try device.DrawUserIndexedPrimitives(
                    .TriangleList, vertexData: data, vertexOffset: 0,
                    numVertices: 6, indexData: [Int32](repeating: 0, count: 6),
                    indexOffset: 0, primitiveCount: 2)
            }

            // Sixteen-bit indices of the same shape are accepted.
            try device.DrawUserIndexedPrimitives(
                .TriangleList, vertexData: data, vertexOffset: 0,
                numVertices: 6, indexData: [Int16](repeating: 0, count: 6),
                indexOffset: 0, primitiveCount: 2)
            game.observations["sixteen bit"] = "accepted"
            game.observations["profile"] =
                "\(device.profileCapabilities.indexElementSize32)"
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["sixteen bit"], "accepted")
        XCTAssertEqual(game.observations["profile"], "false")
    }

    /// `GetElementCountFromPrimitiveType` — how many vertices each topology
    /// needs — comes from CNA's own helper rather than from arithmetic written
    /// here, and it is what the user-primitive bound test compares against.
    func testTheVertexCountPerTopology() throws {
        try requireNative()
        let game = try run { game, device in
            var counts: [String] = []
            for (type, name) in [(G.PrimitiveType.TriangleList, "TriangleList"),
                                 (.TriangleStrip, "TriangleStrip"),
                                 (.LineList, "LineList"),
                                 (.LineStrip, "LineStrip")] {
                let one = try device.elementCount(for: type, primitiveCount: 1)
                let three = try device.elementCount(for: type, primitiveCount: 3)
                counts.append("\(name)=\(one)/\(three)")
            }
            game.observations["counts"] = counts.joined(separator: " ")
        }
        // A triangle list needs 3n; a strip n+2; a line list 2n; a line strip
        // n+1. These are XNA's own formulas and CNA answers the same numbers.
        XCTAssertEqual(
            game.observations["counts"],
            "TriangleList=3/9 TriangleStrip=3/5 LineList=2/6 LineStrip=2/4")
    }

    /// The array bound test itself, over its three numbers.
    ///
    /// XNA's comparison is `ble.un`, **unsigned**, and it cannot be reached
    /// through a draw on this profile: `primitiveCount` is capped at Reach's
    /// 65,535 several tests earlier and the offset is capped below the array's
    /// length, so the sum cannot overflow. A mutation making it signed survived
    /// until this test existed.
    func testTheArrayBoundIsUnsigned() {
        typealias GD = Microsoft.Xna.Framework.Graphics.GraphicsDevice
        XCTAssertTrue(GD.arrayHolds(offset: 0, elements: 6, count: 6))
        XCTAssertTrue(GD.arrayHolds(offset: 2, elements: 4, count: 6))
        XCTAssertFalse(GD.arrayHolds(offset: 1, elements: 6, count: 6))
        XCTAssertFalse(GD.arrayHolds(offset: 0, elements: 9, count: 6))
        // The overflow the unsigned comparison exists for: signed arithmetic
        // wraps to a negative and would pass.
        XCTAssertFalse(
            GD.arrayHolds(offset: Int32.max, elements: Int32.max, count: 6),
            "a wrapped sum must not read as a small one")
        XCTAssertFalse(GD.arrayHolds(offset: 1, elements: Int32.max, count: 6))
        // And a negative offset is NOT caught here -- read unsigned, -1 plus 1
        // wraps to zero, which passes. That is why XNA guards `vertexOffset < 0`
        // separately with `OffsetNotValid` several instructions earlier, and it
        // is why this test asserts `true`: an assertion that the unsigned
        // reading also catches negatives would be asserting something neither
        // XNA nor this projection does.
        XCTAssertTrue(GD.arrayHolds(offset: -1, elements: 1, count: 6))
    }

    /// **The primitive type reaches the device**, and CNA's own range rule is
    /// what makes that observable.
    ///
    /// No draw's *result* can be read back here, so a topology sent as the
    /// wrong one would normally go unnoticed — Foundation 63 withdrew a
    /// mutation for exactly that reason. But CNA refuses a draw whose vertex
    /// range exceeds what the buffer holds, and the range depends on the
    /// topology: four written vertices are enough for a `LineStrip` of three
    /// primitives and not for a `TriangleList` of three. So a forced topology
    /// turns an accepted draw into a refused one.
    ///
    /// Only on a renderer that keeps that managed range guard. XNA forwards
    /// draw ranges to D3D unvalidated, and CNA's EasyGL renderers do the same
    /// (SOFTWARE-322); the guard stays where CPU staging would otherwise read
    /// invalid memory, as on HEADLESS. So the renderer is asked first, with a
    /// draw that is out of range whatever its topology.
    func testThePrimitiveTypeReachesTheDevice() throws {
        try requireNative()
        let game = try run { game, device in
            let effect = try G.Effect.empty(graphicsDevice: device)
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            try buffer.SetData(self.vertices(4))
            try device.SetVertexBuffer(buffer)
            guard let pass = effect.Techniques?[Int32(0)]?.Passes[Int32(0)] else {
                throw CNAError.producerInvariant("no pass")
            }
            try pass.Apply()

            // Out of range for any topology: one primitive from vertex four.
            var guarded = "no"
            do {
                try device.DrawPrimitives(.LineStrip, startVertex: 4, primitiveCount: 1)
            } catch is CNAError { guarded = "yes" }
            game.observations["range guarded"] = guarded

            // A line strip of three needs four vertices, which is what is
            // written.
            game.observations["line strip needs"] =
                "\(try device.elementCount(for: .LineStrip, primitiveCount: 3))"
            try device.DrawPrimitives(.LineStrip, startVertex: 0, primitiveCount: 3)
            game.observations["line strip"] = "accepted"

            // A triangle list of three needs nine, which is not.
            game.observations["triangle list needs"] =
                "\(try device.elementCount(for: .TriangleList, primitiveCount: 3))"
            var triangles = "accepted"
            do {
                try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                          primitiveCount: 3)
            } catch is CNAError { triangles = "refused" }
            game.observations["triangle list"] = triangles

            try device.SetVertexBuffer(nil)
            try buffer.Dispose()
            try effect.Dispose()
        }
        XCTAssertEqual(game.observations["line strip needs"], "4")
        XCTAssertEqual(game.observations["triangle list needs"], "9")
        XCTAssertEqual(game.observations["line strip"], "accepted")
        XCTAssertEqual(game.observations["triangle list"],
                       game.observations["range guarded"] == "yes" ? "refused" : "accepted",
                       "range guarded: \(game.observations["range guarded"] ?? "?")")
    }

    /// A disposed device refuses every draw before any argument is looked at.
    func testADisposedDeviceRefusesEveryDraw() throws {
        try requireNative()

        final class DisposeProbe: F.Game {
            var manager: F.GraphicsDeviceManager?
            var captured: G.GraphicsDevice?
            override func LoadContent() throws {
                captured = try GraphicsDevice
                try Exit()
            }
            override func Update(_ gameTime: F.GameTime) throws { try Exit() }
        }

        let game = try DisposeProbe()
        game.manager = try F.GraphicsDeviceManager(game: game)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        guard let device = game.captured else {
            throw CNAError.producerInvariant("no device captured")
        }
        // Outside the callback the facade's handle no longer validates, so the
        // disposal check -- which is the first instruction of every draw --
        // fires before `primitiveCount` is even read.
        var outcome = "accepted"
        do {
            try device.DrawPrimitives(.TriangleList, startVertex: 0,
                                      primitiveCount: -1)
        } catch { outcome = "refused" }
        XCTAssertEqual(outcome, "refused")
    }
}
