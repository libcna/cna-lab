// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class Foundation14GraphicsEnumProjectionTests: XCTestCase {
    func testGraphicsProfileInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.GraphicsProfile
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Reach),
            (1, .HiDef),
        ]

        XCTAssertEqual(expected.count, 2)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 2))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Reach
        var copy = original
        copy = .HiDef
        XCTAssertEqual(original, .Reach)
        XCTAssertEqual(copy, .HiDef)
    }

    func testPresentIntervalInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.PresentInterval
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Default),
            (1, .One),
            (2, .Two),
            (3, .Immediate),
        ]

        XCTAssertEqual(expected.count, 4)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 4))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Default
        var copy = original
        copy = .One
        XCTAssertEqual(original, .Default)
        XCTAssertEqual(copy, .One)
    }

    func testVertexElementFormatInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.VertexElementFormat
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Single),
            (1, .Vector2),
            (2, .Vector3),
            (3, .Vector4),
            (4, .Color),
            (5, .Byte4),
            (6, .Short2),
            (7, .Short4),
            (8, .NormalizedShort2),
            (9, .NormalizedShort4),
            (10, .HalfVector2),
            (11, .HalfVector4),
        ]

        XCTAssertEqual(expected.count, 12)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 12))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Single
        var copy = original
        copy = .Vector2
        XCTAssertEqual(original, .Single)
        XCTAssertEqual(copy, .Vector2)
    }

    func testVertexElementUsageInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.VertexElementUsage
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Position),
            (1, .Color),
            (2, .TextureCoordinate),
            (3, .Normal),
            (4, .Binormal),
            (5, .Tangent),
            (6, .BlendIndices),
            (7, .BlendWeight),
            (8, .Depth),
            (9, .Fog),
            (10, .PointSize),
            (11, .Sample),
            (12, .TessellateFactor),
        ]

        XCTAssertEqual(expected.count, 13)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 13))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Position
        var copy = original
        copy = .Color
        XCTAssertEqual(original, .Position)
        XCTAssertEqual(copy, .Color)
    }

    func testCompareFunctionInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.CompareFunction
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Always),
            (1, .Never),
            (2, .Less),
            (3, .LessEqual),
            (4, .Equal),
            (5, .GreaterEqual),
            (6, .Greater),
            (7, .NotEqual),
        ]

        XCTAssertEqual(expected.count, 8)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 8))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Always
        var copy = original
        copy = .Never
        XCTAssertEqual(original, .Always)
        XCTAssertEqual(copy, .Never)
    }

    func testCubeMapFaceInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.CubeMapFace
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .PositiveX),
            (1, .NegativeX),
            (2, .PositiveY),
            (3, .NegativeY),
            (4, .PositiveZ),
            (5, .NegativeZ),
        ]

        XCTAssertEqual(expected.count, 6)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 6))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.PositiveX
        var copy = original
        copy = .NegativeX
        XCTAssertEqual(original, .PositiveX)
        XCTAssertEqual(copy, .NegativeX)
    }

    func testIndexElementSizeInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.IndexElementSize
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .SixteenBits),
            (1, .ThirtyTwoBits),
        ]

        XCTAssertEqual(expected.count, 2)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 2))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.SixteenBits
        var copy = original
        copy = .ThirtyTwoBits
        XCTAssertEqual(original, .SixteenBits)
        XCTAssertEqual(copy, .ThirtyTwoBits)
    }

    func testBlendInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.Blend
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .One),
            (1, .Zero),
            (2, .SourceColor),
            (3, .InverseSourceColor),
            (4, .SourceAlpha),
            (5, .InverseSourceAlpha),
            (6, .DestinationColor),
            (7, .InverseDestinationColor),
            (8, .DestinationAlpha),
            (9, .InverseDestinationAlpha),
            (10, .BlendFactor),
            (11, .InverseBlendFactor),
            (12, .SourceAlphaSaturation),
        ]

        XCTAssertEqual(expected.count, 13)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 13))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.One
        var copy = original
        copy = .Zero
        XCTAssertEqual(original, .One)
        XCTAssertEqual(copy, .Zero)
    }

    func testBlendFunctionInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.BlendFunction
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Add),
            (1, .Subtract),
            (2, .ReverseSubtract),
            (3, .Min),
            (4, .Max),
        ]

        XCTAssertEqual(expected.count, 5)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 5))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Add
        var copy = original
        copy = .Subtract
        XCTAssertEqual(original, .Add)
        XCTAssertEqual(copy, .Subtract)
    }

    func testColorWriteChannelsInt32OptionSetProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.ColorWriteChannels

        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        XCTAssertEqual(requireInt32OptionSet(E(rawValue: 0)), 0)
        XCTAssertTrue(E.Red.contains(.Red))
        XCTAssertEqual(E.Red.union(.Green).rawValue, 3)
        XCTAssertEqual(E(rawValue: 3).intersection(.Red).rawValue, 1)
        XCTAssertEqual(E.Red.intersection(.Green).rawValue, 0)

        XCTAssertEqual(E(rawValue: 1 << 20).rawValue, 1 << 20)
        XCTAssertEqual(E(rawValue: -1).rawValue, -1)
        XCTAssertEqual(E(rawValue: Int32.max).rawValue, Int32.max)
        XCTAssertEqual(E(rawValue: Int32.min).rawValue, Int32.min)

        let original = E.Red
        var copy = original
        copy.insert(.Green)
        XCTAssertEqual(original.rawValue, 1)
        XCTAssertEqual(copy.rawValue, 3)
    }

    func testCullModeInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.CullMode
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .None),
            (1, .CullClockwiseFace),
            (2, .CullCounterClockwiseFace),
        ]

        XCTAssertEqual(expected.count, 3)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 3))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.None
        var copy = original
        copy = .CullClockwiseFace
        XCTAssertEqual(original, .None)
        XCTAssertEqual(copy, .CullClockwiseFace)
    }

    func testStencilOperationInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.StencilOperation
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Keep),
            (1, .Zero),
            (2, .Replace),
            (3, .Increment),
            (4, .Decrement),
            (5, .IncrementSaturation),
            (6, .DecrementSaturation),
            (7, .Invert),
        ]

        XCTAssertEqual(expected.count, 8)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 8))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Keep
        var copy = original
        copy = .Zero
        XCTAssertEqual(original, .Keep)
        XCTAssertEqual(copy, .Zero)
    }

    func testTextureAddressModeInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.TextureAddressMode
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Wrap),
            (1, .Clamp),
            (2, .Mirror),
        ]

        XCTAssertEqual(expected.count, 3)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 3))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Wrap
        var copy = original
        copy = .Clamp
        XCTAssertEqual(original, .Wrap)
        XCTAssertEqual(copy, .Clamp)
    }

    func testTextureFilterInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.TextureFilter
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Linear),
            (1, .Point),
            (2, .Anisotropic),
            (3, .LinearMipPoint),
            (4, .PointMipLinear),
            (5, .MinLinearMagPointMipLinear),
            (6, .MinLinearMagPointMipPoint),
            (7, .MinPointMagLinearMipLinear),
            (8, .MinPointMagLinearMipPoint),
        ]

        XCTAssertEqual(expected.count, 9)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 9))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Linear
        var copy = original
        copy = .Point
        XCTAssertEqual(original, .Linear)
        XCTAssertEqual(copy, .Point)
    }

    func testClearOptionsInt32OptionSetProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.ClearOptions

        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        XCTAssertEqual(requireInt32OptionSet(E(rawValue: 0)), 0)
        XCTAssertTrue(E.Target.contains(.Target))
        XCTAssertEqual(E.Target.union(.DepthBuffer).rawValue, 3)
        XCTAssertEqual(E(rawValue: 3).intersection(.Target).rawValue, 1)
        XCTAssertEqual(E.Target.intersection(.DepthBuffer).rawValue, 0)

        XCTAssertEqual(E(rawValue: 1 << 20).rawValue, 1 << 20)
        XCTAssertEqual(E(rawValue: -1).rawValue, -1)
        XCTAssertEqual(E(rawValue: Int32.max).rawValue, Int32.max)
        XCTAssertEqual(E(rawValue: Int32.min).rawValue, Int32.min)

        let original = E.Target
        var copy = original
        copy.insert(.DepthBuffer)
        XCTAssertEqual(original.rawValue, 1)
        XCTAssertEqual(copy.rawValue, 3)
    }

    func testGraphicsDeviceStatusInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.GraphicsDeviceStatus
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Normal),
            (1, .Lost),
            (2, .NotReset),
        ]

        XCTAssertEqual(expected.count, 3)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 3))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Normal
        var copy = original
        copy = .Lost
        XCTAssertEqual(original, .Normal)
        XCTAssertEqual(copy, .Lost)
    }

    func testPrimitiveTypeInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.PrimitiveType
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .TriangleList),
            (1, .TriangleStrip),
            (2, .LineList),
            (3, .LineStrip),
        ]

        XCTAssertEqual(expected.count, 4)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 4))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.TriangleList
        var copy = original
        copy = .TriangleStrip
        XCTAssertEqual(original, .TriangleList)
        XCTAssertEqual(copy, .TriangleStrip)
    }

    func testEffectParameterClassInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.EffectParameterClass
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Scalar),
            (1, .Vector),
            (2, .Matrix),
            (3, .Object),
            (4, .Struct),
        ]

        XCTAssertEqual(expected.count, 5)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 5))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Scalar
        var copy = original
        copy = .Vector
        XCTAssertEqual(original, .Scalar)
        XCTAssertEqual(copy, .Vector)
    }

    func testEffectParameterTypeInt32RawEnumProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.EffectParameterType
        let expected: [(rawValue: Int32, value: E)] = [
            (0, .Void),
            (1, .Bool),
            (2, .Int32),
            (3, .Single),
            (4, .String),
            (5, .Texture),
            (6, .Texture1D),
            (7, .Texture2D),
            (8, .Texture3D),
            (9, .TextureCube),
        ]

        XCTAssertEqual(expected.count, 10)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(E(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(E(rawValue: 10))
        XCTAssertNil(E(rawValue: -1))
        XCTAssertNil(E(rawValue: Int32.max))
        XCTAssertNil(E(rawValue: Int32.min))

        let original = E.Void
        var copy = original
        copy = .Bool
        XCTAssertEqual(original, .Void)
        XCTAssertEqual(copy, .Bool)
    }

    func testSetDataOptionsInt32OptionSetProjectionAndValueSemantics() {
        typealias E = Microsoft.Xna.Framework.Graphics.SetDataOptions

        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        XCTAssertEqual(requireInt32OptionSet(E(rawValue: 0)), 0)
        XCTAssertTrue(E.Discard.contains(.Discard))
        XCTAssertEqual(E.Discard.union(.NoOverwrite).rawValue, 3)
        XCTAssertEqual(E(rawValue: 3).intersection(.Discard).rawValue, 1)
        XCTAssertEqual(E.Discard.intersection(.NoOverwrite).rawValue, 0)

        XCTAssertEqual(E(rawValue: 1 << 20).rawValue, 1 << 20)
        XCTAssertEqual(E(rawValue: -1).rawValue, -1)
        XCTAssertEqual(E(rawValue: Int32.max).rawValue, Int32.max)
        XCTAssertEqual(E(rawValue: Int32.min).rawValue, Int32.min)

        let original = E.Discard
        var copy = original
        copy.insert(.NoOverwrite)
        XCTAssertEqual(original.rawValue, 1)
        XCTAssertEqual(copy.rawValue, 3)
    }
}
