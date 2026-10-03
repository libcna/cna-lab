// SPDX-License-Identifier: MIT

import CNAShim

// Conversions between the four XNA state objects and CNA's POD descriptors.
//
// **Every enum crosses through an explicit map, including the eight that
// agree.** CNA and XNA number `BlendFunction` the other way round —
//
//     XNA  Min = int32(0x00000003)   Max = int32(0x00000004)   (pinned IL)
//     CNA  MAX = UINT32_C(3)         MIN = UINT32_C(4)         (graphics_state.h:53-62)
//
// — while `Blend`, `ColorWriteChannels`, `CompareFunction`,
// `StencilOperation`, `CullMode`, `FillMode`, `TextureAddressMode` and
// `TextureFilter` agree value for value. Eight conversions that would work by
// accident and one that would not is the worst possible ratio, so none of them
// is a `rawValue` cast: a future divergence in any of the nine cannot hide.
extension Microsoft.Xna.Framework.Graphics {

    internal enum NativeStateCodes {
        // The one divergent pair. CNA's own header comments name the values:
        // CNA_BLEND_FUNCTION_MAX is "the component-wise maximum" and sits at 3,
        // where XNA's Min sits.
        static func blendFunction(_ value: BlendFunction) -> UInt32 {
            switch value {
            case .Add: return 0
            case .Subtract: return 1
            case .ReverseSubtract: return 2
            case .Min: return 4
            case .Max: return 3
            }
        }

        static func blendFunction(native value: UInt32) -> BlendFunction? {
            switch value {
            case 0: return .Add
            case 1: return .Subtract
            case 2: return .ReverseSubtract
            case 3: return .Max
            case 4: return .Min
            default: return nil
            }
        }

        static func blend(_ value: Blend) -> UInt32 {
            switch value {
            case .One: return 0
            case .Zero: return 1
            case .SourceColor: return 2
            case .InverseSourceColor: return 3
            case .SourceAlpha: return 4
            case .InverseSourceAlpha: return 5
            case .DestinationColor: return 6
            case .InverseDestinationColor: return 7
            case .DestinationAlpha: return 8
            case .InverseDestinationAlpha: return 9
            case .BlendFactor: return 10
            case .InverseBlendFactor: return 11
            case .SourceAlphaSaturation: return 12
            }
        }

        static func blend(native value: UInt32) -> Blend? {
            Blend(rawValue: Int32(value)).flatMap { blend($0) == value ? $0 : nil }
        }

        static func compareFunction(_ value: CompareFunction) -> UInt32 {
            switch value {
            case .Always: return 0
            case .Never: return 1
            case .Less: return 2
            case .LessEqual: return 3
            case .Equal: return 4
            case .GreaterEqual: return 5
            case .Greater: return 6
            case .NotEqual: return 7
            }
        }

        static func compareFunction(native value: UInt32) -> CompareFunction? {
            CompareFunction(rawValue: Int32(value))
                .flatMap { compareFunction($0) == value ? $0 : nil }
        }

        static func stencilOperation(_ value: StencilOperation) -> UInt32 {
            switch value {
            case .Keep: return 0
            case .Zero: return 1
            case .Replace: return 2
            case .Increment: return 3
            case .Decrement: return 4
            case .IncrementSaturation: return 5
            case .DecrementSaturation: return 6
            case .Invert: return 7
            }
        }

        static func stencilOperation(native value: UInt32) -> StencilOperation? {
            StencilOperation(rawValue: Int32(value))
                .flatMap { stencilOperation($0) == value ? $0 : nil }
        }

        static func cullMode(_ value: CullMode) -> UInt32 {
            switch value {
            case .None: return 0
            case .CullClockwiseFace: return 1
            case .CullCounterClockwiseFace: return 2
            }
        }

        static func cullMode(native value: UInt32) -> CullMode? {
            CullMode(rawValue: Int32(value)).flatMap { cullMode($0) == value ? $0 : nil }
        }

        static func fillMode(_ value: FillMode) -> UInt32 {
            switch value {
            case .Solid: return 0
            case .WireFrame: return 1
            }
        }

        static func fillMode(native value: UInt32) -> FillMode? {
            FillMode(rawValue: Int32(value)).flatMap { fillMode($0) == value ? $0 : nil }
        }

