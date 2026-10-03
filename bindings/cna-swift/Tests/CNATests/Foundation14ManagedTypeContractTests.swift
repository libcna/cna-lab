// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testVertexElementXnaContract() {
        typealias Element = Microsoft.Xna.Framework.Graphics.VertexElement
        typealias Format = Microsoft.Xna.Framework.Graphics.VertexElementFormat
        typealias Usage = Microsoft.Xna.Framework.Graphics.VertexElementUsage

        // The constructor stores offset, elementFormat, elementUsage and
        // usageIndex verbatim; the pinned IL validates nothing, so out-of-table
        // offsets and negative indices are stored as given.
        var element = Element(12, .Vector3, .Normal, 3)
        XCTAssertEqual(element.Offset, 12)
        XCTAssertEqual(element.VertexElementFormat, .Vector3)
        XCTAssertEqual(element.VertexElementUsage, .Normal)
        XCTAssertEqual(element.UsageIndex, 3)

        let extreme = Element(Int32.min, .HalfVector4, .TessellateFactor, Int32.max)
        XCTAssertEqual(extreme.Offset, Int32.min)
        XCTAssertEqual(extreme.UsageIndex, Int32.max)

        // Every property accessor is a plain field load/store.
        element.Offset = -4
        element.VertexElementFormat = .Color
        element.VertexElementUsage = .Color
        element.UsageIndex = 2
        XCTAssertEqual(element.Offset, -4)
        XCTAssertEqual(element.VertexElementFormat, .Color)
        XCTAssertEqual(element.VertexElementUsage, .Color)
        XCTAssertEqual(element.UsageIndex, 2)

        // op_Equality compares offset, usageIndex, usage and format; any one
        // difference is enough for inequality, and op_Inequality is its
        // negation.
        let base = Element(0, .Vector3, .Position, 0)
        XCTAssertTrue(base == Element(0, .Vector3, .Position, 0))
        XCTAssertFalse(base != Element(0, .Vector3, .Position, 0))
        for other in [
            Element(1, .Vector3, .Position, 0),
            Element(0, .Vector4, .Position, 0),
            Element(0, .Vector3, .Normal, 0),
            Element(0, .Vector3, .Position, 1),
        ] {
            XCTAssertFalse(base == other)
            XCTAssertTrue(base != other)
        }

        // Equals(object) is false for null and for a different runtime type,
        // and otherwise defers to op_Equality.
        XCTAssertFalse(base.Equals(nil))
        XCTAssertFalse(base.Equals(42))
        XCTAssertTrue(base.Equals(Element(0, .Vector3, .Position, 0)))
        XCTAssertFalse(base.Equals(Element(1, .Vector3, .Position, 0)))

        // Helpers.SmartGetHashCode XORs the four sequential int32 words and
        // substitutes 0x7FFFFFFF when the accumulated XOR is zero.
        XCTAssertEqual(Element(0, .Single, .Position, 0).GetHashCode(), Int32.max)
        XCTAssertEqual(Element(3, .Vector4, .Position, 0).GetHashCode(), Int32.max)
        XCTAssertEqual(Element(1, .Single, .Position, 0).GetHashCode(), 1)
        XCTAssertEqual(Element(0, .Vector3, .Position, 0).GetHashCode(), 2)
        XCTAssertEqual(Element(0, .Single, .Position, 5).GetHashCode(), 5)
        XCTAssertEqual(Element(8, .Vector2, .Color, 4).GetHashCode(), 8 ^ 1 ^ 1 ^ 4)
        XCTAssertEqual(Element(-1, .Single, .Position, 0).GetHashCode(), -1)
        XCTAssertEqual(Element(Int32.min, .Single, .Position, 0).GetHashCode(), Int32.min)

        // string.Format(CultureInfo.CurrentCulture,
        //   "{{Offset:{0} Format:{1} Usage:{2} UsageIndex:{3}}}", ...) with the
        // two boxed enums rendered through Enum.ToString().
        XCTAssertEqual(
            Element(0, .Vector3, .Position, 0).ToString(),
            "{Offset:0 Format:Vector3 Usage:Position UsageIndex:0}")
        XCTAssertEqual(
            Element(-4, .Color, .Color, 2).ToString(),
            "{Offset:-4 Format:Color Usage:Color UsageIndex:2}")
        XCTAssertEqual(
            Element(28, .HalfVector4, .TessellateFactor, 7).ToString(),
            "{Offset:28 Format:HalfVector4 Usage:TessellateFactor UsageIndex:7}")
        XCTAssertEqual(
            Element(16, .NormalizedShort2, .BlendIndices, 1).ToString(),
            "{Offset:16 Format:NormalizedShort2 Usage:BlendIndices UsageIndex:1}")
    }

    func testIEffectFogXnaContract() {
        // The pinned interface declares exactly four abstract read/write
        // properties with these names and types and no method. Three project
        // to ordinary Swift properties. `FogColor` is fallible in every
        // registered implementor -- both accessors forward to
        // `EffectParameter`, which validates and throws -- so its reader is
        // `{ get throws }` and its writer is the projected `SetFogColor`
        // accessor method. That is one CLR member, not two.
        typealias Fog = Microsoft.Xna.Framework.Graphics.IEffectFog
        let read: (Fog) throws -> (Bool, Float, Float, Microsoft.Xna.Framework.Vector3) = {
            ($0.FogEnabled, $0.FogStart, $0.FogEnd, try $0.FogColor)
        }
        let write: (inout Fog, Bool, Float, Float) -> Void = {
            $0.FogEnabled = $1
            $0.FogStart = $2
            $0.FogEnd = $3
        }
        let writeColor: (Fog, Microsoft.Xna.Framework.Vector3) throws -> Void = {
            try $0.SetFogColor($1)
        }
        _ = read
        _ = write
        _ = writeColor
        XCTAssertEqual(String(describing: Fog.self), "IEffectFog")
    }

    func testIEffectMatricesXnaContract() {
        // The pinned interface declares exactly three abstract read/write
        // Matrix properties and no method.
        typealias Matrices = Microsoft.Xna.Framework.Graphics.IEffectMatrices
        let read: (Matrices) -> (Microsoft.Xna.Framework.Matrix,
                                 Microsoft.Xna.Framework.Matrix,
                                 Microsoft.Xna.Framework.Matrix) = {
            ($0.World, $0.View, $0.Projection)
        }
        let write: (inout Matrices, Microsoft.Xna.Framework.Matrix) -> Void = {
            $0.World = $1
            $0.View = $1
            $0.Projection = $1
        }
        _ = read
        _ = write
        XCTAssertEqual(String(describing: Matrices.self), "IEffectMatrices")
    }

    func testSoundStateXnaContract() {
        typealias E = Microsoft.Xna.Framework.Audio.SoundState
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Playing", .Playing, 0),
            ("Paused", .Paused, 1),
            ("Stopped", .Stopped, 2),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }

    func testAudioChannelsXnaContract() {
        typealias E = Microsoft.Xna.Framework.Audio.AudioChannels
        // The pinned table deliberately starts at 1; there is no zero literal
        // and none is invented.
        let contract: [(name: String, value: E, rawValue: Int32)] = [
            ("Mono", .Mono, 1),
            ("Stereo", .Stereo, 2),
        ]

        XCTAssertEqual(contract.count, 2)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }
}
