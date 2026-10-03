// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

private func xnaReferenceHalfBits(_ value: UInt16) -> UInt32 {
    let raw = UInt32(value)
    let sign = (raw & 0x8000) << 16
    var fraction = raw & 0x03FF
    let storedExponent = (raw >> 10) & 0x1F
    if storedExponent != 0 {
        let exponent = Int32(storedExponent) - 15 + 127
        return sign | (UInt32(exponent) << 23) | (fraction << 13)
    }
    if fraction == 0 { return sign }
    var exponent: Int32 = -14
    while fraction & 0x0400 == 0 {
        exponent -= 1
        fraction <<= 1
    }
    return sign | (UInt32(exponent + 127) << 23) | ((fraction & 0x03FF) << 13)
}

extension PureValueTests {
    func testPackedHalfSingleSpecialValuesTiesAndCanonicalization() {
        typealias HalfSingle = Framework.Graphics.PackedVector.HalfSingle
        let fixtures: [(UInt32, UInt16, UInt32)] = [
            (0x0000_0000, 0x0000, 0x0000_0000),
            (0x8000_0000, 0x8000, 0x8000_0000),
            (0x3380_0000, 0x0001, 0x3380_0000),
            (0x387F_C000, 0x03FF, 0x387F_C000),
            (0x3880_0000, 0x0400, 0x3880_0000),
            (0x477F_E000, 0x7BFF, 0x477F_E000),
            (0x4780_0000, 0x7C00, 0x4780_0000),
            (0x47FF_E000, 0x7FFF, 0x47FF_E000),
            (0x7F80_0000, 0x7FFF, 0x47FF_E000),
            (0xFF80_0000, 0xFFFF, 0xC7FF_E000),
            (0x7FC0_0001, 0x7FFF, 0x47FF_E000),
            (0xFFC1_2345, 0xFFFF, 0xC7FF_E000),
        ]
        for (inputBits, packed, decodedBits) in fixtures {
            let value = HalfSingle(Float(bitPattern: inputBits))
            XCTAssertEqual(value.PackedValue, packed)
            XCTAssertEqual(value.ToSingle().bitPattern, decodedBits)
        }

        XCTAssertEqual(HalfSingle(Float(bitPattern: 0x3300_0000)).PackedValue, 0x0000)
        XCTAssertEqual(HalfSingle(Float(bitPattern: 0x3300_0001)).PackedValue, 0x0000)
        XCTAssertEqual(HalfSingle(Float(bitPattern: 0x3300_0800)).PackedValue, 0x0001)
        XCTAssertEqual(HalfSingle(Float(bitPattern: 0x3F80_0FFF)).PackedValue, 0x3C00)
        XCTAssertEqual(HalfSingle(Float(bitPattern: 0x3F80_1000)).PackedValue, 0x3C00)
        XCTAssertEqual(HalfSingle(Float(bitPattern: 0x3F80_1001)).PackedValue, 0x3C01)
        XCTAssertEqual(HalfSingle(Float(bitPattern: 0x3F80_3000)).PackedValue, 0x3C02)

        var first = HalfSingle(0)
        var second = HalfSingle(0)
        first.PackedValue = 0x7C01
        second.PackedValue = 0x7C02
        XCTAssertFalse(first.Equals(second))
        XCTAssertTrue(first != second)
        XCTAssertFalse(first.Equals(Float.nan as Any))
        XCTAssertEqual(first.GetHashCode(), 0x7C01)
        XCTAssertEqual(HalfSingle(1).ToString(), "1")
        XCTAssertEqual(first.PackedValue, 0x7C01)
    }

    func testPackedHalfExhaustive65536DecodeAndRepackIdentity() {
        typealias HalfSingle = Framework.Graphics.PackedVector.HalfSingle
        var value = HalfSingle(0)
        var decodeFailures = 0
        var repackFailures = 0
        var negativeZeros = 0
        var extendedFinitePatterns = 0

        for raw in UInt32(0)...UInt32(UInt16.max) {
            let packed = UInt16(raw)
            value.PackedValue = packed
            let decoded = value.ToSingle()
            if decoded.bitPattern != xnaReferenceHalfBits(packed) { decodeFailures += 1 }
            if HalfSingle(decoded).PackedValue != packed { repackFailures += 1 }
            if decoded.bitPattern == 0x8000_0000 { negativeZeros += 1 }
            if packed & 0x7C00 == 0x7C00 { extendedFinitePatterns += 1 }
        }

        XCTAssertEqual(decodeFailures, 0)
        XCTAssertEqual(repackFailures, 0)
        XCTAssertEqual(negativeZeros, 1)
        XCTAssertEqual(extendedFinitePatterns, 2_048)
        XCTAssertEqual(value.PackedValue, 0xFFFF)
        XCTAssertEqual(value.ToSingle().bitPattern, 0xC7FF_E000)
    }

    func testPackedHalfVectorLaneOrderMixedSpecialValuesAndStrings() {
        typealias P = Framework.Graphics.PackedVector
        let half2 = P.HalfVector2(-0.0, 1)
        XCTAssertEqual(half2.PackedValue, 0x3C00_8000)
        XCTAssertEqual(half2.ToVector2().X.bitPattern, 0x8000_0000)
        XCTAssertEqual(half2.ToVector2().Y.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(half2.ToString(), "{X:0 Y:1}")

        let half4 = P.HalfVector4(1, -2, 0.5, -0.0)
        XCTAssertEqual(half4.PackedValue, 0x8000_3800_C000_3C00)
        let decoded = half4.ToVector4()
        XCTAssertEqual(decoded.X.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(decoded.Y.bitPattern, Float(-2).bitPattern)
        XCTAssertEqual(decoded.Z.bitPattern, Float(0.5).bitPattern)
        XCTAssertEqual(decoded.W.bitPattern, 0x8000_0000)
        XCTAssertEqual(half4.ToString(), "{X:1 Y:-2 Z:0.5 W:0}")

        let special = P.HalfVector4(.infinity, -.infinity, .nan, -0.0)
        XCTAssertEqual(special.PackedValue, 0x8000_7FFF_FFFF_7FFF)
        let specialDecoded = special.ToVector4()
        XCTAssertEqual(specialDecoded.X.bitPattern, 0x47FF_E000)
        XCTAssertEqual(specialDecoded.Y.bitPattern, 0xC7FF_E000)
        XCTAssertEqual(specialDecoded.Z.bitPattern, 0x47FF_E000)
        XCTAssertEqual(specialDecoded.W.bitPattern, 0x8000_0000)
        XCTAssertEqual(special.GetHashCode(), Int32(bitPattern: 0x7FFF_0000))

        var assigned = P.HalfVector4(0, 0, 0, 0)
        assigned.PackedValue = 0xFFFF_7C01_8000_0001
        XCTAssertEqual(assigned.PackedValue, 0xFFFF_7C01_8000_0001)
        XCTAssertEqual(assigned.ToVector4().X.bitPattern, 0x3380_0000)
        XCTAssertEqual(assigned.ToVector4().Y.bitPattern, 0x8000_0000)
        XCTAssertEqual(assigned.ToVector4().Z.bitPattern, xnaReferenceHalfBits(0x7C01))
        XCTAssertEqual(assigned.ToVector4().W.bitPattern, 0xC7FF_E000)
    }
}
