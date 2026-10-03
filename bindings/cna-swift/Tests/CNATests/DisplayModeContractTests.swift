// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Every value below is retained from the pinned Microsoft XNA Framework 4.0
// Windows runtime. Structure, accessibility and member closure come from the
// hash-matched Microsoft.Xna.Framework.Graphics.dll metadata/IL; the observed
// binary32 and string results come from executing that IL's exact instruction
// sequence on the CLR. See docs/display-mode-evidence.md.
extension PureValueTests {
    private typealias Mode = Microsoft.Xna.Framework.Graphics.DisplayMode
    private typealias ModeFormat = Microsoft.Xna.Framework.Graphics.SurfaceFormat

    func testDisplayModePropertiesXnaContract() {
        // The assembly-accessible .ctor(int32 width, int32 height,
        // SurfaceFormat format) stores its three arguments verbatim; there is
        // no validation, normalisation, clamping or reordering in the IL.
        let stored: [(width: Int32, height: Int32, format: ModeFormat)] = [
            (800, 480, .Color),
            (1920, 1080, .Color),
            (1024, 768, .Color),
            (1280, 720, .Color),
            (1366, 768, .Color),
            (1600, 900, .Color),
            (640, 480, .Color),
            (1, 3, .Color),
            (1023, 769, .Color),
            (7, 13, .Color),
            (3, 7, .Color),
            (16777217, 1, .Color),
            (16777217, 3, .Color),
            (0, 480, .Color),
            (800, 0, .Color),
            (0, 0, .Color),
            (-800, 480, .Color),
            (800, -480, .Color),
            (-800, -480, .Color),
            (-1, -1, .Color),
            (-1, 3, .Color),
            (Int32.max, 1, .Color),
            (1, Int32.max, .Color),
            (Int32.max, Int32.max, .Color),
            (Int32.min, 1, .Color),
            (1, Int32.min, .Color),
            (Int32.min, Int32.min, .Color),
            (Int32.max, Int32.min, .Color),
            (Int32.min, Int32.max, .Color),
        ]
        XCTAssertEqual(stored.count, 29)
        for entry in stored {
            let mode = Mode(width: entry.width, height: entry.height, format: entry.format)
            XCTAssertEqual(mode.Width, entry.width)
            XCTAssertEqual(mode.Height, entry.height)
            XCTAssertEqual(mode.Format, entry.format)
        }

        // The stored SurfaceFormat is preserved exactly for every pinned literal.
        let formats: [(value: ModeFormat, rawValue: Int32)] = [
            (.Color, 0),
            (.Bgr565, 1),
            (.Bgra5551, 2),
            (.Bgra4444, 3),
            (.Dxt1, 4),
            (.Dxt3, 5),
            (.Dxt5, 6),
            (.NormalizedByte2, 7),
            (.NormalizedByte4, 8),
            (.Rgba1010102, 9),
            (.Rg32, 10),
            (.Rgba64, 11),
            (.Alpha8, 12),
            (.Single, 13),
            (.Vector2, 14),
            (.Vector4, 15),
            (.HalfSingle, 16),
            (.HalfVector2, 17),
            (.HalfVector4, 18),
            (.HdrBlendable, 19),
        ]
        XCTAssertEqual(formats.count, 20)
        for entry in formats {
            let mode = Mode(width: 800, height: 480, format: entry.value)
            XCTAssertEqual(mode.Format, entry.value)
            XCTAssertEqual(mode.Format.rawValue, entry.rawValue)
        }
    }

    func testDisplayModeAspectRatioXnaContract() {
        // get_AspectRatio returns +0 when either stored dimension is zero and
        // otherwise divides two conv.r4 conversions in the binary32 domain.
        let observations: [(width: Int32, height: Int32, bitPattern: UInt32)] = [
            (800, 480, 0x3FD55555),
            (1920, 1080, 0x3FE38E39),
            (1024, 768, 0x3FAAAAAB),
            (1280, 720, 0x3FE38E39),
            (1366, 768, 0x3FE3AAAB),
            (1600, 900, 0x3FE38E39),
            (640, 480, 0x3FAAAAAB),
            (1, 3, 0x3EAAAAAB),
            (1023, 769, 0x3FAA473E),
            (7, 13, 0x3F09D89E),
            (3, 7, 0x3EDB6DB7),
            (16777217, 1, 0x4B800000),
            (16777217, 3, 0x4AAAAAAB),
            (0, 480, 0x00000000),
            (800, 0, 0x00000000),
            (0, 0, 0x00000000),
            (-800, 480, 0xBFD55555),
            (800, -480, 0xBFD55555),
            (-800, -480, 0x3FD55555),
            (-1, -1, 0x3F800000),
            (-1, 3, 0xBEAAAAAB),
            (Int32.max, 1, 0x4F000000),
            (1, Int32.max, 0x30000000),
            (Int32.max, Int32.max, 0x3F800000),
            (Int32.min, 1, 0xCF000000),
            (1, Int32.min, 0xB0000000),
            (Int32.min, Int32.min, 0x3F800000),
            (Int32.max, Int32.min, 0xBF800000),
            (Int32.min, Int32.max, 0xBF800000),
        ]
        XCTAssertEqual(observations.count, 29)
        for entry in observations {
            let mode = Mode(width: entry.width, height: entry.height, format: .Color)
            XCTAssertEqual(mode.AspectRatio.bitPattern, entry.bitPattern)
        }
    }

