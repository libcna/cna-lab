// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testSurfaceFormatXnaContract() {
        typealias Format = Microsoft.Xna.Framework.Graphics.SurfaceFormat
        let contract: [(name: String, value: Format, rawValue: Int32)] = [
            ("Color", .Color, 0),
            ("Bgr565", .Bgr565, 1),
            ("Bgra5551", .Bgra5551, 2),
            ("Bgra4444", .Bgra4444, 3),
            ("Dxt1", .Dxt1, 4),
            ("Dxt3", .Dxt3, 5),
            ("Dxt5", .Dxt5, 6),
            ("NormalizedByte2", .NormalizedByte2, 7),
            ("NormalizedByte4", .NormalizedByte4, 8),
            ("Rgba1010102", .Rgba1010102, 9),
            ("Rg32", .Rg32, 10),
            ("Rgba64", .Rgba64, 11),
            ("Alpha8", .Alpha8, 12),
            ("Single", .Single, 13),
            ("Vector2", .Vector2, 14),
            ("Vector4", .Vector4, 15),
            ("HalfSingle", .HalfSingle, 16),
            ("HalfVector2", .HalfVector2, 17),
            ("HalfVector4", .HalfVector4, 18),
            ("HdrBlendable", .HdrBlendable, 19),
        ]

        XCTAssertEqual(contract.count, 20)
        for entry in contract {
            XCTAssertEqual(entry.value.rawValue, entry.rawValue, entry.name)
        }
    }
}
