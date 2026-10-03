// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Swift language projection qualification for the DisplayMode class mapping.
// These are Swift/CLR mapping facts, not XNA runtime observations, so they are
// deliberately kept out of the pure XNA-derived behaviour corpus.
final class DisplayModeProjectionTests: XCTestCase {
    typealias Mode = Microsoft.Xna.Framework.Graphics.DisplayMode
    typealias Format = Microsoft.Xna.Framework.Graphics.SurfaceFormat
    typealias Rectangle = Microsoft.Xna.Framework.Rectangle

    func testInternalConstructionIsTheOnlyRouteAndPublicStateIsImmutable() {
        // The CLR constructor is `assembly` accessible, so the Swift mapping
        // exposes construction only inside this module. A consumer of the
        // package cannot reach this initializer; the compiler Symbol Graph is
        // the authority and contains no public DisplayMode init.
        let mode = Mode(width: 1280, height: 720, format: .Bgra4444)
        XCTAssertEqual(mode.Width, 1280)
        XCTAssertEqual(mode.Height, 720)
        XCTAssertEqual(mode.Format, .Bgra4444)

        // Reading the derived properties repeatedly cannot disturb the stored
        // state: there is no lazily initialised or cached backing object.
        let firstRatio = mode.AspectRatio
        let firstArea = mode.TitleSafeArea
        for _ in 0..<3 {
            XCTAssertEqual(mode.AspectRatio.bitPattern, firstRatio.bitPattern)
            XCTAssertEqual(mode.TitleSafeArea.X, firstArea.X)
            XCTAssertEqual(mode.TitleSafeArea.Y, firstArea.Y)
            XCTAssertEqual(mode.TitleSafeArea.Width, firstArea.Width)
            XCTAssertEqual(mode.TitleSafeArea.Height, firstArea.Height)
        }
        XCTAssertEqual(firstArea.Width, 1280)
        XCTAssertEqual(firstArea.Height, 720)
        XCTAssertEqual(mode.Width, 1280)
        XCTAssertEqual(mode.Height, 720)
        XCTAssertEqual(mode.Format, .Bgra4444)
    }

    func testClrClassMapsToSwiftReferenceIdentitySemantics() {
        // A CLR class is a reference type: assignment aliases one instance and
        // two separately constructed instances stay distinct. This is language
        // mapping behaviour; DisplayMode declares no XNA equality identity, so
        // no value comparison is available or implied.
        let first = Mode(width: 800, height: 480, format: .Color)
        let alias = first
        let second = Mode(width: 800, height: 480, format: .Color)

        XCTAssertTrue(first === alias)
        XCTAssertFalse(first === second)
        XCTAssertTrue(alias.Width == second.Width)
        XCTAssertTrue(alias.Height == second.Height)
        XCTAssertTrue(alias.Format == second.Format)
        XCTAssertTrue(ObjectIdentifier(first) == ObjectIdentifier(alias))
        XCTAssertFalse(ObjectIdentifier(first) == ObjectIdentifier(second))
    }

    func testTitleSafeAreaHasIndependentRectangleValueSemantics() {
        // Rectangle is a Swift struct, so each access yields an independent
        // value. Mutating a retrieved copy cannot reach the DisplayMode, and a
        // second access returns the original rectangle again.
        let mode = Mode(width: 800, height: 480, format: .Color)
        var copy: Rectangle = mode.TitleSafeArea
        copy.X = 42
        copy.Y = 43
        copy.Width = 7
        copy.Height = 9

        let fresh: Rectangle = mode.TitleSafeArea
        XCTAssertEqual(copy.X, 42)
        XCTAssertEqual(copy.Width, 7)
        XCTAssertEqual(fresh.X, 0)
        XCTAssertEqual(fresh.Y, 0)
        XCTAssertEqual(fresh.Width, 800)
        XCTAssertEqual(fresh.Height, 480)
        XCTAssertEqual(mode.Width, 800)
        XCTAssertEqual(mode.Height, 480)
    }

    func testAspectRatioStaysInTheBinary32Domain() {
        // The IL divides two conv.r4 results. Computing in Double and
        // narrowing afterwards is a different operation, so the projection is
        // pinned to the binary32 quotient.
        let cases: [(width: Int32, height: Int32)] = [
            (800, 480), (1920, 1080), (1024, 768), (1023, 769), (7, 13),
            (16777217, 3), (Int32.max, 3), (Int32.min, 7),
        ]
        for entry in cases {
            let mode = Mode(width: entry.width, height: entry.height, format: .Color)
            XCTAssertEqual(
                mode.AspectRatio.bitPattern,
                (Float(entry.width) / Float(entry.height)).bitPattern
            )
        }

        // Either stored dimension being zero short-circuits to positive zero
        // before any division happens, so no infinity or NaN is produced.
        for entry in [(Int32(1), Int32(0)), (0, 1), (0, 0), (-1, 0), (Int32.max, 0)] {
            let ratio = Mode(width: entry.0, height: entry.1, format: .Color).AspectRatio
            XCTAssertEqual(ratio.bitPattern, Float(0).bitPattern)
            XCTAssertFalse(ratio.isNaN)
            XCTAssertFalse(ratio.isInfinite)
            XCTAssertFalse(ratio.sign == .minus)
        }
    }

    func testManagedDescriptorNeedsNoNativeRuntime() {
        // Nothing in this type touches CNAShim, a native handle, a graphics
        // device, or a display query, so the whole contract is exercised with
        // no native library present.
        let mode = Mode(width: 1366, height: 768, format: .HdrBlendable)
        XCTAssertEqual(mode.ToString(), "{Width:1366 Height:768 Format:HdrBlendable AspectRatio:1.778646}")
        XCTAssertEqual(mode.TitleSafeArea.Width, 1366)
        XCTAssertEqual(mode.AspectRatio.bitPattern, 0x3FE3AAAB)
    }
}
