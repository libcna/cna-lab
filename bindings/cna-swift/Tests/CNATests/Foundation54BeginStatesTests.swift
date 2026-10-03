// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class BeginProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((BeginProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (BeginProbeGame, G.GraphicsDevice) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try F.GraphicsDeviceManager(game: self)
        // The custom state below blends alpha separately, which XNA's Reach
        // profile refuses and CNA enforces since ABI 0.35, so the probe asks
        // for HiDef the way an XNA game using such a state must.
        manager?.GraphicsProfile = .HiDef
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

/// Foundation 54: `Begin`'s state overloads, the four `??` defaults, and the
/// moment the states reach the device.
final class Foundation54BeginStatesTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (BeginProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> BeginProbeGame {
        let game = try BeginProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    /// The state objects' writers are throwing methods, not Swift setters:
    /// their CLR setters call `ThrowIfBound`, and Swift has no throwing setter.
    private func custom() throws -> G.BlendState {
        let state = G.BlendState()
        try state.SetColorSourceBlend(.SourceAlpha)
        try state.SetColorDestinationBlend(.One)
        try state.SetAlphaBlendFunction(.Subtract)
        return state
    }

    /// A Deferred batch does not touch the device between `Begin` and `End`.
    /// XNA calls `SetRenderState` from `End` for every mode but `Immediate`,
    /// and the difference is observable through `GraphicsDevice.BlendState`.
    func testADeferredBatchAppliesItsStatesAtEnd() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let state = try self.custom()
            try device.SetBlendState(G.BlendState.Opaque)
            game.observations["before"] = String(describing: device.BlendState?.Name)
            try batch.Begin(.Deferred, blendState: state)
            game.observations["duringPair"] = String(describing: device.BlendState?.Name)
            game.observations["sameAsBefore"] =
                String(device.BlendState === G.BlendState.Opaque)
            try batch.End()
            game.observations["afterEnd"] = String(device.BlendState === state)
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["sameAsBefore"], "true",
                       "a Deferred Begin leaves the device's state alone")
        XCTAssertEqual(game.observations["afterEnd"], "true",
                       "End is where SetRenderState runs")
    }

    /// An Immediate batch applies them at `Begin` instead — the one branch
    /// XNA's `Begin` takes on the sort mode.
    func testAnImmediateBatchAppliesItsStatesAtBegin() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let state = try self.custom()
            try device.SetBlendState(G.BlendState.Opaque)
            try batch.Begin(.Immediate, blendState: state)
            game.observations["duringPair"] = String(device.BlendState === state)
            try batch.End()
            game.observations["afterEnd"] = String(device.BlendState === state)
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["duringPair"], "true",
                       "Immediate applies at Begin")
        XCTAssertEqual(game.observations["afterEnd"], "true")
    }

    /// An Immediate batch's `End` does **not** re-apply the states. XNA's
    /// `End` calls `SetRenderState` only for the non-immediate modes; for
    /// `Immediate` it decrements the device's immediate counter instead. The
    /// difference shows when a caller changes the device's state inside the
    /// pair: XNA leaves that change standing, and an `End` that re-applied
    /// would quietly undo it.
    func testAnImmediateEndDoesNotReapplyTheStates() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let atBegin = try self.custom()
            try batch.Begin(.Immediate, blendState: atBegin)
            try device.SetBlendState(G.BlendState.Additive)
            game.observations["insidePair"] =
                String(device.BlendState === G.BlendState.Additive)
            try batch.End()
            game.observations["afterEnd"] =
                String(device.BlendState === G.BlendState.Additive)
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["insidePair"], "true")
        XCTAssertEqual(game.observations["afterEnd"], "true",
                       "End must not put the batch's own state back")
    }

    /// The four `??` defaults, each an `ldsfld` of a preset in
    /// `SetRenderState`'s null branch. Passing nothing selects them, and they
    /// are four *different* presets — not four copies of one idea.
    func testTheFourNullDefaultsAreTheSetRenderStatePresets() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            try device.SetBlendState(G.BlendState.Opaque)
            try device.SetDepthStencilState(G.DepthStencilState.Default)
            try device.SetRasterizerState(G.RasterizerState.CullNone)
            try batch.Begin()
            try batch.End()
            game.observations["blend"] =
                String(device.BlendState === G.BlendState.AlphaBlend)
            game.observations["depth"] =
                String(device.DepthStencilState === G.DepthStencilState.None)
            game.observations["raster"] =
                String(device.RasterizerState === G.RasterizerState.CullCounterClockwise)
            game.observations["sampler"] = String(
                try device.SamplerStates?.Item(0) === G.SamplerState.LinearClamp)
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["blend"], "true", "BlendState.AlphaBlend")
        XCTAssertEqual(game.observations["depth"], "true", "DepthStencilState.None")
        XCTAssertEqual(game.observations["raster"], "true",
                       "RasterizerState.CullCounterClockwise")
        XCTAssertEqual(game.observations["sampler"], "true", "SamplerState.LinearClamp")
    }

    /// The five-parameter overload carries all four through, and each reaches
    /// its own device member rather than one standing in for another.
    func testTheFiveParameterOverloadCarriesEachStateToItsOwnSlot() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let blend = try self.custom()
            let sampler = G.SamplerState()
            try sampler.SetMaxAnisotropy(8)
            let depth = G.DepthStencilState()
            try depth.SetDepthBufferEnable(false)
            let raster = G.RasterizerState()
            try raster.SetFillMode(.WireFrame)
            try batch.Begin(.Deferred, blendState: blend, samplerState: sampler,
                            depthStencilState: depth, rasterizerState: raster)
            try batch.End()
            game.observations["blend"] = String(device.BlendState === blend)
            game.observations["depth"] = String(device.DepthStencilState === depth)
            game.observations["raster"] = String(device.RasterizerState === raster)
            game.observations["sampler"] =
                String(try device.SamplerStates?.Item(0) === sampler)
            try batch.Dispose()
        }
        for key in ["blend", "depth", "raster", "sampler"] {
            XCTAssertEqual(game.observations[key], "true", key)
        }
    }

    /// A state handed to `Begin` is bound by the device setter, exactly as if
    /// the caller had assigned it — so it is read-only afterwards. That is a
    /// consequence of going *through* the projected device members rather than
    /// around them.
    func testAStatePassedToBeginBecomesReadOnly() throws {
        try requireNative()
        let game = try run { game, device in
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let state = try self.custom()
            try state.SetColorSourceBlend(.One)     // still writable here
            try batch.Begin(.Deferred, blendState: state)
            try batch.End()
            do {
                try state.SetColorSourceBlend(.Zero)
                game.observations["afterBind"] = "accepted"
            } catch let error as CNAInvalidOperationException {
                game.observations["afterBind"] = "refused"
                game.observations["message"] = error.Message
            }
            game.observations["value"] = String(describing: state.ColorSourceBlend)
            try batch.Dispose()
        }
        XCTAssertEqual(game.observations["afterBind"], "refused",
                       "the state is bound, so its writer raises ThrowIfBound's message")
        XCTAssertEqual(
            game.observations["message"],
            "Cannot change read-only BlendState. State objects become read-only "
            + "the first time they are bound to a GraphicsDevice. To change "
            + "property values, create a new BlendState instance.")
        XCTAssertEqual(game.observations["value"], "One",
                       "and the refused write left the value alone")
    }
}
