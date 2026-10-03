// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private enum CallbackProbeError: Error, Equatable {
    case initialize
    case loadContent
    case update
    case draw
    case unloadContent
}

private enum StressProbeError: Error {
    case invalidPNGWasAccepted
}

private let nativeTestPNG = Data(base64Encoded:
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M/wHwAF/gL+Xw2kWQAAAABJRU5ErkJggg=="
)!

private final class LockedErrorBox: @unchecked Sendable {
    private let lock = NSLock()
    private var value: Error?

    func store(_ error: Error) {
        lock.lock()
        value = error
        lock.unlock()
    }

    func take() -> Error? {
        lock.lock()
        defer { lock.unlock() }
        return value
    }
}

private final class LockedGamePadBox: @unchecked Sendable {
    private let lock = NSLock()
    private var state: Microsoft.Xna.Framework.Input.GamePadState?
    private var error: Error?

    func store(state: Microsoft.Xna.Framework.Input.GamePadState) {
        lock.lock()
        self.state = state
        lock.unlock()
    }

    func store(error: Error) {
        lock.lock()
        self.error = error
        lock.unlock()
    }

    func take() -> (Microsoft.Xna.Framework.Input.GamePadState?, Error?) {
        lock.lock()
        defer { lock.unlock() }
        return (state, error)
    }
}

private final class LifecycleProbeGame: Microsoft.Xna.Framework.Game {
    let frameLimit: Int
    var events: [String] = []
    var Updates = 0
    var Draws = 0
    /// (TotalGameTime, ElapsedGameTime) as the host reported it, in order.
    var updateTimings: [(total: Duration, elapsed: Duration)] = []
    var drawTimings: [(total: Duration, elapsed: Duration)] = []
    var viewport: Microsoft.Xna.Framework.Graphics.Viewport?
    var observedPressedKeys: [Microsoft.Xna.Framework.Input.Keys] = []
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var texture: Microsoft.Xna.Framework.Graphics.Texture2D?
    var spriteBatch: Microsoft.Xna.Framework.Graphics.SpriteBatch?

    init(frameLimit: Int, exerciseGraphics: Bool = false) throws {
        self.frameLimit = frameLimit
        try super.init()
        if exerciseGraphics {
            manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
        }
    }

    override func Initialize() throws { events.append("Initialize") }

    override func LoadContent() throws {
        events.append("LoadContent")
        guard manager != nil else { return }
        // `Game.GraphicsDevice` resolves the graphics device SERVICE out of
        // `Services` and returns its Optional device, exactly as XNA's getter
        // does, so a game with no manager raises and a game with one may still
        // report nil before the device exists.
        guard let device = try GraphicsDevice else {
            XCTFail("the registered graphics device service produced no device")
            return
        }
        viewport = try device.Viewport
        try device.Clear(.CornflowerBlue)
        observedPressedKeys = try Microsoft.Xna.Framework.Input.Keyboard.GetState().GetPressedKeys()

        // A test-owned, valid 1x1 RGBA PNG. Its native-reported dimensions are
        // asserted below; Texture2D never synthesizes those dimensions.
        texture = try Microsoft.Xna.Framework.Graphics.Texture2D.FromStream(
            device,
            stream: InputStream(data: nativeTestPNG)
        )
        spriteBatch = try Microsoft.Xna.Framework.Graphics.SpriteBatch(graphicsDevice: device)
    }

    override func BeginRun() throws { events.append("BeginRun") }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        XCTAssertGreaterThanOrEqual(gameTime.TotalGameTime, .zero)
        XCTAssertGreaterThanOrEqual(gameTime.ElapsedGameTime, .zero)
        updateTimings.append((gameTime.TotalGameTime, gameTime.ElapsedGameTime))
        Updates += 1
        if Updates == frameLimit { try Exit() }
    }

    override func Draw(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        drawTimings.append((gameTime.TotalGameTime, gameTime.ElapsedGameTime))
        Draws += 1
        guard let texture, let spriteBatch else { return }
        try GraphicsDevice?.Clear(.CornflowerBlue)
        try spriteBatch.Begin()
        try spriteBatch.Draw(
            texture,
            position: Microsoft.Xna.Framework.Vector2(Float(Draws), 2),
            sourceRectangle: nil,
            color: .White,
            rotation: Float(Draws) * 0.01,
            origin: .Zero,
            scale: 1,
            effects: .None,
            layerDepth: 0
        )
        try spriteBatch.End()
    }

    override func EndRun() throws { events.append("EndRun") }
    override func UnloadContent() throws { events.append("UnloadContent") }
    override func OnExiting(_ sender: Any?, args: CNAEventArgs) throws { events.append("Exiting") }
}

