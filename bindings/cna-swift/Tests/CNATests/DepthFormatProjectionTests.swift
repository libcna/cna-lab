// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class DepthFormatProjectionTests: XCTestCase {
    typealias Format = Microsoft.Xna.Framework.Graphics.DepthFormat

    func testInt32RawEnumProjectionCompleteTableAndValueSemantics() {
        let expected: [(rawValue: Int32, value: Format)] = [
            (0, .None),
            (1, .Depth16),
            (2, .Depth24),
            (3, .Depth24Stencil8),
        ]

        XCTAssertEqual(expected.count, 4)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(Format(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertEqual(Format.None.rawValue, 0)
        XCTAssertNil(Format(rawValue: 4))
        XCTAssertNil(Format(rawValue: -1))
        XCTAssertNil(Format(rawValue: Int32.max))

        let original = Format.Depth24Stencil8
        var copy = original
        copy = .None
        XCTAssertEqual(original, .Depth24Stencil8)
        XCTAssertEqual(copy, .None)
    }
}
