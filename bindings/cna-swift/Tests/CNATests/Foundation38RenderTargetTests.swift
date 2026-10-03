// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias G = Microsoft.Xna.Framework.Graphics

/// A game that runs the render-target slice inside `LoadContent`, where a
/// callback-scoped graphics device exists.
private final class RenderTargetProbeGame: Microsoft.Xna.Framework.Game {
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    var contentLostRaises = 0
    private var body: ((RenderTargetProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (RenderTargetProbeGame, G.GraphicsDevice) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        do {
            guard let device = try GraphicsDevice else {
                throw CNAError.producerInvariant(
                    "the registered graphics device service produced no device")
            }
            try body?(self, device)
        } catch {
            failure = error
        }
        try Exit()
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        try Exit()
    }
}

final class Foundation38RenderTargetTests: XCTestCase {
    private var nativeConfigured: Bool {
        ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] != nil
    }

    private func requireNative() throws {
        if !nativeConfigured {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (RenderTargetProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> RenderTargetProbeGame {
        let game = try RenderTargetProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    // ------------------------------------------------------------------
    // The inheritance itself.
    // ------------------------------------------------------------------

    /// A `RenderTarget2D` **is** a `Texture2D`, a `Texture` and a
    /// `GraphicsResource`, and there is exactly one native handle underneath.
    func testARenderTargetIsATexture2DAndAGraphicsResource() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 64, height: 32)
            game.observations["isTexture2D"] = String((target as Any) is G.Texture2D)
            game.observations["isTexture"] = String((target as Any) is G.Texture)
            game.observations["isGraphicsResource"] =
                String((target as Any) is G.GraphicsResource)
            game.observations["width"] = String(target.Width)
            game.observations["height"] = String(target.Height)
            game.observations["levelCount"] = String(target.LevelCount)
            game.observations["format"] = String(target.Format.rawValue)
            game.observations["usage"] = String(target.RenderTargetUsage.rawValue)
            game.observations["multiSample"] = String(target.MultiSampleCount)
            game.observations["depth"] = String(target.DepthStencilFormat.rawValue)
            game.observations["contentLost"] = String(target.IsContentLost)
            game.observations["isDisposed"] = String(target.IsDisposed)
            game.observations["deviceIsSame"] =
                String(target.GraphicsDevice === device)
            try target.Dispose()
            game.observations["disposedAfter"] = String(target.IsDisposed)
        }
        for key in ["isTexture2D", "isTexture", "isGraphicsResource", "deviceIsSame"] {
            XCTAssertEqual(game.observations[key], "true", key)
        }
        XCTAssertEqual(game.observations["width"], "64")
        XCTAssertEqual(game.observations["height"], "32")
        XCTAssertEqual(game.observations["levelCount"], "1")
        XCTAssertEqual(game.observations["format"], "0", "SurfaceFormat.Color")
        XCTAssertEqual(
            game.observations["usage"], "0", "RenderTargetUsage.DiscardContents")
        XCTAssertEqual(game.observations["multiSample"], "0")
        XCTAssertEqual(
            game.observations["depth"], "0", "DepthFormat.None is the 3-argument default")
        XCTAssertEqual(
            game.observations["contentLost"], "false",
            "HEADLESS cannot lose a device, so a fresh target never reports lost content")
        XCTAssertEqual(game.observations["isDisposed"], "false")
        XCTAssertEqual(game.observations["disposedAfter"], "true")
    }

    /// The base's members answer through the derived object, and the derived
    /// object's dimensions are the ones the target actually got.
    func testTheBaseSurfaceAnswersThroughTheDerivedObject() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 48, height: 24)
            // Held as the BASE type on purpose: this is the substitutability
            // that a final Texture2D made impossible.
            let asTexture: G.Texture2D = target
            game.observations["baseWidth"] = String(asTexture.Width)
            game.observations["baseHeight"] = String(asTexture.Height)
            game.observations["baseBounds"] =
                "\(asTexture.Bounds.X),\(asTexture.Bounds.Y),"
                + "\(asTexture.Bounds.Width),\(asTexture.Bounds.Height)"
            game.observations["baseLevelCount"] = String(asTexture.LevelCount)
            asTexture.Name = "offscreen"
            game.observations["name"] = asTexture.Name ?? "<nil>"
            game.observations["toString"] = asTexture.ToString()
            asTexture.Name = nil
            game.observations["unnamedToString"] = asTexture.ToString()
            try asTexture.Dispose()
        }
        XCTAssertEqual(game.observations["baseWidth"], "48")
        XCTAssertEqual(game.observations["baseHeight"], "24")
        XCTAssertEqual(game.observations["baseBounds"], "0,0,48,24")
        XCTAssertEqual(game.observations["name"], "offscreen")
        XCTAssertEqual(game.observations["toString"], "offscreen")
        // With no name, GraphicsResource.ToString falls through to
        // Object.ToString(), which is the CLR full type name.
        XCTAssertEqual(
            game.observations["unnamedToString"],
            "Microsoft.Xna.Framework.Graphics.RenderTarget2D")
    }

    // ------------------------------------------------------------------
    // Ownership: one handle, one owner, one destruction path.
    // ------------------------------------------------------------------

    /// Disposing through the DERIVED API releases the one shared handle, and
    /// every base member then reports the resource as disposed rather than
    /// reaching a dead native object.
    func testDisposeThroughTheDerivedApiClosesTheOneSharedHandle() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 16, height: 16)
            try target.Dispose()
            game.observations["disposed"] = String(target.IsDisposed)
            // A duplicate Dispose is a no-op, exactly as XNA's
            // `~GraphicsResource()` returns immediately when isDisposed.
            try target.Dispose()
            try target.Dispose()
            game.observations["stillDisposed"] = String(target.IsDisposed)
            // A base-typed reference sees the same disposed object.
            let asTexture: G.Texture2D = target
            game.observations["baseSeesDisposed"] = String(asTexture.IsDisposed)
            do {
                _ = try asTexture.validatedHandle("probe")
                game.observations["baseUseAfterDispose"] = "succeeded"
            } catch let error as CNAObjectDisposedException {
                // Foundation 42 moved this failure off the CNA runtime channel
                // onto the projected class XNA actually raises, so what is
                // asserted is the payload rather than a Swift description.
                game.observations["baseUseAfterDispose"] = error.ObjectName
                game.observations["baseUseAfterDisposeMessage"] = error.Message
                game.observations["baseUseAfterDisposeClass"] =
                    String(describing: type(of: error))
            } catch {
                game.observations["baseUseAfterDispose"] = "\(error)"
            }
        }
        XCTAssertEqual(game.observations["disposed"], "true")
        XCTAssertEqual(game.observations["stillDisposed"], "true")
        XCTAssertEqual(game.observations["baseSeesDisposed"], "true")
        XCTAssertEqual(
            game.observations["baseUseAfterDispose"],
            "RenderTarget2D",
            "the base must report the DERIVED type's name; one storage, one name")
        XCTAssertEqual(
            game.observations["baseUseAfterDisposeClass"],
            "CNAObjectDisposedException")
        XCTAssertEqual(
            game.observations["baseUseAfterDisposeMessage"],
            "Cannot access a disposed object.\r\nObject name: 'RenderTarget2D'.")
    }

    /// `Disposing` fires once, with the resource as sender, and **after** the
    /// native release — which is the order `~GraphicsResource()` uses.
    func testDisposingFiresOnceAfterTheNativeRelease() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 16, height: 16)
            var raises = 0
            var senderWasTarget = false
            var disposedWhenRaised = false
            _ = target.Disposing.Add { sender, _ in
                raises += 1
                senderWasTarget = (sender as AnyObject) === target
                disposedWhenRaised = target.IsDisposed
            }
            try target.Dispose()
            try target.Dispose()
            game.observations["raises"] = String(raises)
            game.observations["sender"] = String(senderWasTarget)
            game.observations["disposedWhenRaised"] = String(disposedWhenRaised)
        }
        XCTAssertEqual(game.observations["raises"], "1")
        XCTAssertEqual(game.observations["sender"], "true")
        XCTAssertEqual(
            game.observations["disposedWhenRaised"], "true",
            "XNA releases the resource and only then raises Disposing")
    }

    /// Disposal releases the native ContentLost subscription **before** the
    /// handle, so no native callback can address a released Swift box.
    func testDisposalReleasesTheNativeContentLostSubscription() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 16, height: 16)
            game.observations["subscribed"] =
                String(target.contentLostRegistration != 0)
            try target.Dispose()
            game.observations["releasedAfterDispose"] =
                String(target.contentLostRegistration == 0)
        }
        XCTAssertEqual(
            game.observations["subscribed"], "true",
            "the target must hold a real CNA ContentLost registration")
        XCTAssertEqual(game.observations["releasedAfterDispose"], "true")
    }

    /// The same, through a resource that does **not** override
    /// `Dispose(Boolean)`.
    ///
    /// `RenderTarget2D` carries its own guard because it has a subscription to
    /// release first, so it short-circuits before the base body ever runs.
    /// `SpriteBatch` does not override, so this is the only test that reaches
    /// `GraphicsResource.Dispose(Boolean)`'s own already-disposed guard — the
    /// one `~GraphicsResource()` opens with.
    func testTheBaseGuardMakesDisposeIdempotentWithoutAnOverride() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            var raises = 0
            _ = batch.Disposing.Add { _, _ in raises += 1 }
            try batch.Dispose()
            try batch.Dispose()
            try batch.Dispose(true)
            game.observations["raises"] = String(raises)
            game.observations["disposed"] = String(batch.IsDisposed)
        }
        XCTAssertEqual(
            game.observations["raises"], "1",
            "the base guard must stop a second Dispose raising Disposing again")
        XCTAssertEqual(game.observations["disposed"], "true")
    }

    // ------------------------------------------------------------------
    // Consumption: the render target used where a texture is expected.
    // ------------------------------------------------------------------

    func testARenderTargetIsAcceptedWhereSpriteBatchExpectsATexture() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 32, height: 32)
            let batch = try G.SpriteBatch(graphicsDevice: device)
            try batch.Begin()
            try batch.Draw(
                target,
                position: Microsoft.Xna.Framework.Vector2(1, 2),
                color: .White)
            try batch.End()
            game.observations["submitted"] = "true"
            game.observations["batchIsGraphicsResource"] =
                String((batch as Any) is G.GraphicsResource)
            try batch.Dispose()
            game.observations["batchDisposed"] = String(batch.IsDisposed)
            try target.Dispose()
        }
        XCTAssertEqual(game.observations["submitted"], "true")
        XCTAssertEqual(
            game.observations["batchIsGraphicsResource"], "true",
            "SpriteBatch gained the same base, which is why it can be disposed "
            + "through it")
        XCTAssertEqual(game.observations["batchDisposed"], "true")
    }

    func testBindingAndRestoringTheBackbuffer() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 32, height: 32)
            try device.SetRenderTarget(target)
            game.observations["bound"] = "true"
            // XNA restores the backbuffer with a null argument; the IL takes a
            // distinct branch for it, so the Swift parameter is Optional.
            try device.SetRenderTarget(nil)
            game.observations["restored"] = "true"
            try target.Dispose()
        }
        XCTAssertEqual(game.observations["bound"], "true")
        XCTAssertEqual(game.observations["restored"], "true")
    }

    /// CNA refuses to destroy a bound target, and the refusal reaches the
    /// caller as a native failure rather than being swallowed.
    func testDisposingABoundTargetIsRefusedByTheHost() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 32, height: 32)
            try device.SetRenderTarget(target)
            do {
                try target.Dispose()
                game.observations["boundDispose"] = "succeeded"
            } catch let error as CNAError {
                game.observations["boundDispose"] = "refused"
                game.observations["boundDisposeIsNative"] = "true"
                _ = error
            }
            game.observations["stillLive"] = String(!target.IsDisposed)
            try device.SetRenderTarget(nil)
            try target.Dispose()
            game.observations["disposedAfterUnbind"] = String(target.IsDisposed)
        }
        XCTAssertEqual(game.observations["boundDispose"], "refused")
        XCTAssertEqual(
            game.observations["boundDisposeIsNative"], "true",
            "a host refusal is a CNAError and never a projected CLR exception")
        XCTAssertEqual(game.observations["stillLive"], "true")
        XCTAssertEqual(game.observations["disposedAfterUnbind"], "true")
    }

    // ------------------------------------------------------------------
    // The requested/granted distinction.
    // ------------------------------------------------------------------

    /// Every property is read back from the target CNA actually made, never
    /// echoed from the request — which is what the "preferred" in the XNA
    /// parameter names means.
    func testThePropertiesAreReadBackFromTheGrantedTarget() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 40, height: 20,
                mipMap: false, preferredFormat: .Color,
                preferredDepthFormat: .Depth24Stencil8,
                preferredMultiSampleCount: 0, usage: .PreserveContents)
            game.observations["usage"] = String(target.RenderTargetUsage.rawValue)
            game.observations["depth"] = String(target.DepthStencilFormat.rawValue)
            game.observations["format"] = String(target.Format.rawValue)
            game.observations["multiSample"] = String(target.MultiSampleCount)
            game.observations["levelCount"] = String(target.LevelCount)
            try target.Dispose()
        }
        // PreserveContents is 1 and is honoured; the depth format the host
        // grants is recorded rather than asserted to be the requested one.
        XCTAssertEqual(game.observations["usage"], "1")
        XCTAssertEqual(game.observations["format"], "0", "SurfaceFormat.Color")
        XCTAssertEqual(game.observations["multiSample"], "0")
        XCTAssertEqual(game.observations["levelCount"], "1")
        XCTAssertNotNil(game.observations["depth"])
    }

    /// The parent game's disposal releases the target with everything else,
    /// and disposing it again afterwards is still a no-op rather than a
    /// double free.
    /// `GraphicsDevice.Present` refuses while a render target is bound.
    ///
    /// XNA raises `InvalidOperationException(CannotPresentActiveRenderTargets)`
    /// and CNA makes no such check, so the refusal is the binding's own. It
    /// matters: presenting here would show the backbuffer the caller was NOT
    /// drawing into, which looks like a dropped frame rather than a mistake.
    func testPresentRefusesWhileARenderTargetIsBound() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try G.RenderTarget2D(
                graphicsDevice: device, width: 16, height: 16)
            try device.SetRenderTarget(target)
            do {
                try device.Present()
                game.observations["refused"] = "false"
            } catch let error as CNAInvalidOperationException {
                game.observations["refused"] = "true"
                game.observations["message"] = error.Message ?? ""
            }
            // Unbound, the same call is accepted again: the refusal is about
            // the binding, not about Present.
            try device.SetRenderTarget(nil)
            do {
                try device.Present()
                game.observations["afterUnbind"] = "accepted"
            } catch {
                game.observations["afterUnbind"] = "\(error)"
            }
        }
        XCTAssertEqual(game.observations["refused"], "true")
        XCTAssertEqual(game.observations["message"],
                       "Cannot call Present when a render target is active.")
        XCTAssertEqual(game.observations["afterUnbind"], "accepted",
                       "unbinding restores what the refusal was protecting")
    }

    func testParentGameDisposalReleasesTheTarget() throws {
        try requireNative()
        var escaped: G.RenderTarget2D?
        let game = try RenderTargetProbeGame { game, device in
            escaped = try G.RenderTarget2D(
                graphicsDevice: device, width: 16, height: 16)
            game.observations["created"] = "true"
        }
        // The `defer` is only the net for a throwing `Run()`. Here the
        // explicit `Dispose()` stays where it was, BEFORE the assertions,
        // because this test's whole subject is what the parent's disposal does
        // to the child -- moving it after them asserts against a game that has
        // not been disposed yet, which is exactly what the first draft of this
        // repair did and what caught it.
        defer { try? game.Dispose() }
        try game.Run()
        try game.Dispose()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.observations["created"], "true")
        XCTAssertNotNil(escaped)
        XCTAssertTrue(escaped?.IsDisposed ?? false,
                      "the parent Game's disposal must have released the child")
        XCTAssertNoThrow(try escaped?.Dispose())
    }
}
