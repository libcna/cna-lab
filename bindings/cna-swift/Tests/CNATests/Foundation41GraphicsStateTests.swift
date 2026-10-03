// SPDX-License-Identifier: MIT

import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

/// A user subclass, which exists to pin two separate facts: `BlendState` is
/// derivable in XNA (`sealed=False`), and `ThrowIfBound` names the **declaring**
/// class rather than the dynamic one.
private final class TintedBlendState: G.BlendState {}

/// Foundation 41: the four XNA graphics state objects.
///
/// Every default and every preset value below is read out of the pinned
/// `Microsoft.Xna.Framework.Graphics.dll` IL — `SetDefaults()` for the
/// defaults, `.cctor()` plus the private presetting constructor for the
/// presets. No value here comes from CNA, from FNA, or from MonoGame.
final class Foundation41GraphicsStateTests: XCTestCase {

    // MARK: - BlendState

    func testBlendStateDefaultsAreTheOnesSetDefaultsWrites() {
        let state = G.BlendState()
        XCTAssertEqual(state.ColorSourceBlend, .One)
        XCTAssertEqual(state.ColorDestinationBlend, .Zero)
        XCTAssertEqual(state.ColorBlendFunction, .Add)
        XCTAssertEqual(state.AlphaSourceBlend, .One)
        XCTAssertEqual(state.AlphaDestinationBlend, .Zero)
        XCTAssertEqual(state.AlphaBlendFunction, .Add)
        XCTAssertEqual(state.ColorWriteChannels, .All)
        XCTAssertEqual(state.ColorWriteChannels1, .All)
        XCTAssertEqual(state.ColorWriteChannels2, .All)
        XCTAssertEqual(state.ColorWriteChannels3, .All)
        XCTAssertEqual(state.BlendFactor.PackedValue, F.Color.White.PackedValue)
        XCTAssertEqual(state.MultiSampleMask, -1)
        XCTAssertNil(state.Name)
        XCTAssertFalse(state.IsDisposed)
    }

    func testBlendStatePresetsCarryTheCctorValues() {
        // The private `.ctor(sourceBlend, destinationBlend, name)` writes the
        // same pair onto BOTH the colour and the alpha channel.
        for (state, source, destination, name) in [
            (G.BlendState.Opaque, G.Blend.One, G.Blend.Zero, "BlendState.Opaque"),
            (G.BlendState.AlphaBlend, .One, .InverseSourceAlpha, "BlendState.AlphaBlend"),
            (G.BlendState.Additive, .SourceAlpha, .One, "BlendState.Additive"),
            (G.BlendState.NonPremultiplied, .SourceAlpha, .InverseSourceAlpha,
             "BlendState.NonPremultiplied"),
        ] as [(G.BlendState, G.Blend, G.Blend, String)] {
            XCTAssertEqual(state.ColorSourceBlend, source, name)
            XCTAssertEqual(state.ColorDestinationBlend, destination, name)
            XCTAssertEqual(state.AlphaSourceBlend, source, name)
            XCTAssertEqual(state.AlphaDestinationBlend, destination, name)
            XCTAssertEqual(state.Name, name)
            // Everything the presetting constructor does not touch keeps the
            // value `SetDefaults` wrote.
            XCTAssertEqual(state.ColorBlendFunction, .Add, name)
            XCTAssertEqual(state.AlphaBlendFunction, .Add, name)
            XCTAssertEqual(state.ColorWriteChannels, .All, name)
            XCTAssertEqual(state.MultiSampleMask, -1, name)
            XCTAssertEqual(state.BlendFactor.PackedValue, F.Color.White.PackedValue, name)
        }
    }

