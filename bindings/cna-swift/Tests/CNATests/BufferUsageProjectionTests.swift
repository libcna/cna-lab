// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class BufferUsageProjectionTests: XCTestCase {
    typealias Usage = Microsoft.Xna.Framework.Graphics.BufferUsage

    func testInt32OptionSetProjectionAndValueSemantics() {
        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        let zero = Usage(rawValue: 0)
        XCTAssertEqual(requireInt32OptionSet(zero), Usage.None.rawValue)
        XCTAssertEqual(Usage.WriteOnly.rawValue, 1)
        XCTAssertTrue(Usage.WriteOnly.contains(.WriteOnly))

        XCTAssertEqual(Usage.None.union(.WriteOnly).rawValue, 1)
        XCTAssertEqual(Usage.WriteOnly.union(.WriteOnly).rawValue, 1)
        XCTAssertEqual(Usage(rawValue: 2).union(.WriteOnly).rawValue, 3)
        XCTAssertEqual(Usage(rawValue: 3).intersection(.WriteOnly).rawValue, 1)
        XCTAssertEqual(Usage(rawValue: 2).intersection(.WriteOnly).rawValue, 0)

        XCTAssertEqual(Usage(rawValue: 2).rawValue, 2)
        XCTAssertEqual(Usage(rawValue: 3).rawValue, 3)
        XCTAssertEqual(Usage(rawValue: 1 << 20).rawValue, 1 << 20)
        XCTAssertEqual(Usage(rawValue: -1).rawValue, -1)

        let original = Usage(rawValue: 2)
        var copy = original
        copy.insert(.WriteOnly)
        XCTAssertEqual(original.rawValue, 2)
        XCTAssertEqual(copy.rawValue, 3)
    }
}