    func testDisplayModeTitleSafeAreaXnaContract() {
        // get_TitleSafeArea calls the internal Viewport.GetTitleSafeArea(0, 0,
        // width, height), which on the Windows runtime is exactly
        // new Rectangle(x, y, w, h). No overscan inset is applied.
        let observations: [(width: Int32, height: Int32,
                            x: Int32, y: Int32, w: Int32, h: Int32)] = [
            (800, 480, 0, 0, 800, 480),
            (1920, 1080, 0, 0, 1920, 1080),
            (1024, 768, 0, 0, 1024, 768),
            (1280, 720, 0, 0, 1280, 720),
            (1366, 768, 0, 0, 1366, 768),
            (1600, 900, 0, 0, 1600, 900),
            (640, 480, 0, 0, 640, 480),
            (1, 3, 0, 0, 1, 3),
            (1023, 769, 0, 0, 1023, 769),
            (7, 13, 0, 0, 7, 13),
            (3, 7, 0, 0, 3, 7),
            (16777217, 1, 0, 0, 16777217, 1),
            (16777217, 3, 0, 0, 16777217, 3),
            (0, 480, 0, 0, 0, 480),
            (800, 0, 0, 0, 800, 0),
            (0, 0, 0, 0, 0, 0),
            (-800, 480, 0, 0, -800, 480),
            (800, -480, 0, 0, 800, -480),
            (-800, -480, 0, 0, -800, -480),
            (-1, -1, 0, 0, -1, -1),
            (-1, 3, 0, 0, -1, 3),
            (Int32.max, 1, 0, 0, Int32.max, 1),
            (1, Int32.max, 0, 0, 1, Int32.max),
            (Int32.max, Int32.max, 0, 0, Int32.max, Int32.max),
            (Int32.min, 1, 0, 0, Int32.min, 1),
            (1, Int32.min, 0, 0, 1, Int32.min),
            (Int32.min, Int32.min, 0, 0, Int32.min, Int32.min),
            (Int32.max, Int32.min, 0, 0, Int32.max, Int32.min),
            (Int32.min, Int32.max, 0, 0, Int32.min, Int32.max),
        ]
        XCTAssertEqual(observations.count, 29)
        for entry in observations {
            let area = Mode(width: entry.width, height: entry.height, format: .Color).TitleSafeArea
            XCTAssertEqual(area.X, entry.x)
            XCTAssertEqual(area.Y, entry.y)
            XCTAssertEqual(area.Width, entry.w)
            XCTAssertEqual(area.Height, entry.h)
        }
    }

