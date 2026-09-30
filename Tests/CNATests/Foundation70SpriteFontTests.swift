// SPDX-License-Identifier: MIT

import CNAShim
import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class FontProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((FontProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (FontProbeGame, G.GraphicsDevice) throws -> Void) throws {
        try super.init()
        self.body = body
        manager = try F.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        do {
            guard let device = try GraphicsDevice else {
                throw CNAError.producerInvariant("no device")
            }
            try body?(self, device)
        } catch {
            failure = error
        }
        try Exit()
    }

    override func Update(_ gameTime: F.GameTime) throws { try Exit() }
}

/// Foundation 70: `SpriteFont` and `SpriteBatch.DrawString`.
///
/// **A font needs no content pipeline here.** `cna_sprite_font_create` takes a
/// texture and a complete glyph table, so the fixture below invents three
/// glyphs and every expected measurement is computed by hand from
/// `SpriteFont.InternalMeasure`'s IL. That is what makes the measurement an
/// assertion rather than a comparison against whatever the native route
/// happens to answer.
///
/// The glyph table is chosen to make the awkward branches observable: `'A'`
/// has a **negative** left bearing, so the first-glyph-of-line clamp shows;
/// the three cropping heights differ, so the height max shows; and the right
/// bearings differ, so the tail addition shows.
final class Foundation70SpriteFontTests: XCTestCase {
    private func requireNative() throws {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
    }

    private func run(
        _ body: @escaping (FontProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> FontProbeGame {
        let game = try FontProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    /// The fixture: `A`, `B`, `C` over a 32x8 atlas.
    ///
    /// | char | crop height | kerning (left, width, right) |
    /// |---|---:|---|
    /// | `A` | 6 | (-2, 5, 1) |
    /// | `B` | 7 | (1, 4, 2) |
    /// | `C` | 9 | (0.5, 3, 0) |
    ///
    /// `LineSpacing = 10`, `Spacing = 1.5`, `DefaultCharacter = 'B'`.
    /// One glyph of a fixture font: character, atlas X, cropping height, and
    /// the three kerning values XNA reads as left bearing, width and right
    /// bearing.
    private typealias GlyphSpec = (Character, Int32, Int32, Float, Float, Float)

    /// The default fixture: `A`, `B`, `C` over a 32x8 atlas.
    ///
    /// | char | crop height | kerning (left, width, right) |
    /// |---|---:|---|
    /// | `A` | 6 | (-2, 5, 1) |
    /// | `B` | 7 | (1, 4, 2) |
    /// | `C` | 9 | (0.5, 3, 0) |
    ///
    /// `LineSpacing = 10`, `Spacing = 1.5`, `DefaultCharacter = 'B'`.
    private static let defaultGlyphs: [GlyphSpec] = [
        ("A", 0, 6, -2, 5, 1),
        ("B", 8, 7, 1, 4, 2),
        ("C", 16, 9, 0.5, 3, 0),
    ]

    private func makeFont(
        _ device: G.GraphicsDevice,
        glyphs specs: [GlyphSpec]? = nil,
        lineSpacing: Int32 = 10,
        spacing: Float = 1.5,
        defaultCharacter: UInt16? = UInt16(UInt8(ascii: "B"))
    ) throws -> (G.SpriteFont, G.Texture2D) {
        let atlas = try G.Texture2D(graphicsDevice: device, width: 64, height: 8)
        let glyphs = (specs ?? Foundation70SpriteFontTests.defaultGlyphs).map {
            spec -> CNASwift_SpriteFontGlyph in
            var g = CNASwift_SpriteFontGlyph()
            g.struct_size = UInt32(MemoryLayout<CNASwift_SpriteFontGlyph>.size)
            g.struct_version = 1
            g.glyph_bounds = CNASwift_Rectangle(
                x: spec.1, y: 0, width: 8, height: 8)
            g.cropping = CNASwift_Rectangle(
                x: 0, y: 0, width: 8, height: spec.2)
            g.character = UInt16(truncatingIfNeeded:
                spec.0.unicodeScalars.first!.value)
            g.kerning = CNASwift_Vector3(x: spec.3, y: spec.4, z: spec.5)
            return g
        }
        let runtime = device.runtimeState
        var handle: UInt64 = 0
        try glyphs.withUnsafeBufferPointer { buffer in
            var info = CNASwift_SpriteFontCreateInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_SpriteFontCreateInfo>.size)
            info.struct_version = 1
            info.texture = try atlas.validatedHandle("atlas")
            info.glyphs = buffer.baseAddress
            info.glyph_count = UInt64(buffer.count)
            info.line_spacing = lineSpacing
            info.spacing = spacing
            info.default_character = defaultCharacter ?? 0
            info.has_default_character = defaultCharacter == nil ? 0 : 1
            try runtime.functions.check(
                runtime.functions.spriteFontCreate(&info, &handle),
                operation: "cna_sprite_font_create")
        }
        let font = try G.SpriteFont(
            box: SpriteFontBox(handle: handle, runtime: runtime), texture: atlas)
        return (font, atlas)
    }

    // ------------------------------------------------------------------
    // The properties.

    /// What the font reports back is what it was built from, and the glyph
    /// table round-trips in the order `Characters` reports.
    func testAFontReportsWhatItWasBuiltFrom() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            game.observations["lineSpacing"] = "\(font.LineSpacing)"
            game.observations["spacing"] = "\(font.Spacing)"
            game.observations["default"] = "\(font.DefaultCharacter ?? 0)"
            var spelled = ""
            if let characters = font.Characters {
                for index in 0..<characters.Count {
                    spelled += String(UnicodeScalar(try characters.Item(index))!)
                }
            }
            game.observations["characters"] = spelled
            game.observations["count"] = "\(font.Characters?.Count ?? -1)"
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertEqual(game.observations["lineSpacing"], "10")
        XCTAssertEqual(game.observations["spacing"], "1.5")
        XCTAssertEqual(game.observations["default"], "\(UInt16(UInt8(ascii: "B")))")
        XCTAssertEqual(game.observations["characters"], "ABC")
        XCTAssertEqual(game.observations["count"], "3")
    }

    /// `Characters` is built once and cached, so the collection has identity —
    /// XNA's getter is a null test and a lazy `newobj` over the same field.
    func testCharactersIsTheSameCollectionEveryTime() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            game.observations["same"] = "\(font.Characters === font.Characters)"
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertEqual(game.observations["same"], "true")
    }

