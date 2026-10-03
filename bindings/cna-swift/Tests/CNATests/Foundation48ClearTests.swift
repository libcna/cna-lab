// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias G = Microsoft.Xna.Framework.Graphics

/// A game that runs the clear slice inside `LoadContent`, where a
/// callback-scoped graphics device exists.
private final class ClearProbeGame: Microsoft.Xna.Framework.Game {
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((ClearProbeGame, G.GraphicsDevice) throws -> Void)?

    init(hiDef: Bool = false,
         _ body: @escaping (ClearProbeGame, G.GraphicsDevice) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
        if hiDef { manager?.GraphicsProfile = .HiDef }
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

/// Foundation 48: `GraphicsDevice.Clear`, its two missing overloads, and the
/// `DefaultClearOptions` rule the colour-only overload forwards.
final class Foundation48ClearTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        hiDef: Bool = false,
        _ body: @escaping (ClearProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> ClearProbeGame {
        let game = try ClearProbeGame(hiDef: hiDef, body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    func testEveryBackBufferReadbackOverloadPreservesReachRefusal() throws {
        try requireNative()
        _ = try run { _, device in
            var pixels = [Microsoft.Xna.Framework.Color](
                repeating: .Transparent, count: 4)
            let calls: [() throws -> Void] = [
                { try device.GetBackBufferData(&pixels) },
                { try device.GetBackBufferData(
                    &pixels, startIndex: 0, elementCount: 4) },
                { try device.GetBackBufferData(
                    Microsoft.Xna.Framework.Rectangle(0, 0, 2, 2),
                    data: &pixels, startIndex: 0, elementCount: 4) },
            ]
            for call in calls {
                XCTAssertThrowsError(try call()) { error in
                    XCTAssertEqual(
                        (error as? CNANotSupportedException)?.Message,
                        "XNA Framework Reach profile does not support GetBackBufferData.")
                }
            }
        }
    }

    /// Under HiDef the managed checks pass and the read reaches the renderer,
    /// which either returns what the clear wrote or refuses on the runtime
    /// channel -- HEADLESS has no readback (`CNA_RESULT_NOT_SUPPORTED`),
    /// OPENGLES3 does. Both directions are asserted exactly.
    func testHiDefBackBufferReadbackReturnsTheClearOrTheRenderersRefusal() throws {
        try requireNative()
        let game = try run(hiDef: true) { game, device in
            try device.Clear(.Red)
            var pixels = [Microsoft.Xna.Framework.Color](repeating: .Transparent, count: 4)
            do {
                try device.GetBackBufferData(
                    Microsoft.Xna.Framework.Rectangle(0, 0, 2, 2),
                    data: &pixels, startIndex: 0, elementCount: 4)
                game.observations["read"] = pixels
                    .map { String($0.PackedValue, radix: 16) }.joined(separator: ",")
            } catch let error as CNAError {
                if case .nativeFailure(let operation, let result, _) = error {
                    game.observations["read"] = "\(operation)=\(result)"
                }
            }
        }
        print("CNA_BACKBUFFER_READ=\(game.observations["read"] ?? "<none>")")
        let red = String(Microsoft.Xna.Framework.Color.Red.PackedValue, radix: 16)
        XCTAssertTrue(
            [Array(repeating: red, count: 4).joined(separator: ","),
             "cna_graphics_device_get_backbuffer_data_window=6"]
                .contains(game.observations["read"]),
            "got \(game.observations["read"] ?? "<none>")")
    }

    func testReachRefusesBackBufferReadbackBeforeArrayValidation() throws {
        try requireNative()
        _ = try run { _, device in
            var empty: [Microsoft.Xna.Framework.Color] = []
            XCTAssertThrowsError(try device.GetBackBufferData(&empty)) { error in
                XCTAssertEqual(
                    (error as? CNANotSupportedException)?.Message,
                    "XNA Framework Reach profile does not support GetBackBufferData.")
            }
        }
    }

    private func target(
        _ device: G.GraphicsDevice, _ depth: G.DepthFormat
    ) throws -> G.RenderTarget2D {
        try G.RenderTarget2D(
            graphicsDevice: device, width: 64, height: 64, mipMap: false,
            preferredFormat: .Color, preferredDepthFormat: depth,
            preferredMultiSampleCount: 0, usage: .DiscardContents)
    }

    /// The XNA rule, written once, so the expectations below are derived from
    /// `get_DefaultClearOptions` rather than transcribed four times.
    private func expected(for format: G.DepthFormat) -> G.ClearOptions {
        if format == .None { return .Target }
        if format == .Depth24Stencil8 { return [.Target, .DepthBuffer, .Stencil] }
        return [.Target, .DepthBuffer]
    }

    // ------------------------------------------------------------------
    // get_DefaultClearOptions, branch by branch.
    // ------------------------------------------------------------------

    /// `currentRenderTargetCount == 0` reads the presentation parameters.
    ///
    /// A device created through `GraphicsDeviceManager` — the only way a
    /// projected game gets one — reports `DepthFormat.Depth24`, so the
    /// backbuffer's `DefaultClearOptions` is `Target|DepthBuffer` and a
    /// colour-only `Clear` clears the depth buffer too. This is the number
    /// that makes Foundation 48 a repair rather than an addition: the
    /// projection used to call CNA's colour-only route here and drop the
    /// depth clear on every frame of every game.
    ///
    /// `build-probe/f48c_manager.c` measures the same value one layer down,
    /// and `build-probe/f48b_params.c` measures 0 for a game that never
    /// creates a manager — so the format is the manager's doing, not the
    /// renderer's.
    func testBackbufferDefaultClearOptionsComeFromThePresentationParameters() throws {
        try requireNative()
        let game = try run { game, device in
            let native = try device.nativePresentationParameters()
            game.observations["depthStencilFormat"] = String(native.depth_stencil_format)
            game.observations["options"] = String(try device.defaultClearOptions.rawValue)
        }
        XCTAssertEqual(
            game.observations["depthStencilFormat"],
            String(G.DepthFormat.Depth24.rawValue))
        XCTAssertEqual(
            game.observations["options"],
            String(G.ClearOptions([.Target, .DepthBuffer]).rawValue))
    }

    /// A bound render target answers with **its** depth format, and all three
    /// of the rule's outcomes are reachable: none, depth, depth and stencil.
    func testABoundRenderTargetDecidesTheDefaultClearOptions() throws {
        try requireNative()
        for requested in [G.DepthFormat.None, .Depth16, .Depth24, .Depth24Stencil8] {
            let game = try run { game, device in
                let target = try self.target(device, requested)
                try device.SetRenderTarget(target)
                game.observations["granted"] = String(target.DepthStencilFormat.rawValue)
                game.observations["options"] = String(try device.defaultClearOptions.rawValue)
                try device.SetRenderTarget(nil)
                game.observations["afterUnset"] =
                    String(try device.defaultClearOptions.rawValue)
                try target.Dispose()
            }
            let granted = G.DepthFormat(
                rawValue: Int32(game.observations["granted"]!)!)!
            XCTAssertEqual(
                game.observations["options"],
                String(expected(for: granted).rawValue),
                "requested \(requested), granted \(granted)")
            // Unsetting restores the backbuffer's own answer, which on a
            // manager-created device is Target|DepthBuffer.
            XCTAssertEqual(
                game.observations["afterUnset"],
                String(G.ClearOptions([.Target, .DepthBuffer]).rawValue))
        }
    }

    /// The depth-and-stencil outcome is not merely reachable in principle:
    /// the qualified artifact grants `Depth24Stencil8`, so the test above
    /// really does exercise the `Target|DepthBuffer|Stencil` branch.
    func testTheQualifiedDeviceGrantsADepthStencilRenderTarget() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try self.target(device, .Depth24Stencil8)
            game.observations["granted"] = String(target.DepthStencilFormat.rawValue)
            try device.SetRenderTarget(target)
            game.observations["options"] = String(try device.defaultClearOptions.rawValue)
            try device.SetRenderTarget(nil)
            try target.Dispose()
        }
        XCTAssertEqual(
            game.observations["granted"],
            String(G.DepthFormat.Depth24Stencil8.rawValue))
        XCTAssertEqual(
            game.observations["options"],
            String(G.ClearOptions([.Target, .DepthBuffer, .Stencil]).rawValue))
    }

    /// A bound render target cannot be destroyed while it is bound: CNA
    /// answers `CNA_RESULT_INVALID_STATE`, measured in
    /// `build-probe/f48b_params.c`. So the case the weak
    /// `RuntimeState.currentRenderTarget` exists to survive — the caller
    /// dropping a target the device still holds — is a state the runtime
    /// cannot leave cleanly in the first place, and the reference is weak for
    /// the retain cycle alone.
    func testUnsettingBeforeDisposingIsWhatKeepsTheDeviceConsistent() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try self.target(device, .Depth24Stencil8)
            try device.SetRenderTarget(target)
            try device.SetRenderTarget(nil)
            try target.Dispose()
            game.observations["disposed"] = String(target.IsDisposed)
            game.observations["options"] = String(try device.defaultClearOptions.rawValue)
        }
        XCTAssertEqual(game.observations["disposed"], "true")
        XCTAssertEqual(
            game.observations["options"],
            String(G.ClearOptions([.Target, .DepthBuffer]).rawValue))
    }

