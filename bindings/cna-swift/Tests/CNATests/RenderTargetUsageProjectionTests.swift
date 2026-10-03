// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class RenderTargetUsageProjectionTests: XCTestCase {
    typealias Usage = Microsoft.Xna.Framework.Graphics.RenderTargetUsage

    func testInt32RawEnumProjectionCompleteTableAndValueSemantics() {
        let expected: [(rawValue: Int32, value: Usage)] = [
            (0, .DiscardContents),
            (1, .PreserveContents),
            (2, .PlatformContents),
        ]

        XCTAssertEqual(expected.count, 3)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(Usage(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertEqual(Usage.DiscardContents.rawValue, 0)
        XCTAssertNil(Usage(rawValue: 3))
        XCTAssertNil(Usage(rawValue: -1))
        XCTAssertNil(Usage(rawValue: Int32.max))
        XCTAssertNil(Usage(rawValue: Int32.min))

        let original = Usage.PreserveContents
        var copy = original
        copy = .PlatformContents
        XCTAssertEqual(original, .PreserveContents)
        XCTAssertEqual(copy, .PlatformContents)
    }
}
