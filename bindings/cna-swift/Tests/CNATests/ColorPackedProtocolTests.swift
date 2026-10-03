// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

extension PureValueTests {
    func testColorPackedProtocolsMutateValueTypes() {
        func packedValue<T>(of value: T) -> UInt32
        where T: Framework.Graphics.PackedVector.IPackedVectorOfT, T.TPacked == UInt32 {
            value.PackedValue
        }

        var color = Framework.Color(Int32(0), Int32(0), Int32(0), Int32(0))
        func repack<T>(_ value: inout T, from vector: Framework.Vector4)
        where T: Framework.Graphics.PackedVector.IPackedVector {
            value.PackFromVector4(vector)
        }
        repack(
            &color,
            from: Framework.Vector4(.nan, .infinity, -.infinity, 0.5)
        )
        XCTAssertEqual(color.PackedValue, 0x8000_FF00)
        XCTAssertEqual(packedValue(of: color), 0x8000_FF00)
        XCTAssertEqual(color.ToVector4().W.bitPattern, 0x3F00_8081)
    }

    func testColorPackedValueLayoutAndChannelMutation() {
        var color = Framework.Color(Int32(1), Int32(2), Int32(3), Int32(4))
        XCTAssertEqual(color.PackedValue, 0x0403_0201)
        color.R = 0x11; XCTAssertEqual(color.PackedValue, 0x0403_0211)
        color.G = 0x22; XCTAssertEqual(color.PackedValue, 0x0403_2211)
        color.B = 0x33; XCTAssertEqual(color.PackedValue, 0x0433_2211)
        color.A = 0x44; XCTAssertEqual(color.PackedValue, 0x4433_2211)
        color.PackedValue = 0xDEAD_BEEF
        XCTAssertEqual(color.R, 0xEF)
        XCTAssertEqual(color.G, 0xBE)
        XCTAssertEqual(color.B, 0xAD)
        XCTAssertEqual(color.A, 0xDE)
    }

    func testColorIntegerConstructorsClampWithoutTraps() {
        let values: [(Int32, UInt32)] = [
            (.min, 0), (-1, 0), (0, 0), (1, 0x0101_0101),
            (254, 0xFEFE_FEFE), (255, 0xFFFF_FFFF),
            (256, 0xFFFF_FFFF), (.max, 0xFFFF_FFFF),
        ]
        for (value, expected) in values {
            XCTAssertEqual(
                Framework.Color(value, value, value, value).PackedValue,
                expected
            )
        }
        XCTAssertEqual(Framework.Color(Int32.min, Int32.max, -1).PackedValue, 0xFF00_FF00)
        XCTAssertEqual(Framework.Color(Int32.min, Int32.max, -1, Int32.min).PackedValue, 0x0000_FF00)
    }

    func testColorFloatConstructorsUseXnaUNormRounding() {
        let values: [(Float, UInt32)] = [
            (0, 0x0000_0000), (1, 0xFFFF_FFFF), (0.5, 0x8080_8080),
            (-0.0, 0x0000_0000), (-1, 0x0000_0000), (2, 0xFFFF_FFFF),
            (.nan, 0x0000_0000), (.infinity, 0xFFFF_FFFF),
            (-.infinity, 0x0000_0000), (.leastNonzeroMagnitude, 0x0000_0000),
            (Float(1) / Float(255), 0x0101_0101),
            (Float(127) / Float(255), 0x7F7F_7F7F),
            (Float(128) / Float(255), 0x8080_8080),
            (Float(0.5) / Float(255), 0x0000_0000),
            (Float(1.5) / Float(255), 0x0202_0202),
            (Float(2.5) / Float(255), 0x0202_0202),
        ]
        for (value, expected) in values {
            XCTAssertEqual(
                Framework.Color(value, value, value, value).PackedValue,
                expected
            )
        }
        let halfByte = Float(0.5) / Float(255)
        XCTAssertEqual(Framework.Color(halfByte.nextDown, 0, 0, 0).R, 0)
        XCTAssertEqual(Framework.Color(halfByte.nextUp, 0, 0, 0).R, 1)
        let oneAndAHalfBytes = Float(1.5) / Float(255)
        XCTAssertEqual(Framework.Color(oneAndAHalfBytes.nextDown, 0, 0, 0).R, 1)
        XCTAssertEqual(Framework.Color(oneAndAHalfBytes.nextUp, 0, 0, 0).R, 2)
        XCTAssertEqual(Framework.Color(Float(0.5), 0, 0).PackedValue, 0xFF00_0080)
    }

