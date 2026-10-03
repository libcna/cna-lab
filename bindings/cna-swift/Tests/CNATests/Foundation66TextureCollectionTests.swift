// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class SlotProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((SlotProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (SlotProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 66: `TextureCollection`, and the three checks that were waiting
/// for something to be bound.
///
/// `Textures` and `VertexTextures` were the last blocker in front of `Effect`,
/// and they make three recorded absences reachable at once:
/// `MustResolveRenderTarget` in `TextureCollection.SetItem` and in the transfer
/// paths of `Texture2D` and `TextureCube`, and `ResourceInUse` in the transfer
/// paths of all three texture types.
final class Foundation66TextureCollectionTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (SlotProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> SlotProbeGame {
        let game = try SlotProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    private func outOfRange(_ name: String = "index") -> String {
        composedArgumentMessage(
            CNAArgumentOutOfRangeException.argArgumentOutOfRangeMessage,
            paramName: name)
    }

    // ------------------------------------------------------------------
    // The collections themselves.

    /// The device hands out two distinct collections, and each is one object
    /// across callbacks.
    func testTheDeviceHasTwoDistinctCollections() throws {
        try requireNative()
        let game = try run { game, device in
            guard let pixel = device.Textures,
                  let vertex = device.VertexTextures else {
                throw CNAError.producerInvariant("no texture collection")
            }
            game.observations["distinct"] = "\(pixel !== vertex)"
            game.observations["stable"] = "\(device.Textures === pixel)"
            game.observations["pixel slots"] = "\(pixel.count)"
            game.observations["vertex slots"] = "\(vertex.count)"
            game.observations["max samplers"] =
                "\(device.profileCapabilities.maxSamplers)"
            game.observations["max vertex samplers"] =
                "\(device.profileCapabilities.maxVertexSamplers)"
        }
        XCTAssertEqual(game.observations["distinct"], "true")
        XCTAssertEqual(game.observations["stable"], "true")
        // Sized from the device's own profile, not from CNA's wider limit.
        XCTAssertEqual(game.observations["pixel slots"], "16")
        XCTAssertEqual(game.observations["vertex slots"], "0")
        XCTAssertEqual(game.observations["max samplers"], "16")
        XCTAssertEqual(game.observations["max vertex samplers"], "0")
    }

    /// **Reach has no vertex samplers**, so its vertex collection has no slots
    /// and every index is out of range — including zero.
    ///
    /// CNA would have taken it: `cna_graphics_device_set_texture` accepts slots
    /// 0 through 15 on the vertex stage too (`build-probe/f66_slots.c`). The
    /// profile is the narrower rule and is the one that decides.
    ///
    /// **The reader and the writer report different things**, and that is
    /// XNA's order rather than an inconsistency: `set_Item` checks the *value*
    /// before the *index*, and Reach's `ValidVertexTextureFormats` is empty, so
    /// every texture fails the vertex-format rule before its slot number is
    /// ever looked at. `get_Item` has no value to check and reports the index.
    /// This test asserted the index for both until the code said otherwise.
    func testTheVertexCollectionHasNoSlotsOnReach() throws {
        try requireNative()
        _ = try run { game, device in
            guard let vertex = device.VertexTextures else {
                throw CNAError.producerInvariant("no vertex collection")
            }
            let texture = try G.Texture2D(graphicsDevice: device, width: 4, height: 4)
            for index in [Int32(0), 1, 15] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: self.outOfRange(), paramName: "index",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) { _ = try vertex.Item(index) }
                assertProjected(
                    CNANotSupportedException.self,
                    message: "XNA Framework Reach profile does not support "
                        + "vertex texture format Color.",
                    hResult: CNANotSupportedException.corNotSupportedHResult
                ) { try vertex.SetItem(index, texture) }
            }
            // A NIL value skips every value test, so the index is reported.
            for index in [Int32(0), 1, 15] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: self.outOfRange(), paramName: "index",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) { try vertex.SetItem(index, nil) }
            }
            try texture.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// The vertex SAMPLER collection is sized from the same table, which is the
    /// divergence Foundation 66 resolved: it was 16 slots long because the
    /// binding had no profile when it was written, and Foundation 62 gave it
    /// one.
    func testTheVertexSamplerCollectionIsAlsoProfileSized() throws {
        try requireNative()
        let game = try run { game, device in
            game.observations["pixel"] = "\(device.SamplerStates?.count ?? -1)"
            game.observations["vertex"] = "\(device.VertexSamplerStates?.count ?? -1)"
            assertProjected(
                CNAArgumentOutOfRangeException.self,
                message: self.outOfRange(), paramName: "index",
                hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
            ) { _ = try device.VertexSamplerStates?.Item(0) }
        }
        XCTAssertEqual(game.observations["pixel"], "16")
        XCTAssertEqual(game.observations["vertex"], "0")
    }

    // ------------------------------------------------------------------
    // Binding.

    /// What is bound is what comes back, by identity, and an unwritten slot is
    /// nil.
    func testWhatIsBoundComesBackByIdentity() throws {
        try requireNative()
        let game = try run { game, device in
            guard let textures = device.Textures else {
                throw CNAError.producerInvariant("no collection")
            }
            let first = try G.Texture2D(graphicsDevice: device, width: 4, height: 4)
            game.observations["initially"] = "\(try textures.Item(0) == nil)"
            try textures.SetItem(0, first)
            game.observations["identity"] = "\(try textures.Item(0) === first)"
            game.observations["neighbour"] = "\(try textures.Item(1) == nil)"

            // A cube binds through the same slot API.
            let cube = try G.TextureCube(graphicsDevice: device, size: 4,
                                         mipMap: false, format: .Color)
            try textures.SetItem(1, cube)
            game.observations["cube"] = "\(try textures.Item(1) === cube)"

            try textures.SetItem(0, nil)
            game.observations["unbound"] = "\(try textures.Item(0) == nil)"
            try first.Dispose()
            try cube.Dispose()
        }
        XCTAssertEqual(game.observations["initially"], "true")
        XCTAssertEqual(game.observations["identity"], "true")
        XCTAssertEqual(game.observations["neighbour"], "true")
        XCTAssertEqual(game.observations["cube"], "true")
        XCTAssertEqual(game.observations["unbound"], "true")
    }

    /// Slot 16 and a negative index are refused, naming `index`.
    func testTheBoundsAreTheCollectionsOwn() throws {
        try requireNative()
        _ = try run { game, device in
            guard let textures = device.Textures else {
                throw CNAError.producerInvariant("no collection")
            }
            let texture = try G.Texture2D(graphicsDevice: device, width: 4, height: 4)
            // 15 is the last accepted slot.
            try textures.SetItem(15, texture)
            for bad in [Int32(-1), 16, 99] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: self.outOfRange(), paramName: "index",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) { _ = try textures.Item(bad) }
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: self.outOfRange(), paramName: "index",
                    hResult: CNAArgumentOutOfRangeException.corArgumentOutOfRangeHResult
                ) { try textures.SetItem(bad, texture) }
            }
            try textures.SetItem(15, nil)
            try texture.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// A disposed texture cannot be bound, and a texture disposed while bound
    /// leaves the collection.
    ///
    /// The second half is a native fact reproduced rather than a managed
    /// invention: `cna_texture2d_destroy` on a bound texture succeeds and the
    /// slot reads back empty (`build-probe/f66_slots.c`), so a managed cache
    /// that kept naming it would be answering with an object the device no
    /// longer holds.
    func testDisposalAndTheCollection() throws {
        try requireNative()
        let game = try run { game, device in
            guard let textures = device.Textures else {
                throw CNAError.producerInvariant("no collection")
            }
            let disposed = try G.Texture2D(graphicsDevice: device, width: 4, height: 4)
            try disposed.Dispose()
            assertProjected(
                CNAObjectDisposedException.self,
                message: "Cannot access a disposed object."
                    + "\r\nObject name: 'Texture2D'.",
                hResult: CNAObjectDisposedException.corObjectDisposedHResult
            ) { try textures.SetItem(0, disposed) }

            let bound = try G.Texture2D(graphicsDevice: device, width: 4, height: 4)
            try textures.SetItem(2, bound)
            game.observations["bound"] = "\(try textures.Item(2) === bound)"
            game.observations["held"] = "\(textures.holds(bound))"
            try bound.Dispose()
            // `holds` is read BEFORE `Item`, and the order is the whole point.
            // `Item` asks the device, finds the slot empty -- CNA unbinds a
            // destroyed texture itself -- and corrects the cache on the way
            // past, so reading it first would hide whether disposal did
            // anything. A caller who never reads that slot gets no such
            // correction, and the collection would go on holding a strong
            // reference to a dead texture. That is the claim, and
            // `disposed-texture-stays-in-the-collection` survived twice before
            // this line was in the right place.
            game.observations["still held"] = "\(textures.holds(bound))"
            game.observations["after disposal"] = "\(try textures.Item(2) == nil)"
        }
        XCTAssertEqual(game.observations["bound"], "true")
        XCTAssertEqual(game.observations["held"], "true")
        XCTAssertEqual(game.observations["after disposal"], "true")
        XCTAssertEqual(game.observations["still held"], "false")
    }

    // ------------------------------------------------------------------
    // The three checks this milestone made reachable.

    /// A render target that is currently the device's target cannot be bound as
    /// a texture, and the value is checked **before** the index.
    ///
    /// CNA enforces the same rule and says so — `cna_graphics_device_set_texture`
    /// on an active target answers `INVALID_STATE` with *"A texture that is
    /// currently bound as a render target cannot be bound for sampling"* — so
    /// the managed check running first is what makes the message XNA's.
    func testAnActiveRenderTargetCannotBeSampled() throws {
        try requireNative()
        let game = try run { game, device in
            guard let textures = device.Textures else {
                throw CNAError.producerInvariant("no collection")
            }
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 8, height: 8)

            // Not bound as a target yet: binding it as a texture is fine.
            try textures.SetItem(3, target)
            game.observations["before"] = "\(try textures.Item(3) === target)"
            try textures.SetItem(3, nil)

            try device.SetRenderTarget(target)
            let message = "The render target must not be set on the device "
                + "when it is used as a texture."
            assertProjected(
                CNAInvalidOperationException.self, message: message,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try textures.SetItem(3, target) }

            // The VALUE is tested before the INDEX: `IL_0088` is the first
            // bounds branch and both value tests precede it. A call that is
            // wrong in both ways reports the render target.
            assertProjected(
                CNAInvalidOperationException.self, message: message,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try textures.SetItem(-1, target) }

            try device.SetRenderTarget(nil as G.RenderTarget2D?)
            // Unbound again, and accepted again.
            try textures.SetItem(3, target)
            game.observations["after"] = "\(try textures.Item(3) === target)"
            try textures.SetItem(3, nil)
            try target.Dispose()
        }
        XCTAssertEqual(game.observations["before"], "true")
        XCTAssertEqual(game.observations["after"], "true")
    }

    /// `SetData` on a texture that is bound to a sampler raises `ResourceInUse`
    /// — `E_ABORT` through `Helpers.GetExceptionFromResult`, which maps it to an
    /// ordinary `InvalidOperationException` with a message about `SetData`.
    ///
    /// `GetData` does **not** scan: the test is `if (isSetting)`.
    func testSetDataOnABoundTextureIsRefused() throws {
        try requireNative()
        let game = try run { game, device in
            guard let textures = device.Textures else {
                throw CNAError.producerInvariant("no collection")
            }
            let texture = try G.Texture2D(graphicsDevice: device, width: 2, height: 2)
            let pixels = [F.Color](repeating: F.Color(Int32(1), Int32(2), Int32(3), Int32(4)),
                                   count: 4)
            // Unbound: accepted.
            try texture.SetData(pixels)

            try textures.SetItem(4, texture)
            assertProjected(
                CNAInvalidOperationException.self,
                message: "You may not call SetData on a resource while it is "
                    + "actively set on the GraphicsDevice. Unset it from the "
                    + "device before calling SetData.",
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try texture.SetData(pixels) }

            // GetData is not scanned -- the test is `if (isSetting)`.
            var read = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)),
                                 count: 4)
            try texture.GetData(&read)
            game.observations["read while bound"] = "\(read[0].R),\(read[0].G)"

            try textures.SetItem(4, nil)
            // Unbound again: accepted again.
            try texture.SetData(pixels)
            game.observations["after unbind"] = "accepted"
            try texture.Dispose()
        }
        XCTAssertEqual(game.observations["read while bound"], "1,2")
        XCTAssertEqual(game.observations["after unbind"], "accepted")
    }

    /// `SetData` on a texture that is the device's render target raises
    /// `MustResolveRenderTarget`, and it is checked **before** the sampler scan
    /// and before every argument test.
    func testSetDataOnAnActiveRenderTargetIsRefused() throws {
        try requireNative()
        _ = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 2, height: 2)
            let pixels = [F.Color](repeating: F.Color(Int32(9), Int32(9), Int32(9), Int32(9)),
                                   count: 4)
            try target.SetData(pixels)

            try device.SetRenderTarget(target)
            let message = "The render target must not be set on the device "
                + "when it is used as a texture."
            assertProjected(
                CNAInvalidOperationException.self, message: message,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try target.SetData(pixels) }

            // But NOT ahead of the array tests: `data == null ||
            // data.Length == 0` is `IL_0015`, the render-target test is
            // `IL_0022`, so an empty array is still reported as a null one.
            // This test asserted the opposite until the code said otherwise.
            assertProjected(
                CNAArgumentNullException.self,
                message: composedArgumentMessage(
                    "This method does not accept null for this parameter.",
                    paramName: "data"),
                paramName: "data",
                hResult: CNAArgumentNullException.argumentNullHResult
            ) { try target.SetData([F.Color]()) }

            // And GETTING is refused as well -- unlike the sampler scan, this
            // test is not guarded by `isSetting`.
            var read = [F.Color](repeating: F.Color(Int32(0), Int32(0), Int32(0), Int32(0)),
                                 count: 4)
            assertProjected(
                CNAInvalidOperationException.self, message: message,
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try target.GetData(&read) }

            try device.SetRenderTarget(nil as G.RenderTarget2D?)
            try target.SetData(pixels)
            try target.Dispose()
            game.observations["checked"] = "yes"
        }
    }

    /// A `TextureCube`'s transfer is scanned too — the same two checks in the
    /// same order, whether or not the renderer stores cube data at all.
    ///
    /// That is the point: the managed refusal must come **first**, so the
    /// caller sees XNA's `ResourceInUse` rather than whatever the renderer
    /// answers. HEADLESS refuses the unbound upload with `NOT_SUPPORTED`, so
    /// without the scan the bound call would report that native failure on a
    /// different channel; OPENGLES3 stores it, so without the scan the bound
    /// call would silently succeed.
    func testACubesTransferIsScannedBeforeTheRendererRefusesIt() throws {
        try requireNative()
        let game = try run { game, device in
            guard let textures = device.Textures else {
                throw CNAError.producerInvariant("no collection")
            }
            let cube = try G.TextureCube(graphicsDevice: device, size: 4,
                                         mipMap: false, format: .Color)
            let pixels = [F.Color](repeating: F.Color(Int32(1), Int32(1), Int32(1), Int32(1)),
                                   count: 16)
            // Unbound, the renderer either stores it or refuses it on the
            // runtime channel.
            var unbound = "no error"
            do {
                try cube.SetData(.PositiveX, data: pixels)
            } catch let error as CNAError {
                if case .nativeFailure(let operation, let result, _) = error {
                    unbound = "\(operation)=\(result)"
                }
            }
            game.observations["unbound"] = unbound

            // Bound, the MANAGED refusal comes first.
            try textures.SetItem(5, cube)
            assertProjected(
                CNAInvalidOperationException.self,
                message: "You may not call SetData on a resource while it is "
                    + "actively set on the GraphicsDevice. Unset it from the "
                    + "device before calling SetData.",
                hResult: CNAInvalidOperationException.corInvalidOperationHResult
            ) { try cube.SetData(.PositiveX, data: pixels) }
            try textures.SetItem(5, nil)
            try cube.Dispose()
        }
        XCTAssertTrue(
            ["no error", "cna_texturecube_set_data=6"].contains(game.observations["unbound"]),
            "got \(game.observations["unbound"] ?? "<none>")")
    }

    /// A texture bound in one callback is still bound in the next, which is
    /// what makes the collection live on the runtime rather than the facade.
    func testTheCollectionSurvivesTheCallbackBoundary() throws {
        try requireNative()

        final class TwoCallbackGame: F.Game {
            var manager: F.GraphicsDeviceManager?
            var failure: Error?
            var stillBound = "not reached"
            private var texture: G.Texture2D?

            override func LoadContent() throws {
                do {
                    guard let device = try GraphicsDevice,
                          let textures = device.Textures else { return }
                    let created = try G.Texture2D(
                        graphicsDevice: device, width: 4, height: 4)
                    texture = created
                    try textures.SetItem(6, created)
                } catch { failure = error }
            }

            override func Update(_ gameTime: F.GameTime) throws {
                do {
                    guard let device = try GraphicsDevice,
                          let textures = device.Textures, let texture else { return }
                    stillBound = "\(try textures.Item(6) === texture)"
                    try textures.SetItem(6, nil)
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
        XCTAssertEqual(game.stillBound, "true")
    }
}
