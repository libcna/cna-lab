// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testPackedByteRawDomainBankersRoundingAndNonFiniteValues() {
        typealias Byte4 = Framework.Graphics.PackedVector.Byte4
        XCTAssertEqual(Byte4(-1, 0.5, 127.5, 256).PackedValue, 0xFF80_0000)
        XCTAssertEqual(Byte4(Framework.Vector4(-1, 0.5, 127.5, 256)).PackedValue, 0xFF80_0000)
        XCTAssertEqual(Byte4(0.5, 1.5, 2.5, 255.5).PackedValue, 0xFF02_0200)
        XCTAssertEqual(Byte4(.nan, .infinity, -.infinity, 255).PackedValue, 0xFF00_FF00)

        var value = Byte4(0, 0, 0, 0)
        value.PackedValue = 0x80FF_017F
        XCTAssertEqual(value.PackedValue, 0x80FF_017F)
        let decoded = value.ToVector4()
        XCTAssertEqual(decoded.X.bitPattern, Float(127).bitPattern)
        XCTAssertEqual(decoded.Y.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(decoded.Z.bitPattern, Float(255).bitPattern)
        XCTAssertEqual(decoded.W.bitPattern, Float(128).bitPattern)
        XCTAssertEqual(value.ToString(), "80FF017F")
        XCTAssertEqual(value.GetHashCode(), Int32(bitPattern: 0x80FF_017F))
        XCTAssertTrue(value.Equals(value as Any))
        XCTAssertFalse(value.Equals(nil))
        XCTAssertTrue(value == value)
    }

    func testPackedRgRgbaIndependentBitLayoutsRoundingAndDecode() {
        typealias P = Framework.Graphics.PackedVector
        let rg = P.Rg32(0.5, 1)
        XCTAssertEqual(rg.PackedValue, 0xFFFF_8000)
        XCTAssertEqual(P.Rg32(Framework.Vector2(0.5, 1)).PackedValue, 0xFFFF_8000)
        XCTAssertEqual(P.Rg32(.nan, .infinity).PackedValue, 0xFFFF_0000)
        XCTAssertEqual(P.Rg32(-.infinity, 2).PackedValue, 0xFFFF_0000)
        XCTAssertEqual(rg.ToVector2().X.bitPattern, (Float(32_768) / Float(65_535)).bitPattern)
        XCTAssertEqual(rg.ToVector2().Y.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(rg.ToString(), "FFFF8000")

        let rgba10 = P.Rgba1010102(1, 0.5, 0.25, 0.5)
        XCTAssertEqual(rgba10.PackedValue, 0x9008_03FF)
        XCTAssertEqual(P.Rgba1010102(Framework.Vector4(1, 0.5, 0.25, 0.5)).PackedValue, 0x9008_03FF)
        XCTAssertEqual(P.Rgba1010102(1, 0, 0, 0).PackedValue, 0x0000_03FF)
        XCTAssertEqual(P.Rgba1010102(0, 1, 0, 0).PackedValue, 0x000F_FC00)
        XCTAssertEqual(P.Rgba1010102(0, 0, 1, 0).PackedValue, 0x3FF0_0000)
        XCTAssertEqual(P.Rgba1010102(0, 0, 0, 1).PackedValue, 0xC000_0000)
        XCTAssertEqual(P.Rgba1010102(.nan, .infinity, -.infinity, .nan).PackedValue, 0x000F_FC00)
        let decoded10 = rgba10.ToVector4()
        XCTAssertEqual(decoded10.X.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(decoded10.Y.bitPattern, (Float(512) / Float(1023)).bitPattern)
        XCTAssertEqual(decoded10.Z.bitPattern, (Float(256) / Float(1023)).bitPattern)
        XCTAssertEqual(decoded10.W.bitPattern, (Float(2) / Float(3)).bitPattern)
        XCTAssertEqual(rgba10.ToString(), "900803FF")
        XCTAssertEqual(rgba10.GetHashCode(), Int32(bitPattern: 0x9008_03FF))

        let rgba64 = P.Rgba64(1, 0.5, 0.25, 0.75)
        XCTAssertEqual(rgba64.PackedValue, 0xBFFF_4000_8000_FFFF)
        XCTAssertEqual(P.Rgba64(Framework.Vector4(1, 0.5, 0.25, 0.75)).PackedValue, 0xBFFF_4000_8000_FFFF)
        XCTAssertEqual(P.Rgba64(1, 0, 0, 0).PackedValue, 0x0000_0000_0000_FFFF)
        XCTAssertEqual(P.Rgba64(0, 1, 0, 0).PackedValue, 0x0000_0000_FFFF_0000)
        XCTAssertEqual(P.Rgba64(0, 0, 1, 0).PackedValue, 0x0000_FFFF_0000_0000)
        XCTAssertEqual(P.Rgba64(0, 0, 0, 1).PackedValue, 0xFFFF_0000_0000_0000)
        XCTAssertEqual(P.Rgba64(.nan, .infinity, -.infinity, 2).PackedValue, 0xFFFF_0000_FFFF_0000)
        let decoded64 = rgba64.ToVector4()
        XCTAssertEqual(decoded64.X.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(decoded64.Y.bitPattern, (Float(32_768) / Float(65_535)).bitPattern)
        XCTAssertEqual(decoded64.Z.bitPattern, (Float(16_384) / Float(65_535)).bitPattern)
        XCTAssertEqual(decoded64.W.bitPattern, (Float(49_151) / Float(65_535)).bitPattern)
        XCTAssertEqual(rgba64.ToString(), "BFFF40008000FFFF")
        XCTAssertEqual(rgba64.GetHashCode(), Int32(bitPattern: 0x3FFF_BFFF))
        XCTAssertTrue(rgba64.Equals(P.Rgba64(1, 0.5, 0.25, 0.75)))
        XCTAssertFalse(rgba64.Equals(rgba10 as Any))
    }

    func testPackedRgRgbaThresholdNeighborsAndDirectAssignment() {
        typealias P = Framework.Graphics.PackedVector
        let belowHalf = Float(bitPattern: 0x3EFF_FFFE)
        let exactHalf = Float(bitPattern: 0x3F00_0000)
        let aboveHalf = Float(bitPattern: 0x3F00_0001)
        XCTAssertEqual(P.Rg32(belowHalf, 0).PackedValue, 0x0000_7FFF)
        XCTAssertEqual(P.Rg32(exactHalf, 0).PackedValue, 0x0000_8000)
        XCTAssertEqual(P.Rg32(aboveHalf, 0).PackedValue, 0x0000_8000)
        XCTAssertEqual(P.Rgba1010102(0, 0, 0, exactHalf).PackedValue, 0x8000_0000)

        var rgba10 = P.Rgba1010102(0, 0, 0, 0)
        rgba10.PackedValue = 0xFFFF_FFFF
        XCTAssertEqual(rgba10.PackedValue, 0xFFFF_FFFF)
        XCTAssertEqual(rgba10.ToVector4().X.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(rgba10.ToVector4().W.bitPattern, Float(1).bitPattern)

        var rgba64 = P.Rgba64(0, 0, 0, 0)
        rgba64.PackedValue = 0x0123_4567_89AB_CDEF
        XCTAssertEqual(rgba64.PackedValue, 0x0123_4567_89AB_CDEF)
        XCTAssertEqual(rgba64.GetHashCode(), Int32(bitPattern: 0x8888_8888))
    }
}
