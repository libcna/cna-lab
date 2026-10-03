// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testDepthFormatXnaContract() {
        typealias Format = Microsoft.Xna.Framework.Graphics.DepthFormat
        let contract: [(name: String, value: Format, rawValue: Int32)] = [
            ("None", .None, 0),
            ("Depth16", .Depth16, 1),
            ("Depth24", .Depth24, 2),
            ("Depth24Stencil8", .Depth24Stencil8, 3),
        ]

        XCTAssertEqual(contract.count, 4)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }
}
