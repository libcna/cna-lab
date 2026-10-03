// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class SurfaceFormatProjectionTests: XCTestCase {
    typealias Format = Microsoft.Xna.Framework.Graphics.SurfaceFormat

    func testInt32RawEnumProjectionCompleteTableAndValueSemantics() {
        let expected: [(rawValue: Int32, value: Format)] = [
            (0, .Color),
            (1, .Bgr565),
            (2, .Bgra5551),
            (3, .Bgra4444),
            (4, .Dxt1),
            (5, .Dxt3),
            (6, .Dxt5),
            (7, .NormalizedByte2),
            (8, .NormalizedByte4),
            (9, .Rgba1010102),
            (10, .Rg32),
            (11, .Rgba64),
            (12, .Alpha8),
            (13, .Single),
            (14, .Vector2),
            (15, .Vector4),
            (16, .HalfSingle),
            (17, .HalfVector2),
            (18, .HalfVector4),
            (19, .HdrBlendable),
        ]

        XCTAssertEqual(expected.count, 20)
        for entry in expected {
            let rawValue: Int32 = entry.value.rawValue
            XCTAssertEqual(rawValue, entry.rawValue)
            XCTAssertEqual(Format(rawValue: entry.rawValue), entry.value)
        }

        XCTAssertNil(Format(rawValue: 20))
        XCTAssertNil(Format(rawValue: -1))
        XCTAssertNil(Format(rawValue: Int32.max))

        let original = Format.HalfVector4
        var copy = original
        copy = .Color
        XCTAssertEqual(original, .HalfVector4)
        XCTAssertEqual(copy, .Color)
    }
}
