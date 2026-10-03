// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testPackedEqualityOperatorsUseExactPackedBitsForAllFormats() {
        typealias P = Framework.Graphics.PackedVector

        let alpha = P.Alpha8(0.5)
        XCTAssertTrue(alpha.Equals(alpha as Any)); XCTAssertTrue(alpha == alpha); XCTAssertFalse(alpha != alpha)
        let bgr = P.Bgr565(1, 0.5, 0.25)
        XCTAssertTrue(bgr.Equals(bgr as Any)); XCTAssertTrue(bgr == bgr); XCTAssertFalse(bgr != bgr)
        let bgra4444 = P.Bgra4444(1, 0.5, 0.25, 0.75)
        XCTAssertTrue(bgra4444.Equals(bgra4444 as Any)); XCTAssertTrue(bgra4444 == bgra4444); XCTAssertFalse(bgra4444 != bgra4444)
        let bgra5551 = P.Bgra5551(1, 0.5, 0.25, 0.5)
        XCTAssertTrue(bgra5551.Equals(bgra5551 as Any)); XCTAssertTrue(bgra5551 == bgra5551); XCTAssertFalse(bgra5551 != bgra5551)
        let byte4 = P.Byte4(-1, 0.5, 127.5, 256)
        XCTAssertTrue(byte4.Equals(byte4 as Any)); XCTAssertTrue(byte4 == byte4); XCTAssertFalse(byte4 != byte4)
        var halfSingleA = P.HalfSingle(0); halfSingleA.PackedValue = 0x7FFF
        var halfSingleB = P.HalfSingle(0); halfSingleB.PackedValue = 0x7FFF
        XCTAssertTrue(halfSingleA.Equals(halfSingleB as Any)); XCTAssertTrue(halfSingleA == halfSingleB); XCTAssertFalse(halfSingleA != halfSingleB)
        let half2 = P.HalfVector2(1, -2)
        XCTAssertTrue(half2.Equals(half2 as Any)); XCTAssertTrue(half2 == half2); XCTAssertFalse(half2 != half2)
        let half4 = P.HalfVector4(1, -2, 0.5, -0.0)
        XCTAssertTrue(half4.Equals(half4 as Any)); XCTAssertTrue(half4 == half4); XCTAssertFalse(half4 != half4)
        let nb2 = P.NormalizedByte2(-1, 0.5)
        XCTAssertTrue(nb2.Equals(nb2 as Any)); XCTAssertTrue(nb2 == nb2); XCTAssertFalse(nb2 != nb2)
        let nb4 = P.NormalizedByte4(-1, -0.5, 0.5, 1)
        XCTAssertTrue(nb4.Equals(nb4 as Any)); XCTAssertTrue(nb4 == nb4); XCTAssertFalse(nb4 != nb4)
        let ns2 = P.NormalizedShort2(-1, 0.5)
        XCTAssertTrue(ns2.Equals(ns2 as Any)); XCTAssertTrue(ns2 == ns2); XCTAssertFalse(ns2 != ns2)
        let ns4 = P.NormalizedShort4(-1, -0.5, 0.5, 1)
        XCTAssertTrue(ns4.Equals(ns4 as Any)); XCTAssertTrue(ns4 == ns4); XCTAssertFalse(ns4 != ns4)
        let rg = P.Rg32(0.5, 1)
        XCTAssertTrue(rg.Equals(rg as Any)); XCTAssertTrue(rg == rg); XCTAssertFalse(rg != rg)
        let rgba10 = P.Rgba1010102(1, 0.5, 0.25, 0.5)
        XCTAssertTrue(rgba10.Equals(rgba10 as Any)); XCTAssertTrue(rgba10 == rgba10); XCTAssertFalse(rgba10 != rgba10)
        let rgba64 = P.Rgba64(1, 0.5, 0.25, 0.75)
        XCTAssertTrue(rgba64.Equals(rgba64 as Any)); XCTAssertTrue(rgba64 == rgba64); XCTAssertFalse(rgba64 != rgba64)
        let short2 = P.Short2(-32768, 32767)
        XCTAssertTrue(short2.Equals(short2 as Any)); XCTAssertTrue(short2 == short2); XCTAssertFalse(short2 != short2)
        let short4 = P.Short4(-32768, 0, 0, 32767)
        XCTAssertTrue(short4.Equals(short4 as Any)); XCTAssertTrue(short4 == short4); XCTAssertFalse(short4 != short4)

        var differentHalf = halfSingleB
        differentHalf.PackedValue = 0xFFFF
        XCTAssertFalse(halfSingleA.Equals(differentHalf))
        XCTAssertTrue(halfSingleA != differentHalf)
    }

    func testPackedHashAndStringAreExactForAllStorageWidths() {
        typealias P = Framework.Graphics.PackedVector
        XCTAssertEqual(P.Alpha8(0.5).ToString(), "80")
        XCTAssertEqual(P.Alpha8(0.5).GetHashCode(), 0x80)
        XCTAssertEqual(P.Bgr565(1, 0.5, 0.25).ToString(), "FC08")
        XCTAssertEqual(P.Bgr565(1, 0.5, 0.25).GetHashCode(), 0xFC08)
        XCTAssertEqual(P.Bgra4444(1, 0.5, 0.25, 0.75).ToString(), "BF84")
        XCTAssertEqual(P.Bgra4444(1, 0.5, 0.25, 0.75).GetHashCode(), 0xBF84)
        XCTAssertEqual(P.Bgra5551(1, 0.5, 0.25, 0.5).ToString(), "7E08")
        XCTAssertEqual(P.Bgra5551(1, 0.5, 0.25, 0.5).GetHashCode(), 0x7E08)
        XCTAssertEqual(P.Byte4(-1, 0.5, 127.5, 256).ToString(), "FF800000")
        XCTAssertEqual(P.Byte4(-1, 0.5, 127.5, 256).GetHashCode(), Int32(bitPattern: 0xFF80_0000))
        XCTAssertEqual(P.HalfSingle(1).ToString(), "1")
        XCTAssertEqual(P.HalfSingle(1).GetHashCode(), 0x3C00)
        XCTAssertEqual(P.HalfVector2(1, -2).ToString(), "{X:1 Y:-2}")
        XCTAssertEqual(P.HalfVector2(1, -2).GetHashCode(), Int32(bitPattern: 0xC000_3C00))
        XCTAssertEqual(P.HalfVector4(1, -2, 0.5, -0.0).ToString(), "{X:1 Y:-2 Z:0.5 W:0}")
        XCTAssertEqual(P.HalfVector4(1, -2, 0.5, -0.0).GetHashCode(), Int32(bitPattern: 0x4000_0400))
        XCTAssertEqual(P.NormalizedByte2(-1, 0.5).ToString(), "4081")
        XCTAssertEqual(P.NormalizedByte2(-1, 0.5).GetHashCode(), 0x4081)
        XCTAssertEqual(P.NormalizedByte4(-1, -0.5, 0.5, 1).ToString(), "7F40C081")
        XCTAssertEqual(P.NormalizedByte4(-1, -0.5, 0.5, 1).GetHashCode(), 0x7F40_C081)
        XCTAssertEqual(P.NormalizedShort2(-1, 0.5).ToString(), "40008001")
        XCTAssertEqual(P.NormalizedShort2(-1, 0.5).GetHashCode(), 0x4000_8001)
        XCTAssertEqual(P.NormalizedShort4(-1, -0.5, 0.5, 1).ToString(), "7FFF4000C0008001")
        XCTAssertEqual(P.NormalizedShort4(-1, -0.5, 0.5, 1).GetHashCode(), Int32(bitPattern: 0xBFFF_C001))
        XCTAssertEqual(P.Rg32(0.5, 1).ToString(), "FFFF8000")
        XCTAssertEqual(P.Rg32(0.5, 1).GetHashCode(), Int32(bitPattern: 0xFFFF_8000))
        XCTAssertEqual(P.Rgba1010102(1, 0.5, 0.25, 0.5).ToString(), "900803FF")
        XCTAssertEqual(P.Rgba1010102(1, 0.5, 0.25, 0.5).GetHashCode(), Int32(bitPattern: 0x9008_03FF))
        XCTAssertEqual(P.Rgba64(1, 0.5, 0.25, 0.75).ToString(), "BFFF40008000FFFF")
        XCTAssertEqual(P.Rgba64(1, 0.5, 0.25, 0.75).GetHashCode(), Int32(bitPattern: 0x3FFF_BFFF))
        XCTAssertEqual(P.Short2(-32768, 32767).ToString(), "7FFF8000")
        XCTAssertEqual(P.Short2(-32768, 32767).GetHashCode(), 0x7FFF_8000)
        XCTAssertEqual(P.Short4(-32768, 0, 0, 32767).ToString(), "7FFF000000008000")
        XCTAssertEqual(P.Short4(-32768, 0, 0, 32767).GetHashCode(), Int32(bitPattern: 0x7FFF_8000))
    }
}
