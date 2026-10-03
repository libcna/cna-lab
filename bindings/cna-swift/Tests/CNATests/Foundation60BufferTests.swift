// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class BufferProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((BufferProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (BufferProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 60: `VertexBuffer`, `IndexBuffer` and `VertexBufferBinding`.
///
/// Buffer *contents* are observable — `cna_vertex_buffer_get_data_raw` and
/// `cna_index_buffer_get_data` both round-trip, measured in
/// `build-probe/f60_buffers.c` — so these assert values rather than "the call
/// returned zero". What is not observable is anything drawn with them; that
/// stays out of this suite entirely.
final class Foundation60BufferTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (BufferProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> BufferProbeGame {
        let game = try BufferProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    private func vertices(_ values: [(Float, Float, Float, UInt8)]) -> [G.VertexPositionColor] {
        values.map {
            G.VertexPositionColor(
                F.Vector3($0.0, $0.1, $0.2),
                F.Color(Int32($0.3), Int32($0.3), Int32($0.3), 255))
        }
    }

    private func describe(_ data: [G.VertexPositionColor]) -> String {
        data.map { "\($0.Position.X),\($0.Position.Y),\($0.Position.Z),\($0.Color.R)" }
            .joined(separator: " ")
    }

    // MARK: - Construction

    /// The declaration a buffer is given is the object it hands back.
    ///
    /// XNA stores the reference and its getter reads the field, so a caller can
    /// compare the two with `===`. The `Type` overload's declaration is the
    /// vertex struct's own static one, which is the object XNA's `FromType`
    /// cache would hold, so two buffers of one vertex type share it.
    func testTheDeclarationIsStoredByReference() throws {
        try requireNative()
        let game = try run { game, device in
            let declaration = try G.VertexDeclaration(elements: [
                G.VertexElement(0, .Vector3, .Position, 0),
                G.VertexElement(12, .Color, .Color, 0),
            ])
            let explicit = try G.VertexBuffer(
                graphicsDevice: device, vertexDeclaration: declaration,
                vertexCount: 4, usage: .None)
            game.observations["same object"] =
                "\(explicit.VertexDeclaration === declaration)"
            game.observations["stride"] = "\(declaration.VertexStride)"

            let byType = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            let second = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 8, usage: .None)
            game.observations["from type is the struct's own"] =
                "\(byType.VertexDeclaration === G.VertexPositionColor.VertexDeclaration)"
            game.observations["two buffers share one"] =
                "\(byType.VertexDeclaration === second.VertexDeclaration)"
            game.observations["counts"] =
                "\(explicit.VertexCount) \(byType.VertexCount) \(second.VertexCount)"
            game.observations["usage"] =
                "\(explicit.BufferUsage == G.BufferUsage.None)"
            try explicit.Dispose()
            try byType.Dispose()
            try second.Dispose()
        }
        XCTAssertEqual(game.observations["same object"], "true")
        XCTAssertEqual(game.observations["stride"], "16")
        XCTAssertEqual(game.observations["from type is the struct's own"], "true")
        XCTAssertEqual(game.observations["two buffers share one"], "true")
        XCTAssertEqual(game.observations["counts"], "4 4 8")
        XCTAssertEqual(game.observations["usage"], "true")
    }

    /// A non-positive vertex or index count is refused, naming its own parameter.
    func testANonPositiveCountIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            let message = G.Texture2D.resourcesMustBeGreaterThanZeroSizeMessage
            for count in [Int32(0), -1] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(message, paramName: "vertexCount"),
                    paramName: "vertexCount",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) {
                    _ = try G.VertexBuffer(
                        graphicsDevice: device,
                        vertexType: G.VertexPositionColor.self,
                        vertexCount: count, usage: .None)
                }
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(message, paramName: "indexCount"),
                    paramName: "indexCount",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) {
                    _ = try G.IndexBuffer(
                        graphicsDevice: device, indexElementSize: .SixteenBits,
                        indexCount: count, usage: .None)
                }
            }
            game.observations["checked"] = "yes"
        }
    }

    /// `FromType` refuses a reference type, and a value type that is not one of
    /// the four, with XNA's own formatted messages.
    func testFromTypeRefusesWhatXnaRefuses() throws {
        try requireNative()
        _ = try run { game, device in
            // The name in the message is the Swift type's, because that is what
            // the caller passed. XNA formats System.Type.ToString(); a CLR name
            // for a Swift type would be an invention.
            assertProjected(
                CNAArgumentException.self,
                message: "Invalid vertex type. "
                    + "Microsoft.Xna.Framework.Graphics.SpriteBatch is not a value type.",
                hResult: CNAArgumentException.corArgumentHResult
            ) {
                _ = try G.VertexBuffer(
                    graphicsDevice: device, vertexType: G.SpriteBatch.self,
                    vertexCount: 4, usage: .None)
            }
            assertProjected(
                CNAArgumentException.self,
                message: "Invalid vertex type. Swift.Int32 does not implement "
                    + "the IVertexType interface.",
                hResult: CNAArgumentException.corArgumentHResult
            ) {
                _ = try G.VertexBuffer(
                    graphicsDevice: device, vertexType: Int32.self,
                    vertexCount: 4, usage: .None)
            }
            game.observations["checked"] = "yes"
        }
    }

    /// Each of the four vertex structs measures the stride its declaration
    /// claims.
    ///
    /// This is `FromType`'s last test — `Marshal.SizeOf(vertexType) !=
    /// declaration._vertexStride` — with the size measured by the Swift
    /// compiler. A struct whose layout drifted from its declared stride would
    /// fail here rather than corrupt a transfer.
    func testEveryVertexTypesSizeMatchesItsDeclaredStride() throws {
        XCTAssertEqual(MemoryLayout<G.VertexPositionColor>.size,
                       Int(G.VertexPositionColor.VertexDeclaration.VertexStride))
        XCTAssertEqual(MemoryLayout<G.VertexPositionColorTexture>.size,
                       Int(G.VertexPositionColorTexture.VertexDeclaration.VertexStride))
        XCTAssertEqual(MemoryLayout<G.VertexPositionNormalTexture>.size,
                       Int(G.VertexPositionNormalTexture.VertexDeclaration.VertexStride))
        XCTAssertEqual(MemoryLayout<G.VertexPositionTexture>.size,
                       Int(G.VertexPositionTexture.VertexDeclaration.VertexStride))
    }

    /// Exercises the size stored in `FromType`'s private registration record,
    /// not merely the independently visible struct and declaration sizes.
    /// A bad registered size must be refused before any native resource is
    /// created, which also keeps this invariant's mutation test deterministic.
    func testFromTypeAcceptsItsMatchingRegisteredSize() throws {
        let declaration = try G.VertexDeclaration.fromVertexType(
            G.VertexPositionColor.self)
        XCTAssertTrue(declaration === G.VertexPositionColor.VertexDeclaration)
    }

    // MARK: - Vertex transfers

    /// What goes in comes back out, vertex for vertex.
    func testVertexDataRoundTrips() throws {
        try requireNative()
        let written = vertices([(1, 2, 3, 10), (4, 5, 6, 20),
                                (7, 8, 9, 30), (10, 11, 12, 40)])
        let game = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            try buffer.SetData(written)
            var read = [G.VertexPositionColor](
                repeating: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.Transparent),
                count: 4)
            try buffer.GetData(&read)
            game.observations["read"] = self.describe(read)
            try buffer.Dispose()
        }
        XCTAssertEqual(game.observations["read"], describe(written))
    }

    /// A window written at a byte offset leaves the rest of the buffer alone.
    ///
    /// The five-argument overload is the one that carries `offsetInBytes`, and
    /// a projection that dropped it would overwrite the first vertex instead of
    /// the second.
    func testAWindowWriteLeavesTheRestAlone() throws {
        try requireNative()
        let initial = vertices([(1, 1, 1, 11), (2, 2, 2, 22),
                                (3, 3, 3, 33), (4, 4, 4, 44)])
        let replacement = vertices([(9, 9, 9, 99)])
        let game = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            try buffer.SetData(initial)
            try buffer.SetData(16, data: replacement, startIndex: 0,
                               elementCount: 1, vertexStride: 0)
            var read = [G.VertexPositionColor](
                repeating: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.Transparent),
                count: 4)
            try buffer.GetData(&read)
            game.observations["read"] = self.describe(read)
            try buffer.Dispose()
        }
        XCTAssertEqual(
            game.observations["read"],
            "1.0,1.0,1.0,11 9.0,9.0,9.0,99 3.0,3.0,3.0,33 4.0,4.0,4.0,44")
    }

    /// `startIndex` selects a window of the caller's array, not of the buffer.
    func testTheArrayWindowIsTheCallersOwn() throws {
        try requireNative()
        let source = vertices([(1, 1, 1, 11), (2, 2, 2, 22),
                               (3, 3, 3, 33), (4, 4, 4, 44),
                               (5, 5, 5, 55), (6, 6, 6, 66)])
        let game = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 2, usage: .None)
            try buffer.SetData(source, startIndex: 2, elementCount: 2)
            var read = [G.VertexPositionColor](
                repeating: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.Transparent),
                count: 2)
            try buffer.GetData(&read)
            game.observations["read"] = self.describe(read)
            try buffer.Dispose()
        }
        XCTAssertEqual(game.observations["read"], "3.0,3.0,3.0,33 4.0,4.0,4.0,44")
    }

    /// `ValidateCopyParameters`' three refusals, in XNA's own order and with
    /// XNA's own parameter names.
    ///
    /// The first names **`dataIndex`** and not the caller's `startIndex`: the
    /// helper is shared and reports its own parameter. An index past the end is
    /// blamed on the index; only a *window* past the end is blamed on the count.
    func testTheCopyParameterRefusalsKeepTheirOwnNames() throws {
        try requireNative()
        _ = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            let data = self.vertices([(1, 1, 1, 1), (2, 2, 2, 2)])
            let message = G.BufferResources.mustBeValidIndex

            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(message, paramName: "dataIndex"),
                paramName: "dataIndex",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { try buffer.SetData(data, startIndex: 3, elementCount: 1) }

            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(message, paramName: "elementCount"),
                paramName: "elementCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { try buffer.SetData(data, startIndex: 1, elementCount: 2) }

            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(message, paramName: "elementCount"),
                paramName: "elementCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { try buffer.SetData(data, startIndex: 0, elementCount: 0) }

            game.observations["checked"] = "yes"
            try buffer.Dispose()
        }
    }

    /// An empty array is an `ArgumentNullException`, which is XNA's own quirk.
    ///
    /// `IL_001c: ldlen; IL_001d: brfalse IL_02be` branches to the same throw the
    /// null test uses, so a zero-length array reports `"data"` as null.
    func testAnEmptyArrayIsReportedAsNull() throws {
        try requireNative()
        _ = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    G.GraphicsDevice.nullNotAllowedMessage, paramName: "data"),
                paramName: "data",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) { try buffer.SetData([G.VertexPositionColor]()) }
            game.observations["checked"] = "yes"
            try buffer.Dispose()
        }
    }

    /// A transfer larger than the buffer is refused before any native call.
    func testATransferPastTheEndIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 2, usage: .None)
            let data = self.vertices([(1, 1, 1, 1), (2, 2, 2, 2), (3, 3, 3, 3)])
            assertProjected(
                CNAInvalidOperationException.self,
                message: G.BufferResources.resourceDataMustBeCorrectSize,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try buffer.SetData(data) }
            game.observations["checked"] = "yes"
            try buffer.Dispose()
        }
    }

    /// A vertex stride smaller than the element is refused, naming the stride.
    func testAStrideSmallerThanTheElementIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            let data = self.vertices([(1, 1, 1, 1), (2, 2, 2, 2)])
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    G.BufferResources.vertexStrideTooSmall, paramName: "vertexStride"),
                paramName: "vertexStride",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                try buffer.SetData(0, data: data, startIndex: 0,
                                   elementCount: 2, vertexStride: 8)
            }
            game.observations["checked"] = "yes"
            try buffer.Dispose()
        }
    }

    /// A `WriteOnly` buffer refuses `GetData` with XNA's own message.
    ///
    /// CNA refuses it natively too — `cna_vertex_buffer_get_data_raw` answers
    /// `NOT_SUPPORTED` on a write-only buffer, measured in
    /// `build-probe/f60_buffers.c` — but the check is made here, before the
    /// call, because XNA's is managed and carries a class and a message CNA's
    /// result code does not.
    func testWriteOnlyRefusesGetData() throws {
        try requireNative()
        _ = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 2, usage: .WriteOnly)
            assertProjected(
                CNANotSupportedException.self,
                message: G.BufferResources.writeOnlyGetNotSupported,
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                var read = [G.VertexPositionColor](
                    repeating: G.VertexPositionColor(F.Vector3(0, 0, 0), F.Color.Transparent),
                    count: 2)
                try buffer.GetData(&read)
            }
            // Writing to it still works.
            try buffer.SetData(self.vertices([(1, 1, 1, 1), (2, 2, 2, 2)]))
            game.observations["checked"] = "yes"
            try buffer.Dispose()
        }
    }

    /// A strided partial write is refused rather than written wrongly.
    ///
    /// XNA accepts `vertexStride > sizeof(T)` and writes `sizeof(T)` bytes every
    /// `vertexStride`; CNA's raw transfer moves whole vertices at the
    /// declaration's stride and cannot express the gap
    /// (`build-probe/f60_stride.c`). The refusal is on the runtime channel,
    /// **after** every XNA validation, so a call XNA would reject still reports
    /// XNA's exception.
    func testAStridedPartialWriteIsRefusedAndNotGuessed() throws {
        try requireNative()
        _ = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            let floats: [Float] = [1, 2, 3, 4]
            do {
                try buffer.SetData(0, data: floats, startIndex: 0,
                                   elementCount: 4, vertexStride: 16)
                XCTFail("a strided partial write must be refused")
            } catch let error as CNAError {
                XCTAssertTrue("\(error)".contains("unwritten bytes"), "\(error)")
            }
            game.observations["checked"] = "yes"
            try buffer.Dispose()
        }
    }

    // MARK: - Index transfers

    /// Both index widths round-trip, and the width is the buffer's own.
    func testIndexDataRoundTripsAtBothWidths() throws {
        try requireNative()
        let game = try run { game, device in
            let short = try G.IndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 6, usage: .None)
            try short.SetData([Int16(1), 2, 3, 4, 5, 6])
            var readShort = [Int16](repeating: 0, count: 6)
            try short.GetData(&readShort)
            game.observations["16"] = readShort.map(String.init).joined(separator: " ")
            game.observations["16 size"] = "\(short.IndexElementSize == .SixteenBits)"
            game.observations["16 count"] = "\(short.IndexCount)"
            try short.Dispose()

            // A 32-bit index buffer is REFUSED on this device, and that is
            // XNA's own rule rather than a limit of this binding: the device
            // reports GraphicsProfile.Reach, and Reach's IndexElementSize32 is
            // false in the capability table extracted from the assembly.
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile does not support 32 bit "
                    + "indices. Use IndexElementSize.SixteenBits or a type that "
                    + "has a size of two bytes.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                _ = try G.IndexBuffer(
                    graphicsDevice: device, indexElementSize: .ThirtyTwoBits,
                    indexCount: 6, usage: .None)
            }
            game.observations["profile"] = "\(device.GraphicsProfile)"
        }
        XCTAssertEqual(game.observations["16"], "1 2 3 4 5 6")
        XCTAssertEqual(game.observations["16 size"], "true")
        XCTAssertEqual(game.observations["16 count"], "6")
        XCTAssertEqual(game.observations["profile"], "Reach")
    }

    /// `IndexBuffer(GraphicsDevice, Type, …)` maps the four CLR index widths
    /// and refuses everything else with XNA's own message.
    func testTheIndexTypeOverloadMapsTheFourWidths() throws {
        try requireNative()
        let game = try run { game, device in
            let short = try G.IndexBuffer(
                graphicsDevice: device, indexType: UInt16.self,
                indexCount: 3, usage: .None)
            // typeof(int) maps to ThirtyTwoBits, which Reach then refuses --
            // the width map and the profile check are separate steps and the
            // message says which one spoke.
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile does not support 32 bit "
                    + "indices. Use IndexElementSize.SixteenBits or a type that "
                    + "has a size of two bytes.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                _ = try G.IndexBuffer(
                    graphicsDevice: device, indexType: Int32.self,
                    indexCount: 3, usage: .None)
            }
            game.observations["short"] = "\(short.IndexElementSize == .SixteenBits)"
            game.observations["long"] = "true"
            assertProjected(
                CNAArgumentException.self,
                message: G.BufferResources.indexBuffersMustBeSizedCorrectly,
                hResult: CNAArgumentException.corArgumentHResult
            ) {
                _ = try G.IndexBuffer(
                    graphicsDevice: device, indexType: Int64.self,
                    indexCount: 3, usage: .None)
            }
            try short.Dispose()
        }
        XCTAssertEqual(game.observations["short"], "true")
        XCTAssertEqual(game.observations["long"], "true")
    }

    /// A windowed index write leaves the indices around it alone.
    func testAWindowedIndexWriteLeavesTheRestAlone() throws {
        try requireNative()
        let game = try run { game, device in
            let buffer = try G.IndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 6, usage: .None)
            try buffer.SetData([Int16(1), 2, 3, 4, 5, 6])
            try buffer.SetData(4, data: [Int16(77), 88], startIndex: 0, elementCount: 2)
            var read = [Int16](repeating: 0, count: 6)
            try buffer.GetData(&read)
            game.observations["read"] = read.map(String.init).joined(separator: " ")
            try buffer.Dispose()
        }
        XCTAssertEqual(game.observations["read"], "1 2 77 88 5 6")
    }

    // MARK: - VertexBufferBinding

    /// The three constructors store what they are given, and refuse what XNA
    /// refuses in XNA's order.
    func testTheBindingValidatesInTheIlsOrder() throws {
        try requireNative()
        let game = try run { game, device in
            let buffer = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)

            let plain = try G.VertexBufferBinding(buffer)
            let offset = try G.VertexBufferBinding(buffer, 2)
            let instanced = try G.VertexBufferBinding(buffer, 1, 3)
            game.observations["plain"] =
                "\(plain.VertexOffset) \(plain.InstanceFrequency)"
            game.observations["offset"] =
                "\(offset.VertexOffset) \(offset.InstanceFrequency)"
            game.observations["instanced"] =
                "\(instanced.VertexOffset) \(instanced.InstanceFrequency)"
            game.observations["identity"] = "\(instanced.VertexBuffer === buffer)"
            let converted = try G.VertexBufferBinding.op_Implicit(buffer)
            game.observations["converted"] =
                "\(converted.VertexOffset) \(converted.InstanceFrequency)"

            let outOfRange = CNAArgumentOutOfRangeException
                .argArgumentOutOfRangeMessage
            // The offset is tested against the vertex count, unsigned, so both
            // a negative offset and one at the count are refused.
            for bad in [Int32(-1), 4, 5] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(outOfRange, paramName: "vertexOffset"),
                    paramName: "vertexOffset",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) { _ = try G.VertexBufferBinding(buffer, bad) }
            }
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    outOfRange, paramName: "instanceFrequency"),
                paramName: "instanceFrequency",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { _ = try G.VertexBufferBinding(buffer, 0, -1) }
            // Wrong in both: the offset is checked first and is what is blamed.
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(outOfRange, paramName: "vertexOffset"),
                paramName: "vertexOffset",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { _ = try G.VertexBufferBinding(buffer, 9, -1) }

            try buffer.Dispose()
        }
        XCTAssertEqual(game.observations["plain"], "0 0")
        XCTAssertEqual(game.observations["offset"], "2 0")
        XCTAssertEqual(game.observations["instanced"], "1 3")
        XCTAssertEqual(game.observations["identity"], "true")
        XCTAssertEqual(game.observations["converted"], "0 0")
    }

    // MARK: - Disposal

    /// A disposed buffer reports the disposal, on the CLR channel, before it
    /// looks at the arguments.
    func testADisposedBufferReportsTheDisposalFirst() throws {
        try requireNative()
        _ = try run { game, device in
            let vertex = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 4, usage: .None)
            try vertex.Dispose()
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object.\r\nObject name: 'VertexBuffer'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try vertex.SetData(self.vertices([(1, 1, 1, 1)])) }

            let index = try G.IndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 4, usage: .None)
            try index.Dispose()
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object.\r\nObject name: 'IndexBuffer'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try index.SetData([Int16(1)]) }
            game.observations["checked"] = "yes"
        }
    }

    /// `Dispose(false)` disposes and announces nothing; `Dispose(true)` does both.
    func testBothDisposalPathsReachTheBase() throws {
        try requireNative()
        let game = try run { game, device in
            let quiet = try G.VertexBuffer(
                graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                vertexCount: 2, usage: .None)
            var quietCount = 0
            _ = quiet.Disposing.Add { _, _ in quietCount += 1 }
            try quiet.Dispose(false)
            game.observations["finalizer"] = "\(quietCount) \(quiet.IsDisposed)"

            let loud = try G.IndexBuffer(
                graphicsDevice: device, indexElementSize: .SixteenBits,
                indexCount: 2, usage: .None)
            var loudCount = 0
            _ = loud.Disposing.Add { _, _ in loudCount += 1 }
            try loud.Dispose(true)
            game.observations["disposing"] = "\(loudCount) \(loud.IsDisposed)"
        }
        XCTAssertEqual(game.observations["finalizer"], "0 true")
        XCTAssertEqual(game.observations["disposing"], "1 true")
    }
}
