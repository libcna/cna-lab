// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class DataProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((DataProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (DataProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 57: `SetData` and `GetData`, and the three validations that
/// stand in front of them.
///
/// Unlike anything drawn, texture data really is observable here: CNA's
/// transfer round-trips, so these assert pixel values rather than "the call
/// returned zero".
final class Foundation57TextureDataTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (DataProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> DataProbeGame {
        let game = try DataProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    private func colors(_ values: [(UInt8, UInt8, UInt8, UInt8)]) -> [F.Color] {
        values.map { F.Color(Int32($0.0), Int32($0.1), Int32($0.2), Int32($0.3)) }
    }

    private func describe(_ data: [F.Color]) -> String {
        data.map { "\($0.R),\($0.G),\($0.B),\($0.A)" }.joined(separator: " ")
    }

    /// What goes in comes back out, texel for texel.
    func testTheWholeSurfaceRoundTrips() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            let written = self.colors([(10, 20, 30, 40), (50, 60, 70, 80),
                                       (90, 100, 110, 120), (130, 140, 150, 160)])
            try texture.SetData(written)
            var read = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)), count: 4)
            try texture.GetData(&read)
            game.observations["written"] = self.describe(written)
            game.observations["read"] = self.describe(read)
            try texture.Dispose()
        }
        XCTAssertEqual(game.observations["read"], game.observations["written"])
        XCTAssertEqual(game.observations["read"],
                       "10,20,30,40 50,60,70,80 90,100,110,120 130,140,150,160")
    }

    /// A rectangle writes only where it is aimed, and reads back only what it
    /// covers. The transfer's `has_rectangle` and its bounds are the same
    /// field pair XNA's `GetAndValidateRect` fills in.
    func testARectangleWritesOnlyItsOwnTexels() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try texture.SetData(self.colors([(1, 1, 1, 1), (2, 2, 2, 2),
                                             (3, 3, 3, 3), (4, 4, 4, 4)]))
            let corner = self.colors([(9, 9, 9, 9)])
            try texture.SetData(0, rect: F.Rectangle(1, 1, 1, 1), data: corner,
                                startIndex: 0, elementCount: 1)
            var whole = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)), count: 4)
            try texture.GetData(&whole)
            game.observations["whole"] = self.describe(whole)

            var one = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)), count: 1)
            try texture.GetData(0, rect: F.Rectangle(1, 1, 1, 1), data: &one,
                                startIndex: 0, elementCount: 1)
            game.observations["corner"] = self.describe(one)
            try texture.Dispose()
        }
        XCTAssertEqual(game.observations["whole"], "1,1,1,1 2,2,2,2 3,3,3,3 9,9,9,9",
                       "only the aimed texel moved")
        XCTAssertEqual(game.observations["corner"], "9,9,9,9")
    }

    /// `GetAndValidateSizes<T>`: T must be the format's size, or divide it.
    /// A three-byte T divides neither 4 nor anything else here.
    func testAnElementSizeThatDoesNotDivideTheFormatIsRefused() throws {
        try requireNative()
        struct ThreeBytes { var a: UInt8; var b: UInt8; var c: UInt8 }
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            do {
                try texture.SetData([ThreeBytes(a: 0, b: 0, c: 0)])
                game.observations["threw"] = "nothing"
            } catch let error as CNAArgumentException {
                game.observations["threw"] = error.Message
            }
            try texture.Dispose()
        }
        XCTAssertEqual(
            game.observations["threw"],
            "The type you are using for T in this method is an invalid size "
            + "for this resource.")
    }

    /// XNA's size rule accepts a T that **divides** the format's size, not
    /// only one that equals it: `SetData<byte>` on a Color surface is legal,
    /// and sixteen bytes cover a 2x2 texture. CNA counts in elements of the
    /// format's own type, so the count converts through bytes — and the bytes
    /// have to land where the Color texels do.
    func testAnElementThatDividesTheFormatIsAccepted() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            // Four texels, spelled a byte at a time, in CNA's RGBA8 order.
            let bytes: [UInt8] = [11, 12, 13, 14,
                                  21, 22, 23, 24,
                                  31, 32, 33, 34,
                                  41, 42, 43, 44]
            try texture.SetData(bytes)
            var read = [F.Color](
                repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)), count: 4)
            try texture.GetData(&read)
            game.observations["read"] = self.describe(read)
            try texture.Dispose()
        }
        XCTAssertEqual(game.observations["read"],
                       "11,12,13,14 21,22,23,24 31,32,33,34 41,42,43,44",
                       "a byte array covers the surface exactly as a Color array does")
    }

    /// `ValidateTotalSize`: the array window's bytes must equal the region's,
    /// exactly — too few and too many are both refused.
    func testAWindowThatDoesNotCoverTheRegionIsRefused() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            func attempt(_ count: Int) -> String {
                do {
                    try texture.SetData(
                        [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)), count: count))
                    return "accepted"
                } catch let error as CNAArgumentException {
                    return error.Message
                } catch { return "other" }
            }
            game.observations["tooFew"] = attempt(3)
            game.observations["tooMany"] = attempt(5)
            game.observations["exact"] = attempt(4)
            try texture.Dispose()
        }
        let expected = "The size of the data passed in is too large or too "
            + "small for this resource."
        XCTAssertEqual(game.observations["tooFew"], expected)
        XCTAssertEqual(game.observations["tooMany"], expected)
        XCTAssertEqual(game.observations["exact"], "accepted")
    }

    /// `GetAndValidateRect`: a negative origin, a non-positive extent, and an
    /// edge past the surface are all refused, and the refusal names `rect`.
    func testAnInvalidRectangleIsRefused() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            let one = self.colors([(1, 1, 1, 1)])
            func attempt(_ rect: F.Rectangle) -> String {
                do {
                    try texture.SetData(0, rect: rect, data: one,
                                        startIndex: 0, elementCount: 1)
                    return "accepted"
                } catch let error as CNAArgumentException {
                    return error.ParamName ?? "no name"
                } catch { return "other" }
            }
            game.observations["negativeOrigin"] = attempt(F.Rectangle(-1, 0, 1, 1))
            game.observations["zeroExtent"] = attempt(F.Rectangle(0, 0, 0, 1))
            game.observations["pastTheEdge"] = attempt(F.Rectangle(2, 0, 1, 1))
            game.observations["overflowing"] =
                attempt(F.Rectangle(1, 0, Int32.max, 1))
            game.observations["valid"] = attempt(F.Rectangle(1, 1, 1, 1))
            do {
                try texture.SetData(0, rect: F.Rectangle(-1, 0, 1, 1), data: one,
                                    startIndex: 0, elementCount: 1)
            } catch let error as CNAArgumentException {
                game.observations["message"] = error.Message
            }
            try texture.Dispose()
        }
        for key in ["negativeOrigin", "zeroExtent", "pastTheEdge", "overflowing"] {
            XCTAssertEqual(game.observations[key], "rect", key)
        }
        XCTAssertEqual(game.observations["valid"], "accepted")
        XCTAssertEqual(
            game.observations["message"],
            "The rectangle is too large or too small for this resource."
            + "\r\nParameter name: rect")
    }

    /// The array window: `startIndex` and `elementCount` select a slice, and
    /// the slice is what crosses.
    func testTheArrayWindowSelectsWhatCrosses() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 1)
            let padded = self.colors([(0, 0, 0, 0), (0, 0, 0, 0),
                                      (7, 7, 7, 7), (8, 8, 8, 8)])
            try texture.SetData(padded, startIndex: 2, elementCount: 2)
            var read = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)), count: 2)
            try texture.GetData(&read)
            game.observations["read"] = self.describe(read)
            try texture.Dispose()
        }
        XCTAssertEqual(game.observations["read"], "7,7,7,7 8,8,8,8")
    }

    /// An **empty** array is an `ArgumentNullException` naming `"data"`.
    ///
    /// Added in Foundation 64, with the check it asserts: `CopyData`'s second
    /// test is `if (data == null || data.Length == 0)` and both arms reach the
    /// same `throw`, so a zero-length array reports the parameter as null. A
    /// Swift array cannot be null; it can be empty, and that is the reachable
    /// half of XNA's own test. `VertexBuffer.CopyData` had this from Foundation
    /// 60 and `Texture2D` did not — the mutation
    /// `texture2d-empty-array-not-reported-as-null` survived until this test
    /// existed.
    func testAnEmptyArrayIsReportedAsNull() throws {
        try requireNative()
        _ = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            let empty: [F.Color] = []
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    "This method does not accept null for this parameter.",
                    paramName: "data"),
                paramName: "data",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) { try texture.SetData(empty) }

            var readInto: [F.Color] = []
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    "This method does not accept null for this parameter.",
                    paramName: "data"),
                paramName: "data",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) { try texture.GetData(&readInto) }
            try texture.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// The array window is `Helpers.ValidateCopyParameters`, which raises
    /// `ArgumentOutOfRangeException(MustBeValidIndex)` naming `dataIndex` or
    /// `elementCount` — **not** `ArgumentException(InvalidTotalSize)` — and
    /// raises it before the element size and the rectangle are looked at.
    ///
    /// Also Foundation 64. Until then this projection checked the window inline,
    /// with the wrong exception class, the wrong message, the wrong parameter
    /// name, the wrong `HResult` and in the wrong order.
    func testTheArrayWindowIsValidateCopyParameters() throws {
        try requireNative()
        _ = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            let four = self.colors([(1, 1, 1, 1), (2, 2, 2, 2),
                                    (3, 3, 3, 3), (4, 4, 4, 4)])
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    "This parameter must be a valid index within the array.",
                    paramName: "dataIndex"),
                paramName: "dataIndex",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { try texture.SetData(four, startIndex: -1, elementCount: 4) }

            let countMessage = composedArgumentMessage(
                "This parameter must be a valid index within the array.",
                paramName: "elementCount")
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: countMessage, paramName: "elementCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { try texture.SetData(four, startIndex: 2, elementCount: 4) }

            // The window is checked BEFORE the element size, so a call that is
            // wrong in both ways reports the window.
            let bytes = [UInt8](repeating: 0, count: 4)
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: countMessage, paramName: "elementCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { try texture.SetData(bytes, startIndex: 0, elementCount: 99) }
            try texture.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// The format's byte size is XNA's own table, decoded from the pinned IL.
    /// `HalfVector4` and `HdrBlendable` really do share a D3D format and a
    /// size; the DXT formats have none, because they are block-compressed.
    func testTheFormatSizeTableIsTheOneTheIlDecodesTo() {
        typealias GG = Microsoft.Xna.Framework.Graphics
        XCTAssertEqual(GG.expectedByteSize(of: .Color), 4)
        XCTAssertEqual(GG.expectedByteSize(of: .Bgr565), 2)
        XCTAssertEqual(GG.expectedByteSize(of: .Alpha8), 1)
        XCTAssertEqual(GG.expectedByteSize(of: .Vector4), 16)
        XCTAssertEqual(GG.expectedByteSize(of: .HalfVector4), 8)
        XCTAssertEqual(GG.expectedByteSize(of: .HdrBlendable), 8)
        XCTAssertNil(GG.expectedByteSize(of: .Dxt1))
        XCTAssertNil(GG.expectedByteSize(of: .Dxt5))
    }
}