    func testBlendStateWritersRoundTrip() throws {
        let state = G.BlendState()
        try state.SetColorSourceBlend(.DestinationColor)
        try state.SetColorDestinationBlend(.BlendFactor)
        try state.SetColorBlendFunction(.ReverseSubtract)
        try state.SetAlphaSourceBlend(.SourceAlphaSaturation)
        try state.SetAlphaDestinationBlend(.InverseBlendFactor)
        try state.SetAlphaBlendFunction(.Max)
        try state.SetColorWriteChannels(.Red)
        try state.SetColorWriteChannels1(.Green)
        try state.SetColorWriteChannels2(.Blue)
        try state.SetColorWriteChannels3(.Alpha)
        try state.SetBlendFactor(F.Color.CornflowerBlue)
        try state.SetMultiSampleMask(0x0F0F)
        XCTAssertEqual(state.ColorSourceBlend, .DestinationColor)
        XCTAssertEqual(state.ColorDestinationBlend, .BlendFactor)
        XCTAssertEqual(state.ColorBlendFunction, .ReverseSubtract)
        XCTAssertEqual(state.AlphaSourceBlend, .SourceAlphaSaturation)
        XCTAssertEqual(state.AlphaDestinationBlend, .InverseBlendFactor)
        XCTAssertEqual(state.AlphaBlendFunction, .Max)
        XCTAssertEqual(state.ColorWriteChannels, .Red)
        XCTAssertEqual(state.ColorWriteChannels1, .Green)
        XCTAssertEqual(state.ColorWriteChannels2, .Blue)
        XCTAssertEqual(state.ColorWriteChannels3, .Alpha)
        XCTAssertEqual(state.BlendFactor.PackedValue, F.Color.CornflowerBlue.PackedValue)
        XCTAssertEqual(state.MultiSampleMask, 0x0F0F)
    }

    // MARK: - DepthStencilState

    func testDepthStencilStateDefaultsAreTheOnesSetDefaultsWrites() {
        let state = G.DepthStencilState()
        XCTAssertTrue(state.DepthBufferEnable)
        XCTAssertTrue(state.DepthBufferWriteEnable)
        XCTAssertEqual(state.DepthBufferFunction, .LessEqual)
        XCTAssertFalse(state.StencilEnable)
        XCTAssertEqual(state.StencilFunction, .Always)
        XCTAssertEqual(state.StencilPass, .Keep)
        XCTAssertEqual(state.StencilFail, .Keep)
        XCTAssertEqual(state.StencilDepthBufferFail, .Keep)
        XCTAssertFalse(state.TwoSidedStencilMode)
        XCTAssertEqual(state.CounterClockwiseStencilFunction, .Always)
        XCTAssertEqual(state.CounterClockwiseStencilPass, .Keep)
        XCTAssertEqual(state.CounterClockwiseStencilFail, .Keep)
        XCTAssertEqual(state.CounterClockwiseStencilDepthBufferFail, .Keep)
        XCTAssertEqual(state.StencilMask, -1)
        XCTAssertEqual(state.StencilWriteMask, -1)
        XCTAssertEqual(state.ReferenceStencil, 0)
        XCTAssertNil(state.Name)
    }

    func testDepthStencilStatePresetsCarryTheCctorValues() {
        // `.ctor(depthBufferEnable, depthBufferWriteEnable, name)`.
        for (state, enable, write, name) in [
            (G.DepthStencilState.None, false, false, "DepthStencilState.None"),
            (G.DepthStencilState.Default, true, true, "DepthStencilState.Default"),
            (G.DepthStencilState.DepthRead, true, false, "DepthStencilState.DepthRead"),
        ] as [(G.DepthStencilState, Bool, Bool, String)] {
            XCTAssertEqual(state.DepthBufferEnable, enable, name)
            XCTAssertEqual(state.DepthBufferWriteEnable, write, name)
            XCTAssertEqual(state.Name, name)
            XCTAssertEqual(state.DepthBufferFunction, .LessEqual, name)
            XCTAssertFalse(state.StencilEnable, name)
            XCTAssertEqual(state.StencilMask, -1, name)
            XCTAssertEqual(state.StencilWriteMask, -1, name)
        }
    }

