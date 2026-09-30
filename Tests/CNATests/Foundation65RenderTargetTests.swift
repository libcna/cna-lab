// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class TargetProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((TargetProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (TargetProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 65: `RenderTargetCube`, `RenderTargetBinding` and the device's
/// three remaining render-target members.
///
/// The whole family binds on this artifact — `build-probe/f65_rtcube.c` binds a
/// cube face, binds two distinct targets at once, reads the count back and
/// copies the binding array — so unlike the cube *transfer* of Foundation 64,
/// nothing here is renderer-blocked. What is still blocked is what was always
/// blocked: **no pixel that reaches a target can be read**, so nothing below
/// asserts a rendered result.
final class Foundation65RenderTargetTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (TargetProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> TargetProbeGame {
        let game = try TargetProbeGame(body)
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
        -> G.RenderTargetCube {
        try G.RenderTargetCube(
            graphicsDevice: device, size: size, mipMap: false,
            preferredFormat: .Color, preferredDepthFormat: .None)
    }

    private func target(_ device: G.GraphicsDevice, side: Int32 = 8) throws
        -> G.RenderTarget2D {
        try G.RenderTarget2D(graphicsDevice: device, width: side, height: side)
    }

    // ------------------------------------------------------------------
    // RenderTargetCube.

    /// A cube target reports what CNA granted — **through the render-target
    /// route, not the cube route.**
    ///
    /// `cna_texturecube_get_info` accepts a render-target-cube handle and
    /// answers `SUCCESS` with size 0, level count 0 and format 0
    /// (`build-probe/f65_rtcube.c`). A projection that read the inherited path
    /// would report a zero-sized cube with no call having failed, so `Size`
    /// coming back as 4 is the assertion that the right route was read.
    func testACubeTargetReportsWhatWasGranted() throws {
        try requireNative()
        let game = try run { game, device in
            let cube = try self.cube(device, size: 4)
            game.observations["size"] = "\(cube.Size)"
            game.observations["levels"] = "\(cube.LevelCount)"
            game.observations["format"] = "\(cube.Format)"
            game.observations["depth"] = "\(cube.DepthStencilFormat)"
            game.observations["samples"] = "\(cube.MultiSampleCount)"
            game.observations["usage"] = "\(cube.RenderTargetUsage)"
            game.observations["lost"] = "\(cube.IsContentLost)"
            game.observations["subscribed"] = "\(cube.contentLostRegistration != 0)"
            let asTextureCube: G.TextureCube = cube
            game.observations["is a TextureCube"] = "\(asTextureCube === cube)"
            let asTexture: G.Texture = cube
            game.observations["is a Texture"] = "\(asTexture === cube)"
            try cube.Dispose()
            game.observations["released"] = "\(cube.contentLostRegistration == 0)"
        }
        XCTAssertEqual(game.observations["size"], "4")
        XCTAssertEqual(game.observations["levels"], "1")
        XCTAssertEqual(game.observations["format"], "Color")
        XCTAssertEqual(game.observations["depth"], "None")
        XCTAssertEqual(game.observations["samples"], "0")
        XCTAssertEqual(game.observations["usage"], "DiscardContents")
        XCTAssertEqual(game.observations["lost"], "false")
        XCTAssertEqual(game.observations["subscribed"], "true")
        XCTAssertEqual(game.observations["is a TextureCube"], "true")
        XCTAssertEqual(game.observations["is a Texture"], "true")
        XCTAssertEqual(game.observations["released"], "true")
    }

    /// The cube target runs `TextureCube.ValidateCreationParameters` — the same
    /// five tests on the same extracted table — so the profile refuses the same
    /// sizes and formats it refuses for a plain cube.
    func testTheCubeTargetRunsTheCubeProfileChecks() throws {
        try requireNative()
        _ = try run { game, device in
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: composedArgumentMessage(
                    "Resource size must be greater than zero.", paramName: "size"),
                paramName: "size",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { _ = try self.cube(device, size: 0) }

            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile supports a maximum "
                    + "TextureCube size of 512.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) { _ = try self.cube(device, size: 1024) }

            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile requires TextureCube "
                    + "sizes to be powers of two.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) { _ = try self.cube(device, size: 6) }
            game.observations["checked"] = "yes"
        }
    }

    // ------------------------------------------------------------------
    // RenderTargetBinding.

    /// The binding stores what it was given, and a 2D target's face is
    /// `PositiveX` — the value `ldc.i4.0` puts there, which means "no face"
    /// rather than "unset".
    func testTheBindingStoresWhatItWasGiven() throws {
        try requireNative()
        let game = try run { game, device in
            let flat = try self.target(device)
            let curved = try self.cube(device)

            let flatBinding = G.RenderTargetBinding(flat)
            game.observations["flat target"] = "\(flatBinding.RenderTarget === flat)"
            game.observations["flat face"] = "\(flatBinding.CubeMapFace)"

            let cubeBinding = G.RenderTargetBinding(curved, .NegativeZ)
            game.observations["cube target"] = "\(cubeBinding.RenderTarget === curved)"
            game.observations["cube face"] = "\(cubeBinding.CubeMapFace)"

            // op_Implicit is the named static an implicit conversion projects
            // to; ten bytes of IL that call the one-argument constructor.
            let converted = G.RenderTargetBinding.op_Implicit(flat)
            game.observations["converted"] = "\(converted.RenderTarget === flat)"
            game.observations["converted face"] = "\(converted.CubeMapFace)"

            try flat.Dispose()
            try curved.Dispose()
        }
        XCTAssertEqual(game.observations["flat target"], "true")
        XCTAssertEqual(game.observations["flat face"], "PositiveX")
        XCTAssertEqual(game.observations["cube target"], "true")
        XCTAssertEqual(game.observations["cube face"], "NegativeZ")
        XCTAssertEqual(game.observations["converted"], "true")
        XCTAssertEqual(game.observations["converted face"], "PositiveX")
    }

    // ------------------------------------------------------------------
    // The device members.

    /// What is bound is what comes back — the same objects, and the face with
    /// them.
    func testWhatIsBoundComesBackByIdentity() throws {
        try requireNative()
        let game = try run { game, device in
            game.observations["initially"] = "\(device.GetRenderTargets().count)"

            let flat = try self.target(device)
            try device.SetRenderTarget(flat)
            let afterFlat = device.GetRenderTargets()
            game.observations["flat count"] = "\(afterFlat.count)"
            game.observations["flat identity"] =
                "\(afterFlat.first?.RenderTarget === flat)"
            game.observations["flat face"] = "\(afterFlat.first?.CubeMapFace ?? .NegativeZ)"

            let curved = try self.cube(device)
            try device.SetRenderTarget(curved, cubeMapFace: .PositiveY)
            let afterCube = device.GetRenderTargets()
            game.observations["cube count"] = "\(afterCube.count)"
            game.observations["cube identity"] =
                "\(afterCube.first?.RenderTarget === curved)"
            game.observations["cube face"] = "\(afterCube.first?.CubeMapFace ?? .NegativeZ)"

            try device.SetRenderTarget(nil as G.RenderTarget2D?)
            game.observations["after unbind"] = "\(device.GetRenderTargets().count)"

            // A nil CUBE also restores the backbuffer, and the face it is given
            // is ignored: XNA's null branch never builds a binding.
            try device.SetRenderTarget(curved, cubeMapFace: .PositiveY)
            try device.SetRenderTarget(nil as G.RenderTargetCube?, cubeMapFace: .NegativeX)
            game.observations["after nil cube"] = "\(device.GetRenderTargets().count)"

            try flat.Dispose()
            try curved.Dispose()
        }
        XCTAssertEqual(game.observations["initially"], "0")
        XCTAssertEqual(game.observations["flat count"], "1")
        XCTAssertEqual(game.observations["flat identity"], "true")
        XCTAssertEqual(game.observations["flat face"], "PositiveX")
        XCTAssertEqual(game.observations["cube count"], "1")
        XCTAssertEqual(game.observations["cube identity"], "true")
        XCTAssertEqual(game.observations["cube face"], "PositiveY")
        XCTAssertEqual(game.observations["after unbind"], "0")
        XCTAssertEqual(game.observations["after nil cube"], "0")
    }

    /// `GetRenderTargets` returns a copy, so mutating the result binds nothing.
    func testTheReturnedArrayIsACopy() throws {
        try requireNative()
        let game = try run { game, device in
            let flat = try self.target(device)
            try device.SetRenderTarget(flat)
            var copy = device.GetRenderTargets()
            copy.removeAll()
            game.observations["after mutating the copy"] =
                "\(device.GetRenderTargets().count)"
            try device.SetRenderTarget(nil as G.RenderTarget2D?)
            try flat.Dispose()
        }
        XCTAssertEqual(game.observations["after mutating the copy"], "1")
    }

    /// A null array and an **empty** array take the same IL branch, so both
    /// restore the backbuffer.
    func testBothSpellingsOfNothingRestoreTheBackbuffer() throws {
        try requireNative()
        let game = try run { game, device in
            let flat = try self.target(device)
            try device.SetRenderTargets([G.RenderTargetBinding(flat)])
            game.observations["bound"] = "\(device.GetRenderTargets().count)"
            try device.SetRenderTargets(nil)
            game.observations["null"] = "\(device.GetRenderTargets().count)"
            try device.SetRenderTargets([G.RenderTargetBinding(flat)])
            try device.SetRenderTargets([])
            game.observations["empty"] = "\(device.GetRenderTargets().count)"
            try flat.Dispose()
        }
        XCTAssertEqual(game.observations["bound"], "1")
        XCTAssertEqual(game.observations["null"], "0")
        XCTAssertEqual(game.observations["empty"], "0")
    }

    /// More targets than the profile allows is refused, naming the profile and
    /// the **limit** — Reach's `MaxRenderTargets` is 1, so two is the first
    /// count XNA refuses, and the check is managed and never reaches CNA.
    ///
    /// The artifact would have accepted them: `build-probe/f65_rtcube.c` binds
    /// two distinct 8×8 targets successfully and reads both back. This is XNA's
    /// refusal, not the renderer's.
    func testMoreTargetsThanTheProfileAllowsIsRefused() throws {
        try requireNative()
        let game = try run { game, device in
            let first = try self.target(device)
            let second = try self.target(device)
            assertProjected(
                CNANotSupportedException.self,
                message: "XNA Framework Reach profile supports a maximum of 1 "
                    + "simultaneous rendertargets.",
                hResult: CNANotSupportedException.corNotSupportedHResult
            ) {
                try device.SetRenderTargets([
                    G.RenderTargetBinding(first), G.RenderTargetBinding(second),
                ])
            }
            // The refused call bound nothing.
            game.observations["after refusal"] = "\(device.GetRenderTargets().count)"
            game.observations["limit"] = "\(device.profileCapabilities.maxRenderTargets)"
            try first.Dispose()
            try second.Dispose()
        }
        XCTAssertEqual(game.observations["after refusal"], "0")
        XCTAssertEqual(game.observations["limit"], "1")
    }

    /// A disposed target is refused by every binder, on the CLR channel.
    func testADisposedTargetCannotBeBound() throws {
        try requireNative()
        _ = try run { game, device in
            let flat = try self.target(device)
            try flat.Dispose()
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object."
                    + "\r\nObject name: 'RenderTarget2D'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try device.SetRenderTarget(flat) }
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object."
                    + "\r\nObject name: 'RenderTarget2D'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try device.SetRenderTargets([G.RenderTargetBinding(flat)]) }

            let curved = try self.cube(device)
            try curved.Dispose()
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object."
                    + "\r\nObject name: 'RenderTargetCube'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try device.SetRenderTarget(curved, cubeMapFace: .PositiveX) }
            game.observations["checked"] = "yes"
        }
    }

    /// A target created in one callback binds in a later one, which is the
    /// reason the device-identity test compares the runtime and not the facade.
    func testATargetBindsInALaterCallbackThanItWasCreatedIn() throws {
        try requireNative()

        final class TwoCallbackGame: F.Game {
            var manager: F.GraphicsDeviceManager?
            var failure: Error?
            var boundLater = "not reached"
            private var target: G.RenderTarget2D?

            override func LoadContent() throws {
                do {
                    guard let device = try GraphicsDevice else { return }
                    target = try G.RenderTarget2D(
                        graphicsDevice: device, width: 8, height: 8)
                } catch { failure = error }
            }

            override func Update(_ gameTime: F.GameTime) throws {
                do {
                    guard let device = try GraphicsDevice, let target else { return }
                    try device.SetRenderTarget(target)
                    boundLater =
                        "\(device.GetRenderTargets().first?.RenderTarget === target)"
                    try device.SetRenderTarget(nil as G.RenderTarget2D?)
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
        XCTAssertEqual(game.boundLater, "true")
    }

    /// `IsSameSize` compares four things, not two: both extents, the
    /// multisample count and the format's **byte size**.
    ///
    /// Reach's `MaxRenderTargets` is 1, so no device here can reach the
    /// duplicate and same-size tests through `SetRenderTargets` — they are
    /// `i > 0` only. The predicate is therefore exercised where it lives.
    func testTheSameSizeTestComparesFourThings() throws {
        try requireNative()
        let game = try run { game, device in
            let eight = try self.target(device, side: 8)
            let alsoEight = try self.target(device, side: 8)
            let sixteen = try self.target(device, side: 16)
            let cubeEight = try self.cube(device, size: 8)

            typealias GG = Microsoft.Xna.Framework.Graphics
            game.observations["same 2d"] =
                "\(GG.renderTargetsAreSameSize(eight, alsoEight))"
            game.observations["different 2d"] =
                "\(GG.renderTargetsAreSameSize(eight, sixteen))"
            // A cube face and a 2D target of the same extent, format and sample
            // count ARE the same size to this test: it compares the helper's
            // four numbers and never the kind.
            game.observations["cube vs 2d"] =
                "\(GG.renderTargetsAreSameSize(cubeEight, eight))"
            // A texture that is neither reaches XNA's null branch, which then
            // dereferences null. No caller can get there through a binding, and
            // false is the honest answer here.
            let plain = try G.Texture2D(graphicsDevice: device, width: 8, height: 8)
            game.observations["not a target"] =
                "\(GG.renderTargetsAreSameSize(plain, eight))"

            try eight.Dispose()
            try alsoEight.Dispose()
            try sixteen.Dispose()
            try cubeEight.Dispose()
            try plain.Dispose()
        }
        XCTAssertEqual(game.observations["same 2d"], "true")
        XCTAssertEqual(game.observations["different 2d"], "false")
        XCTAssertEqual(game.observations["cube vs 2d"], "true")
        XCTAssertEqual(game.observations["not a target"], "false")
    }

    /// The four-way comparison itself, over the four numbers.
    ///
    /// The test above cannot reach two of them. Only `SurfaceFormat.Color` can
    /// be created on this host and the artifact grants a multisample count of
    /// zero, so no two render targets this binding can construct differ in the
    /// sample count or the pixel size — and a mutation that dropped either
    /// comparison survived until this test existed. The arithmetic is the
    /// claim, so the arithmetic is what is asserted.
    func testTheFourWayShapeComparison() {
        typealias GG = Microsoft.Xna.Framework.Graphics
        let base = (width: Int32(8), height: Int32(8), samples: Int32(0),
                    pixelSize: Int32(4))
        XCTAssertTrue(GG.sameRenderTargetShape(base, base))
        XCTAssertFalse(GG.sameRenderTargetShape(
            base, (width: 16, height: 8, samples: 0, pixelSize: 4)))
        XCTAssertFalse(GG.sameRenderTargetShape(
            base, (width: 8, height: 16, samples: 0, pixelSize: 4)))
        // Same extents, different multisample count.
        XCTAssertFalse(GG.sameRenderTargetShape(
            base, (width: 8, height: 8, samples: 4, pixelSize: 4)))
        // Same extents and sample count, different BIT DEPTH -- which is what
        // the message calls it: "the same size with the same multisample type
        // and bit depth".
        XCTAssertFalse(GG.sameRenderTargetShape(
            base, (width: 8, height: 8, samples: 0, pixelSize: 2)))
        // And two DIFFERENT formats of the same width are the same shape,
        // because the field is the byte size and not the format.
        XCTAssertEqual(GG.expectedByteSize(of: .Color), 4)
        XCTAssertEqual(GG.expectedByteSize(of: .Rgba1010102), 4)
        XCTAssertTrue(GG.sameRenderTargetShape(
            base, (width: 8, height: 8, samples: 0, pixelSize: 4)))
    }

    /// A bound cube target is what `Clear` and `Viewport` measure against, not
    /// the backbuffer — the `currentRenderTargetCount > 0` branch reached from
    /// a cube for the first time.
    func testABoundCubeIsWhatTheViewportIsValidatedAgainst() throws {
        try requireNative()
        let game = try run { game, device in
            let curved = try self.cube(device, size: 4)
            try device.SetRenderTarget(curved, cubeMapFace: .PositiveX)
            let bounds = try device.currentTargetBounds()
            game.observations["bounds"] = "\(bounds.width)x\(bounds.height)"

            // 4x4 is the whole target and is accepted; 8x8 is past it.
            try device.SetViewport(G.Viewport(0, 0, 4, 4))
            game.observations["viewport"] = "\(try device.Viewport.Width)"
            var refused = "accepted"
            do {
                try device.SetViewport(G.Viewport(0, 0, 8, 8))
            } catch { refused = "refused" }
            game.observations["too big"] = refused

            try device.SetRenderTarget(nil as G.RenderTargetCube?, cubeMapFace: .PositiveX)
            let backbuffer = try device.currentTargetBounds()
            game.observations["backbuffer"] = "\(backbuffer.width)x\(backbuffer.height)"
            try curved.Dispose()
        }
        XCTAssertEqual(game.observations["bounds"], "4x4")
        XCTAssertEqual(game.observations["viewport"], "4")
        XCTAssertEqual(game.observations["too big"], "refused")
        XCTAssertEqual(game.observations["backbuffer"], "800x480")
    }
}
