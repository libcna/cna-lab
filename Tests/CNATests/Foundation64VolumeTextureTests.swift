// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class VolumeProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((VolumeProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (VolumeProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 64: `TextureCube` and `Texture3D`.
///
/// Two facts shape every test here, and they are different in kind:
///
/// * **No `Texture3D` can exist on a Reach device**, and that is *XNA's* rule.
///   `MaxVolumeExtent` is 0 in the table extracted from the pinned assembly, so
///   `ValidateCreationParameters` raises `ProfileFeatureNotSupported` before
///   anything native is touched. CNA agrees independently —
///   `cna_texture3d_create` answers `NOT_SUPPORTED` — but the refusal asserted
///   below is the managed one.
/// * **A `TextureCube` can be created here but its faces cannot be moved.**
///   `cna_texturecube_set_data` and `_get_data` answer `NOT_SUPPORTED` on the
///   qualified HEADLESS artifact for all six faces (`build-probe/f64_cube.c`).
///   That is a *renderer* limit, not an XNA rule, so the six transfer members
///   are implemented, every XNA validation in front of them is asserted, and
///   the transfer itself is asserted to be exactly that refusal and nothing
///   else.
final class Foundation64VolumeTextureTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (VolumeProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> VolumeProbeGame {
        let game = try VolumeProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    private func cube(_ device: G.GraphicsDevice, size: Int32 = 4) throws
        -> G.TextureCube {
        try G.TextureCube(graphicsDevice: device, size: size, mipMap: false,
                          format: .Color)
    }

    // ------------------------------------------------------------------
    // TextureCube creation.

    /// A cube reports what CNA granted, not what was asked for.
    func testACubeReportsWhatWasGranted() throws {
        try requireNative()
        let game = try run { game, device in
            let cube = try self.cube(device, size: 4)
            game.observations["size"] = "\(cube.Size)"
            game.observations["levels"] = "\(cube.LevelCount)"
            game.observations["format"] = "\(cube.Format)"
            game.observations["device"] = "\(cube.GraphicsDevice === device)"
            game.observations["name"] = "\(cube.Name == nil)"
            try cube.Dispose()
            game.observations["disposed"] = "\(cube.IsDisposed)"
        }
        XCTAssertEqual(game.observations["size"], "4")
        XCTAssertEqual(game.observations["levels"], "1")
        XCTAssertEqual(game.observations["format"], "Color")
        XCTAssertEqual(game.observations["device"], "true")
        // `GraphicsResource.Name` is nil until a caller assigns it: no XNA
        // constructor assigns it either, which is why it projects Optional.
        XCTAssertEqual(game.observations["name"], "true")
        XCTAssertEqual(game.observations["disposed"], "true")
    }

    /// `size <= 0` is `ArgumentOutOfRangeException("size")`, the first test in
    /// `ValidateCreationParameters` and ahead of every profile test.
    func testANonPositiveSizeIsRefusedFirst() throws {
        try requireNative()
        _ = try run { game, device in
            for bad in [Int32(0), -1] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(
                        "Resource size must be greater than zero.",
                        paramName: "size"),
                    paramName: "size",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) {
                    _ = try G.TextureCube(graphicsDevice: device, size: bad,
                                          mipMap: false, format: .Color)
                }
            }
            game.observations["checked"] = "yes"
        }
    }

    /// Reach's `MaxCubeSize` is 512 — a quarter of its `MaxTextureSize`. The
    /// message names the profile and the limit, in that order.
    func testACubeLargerThanTheProfileAllowsIsRefused() throws {
        try requireNative()
        let game = try run { game, device in
            // 512 is exactly the limit and is accepted; the failure would be a
            // `<` written where XNA has `<=`.
            let atTheLimit = try self.cube(device, size: 512)
            game.observations["at the limit"] = "\(atTheLimit.Size)"
            try atTheLimit.Dispose()

            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile supports a maximum "
                    + "TextureCube size of 512.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) { _ = try self.cube(device, size: 1024) }
        }
        XCTAssertEqual(game.observations["at the limit"], "512")
    }

    /// Reach's `NonPow2Cube` is false, so a cube's size must be a power of two
    /// — **unconditionally**, with no mipmap or compression qualifier, and with
    /// the plain `ProfileNotPowerOfTwo` message rather than `Texture2D`'s
    /// mipmap-specific one.
    func testANonPowerOfTwoCubeIsRefusedEvenWithoutMipmaps() throws {
        try requireNative()
        _ = try run { game, device in
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile requires TextureCube "
                    + "sizes to be powers of two.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) { _ = try self.cube(device, size: 6) }
            game.observations["checked"] = "yes"
        }
    }

    /// The cube's format list is `ValidCubeFormats`, which is **not**
    /// `ValidTextureFormats`: Reach has nine texture formats and seven cube
    /// formats, and `Bgr565` is in both while `NormalizedByte4` is in neither.
    func testTheCubeFormatListIsItsOwn() throws {
        try requireNative()
        _ = try run { game, device in
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile does not support "
                    + "TextureCube format NormalizedByte4.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                _ = try G.TextureCube(graphicsDevice: device, size: 4,
                                      mipMap: false, format: .NormalizedByte4)
            }
            game.observations["checked"] = "yes"
        }
    }

    // ------------------------------------------------------------------
    // TextureCube transfer: the validations, then the renderer's refusal.

    /// An **empty** array is an `ArgumentNullException` naming `"data"`.
    ///
    /// `CopyData`'s second test is `if (data == null || data.Length == 0)` and
    /// both arms reach the same `throw`, so a zero-length array reports the
    /// parameter as null. A Swift array cannot be null; it can be empty, and
    /// that is the reachable half of XNA's own test.
    func testAnEmptyArrayIsReportedAsNull() throws {
        try requireNative()
        _ = try run { game, device in
            let cube = try self.cube(device)
            let empty: [F.Color] = []
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    "This method does not accept null for this parameter.",
                    paramName: "data"),
                paramName: "data",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) { try cube.SetData(.PositiveX, data: empty) }

            var readInto: [F.Color] = []
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    "This method does not accept null for this parameter.",
                    paramName: "data"),
                paramName: "data",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) { try cube.GetData(.PositiveX, data: &readInto) }
            try cube.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// The array window is `Helpers.ValidateCopyParameters`, which raises
    /// `ArgumentOutOfRangeException` naming `dataIndex` or `elementCount` —
    /// **not** the total-size `ArgumentException` — and raises it before the
    /// element size and the rectangle are looked at.
    func testTheArrayWindowIsValidateCopyParameters() throws {
        try requireNative()
        _ = try run { game, device in
            let cube = try self.cube(device)
            let sixteen = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)),
                                    count: 16)
            let outOfRange = composedArgumentMessage(
                "This parameter must be a valid index within the array.",
                paramName: "dataIndex")
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: outOfRange, paramName: "dataIndex",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                try cube.SetData(.PositiveX, data: sixteen,
                                 startIndex: -1, elementCount: 16)
            }

            let countMessage = composedArgumentMessage(
                "This parameter must be a valid index within the array.",
                paramName: "elementCount")
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: countMessage, paramName: "elementCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                try cube.SetData(.PositiveX, data: sixteen,
                                 startIndex: 8, elementCount: 16)
            }

            // The window is checked BEFORE the element size: a call that is
            // wrong in both ways reports the window.
            let bytes = [UInt8](repeating: 0, count: 16)
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: countMessage, paramName: "elementCount",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) {
                try cube.SetData(.PositiveX, data: bytes,
                                 startIndex: 0, elementCount: 99)
            }
            try cube.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// The element size, the rectangle and the total size, each with its own
    /// message, in `CopyData`'s order.
    func testTheThreeTransferValidations() throws {
        try requireNative()
        _ = try run { game, device in
            let cube = try self.cube(device)
            struct ThreeBytes { var a: UInt8; var b: UInt8; var c: UInt8 }

            // sizeof(T) neither equals 4 nor divides it.
            assertProjected(
                CNAArgumentException.self,
                message: "The type you are using for T in this method is an "
                    + "invalid size for this resource.",
                hResult: CNAArgumentException.corArgumentHResult
            ) {
                try cube.SetData(
                    .PositiveX, level: 0, rect: nil,
                    data: [ThreeBytes](repeating: ThreeBytes(a: 0, b: 0, c: 0),
                                       count: 16),
                    startIndex: 0, elementCount: 16)
            }

            let sixteen = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)),
                                    count: 16)
            assertProjected(
                CNAArgumentException.self,
                message: composedArgumentMessage(
                    "The rectangle is too large or too small for this resource.",
                    paramName: "rect"),
                paramName: "rect",
                hResult: CNAArgumentException.corArgumentHResult
            ) {
                try cube.SetData(.PositiveX, level: 0,
                                 rect: F.Rectangle(0, 0, 8, 8), data: sixteen,
                                 startIndex: 0, elementCount: 16)
            }

            assertProjected(
                CNAArgumentException.self,
                message: "The size of the data passed in is too large or too "
                    + "small for this resource.",
                hResult: CNAArgumentException.corArgumentHResult
            ) {
                try cube.SetData(.PositiveX, data: sixteen,
                                 startIndex: 0, elementCount: 8)
            }
            try cube.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// A disposed cube reports the disposal, ahead of every argument test.
    func testADisposedCubeReportsTheDisposalFirst() throws {
        try requireNative()
        _ = try run { game, device in
            let cube = try self.cube(device)
            try cube.Dispose()
            // Every argument below is ALSO wrong; the disposal still wins,
            // because `Helpers.CheckDisposed` is `CopyData`'s first
            // instruction.
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object."
                    + "\r\nObject name: 'TextureCube'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) {
                try cube.SetData(.PositiveX, data: [F.Color](),
                                 startIndex: -5, elementCount: -5)
            }
            game.observations["checked"] = "yes"
        }
    }

    /// The transfer itself, on each qualified artifact.
    ///
    /// Every XNA validation passes and the call reaches
    /// `cna_texturecube_set_data`. Cube storage is the renderer's: HEADLESS
    /// answers `CNA_RESULT_NOT_SUPPORTED` for every face, and OPENGLES3 (like
    /// SOFTWARE, `docs/foundation-64-volume-texture-evidence.md`) stores all
    /// six. Both directions are asserted in full rather than guessed from a
    /// renderer name: a refusal must name the route for all six faces, and an
    /// acceptance must read back exactly what was written for all six.
    func testTheFaceTransferReachesTheRouteAndIsRefusedByTheRenderer() throws {
        try requireNative()
        let game = try run { game, device in
            let cube = try self.cube(device)
            var outcomes: [String] = []
            for (index, face) in [G.CubeMapFace.PositiveX, .NegativeX, .PositiveY,
                                  .NegativeY, .PositiveZ, .NegativeZ].enumerated() {
                let value = Int32(index + 1)
                let sixteen = [F.Color](repeating: F.Color(value, Int32(2), Int32(3), Int32(4)),
                                        count: 16)
                do {
                    try cube.SetData(face, data: sixteen)
                    var back = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)),
                                         count: 16)
                    try cube.GetData(face, data: &back)
                    outcomes.append(back.map(\.PackedValue) == sixteen.map(\.PackedValue)
                                    ? "round-trip" : "mismatch")
                } catch let error as CNAError {
                    guard case .nativeFailure(let operation, let result, _) = error else {
                        outcomes.append("other CNAError")
                        continue
                    }
                    outcomes.append("\(operation)=\(result)")
                } catch {
                    outcomes.append("non-CNAError")
                }
            }
            game.observations["set"] = Set(outcomes).sorted().joined(separator: "|")
            try cube.Dispose()
        }
        // 6 == CNA_RESULT_NOT_SUPPORTED. One distinct outcome for all six
        // faces: the renderer's refusal naming the route, or a faithful
        // round trip of every face.
        XCTAssertTrue(
            ["cna_texturecube_set_data=6", "round-trip"].contains(game.observations["set"]),
            "got \(game.observations["set"] ?? "<none>")")
    }

    // ------------------------------------------------------------------
    // Texture3D: complete, and unreachable on this profile.

    /// **Every** `Texture3D` is refused on a Reach device, because
    /// `MaxVolumeExtent` is zero — and the refusal is the profile's, ahead of
    /// the format list and every extent test.
    func testNoVolumeTextureExistsOnAReachDevice() throws {
        try requireNative()
        let game = try run { game, device in
            for (w, h, d) in [(2, 2, 2), (1, 1, 1), (256, 256, 256)] {
                assertProjected(
                    CNANotSupportedException.self,
                    message: "XNA Framework Reach profile does not support Texture3D.",
                    hResult: CNANotSupportedException.corNotSupportedHResult
                ) {
                    _ = try G.Texture3D(
                        graphicsDevice: device, width: Int32(w), height: Int32(h),
                        depth: Int32(d), mipMap: false, format: .Color)
                }
            }
            game.observations["extent"] = "\(device.profileCapabilities.maxVolumeExtent)"
            game.observations["formats"] =
                "\(device.profileCapabilities.validVolumeFormats.count)"
        }
        XCTAssertEqual(game.observations["extent"], "0")
        XCTAssertEqual(game.observations["formats"], "0")
    }

    /// The three extent guards run **before** the profile test, so a
    /// non-positive extent reports its own parameter rather than the profile.
    func testANonPositiveExtentIsRefusedBeforeTheProfileTest() throws {
        try requireNative()
        _ = try run { game, device in
            for (w, h, d, name) in [(0, 2, 2, "width"), (2, 0, 2, "height"),
                                    (2, 2, 0, "depth")] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(
                        "Resource size must be greater than zero.",
                        paramName: name),
                    paramName: name,
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) {
                    _ = try G.Texture3D(
                        graphicsDevice: device, width: Int32(w), height: Int32(h),
                        depth: Int32(d), mipMap: false, format: .Color)
                }
            }
            game.observations["checked"] = "yes"
        }
    }

    /// The volume validator on a **HiDef** table, where it is not short-circuited
    /// by a zero extent.
    ///
    /// No HiDef device exists on this host, so this exercises the extracted
    /// table directly rather than a device: it is the only way to reach the
    /// four checks that Reach's `MaxVolumeExtent == 0` stands in front of, and
    /// the table it uses is the one the pinned assembly yielded.
    func testTheVolumeChecksBeyondTheZeroExtent() throws {
        let hidef = G.ProfileCapabilities.hidef
        XCTAssertEqual(hidef.maxVolumeExtent, 256)

        // Accepted: a legal HiDef volume.
        XCTAssertNoThrow(try hidef.validateVolumeCreation(
            width: 16, height: 16, depth: 16, format: .Color))

        // The format list: HiDef has fifteen volume formats and NormalizedByte4
        // is not one of them.
        assertProjected(
            CNANotSupportedException.self,
            message: "XNA Framework HiDef profile does not support Texture3D "
                + "format NormalizedByte4.",
            hResult: CNANotSupportedException.corNotSupportedHResult
        ) {
            try hidef.validateVolumeCreation(
                width: 16, height: 16, depth: 16, format: .NormalizedByte4)
        }

        // The extent, compared against all three dimensions.
        assertProjected(
            CNANotSupportedException.self,
            message: "XNA Framework HiDef profile supports a maximum Texture3D "
                + "size of 256.",
            hResult: CNANotSupportedException.corNotSupportedHResult
        ) {
            try hidef.validateVolumeCreation(
                width: 16, height: 16, depth: 512, format: .Color)
        }

        // The aspect ratio is `Max(Max(w, h), d)` over `Min(Min(w, h), d)` --
        // ALL THREE extents. 256x256x1 has a ratio of 256 and passes HiDef's
        // limit of 2048; a two-dimensional reading of the same call would
        // compute 1 and could never fail. The power-of-two test is what a
        // ratio past 2048 would have to get past first, so the ratio is
        // asserted where it holds rather than where it cannot be isolated.
        XCTAssertNoThrow(try hidef.validateVolumeCreation(
            width: 256, height: 256, depth: 1, format: .Color))

        // NonPow2Volume is true on HiDef, so a non-power-of-two volume is
        // accepted there and refused on Reach -- which is the difference the
        // flag names.
        XCTAssertNoThrow(try hidef.validateVolumeCreation(
            width: 24, height: 16, depth: 16, format: .Color))
        XCTAssertFalse(G.ProfileCapabilities.reach.nonPow2Volume)
        XCTAssertTrue(hidef.nonPow2Volume)
    }

    /// The box validator, which is `GetAndValidateRect` in three dimensions.
    ///
    /// Reach cannot construct a `Texture3D`, so the box arithmetic is exercised
    /// where it lives — on the shared helper — with the extents a HiDef volume
    /// would report. Its six comparisons are unsigned, so a negative coordinate
    /// wraps rather than slipping through, and its message names `"box"`, which
    /// is not the name of any parameter the six overloads take.
    func testTheBoxValidatorIsUnsignedAndNamesBox() {
        typealias GG = Microsoft.Xna.Framework.Graphics
        // (left, top, right, bottom, front, back, accepted)
        let cases: [(Int32, Int32, Int32, Int32, Int32, Int32, Bool)] = [
            (0, 0, 4, 4, 0, 4, true),
            (1, 1, 3, 3, 1, 3, true),
            (0, 0, 5, 4, 0, 4, false),   // right past the width
            (2, 0, 2, 4, 0, 4, false),   // left == right
            (0, 0, 4, 5, 0, 4, false),   // bottom past the height
            (0, 2, 4, 2, 0, 4, false),   // top == bottom
            (0, 0, 4, 4, 0, 5, false),   // back past the depth
            (0, 0, 4, 4, 2, 2, false),   // front == back
            (-1, 0, 4, 4, 0, 4, false),  // negative left, caught unsigned
            (0, 0, -1, 4, 0, 4, false),  // negative right, caught unsigned
        ]
        for (left, top, right, bottom, front, back, accepted) in cases {
            let ok = GG.volumeBoxIsValid(
                width: 4, height: 4, depth: 4, left: left, top: top,
                right: right, bottom: bottom, front: front, back: back)
            XCTAssertEqual(ok, accepted,
                           "box (\(left),\(top),\(right),\(bottom),\(front),\(back))")
        }
    }

    /// The volume aspect ratio takes its extremes over **all three** extents.
    ///
    /// Tested directly because it cannot be reached through `Texture3D`: every
    /// extent is bounded by `maxVolumeExtent` first -- 256 on HiDef, and Reach
    /// refuses volumes outright -- so the ratio never exceeds 256 against a
    /// limit of 2048. A mutation taking the extremes over width and height
    /// only survived the entire public surface, and this is what catches it.
    func testVolumeAspectExtremesUseTheDepth() {
        typealias Caps = Microsoft.Xna.Framework.Graphics.ProfileCapabilities
        // Depth is the longest extent.
        let deep = Caps.volumeAspectExtremes(4, 4, 64)
        XCTAssertEqual(deep.longer, 64, "depth must be able to be the longest")
        XCTAssertEqual(deep.shorter, 4)
        // Depth is the shortest extent.
        let flat = Caps.volumeAspectExtremes(64, 64, 4)
        XCTAssertEqual(flat.longer, 64)
        XCTAssertEqual(flat.shorter, 4, "depth must be able to be the shortest")
    }

}