    func testDepthStencilStateWritersRoundTrip() throws {
        let state = G.DepthStencilState()
        try state.SetDepthBufferEnable(false)
        try state.SetDepthBufferWriteEnable(false)
        try state.SetDepthBufferFunction(.GreaterEqual)
        try state.SetStencilEnable(true)
        try state.SetStencilFunction(.NotEqual)
        try state.SetStencilPass(.Increment)
        try state.SetStencilFail(.Decrement)
        try state.SetStencilDepthBufferFail(.Invert)
        try state.SetTwoSidedStencilMode(true)
        try state.SetCounterClockwiseStencilFunction(.Less)
        try state.SetCounterClockwiseStencilPass(.Replace)
        try state.SetCounterClockwiseStencilFail(.Zero)
        try state.SetCounterClockwiseStencilDepthBufferFail(.IncrementSaturation)
        try state.SetStencilMask(0x00FF)
        try state.SetStencilWriteMask(0xFF00)
        try state.SetReferenceStencil(7)
        XCTAssertFalse(state.DepthBufferEnable)
        XCTAssertFalse(state.DepthBufferWriteEnable)
        XCTAssertEqual(state.DepthBufferFunction, .GreaterEqual)
        XCTAssertTrue(state.StencilEnable)
        XCTAssertEqual(state.StencilFunction, .NotEqual)
        XCTAssertEqual(state.StencilPass, .Increment)
        XCTAssertEqual(state.StencilFail, .Decrement)
        XCTAssertEqual(state.StencilDepthBufferFail, .Invert)
        XCTAssertTrue(state.TwoSidedStencilMode)
        XCTAssertEqual(state.CounterClockwiseStencilFunction, .Less)
        XCTAssertEqual(state.CounterClockwiseStencilPass, .Replace)
        XCTAssertEqual(state.CounterClockwiseStencilFail, .Zero)
        XCTAssertEqual(state.CounterClockwiseStencilDepthBufferFail, .IncrementSaturation)
        XCTAssertEqual(state.StencilMask, 0x00FF)
        XCTAssertEqual(state.StencilWriteMask, 0xFF00)
        XCTAssertEqual(state.ReferenceStencil, 7)
    }

    // MARK: - RasterizerState

    func testRasterizerStateDefaultsAreTheOnesSetDefaultsWrites() {
        let state = G.RasterizerState()
        // The one default that is easy to get wrong: XNA turns multisample
        // antialiasing ON by default, and culls counter-clockwise faces.
        XCTAssertEqual(state.CullMode, .CullCounterClockwiseFace)
        XCTAssertEqual(state.FillMode, .Solid)
        XCTAssertFalse(state.ScissorTestEnable)
        XCTAssertTrue(state.MultiSampleAntiAlias)
        XCTAssertEqual(state.DepthBias, 0)
        XCTAssertEqual(state.SlopeScaleDepthBias, 0)
        XCTAssertNil(state.Name)
    }

    func testRasterizerStatePresetsCarryTheCctorValues() {
        // `.ctor(cullMode, name)`.
        for (state, cull, name) in [
            (G.RasterizerState.CullNone, G.CullMode.None, "RasterizerState.CullNone"),
            (G.RasterizerState.CullClockwise, .CullClockwiseFace,
             "RasterizerState.CullClockwise"),
            (G.RasterizerState.CullCounterClockwise, .CullCounterClockwiseFace,
             "RasterizerState.CullCounterClockwise"),
        ] as [(G.RasterizerState, G.CullMode, String)] {
            XCTAssertEqual(state.CullMode, cull, name)
            XCTAssertEqual(state.Name, name)
            XCTAssertEqual(state.FillMode, .Solid, name)
            XCTAssertTrue(state.MultiSampleAntiAlias, name)
            XCTAssertFalse(state.ScissorTestEnable, name)
        }
    }