        static func textureFilter(_ value: TextureFilter) -> UInt32 {
            switch value {
            case .Linear: return 0
            case .Point: return 1
            case .Anisotropic: return 2
            case .LinearMipPoint: return 3
            case .PointMipLinear: return 4
            case .MinLinearMagPointMipLinear: return 5
            case .MinLinearMagPointMipPoint: return 6
            case .MinPointMagLinearMipLinear: return 7
            case .MinPointMagLinearMipPoint: return 8
            }
        }

        static func textureFilter(native value: UInt32) -> TextureFilter? {
            TextureFilter(rawValue: Int32(value))
                .flatMap { textureFilter($0) == value ? $0 : nil }
        }

        static func addressMode(_ value: TextureAddressMode) -> UInt32 {
            switch value {
            case .Wrap: return 0
            case .Clamp: return 1
            case .Mirror: return 2
            }
        }

        static func addressMode(native value: UInt32) -> TextureAddressMode? {
            TextureAddressMode(rawValue: Int32(value))
                .flatMap { addressMode($0) == value ? $0 : nil }
        }

        // `ColorWriteChannels` is a `[Flags]` OptionSet, so it is a bit set
        // rather than an enumeration and the two sides agree bit for bit —
        // None 0, Red 1, Green 2, Blue 4, Alpha 8, All 15. The round trip is
        // still written out rather than cast, and rejects any bit outside the
        // four the CLR declares.
        static func colorWriteChannels(_ value: ColorWriteChannels) -> UInt32 {
            UInt32(bitPattern: value.rawValue) & 0xF
        }

        static func colorWriteChannels(native value: UInt32) -> ColorWriteChannels? {
            guard value & ~UInt32(0xF) == 0 else { return nil }
            return ColorWriteChannels(rawValue: Int32(value))
        }
    }
}

// The four descriptor conversions. Each writes CNA's own field order, which is
// grouped by width and is NOT the XNA property order; the native ABI verifier
// compares the mirrored structures against the canonical ones field for field,
// so a transposition there is caught, but the assignment below is hand-written
// and has its own mutation.
extension Microsoft.Xna.Framework.Graphics.BlendState {
    internal func nativeDescriptor() -> CNASwift_BlendState {
        typealias Codes = Microsoft.Xna.Framework.Graphics.NativeStateCodes
        var native = CNASwift_BlendState()
        native.struct_size = UInt32(MemoryLayout<CNASwift_BlendState>.size)
        native.struct_version = 1
        native.alpha_blend_function = Codes.blendFunction(AlphaBlendFunction)
        native.alpha_destination_blend = Codes.blend(AlphaDestinationBlend)
        native.alpha_source_blend = Codes.blend(AlphaSourceBlend)
        native.color_blend_function = Codes.blendFunction(ColorBlendFunction)
        native.color_destination_blend = Codes.blend(ColorDestinationBlend)
        native.color_source_blend = Codes.blend(ColorSourceBlend)
        native.color_write_channels = Codes.colorWriteChannels(ColorWriteChannels)
        native.color_write_channels1 = Codes.colorWriteChannels(ColorWriteChannels1)
        native.color_write_channels2 = Codes.colorWriteChannels(ColorWriteChannels2)
        native.color_write_channels3 = Codes.colorWriteChannels(ColorWriteChannels3)
        native.blend_factor = CNASwift_Color(
            r: BlendFactor.R, g: BlendFactor.G, b: BlendFactor.B, a: BlendFactor.A)
        native.multi_sample_mask = MultiSampleMask
        return native
    }
}

extension Microsoft.Xna.Framework.Graphics.DepthStencilState {
    internal func nativeDescriptor() -> CNASwift_DepthStencilState {
        typealias Codes = Microsoft.Xna.Framework.Graphics.NativeStateCodes
        var native = CNASwift_DepthStencilState()
        native.struct_size = UInt32(MemoryLayout<CNASwift_DepthStencilState>.size)
        native.struct_version = 1
        native.depth_buffer_enable = DepthBufferEnable ? 1 : 0
        native.depth_buffer_write_enable = DepthBufferWriteEnable ? 1 : 0
        native.stencil_enable = StencilEnable ? 1 : 0
        native.two_sided_stencil_mode = TwoSidedStencilMode ? 1 : 0
        native.depth_buffer_function = Codes.compareFunction(DepthBufferFunction)
        native.stencil_function = Codes.compareFunction(StencilFunction)
        native.stencil_mask = StencilMask
        native.stencil_write_mask = StencilWriteMask
        native.reference_stencil = ReferenceStencil
        native.stencil_fail = Codes.stencilOperation(StencilFail)
        native.stencil_depth_buffer_fail = Codes.stencilOperation(StencilDepthBufferFail)
        native.stencil_pass = Codes.stencilOperation(StencilPass)
        native.counter_clockwise_stencil_function =
            Codes.compareFunction(CounterClockwiseStencilFunction)
        native.counter_clockwise_stencil_fail =
            Codes.stencilOperation(CounterClockwiseStencilFail)
        native.counter_clockwise_stencil_depth_buffer_fail =
            Codes.stencilOperation(CounterClockwiseStencilDepthBufferFail)
        native.counter_clockwise_stencil_pass =
            Codes.stencilOperation(CounterClockwiseStencilPass)
        return native
    }
}