private final class ResourceStressGame: Microsoft.Xna.Framework.Game {
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var retainedTexture: Microsoft.Xna.Framework.Graphics.Texture2D?
    var retainedBatch: Microsoft.Xna.Framework.Graphics.SpriteBatch?
    var retainedBorrow: Microsoft.Xna.Framework.Graphics.GraphicsDevice?
    var TextureCycles = 0
    var SpriteBatchCycles = 0

    override init() throws {
        try super.init()
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        guard let device = try GraphicsDevice else {
            XCTFail("the registered graphics device service produced no device")
            return
        }
        retainedBorrow = device
        for index in 0..<20 {
            let texture = try Microsoft.Xna.Framework.Graphics.Texture2D.FromStream(
                device,
                stream: InputStream(data: nativeTestPNG)
            )
            TextureCycles += 1
            let batch = try Microsoft.Xna.Framework.Graphics.SpriteBatch(graphicsDevice: device)
            SpriteBatchCycles += 1
            try batch.Begin()
            try batch.Draw(
                texture,
                position: Microsoft.Xna.Framework.Vector2(Float(index), Float(index)),
                color: .White
            )
            try batch.End()
            try batch.Dispose()
            try batch.Dispose()
            try texture.Dispose()
            try texture.Dispose()
        }

        do {
            _ = try Microsoft.Xna.Framework.Graphics.Texture2D.FromStream(
                device,
                stream: InputStream(data: Data([0x00, 0x01, 0x02, 0x03]))
            )
            throw StressProbeError.invalidPNGWasAccepted
        } catch StressProbeError.invalidPNGWasAccepted {
            throw StressProbeError.invalidPNGWasAccepted
        } catch {
            // A structured native decode failure is the required result.
        }

        retainedTexture = try Microsoft.Xna.Framework.Graphics.Texture2D.FromStream(
            device,
            stream: InputStream(data: nativeTestPNG)
        )
        retainedBatch = try Microsoft.Xna.Framework.Graphics.SpriteBatch(graphicsDevice: device)
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws { try Exit() }
}

private final class FailingProbeGame: Microsoft.Xna.Framework.Game {
    let failure: CallbackProbeError

    init(_ failure: CallbackProbeError) throws {
        self.failure = failure
        try super.init()
    }

    override func Initialize() throws {
        if failure == .initialize { throw CallbackProbeError.initialize }
    }

    override func LoadContent() throws {
        if failure == .loadContent { throw CallbackProbeError.loadContent }
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        if failure == .update { throw CallbackProbeError.update }
        if failure == .unloadContent { try Exit() }
    }

    override func Draw(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        if failure == .draw { throw CallbackProbeError.draw }
    }

    override func UnloadContent() throws {
        if failure == .unloadContent { throw CallbackProbeError.unloadContent }
    }
}

private final class GamePadProbeGame: Microsoft.Xna.Framework.Game {
    var defaultState: Microsoft.Xna.Framework.Input.GamePadState?
    var noneState: Microsoft.Xna.Framework.Input.GamePadState?
    var independentAxesState: Microsoft.Xna.Framework.Input.GamePadState?
    var circularState: Microsoft.Xna.Framework.Input.GamePadState?
    var capabilities: Microsoft.Xna.Framework.Input.GamePadCapabilities?
    var vibrationApplied: Bool?
    var stateCycles = 0
    var capabilityCycles = 0

    override func LoadContent() throws {
        typealias I = Microsoft.Xna.Framework.Input
        for _ in 0..<50 {
            defaultState = try I.GamePad.GetState(.One)
            noneState = try I.GamePad.GetState(.One, deadZoneMode: .None)
            independentAxesState = try I.GamePad.GetState(
                .One, deadZoneMode: .IndependentAxes
            )
            circularState = try I.GamePad.GetState(.One, deadZoneMode: .Circular)
            stateCycles += 1
        }
        for _ in 0..<20 {
            capabilities = try I.GamePad.GetCapabilities(.One)
            capabilityCycles += 1
        }
        vibrationApplied = try I.GamePad.SetVibration(.One, leftMotor: 0, rightMotor: 0)
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws { try Exit() }
}

private final class GamePadThreadProbeGame: Microsoft.Xna.Framework.Game {
    let result = LockedGamePadBox()

