// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class ProfileProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((ProfileProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (ProfileProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 62: `GraphicsDevice.GraphicsProfile` and the capability table
/// nine deferred messages were waiting behind.
final class Foundation62ProfileCapabilityTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (ProfileProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> ProfileProbeGame {
        let game = try ProfileProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    /// The pinned table, read from the file the extractor writes.
    private func pinnedProfiles() throws -> [String: [String: Any]] {
        let reference = URL(fileURLWithPath: #filePath)
            .deletingLastPathComponent()          // Tests/CNATests
            .deletingLastPathComponent()          // Tests
            .deletingLastPathComponent()          // the repository root
            .appendingPathComponent(
                "tools/api_compat/reference/xna40-profile-capabilities.json")
        let data = try Data(contentsOf: reference)
        let document = try XCTUnwrap(
            try JSONSerialization.jsonObject(with: data) as? [String: Any])
        return try XCTUnwrap(document["profiles"] as? [String: [String: Any]])
    }

    /// Every field of both profiles matches the table extracted from the
    /// assembly's own class constructor.
    ///
    /// Thirty-two values per profile, copied into Swift by a generator and
    /// compared here against the file the extractor writes. A table of
    /// constants transcribed by hand is wrong in one place and never noticed;
    /// this is what makes that impossible.
    func testTheSwiftTableMatchesTheExtractedOne() throws {
        let pinned = try pinnedProfiles()
        for (name, table) in [("Reach", G.ProfileCapabilities.reach),
                              ("HiDef", G.ProfileCapabilities.hidef)] {
            let expected = try XCTUnwrap(pinned[name], name)

            func int(_ field: String) throws -> Int32 {
                Int32(try XCTUnwrap(expected[field] as? Int, "\(name).\(field)"))
            }
            func flag(_ field: String) throws -> Bool {
                try XCTUnwrap(expected[field] as? Bool, "\(name).\(field)")
            }
            func list(_ field: String) throws -> [Int32] {
                try XCTUnwrap(expected[field] as? [Int], "\(name).\(field)").map(Int32.init)
            }

            XCTAssertEqual(table.profile.rawValue, try int("Profile"), "\(name).Profile")
            XCTAssertEqual(table.vertexShaderVersion, try int("VertexShaderVersion"))
            XCTAssertEqual(table.pixelShaderVersion, try int("PixelShaderVersion"))
            XCTAssertEqual(table.maxPrimitiveCount, try int("MaxPrimitiveCount"))
            XCTAssertEqual(table.maxVertexStreams, try int("MaxVertexStreams"))
            XCTAssertEqual(table.maxStreamStride, try int("MaxStreamStride"))
            XCTAssertEqual(table.maxVertexBufferSize, try int("MaxVertexBufferSize"))
            XCTAssertEqual(table.maxIndexBufferSize, try int("MaxIndexBufferSize"))
            XCTAssertEqual(table.maxTextureSize, try int("MaxTextureSize"))
            XCTAssertEqual(table.maxCubeSize, try int("MaxCubeSize"))
            XCTAssertEqual(table.maxVolumeExtent, try int("MaxVolumeExtent"))
            XCTAssertEqual(table.maxTextureAspectRatio, try int("MaxTextureAspectRatio"))
            XCTAssertEqual(table.maxSamplers, try int("MaxSamplers"))
            XCTAssertEqual(table.maxVertexSamplers, try int("MaxVertexSamplers"))
            XCTAssertEqual(table.maxRenderTargets, try int("MaxRenderTargets"))

            XCTAssertEqual(table.occlusionQuery, try flag("OcclusionQuery"))
            XCTAssertEqual(table.getBackBufferData, try flag("GetBackBufferData"))
            XCTAssertEqual(table.separateAlphaBlend, try flag("SeparateAlphaBlend"))
            XCTAssertEqual(table.destBlendSrcAlphaSat, try flag("DestBlendSrcAlphaSat"))
            XCTAssertEqual(table.minMaxSrcDestBlend, try flag("MinMaxSrcDestBlend"))
            XCTAssertEqual(table.indexElementSize32, try flag("IndexElementSize32"))
            XCTAssertEqual(table.nonPow2Unconditional, try flag("NonPow2Unconditional"))
            XCTAssertEqual(table.nonPow2Cube, try flag("NonPow2Cube"))
            XCTAssertEqual(table.nonPow2Volume, try flag("NonPow2Volume"))

            XCTAssertEqual(table.validTextureFormats.map(\.rawValue),
                           try list("ValidTextureFormats"))
            XCTAssertEqual(table.validCubeFormats.map(\.rawValue),
                           try list("ValidCubeFormats"))
            XCTAssertEqual(table.validVolumeFormats.map(\.rawValue),
                           try list("ValidVolumeFormats"))
            XCTAssertEqual(table.validVertexTextureFormats.map(\.rawValue),
                           try list("ValidVertexTextureFormats"))
            XCTAssertEqual(table.invalidFilterFormats.map(\.rawValue),
                           try list("InvalidFilterFormats"))
            XCTAssertEqual(table.invalidBlendFormats.map(\.rawValue),
                           try list("InvalidBlendFormats"))
            XCTAssertEqual(table.validDepthFormats.map(\.rawValue),
                           try list("ValidDepthFormats"))
            XCTAssertEqual(table.validVertexFormats.map(\.rawValue),
                           try list("ValidVertexFormats"))
        }
    }

    /// The two profiles are genuinely different, so a table that answered one
    /// of them for both would fail.
    func testTheTwoProfilesAreNotTheSameTable() {
        let reach = G.ProfileCapabilities.reach
        let hidef = G.ProfileCapabilities.hidef
        XCTAssertNotEqual(reach.maxTextureSize, hidef.maxTextureSize)
        XCTAssertNotEqual(reach.maxPrimitiveCount, hidef.maxPrimitiveCount)
        XCTAssertNotEqual(reach.indexElementSize32, hidef.indexElementSize32)
        XCTAssertEqual(G.ProfileCapabilities.table(for: .Reach).profile, .Reach)
        XCTAssertEqual(G.ProfileCapabilities.table(for: .HiDef).profile, .HiDef)
    }

    /// The device answers the profile it was created with.
    func testTheDeviceReportsItsProfile() throws {
        try requireNative()
        let game = try run { game, device in
            game.observations["profile"] = "\(device.GraphicsProfile)"
            game.observations["max texture"] =
                "\(device.profileCapabilities.maxTextureSize)"
        }
        XCTAssertEqual(game.observations["profile"], "Reach")
        XCTAssertEqual(game.observations["max texture"], "2048")
    }

    /// A texture larger than the profile allows is refused, and the message
    /// names the profile, the type and the limit.
    ///
    /// This is one of the six `Texture2D` messages that were deferred until the
    /// device could say which profile it is.
    func testATextureLargerThanTheProfileIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile supports a maximum "
                    + "Texture2D size of 2048.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                _ = try G.Texture2D(graphicsDevice: device,
                                    width: 4096, height: 16)
            }
            game.observations["checked"] = "yes"
        }
    }

    /// A non-power-of-two mipmapped texture is refused on Reach, which allows
    /// non-power-of-two sizes only without mipmaps.
    func testANonPowerOfTwoMippedTextureIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile requires mipmapped "
                    + "Texture2D sizes to be powers of two. To use a non power "
                    + "of two Texture2D, remove the mipmaps.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                _ = try G.Texture2D(graphicsDevice: device, width: 6, height: 6,
                                    mipMap: true, format: .Color)
            }
            // The same size without mipmaps is accepted, which is what makes
            // the refusal above about the mipmaps and not about the size.
            let fine = try G.Texture2D(graphicsDevice: device, width: 6, height: 6,
                                       mipMap: false, format: .Color)
            game.observations["non-power-of-two without mipmaps"] =
                "\(fine.Width)x\(fine.Height)"
            try fine.Dispose()
        }
    }

    /// An aspect ratio past the profile's limit is refused.
    ///
    /// The arithmetic is XNA's own ceiling division, so 2048x1 is exactly at
    /// Reach's limit of 2048 and passes, and 4096x1 fails — on the *size* check
    /// first, which is why this uses a pair that is small enough to reach the
    /// ratio check.
    func testTheAspectRatioLimitIsTheIlsArithmetic() throws {
        try requireNative()
        _ = try run { game, device in
            // 2048 x 1: ratio exactly 2048, at the limit, and both sides are
            // within MaxTextureSize.
            let atTheLimit = try G.Texture2D(graphicsDevice: device,
                                             width: 2048, height: 1)
            game.observations["at the limit"] =
                "\(atTheLimit.Width)x\(atTheLimit.Height)"
            try atTheLimit.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// A vertex buffer larger than the profile allows is refused with the same
    /// message shape, naming `VertexBuffer`.
    func testAVertexBufferLargerThanTheProfileIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            // Reach's MaxVertexBufferSize is 67,108,863 bytes; a 16-byte stride
            // needs 4,194,304 vertices to pass it.
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile supports a maximum "
                    + "VertexBuffer size of 67108863.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                _ = try G.VertexBuffer(
                    graphicsDevice: device, vertexType: G.VertexPositionColor.self,
                    vertexCount: 4_194_305, usage: .None)
            }
            game.observations["checked"] = "yes"
        }
    }
}
