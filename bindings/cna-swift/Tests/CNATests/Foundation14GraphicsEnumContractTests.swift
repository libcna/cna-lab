// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testGraphicsProfileXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.GraphicsProfile
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Reach", .Reach, 0),
            ("HiDef", .HiDef, 1),
        ]

        XCTAssertEqual(contract.count, 2)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testPresentIntervalXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.PresentInterval
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Default", .Default, 0),
            ("One", .One, 1),
            ("Two", .Two, 2),
            ("Immediate", .Immediate, 3),
        ]

        XCTAssertEqual(contract.count, 4)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testVertexElementFormatXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.VertexElementFormat
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Single", .Single, 0),
            ("Vector2", .Vector2, 1),
            ("Vector3", .Vector3, 2),
            ("Vector4", .Vector4, 3),
            ("Color", .Color, 4),
            ("Byte4", .Byte4, 5),
            ("Short2", .Short2, 6),
            ("Short4", .Short4, 7),
            ("NormalizedShort2", .NormalizedShort2, 8),
            ("NormalizedShort4", .NormalizedShort4, 9),
            ("HalfVector2", .HalfVector2, 10),
            ("HalfVector4", .HalfVector4, 11),
        ]

        XCTAssertEqual(contract.count, 12)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testVertexElementUsageXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.VertexElementUsage
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Position", .Position, 0),
            ("Color", .Color, 1),
            ("TextureCoordinate", .TextureCoordinate, 2),
            ("Normal", .Normal, 3),
            ("Binormal", .Binormal, 4),
            ("Tangent", .Tangent, 5),
            ("BlendIndices", .BlendIndices, 6),
            ("BlendWeight", .BlendWeight, 7),
            ("Depth", .Depth, 8),
            ("Fog", .Fog, 9),
            ("PointSize", .PointSize, 10),
            ("Sample", .Sample, 11),
            ("TessellateFactor", .TessellateFactor, 12),
        ]

        XCTAssertEqual(contract.count, 13)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testCompareFunctionXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.CompareFunction
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Always", .Always, 0),
            ("Never", .Never, 1),
            ("Less", .Less, 2),
            ("LessEqual", .LessEqual, 3),
            ("Equal", .Equal, 4),
            ("GreaterEqual", .GreaterEqual, 5),
            ("Greater", .Greater, 6),
            ("NotEqual", .NotEqual, 7),
        ]

        XCTAssertEqual(contract.count, 8)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testCubeMapFaceXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.CubeMapFace
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("PositiveX", .PositiveX, 0),
            ("NegativeX", .NegativeX, 1),
            ("PositiveY", .PositiveY, 2),
            ("NegativeY", .NegativeY, 3),
            ("PositiveZ", .PositiveZ, 4),
            ("NegativeZ", .NegativeZ, 5),
        ]

        XCTAssertEqual(contract.count, 6)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testIndexElementSizeXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.IndexElementSize
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("SixteenBits", .SixteenBits, 0),
            ("ThirtyTwoBits", .ThirtyTwoBits, 1),
        ]

        XCTAssertEqual(contract.count, 2)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testBlendXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.Blend
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("One", .One, 0),
            ("Zero", .Zero, 1),
            ("SourceColor", .SourceColor, 2),
            ("InverseSourceColor", .InverseSourceColor, 3),
            ("SourceAlpha", .SourceAlpha, 4),
            ("InverseSourceAlpha", .InverseSourceAlpha, 5),
            ("DestinationColor", .DestinationColor, 6),
            ("InverseDestinationColor", .InverseDestinationColor, 7),
            ("DestinationAlpha", .DestinationAlpha, 8),
            ("InverseDestinationAlpha", .InverseDestinationAlpha, 9),
            ("BlendFactor", .BlendFactor, 10),
            ("InverseBlendFactor", .InverseBlendFactor, 11),
            ("SourceAlphaSaturation", .SourceAlphaSaturation, 12),
        ]

        XCTAssertEqual(contract.count, 13)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testBlendFunctionXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.BlendFunction
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Add", .Add, 0),
            ("Subtract", .Subtract, 1),
            ("ReverseSubtract", .ReverseSubtract, 2),
            ("Min", .Min, 3),
            ("Max", .Max, 4),
        ]

        XCTAssertEqual(contract.count, 5)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testColorWriteChannelsXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.ColorWriteChannels

        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("None", .None, 0),
            ("Red", .Red, 1),
            ("Green", .Green, 2),
            ("Blue", .Blue, 4),
            ("Alpha", .Alpha, 8),
            ("All", .All, 15),
        ]

        XCTAssertEqual(contract.count, 6)
        for entry in contract {
            XCTAssertEqual(requireInt32OptionSet(entry.value), entry.rawValue, entry.name)
        }
        XCTAssertEqual(E.All.rawValue, E.Red.rawValue | E.Green.rawValue | E.Blue.rawValue | E.Alpha.rawValue)
    }

    func testCullModeXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.CullMode
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("None", .None, 0),
            ("CullClockwiseFace", .CullClockwiseFace, 1),
            ("CullCounterClockwiseFace", .CullCounterClockwiseFace, 2),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testStencilOperationXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.StencilOperation
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Keep", .Keep, 0),
            ("Zero", .Zero, 1),
            ("Replace", .Replace, 2),
            ("Increment", .Increment, 3),
            ("Decrement", .Decrement, 4),
            ("IncrementSaturation", .IncrementSaturation, 5),
            ("DecrementSaturation", .DecrementSaturation, 6),
            ("Invert", .Invert, 7),
        ]

        XCTAssertEqual(contract.count, 8)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testTextureAddressModeXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.TextureAddressMode
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Wrap", .Wrap, 0),
            ("Clamp", .Clamp, 1),
            ("Mirror", .Mirror, 2),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testTextureFilterXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.TextureFilter
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Linear", .Linear, 0),
            ("Point", .Point, 1),
            ("Anisotropic", .Anisotropic, 2),
            ("LinearMipPoint", .LinearMipPoint, 3),
            ("PointMipLinear", .PointMipLinear, 4),
            ("MinLinearMagPointMipLinear", .MinLinearMagPointMipLinear, 5),
            ("MinLinearMagPointMipPoint", .MinLinearMagPointMipPoint, 6),
            ("MinPointMagLinearMipLinear", .MinPointMagLinearMipLinear, 7),
            ("MinPointMagLinearMipPoint", .MinPointMagLinearMipPoint, 8),
        ]

        XCTAssertEqual(contract.count, 9)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testClearOptionsXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.ClearOptions

        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Target", .Target, 1),
            ("DepthBuffer", .DepthBuffer, 2),
            ("Stencil", .Stencil, 4),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(requireInt32OptionSet(entry.value), entry.rawValue, entry.name)
        }
    }

    func testGraphicsDeviceStatusXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.GraphicsDeviceStatus
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Normal", .Normal, 0),
            ("Lost", .Lost, 1),
            ("NotReset", .NotReset, 2),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testPrimitiveTypeXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.PrimitiveType
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("TriangleList", .TriangleList, 0),
            ("TriangleStrip", .TriangleStrip, 1),
            ("LineList", .LineList, 2),
            ("LineStrip", .LineStrip, 3),
        ]

        XCTAssertEqual(contract.count, 4)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testEffectParameterClassXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.EffectParameterClass
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Scalar", .Scalar, 0),
            ("Vector", .Vector, 1),
            ("Matrix", .Matrix, 2),
            ("Object", .Object, 3),
            ("Struct", .Struct, 4),
        ]

        XCTAssertEqual(contract.count, 5)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testEffectParameterTypeXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.EffectParameterType
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Void", .Void, 0),
            ("Bool", .Bool, 1),
            ("Int32", .Int32, 2),
            ("Single", .Single, 3),
            ("String", .String, 4),
            ("Texture", .Texture, 5),
            ("Texture1D", .Texture1D, 6),
            ("Texture2D", .Texture2D, 7),
            ("Texture3D", .Texture3D, 8),
            ("TextureCube", .TextureCube, 9),
        ]

        XCTAssertEqual(contract.count, 10)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testSetDataOptionsXnaContract() {
        typealias E = Microsoft.Xna.Framework.Graphics.SetDataOptions

        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("None", .None, 0),
            ("Discard", .Discard, 1),
            ("NoOverwrite", .NoOverwrite, 2),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(requireInt32OptionSet(entry.value), entry.rawValue, entry.name)
        }
    }
}