    override func LoadContent() throws {
        let finished = DispatchSemaphore(value: 0)
        let result = self.result
        Thread.detachNewThread {
            do {
                result.store(state: try Microsoft.Xna.Framework.Input.GamePad.GetState(.One))
            } catch {
                result.store(error: error)
            }
            finished.signal()
        }
        guard finished.wait(timeout: .now() + 10) == .success else {
            throw CNAError.nativeFailure(
                operation: "GamePad wrong-thread qualification",
                result: UInt32.max,
                message: "detached query did not return"
            )
        }
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws { try Exit() }
}

final class NativeLifecycleTests: XCTestCase {
    internal var nativeConfigured: Bool {
        guard let value = ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] else { return false }
        return value.hasPrefix("/") && FileManager.default.fileExists(atPath: value)
    }

    internal func requireNative() throws {
        if !nativeConfigured { throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library") }
    }

    func testNativeGameSixtyFramesAndGraphicsCanary() throws {
        try requireNative()
        let game = try LifecycleProbeGame(frameLimit: 60, exerciseGraphics: true)
        try game.Run()
        // Exit is requested from the 60th Update; a fixed-step tick that
        // overran (a real renderer under vsync, never HEADLESS) still finishes
        // its catch-up Updates, as XNA's Tick does -- SW-44.
        XCTAssertGreaterThanOrEqual(game.Updates, 60)
        XCTAssertGreaterThan(game.Draws, 0)
        XCTAssertLessThanOrEqual(game.Draws, game.Updates)
        XCTAssertEqual(game.viewport?.Width, 800)
        XCTAssertEqual(game.viewport?.Height, 480)
        XCTAssertEqual(game.texture?.Width, 1)
        XCTAssertEqual(game.texture?.Height, 1)
        // `Texture2D.get_Bounds` builds a Rectangle at the origin over the
        // texture's own dimensions and has no failure path, so it composes the
        // two members above rather than asking the runtime anything.
        XCTAssertEqual(game.texture?.Bounds.X, 0)
        XCTAssertEqual(game.texture?.Bounds.Y, 0)
        XCTAssertEqual(game.texture?.Bounds.Width, game.texture?.Width)
        XCTAssertEqual(game.texture?.Bounds.Height, game.texture?.Height)
        XCTAssertTrue(game.observedPressedKeys.isEmpty)
        XCTAssertEqual(Array(game.events.prefix(3)), ["Initialize", "LoadContent", "BeginRun"])
        XCTAssertTrue(game.events.contains("EndRun"))
        try game.Dispose()
        XCTAssertTrue(game.events.contains("UnloadContent"))
        XCTAssertTrue(game.events.contains("Exiting"))
    }

    func testNativeGameSixHundredFrames() throws {
        try requireNative()
        let game = try LifecycleProbeGame(frameLimit: 600)
        try game.Run()
        XCTAssertGreaterThanOrEqual(game.Updates, 600)
        // A fixed-step loop may suppress Draw while it catches up after more
        // than one delayed Update. The soak contract is that rendering stays
        // live and never outruns updates, not that at most one Draw is skipped.
        XCTAssertGreaterThan(game.Draws, 0)
        XCTAssertLessThanOrEqual(game.Draws, game.Updates)
        try game.Dispose()
    }

    func testCallbackErrorsReturnToRunBoundary() throws {
        try requireNative()
        for failure in [CallbackProbeError.initialize, .loadContent, .update, .draw] {
            let game = try FailingProbeGame(failure)
            XCTAssertThrowsError(try game.Run()) { error in
                XCTAssertEqual(error as? CallbackProbeError, failure)
            }
            try game.Dispose()
        }

        let unloadGame = try FailingProbeGame(.unloadContent)
        try unloadGame.Run()
        XCTAssertThrowsError(try unloadGame.Dispose()) { error in
            XCTAssertEqual(error as? CallbackProbeError, .unloadContent)
        }
    }

    func testTwentyGameRecreations() throws {
        try requireNative()
        for _ in 0..<20 {
            let game = try LifecycleProbeGame(frameLimit: 1)
            try game.Run()
            try game.Dispose()
            XCTAssertThrowsError(try game.Run())
        }
    }

    func testTwentyTextureAndSpriteBatchCycles() throws {
        try requireNative()
        let game = try ResourceStressGame()
        try game.Run()
        XCTAssertEqual(game.TextureCycles, 20)
        XCTAssertEqual(game.SpriteBatchCycles, 20)
        XCTAssertThrowsError(try game.retainedBorrow?.Clear(.Black))

        let captured = LockedErrorBox()
        let finished = expectation(description: "wrong-thread Texture2D.Dispose returned")
        Thread.detachNewThread {
            do { try game.retainedTexture?.Dispose() } catch { captured.store(error) }
            finished.fulfill()
        }
        wait(for: [finished], timeout: 10)
        guard case .ownerThreadViolation? = captured.take() as? CNAError else {
            return XCTFail("wrong-thread resource disposal did not report owner-thread failure")
        }
        try game.retainedTexture?.Dispose()

        let retainedTexture = game.retainedTexture
        let retainedBatch = game.retainedBatch
        try game.Dispose()
        try retainedTexture?.Dispose()
        try retainedBatch?.Dispose()
        XCTAssertThrowsError(try game.retainedBorrow?.Clear(.Black))
    }

    func testTwentyCallbackErrorCycles() throws {
        try requireNative()
        for index in 0..<20 {
            let failure: CallbackProbeError = index.isMultiple(of: 2) ? .update : .draw
            let game = try FailingProbeGame(failure)
            XCTAssertThrowsError(try game.Run()) { error in
                XCTAssertEqual(error as? CallbackProbeError, failure)
            }
            try game.Dispose()
        }
    }

    func testWrongThreadDisposeKeepsHandleForOwnerRetry() throws {
        try requireNative()
        let game = try LifecycleProbeGame(frameLimit: 1)
        try game.Run()
        let finished = expectation(description: "wrong-thread Dispose returned")
        let captured = LockedErrorBox()
        Thread.detachNewThread {
            do { try game.Dispose() } catch { captured.store(error) }
            finished.fulfill()
        }
        wait(for: [finished], timeout: 10)
        let error = captured.take()
        guard case .ownerThreadViolation? = error as? CNAError else {
            return XCTFail("expected owner-thread error, got \(String(describing: error))")
        }
        try game.Dispose()
    }

    func testGamePadNativeRoutesAndDisconnectedOrHardwareSnapshot() throws {
        try requireNative()
        let game = try GamePadProbeGame()
        try game.Run()
        XCTAssertEqual(game.stateCycles, 50)
        XCTAssertEqual(game.capabilityCycles, 20)
        let states = [
            game.defaultState, game.noneState,
            game.independentAxesState, game.circularState,
        ]
        XCTAssertTrue(states.allSatisfy { $0 != nil })
        guard let capabilities = game.capabilities,
              let defaultState = game.defaultState else {
            return XCTFail("GamePad native routes did not return snapshots")
        }
        if !capabilities.IsConnected {
            XCTAssertEqual(capabilities.GamePadType, .Unknown)
            XCTAssertFalse(capabilities.HasAButton)
            XCTAssertFalse(capabilities.HasBackButton)
            XCTAssertFalse(capabilities.HasBButton)
            XCTAssertFalse(capabilities.HasDPadDownButton)
            XCTAssertFalse(capabilities.HasDPadLeftButton)
            XCTAssertFalse(capabilities.HasDPadRightButton)
            XCTAssertFalse(capabilities.HasDPadUpButton)
            XCTAssertFalse(capabilities.HasLeftShoulderButton)
            XCTAssertFalse(capabilities.HasLeftStickButton)
            XCTAssertFalse(capabilities.HasRightShoulderButton)
            XCTAssertFalse(capabilities.HasRightStickButton)
            XCTAssertFalse(capabilities.HasStartButton)
            XCTAssertFalse(capabilities.HasXButton)
            XCTAssertFalse(capabilities.HasYButton)
            XCTAssertFalse(capabilities.HasBigButton)
            XCTAssertFalse(capabilities.HasLeftXThumbStick)
            XCTAssertFalse(capabilities.HasLeftYThumbStick)
            XCTAssertFalse(capabilities.HasRightXThumbStick)
            XCTAssertFalse(capabilities.HasRightYThumbStick)
            XCTAssertFalse(capabilities.HasLeftTrigger)
            XCTAssertFalse(capabilities.HasRightTrigger)
            XCTAssertFalse(capabilities.HasLeftVibrationMotor)
            XCTAssertFalse(capabilities.HasRightVibrationMotor)
            XCTAssertFalse(capabilities.HasVoiceSupport)
            XCTAssertFalse(defaultState.IsConnected)
            XCTAssertEqual(defaultState.PacketNumber, 0)
            XCTAssertEqual(defaultState.ThumbSticks.Left.X, 0)
            XCTAssertEqual(defaultState.ThumbSticks.Left.Y, 0)
            XCTAssertEqual(defaultState.ThumbSticks.Right.X, 0)
            XCTAssertEqual(defaultState.ThumbSticks.Right.Y, 0)
            XCTAssertEqual(defaultState.Triggers.Left, 0)
            XCTAssertEqual(defaultState.Triggers.Right, 0)
            XCTAssertEqual(game.vibrationApplied, false)
        }

        if let output = ProcessInfo.processInfo.environment["CNA_GAMEPAD_EVIDENCE_OUTPUT"] {
            let payload: [String: Any] = [
                "backend": "HEADLESS/NULL",
                "playerSlot": 0,
                "connected": capabilities.IsConnected,
                "gamePadType": capabilities.GamePadType.rawValue,
                "default": Self.stateEvidence(game.defaultState),
                "none": Self.stateEvidence(game.noneState),
                "independentAxes": Self.stateEvidence(game.independentAxesState),
                "circular": Self.stateEvidence(game.circularState),
                "capabilities": Self.capabilityEvidence(capabilities),
                "vibrationApplied": game.vibrationApplied as Any,
                "stateCycles": game.stateCycles,
                "capabilityCycles": game.capabilityCycles,
            ]
            let data = try JSONSerialization.data(
                withJSONObject: payload, options: [.prettyPrinted, .sortedKeys]
            )
            try data.write(to: URL(fileURLWithPath: output))
        }
        try game.Dispose()
        XCTAssertThrowsError(try Microsoft.Xna.Framework.Input.GamePad.GetState(.One))
    }

    func testGamePadQueriesFollowCurrentGeneration() throws {
        try requireNative()
        let first = try GamePadProbeGame()
        try first.Run()
        XCTAssertNotNil(first.defaultState)
        try first.Dispose()
        let second = try GamePadProbeGame()
        try second.Run()
        XCTAssertNotNil(second.defaultState)
        try second.Dispose()
    }

    func testGamePadWrongThreadQueryRejectsBeforeNativeEntry() throws {
        try requireNative()
        let game = try GamePadThreadProbeGame()
        try game.Run()
        let (state, error) = game.result.take()
        XCTAssertNil(state)
        guard case .ownerThreadViolation("GamePad.GetState")? = error as? CNAError else {
            try game.Dispose()
            return XCTFail("wrong-thread GamePad query did not report owner-thread failure")
        }
        try game.Dispose()
    }

    private static func stateEvidence(
        _ state: Microsoft.Xna.Framework.Input.GamePadState?
    ) -> [String: Any] {
        guard let state else { return ["returned": false] }
        return [
            "returned": true,
            "isConnected": state.IsConnected,
            "packetNumber": state.PacketNumber,
            "leftX": state.ThumbSticks.Left.X,
            "leftY": state.ThumbSticks.Left.Y,
            "rightX": state.ThumbSticks.Right.X,
            "rightY": state.ThumbSticks.Right.Y,
            "leftTrigger": state.Triggers.Left,
            "rightTrigger": state.Triggers.Right,
        ]
    }

    private static func capabilityEvidence(
        _ value: Microsoft.Xna.Framework.Input.GamePadCapabilities
    ) -> [String: Any] {
        [
            "gamePadType": value.GamePadType.rawValue,
            "isConnected": value.IsConnected,
            "hasAButton": value.HasAButton,
            "hasBackButton": value.HasBackButton,
            "hasBButton": value.HasBButton,
            "hasDPadDownButton": value.HasDPadDownButton,
            "hasDPadLeftButton": value.HasDPadLeftButton,
            "hasDPadRightButton": value.HasDPadRightButton,
            "hasDPadUpButton": value.HasDPadUpButton,
            "hasLeftShoulderButton": value.HasLeftShoulderButton,
            "hasLeftStickButton": value.HasLeftStickButton,
            "hasRightShoulderButton": value.HasRightShoulderButton,
            "hasRightStickButton": value.HasRightStickButton,
            "hasStartButton": value.HasStartButton,
            "hasXButton": value.HasXButton,
            "hasYButton": value.HasYButton,
            "hasBigButton": value.HasBigButton,
            "hasLeftXThumbStick": value.HasLeftXThumbStick,
            "hasLeftYThumbStick": value.HasLeftYThumbStick,
            "hasRightXThumbStick": value.HasRightXThumbStick,
            "hasRightYThumbStick": value.HasRightYThumbStick,
            "hasLeftTrigger": value.HasLeftTrigger,
            "hasRightTrigger": value.HasRightTrigger,
            "hasLeftVibrationMotor": value.HasLeftVibrationMotor,
            "hasRightVibrationMotor": value.HasRightVibrationMotor,
            "hasVoiceSupport": value.HasVoiceSupport,
        ]
    }
}

// The exact order in which the native host issues its lifecycle callbacks.
//
// This is not a projection question and it cannot be answered from XNA's IL:
// it is a fact about the CNA host, and the LoadContent dispatch decision turns
// on it. It is measured rather than assumed.
private final class OrderRecordingGame: Microsoft.Xna.Framework.Game {
    nonisolated(unsafe) static var order: [String] = []