    func testRasterizerStateWritersRoundTrip() throws {
        let state = G.RasterizerState()
        try state.SetCullMode(.None)
        try state.SetFillMode(.WireFrame)
        try state.SetScissorTestEnable(true)
        try state.SetMultiSampleAntiAlias(false)
        try state.SetDepthBias(0.25)
        try state.SetSlopeScaleDepthBias(-1.5)
        XCTAssertEqual(state.CullMode, .None)
        XCTAssertEqual(state.FillMode, .WireFrame)
        XCTAssertTrue(state.ScissorTestEnable)
        XCTAssertFalse(state.MultiSampleAntiAlias)
        XCTAssertEqual(state.DepthBias, 0.25)
        XCTAssertEqual(state.SlopeScaleDepthBias, -1.5)
    }

    // MARK: - SamplerState

    func testSamplerStateDefaultsAreTheOnesSetDefaultsWrites() {
        let state = G.SamplerState()
        XCTAssertEqual(state.Filter, .Linear)
        XCTAssertEqual(state.AddressU, .Wrap)
        XCTAssertEqual(state.AddressV, .Wrap)
        XCTAssertEqual(state.AddressW, .Wrap)
        XCTAssertEqual(state.MaxAnisotropy, 4)
        XCTAssertEqual(state.MaxMipLevel, 0)
        XCTAssertEqual(state.MipMapLevelOfDetailBias, 0)
        XCTAssertNil(state.Name)
    }

    func testSamplerStatePresetsCarryTheCctorValues() {
        // `.ctor(filter, addressMode, name)` writes ONE address mode onto all
        // three axes.
        for (state, filter, address, name) in [
            (G.SamplerState.PointWrap, G.TextureFilter.Point,
             G.TextureAddressMode.Wrap, "SamplerState.PointWrap"),
            (G.SamplerState.PointClamp, .Point, .Clamp, "SamplerState.PointClamp"),
            (G.SamplerState.LinearWrap, .Linear, .Wrap, "SamplerState.LinearWrap"),
            (G.SamplerState.LinearClamp, .Linear, .Clamp, "SamplerState.LinearClamp"),
            (G.SamplerState.AnisotropicWrap, .Anisotropic, .Wrap,
             "SamplerState.AnisotropicWrap"),
            (G.SamplerState.AnisotropicClamp, .Anisotropic, .Clamp,
             "SamplerState.AnisotropicClamp"),
        ] as [(G.SamplerState, G.TextureFilter, G.TextureAddressMode, String)] {
            XCTAssertEqual(state.Filter, filter, name)
            XCTAssertEqual(state.AddressU, address, name)
            XCTAssertEqual(state.AddressV, address, name)
            XCTAssertEqual(state.AddressW, address, name)
            XCTAssertEqual(state.Name, name)
            XCTAssertEqual(state.MaxAnisotropy, 4, name)
            XCTAssertEqual(state.MaxMipLevel, 0, name)
            XCTAssertEqual(state.MipMapLevelOfDetailBias, 0, name)
        }
    }

    func testSamplerStateWritersRoundTrip() throws {
        let state = G.SamplerState()
        try state.SetFilter(.MinLinearMagPointMipLinear)
        try state.SetAddressU(.Mirror)
        try state.SetAddressV(.Clamp)
        try state.SetAddressW(.Mirror)
        try state.SetMaxAnisotropy(16)
        try state.SetMaxMipLevel(3)
        try state.SetMipMapLevelOfDetailBias(0.5)
        XCTAssertEqual(state.Filter, .MinLinearMagPointMipLinear)
        XCTAssertEqual(state.AddressU, .Mirror)
        XCTAssertEqual(state.AddressV, .Clamp)
        XCTAssertEqual(state.AddressW, .Mirror)
        XCTAssertEqual(state.MaxAnisotropy, 16)
        XCTAssertEqual(state.MaxMipLevel, 3)
        XCTAssertEqual(state.MipMapLevelOfDetailBias, 0.5)
    }

    // MARK: - ThrowIfBound