    func testColorVectorConstructorsAndPackFromVector4() {
        XCTAssertEqual(
            Framework.Color(Framework.Vector3(0, 0.5, 1)).PackedValue,
            0xFFFF_8000
        )
        XCTAssertEqual(
            Framework.Color(Framework.Vector4(0, 0.5, 1, 0.25)).PackedValue,
            0x40FF_8000
        )
        var color = Framework.Color.Black
        color.PackFromVector4(Framework.Vector4(-1, 2, .nan, -.zero))
        XCTAssertEqual(color.PackedValue, 0x0000_FF00)
    }

    func testColorVectorConversionsUseExactBinary32Division() {
        let expected: [(Int32, UInt32)] = [
            (0, 0x0000_0000), (1, 0x3B80_8081), (2, 0x3C00_8081),
            (127, 0x3EFE_FEFF), (128, 0x3F00_8081),
            (254, 0x3F7E_FEFF), (255, 0x3F80_0000),
        ]
        for (channel, bits) in expected {
            let color = Framework.Color(channel, 0, 0, 0)
            XCTAssertEqual(color.ToVector3().X.bitPattern, bits)
            XCTAssertEqual(color.ToVector4().X.bitPattern, bits)
        }
        let color = Framework.Color(Int32(1), Int32(2), Int32(127), Int32(128))
        let vector = color.ToVector4()
        XCTAssertEqual(vector.X.bitPattern, 0x3B80_8081)
        XCTAssertEqual(vector.Y.bitPattern, 0x3C00_8081)
        XCTAssertEqual(vector.Z.bitPattern, 0x3EFE_FEFF)
        XCTAssertEqual(vector.W.bitPattern, 0x3F00_8081)
    }

    func testColorFromNonPremultipliedVectorOperationOrder() {
        let values: [(Framework.Vector4, UInt32)] = [
            (Framework.Vector4(1, 0.5, 0.25, 0), 0x0000_0000),
            (Framework.Vector4(1, 0.5, 0.25, 1), 0xFF40_80FF),
            (Framework.Vector4(1, 0.5, 0.25, 0.5), 0x8020_4080),
            (Framework.Vector4(.nan, .infinity, -.infinity, .nan), 0x0000_0000),
            (Framework.Vector4(.infinity, -.infinity, -0.0, .infinity), 0xFF00_00FF),
        ]
        for (value, expected) in values {
            XCTAssertEqual(Framework.Color.FromNonPremultiplied(value).PackedValue, expected)
        }
    }

    func testColorFromNonPremultipliedIntegerOperationOrder() {
        let values: [(Int32, UInt32)] = [
            (.min, 0x00FF_0000), (-1, 0x0000_0000), (0, 0x0000_0000),
            (1, 0x0100_0001), (127, 0x7F00_3F7F), (128, 0x8000_4080),
            (254, 0xFE00_7FFE), (255, 0xFF00_80FF),
            (256, 0xFF00_80FF), (.max, 0xFF00_FFFF),
        ]
        for (alpha, expected) in values {
            XCTAssertEqual(
                Framework.Color.FromNonPremultiplied(255, g: 128, b: -1, a: alpha).PackedValue,
                expected
            )
        }
        XCTAssertEqual(
            Framework.Color.FromNonPremultiplied(.min, g: .max, b: -1, a: .min).PackedValue,
            0x00FF_00FF
        )
    }

    func testColorLerpUsesXnaFixedPointAsymmetry() {
        let first = Framework.Color(Int32(1), Int32(127), Int32(128), Int32(254))
        let second = Framework.Color(Int32(254), Int32(128), Int32(127), Int32(1))
        let values: [(Float, UInt32)] = [
            (-1, 0xFE80_7F01), (0, 0xFE80_7F01),
            (0.25, 0xBE7F_7F40), (0.5, 0x7F7F_7F7F),
            (0.75, 0x407F_7FBE), (1, 0x017F_80FE), (2, 0x017F_80FE),
            (.nan, 0xFE80_7F01), (.infinity, 0x017F_80FE),
            (-.infinity, 0xFE80_7F01),
        ]
        for (amount, expected) in values {
            XCTAssertEqual(Framework.Color.Lerp(first, value2: second, amount: amount).PackedValue, expected)
        }
    }