    override func Initialize() throws {
        OrderRecordingGame.order.append("Initialize")
        try super.Initialize()
    }
    override func LoadContent() throws {
        OrderRecordingGame.order.append("LoadContent")
    }
    override func BeginRun() throws {
        OrderRecordingGame.order.append("BeginRun")
    }
    nonisolated(unsafe) static var exitAfterFirstUpdate = false

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        OrderRecordingGame.order.append("Update")
        try super.Update(gameTime)
        if OrderRecordingGame.exitAfterFirstUpdate { try Exit() }
    }
    override func Draw(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        OrderRecordingGame.order.append("Draw")
        try super.Draw(gameTime)
    }
    override func EndRun() throws {
        OrderRecordingGame.order.append("EndRun")
    }
    override func UnloadContent() throws {
        OrderRecordingGame.order.append("UnloadContent")
    }
}

extension NativeLifecycleTests {
    func testNativeCallbackOrderIsMeasuredNotAssumed() throws {
        try requireNative()
        OrderRecordingGame.order = []
        let game = try OrderRecordingGame()
        try game.RunOneFrame()
        try game.Dispose()
        print("CNA_CALLBACK_ORDER=\(OrderRecordingGame.order)")

        // LoadContent must be issued exactly once per lifecycle. The whole
        // point of resolving the dispatch question is that a consumer never
        // sees it twice.
        XCTAssertEqual(
            OrderRecordingGame.order.filter { $0 == "LoadContent" }.count, 1,
            "one XNA lifecycle occurrence must be one virtual invocation")
        XCTAssertEqual(
            OrderRecordingGame.order.filter { $0 == "Initialize" }.count, 1)

        // Initialize precedes LoadContent, which is XNA's order: the base
        // Initialize is where XNA calls LoadContent from.
        let initializeAt = OrderRecordingGame.order.firstIndex(of: "Initialize")
        let loadAt = OrderRecordingGame.order.firstIndex(of: "LoadContent")
        XCTAssertNotNil(initializeAt)
        XCTAssertNotNil(loadAt)
        if let initializeAt, let loadAt {
            XCTAssertLessThan(initializeAt, loadAt)
        }

        // The same measurement under a full `Run`, which is where the frame
        // hooks fire and therefore where `inRun` can change.
        OrderRecordingGame.order = []
        OrderRecordingGame.exitAfterFirstUpdate = true
        defer { OrderRecordingGame.exitAfterFirstUpdate = false }
        let running = try OrderRecordingGame()
        try running.Run()
        try running.Dispose()
        print("CNA_RUN_CALLBACK_ORDER=\(OrderRecordingGame.order)")
        XCTAssertEqual(
            OrderRecordingGame.order.filter { $0 == "LoadContent" }.count, 1,
            "a full Run must also issue LoadContent exactly once")
    }
}

extension NativeLifecycleTests {
    /// The `GameTime` the host hands each callback, pinned as a measurement.
    ///
    /// This is a **native runtime** observation, not XNA behaviour evidence.
    /// It is pinned because migrating from CNA C ABI 0.7.0 to 0.21.0 changed
    /// it, and nothing here noticed until a `> .zero` assertion failed.
    ///
    /// Measured on CNA 0.21.0, `IsFixedTimeStep` true, target 166,667 ticks,
    /// four consecutive `RunOneFrame` calls:
    ///
    /// ```text
    /// frame 0  Update total=0        elapsed=0        Draw total=0       elapsed=166667
    /// frame 1  Update total=0        elapsed=166667   Draw total=166667  elapsed=166667
    /// frame 2  Update total=166667   elapsed=166667   Draw total=333334  elapsed=166667
    /// frame 3  Update total=333334   elapsed=166667   Draw total=500001  elapsed=166667
    /// ```
    ///
    /// Frames 1 onward reproduce pinned XNA `Game.Tick` exactly: it assigns
    /// `gameTime.ElapsedGameTime = targetElapsedTime` and
    /// `gameTime.TotalGameTime = totalGameTime` *before* the `finally` adds a
    /// step, while `DrawFrame` re-reads the already advanced `totalGameTime`.
    /// **Frame 0's `Update` is XNA's, its `Draw` is the open question.**
    /// XNA's `RunGame`, after `BeginRun`, runs one pre-loop `Update` with
    /// `ElapsedGameTime = TimeSpan.Zero` and `TotalGameTime = totalGameTime`
    /// before the loop starts -- exactly frame 0's `Update`. (This file used to
    /// say no XNA callback ever sees a zero elapsed time under a fixed step;
    /// that read `Tick` alone and missed `RunGame`.) What XNA does not do is
    /// draw before the first `Tick` update: `Tick` returns without `DrawFrame`
    /// while its step count is zero. CNA 0.21 and 0.35 both draw once there.
    /// See `docs/native-abi-migration-evidence.md`.
    ///
    /// CNA 0.7.0 diverged differently, and in both halves: its `Update` saw a
    /// total one step ahead of XNA's, and its `Draw` saw the same total as its
    /// `Update` rather than the advanced one.
    func testHostGameTimeSequenceIsMeasuredNotAssumed() throws {
        try requireNative()
        let target = Duration(secondsComponent: 0, attosecondsComponent: 166_667 * 100_000_000_000)
        let game = try LifecycleProbeGame(frameLimit: 1_000)
        for _ in 0..<4 { try game.RunOneFrame() }
        try game.Dispose()

        print("CNA_HOST_UPDATE_TIMINGS=\(game.updateTimings)")
        print("CNA_HOST_DRAW_TIMINGS=\(game.drawTimings)")
        XCTAssertEqual(game.updateTimings.count, 4)
        XCTAssertEqual(game.drawTimings.count, 4)

        // The leading frame: XNA's pre-loop Update, then a Draw XNA's Tick
        // would not issue yet.
        XCTAssertEqual(game.updateTimings[0].total, .zero)
        XCTAssertEqual(game.updateTimings[0].elapsed, .zero,
                       "the pre-loop Update carries a zero elapsed time, as RunGame's does")
        XCTAssertEqual(game.drawTimings[0].total, .zero)
        XCTAssertEqual(game.drawTimings[0].elapsed, target)

        // Every later frame is XNA's own sequence, one step apart, with Draw
        // reading the total that Update's step already advanced.
        for frame in 1..<4 {
            XCTAssertEqual(game.updateTimings[frame].elapsed, target)
            XCTAssertEqual(game.drawTimings[frame].elapsed, target)
            XCTAssertEqual(game.updateTimings[frame].total, target * (frame - 1))
            XCTAssertEqual(game.drawTimings[frame].total, target * frame)
        }
    }
}
