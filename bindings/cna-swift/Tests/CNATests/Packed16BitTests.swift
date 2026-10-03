// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testPackedAlphaXnaBitsRoundingNonFiniteAndAllPatterns() {
        typealias Alpha8 = Framework.Graphics.PackedVector.Alpha8
        XCTAssertEqual(Alpha8(0).PackedValue, 0x00)
        XCTAssertEqual(Alpha8(1).PackedValue, 0xFF)
        XCTAssertEqual(Alpha8(0.5).PackedValue, 0x80)
        XCTAssertEqual(Alpha8(-1).PackedValue, 0x00)
        XCTAssertEqual(Alpha8(2).PackedValue, 0xFF)
        XCTAssertEqual(Alpha8(.nan).PackedValue, 0x00)
        XCTAssertEqual(Alpha8(.infinity).PackedValue, 0xFF)
        XCTAssertEqual(Alpha8(-.infinity).PackedValue, 0x00)
        XCTAssertEqual(Alpha8(Float(bitPattern: 0x3EFF_FFFE)).PackedValue, 0x7F)
        XCTAssertEqual(Alpha8(Float(bitPattern: 0x3F00_0000)).PackedValue, 0x80)
        XCTAssertEqual(Alpha8(Float(bitPattern: 0x3F00_0001)).PackedValue, 0x80)

        var failures = 0
        var value = Alpha8(0)
        for raw in UInt16(0)...UInt16(255) {
            value.PackedValue = UInt8(raw)
            let expected = Float(UInt8(raw)) / Float(255)
            if value.ToAlpha().bitPattern != expected.bitPattern || value.PackedValue != UInt8(raw) {
                failures += 1
            }
        }
        XCTAssertEqual(failures, 0)
        XCTAssertEqual(value.PackedValue, 0xFF)
        XCTAssertEqual(value.ToString(), "FF")
        XCTAssertEqual(value.GetHashCode(), 255)
        XCTAssertTrue(value.Equals(Alpha8(1)))
        XCTAssertTrue(value.Equals(Alpha8(1) as Any))
        XCTAssertFalse(value.Equals(nil))
        XCTAssertTrue(value == Alpha8(1))
        XCTAssertFalse(value != Alpha8(1))
    }

    func testPacked565_4444_5551IndependentGoldensAndMidpoints() {
        typealias P = Framework.Graphics.PackedVector
        XCTAssertEqual(P.Bgr565(1, 0.5, 0.25).PackedValue, 0xFC08)
        XCTAssertEqual(P.Bgr565(Framework.Vector3(1, 0.5, 0.25)).PackedValue, 0xFC08)
        XCTAssertEqual(P.Bgr565(1, 0, 0).PackedValue, 0xF800)
        XCTAssertEqual(P.Bgr565(0, 1, 0).PackedValue, 0x07E0)
        XCTAssertEqual(P.Bgr565(0, 0, 1).PackedValue, 0x001F)
        XCTAssertEqual(P.Bgr565(.nan, .infinity, -.infinity).PackedValue, 0x07E0)

        XCTAssertEqual(P.Bgra4444(1, 0.5, 0.25, 0.75).PackedValue, 0xBF84)
        XCTAssertEqual(P.Bgra4444(Framework.Vector4(1, 0.5, 0.25, 0.75)).PackedValue, 0xBF84)
        XCTAssertEqual(P.Bgra4444(1, 0, 0, 1).PackedValue, 0xFF00)
        XCTAssertEqual(P.Bgra4444(0, 1, 0, 0).PackedValue, 0x00F0)
        XCTAssertEqual(P.Bgra4444(0, 0, 1, 0).PackedValue, 0x000F)

        let below = Float(bitPattern: 0x3EFF_FFFE)
        let exact = Float(bitPattern: 0x3F00_0000)
        let above = Float(bitPattern: 0x3F00_0001)
        XCTAssertEqual(P.Bgra5551(0, 0, 0, below).PackedValue, 0x0000)
        XCTAssertEqual(P.Bgra5551(0, 0, 0, exact).PackedValue, 0x0000)
        XCTAssertEqual(P.Bgra5551(0, 0, 0, above).PackedValue, 0x8000)
        XCTAssertEqual(P.Bgra5551(1, 0.5, 0.25, exact).PackedValue, 0x7E08)
        XCTAssertEqual(P.Bgra5551(Framework.Vector4(1, 0.5, 0.25, exact)).PackedValue, 0x7E08)
        XCTAssertEqual(P.Bgra5551(.nan, .infinity, -.infinity, .infinity).PackedValue, 0x83E0)
    }

    func testPacked565_4444_5551ExhaustiveDecodeSweeps() {
        typealias P = Framework.Graphics.PackedVector
        var bgr = P.Bgr565(0, 0, 0)
        var bgra4444 = P.Bgra4444(0, 0, 0, 0)
        var bgra5551 = P.Bgra5551(0, 0, 0, 0)
        var bgrFailures = 0
        var bgra4444Failures = 0
        var bgra5551Failures = 0

        for raw in UInt32(0)...UInt32(UInt16.max) {
            let packed = UInt16(raw)
            bgr.PackedValue = packed
            let bgrValue = bgr.ToVector3()
            if bgrValue.X.bitPattern != (Float((raw >> 11) & 31) / Float(31)).bitPattern ||
                bgrValue.Y.bitPattern != (Float((raw >> 5) & 63) / Float(63)).bitPattern ||
                bgrValue.Z.bitPattern != (Float(raw & 31) / Float(31)).bitPattern ||
                bgr.PackedValue != packed {
                bgrFailures += 1
            }

            bgra4444.PackedValue = packed
            let value4444 = bgra4444.ToVector4()
            if value4444.X.bitPattern != (Float((raw >> 8) & 15) / Float(15)).bitPattern ||
                value4444.Y.bitPattern != (Float((raw >> 4) & 15) / Float(15)).bitPattern ||
                value4444.Z.bitPattern != (Float(raw & 15) / Float(15)).bitPattern ||
                value4444.W.bitPattern != (Float((raw >> 12) & 15) / Float(15)).bitPattern ||
                bgra4444.PackedValue != packed {
                bgra4444Failures += 1
            }

            bgra5551.PackedValue = packed
            let value5551 = bgra5551.ToVector4()
            if value5551.X.bitPattern != (Float((raw >> 10) & 31) / Float(31)).bitPattern ||
                value5551.Y.bitPattern != (Float((raw >> 5) & 31) / Float(31)).bitPattern ||
                value5551.Z.bitPattern != (Float(raw & 31) / Float(31)).bitPattern ||
                value5551.W.bitPattern != Float((raw >> 15) & 1).bitPattern ||
                bgra5551.PackedValue != packed {
                bgra5551Failures += 1
            }
        }

        XCTAssertEqual(bgrFailures, 0)
        XCTAssertEqual(bgra4444Failures, 0)
        XCTAssertEqual(bgra5551Failures, 0)
        XCTAssertEqual(bgr.ToString(), "FFFF")
        XCTAssertEqual(bgra4444.GetHashCode(), 65_535)
        XCTAssertTrue(bgra5551.Equals(bgra5551))
        XCTAssertFalse(bgra5551.Equals(bgra4444 as Any))
    }
}