    func testColorMultiplyAndOperatorUseXnaSaturation() {
        let color = Framework.Color(Int32(1), Int32(127), Int32(128), Int32(254))
        let values: [(Float, UInt32)] = [
            (-1, 0), (0, 0), (0.5, 0x7F40_3F00), (1, 0xFE80_7F01),
            (2, 0xFFFF_FE02), (1_000_000, 0xFFFF_FFFF),
            (.nan, 0), (.infinity, 0xFFFF_FFFF), (-.infinity, 0),
        ]
        for (scale, expected) in values {
            XCTAssertEqual(Framework.Color.Multiply(color, scale: scale).PackedValue, expected)
            XCTAssertEqual((color * scale).PackedValue, expected)
        }
    }

    func testColorEqualityHashStringAndValueSemantics() {
        let viaChannels = Framework.Color(Int32(239), Int32(190), Int32(173), Int32(222))
        var viaPacked = Framework.Color.Transparent
        viaPacked.PackedValue = 0xDEAD_BEEF
        XCTAssertTrue(viaChannels == viaPacked)
        XCTAssertTrue(viaChannels.Equals(viaPacked))
        XCTAssertTrue(viaChannels.Equals(viaPacked as Any))
        XCTAssertFalse(viaChannels.Equals(nil as Any?))
        XCTAssertFalse(viaChannels != viaPacked)
        viaPacked.R = 0
        XCTAssertFalse(viaChannels == viaPacked)
        XCTAssertEqual(viaChannels.GetHashCode(), -559_038_737)
        XCTAssertEqual(viaChannels.ToString(), "{R:239 G:190 B:173 A:222}")

        var highBit = Framework.Color.Transparent; highBit.PackedValue = 0x8000_0000
        XCTAssertEqual(highBit.GetHashCode(), Int32.min)
        var allBits = Framework.Color.Transparent; allBits.PackedValue = 0xFFFF_FFFF
        XCTAssertEqual(allBits.GetHashCode(), -1)

        var first = Framework.Color.Red
        var second = Framework.Color.Red
        first.R = 0
        XCTAssertEqual(second.PackedValue, 0xFF00_00FF)
        XCTAssertEqual(first.PackedValue, 0xFF00_0000)
        second.G = 1
        XCTAssertEqual(Framework.Color.Red.PackedValue, 0xFF00_00FF)
    }

