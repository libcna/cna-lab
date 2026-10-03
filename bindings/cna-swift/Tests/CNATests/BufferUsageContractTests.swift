// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testBufferUsageXnaContract() {
        typealias Usage = Microsoft.Xna.Framework.Graphics.BufferUsage

        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        XCTAssertEqual(requireInt32OptionSet(.None as Usage), 0)
        XCTAssertEqual(Usage.WriteOnly.rawValue, 1)
    }
}
