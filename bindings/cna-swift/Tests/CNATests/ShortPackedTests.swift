// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testPackedShortRawDomainClampBankersRoundingAndTwosComplement() {
        typealias P = Framework.Graphics.PackedVector
        let two = P.Short2(-32768, 32767)
        XCTAssertEqual(two.PackedValue, 0x7FFF_8000)
        XCTAssertEqual(P.Short2(Framework.Vector2(-32768, 32767)).PackedValue, 0x7FFF_8000)
        XCTAssertEqual(P.Short2(-40000, 40000).PackedValue, 0x7FFF_8000)
        XCTAssertEqual(P.Short2(-0.5, 0.5).PackedValue, 0x0000_0000)
        XCTAssertEqual(P.Short2(-1.5, 1.5).PackedValue, 0x0002_FFFE)
        XCTAssertEqual(P.Short2(.nan, .infinity).PackedValue, 0x7FFF_0000)
        XCTAssertEqual(P.Short2(-.infinity, .nan).PackedValue, 0x0000_8000)
        XCTAssertEqual(two.ToVector2().X.bitPattern, Float(-32768).bitPattern)
        XCTAssertEqual(two.ToVector2().Y.bitPattern, Float(32767).bitPattern)
        XCTAssertEqual(two.ToString(), "7FFF8000")
        XCTAssertEqual(two.GetHashCode(), 0x7FFF_8000)

        let four = P.Short4(-32768, -0.5, 0.5, 32767)
        XCTAssertEqual(four.PackedValue, 0x7FFF_0000_0000_8000)
        XCTAssertEqual(P.Short4(Framework.Vector4(-32768, -0.5, 0.5, 32767)).PackedValue, 0x7FFF_0000_0000_8000)
        XCTAssertEqual(P.Short4(-0.5, 0.5, 1.5, 2.5).PackedValue, 0x0002_0002_0000_0000)
        XCTAssertEqual(P.Short4(-.infinity, .infinity, .nan, -0.0).PackedValue, 0x0000_0000_7FFF_8000)
        let decoded = four.ToVector4()
        XCTAssertEqual(decoded.X.bitPattern, Float(-32768).bitPattern)
        XCTAssertEqual(decoded.Y.bitPattern, 0)
        XCTAssertEqual(decoded.Z.bitPattern, 0)
        XCTAssertEqual(decoded.W.bitPattern, Float(32767).bitPattern)
        XCTAssertEqual(four.ToString(), "7FFF000000008000")
        XCTAssertEqual(four.GetHashCode(), Int32(bitPattern: 0x7FFF_8000))
        XCTAssertTrue(four.Equals(P.Short4(-32768, -0.5, 0.5, 32767)))
        XCTAssertFalse(four.Equals(two as Any))
        XCTAssertTrue(four == P.Short4(-32768, 0, 0, 32767))
        XCTAssertFalse(four != P.Short4(-32768, 0, 0, 32767))
    }

    func testPackedShortDirectAssignmentPreservesEveryBit() {
        typealias P = Framework.Graphics.PackedVector
        var two = P.Short2(0, 0)
        two.PackedValue = 0x8000_FFFF
        XCTAssertEqual(two.PackedValue, 0x8000_FFFF)
        XCTAssertEqual(two.ToVector2().X.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(two.ToVector2().Y.bitPattern, Float(-32768).bitPattern)

        var four = P.Short4(0, 0, 0, 0)
        four.PackedValue = 0x8000_7FFF_FFFF_0001
        XCTAssertEqual(four.PackedValue, 0x8000_7FFF_FFFF_0001)
        let decoded = four.ToVector4()
        XCTAssertEqual(decoded.X.bitPattern, Float(1).bitPattern)
        XCTAssertEqual(decoded.Y.bitPattern, Float(-1).bitPattern)
        XCTAssertEqual(decoded.Z.bitPattern, Float(32767).bitPattern)
        XCTAssertEqual(decoded.W.bitPattern, Float(-32768).bitPattern)
        XCTAssertEqual(four.GetHashCode(), Int32(bitPattern: 0x7FFF_7FFE))
    }
}