    // ------------------------------------------------------------------
    // The three overloads.
    // ------------------------------------------------------------------

    /// All three forms clear, on the backbuffer and on a render target that
    /// has a depth and a stencil buffer — which is where the colour-only
    /// projection used to differ from XNA.
    func testEveryClearOverloadExecutes() throws {
        try requireNative()
        _ = try run { game, device in
            try device.Clear(Microsoft.Xna.Framework.Color.CornflowerBlue)
            try device.Clear(
                [.Target, .DepthBuffer], color: Microsoft.Xna.Framework.Color.Black,
                depth: 1.0, stencil: 0)
            try device.Clear(
                .Target, color: Microsoft.Xna.Framework.Vector4(0, 0, 0, 1),
                depth: 1.0, stencil: 0)
            let target = try self.target(device, .Depth24Stencil8)
            try device.SetRenderTarget(target)
            // Forwards Target|DepthBuffer|Stencil, which is the whole point.
            try device.Clear(Microsoft.Xna.Framework.Color.CornflowerBlue)
            try device.SetRenderTarget(nil)
            try target.Dispose()
        }
    }

    // ------------------------------------------------------------------
    // CannotClearNullDepth: a diagnosis, not a pre-validation.
    // ------------------------------------------------------------------

    /// A failed clear that asked for a depth buffer the device does not have
    /// is reported as XNA reports it, with the pinned message.
    func testAFailedClearOfAnAbsentDepthBufferIsDiagnosed() throws {
        try requireNative()
        let game = try run { game, device in
            // The backbuffer has a depth buffer but no stencil buffer, and
            // 0x10 is a bit CNA rejects — so the clear fails while Stencil
            // was requested and could not have been honoured.
            let options = G.ClearOptions(
                rawValue: G.ClearOptions.Stencil.rawValue | 0x10)
            do {
                try device.Clear(
                    options, color: Microsoft.Xna.Framework.Color.Black,
                    depth: 1.0, stencil: 0)
                game.observations["threw"] = "nothing"
            } catch let error as CNAInvalidOperationException {
                game.observations["threw"] = "InvalidOperationException"
                game.observations["message"] = error.Message
            }
        }
        XCTAssertEqual(game.observations["threw"], "InvalidOperationException")
        XCTAssertEqual(
            game.observations["message"],
            "Cannot clear depth or stencil because the device does not have "
            + "an active depth or stencil buffer.")
    }

