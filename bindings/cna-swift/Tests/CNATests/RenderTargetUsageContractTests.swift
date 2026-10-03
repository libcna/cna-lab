// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testRenderTargetUsageXnaContract() {
        typealias Usage = Microsoft.Xna.Framework.Graphics.RenderTargetUsage
        let contract: [(name: String, value: Usage, rawValue: Int32)] = [
            ("DiscardContents", .DiscardContents, 0),
            ("PreserveContents", .PreserveContents, 1),
            ("PlatformContents", .PlatformContents, 2),
        ]

        XCTAssertEqual(contract.count, 3)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }
}