    func testDisplayModeToStringXnaContract() {
        // string.Format(CultureInfo.CurrentCulture,
        //   "{{Width:{0} Height:{1} Format:{2} AspectRatio:{3}}}", ...).
        // The doubled braces are composite-format escapes, so one brace pair is
        // emitted. Retained strings are the invariant-culture rendering.
        let observations: [(width: Int32, height: Int32, text: String)] = [
            (800, 480, "{Width:800 Height:480 Format:Color AspectRatio:1.666667}"),
            (1920, 1080, "{Width:1920 Height:1080 Format:Color AspectRatio:1.777778}"),
            (1024, 768, "{Width:1024 Height:768 Format:Color AspectRatio:1.333333}"),
            (1280, 720, "{Width:1280 Height:720 Format:Color AspectRatio:1.777778}"),
            (1366, 768, "{Width:1366 Height:768 Format:Color AspectRatio:1.778646}"),
            (1600, 900, "{Width:1600 Height:900 Format:Color AspectRatio:1.777778}"),
            (640, 480, "{Width:640 Height:480 Format:Color AspectRatio:1.333333}"),
            (1, 3, "{Width:1 Height:3 Format:Color AspectRatio:0.3333333}"),
            (1023, 769, "{Width:1023 Height:769 Format:Color AspectRatio:1.330299}"),
            (7, 13, "{Width:7 Height:13 Format:Color AspectRatio:0.5384616}"),
            (3, 7, "{Width:3 Height:7 Format:Color AspectRatio:0.4285714}"),
            (16777217, 1, "{Width:16777217 Height:1 Format:Color AspectRatio:1.677722E+07}"),
            (16777217, 3, "{Width:16777217 Height:3 Format:Color AspectRatio:5592406}"),
            (0, 480, "{Width:0 Height:480 Format:Color AspectRatio:0}"),
            (800, 0, "{Width:800 Height:0 Format:Color AspectRatio:0}"),
            (0, 0, "{Width:0 Height:0 Format:Color AspectRatio:0}"),
            (-800, 480, "{Width:-800 Height:480 Format:Color AspectRatio:-1.666667}"),
            (800, -480, "{Width:800 Height:-480 Format:Color AspectRatio:-1.666667}"),
            (-800, -480, "{Width:-800 Height:-480 Format:Color AspectRatio:1.666667}"),
            (-1, -1, "{Width:-1 Height:-1 Format:Color AspectRatio:1}"),
            (-1, 3, "{Width:-1 Height:3 Format:Color AspectRatio:-0.3333333}"),
            (Int32.max, 1, "{Width:2147483647 Height:1 Format:Color AspectRatio:2.147484E+09}"),
            (1, Int32.max, "{Width:1 Height:2147483647 Format:Color AspectRatio:4.656613E-10}"),
            (Int32.max, Int32.max, "{Width:2147483647 Height:2147483647 Format:Color AspectRatio:1}"),
            (Int32.min, 1, "{Width:-2147483648 Height:1 Format:Color AspectRatio:-2.147484E+09}"),
            (1, Int32.min, "{Width:1 Height:-2147483648 Format:Color AspectRatio:-4.656613E-10}"),
            (Int32.min, Int32.min, "{Width:-2147483648 Height:-2147483648 Format:Color AspectRatio:1}"),
            (Int32.max, Int32.min, "{Width:2147483647 Height:-2147483648 Format:Color AspectRatio:-1}"),
            (Int32.min, Int32.max, "{Width:-2147483648 Height:2147483647 Format:Color AspectRatio:-1}"),
        ]
        XCTAssertEqual(observations.count, 29)
        for entry in observations {
            let mode = Mode(width: entry.width, height: entry.height, format: .Color)
            XCTAssertEqual(mode.ToString(), entry.text)
        }

        // The boxed SurfaceFormat renders its declared CLR literal name.
        let formats: [(value: ModeFormat, text: String)] = [
            (.Color, "{Width:800 Height:480 Format:Color AspectRatio:1.666667}"),
            (.Bgr565, "{Width:800 Height:480 Format:Bgr565 AspectRatio:1.666667}"),
            (.Bgra5551, "{Width:800 Height:480 Format:Bgra5551 AspectRatio:1.666667}"),
            (.Bgra4444, "{Width:800 Height:480 Format:Bgra4444 AspectRatio:1.666667}"),
            (.Dxt1, "{Width:800 Height:480 Format:Dxt1 AspectRatio:1.666667}"),
            (.Dxt3, "{Width:800 Height:480 Format:Dxt3 AspectRatio:1.666667}"),
            (.Dxt5, "{Width:800 Height:480 Format:Dxt5 AspectRatio:1.666667}"),
            (.NormalizedByte2, "{Width:800 Height:480 Format:NormalizedByte2 AspectRatio:1.666667}"),
            (.NormalizedByte4, "{Width:800 Height:480 Format:NormalizedByte4 AspectRatio:1.666667}"),
            (.Rgba1010102, "{Width:800 Height:480 Format:Rgba1010102 AspectRatio:1.666667}"),
            (.Rg32, "{Width:800 Height:480 Format:Rg32 AspectRatio:1.666667}"),
            (.Rgba64, "{Width:800 Height:480 Format:Rgba64 AspectRatio:1.666667}"),
            (.Alpha8, "{Width:800 Height:480 Format:Alpha8 AspectRatio:1.666667}"),
            (.Single, "{Width:800 Height:480 Format:Single AspectRatio:1.666667}"),
            (.Vector2, "{Width:800 Height:480 Format:Vector2 AspectRatio:1.666667}"),
            (.Vector4, "{Width:800 Height:480 Format:Vector4 AspectRatio:1.666667}"),
            (.HalfSingle, "{Width:800 Height:480 Format:HalfSingle AspectRatio:1.666667}"),
            (.HalfVector2, "{Width:800 Height:480 Format:HalfVector2 AspectRatio:1.666667}"),
            (.HalfVector4, "{Width:800 Height:480 Format:HalfVector4 AspectRatio:1.666667}"),
            (.HdrBlendable, "{Width:800 Height:480 Format:HdrBlendable AspectRatio:1.666667}"),
        ]
        XCTAssertEqual(formats.count, 20)
        for entry in formats {
            XCTAssertEqual(Mode(width: 800, height: 480, format: entry.value).ToString(), entry.text)
        }
    }
}
