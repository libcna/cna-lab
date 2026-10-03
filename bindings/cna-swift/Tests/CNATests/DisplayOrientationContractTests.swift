// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testDisplayOrientationXnaContract() {
        typealias Orientation = Microsoft.Xna.Framework.DisplayOrientation
        let defaultRaw: Int32 = Orientation.Default.rawValue

        XCTAssertEqual(defaultRaw, 0)
        XCTAssertEqual(Orientation.LandscapeLeft.rawValue, 1)
        XCTAssertEqual(Orientation.LandscapeRight.rawValue, 2)
        XCTAssertEqual(Orientation.Portrait.rawValue, 4)
    }
}