    func testColorFullXnaGoldenPalette() {
        let golden: [(String, Framework.Color, UInt32)] = [
            ("Transparent", .Transparent, 0x00000000),
            ("AliceBlue", .AliceBlue, 0xFFFFF8F0),
            ("AntiqueWhite", .AntiqueWhite, 0xFFD7EBFA),
            ("Aqua", .Aqua, 0xFFFFFF00),
            ("Aquamarine", .Aquamarine, 0xFFD4FF7F),
            ("Azure", .Azure, 0xFFFFFFF0),
            ("Beige", .Beige, 0xFFDCF5F5),
            ("Bisque", .Bisque, 0xFFC4E4FF),
            ("Black", .Black, 0xFF000000),
            ("BlanchedAlmond", .BlanchedAlmond, 0xFFCDEBFF),
            ("Blue", .Blue, 0xFFFF0000),
            ("BlueViolet", .BlueViolet, 0xFFE22B8A),
            ("Brown", .Brown, 0xFF2A2AA5),
            ("BurlyWood", .BurlyWood, 0xFF87B8DE),
            ("CadetBlue", .CadetBlue, 0xFFA09E5F),
            ("Chartreuse", .Chartreuse, 0xFF00FF7F),
            ("Chocolate", .Chocolate, 0xFF1E69D2),
            ("Coral", .Coral, 0xFF507FFF),
            ("CornflowerBlue", .CornflowerBlue, 0xFFED9564),
            ("Cornsilk", .Cornsilk, 0xFFDCF8FF),
            ("Crimson", .Crimson, 0xFF3C14DC),
            ("Cyan", .Cyan, 0xFFFFFF00),
            ("DarkBlue", .DarkBlue, 0xFF8B0000),
            ("DarkCyan", .DarkCyan, 0xFF8B8B00),
            ("DarkGoldenrod", .DarkGoldenrod, 0xFF0B86B8),
            ("DarkGray", .DarkGray, 0xFFA9A9A9),
            ("DarkGreen", .DarkGreen, 0xFF006400),
            ("DarkKhaki", .DarkKhaki, 0xFF6BB7BD),
            ("DarkMagenta", .DarkMagenta, 0xFF8B008B),
            ("DarkOliveGreen", .DarkOliveGreen, 0xFF2F6B55),
            ("DarkOrange", .DarkOrange, 0xFF008CFF),
            ("DarkOrchid", .DarkOrchid, 0xFFCC3299),
            ("DarkRed", .DarkRed, 0xFF00008B),
            ("DarkSalmon", .DarkSalmon, 0xFF7A96E9),
            ("DarkSeaGreen", .DarkSeaGreen, 0xFF8BBC8F),
            ("DarkSlateBlue", .DarkSlateBlue, 0xFF8B3D48),
            ("DarkSlateGray", .DarkSlateGray, 0xFF4F4F2F),
            ("DarkTurquoise", .DarkTurquoise, 0xFFD1CE00),
            ("DarkViolet", .DarkViolet, 0xFFD30094),
            ("DeepPink", .DeepPink, 0xFF9314FF),
            ("DeepSkyBlue", .DeepSkyBlue, 0xFFFFBF00),
            ("DimGray", .DimGray, 0xFF696969),
            ("DodgerBlue", .DodgerBlue, 0xFFFF901E),
            ("Firebrick", .Firebrick, 0xFF2222B2),
            ("FloralWhite", .FloralWhite, 0xFFF0FAFF),
            ("ForestGreen", .ForestGreen, 0xFF228B22),
            ("Fuchsia", .Fuchsia, 0xFFFF00FF),
            ("Gainsboro", .Gainsboro, 0xFFDCDCDC),
            ("GhostWhite", .GhostWhite, 0xFFFFF8F8),
            ("Gold", .Gold, 0xFF00D7FF),
            ("Goldenrod", .Goldenrod, 0xFF20A5DA),
            ("Gray", .Gray, 0xFF808080),
            ("Green", .Green, 0xFF008000),
            ("GreenYellow", .GreenYellow, 0xFF2FFFAD),
            ("Honeydew", .Honeydew, 0xFFF0FFF0),
            ("HotPink", .HotPink, 0xFFB469FF),
            ("IndianRed", .IndianRed, 0xFF5C5CCD),
            ("Indigo", .Indigo, 0xFF82004B),
            ("Ivory", .Ivory, 0xFFF0FFFF),
            ("Khaki", .Khaki, 0xFF8CE6F0),
            ("Lavender", .Lavender, 0xFFFAE6E6),
            ("LavenderBlush", .LavenderBlush, 0xFFF5F0FF),
            ("LawnGreen", .LawnGreen, 0xFF00FC7C),
            ("LemonChiffon", .LemonChiffon, 0xFFCDFAFF),
            ("LightBlue", .LightBlue, 0xFFE6D8AD),
            ("LightCoral", .LightCoral, 0xFF8080F0),
            ("LightCyan", .LightCyan, 0xFFFFFFE0),
            ("LightGoldenrodYellow", .LightGoldenrodYellow, 0xFFD2FAFA),
            ("LightGreen", .LightGreen, 0xFF90EE90),
            ("LightGray", .LightGray, 0xFFD3D3D3),
            ("LightPink", .LightPink, 0xFFC1B6FF),
            ("LightSalmon", .LightSalmon, 0xFF7AA0FF),
            ("LightSeaGreen", .LightSeaGreen, 0xFFAAB220),
            ("LightSkyBlue", .LightSkyBlue, 0xFFFACE87),
            ("LightSlateGray", .LightSlateGray, 0xFF998877),
            ("LightSteelBlue", .LightSteelBlue, 0xFFDEC4B0),
            ("LightYellow", .LightYellow, 0xFFE0FFFF),
            ("Lime", .Lime, 0xFF00FF00),
            ("LimeGreen", .LimeGreen, 0xFF32CD32),
            ("Linen", .Linen, 0xFFE6F0FA),
            ("Magenta", .Magenta, 0xFFFF00FF),
            ("Maroon", .Maroon, 0xFF000080),
            ("MediumAquamarine", .MediumAquamarine, 0xFFAACD66),
            ("MediumBlue", .MediumBlue, 0xFFCD0000),
            ("MediumOrchid", .MediumOrchid, 0xFFD355BA),
            ("MediumPurple", .MediumPurple, 0xFFDB7093),
            ("MediumSeaGreen", .MediumSeaGreen, 0xFF71B33C),
            ("MediumSlateBlue", .MediumSlateBlue, 0xFFEE687B),
            ("MediumSpringGreen", .MediumSpringGreen, 0xFF9AFA00),
            ("MediumTurquoise", .MediumTurquoise, 0xFFCCD148),
            ("MediumVioletRed", .MediumVioletRed, 0xFF8515C7),
            ("MidnightBlue", .MidnightBlue, 0xFF701919),
            ("MintCream", .MintCream, 0xFFFAFFF5),
            ("MistyRose", .MistyRose, 0xFFE1E4FF),
            ("Moccasin", .Moccasin, 0xFFB5E4FF),
            ("NavajoWhite", .NavajoWhite, 0xFFADDEFF),
            ("Navy", .Navy, 0xFF800000),
            ("OldLace", .OldLace, 0xFFE6F5FD),
            ("Olive", .Olive, 0xFF008080),
            ("OliveDrab", .OliveDrab, 0xFF238E6B),
            ("Orange", .Orange, 0xFF00A5FF),
            ("OrangeRed", .OrangeRed, 0xFF0045FF),
            ("Orchid", .Orchid, 0xFFD670DA),
            ("PaleGoldenrod", .PaleGoldenrod, 0xFFAAE8EE),
            ("PaleGreen", .PaleGreen, 0xFF98FB98),
            ("PaleTurquoise", .PaleTurquoise, 0xFFEEEEAF),
            ("PaleVioletRed", .PaleVioletRed, 0xFF9370DB),
            ("PapayaWhip", .PapayaWhip, 0xFFD5EFFF),
            ("PeachPuff", .PeachPuff, 0xFFB9DAFF),
            ("Peru", .Peru, 0xFF3F85CD),
            ("Pink", .Pink, 0xFFCBC0FF),
            ("Plum", .Plum, 0xFFDDA0DD),
            ("PowderBlue", .PowderBlue, 0xFFE6E0B0),
            ("Purple", .Purple, 0xFF800080),
            ("Red", .Red, 0xFF0000FF),
            ("RosyBrown", .RosyBrown, 0xFF8F8FBC),
            ("RoyalBlue", .RoyalBlue, 0xFFE16941),
            ("SaddleBrown", .SaddleBrown, 0xFF13458B),
            ("Salmon", .Salmon, 0xFF7280FA),
            ("SandyBrown", .SandyBrown, 0xFF60A4F4),
            ("SeaGreen", .SeaGreen, 0xFF578B2E),
            ("SeaShell", .SeaShell, 0xFFEEF5FF),
            ("Sienna", .Sienna, 0xFF2D52A0),
            ("Silver", .Silver, 0xFFC0C0C0),
            ("SkyBlue", .SkyBlue, 0xFFEBCE87),
            ("SlateBlue", .SlateBlue, 0xFFCD5A6A),
            ("SlateGray", .SlateGray, 0xFF908070),
            ("Snow", .Snow, 0xFFFAFAFF),
            ("SpringGreen", .SpringGreen, 0xFF7FFF00),
            ("SteelBlue", .SteelBlue, 0xFFB48246),
            ("Tan", .Tan, 0xFF8CB4D2),
            ("Teal", .Teal, 0xFF808000),
            ("Thistle", .Thistle, 0xFFD8BFD8),
            ("Tomato", .Tomato, 0xFF4763FF),
            ("Turquoise", .Turquoise, 0xFFD0E040),
            ("Violet", .Violet, 0xFFEE82EE),
            ("Wheat", .Wheat, 0xFFB3DEF5),
            ("White", .White, 0xFFFFFFFF),
            ("WhiteSmoke", .WhiteSmoke, 0xFFF5F5F5),
            ("Yellow", .Yellow, 0xFF00FFFF),
            ("YellowGreen", .YellowGreen, 0xFF32CD9A),
        ]
        XCTAssertEqual(golden.count, 141)
        for (name, color, packed) in golden {
            XCTAssertEqual(color.PackedValue, packed, name)
            XCTAssertEqual(color.R, UInt8(truncatingIfNeeded: packed), name)
            XCTAssertEqual(color.G, UInt8(truncatingIfNeeded: packed >> 8), name)
            XCTAssertEqual(color.B, UInt8(truncatingIfNeeded: packed >> 16), name)
            XCTAssertEqual(color.A, UInt8(truncatingIfNeeded: packed >> 24), name)
        }
    }
}
