// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class FillModeProjectionTests: XCTestCase {
    typealias Mode = Microsoft.Xna.Framework.Graphics.FillMode

    func testInt32RawEnumProjectionAndValueSemantics() {
        let solidRaw: Int32 = Mode.Solid.rawValue
        let wireFrameRaw: Int32 = Mode.WireFrame.rawValue
        XCTAssertEqual(solidRaw, 0)
        XCTAssertEqual(wireFrameRaw, 1)

        XCTAssertEqual(Mode(rawValue: 0), .Solid)
        XCTAssertEqual(Mode(rawValue: 1), .WireFrame)
        XCTAssertNil(Mode(rawValue: 2))
        XCTAssertNil(Mode(rawValue: -1))

        let original = Mode.WireFrame
        var copy = original
        copy = .Solid
        XCTAssertEqual(original, .WireFrame)
        XCTAssertEqual(copy, .Solid)
    }
}
