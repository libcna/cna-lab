// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testFillModeXnaContract() {
        typealias Mode = Microsoft.Xna.Framework.Graphics.FillMode
        let solidRaw: Int32 = Mode.Solid.rawValue

        XCTAssertEqual(solidRaw, 0)
        XCTAssertEqual(Mode.WireFrame.rawValue, 1)
    }
}
