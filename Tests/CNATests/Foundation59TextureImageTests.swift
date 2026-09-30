// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class ImageProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((ImageProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (ImageProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 59: `SaveAsPng`, `SaveAsJpeg`, `FromStream`'s resizing overload,
/// and the three `Dispose(Boolean)` links the family was missing.
///
/// The encoded bytes are read back by `PortableNetworkGraphics`, which is in
/// this test target and knows nothing about CNA. Handing the output to
/// `Texture2D.FromStream` would have been a round trip through the same
/// encoder and decoder, and a transposed dimension or a swapped channel
/// survives that perfectly.
final class Foundation59TextureImageTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (ImageProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> ImageProbeGame {
        let game = try ImageProbeGame(body)
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

    private func written(_ stream: OutputStream) -> [UInt8] {
        guard let data = stream.property(
            forKey: .dataWrittenToMemoryStreamKey) as? Data else { return [] }
        return [UInt8](data)
    }

    private func describe(_ image: PortableNetworkGraphics.Image) -> String {
        var parts: [String] = ["\(image.width)x\(image.height)"]
        for y in 0..<image.height {
            for x in 0..<image.width {
                let texel = image.pixel(x: x, y: y)
                parts.append("\(texel.r),\(texel.g),\(texel.b),\(texel.a)")
            }
        }
        return parts.joined(separator: " ")
    }

    // MARK: - The decoder this suite depends on

    /// The independent decoder is itself checked, on a PNG built here.
    ///
    /// A decoder that agreed with a broken encoder would make every assertion
    /// below vacuous, so it is first pointed at a file whose bytes are written
    /// by hand: an uncompressed DEFLATE block, a `Sub`-filtered row and an
    /// `Up`-filtered row, carrying texels this test knows.
    func testTheIndependentDecoderReadsAPngWrittenByHand() throws {
        // Two rows of two RGBA texels, filtered so two of the five filters run.
        var scanlines: [UInt8] = []
        scanlines += [1]                                    // filter Sub
        scanlines += [10, 20, 30, 40]                       // (10,20,30,40)
        scanlines += [5, 5, 5, 5]                           // + left = (15,25,35,45)
        scanlines += [2]                                    // filter Up
        scanlines += [1, 1, 1, 1]                           // + above = (11,21,31,41)
        scanlines += [2, 2, 2, 2]                           // + above = (17,27,37,47)

        var zlib: [UInt8] = [0x78, 0x01]                    // CM=8, no preset dictionary
        let length = UInt16(scanlines.count)
        zlib += [0x01,                                       // final, stored block
                 UInt8(length & 0xFF), UInt8(length >> 8),
                 UInt8(~length & 0xFF), UInt8((~length >> 8) & 0xFF)]
        zlib += scanlines
        zlib += [0, 0, 0, 0]                                 // Adler-32, unchecked here

        func chunk(_ kind: String, _ body: [UInt8]) -> [UInt8] {
            var out: [UInt8] = []
            let count = UInt32(body.count)
            out += [UInt8(count >> 24 & 0xFF), UInt8(count >> 16 & 0xFF),
                    UInt8(count >> 8 & 0xFF), UInt8(count & 0xFF)]
            out += Array(kind.utf8)
            out += body
            out += [0, 0, 0, 0]                              // CRC, unchecked here
            return out
        }

        var png = PortableNetworkGraphics.signature
        png += chunk("IHDR", [0, 0, 0, 2, 0, 0, 0, 2, 8, 6, 0, 0, 0])
        png += chunk("IDAT", zlib)
        png += chunk("IEND", [])

        let image = try PortableNetworkGraphics.decode(png)
        XCTAssertEqual(describe(image),
                       "2x2 10,20,30,40 15,25,35,45 11,21,31,41 17,27,37,47")

        // And it refuses what it does not implement rather than guessing.
        XCTAssertThrowsError(try PortableNetworkGraphics.decode([1, 2, 3, 4]))
    }

    // MARK: - SaveAsPng

    /// Every authored texel comes back out of the file, in the authored order.
    func testSaveAsPngWritesTheAuthoredTexels() throws {
        try requireNative()
        let authored = colors([(255, 0, 0, 255), (0, 255, 0, 255),
                               (0, 0, 255, 255), (11, 22, 33, 255)])
        var encoded: [UInt8] = []
        _ = try run { _, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try texture.SetData(authored)
            let stream = OutputStream.toMemory()
            try texture.SaveAsPng(stream, width: 2, height: 2)
            stream.close()
            encoded = self.written(stream)
            try texture.Dispose()
        }
        let image = try PortableNetworkGraphics.decode(encoded)
        XCTAssertEqual(
            describe(image),
            "2x2 255,0,0,255 0,255,0,255 0,0,255,255 11,22,33,255")
    }

    /// A texel whose alpha is zero is written as `Color.Transparent`.
    ///
    /// `SaveAsImage` rewrites the whole array before encoding:
    /// `if (colors[i].A == 0) colors[i] = Color.Transparent;`. CNA's encoder
    /// does not — `build-probe/f59_encode.c` authored `(200,100,50,0)` and an
    /// independent decode read `(200,100,50,0)` straight back — so this
    /// asserts the projection's own step and would fail the moment it were
    /// dropped in favour of encoding the texture directly.
    func testSaveAsPngRewritesEveryAlphaZeroTexelToTransparent() throws {
        try requireNative()
        let authored = colors([(200, 100, 50, 0), (0, 255, 0, 255),
                               (7, 8, 9, 0), (11, 22, 33, 255)])
        var encoded: [UInt8] = []
        _ = try run { _, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try texture.SetData(authored)
            let stream = OutputStream.toMemory()
            try texture.SaveAsPng(stream, width: 2, height: 2)
            stream.close()
            encoded = self.written(stream)
            try texture.Dispose()
        }
        let image = try PortableNetworkGraphics.decode(encoded)
        XCTAssertEqual(
            describe(image),
            "2x2 0,0,0,0 0,255,0,255 0,0,0,0 11,22,33,255")
    }

    /// The two dimensions reach the encoder, in the right order.
    ///
    /// A non-square target is the point: a projection that transposed them, or
    /// passed the texture's own size, would produce a differently shaped file
    /// and this reads the shape out of the IHDR.
    func testSaveAsPngEncodesAtTheRequestedNonSquareSize() throws {
        try requireNative()
        var encoded: [UInt8] = []
        _ = try run { _, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try texture.SetData(self.colors([(255, 0, 0, 255), (0, 255, 0, 255),
                                             (0, 0, 255, 255), (255, 255, 255, 255)]))
            let stream = OutputStream.toMemory()
            try texture.SaveAsPng(stream, width: 6, height: 3)
            stream.close()
            encoded = self.written(stream)
            try texture.Dispose()
        }
        let image = try PortableNetworkGraphics.decode(encoded)
        XCTAssertEqual(image.width, 6)
        XCTAssertEqual(image.height, 3)
        // The corners keep the authored hues, which a transposed resample
        // would not: red at the origin, white at the far corner.
        XCTAssertEqual(image.pixel(x: 0, y: 0).r, 255)
        XCTAssertEqual(image.pixel(x: 0, y: 0).g, 0)
        XCTAssertEqual(image.pixel(x: 5, y: 2).r, 255)
        XCTAssertEqual(image.pixel(x: 5, y: 2).g, 255)
    }

    /// `SaveAsJpeg` writes a JPEG, and it is not the PNG path with a new name.
    ///
    /// JPEG is lossy, so the texels are not asserted; the container is. The
    /// SOI marker, the EOI marker, and the frame header's own dimensions are
    /// all read out of the bytes, which is enough to catch the two ways this
    /// could be wrong: the PNG format constant reaching the JPEG entry point,
    /// and the size arguments not reaching the encoder.
    func testSaveAsJpegWritesAJpegAtTheRequestedSize() throws {
        try requireNative()
        var encoded: [UInt8] = []
        _ = try run { _, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try texture.SetData(self.colors([(255, 0, 0, 255), (0, 255, 0, 255),
                                             (0, 0, 255, 255), (255, 255, 255, 255)]))
            let stream = OutputStream.toMemory()
            try texture.SaveAsJpeg(stream, width: 8, height: 4)
            stream.close()
            encoded = self.written(stream)
            try texture.Dispose()
        }
        XCTAssertGreaterThan(encoded.count, 4)
        XCTAssertEqual(Array(encoded.prefix(3)), [0xFF, 0xD8, 0xFF], "SOI")
        XCTAssertEqual(Array(encoded.suffix(2)), [0xFF, 0xD9], "EOI")
        XCTAssertNotEqual(Array(encoded.prefix(8)), PortableNetworkGraphics.signature)

        // The baseline/progressive frame header: FF C0..CF except C4/C8/CC,
        // then length, precision, height, width.
        var index = 2
        var frame: (width: Int, height: Int)?
        while index + 4 < encoded.count {
            guard encoded[index] == 0xFF else { index += 1; continue }
            let marker = encoded[index + 1]
            let length = Int(encoded[index + 2]) << 8 | Int(encoded[index + 3])
            if (0xC0...0xCF).contains(marker),
               marker != 0xC4, marker != 0xC8, marker != 0xCC {
                frame = (width: Int(encoded[index + 7]) << 8 | Int(encoded[index + 8]),
                         height: Int(encoded[index + 5]) << 8 | Int(encoded[index + 6]))
                break
            }
            index += 2 + length
        }
        XCTAssertEqual(frame?.width, 8)
        XCTAssertEqual(frame?.height, 4)
    }

    /// A stream that cannot be written raises the message XNA's IL pushes.
    ///
    /// `newobj ArgumentException::.ctor(string)` with `ldstr "stream"` — the
    /// one-argument overload — so `Message` is the parameter's name and
    /// `ParamName` is nil. That is XNA's own slip, and it is reproduced.
    func testSavingToAClosedStreamRaisesTheStreamArgumentException() throws {
        try requireNative()
        _ = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            let stream = OutputStream.toMemory()
            stream.open()
            stream.close()
            assertProjected(CNAArgumentException.self, message: "stream",
                            paramName: nil,
                            hResult: CNAArgumentException.corArgumentHResult) {
                try texture.SaveAsPng(stream, width: 2, height: 2)
            }
            assertProjected(CNAArgumentException.self, message: "stream",
                            paramName: nil,
                            hResult: CNAArgumentException.corArgumentHResult) {
                try texture.SaveAsJpeg(stream, width: 2, height: 2)
            }
            game.observations["closed"] = "checked"
            try texture.Dispose()
        }
    }

    /// A disposed texture fails where XNA's does: inside the data read.
    ///
    /// The stream checks come first — XNA validates the stream before it
    /// touches the surface — so a closed stream on a disposed texture reports
    /// the stream, not the disposal. Both orders are asserted.
    func testSavingADisposedTextureReportsTheDisposedObject() throws {
        try requireNative()
        _ = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try texture.Dispose()
            let live = OutputStream.toMemory()
            assertProjected(CNAObjectDisposedException.self,
                            message: "Cannot access a disposed object.\r\nObject name: 'Texture2D'.",
                            hResult: CNAObjectDisposedException.corObjectDisposedHResult) {
                try texture.SaveAsPng(live, width: 2, height: 2)
            }
            let closed = OutputStream.toMemory()
            closed.open()
            closed.close()
            assertProjected(CNAArgumentException.self, message: "stream",
                            paramName: nil,
                            hResult: CNAArgumentException.corArgumentHResult) {
                try texture.SaveAsPng(closed, width: 2, height: 2)
            }
            game.observations["disposed"] = "checked"
        }
    }

    /// A disposed texture reports the disposal from `SetData` and `GetData`,
    /// and reports it *before* it looks at the arguments.
    ///
    /// `CopyData`'s first instruction is `Helpers.CheckDisposed(this,
    /// pComPtr)` — ahead of `GetAndValidateSizes`, `GetAndValidateRect` and
    /// `ValidateTotalSize`, all three of which the public overloads reach only
    /// through it. So a disposed texture handed a wrong-sized array reports the
    /// disposal, on the CLR channel, naming the dynamic type. Both halves were
    /// wrong until Foundation 59: the failure arrived on the runtime channel as
    /// `CNAError.disposedObject`, and it arrived after the argument checks.
    func testDisposalIsReportedBeforeTheArgumentChecks() throws {
        try requireNative()
        _ = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try texture.Dispose()
            let disposed = "Cannot access a disposed object.\r\nObject name: 'Texture2D'."

            assertProjected(CNAObjectDisposedException.self, message: disposed,
                            hResult: CNAObjectDisposedException.corObjectDisposedHResult) {
                try texture.SetData(self.colors([(1, 2, 3, 4), (5, 6, 7, 8),
                                                 (9, 10, 11, 12), (13, 14, 15, 16)]))
            }
            // An array of the wrong size as well: the disposal still wins,
            // which is what proves the order rather than merely the class.
            assertProjected(CNAObjectDisposedException.self, message: disposed,
                            hResult: CNAObjectDisposedException.corObjectDisposedHResult) {
                try texture.SetData(self.colors([(1, 2, 3, 4)]))
            }
            assertProjected(CNAObjectDisposedException.self, message: disposed,
                            hResult: CNAObjectDisposedException.corObjectDisposedHResult) {
                var read = [F.Color](repeating: F.Color.Transparent, count: 1)
                try texture.GetData(&read)
            }
            game.observations["ordered"] = "checked"
        }
    }

    // MARK: - FromStream(width:height:zoom:)

    /// `zoom` reaches the decoder, and the two dimensions are not transposed.
    ///
    /// The source is 4×2, so a 6×3 target tells the two operations apart:
    /// cover-and-crop grants exactly 6×3, and fit grants the width with the
    /// height derived from the source's aspect. A projection that dropped
    /// `zoom`, or swapped the two dimensions, produces a different pair in
    /// every one of the four cases below.
    ///
    /// The granted numbers are **CNA's**, not XNA's: XNA's own resize happens
    /// inside `UnsafeNativeMethods.DecodeStreamToTexture`, unmanaged code that
    /// is not in the registered assembly's IL, so there is no pinned authority
    /// for the arithmetic. What the IL does decide — that `zoom` selects
    /// `Scale|Crop` over `Scale`, and that the decoder's granted dimensions
    /// become the texture's — is what this asserts.
    func testFromStreamPassesWidthHeightAndZoomToTheDecoder() throws {
        try requireNative()
        let game = try run { game, device in
            let source = try G.Texture2D(graphicsDevice: device, width: 4, height: 2)
            try source.SetData(self.colors([
                (255, 0, 0, 255), (0, 255, 0, 255), (0, 0, 255, 255), (255, 255, 0, 255),
                (255, 0, 255, 255), (0, 255, 255, 255), (255, 255, 255, 255), (0, 0, 0, 255),
            ]))
            let saved = OutputStream.toMemory()
            try source.SaveAsPng(saved, width: 4, height: 2)
            saved.close()
            let encoded = Data(self.written(saved))
            try source.Dispose()

            func decode(width: Int32, height: Int32, zoom: Bool) throws -> String {
                let texture = try G.Texture2D.FromStream(
                    device, stream: InputStream(data: encoded),
                    width: width, height: height, zoom: zoom)
                let described = "\(texture.Width)x\(texture.Height)"
                try texture.Dispose()
                return described
            }

            game.observations["natural"] = try {
                let texture = try G.Texture2D.FromStream(
                    device, stream: InputStream(data: encoded))
                let described = "\(texture.Width)x\(texture.Height)"
                try texture.Dispose()
                return described
            }()
            game.observations["6x3 zoom"] = try decode(width: 6, height: 3, zoom: true)
            game.observations["3x6 zoom"] = try decode(width: 3, height: 6, zoom: true)
            game.observations["6x3 fit"] = try decode(width: 6, height: 3, zoom: false)
            game.observations["3x6 fit"] = try decode(width: 3, height: 6, zoom: false)

            // A square source zoomed into a target wider than tall: CNA 0.21
            // refused it ("crop rectangle lies outside the source image",
            // SW-05); CNA 0.35 (BINDFIX-043) fills and crops like the rest --
            // except, still, where the crop is under one source pixel high.
            let square = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try square.SetData(self.colors([
                (255, 0, 0, 255), (0, 255, 0, 255), (0, 0, 255, 255), (255, 255, 0, 255),
            ]))
            let squareSaved = OutputStream.toMemory()
            try square.SaveAsPng(squareSaved, width: 2, height: 2)
            squareSaved.close()
            let squareEncoded = Data(self.written(squareSaved))
            try square.Dispose()
            let wide = try G.Texture2D.FromStream(
                device, stream: InputStream(data: squareEncoded),
                width: 4, height: 2, zoom: true)
            game.observations["square 4x2 zoom"] = "\(wide.Width)x\(wide.Height)"
            try wide.Dispose()
            do {
                let wider = try G.Texture2D.FromStream(
                    device, stream: InputStream(data: squareEncoded),
                    width: 8, height: 2, zoom: true)
                game.observations["square 8x2 zoom"] = "\(wider.Width)x\(wider.Height)"
                try wider.Dispose()
            } catch let error as CNAError {
                if case .nativeFailure(let operation, let result, _) = error {
                    game.observations["square 8x2 zoom"] = "\(operation)=\(result)"
                }
            }
        }
        XCTAssertEqual(game.observations["natural"], "4x2")
        XCTAssertEqual(game.observations["6x3 zoom"], "6x3")
        XCTAssertEqual(game.observations["3x6 zoom"], "3x6")
        // Fit scales to the requested width and derives the height from the
        // source's 2:1 aspect. Measured, not assumed: `build-probe/f59_decode.c`
        // sweeps sixteen targets under both settings.
        XCTAssertEqual(game.observations["6x3 fit"], "6x3")
        XCTAssertEqual(game.observations["3x6 fit"], "3x1")
        XCTAssertEqual(game.observations["square 4x2 zoom"], "4x2")
        // CURRENT UPSTREAM DEFECT (SW-05 residual, CNA 0.35 at 4228ff913): a
        // 2x2 source zoomed to 8x2 needs a half-pixel crop and is still refused
        // with INVALID_ARGUMENT. Reproducer:
        // cna/build-probe/qual-probes/sw-decode-zoom.c. Expect "8x2" once fixed.
        XCTAssertEqual(game.observations["square 8x2 zoom"],
                       "cna_texture2d_create_from_encoded_memory=1")
    }

    /// The decoded texels are the source's, so the stream really is decoded.
    func testFromStreamAtTheNaturalSizeReproducesTheAuthoredTexels() throws {
        try requireNative()
        let authored = colors([(255, 0, 0, 255), (0, 255, 0, 255),
                               (0, 0, 255, 255), (12, 34, 56, 255)])
        let game = try run { game, device in
            let source = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            try source.SetData(authored)
            let saved = OutputStream.toMemory()
            try source.SaveAsPng(saved, width: 2, height: 2)
            saved.close()
            let encoded = Data(self.written(saved))
            try source.Dispose()

            let decoded = try G.Texture2D.FromStream(
                device, stream: InputStream(data: encoded),
                width: 2, height: 2, zoom: true)
            var read = [F.Color](repeating: F.Color.Transparent, count: 4)
            try decoded.GetData(&read)
            game.observations["texels"] =
                read.map { "\($0.R),\($0.G),\($0.B),\($0.A)" }.joined(separator: " ")
            try decoded.Dispose()
        }
        XCTAssertEqual(game.observations["texels"],
                       "255,0,0,255 0,255,0,255 0,0,255,255 12,34,56,255")
    }

    // MARK: - Dispose(Boolean)

    /// `Dispose(false)` is the finalizer path and announces nothing.
    ///
    /// `GraphicsResource.Dispose(bool)` raises `Disposing` only through
    /// `~GraphicsResource()`, which the disposing branch alone calls; the
    /// finalizer branch runs `!GraphicsResource()` and `Object.Finalize()`,
    /// neither of which touches the delegate. Both paths still dispose.
    func testDisposeFalseDisposesWithoutRaisingDisposing() throws {
        try requireNative()
        let game = try run { game, device in
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            var announced = 0
            _ = texture.Disposing.Add { _, _ in announced += 1 }
            try texture.Dispose(false)
            game.observations["finalizer announced"] = "\(announced)"
            game.observations["finalizer disposed"] = "\(texture.IsDisposed)"

            let second = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            var announcedSecond = 0
            _ = second.Disposing.Add { _, _ in announcedSecond += 1 }
            try second.Dispose(true)
            game.observations["disposing announced"] = "\(announcedSecond)"
            game.observations["disposing disposed"] = "\(second.IsDisposed)"

            // And a second call on either path announces nothing more.
            try second.Dispose(true)
            game.observations["idempotent"] = "\(announcedSecond)"
        }
        XCTAssertEqual(game.observations["finalizer announced"], "0")
        XCTAssertEqual(game.observations["finalizer disposed"], "true")
        XCTAssertEqual(game.observations["disposing announced"], "1")
        XCTAssertEqual(game.observations["disposing disposed"], "true")
        XCTAssertEqual(game.observations["idempotent"], "1")
    }

    /// `SpriteBatch.Dispose(Boolean)` is the same link at the other subclass.
    func testSpriteBatchDisposeBooleanReachesTheBase() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            var announced = 0
            _ = batch.Disposing.Add { _, _ in announced += 1 }
            try batch.Dispose(false)
            game.observations["finalizer announced"] = "\(announced)"
            game.observations["finalizer disposed"] = "\(batch.IsDisposed)"

            let second = try G.SpriteBatch(graphicsDevice: device)
            var announcedSecond = 0
            _ = second.Disposing.Add { _, _ in announcedSecond += 1 }
            try second.Dispose(true)
            game.observations["disposing announced"] = "\(announcedSecond)"
            game.observations["disposing disposed"] = "\(second.IsDisposed)"
        }
        XCTAssertEqual(game.observations["finalizer announced"], "0")
        XCTAssertEqual(game.observations["finalizer disposed"], "true")
        XCTAssertEqual(game.observations["disposing announced"], "1")
        XCTAssertEqual(game.observations["disposing disposed"], "true")
    }

    /// A `RenderTarget2D` still reaches every link of the chain it now has.
    ///
    /// The chain is three deep since `Texture2D` declares its own override:
    /// `RenderTarget2D` releases the native content-lost subscription, then
    /// `Texture2D`, then `GraphicsResource`. A broken link would leave the
    /// registration alive or the resource undisposed.
    func testRenderTargetDisposeWalksTheWholeChain() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 2, height: 2)
            game.observations["registered"] =
                "\(target.contentLostRegistration != 0)"
            var announced = 0
            _ = target.Disposing.Add { _, _ in announced += 1 }
            try target.Dispose(true)
            game.observations["announced"] = "\(announced)"
            game.observations["disposed"] = "\(target.IsDisposed)"
            game.observations["released"] = "\(target.contentLostRegistration == 0)"
        }
        XCTAssertEqual(game.observations["registered"], "true")
        XCTAssertEqual(game.observations["announced"], "1")
        XCTAssertEqual(game.observations["disposed"], "true")
        XCTAssertEqual(game.observations["released"], "true")
    }
}