    /// `LineSpacing` and `Spacing` are plain field writes in XNA and plain
    /// property writes here — and they change what the next measurement says.
    func testTheLayoutPropertiesChangeTheMeasurement() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            game.observations["before"] = "\(try font.MeasureString("AB"))"
            font.Spacing = 3
            game.observations["wider"] = "\(try font.MeasureString("AB"))"
            font.LineSpacing = 20
            game.observations["taller"] = "\(try font.MeasureString("AB"))"
            font.box.release()
            try atlas.Dispose()
        }
        // "AB" with spacing 1.5: 0 + 5, then +1.5 +1 (right bearing) +1 +4,
        // then the tail's +2 -> 14.5 wide, 10 tall.
        XCTAssertEqual(game.observations["before"], "\(F.Vector2(14.5, 10))")
        // Spacing 3 adds 1.5 to the one inter-glyph gap.
        XCTAssertEqual(game.observations["wider"], "\(F.Vector2(16, 10))")
        // LineSpacing is the starting height, and no glyph is taller than 20.
        XCTAssertEqual(game.observations["taller"], "\(F.Vector2(16, 20))")
    }

    /// `set_DefaultCharacter` refuses a character the font does not have, with
    /// an `ArgumentException` carrying **no `ParamName`**.
    func testDefaultCharacterRefusesACharacterNotInTheFont() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            do {
                try font.SetDefaultCharacter(UInt16(UInt8(ascii: "Z")))
                game.observations["refused"] = "accepted"
            } catch let error as CNAArgumentException {
                game.observations["refused"] =
                    "\(error.ParamName ?? "nil")|\(error.Message)"
            }
            // The value is unchanged, because the test precedes the write.
            game.observations["unchanged"] = "\(font.DefaultCharacter ?? 0)"
            // Clearing is always accepted: the test is guarded by HasValue.
            try font.SetDefaultCharacter(nil)
            game.observations["cleared"] = "\(font.DefaultCharacter == nil)"
            // And setting one the font does have.
            try font.SetDefaultCharacter(UInt16(UInt8(ascii: "C")))
            game.observations["set"] = "\(font.DefaultCharacter ?? 0)"
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertTrue(
            game.observations["refused"]?.hasPrefix(
                "nil|The character 'Z' (0x005a) is not available in this SpriteFont.")
                == true,
            "\(game.observations["refused"] ?? "-")")
        XCTAssertEqual(game.observations["unchanged"], "\(UInt16(UInt8(ascii: "B")))")
        XCTAssertEqual(game.observations["cleared"], "true")
        XCTAssertEqual(game.observations["set"], "\(UInt16(UInt8(ascii: "C")))")
    }

    // ------------------------------------------------------------------
    // The measurement, computed by hand from InternalMeasure's IL.

    /// Every branch of `InternalMeasure`, each with its arithmetic written out.
    func testMeasureStringReproducesInternalMeasure() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            for text in ["", "A", "AB", "ABC", "A\nB", "A\r\nB", "\n", "\r", "Z"] {
                game.observations[text] = "\(try font.MeasureString(text))"
            }
            font.box.release()
            try atlas.Dispose()
        }
        // Empty: the very first branch, before anything is initialised.
        XCTAssertEqual(game.observations[""], "\(F.Vector2.Zero)")
        // "A": first of line clamps the -2 bearing to 0, so 0 + 5, and the
        // tail adds max(1, 0). Height is max(lineSpacing 10, crop 6).
        XCTAssertEqual(game.observations["A"], "\(F.Vector2(6, 10))")
        // "AB": 5, then +spacing(1.5) +rightBearing(1) +left(1) +width(4),
        // then tail +2.
        XCTAssertEqual(game.observations["AB"], "\(F.Vector2(14.5, 10))")
        // "ABC": ... + 1.5 + 2 + 0.5 + 3, tail + max(0, 0). Height picks up
        // C's cropping height of 9, which is still under lineSpacing.
        XCTAssertEqual(game.observations["ABC"], "\(F.Vector2(19.5, 10))")
        // "A\nB": the first line ends at 5 + max(1,0) = 6 and is remembered;
        // the second line is 1 + 4 = 5 plus the tail's 2 = 7. The width is
        // max(7, 6) -- the LAST line's, not the widest seen first.
        XCTAssertEqual(game.observations["A\nB"], "\(F.Vector2(7, 20))")
        // A carriage return is skipped before any other test, so it cannot
        // change anything.
        XCTAssertEqual(game.observations["A\r\nB"], game.observations["A\nB"])
        // A lone newline: one line break, no glyph. Height is two lines.
        XCTAssertEqual(game.observations["\n"], "\(F.Vector2(0, 20))")
        // A lone carriage return measures like a one-line empty string --
        // NOT like the empty string, because the length test passed.
        XCTAssertEqual(game.observations["\r"], "\(F.Vector2(0, 10))")
        // 'Z' is not in the font and falls back to 'B'.
        XCTAssertEqual(game.observations["Z"], "\(F.Vector2(7, 10))")
    }

    /// **The left-bearing clamp applies to the first glyph of a line and to no
    /// other**, which needs a negative bearing on a glyph that is not first.
    ///
    /// The default fixture cannot show this: only `A` has a negative bearing
    /// and `A` is first in every string it appears in. Two mutations live
    /// here — clamping nothing and clamping everything — and the first
    /// fixture caught only one of them.
    func testOnlyTheFirstGlyphOfALineHasItsBearingClamped() throws {
        try requireNative()
        let game = try run { game, device in
            // `B` first, then `A` with its -2 bearing in second place.
            let (font, atlas) = try self.makeFont(device, spacing: 0)
            game.observations["BA"] = "\(try font.MeasureString("BA"))"
            game.observations["AB"] = "\(try font.MeasureString("AB"))"
            game.observations["A"] = "\(try font.MeasureString("A"))"
            game.observations["B\nA"] = "\(try font.MeasureString("B\nA"))"
            font.box.release()
            try atlas.Dispose()
        }
        // "BA", spacing 0: B first, clamp irrelevant -> 1 + 4 = 5;
        // A second, bearing NOT clamped -> +rightBearing(2) -2 + 5 = 10;
        // tail + max(1, 0) = 11.
        XCTAssertEqual(game.observations["BA"], "\(F.Vector2(11, 10))")
        // "AB", spacing 0: A first, -2 clamped to 0 -> 5; B second ->
        // +1 +1 +4 = 11; tail +2 = 13.
        XCTAssertEqual(game.observations["AB"], "\(F.Vector2(13, 10))")
        // "A" alone is the clamp on its own: 0 + 5, tail + 1.
        XCTAssertEqual(game.observations["A"], "\(F.Vector2(6, 10))")
        // After a newline `A` is first again, so its bearing IS clamped:
        // second line = 0 + 5 + tail 1 = 6, first line was 5 + 2 = 7.
        XCTAssertEqual(game.observations["B\nA"], "\(F.Vector2(7, 20))")
    }

    /// **The trailing right bearing is clamped too**, which needs a glyph
    /// whose right bearing is negative — the default fixture has none.
    func testATrailingNegativeBearingIsClampedAway() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(
                device,
                glyphs: [("D", 0, 6, 0, 2, -1), ("E", 8, 6, 0, 2, 3)],
                spacing: 0, defaultCharacter: nil)
            game.observations["D"] = "\(try font.MeasureString("D"))"
            game.observations["E"] = "\(try font.MeasureString("E"))"
            // Mid-string the negative bearing is NOT clamped: it is added.
            game.observations["DE"] = "\(try font.MeasureString("DE"))"
            font.box.release()
            try atlas.Dispose()
        }
        // 0 + 2, then the tail's max(-1, 0) contributes nothing.
        XCTAssertEqual(game.observations["D"], "\(F.Vector2(2, 10))")
        // The same glyph shape with a positive bearing does contribute.
        XCTAssertEqual(game.observations["E"], "\(F.Vector2(5, 10))")
        // "DE": 2, then +spacing(0) + rightBearing(-1) + 0 + 2 = 3,
        // then the tail's +3.
        XCTAssertEqual(game.observations["DE"], "\(F.Vector2(6, 10))")
    }

    /// **The height comes from the CROPPING rectangle, not the atlas one** —
    /// which only shows when a cropping height exceeds the line spacing, since
    /// the running maximum starts at `LineSpacing`.
    func testTheHeightComesFromTheCroppingRectangle() throws {
        try requireNative()
        let game = try run { game, device in
            // Cropping heights 6 and 9; every atlas rectangle is 8 tall.
            // LineSpacing 4 is under all of them, so the max is a real one.
            let (font, atlas) = try self.makeFont(
                device, lineSpacing: 4, spacing: 0, defaultCharacter: nil)
            game.observations["A"] = "\(try font.MeasureString("A"))"
            game.observations["C"] = "\(try font.MeasureString("C"))"
            game.observations["AC"] = "\(try font.MeasureString("AC"))"
            font.box.release()
            try atlas.Dispose()
        }
        // A's cropping height is 6, its atlas height 8: the answer is 6.
        XCTAssertEqual(game.observations["A"], "\(F.Vector2(6, 6))")
        // C's cropping height is 9, its atlas height 8: the answer is 9,
        // which the atlas rectangle could not produce.
        XCTAssertEqual(game.observations["C"], "\(F.Vector2(3.5, 9))")
        // And across two glyphs the taller cropping wins.
        XCTAssertEqual(game.observations["AC"], "\(F.Vector2(9.5, 9))")
    }

    /// The fallback is **one** step deep, and without a default the character
    /// is reported — with a `ParamName`, unlike `set_DefaultCharacter`'s.
    func testAnUnknownCharacterWithoutADefaultIsReported() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device, defaultCharacter: nil)
            do {
                _ = try font.MeasureString("Z")
                game.observations["refused"] = "accepted"
            } catch let error as CNAArgumentException {
                game.observations["refused"] =
                    "\(error.ParamName ?? "nil")|\(error.Message)"
            }
            // A character that IS in the font still measures.
            game.observations["known"] = "\(try font.MeasureString("A"))"
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertTrue(
            game.observations["refused"]?.hasPrefix(
                "character|The character 'Z' (0x005a) is not available in this SpriteFont.")
                == true,
            "\(game.observations["refused"] ?? "-")")
        XCTAssertEqual(game.observations["known"], "\(F.Vector2(6, 10))")
    }

    /// The binary search finds every character, not just the middle one — a
    /// search that only ever looked at the midpoint would pass a one-glyph
    /// font and fail here.
    func testEveryCharacterIsFoundByTheBinarySearch() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device, defaultCharacter: nil)
            for text in ["A", "B", "C"] {
                game.observations[text] = "\(try font.MeasureString(text))"
            }
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertEqual(game.observations["A"], "\(F.Vector2(6, 10))")
        XCTAssertEqual(game.observations["B"], "\(F.Vector2(7, 10))")
        XCTAssertEqual(game.observations["C"], "\(F.Vector2(3.5, 10))")
    }

    /// **CNA neither sorts a glyph table nor refuses an unsorted one**, and
    /// XNA's binary search requires sortedness. The invariant is checked at
    /// construction so a character that IS in the font can never be reported
    /// as one that is not.
    ///
    /// Measured, `build-probe/f70_sorted.c`: a font built from `C, A, B` is
    /// accepted and `cna_sprite_font_copy_characters` answers `CAB`. CNA's
    /// own `measure_utf8` handles that order; XNA's algorithm cannot, and
    /// this projection reproduces XNA's.
    func testAnUnsortedGlyphTableIsRefusedAtConstruction() throws {
        try requireNative()
        let game = try run { game, device in
            do {
                let (font, atlas) = try self.makeFont(
                    device,
                    glyphs: [("C", 0, 6, 0, 3, 0),
                             ("A", 8, 6, 0, 5, 0),
                             ("B", 16, 6, 0, 4, 0)],
                    defaultCharacter: nil)
                game.observations["unsorted"] = "accepted"
                font.box.release()
                try atlas.Dispose()
            } catch CNAError.producerInvariant(let detail) {
                game.observations["unsorted"] = detail
            }
            // The same three glyphs in order are accepted, so it is the order
            // that was refused and not the glyphs.
            let (font, atlas) = try self.makeFont(
                device,
                glyphs: [("A", 0, 6, 0, 5, 0),
                         ("B", 8, 6, 0, 4, 0),
                         ("C", 16, 6, 0, 3, 0)],
                spacing: 0, defaultCharacter: nil)
            game.observations["sorted"] = "\(try font.MeasureString("ABC"))"
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertEqual(
            game.observations["unsorted"],
            "SpriteFont's character map is not sorted ascending, which "
            + "GetIndexForCharacter's binary search requires")
        XCTAssertEqual(game.observations["sorted"], "\(F.Vector2(12, 10))")
    }

    // ------------------------------------------------------------------
    // DrawString.

    /// A draw outside a begin/end pair is refused — but only when the text
    /// would produce a glyph, which is where XNA raises it from.
    func testDrawStringOutsideABeginEndPair() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            let batch = try G.SpriteBatch(graphicsDevice: device)
            do {
                try batch.DrawString(
                    font, text: "AB", position: F.Vector2(1, 2), color: .White)
                game.observations["glyphs"] = "accepted"
            } catch let error as CNAInvalidOperationException {
                game.observations["glyphs"] = error.Message
            }
            // An empty string draws nothing and raises nothing. XNA's check
            // lives inside the per-glyph loop, so a string with no glyph never
            // reaches it -- and CNA's route would have refused this.
            try batch.DrawString(
                font, text: "", position: .Zero, color: .White)
            game.observations["empty"] = "accepted"
            // Same for a string of nothing but line breaks.
            try batch.DrawString(
                font, text: "\r\n", position: .Zero, color: .White)
            game.observations["breaks"] = "accepted"
            try batch.Dispose()
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertEqual(
            game.observations["glyphs"],
            "Begin must be called successfully before a Draw can be called.")
        XCTAssertEqual(game.observations["empty"], "accepted")
        XCTAssertEqual(game.observations["breaks"], "accepted")
    }

    /// Inside a pair the draw is accepted, through all three overloads.
    ///
    /// **What a passing draw means here is what it meant in Foundation 68**: a
    /// draw that returns is a draw the device accepted. Nothing asserts a
    /// pixel, because `cna_graphics_device_get_backbuffer_data_window` still
    /// answers `NOT_SUPPORTED`.
    func testTheThreeDrawStringOverloadsAreAccepted() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            let batch = try G.SpriteBatch(graphicsDevice: device)
            try batch.Begin()
            try batch.DrawString(
                font, text: "AB", position: F.Vector2(1, 2), color: .White)
            try batch.DrawString(
                font, text: "AB", position: F.Vector2(1, 2), color: .White,
                rotation: 0.5, origin: F.Vector2(1, 1), scale: 2,
                effects: .FlipHorizontally, layerDepth: 0.25)
            try batch.DrawString(
                font, text: "AB", position: F.Vector2(1, 2), color: .White,
                rotation: 0.5, origin: F.Vector2(1, 1),
                scale: F.Vector2(2, 3),
                effects: .FlipVertically, layerDepth: 0.25)
            try batch.End()
            game.observations["accepted"] = "yes"
            try batch.Dispose()
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertEqual(game.observations["accepted"], "yes")
    }

    /// A character the font does not have is reported by `DrawString` as
    /// `MeasureString` reports it, not forwarded to CNA — the loop XNA raises
    /// from is a drawing loop, and its exception is the managed one.
    func testDrawStringReportsACharacterNotInTheFont() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device, defaultCharacter: nil)
            let batch = try G.SpriteBatch(graphicsDevice: device)
            try batch.Begin()
            do {
                try batch.DrawString(
                    font, text: "AZ", position: .Zero, color: .White)
                game.observations["refused"] = "accepted"
            } catch let error as CNAArgumentException {
                game.observations["refused"] =
                    "\(error.ParamName ?? "nil")|\(error.Message)"
            }
            try batch.End()
            try batch.Dispose()
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertTrue(
            game.observations["refused"]?.hasPrefix("character|The character 'Z'")
                == true,
            "\(game.observations["refused"] ?? "-")")
    }

    /// A layout property assigned through an infallible setter reaches the
    /// device at the next draw, which is the only member that can report a
    /// refusal. The same deferral the stock effects' `OnApply` performs.
    func testALayoutChangeReachesTheDeviceAtTheNextDraw() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            let batch = try G.SpriteBatch(graphicsDevice: device)
            font.LineSpacing = 20
            font.Spacing = 3
            let functions = device.runtimeState.functions
            let handle = try font.box.validated("test")

            var before = CNASwift_SpriteFontInfo()
            before.struct_size = UInt32(MemoryLayout<CNASwift_SpriteFontInfo>.size)
            before.struct_version = 1
            _ = functions.spriteFontGetInfo(handle, &before)
            game.observations["before"] = "\(before.line_spacing) \(before.spacing)"

            try batch.Begin()
            try batch.DrawString(font, text: "A", position: .Zero, color: .White)
            try batch.End()

            var after = CNASwift_SpriteFontInfo()
            after.struct_size = UInt32(MemoryLayout<CNASwift_SpriteFontInfo>.size)
            after.struct_version = 1
            _ = functions.spriteFontGetInfo(handle, &after)
            game.observations["after"] = "\(after.line_spacing) \(after.spacing)"

            try batch.Dispose()
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertEqual(game.observations["before"], "10 1.5")
        XCTAssertEqual(game.observations["after"], "20 3.0")
    }

    /// The two deferred writes are independent: setting one alone must still
    /// reach the device. Flushing both under one flag would pass the test
    /// above and lose everything here.
    func testEachLayoutPropertyFlushesOnItsOwn() throws {
        try requireNative()
        let game = try run { game, device in
            let functions = device.runtimeState.functions
            func flushed(
                _ change: (G.SpriteFont) -> Void
            ) throws -> String {
                let (font, atlas) = try self.makeFont(device)
                let batch = try G.SpriteBatch(graphicsDevice: device)
                change(font)
                try batch.Begin()
                try batch.DrawString(font, text: "A", position: .Zero, color: .White)
                try batch.End()
                var info = CNASwift_SpriteFontInfo()
                info.struct_size = UInt32(MemoryLayout<CNASwift_SpriteFontInfo>.size)
                info.struct_version = 1
                _ = functions.spriteFontGetInfo(
                    try font.box.validated("test"), &info)
                try batch.Dispose()
                font.box.release()
                try atlas.Dispose()
                return "\(info.line_spacing) \(info.spacing)"
            }
            game.observations["spacing only"] = try flushed { $0.Spacing = 4 }
            game.observations["line only"] = try flushed { $0.LineSpacing = 30 }
        }
        XCTAssertEqual(game.observations["spacing only"], "10 4.0")
        XCTAssertEqual(game.observations["line only"], "30 1.5")
    }

    /// CNA retains the atlas: a font's texture cannot be disposed while the
    /// font holds it. XNA has no such rule — this is a native-owned refusal a
    /// consumer meets, recorded rather than worked around.
    func testTheAtlasIsRetainedWhileTheFontLivesOn() throws {
        try requireNative()
        let game = try run { game, device in
            let (font, atlas) = try self.makeFont(device)
            do {
                try atlas.Dispose()
                game.observations["retained"] = "accepted"
            } catch {
                game.observations["retained"] = "refused"
            }
            font.box.release()
            try atlas.Dispose()
            game.observations["released"] = "accepted"
        }
        XCTAssertEqual(game.observations["retained"], "refused")
        XCTAssertEqual(game.observations["released"], "accepted")
    }
}

private extension UInt32 {
    var littleEndianUInt16: UInt16 { UInt16(truncatingIfNeeded: self) }
}