    /// The message template is the assembly's own `BoundStateObject`, pinned in
    /// `xna40-selected-resource-strings.json`, with the class's simple name
    /// substituted into BOTH placeholders.
    private func boundMessage(_ simpleName: String) -> String {
        "Cannot change read-only \(simpleName). State objects become "
        + "read-only the first time they are bound to a GraphicsDevice. To "
        + "change property values, create a new \(simpleName) instance."
    }

    func testEverySetterOnABoundStateRaisesInvalidOperation() {
        let blend = G.BlendState()
        blend.markBound()
        assertProjected(
            CNAInvalidOperationException.self,
            message: boundMessage("BlendState"), hResult: Int32(bitPattern: 0x80131509)
        ) { try blend.SetColorSourceBlend(.Zero) }
        assertProjected(
            CNAInvalidOperationException.self,
            message: boundMessage("BlendState"), hResult: Int32(bitPattern: 0x80131509)
        ) { try blend.SetMultiSampleMask(0) }

        let depth = G.DepthStencilState()
        depth.markBound()
        assertProjected(
            CNAInvalidOperationException.self,
            message: boundMessage("DepthStencilState"),
            hResult: Int32(bitPattern: 0x80131509)
        ) { try depth.SetStencilEnable(true) }

        let raster = G.RasterizerState()
        raster.markBound()
        assertProjected(
            CNAInvalidOperationException.self,
            message: boundMessage("RasterizerState"),
            hResult: Int32(bitPattern: 0x80131509)
        ) { try raster.SetCullMode(.None) }

        let sampler = G.SamplerState()
        sampler.markBound()
        assertProjected(
            CNAInvalidOperationException.self,
            message: boundMessage("SamplerState"), hResult: Int32(bitPattern: 0x80131509)
        ) { try sampler.SetFilter(.Point) }
    }

    /// XNA's `ThrowIfBound` loads `ldtoken <declaring class>`, not `GetType()`,
    /// so a subclass still reports the base name. This test is the reason the
    /// projection carries a static `boundStateTypeName` rather than reading the
    /// dynamic type.
    func testBoundMessageNamesTheDeclaringClassNotTheDynamicOne() {
        let tinted = TintedBlendState()
        tinted.markBound()
        assertProjected(
            CNAInvalidOperationException.self,
            message: boundMessage("BlendState"), hResult: Int32(bitPattern: 0x80131509)
        ) { try tinted.SetColorSourceBlend(.Zero) }
    }

    /// The single most consequential fact in this milestone. Each presetting
    /// constructor's last instruction is `ldc.i4.1; stfld bool …::isBound`, so
    /// the static presets are **born bound**: they are process-wide shared
    /// objects that no consumer can mutate. A projection that left them
    /// writable would let one caller silently change `BlendState.Opaque` for
    /// every other caller in the process.
    func testEveryStaticPresetIsBornBoundAndThereforeReadOnly() throws {
        for state in [G.BlendState.Opaque, .AlphaBlend, .Additive, .NonPremultiplied] {
            XCTAssertTrue(state.isBound, state.Name ?? "")
            assertProjected(
                CNAInvalidOperationException.self,
                message: boundMessage("BlendState"),
                hResult: Int32(bitPattern: 0x80131509)
            ) { try state.SetColorSourceBlend(.Zero) }
        }
        for state in [G.DepthStencilState.None, .Default, .DepthRead] {
            XCTAssertTrue(state.isBound, state.Name ?? "")
            assertProjected(
                CNAInvalidOperationException.self,
                message: boundMessage("DepthStencilState"),
                hResult: Int32(bitPattern: 0x80131509)
            ) { try state.SetDepthBufferEnable(false) }
        }
        for state in [G.RasterizerState.CullNone, .CullClockwise, .CullCounterClockwise] {
            XCTAssertTrue(state.isBound, state.Name ?? "")
            assertProjected(
                CNAInvalidOperationException.self,
                message: boundMessage("RasterizerState"),
                hResult: Int32(bitPattern: 0x80131509)
            ) { try state.SetFillMode(.WireFrame) }
        }
        for state in [G.SamplerState.PointWrap, .PointClamp, .LinearWrap,
                      .LinearClamp, .AnisotropicWrap, .AnisotropicClamp] {
            XCTAssertTrue(state.isBound, state.Name ?? "")
            assertProjected(
                CNAInvalidOperationException.self,
                message: boundMessage("SamplerState"),
                hResult: Int32(bitPattern: 0x80131509)
            ) { try state.SetMaxAnisotropy(1) }
        }
    }

