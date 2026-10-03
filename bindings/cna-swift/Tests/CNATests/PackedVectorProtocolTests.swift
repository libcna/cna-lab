// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testPackedProtocolExactGenericWitnessesAndValueSemantics() {
        typealias P = Framework.Graphics.PackedVector

        func packed8<T>(_ value: T) -> UInt8 where T: P.IPackedVectorOfT, T.TPacked == UInt8 {
            value.PackedValue
        }
        func packed16<T>(_ value: T) -> UInt16 where T: P.IPackedVectorOfT, T.TPacked == UInt16 {
            value.PackedValue
        }
        func packed32<T>(_ value: T) -> UInt32 where T: P.IPackedVectorOfT, T.TPacked == UInt32 {
            value.PackedValue
        }
        func packed64<T>(_ value: T) -> UInt64 where T: P.IPackedVectorOfT, T.TPacked == UInt64 {
            value.PackedValue
        }
        func repack<T>(_ value: inout T, _ vector: Framework.Vector4) where T: P.IPackedVector {
            value.PackFromVector4(vector)
        }
        func expand<T>(_ value: T) -> Framework.Vector4 where T: P.IPackedVector {
            value.ToVector4()
        }

        var alpha = P.Alpha8(0)
        repack(&alpha, Framework.Vector4(9, 8, 7, 1))
        XCTAssertEqual(packed8(alpha), 0xFF)
        XCTAssertEqual(expand(alpha).X.bitPattern, 0)
        XCTAssertEqual(expand(alpha).W.bitPattern, Float(1).bitPattern)

        var bgr = P.Bgr565(0, 0, 0)
        repack(&bgr, Framework.Vector4(1, 0.5, 0.25, 0))
        XCTAssertEqual(packed16(bgr), 0xFC08)
        XCTAssertEqual(expand(bgr).W.bitPattern, Float(1).bitPattern)

        var halfSingle = P.HalfSingle(0)
        repack(&halfSingle, Framework.Vector4(-2, 9, 8, 7))
        XCTAssertEqual(packed16(halfSingle), 0xC000)
        XCTAssertEqual(expand(halfSingle).Y.bitPattern, 0)
        XCTAssertEqual(expand(halfSingle).W.bitPattern, Float(1).bitPattern)

        var half2 = P.HalfVector2(0, 0)
        repack(&half2, Framework.Vector4(1, -2, 9, 8))
        XCTAssertEqual(packed32(half2), 0xC000_3C00)
        XCTAssertEqual(expand(half2).Z.bitPattern, 0)
        XCTAssertEqual(expand(half2).W.bitPattern, Float(1).bitPattern)

        var normalizedByte2 = P.NormalizedByte2(0, 0)
        repack(&normalizedByte2, Framework.Vector4(-1, 1, 9, 8))
        XCTAssertEqual(packed16(normalizedByte2), 0x7F81)
        XCTAssertEqual(expand(normalizedByte2).Z.bitPattern, 0)

        var normalizedShort2 = P.NormalizedShort2(0, 0)
        repack(&normalizedShort2, Framework.Vector4(-1, 1, 9, 8))
        XCTAssertEqual(packed32(normalizedShort2), 0x7FFF_8001)

        var rg = P.Rg32(0, 0)
        repack(&rg, Framework.Vector4(0.5, 1, 9, 8))
        XCTAssertEqual(packed32(rg), 0xFFFF_8000)

        var short2 = P.Short2(0, 0)
        repack(&short2, Framework.Vector4(-32768, 32767, 9, 8))
        XCTAssertEqual(packed32(short2), 0x7FFF_8000)

        XCTAssertEqual(packed16(P.Bgra4444(1, 0, 0, 1)), 0xFF00)
        XCTAssertEqual(packed16(P.Bgra5551(1, 0, 0, 1)), 0xFC00)
        XCTAssertEqual(packed32(P.Byte4(1, 2, 3, 4)), 0x0403_0201)
        XCTAssertEqual(packed64(P.HalfVector4(1, 0, 0, 1)), 0x3C00_0000_0000_3C00)
        XCTAssertEqual(packed32(P.NormalizedByte4(-1, 0, 0, 1)), 0x7F00_0081)
        XCTAssertEqual(packed64(P.NormalizedShort4(-1, 0, 0, 1)), 0x7FFF_0000_0000_8001)
        XCTAssertEqual(packed32(P.Rgba1010102(1, 0, 0, 1)), 0xC000_03FF)
        XCTAssertEqual(packed64(P.Rgba64(1, 0, 0, 1)), 0xFFFF_0000_0000_FFFF)
        XCTAssertEqual(packed64(P.Short4(-32768, 0, 0, 32767)), 0x7FFF_0000_0000_8000)

        let original = P.Rgba64(1, 0.5, 0.25, 0.75)
        var copy = original
        copy.PackedValue = 0
        XCTAssertEqual(original.PackedValue, 0xBFFF_4000_8000_FFFF)
        XCTAssertEqual(copy.PackedValue, 0)
    }

    func testPackedProtocolPublicConvertersMatchWitnessExpansion() {
        typealias P = Framework.Graphics.PackedVector
        func expand<T>(_ value: T) -> Framework.Vector4 where T: P.IPackedVector {
            value.ToVector4()
        }

        let alpha = P.Alpha8(0.25)
        XCTAssertEqual(expand(alpha).W.bitPattern, alpha.ToAlpha().bitPattern)
        let bgr = P.Bgr565(0.25, 0.5, 0.75)
        let bgr3 = bgr.ToVector3()
        XCTAssertEqual(expand(bgr).X.bitPattern, bgr3.X.bitPattern)
        XCTAssertEqual(expand(bgr).Y.bitPattern, bgr3.Y.bitPattern)
        XCTAssertEqual(expand(bgr).Z.bitPattern, bgr3.Z.bitPattern)
        let halfSingle = P.HalfSingle(-2)
        XCTAssertEqual(expand(halfSingle).X.bitPattern, halfSingle.ToSingle().bitPattern)

        let vector2Values: [(Framework.Vector2, Framework.Vector4)] = [
            (P.HalfVector2(1, -2).ToVector2(), expand(P.HalfVector2(1, -2))),
            (P.NormalizedByte2(-1, 1).ToVector2(), expand(P.NormalizedByte2(-1, 1))),
            (P.NormalizedShort2(-1, 1).ToVector2(), expand(P.NormalizedShort2(-1, 1))),
            (P.Rg32(0.25, 0.75).ToVector2(), expand(P.Rg32(0.25, 0.75))),
            (P.Short2(-2, 3).ToVector2(), expand(P.Short2(-2, 3))),
        ]
        for (publicValue, witnessValue) in vector2Values {
            XCTAssertEqual(witnessValue.X.bitPattern, publicValue.X.bitPattern)
            XCTAssertEqual(witnessValue.Y.bitPattern, publicValue.Y.bitPattern)
            XCTAssertEqual(witnessValue.Z.bitPattern, 0)
            XCTAssertEqual(witnessValue.W.bitPattern, Float(1).bitPattern)
        }
    }
}
