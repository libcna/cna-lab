// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

final class PureValueTests: XCTestCase {
    typealias Framework = Microsoft.Xna.Framework

    func testNamespaceAndTypeKindsCompile() {
        let point: Microsoft.Xna.Framework.Point = .Zero
        let rectangle: Microsoft.Xna.Framework.Rectangle = .Empty
        let vector: Microsoft.Xna.Framework.Vector2 = .UnitX
        XCTAssertEqual(point.X, 0)
        XCTAssertTrue(rectangle.IsEmpty)
        XCTAssertEqual(vector.X.bitPattern, Float(1).bitPattern)
    }

    func testMathHelperReferenceObservations() {
        XCTAssertEqual(Framework.MathHelper.Clamp(-1, min: 0, max: 1).bitPattern, Float(0).bitPattern)
        XCTAssertEqual(Framework.MathHelper.Clamp(2, min: 0, max: 1).bitPattern, Float(1).bitPattern)
        XCTAssertEqual(Framework.MathHelper.Lerp(10, value2: 20, amount: 0.25).bitPattern, Float(12.5).bitPattern)
        XCTAssertEqual(Framework.MathHelper.Barycentric(1, value2: 3, value3: 5, amount1: 0.5, amount2: 0.25).bitPattern, Float(3).bitPattern)
        XCTAssertEqual(Framework.MathHelper.SmoothStep(0, value2: 1, amount: -1).bitPattern, Float(0).bitPattern)
        XCTAssertEqual(Framework.MathHelper.SmoothStep(0, value2: 1, amount: 2).bitPattern, Float(1).bitPattern)
        XCTAssertEqual(Framework.MathHelper.WrapAngle(Framework.MathHelper.TwoPi).bitPattern, Float(0).bitPattern)
        XCTAssertTrue(Framework.MathHelper.Distance(.infinity, value2: .infinity).isNaN)
    }

    func testPointCompleteContractBehavior() {
        var point = Framework.Point(4, -3)
        XCTAssertTrue(point.Equals(Framework.Point(4, -3)))
        XCTAssertFalse(point.Equals(Framework.Point(4, 3)))
        XCTAssertEqual(point.GetHashCode(), Int32(4) ^ Int32(-3))
        XCTAssertEqual(point.ToString(), "{X:4 Y:-3}")
        point.X = 8
        XCTAssertTrue(point != Framework.Point(4, -3))
    }

    func testRectangleCompleteContractBehavior() {
        var rectangle = Framework.Rectangle(10, 20, 30, 40)
        XCTAssertEqual(rectangle.Right, 40)
        XCTAssertEqual(rectangle.Bottom, 60)
        XCTAssertTrue(rectangle.Contains(10, y: 20))
        XCTAssertFalse(rectangle.Contains(40, y: 20))
        XCTAssertTrue(rectangle.Intersects(Framework.Rectangle(39, 59, 2, 2)))
        XCTAssertFalse(rectangle.Intersects(Framework.Rectangle(40, 60, 2, 2)))
        rectangle.Inflate(2, verticalAmount: 3)
        XCTAssertTrue(rectangle == Framework.Rectangle(8, 17, 34, 46))
        let intersection = Framework.Rectangle.Intersect(rectangle, value2: Framework.Rectangle(0, 0, 10, 20))
        XCTAssertTrue(intersection == Framework.Rectangle(8, 17, 2, 3))
        let union = Framework.Rectangle.Union(Framework.Rectangle(0, 0, 2, 3), value2: Framework.Rectangle(4, 5, 2, 3))
        XCTAssertTrue(union == Framework.Rectangle(0, 0, 6, 8))
    }

    func testVector2ImplementedMembersAreRealBinary32Math() {
        let vector = Framework.Vector2(3, 4)
        XCTAssertEqual(vector.Length().bitPattern, Float(5).bitPattern)
        XCTAssertEqual(vector.LengthSquared().bitPattern, Float(25).bitPattern)
        let normalized = Framework.Vector2.Normalize(vector)
        XCTAssertEqual(normalized.X.bitPattern, Float(0.6).bitPattern)
        XCTAssertEqual(normalized.Y.bitPattern, Float(0.8).bitPattern)
        XCTAssertTrue(Framework.Vector2(2, 3) + Framework.Vector2(5, 7) == Framework.Vector2(7, 10))
        XCTAssertTrue(Framework.Vector2(2, 3) * Framework.Vector2(5, 7) == Framework.Vector2(10, 21))
    }

    func testColorPackingAndCanaryValues() {
        let color = Framework.Color(Int32(100), Int32(149), Int32(237), Int32(255))
        XCTAssertEqual(color.PackedValue, 0xFFED_9564)
        XCTAssertTrue(color == .CornflowerBlue)
        XCTAssertEqual(Framework.Color.White.PackedValue, UInt32.max)
        XCTAssertEqual(Framework.Color.Transparent.PackedValue, 0)
        XCTAssertEqual(Framework.Color(Int32(-10), Int32(300), Int32(1), Int32(999)).PackedValue, 0xFF01_FF00)
    }

    func testGameTimeDurationMappingIsExactAtHundredNanoseconds() {
        let value = Framework.GameTime(totalTicks: 10_000_001, elapsedTicks: 166_667, runningSlowly: true)
        XCTAssertEqual(value.TotalGameTime.components.seconds, 1)
        XCTAssertEqual(value.TotalGameTime.components.attoseconds, 100_000_000_000)
        XCTAssertEqual(value.ElapsedGameTime.components.attoseconds, 16_666_700_000_000_000)
        XCTAssertTrue(value.IsRunningSlowly)
    }

    func testKeyboardStateAndPinnedKeys() {
        let state = Framework.Input.KeyboardState([.Escape, .A, .F24, .Escape])
        XCTAssertTrue(state.IsKeyDown(.Escape))
        XCTAssertTrue(state.IsKeyDown(.A))
        XCTAssertTrue(state.IsKeyUp(.B))
        XCTAssertEqual(state[.F24], .Down)
        XCTAssertEqual(state.GetPressedKeys().map(\.rawValue), [27, 65, 135])
        XCTAssertEqual(Framework.Input.Keys.Escape.rawValue, 27)
        XCTAssertEqual(Framework.Input.Keys.ChatPadOrange.rawValue, 203)
    }
}