extension Microsoft.Xna.Framework.Graphics.RasterizerState {
    internal func nativeDescriptor() -> CNASwift_RasterizerState {
        typealias Codes = Microsoft.Xna.Framework.Graphics.NativeStateCodes
        var native = CNASwift_RasterizerState()
        native.struct_size = UInt32(MemoryLayout<CNASwift_RasterizerState>.size)
        native.struct_version = 1
        native.cull_mode = Codes.cullMode(CullMode)
        native.fill_mode = Codes.fillMode(FillMode)
        native.depth_bias = DepthBias
        native.slope_scale_depth_bias = SlopeScaleDepthBias
        native.multi_sample_anti_alias = MultiSampleAntiAlias ? 1 : 0
        native.scissor_test_enable = ScissorTestEnable ? 1 : 0
        return native
    }
}

extension Microsoft.Xna.Framework.Graphics.SamplerState {
    internal func nativeDescriptor() -> CNASwift_SamplerState {
        typealias Codes = Microsoft.Xna.Framework.Graphics.NativeStateCodes
        var native = CNASwift_SamplerState()
        native.struct_size = UInt32(MemoryLayout<CNASwift_SamplerState>.size)
        native.struct_version = 1
        native.address_u = Codes.addressMode(AddressU)
        native.address_v = Codes.addressMode(AddressV)
        native.address_w = Codes.addressMode(AddressW)
        native.filter = Codes.textureFilter(Filter)
        native.max_anisotropy = MaxAnisotropy
        native.max_mip_level = MaxMipLevel
        native.mip_map_level_of_detail_bias = MipMapLevelOfDetailBias
        return native
    }
}

extension Microsoft.Xna.Framework.Graphics.NativeStateCodes {
    /// `ClearOptions` as `CNA_CLEAR_OPTION_*` bits.
    ///
    /// The three declared options are mapped one at a time, like every other
    /// enum crossing this boundary, and `tools/native_abi/probe.c` compiles
    /// the three canonical values so a renumbering upstream is a build
    /// failure rather than a wrong clear. Bits XNA does not declare are
    /// carried across untouched: XNA passes the raw word to D3D9 and lets the
    /// driver refuse it, and CNA refuses it the same way — measured, in
    /// `build-probe/f48_clear.c`, as `CNA_RESULT_INVALID_ARGUMENT` for bit
    /// 0x10. Dropping them would turn a clear XNA fails into one that
    /// silently succeeds.
    static func clearOptions(
        _ value: Microsoft.Xna.Framework.Graphics.ClearOptions
    ) -> UInt32 {
        var native: UInt32 = 0
        if value.contains(.Target) { native |= 1 }
        if value.contains(.DepthBuffer) { native |= 2 }
        if value.contains(.Stencil) { native |= 4 }
        let declared: Int32 = 1 | 2 | 4
        native |= UInt32(bitPattern: value.rawValue & ~declared)
        return native
    }
}

/// `FrameworkResources.CannotClearNullDepth`, pinned from the embedded string
/// table of the registered `Microsoft.Xna.Framework.dll`.
internal let cannotClearNullDepthMessage =
    "Cannot clear depth or stencil because the device does not have an "
    + "active depth or stencil buffer."

/// `FrameworkResources.ViewportInvalid`, pinned from the embedded string table
/// of the registered `Microsoft.Xna.Framework.dll`.
internal let viewportInvalidMessage =
    "The viewport is invalid. The viewport cannot be larger than or outside "
    + "of the current render target bounds. The MinDepth and MaxDepth must be "
    + "between 0 and 1."

/// `FrameworkResources.ScissorInvalid`, from the same table.
internal let scissorInvalidMessage =
    "The scissor rectangle is invalid. The scissor rectangle cannot be larger "
    + "than or outside of the current render target bounds."
