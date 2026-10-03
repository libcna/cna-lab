// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class DisplayOrientationProjectionTests: XCTestCase {
    typealias Orientation = Microsoft.Xna.Framework.DisplayOrientation

    func testInt32OptionSetProjectionAndValueSemantics() {
        func requireInt32OptionSet<T: OptionSet>(_ value: T) -> Int32 where T.RawValue == Int32 {
            value.rawValue
        }

        let zero = Orientation(rawValue: 0)
        XCTAssertEqual(requireInt32OptionSet(zero), Orientation.Default.rawValue)
        XCTAssertEqual(Orientation.LandscapeLeft.union(.LandscapeRight).rawValue, 3)
        XCTAssertEqual(Orientation.LandscapeLeft.union(.Portrait).rawValue, 5)
        XCTAssertEqual(Orientation.LandscapeRight.union(.Portrait).rawValue, 6)
        XCTAssertEqual(
            Orientation.LandscapeLeft.union(.LandscapeRight).union(.Portrait).rawValue,
            7
        )

        let leftAndPortrait = Orientation(
            rawValue: Orientation.LandscapeLeft.rawValue | Orientation.Portrait.rawValue
        )
        XCTAssertEqual(leftAndPortrait.intersection(.LandscapeLeft).rawValue, 1)
        XCTAssertEqual(leftAndPortrait.intersection(.LandscapeRight).rawValue, 0)
        XCTAssertEqual(Orientation(rawValue: 8).rawValue, 8)
        XCTAssertEqual(Orientation(rawValue: -1).rawValue, -1)

        let original: Orientation = [.LandscapeLeft, .LandscapeRight]
        var copy = original
        copy.insert(.Portrait)
        XCTAssertEqual(original.rawValue, 3)
        XCTAssertEqual(copy.rawValue, 7)
    }
}
