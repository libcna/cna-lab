// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

/// A game that runs the device-state slice inside `LoadContent`, where a
/// callback-scoped graphics device exists.
private final class DeviceStateProbeGame: Microsoft.Xna.Framework.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((DeviceStateProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (DeviceStateProbeGame, G.GraphicsDevice) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try F.GraphicsDeviceManager(game: self)
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

    override func Update(_ gameTime: F.GameTime) throws { try Exit() }
}

/// Foundation 45: the three `GraphicsDevice` state properties and
/// `SamplerStateCollection`, over real CNA routes.
final class Foundation45DeviceStateTests: XCTestCase {
    private var nativeConfigured: Bool {
        ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] != nil
    }

    private func requireNative() throws {
        if !nativeConfigured {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    @discardableResult
    private func run(
        _ body: @escaping (DeviceStateProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> DeviceStateProbeGame {
        let game = try DeviceStateProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    // MARK: - The enum bridge, which needs no device

    /// CNA and XNA number `BlendFunction` the other way round. Eight
    /// neighbouring enums agree, which is why every one of the nine crosses
    /// through an explicit map: a `rawValue` cast would work for eight and
    /// silently exchange `Min` and `Max` for the ninth.
    func testBlendFunctionMinAndMaxAreExchangedAcrossTheBoundary() {
        typealias Codes = G.NativeStateCodes
        XCTAssertEqual(Codes.blendFunction(.Add), 0)
        XCTAssertEqual(Codes.blendFunction(.Subtract), 1)
        XCTAssertEqual(Codes.blendFunction(.ReverseSubtract), 2)
        XCTAssertEqual(Codes.blendFunction(.Min), 4, "CNA_BLEND_FUNCTION_MIN is 4")
        XCTAssertEqual(Codes.blendFunction(.Max), 3, "CNA_BLEND_FUNCTION_MAX is 3")
        // The XNA side, for contrast: the raw values are the other way round.
        XCTAssertEqual(G.BlendFunction.Min.rawValue, 3)
        XCTAssertEqual(G.BlendFunction.Max.rawValue, 4)
        // And the round trip returns what it started with.
        for value in [G.BlendFunction.Add, .Subtract, .ReverseSubtract, .Min, .Max] {
            XCTAssertEqual(Codes.blendFunction(native: Codes.blendFunction(value)), value)
        }
    }

    /// The other eight agree value for value, and the round trip is asserted
    /// rather than assumed for every case of every one.
    func testTheOtherEightEnumsRoundTripThroughTheBoundary() {
        typealias Codes = G.NativeStateCodes
        for value in [G.Blend.One, .Zero, .SourceColor, .InverseSourceColor,
                      .SourceAlpha, .InverseSourceAlpha, .DestinationColor,
                      .InverseDestinationColor, .DestinationAlpha,
                      .InverseDestinationAlpha, .BlendFactor,
                      .InverseBlendFactor, .SourceAlphaSaturation] {
            XCTAssertEqual(Codes.blend(value), UInt32(value.rawValue))
            XCTAssertEqual(Codes.blend(native: Codes.blend(value)), value)
        }
        for value in [G.CompareFunction.Always, .Never, .Less, .LessEqual,
                      .Equal, .GreaterEqual, .Greater, .NotEqual] {
            XCTAssertEqual(Codes.compareFunction(value), UInt32(value.rawValue))
            XCTAssertEqual(
                Codes.compareFunction(native: Codes.compareFunction(value)), value)
        }
        for value in [G.StencilOperation.Keep, .Zero, .Replace, .Increment,
                      .Decrement, .IncrementSaturation, .DecrementSaturation,
                      .Invert] {
            XCTAssertEqual(Codes.stencilOperation(value), UInt32(value.rawValue))
            XCTAssertEqual(
                Codes.stencilOperation(native: Codes.stencilOperation(value)), value)
        }
        for value in [G.CullMode.None, .CullClockwiseFace, .CullCounterClockwiseFace] {
            XCTAssertEqual(Codes.cullMode(value), UInt32(value.rawValue))
            XCTAssertEqual(Codes.cullMode(native: Codes.cullMode(value)), value)
        }
        for value in [G.FillMode.Solid, .WireFrame] {
            XCTAssertEqual(Codes.fillMode(value), UInt32(value.rawValue))
            XCTAssertEqual(Codes.fillMode(native: Codes.fillMode(value)), value)
        }
        for value in [G.TextureAddressMode.Wrap, .Clamp, .Mirror] {
            XCTAssertEqual(Codes.addressMode(value), UInt32(value.rawValue))
            XCTAssertEqual(Codes.addressMode(native: Codes.addressMode(value)), value)
        }
        for value in [G.TextureFilter.Linear, .Point, .Anisotropic,
                      .LinearMipPoint, .PointMipLinear,
                      .MinLinearMagPointMipLinear, .MinLinearMagPointMipPoint,
                      .MinPointMagLinearMipLinear, .MinPointMagLinearMipPoint] {
            XCTAssertEqual(Codes.textureFilter(value), UInt32(value.rawValue))
            XCTAssertEqual(Codes.textureFilter(native: Codes.textureFilter(value)), value)
        }
        for value in [G.ColorWriteChannels.None, .Red, .Green, .Blue, .Alpha, .All] {
            XCTAssertEqual(Codes.colorWriteChannels(value), UInt32(value.rawValue))
            XCTAssertEqual(
                Codes.colorWriteChannels(native: Codes.colorWriteChannels(value)), value)
        }
    }

    /// The descriptor writes CNA's field order, which groups by width and is
    /// not the XNA property order. The alpha and colour channels are the pair
    /// most likely to be transposed.
    func testTheBlendDescriptorCarriesEachChannelToItsOwnField() throws {
        let state = G.BlendState()
        try state.SetColorSourceBlend(.SourceAlpha)
        try state.SetColorDestinationBlend(.One)
        try state.SetAlphaSourceBlend(.DestinationColor)
        try state.SetAlphaDestinationBlend(.Zero)
        try state.SetColorBlendFunction(.Min)
        try state.SetAlphaBlendFunction(.Max)
        try state.SetMultiSampleMask(0x1234)
        try state.SetBlendFactor(F.Color.CornflowerBlue)
        let native = state.nativeDescriptor()
        XCTAssertEqual(native.color_source_blend, 4)
        XCTAssertEqual(native.color_destination_blend, 0)
        XCTAssertEqual(native.alpha_source_blend, 6)
        XCTAssertEqual(native.alpha_destination_blend, 1)
        XCTAssertEqual(native.color_blend_function, 4, "XNA Min is CNA 4")
        XCTAssertEqual(native.alpha_blend_function, 3, "XNA Max is CNA 3")
        XCTAssertEqual(native.multi_sample_mask, 0x1234)
        XCTAssertEqual(native.blend_factor.r, F.Color.CornflowerBlue.R)
        XCTAssertEqual(native.struct_version, 1)
    }

    // MARK: - The native round trip

    /// The first state this binding has actually applied to a live device.
    func testAssigningABlendStateReachesTheDeviceAndCachesIt() throws {
        try requireNative()
        let game = try run { game, device in
            game.observations["before"] = String(describing: device.BlendState)
            try device.SetBlendState(G.BlendState.Additive)
            game.observations["after"] = device.BlendState?.Name ?? "nil"
            game.observations["same"] =
                String(device.BlendState === G.BlendState.Additive)
            game.observations["factor"] =
                String(device.runtimeState.cachedBlendFactor.PackedValue)
            game.observations["mask"] = String(device.runtimeState.cachedMultiSampleMask)

            // `set_BlendState` copies BlendFactor and MultiSampleMask OUT of
            // the state into two fields of its own. Every preset carries
            // SetDefaults' Color.White and -1, which are also the runtime's
            // starting values — so a state with neither is the only way to see
            // the copy happen at all. The first version of this test used a
            // preset and could not tell a dropped copy from a working one.
            let custom = G.BlendState()
            try custom.SetBlendFactor(F.Color.CornflowerBlue)
            try custom.SetMultiSampleMask(0x0F0F)
            try device.SetBlendState(custom)
            game.observations["customFactor"] =
                String(device.runtimeState.cachedBlendFactor.PackedValue)
            game.observations["customMask"] =
                String(device.runtimeState.cachedMultiSampleMask)
        }
        XCTAssertEqual(game.observations["before"], "nil",
                       "no constructor assigns it, which is why the getter is Optional")
        XCTAssertEqual(game.observations["after"], "BlendState.Additive")
        XCTAssertEqual(game.observations["same"], "true",
                       "the device caches the instance, not a copy")
        XCTAssertEqual(
            game.observations["factor"], String(F.Color.White.PackedValue),
            "set_BlendState copies BlendFactor out of the state")
        XCTAssertEqual(game.observations["mask"], "-1")
        XCTAssertEqual(
            game.observations["customFactor"],
            String(F.Color.CornflowerBlue.PackedValue),
            "the copied BlendFactor moved with the state")
        XCTAssertEqual(game.observations["customMask"], String(0x0F0F))
    }

    /// Assignment binds the state, and a bound state refuses every write.
    func testAssignmentBindsTheStateSoLaterWritesAreRefused() throws {
        try requireNative()
        let state = G.DepthStencilState()
        XCTAssertFalse(state.isBound)
        try run { _, device in
            try device.SetDepthStencilState(state)
        }
        XCTAssertTrue(state.isBound, "Apply set isBound on first attachment")
        XCTAssertThrowsError(try state.SetStencilEnable(true)) { error in
            XCTAssertTrue(error is CNAInvalidOperationException, "\(error)")
        }
    }

    /// All three setters refuse null with the assembly's own message, and with
    /// the parameter name FIRST — `ArgumentNullException(paramName, message)`.
    func testAllThreeSettersRefuseNull() throws {
        try requireNative()
        try run { _, device in
            for body in [
                { try device.SetBlendState(nil) },
                { try device.SetDepthStencilState(nil) },
                { try device.SetRasterizerState(nil) },
            ] as [() throws -> Void] {
                assertProjected(
                    CNAArgumentNullException.self,
                    message: composedArgumentMessage(
                        "This method does not accept null for this parameter.",
                        paramName: "value"),
                    paramName: "value",
                    hResult: Int32(bitPattern: 0x8000_4003),
                    body)
            }
        }
    }

    // MARK: - The three values the state setters copy out

    /// `BlendFactor`, `MultiSampleMask` and `ReferenceStencil` are the fields
    /// `set_BlendState` and `set_DepthStencilState` copy out of the state they
    /// accept, and each is separately settable through a route of its own. A
    /// later state assignment overwrites what a direct write put there, which
    /// is the ordering XNA has.
    func testTheCopiedValuesAreAlsoSettableOnTheirOwn() throws {
        try requireNative()
        let game = try run { game, device in
            game.observations["factorBefore"] = String(device.BlendFactor.PackedValue)
            game.observations["maskBefore"] = String(device.MultiSampleMask)
            game.observations["stencilBefore"] = String(device.ReferenceStencil)

            try device.SetBlendFactor(F.Color.CornflowerBlue)
            try device.SetMultiSampleMask(0x00FF)
            try device.SetReferenceStencil(7)
            game.observations["factorAfter"] = String(device.BlendFactor.PackedValue)
            game.observations["maskAfter"] = String(device.MultiSampleMask)
            game.observations["stencilAfter"] = String(device.ReferenceStencil)

            // A state assignment overwrites all three from the state.
            let blend = G.BlendState()
            try blend.SetBlendFactor(F.Color.White)
            try blend.SetMultiSampleMask(-1)
            try device.SetBlendState(blend)
            game.observations["factorAfterState"] = String(device.BlendFactor.PackedValue)
            game.observations["maskAfterState"] = String(device.MultiSampleMask)
            try device.SetDepthStencilState(G.DepthStencilState.Default)
            game.observations["stencilAfterState"] = String(device.ReferenceStencil)
        }
        XCTAssertEqual(game.observations["factorBefore"],
                       String(F.Color.White.PackedValue))
        XCTAssertEqual(game.observations["maskBefore"], "-1")
        XCTAssertEqual(game.observations["stencilBefore"], "0")
        XCTAssertEqual(game.observations["factorAfter"],
                       String(F.Color.CornflowerBlue.PackedValue))
        XCTAssertEqual(game.observations["maskAfter"], String(0x00FF))
        XCTAssertEqual(game.observations["stencilAfter"], "7")
        XCTAssertEqual(game.observations["factorAfterState"],
                       String(F.Color.White.PackedValue),
                       "a state assignment overwrites a direct write")
        XCTAssertEqual(game.observations["maskAfterState"], "-1")
        XCTAssertEqual(game.observations["stencilAfterState"], "0",
                       "DepthStencilState.Default carries ReferenceStencil 0")
    }

    /// Each writer's recorded verdict is `IL_REACHABLE_THROW` with
    /// `ObjectDisposedException`, because XNA's setter opens with
    /// `Helpers.CheckDisposed`. Outside the callback that produced the facade
    /// there is no device to push to, which is this binding's analogue.
    func testTheWritersRefuseAFacadeOutsideItsCallback() throws {
        try requireNative()
        var escaped: G.GraphicsDevice?
        try run { _, device in escaped = device }
        guard let escaped else { return XCTFail("no device escaped") }
        XCTAssertThrowsError(try escaped.SetMultiSampleMask(1))
        XCTAssertThrowsError(try escaped.SetReferenceStencil(1))
        XCTAssertThrowsError(try escaped.SetBlendFactor(F.Color.White))
    }

    // MARK: - Viewport, scissor rectangle and device status

    /// `SetViewport` is the projection of the CLR setter accessor, and its
    /// absence was the last `PROPERTY_MAPPING_MISMATCH` in the scoreboard.
    /// A written viewport is read back through the reader that already existed.
    func testTheViewportRoundTripsThroughItsWriterMethod() throws {
        try requireNative()
        let game = try run { game, device in
            let before = try device.Viewport
            game.observations["before"] = "\(before.Width)x\(before.Height)"
            var moved = before
            moved.X = 8
            moved.Y = 16
            moved.Width = 320
            moved.Height = 240
            moved.MinDepth = 0.25
            moved.MaxDepth = 0.75
            try device.SetViewport(moved)
            let after = try device.Viewport
            game.observations["after"] =
                "\(after.X),\(after.Y) \(after.Width)x\(after.Height) "
                + "\(after.MinDepth)..\(after.MaxDepth)"
        }
        XCTAssertEqual(game.observations["before"], "800x480")
        XCTAssertEqual(game.observations["after"], "8,16 320x240 0.25..0.75")
    }

    /// Both scissor accessors are `IL_DIRECT_THROW`, so the reader throws and
    /// the writer is a throwing `SetScissorRectangle`.
    func testTheScissorRectangleRoundTrips() throws {
        try requireNative()
        let game = try run { game, device in
            try device.SetScissorRectangle(
                Microsoft.Xna.Framework.Rectangle(4, 8, 100, 50))
            let read = try device.ScissorRectangle
            game.observations["scissor"] =
                "\(read.X),\(read.Y) \(read.Width)x\(read.Height)"
        }
        XCTAssertEqual(game.observations["scissor"], "4,8 100x50")
    }

    /// `GraphicsDeviceStatus` has no setter and three declared values. On the
    /// qualified HEADLESS renderer the device is never lost, so `Normal` is
    /// the only value this environment can produce — which is stated rather
    /// than dressed up as coverage of all three.
    func testTheDeviceReportsItsStatus() throws {
        try requireNative()
        let game = try run { game, device in
            game.observations["status"] =
                String(describing: try device.GraphicsDeviceStatus)
        }
        XCTAssertEqual(game.observations["status"], "Normal")
    }

    /// All three refuse a facade outside the callback that produced it, which
    /// is this binding's analogue of XNA's `Helpers.CheckDisposed`.
    func testTheNewMembersRefuseAStaleFacade() throws {
        try requireNative()
        var escaped: G.GraphicsDevice?
        try run { _, device in escaped = device }
        guard let escaped else { return XCTFail("no device escaped") }
        XCTAssertThrowsError(try escaped.SetViewport(
            Microsoft.Xna.Framework.Graphics.Viewport()))
        XCTAssertThrowsError(try escaped.ScissorRectangle)
        XCTAssertThrowsError(try escaped.SetScissorRectangle(
            Microsoft.Xna.Framework.Rectangle(0, 0, 1, 1)))
        XCTAssertThrowsError(try escaped.GraphicsDeviceStatus)
    }

    // MARK: - The sampler collection

    func testTheSamplerCollectionAppliesThroughTheNativeRoute() throws {
        try requireNative()
        let game = try run { game, device in
            guard let samplers = device.SamplerStates else {
                throw CNAError.producerInvariant("no sampler collection")
            }
            game.observations["default"] = try samplers.Item(0).Name ?? "unnamed"
            try samplers.SetItem(3, G.SamplerState.PointClamp)
            game.observations["applied"] = try samplers.Item(3).Name ?? "unnamed"
            game.observations["untouched"] = try samplers.Item(4).Name ?? "unnamed"
            game.observations["identity"] =
                String(try samplers.Item(3) === G.SamplerState.PointClamp)
            // The collection outlives any one facade and is one object.
            game.observations["sameCollection"] =
                String(device.SamplerStates === samplers)
            game.observations["vertexIsDistinct"] =
                String(device.VertexSamplerStates !== samplers)
        }
        XCTAssertEqual(game.observations["default"], "SamplerState.LinearWrap",
                       "an unwritten slot answers what InitializeDeviceState puts there")
        XCTAssertEqual(game.observations["applied"], "SamplerState.PointClamp")
        XCTAssertEqual(game.observations["untouched"], "SamplerState.LinearWrap")
        XCTAssertEqual(game.observations["identity"], "true")
        XCTAssertEqual(game.observations["sameCollection"], "true")
        XCTAssertEqual(game.observations["vertexIsDistinct"], "true")
    }

    /// The indexer's bounds are observable, and both accessors raise
    /// `ArgumentOutOfRangeException("index")`.
    func testTheIndexerRefusesAnOutOfRangeSlot() throws {
        try requireNative()
        try run { _, device in
            guard let samplers = device.SamplerStates else {
                throw CNAError.producerInvariant("no sampler collection")
            }
            for index: Int32 in [-1, 16] {
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(
                        "Specified argument was out of the range of valid values.",
                        paramName: "index"),
                    paramName: "index",
                    hResult: Int32(bitPattern: 0x8013_1502)
                ) { _ = try samplers.Item(index) }
                assertProjected(
                    CNAArgumentOutOfRangeException.self,
                    message: composedArgumentMessage(
                        "Specified argument was out of the range of valid values.",
                        paramName: "index"),
                    paramName: "index",
                    hResult: Int32(bitPattern: 0x8013_1502)
                ) { try samplers.SetItem(index, G.SamplerState.PointWrap) }
            }
        }
    }

    /// XNA's `set_Item` refuses null with
    /// `ArgumentNullException("value", NullNotAllowed)`. That branch is
    /// **unreachable** here: the writer's parameter is the property's type,
    /// `Item`'s recorded verdict is `UNKNOWN_REFERENCE_NULLABILITY` so the
    /// deferral rule makes it non-Optional, and a non-Optional `SamplerState`
    /// cannot be nil. What is asserted instead is that the message is still
    /// reproduced where it IS reachable — the three device setters, whose
    /// properties are proven nullable and whose parameters are therefore
    /// Optional. `testAllThreeSettersRefuseNull` covers that; this test pins
    /// the reason the collection has no such case.
    func testTheCollectionWriterCannotBeHandedNull() throws {
        try requireNative()
        try run { _, device in
            guard let samplers = device.SamplerStates else {
                throw CNAError.producerInvariant("no sampler collection")
            }
            // A non-Optional parameter: the compiler is the guard, so the only
            // thing to assert is that a real value still round trips.
            try samplers.SetItem(0, G.SamplerState.PointWrap)
            XCTAssertTrue(try samplers.Item(0) === G.SamplerState.PointWrap)
        }
    }
}