    /// A freshly constructed state is NOT bound: the parameterless constructor
    /// ends with `isBound = false`, which is the other half of the same fact.
    func testAFreshlyConstructedStateIsNotBound() {
        XCTAssertFalse(G.BlendState().isBound)
        XCTAssertFalse(G.DepthStencilState().isBound)
        XCTAssertFalse(G.RasterizerState().isBound)
        XCTAssertFalse(G.SamplerState().isBound)
    }

    /// `GraphicsResource.set_Name` is on the base and knows nothing about
    /// `isBound`, so XNA lets a caller rename even a bound preset. Renaming
    /// one here would leak into every other test, so the assertion is made on
    /// a state this test binds itself.
    func testBindingDoesNotFreezeTheInheritedNameSetter() {
        let state = G.SamplerState()
        state.markBound()
        state.Name = "renamed while bound"
        XCTAssertEqual(state.Name, "renamed while bound")
    }

    func testAnUnboundStateAcceptsWrites() throws {
        let state = G.BlendState()
        XCTAssertFalse(state.isBound)
        try state.SetMultiSampleMask(3)
        XCTAssertEqual(state.MultiSampleMask, 3)
    }

    // MARK: - Disposal and shape

    func testStateObjectsDisposeWithoutNativeStorageAndAreIdempotent() throws {
        let state = G.SamplerState()
        XCTAssertFalse(state.IsDisposed)
        var disposedRaises = 0
        _ = state.Disposing.Add { _, _ in disposedRaises += 1 }
        try state.Dispose()
        XCTAssertTrue(state.IsDisposed)
        XCTAssertEqual(disposedRaises, 1)
        try state.Dispose()
        XCTAssertTrue(state.IsDisposed)
        XCTAssertEqual(disposedRaises, 1, "Dispose is idempotent")
    }

    func testStateObjectsCarryTheGraphicsResourceSurface() throws {
        let state = G.RasterizerState()
        XCTAssertNil(state.GraphicsDevice)
        XCTAssertNil(state.Tag)
        state.Name = "custom"
        XCTAssertEqual(state.Name, "custom")
        XCTAssertEqual(state.ToString(), "custom")
    }

    /// The four state classes derive from `GraphicsResource` directly, with no
    /// invented intermediate public type between them.
    ///
    /// The instances are erased to `Any` first: a statically-known `is` test
    /// is folded away by the compiler and asserts nothing, so the check has to
    /// be a real dynamic cast. The **direct** base is asserted by the
    /// verifier's `BASE_MAPPING_MISMATCH`, which is where the shape lives;
    /// what this pins is that a `GraphicsResource` reference genuinely reaches
    /// each of them and that nothing has slipped into the texture hierarchy.
    func testStateObjectsDeriveDirectlyFromGraphicsResource() {
        for state in [G.BlendState() as Any, G.DepthStencilState() as Any,
                      G.RasterizerState() as Any, G.SamplerState() as Any] {
            XCTAssertTrue(state is G.GraphicsResource)
            XCTAssertFalse(state is G.Texture)
            XCTAssertFalse(state is G.Texture2D)
        }
    }

    func testGraphicsDeviceManagerDefaultBackBufferConstants() {
        // `public static initonly int32`, assigned 0x320 and 0x1e0 by the
        // class constructor.
        XCTAssertEqual(F.GraphicsDeviceManager.DefaultBackBufferWidth, 800)
        XCTAssertEqual(F.GraphicsDeviceManager.DefaultBackBufferHeight, 480)
    }
}