    /// The same failing call, with a depth-stencil render target bound, is
    /// **not** diagnosed that way: the device does have a stencil buffer, so the
    /// failure is the native one and is surfaced as such. This is what
    /// separates the diagnosis from a blanket rewrite of every failure.
    func testTheDiagnosisDoesNotFireWhenTheBuffersExist() throws {
        try requireNative()
        let game = try run { game, device in
            let target = try self.target(device, .Depth24Stencil8)
            try device.SetRenderTarget(target)
            let options = G.ClearOptions(
                rawValue: G.ClearOptions.Stencil.rawValue | 0x10)
            do {
                try device.Clear(
                    options, color: Microsoft.Xna.Framework.Color.Black,
                    depth: 1.0, stencil: 0)
                game.observations["threw"] = "nothing"
            } catch is CNAInvalidOperationException {
                game.observations["threw"] = "InvalidOperationException"
            } catch let error as CNAError {
                game.observations["threw"] = "CNAError"
                game.observations["operation"] = String(describing: error)
            }
            try device.SetRenderTarget(nil)
            try target.Dispose()
        }
        XCTAssertEqual(game.observations["threw"], "CNAError")
        XCTAssertEqual(
            game.observations["operation"]?.contains("cna_graphics_device_clear_options"),
            true)
    }

