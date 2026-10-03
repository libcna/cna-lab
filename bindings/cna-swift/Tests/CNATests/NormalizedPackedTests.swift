// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testPackedNormalizedByteExactEndpointsTiesAndSignExtension() {
        typealias P = Framework.Graphics.PackedVector
        let two = P.NormalizedByte2(-1, 0.5)
        XCTAssertEqual(two.PackedValue, 0x4081)
        XCTAssertEqual(P.NormalizedByte2(Framework.Vector2(-1, 0.5)).PackedValue, 0x4081)
        XCTAssertEqual(two.ToVector2().X.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(two.ToVector2().Y.bitPattern, (Float(64) / Float(127)).bitPattern)
        XCTAssertEqual(P.NormalizedByte2(-0.5, 0.5).PackedValue, 0x40C0)
        XCTAssertEqual(P.NormalizedByte2(-2, 2).PackedValue, 0x7F81)
        XCTAssertEqual(P.NormalizedByte2(.nan, .infinity).PackedValue, 0x7F00)
        XCTAssertEqual(P.NormalizedByte2(-.infinity, .nan).PackedValue, 0x0081)

        var minimum = P.NormalizedByte2(0, 0)
        minimum.PackedValue = 0x0080
        XCTAssertEqual(minimum.PackedValue, 0x0080)
        XCTAssertEqual(minimum.ToVector2().X.bitPattern, Float(-1).bitPattern)
        minimum.PackedValue = 0x0082
        XCTAssertEqual(minimum.ToVector2().X.bitPattern, (Float(-126) / Float(127)).bitPattern)
        XCTAssertEqual(minimum.ToString(), "0082")
        XCTAssertEqual(minimum.GetHashCode(), 0x82)

        let four = P.NormalizedByte4(-1, -0.5, 0.5, 1)
        XCTAssertEqual(four.PackedValue, 0x7F40_C081)
        XCTAssertEqual(P.NormalizedByte4(Framework.Vector4(-1, -0.5, 0.5, 1)).PackedValue, 0x7F40_C081)
        XCTAssertEqual(four.ToVector4().X.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(four.ToVector4().Y.bitPattern, (Float(-64) / Float(127)).bitPattern)
        XCTAssertEqual(four.ToVector4().Z.bitPattern, (Float(64) / Float(127)).bitPattern)
        XCTAssertEqual(four.ToVector4().W.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(four.ToString(), "7F40C081")
        XCTAssertEqual(four.GetHashCode(), Int32(bitPattern: 0x7F40_C081))
        XCTAssertTrue(four.Equals(P.NormalizedByte4(-1, -0.5, 0.5, 1)))
        XCTAssertFalse(four.Equals(two as Any))
    }

    func testPackedNormalizedShortExactEndpointsTiesAndSignExtension() {
        typealias P = Framework.Graphics.PackedVector
        let two = P.NormalizedShort2(-1, 0.5)
        XCTAssertEqual(two.PackedValue, 0x4000_8001)
        XCTAssertEqual(P.NormalizedShort2(Framework.Vector2(-1, 0.5)).PackedValue, 0x4000_8001)
        XCTAssertEqual(two.ToVector2().X.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(two.ToVector2().Y.bitPattern, (Float(16_384) / Float(32_767)).bitPattern)
        XCTAssertEqual(P.NormalizedShort2(-0.5, 0.5).PackedValue, 0x4000_C000)
        XCTAssertEqual(P.NormalizedShort2(-2, 2).PackedValue, 0x7FFF_8001)
        XCTAssertEqual(P.NormalizedShort2(.nan, .infinity).PackedValue, 0x7FFF_0000)
        XCTAssertEqual(P.NormalizedShort2(-.infinity, .nan).PackedValue, 0x0000_8001)

        var minimum = P.NormalizedShort2(0, 0)
        minimum.PackedValue = 0x0000_8000
        XCTAssertEqual(minimum.PackedValue, 0x0000_8000)
        XCTAssertEqual(minimum.ToVector2().X.bitPattern, Float(-1).bitPattern)
        minimum.PackedValue = 0x0000_8002
        XCTAssertEqual(minimum.ToVector2().X.bitPattern, (Float(-32_766) / Float(32_767)).bitPattern)
        XCTAssertEqual(minimum.ToString(), "00008002")
        XCTAssertEqual(minimum.GetHashCode(), 0x8002)

        let four = P.NormalizedShort4(-1, -0.5, 0.5, 1)
        XCTAssertEqual(four.PackedValue, 0x7FFF_4000_C000_8001)
        XCTAssertEqual(P.NormalizedShort4(Framework.Vector4(-1, -0.5, 0.5, 1)).PackedValue, 0x7FFF_4000_C000_8001)
        let decoded = four.ToVector4()
        XCTAssertEqual(decoded.X.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(decoded.Y.bitPattern, (Float(-16_384) / Float(32_767)).bitPattern)
        XCTAssertEqual(decoded.Z.bitPattern, (Float(16_384) / Float(32_767)).bitPattern)
        XCTAssertEqual(decoded.W.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(four.ToString(), "7FFF4000C0008001")
        XCTAssertEqual(four.GetHashCode(), Int32(bitPattern: 0xBFFF_C001))
        XCTAssertTrue(four == P.NormalizedShort4(-1, -0.5, 0.5, 1))
        XCTAssertFalse(four != P.NormalizedShort4(-1, -0.5, 0.5, 1))
    }
}