    /// A failure that asked for no depth or stencil bits at all is never the
    /// null-depth diagnosis, whatever the device has.
    func testAFailureWithoutDepthOrStencilBitsSurfacesTheNativeError() throws {
        try requireNative()
        let game = try run { game, device in
            do {
                try device.Clear(
                    G.ClearOptions(rawValue: 0x10),
                    color: Microsoft.Xna.Framework.Color.Black,
                    depth: 1.0, stencil: 0)
                game.observations["threw"] = "nothing"
            } catch is CNAInvalidOperationException {
                game.observations["threw"] = "InvalidOperationException"
            } catch is CNAError {
                game.observations["threw"] = "CNAError"
            }
        }
        XCTAssertEqual(game.observations["threw"], "CNAError")
    }

    /// The `Vector4` overload really does reach the same body: it produces
    /// the same diagnosis for the same mask.
    func testTheVector4OverloadForwardsIntoTheColorOverload() throws {
        try requireNative()
        let game = try run { game, device in
            let options = G.ClearOptions(
                rawValue: G.ClearOptions.Stencil.rawValue | 0x10)
            do {
                try device.Clear(
                    options, color: Microsoft.Xna.Framework.Vector4(0, 0, 0, 1),
                    depth: 1.0, stencil: 0)
                game.observations["threw"] = "nothing"
            } catch let error as CNAInvalidOperationException {
                game.observations["threw"] = "InvalidOperationException"
                game.observations["message"] = error.Message
            }
        }
        XCTAssertEqual(game.observations["threw"], "InvalidOperationException")
        XCTAssertEqual(
            game.observations["message"],
            "Cannot clear depth or stencil because the device does not have "
            + "an active depth or stencil buffer.")
    }

    // ------------------------------------------------------------------
    // The mask conversion, which no runtime observation can see.
    // ------------------------------------------------------------------

    /// Each declared option maps to its own canonical bit, and a bit XNA does
    /// not declare crosses unchanged so the device still refuses it.
    func testClearOptionsMapBitForBit() throws {
        typealias N = G.NativeStateCodes
        XCTAssertEqual(N.clearOptions([]), 0)
        XCTAssertEqual(N.clearOptions(.Target), 1)
        XCTAssertEqual(N.clearOptions(.DepthBuffer), 2)
        XCTAssertEqual(N.clearOptions(.Stencil), 4)
        XCTAssertEqual(N.clearOptions([.Target, .DepthBuffer]), 3)
        XCTAssertEqual(N.clearOptions([.Target, .DepthBuffer, .Stencil]), 7)
        XCTAssertEqual(N.clearOptions(G.ClearOptions(rawValue: 0x10)), 0x10)
        XCTAssertEqual(N.clearOptions(G.ClearOptions(rawValue: 0x11)), 0x11)
    }
}
