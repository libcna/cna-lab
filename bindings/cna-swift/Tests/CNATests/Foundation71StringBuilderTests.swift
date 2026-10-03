// SPDX-License-Identifier: MIT

import CNAShim
import Foundation
import XCTest
@testable import CNA

private typealias F = Microsoft.Xna.Framework
private typealias G = Microsoft.Xna.Framework.Graphics

private final class BuilderProbeGame: F.Game {
    var manager: F.GraphicsDeviceManager?
    var failure: Error?
    var observations: [String: String] = [:]
    private var body: ((BuilderProbeGame, G.GraphicsDevice) throws -> Void)?

    init(_ body: @escaping (BuilderProbeGame, G.GraphicsDevice) throws -> Void) throws {
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

/// Foundation 71: `System.Text.StringBuilder`, admitted as a BCL support
/// family, and the four XNA members that were waiting for it.
///
/// Every message here is the one `mscorlib`'s embedded string table holds, and
/// every refusal's order is the order its IL raises them in. None of this
/// needs a device: a `StringBuilder` is pure managed state, which is why the
/// only device-scoped tests are the four XNA members at the end.
final class Foundation71StringBuilderTests: XCTestCase {

    // ------------------------------------------------------------------
    // Construction.

    /// `StringBuilder()` and `StringBuilder(String)` both start at
    /// `DefaultCapacity`, which is `ldc.i4.s 16` in each of them.
    func testTheDefaultCapacityIsSixteen() {
        XCTAssertEqual(CNAStringBuilder().Capacity, 16)
        XCTAssertEqual(CNAStringBuilder().Length, 0)
        XCTAssertEqual(CNAStringBuilder().MaxCapacity, Int32.max)
        // A longer seed raises the capacity to fit it.
        let seeded = CNAStringBuilder("abcdefghijklmnopqrstuvwxyz")
        XCTAssertEqual(seeded.Length, 26)
        XCTAssertEqual(seeded.Capacity, 26)
    }

    /// The two-argument constructor's three refusals, in the order the IL
    /// raises them — and the third's message is the only one that is
    /// *formatted*, with the parameter name as its `{0}`.
    func testTheCapacityConstructorsThreeRefusalsInOrder() throws {
        // capacity > maxCapacity comes FIRST, so a call that is wrong in two
        // ways reports the capacity against the ceiling.
        XCTAssertThrowsError(try CNAStringBuilder(capacity: 10, maxCapacity: 5)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "capacity")
            XCTAssertTrue(
                error?.Message.hasPrefix("Capacity exceeds maximum capacity.") == true,
                "\(error?.Message ?? "-")")
        }
        XCTAssertThrowsError(try CNAStringBuilder(capacity: 0, maxCapacity: 0)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "maxCapacity")
            XCTAssertTrue(
                error?.Message.hasPrefix("MaxCapacity must be one or greater.") == true,
                "\(error?.Message ?? "-")")
        }
        XCTAssertThrowsError(try CNAStringBuilder(capacity: -1, maxCapacity: 10)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "capacity")
            // Formatted: the parameter name appears INSIDE the sentence too.
            XCTAssertTrue(
                error?.Message.hasPrefix("'capacity' must be greater than zero.") == true,
                "\(error?.Message ?? "-")")
        }
    }

    /// `if (capacity == 0) capacity = Math.Min(DefaultCapacity, maxCapacity)`
    /// — the floor is the smaller of sixteen and the ceiling just accepted.
    func testAZeroCapacityTakesTheSmallerOfSixteenAndTheCeiling() throws {
        XCTAssertEqual(try CNAStringBuilder(capacity: 0, maxCapacity: 4).Capacity, 4)
        XCTAssertEqual(try CNAStringBuilder(capacity: 0, maxCapacity: 100).Capacity, 16)
    }

    // ------------------------------------------------------------------
    // The indexer, whose two accessors raise different exceptions.

    /// **`get_Chars` raises a bare `IndexOutOfRangeException`** — no message,
    /// no parameter name — while `set_Chars` raises
    /// `ArgumentOutOfRangeException("index", …)`. One indexer, two exception
    /// types, for the same kind of mistake.
    func testTheIndexersTwoAccessorsRaiseDifferentExceptions() throws {
        let builder = CNAStringBuilder("abc")
        XCTAssertEqual(try builder.Item(0), UInt16(UInt8(ascii: "a")))
        XCTAssertEqual(try builder.Item(2), UInt16(UInt8(ascii: "c")))

        XCTAssertThrowsError(try builder.Item(3)) {
            XCTAssertTrue($0 is CNAIndexOutOfRangeException, "\(type(of: $0))")
        }
        XCTAssertThrowsError(try builder.Item(-1)) {
            XCTAssertTrue($0 is CNAIndexOutOfRangeException, "\(type(of: $0))")
        }
        XCTAssertThrowsError(try builder.SetItem(3, 0)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "index")
            XCTAssertTrue(
                error?.Message.hasPrefix("Index was out of range.") == true,
                "\(error?.Message ?? "-")")
        }
        try builder.SetItem(1, UInt16(UInt8(ascii: "X")))
        XCTAssertEqual(builder.ToString(), "aXc")
    }

    // ------------------------------------------------------------------
    // Length, and what growing it writes.

    func testSetLengthTruncatesAndPadsWithNul() throws {
        let builder = CNAStringBuilder("abcdef")
        try builder.SetLength(3)
        XCTAssertEqual(builder.ToString(), "abc")
        // Growing pads with U+0000, which is a code unit like any other.
        try builder.SetLength(5)
        XCTAssertEqual(builder.Length, 5)
        XCTAssertEqual(try builder.Item(3), 0)
        XCTAssertEqual(try builder.Item(4), 0)
    }

    /// Both refusals name `"value"`, and they carry two different messages.
    func testSetLengthsTwoRefusalsBothNameValue() throws {
        let builder = try CNAStringBuilder(capacity: 4, maxCapacity: 8)
        XCTAssertThrowsError(try builder.SetLength(-1)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "value")
            XCTAssertTrue(
                error?.Message.hasPrefix("Length cannot be less than zero.") == true,
                "\(error?.Message ?? "-")")
        }
        XCTAssertThrowsError(try builder.SetLength(9)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "value")
            XCTAssertTrue(
                error?.Message.hasPrefix("capacity was less than the current size.")
                    == true,
                "\(error?.Message ?? "-")")
        }
    }

    // ------------------------------------------------------------------
    // Growing, and the refusal every growing member shares.

    /// **Every growing member is fallible, and not for a reason its own body
    /// shows.** `ExpandByABlock` raises
    /// `ArgumentOutOfRangeException("requiredLength", …)` when the result
    /// would pass `MaxCapacity`, so a plain `Append` reaches it.
    func testAppendingPastMaxCapacityIsRefusedByTheSharedGuard() throws {
        let builder = try CNAStringBuilder(capacity: 4, maxCapacity: 4)
        try builder.Append("abcd")
        XCTAssertEqual(builder.Length, 4)
        XCTAssertThrowsError(try builder.Append("e")) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "requiredLength")
            XCTAssertTrue(
                error?.Message.hasPrefix("capacity was less than the current size.")
                    == true,
                "\(error?.Message ?? "-")")
        }
        // The refused append wrote nothing.
        XCTAssertEqual(builder.ToString(), "abcd")
    }

    /// `Append(Char, Int32)`'s own refusal comes **before** anything is
    /// written, and names `repeatCount`.
    func testAppendRepeatCountRefusesANegativeCountBeforeWriting() throws {
        let builder = CNAStringBuilder("ab")
        XCTAssertThrowsError(try builder.Append(UInt16(UInt8(ascii: "z")), repeatCount: -1)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "repeatCount")
            XCTAssertTrue(
                error?.Message.hasPrefix("Count cannot be less than zero.") == true,
                "\(error?.Message ?? "-")")
        }
        XCTAssertEqual(builder.ToString(), "ab")
        // Zero is accepted and writes nothing.
        try builder.Append(UInt16(UInt8(ascii: "z")), repeatCount: 0)
        XCTAssertEqual(builder.ToString(), "ab")
        try builder.Append(UInt16(UInt8(ascii: "z")), repeatCount: 3)
        XCTAssertEqual(builder.ToString(), "abzzz")
    }

    /// `AppendLine` appends `Environment.NewLine`, which on the Windows CLR
    /// this binding targets is `"\r\n"` — not the host's separator.
    func testAppendLineUsesTheWindowsSeparator() throws {
        let builder = CNAStringBuilder()
        try builder.AppendLine("a")
        try builder.AppendLine()
        XCTAssertEqual(builder.ToString(), "a\r\n\r\n")
    }

    /// Every mutator returns the builder, so calls chain — which is the whole
    /// reason `StringBuilder` exists rather than a `String` loop.
    func testTheMutatorsChain() throws {
        let builder = CNAStringBuilder()
        let same = try builder.Append("a").Append("b")
        XCTAssertTrue(same === builder)
        try builder.Insert(0, "X")
        XCTAssertEqual(builder.ToString(), "Xab")
    }

    // ------------------------------------------------------------------
    // Insert, Remove, ToString.

    func testInsertAcceptsTheEndAndRefusesPastIt() throws {
        let builder = CNAStringBuilder("ac")
        try builder.Insert(1, "b")
        XCTAssertEqual(builder.ToString(), "abc")
        // Inserting AT Length is legal -- the bound is `<=`, not `<`.
        try builder.Insert(builder.Length, "d")
        XCTAssertEqual(builder.ToString(), "abcd")
        XCTAssertThrowsError(try builder.Insert(builder.Length + 1, "e")) {
            XCTAssertEqual(($0 as? CNAArgumentOutOfRangeException)?.ParamName, "index")
        }
        XCTAssertThrowsError(try builder.Insert(-1, "e")) {
            XCTAssertEqual(($0 as? CNAArgumentOutOfRangeException)?.ParamName, "index")
        }
    }

    /// `Remove`'s three refusals, in order — and the third names **`"index"`**,
    /// not `"startIndex"`, which is the one a reimplementation gets wrong.
    func testRemovesThreeRefusalsAndItsThirdParameterName() throws {
        let builder = CNAStringBuilder("abcdef")
        XCTAssertThrowsError(try builder.Remove(0, -1)) {
            XCTAssertEqual(($0 as? CNAArgumentOutOfRangeException)?.ParamName, "length")
        }
        XCTAssertThrowsError(try builder.Remove(-1, 1)) {
            XCTAssertEqual(($0 as? CNAArgumentOutOfRangeException)?.ParamName, "startIndex")
        }
        XCTAssertThrowsError(try builder.Remove(4, 3)) {
            XCTAssertEqual(($0 as? CNAArgumentOutOfRangeException)?.ParamName, "index")
        }
        try builder.Remove(1, 2)
        XCTAssertEqual(builder.ToString(), "adef")
    }

    /// `ToString(startIndex:length:)`'s four refusals, in order.
    func testToStringsFourRefusalsInOrder() throws {
        let builder = CNAStringBuilder("abcdef")
        XCTAssertEqual(try builder.ToString(1, 3), "bcd")
        XCTAssertThrowsError(try builder.ToString(-1, 1)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "startIndex")
            XCTAssertTrue(
                error?.Message.hasPrefix("StartIndex cannot be less than zero.") == true,
                "\(error?.Message ?? "-")")
        }
        XCTAssertThrowsError(try builder.ToString(7, 0)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "startIndex")
            XCTAssertTrue(
                error?.Message.hasPrefix(
                    "startIndex cannot be larger than length of string.") == true,
                "\(error?.Message ?? "-")")
        }
        XCTAssertThrowsError(try builder.ToString(0, -1)) {
            XCTAssertEqual(($0 as? CNAArgumentOutOfRangeException)?.ParamName, "length")
        }
        XCTAssertThrowsError(try builder.ToString(4, 3)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "length")
            XCTAssertTrue(
                error?.Message.hasPrefix(
                    "Index and length must refer to a location within the string.")
                    == true,
                "\(error?.Message ?? "-")")
        }
        // A call wrong in TWO ways is what pins the ORDER: each test above is
        // wrong in one, and any permutation of the four guards passes them
        // all. XNA tests startIndex first, so this reports `startIndex`.
        XCTAssertThrowsError(try builder.ToString(-1, -1)) {
            let error = $0 as? CNAArgumentOutOfRangeException
            XCTAssertEqual(error?.ParamName, "startIndex")
            XCTAssertTrue(
                error?.Message.hasPrefix("StartIndex cannot be less than zero.") == true,
                "\(error?.Message ?? "-")")
        }
    }

    /// `Capacity` is a **lower bound**, not .NET's chunk arithmetic — the
    /// difference is recorded on the property, and this pins what is actually
    /// promised so the promise cannot drift silently.
    func testCapacityIsAtLeastTheLengthAndSurvivesClear() throws {
        let builder = CNAStringBuilder()
        XCTAssertEqual(builder.Capacity, 16)
        // Growth past the initial capacity keeps the invariant, whatever
        // number the strategy picks.
        try builder.Append(String(repeating: "x", count: 20))
        XCTAssertEqual(builder.Length, 20)
        XCTAssertGreaterThanOrEqual(builder.Capacity, builder.Length)
        let grown = builder.Capacity
        // Clearing empties the text and keeps the room.
        builder.Clear()
        XCTAssertEqual(builder.Length, 0)
        XCTAssertEqual(builder.Capacity, grown)
        // And an explicit capacity is exact, because that path is XNA's.
        try builder.SetCapacity(64)
        XCTAssertEqual(builder.Capacity, 64)
    }

    /// `Clear()` is `this.Length = 0` and nothing else — **the capacity
    /// survives**, which is the whole reason to clear rather than rebuild.
    func testClearKeepsTheCapacity() throws {
        let builder = try CNAStringBuilder(capacity: 64, maxCapacity: 128)
        try builder.Append("abcdef")
        XCTAssertEqual(builder.Capacity, 64)
        builder.Clear()
        XCTAssertEqual(builder.Length, 0)
        XCTAssertEqual(builder.Capacity, 64)
        XCTAssertEqual(builder.ToString(), "")
    }

    // ------------------------------------------------------------------
    // The store is code units, not characters.

    // ------------------------------------------------------------------
    // The four XNA members this family was admitted for.

    private func run(
        _ body: @escaping (BuilderProbeGame, G.GraphicsDevice) throws -> Void
    ) throws -> BuilderProbeGame {
        if ProcessInfo.processInfo.environment["CNA_NATIVE_LIBRARY"] == nil {
            throw XCTSkip("set CNA_NATIVE_LIBRARY to a CNA C ABI 0.35 or later library")
        }
        let game = try BuilderProbeGame(body)
        // Dispose unconditionally. A throwing `Run()` skipped it, and the
        // native game it leaked is the process's ONE active CNA game -- a
        // later `Game.Run()` then blocks forever, which is how a caught
        // mutation came back HUNG at test 346 of 809.
        defer { try? game.Dispose() }
        try game.Run()
        if let failure = game.failure { throw failure }
        return game
    }

    /// A font over four glyphs: `A`, `B`, and the two halves of `U+1F600`.
    ///
    /// A `SpriteFont`'s character map is UTF-16 **code units**, so a glyph at
    /// `0xD83D` is as legitimate as one at `0x0041` — which is what makes the
    /// next test able to tell a code-unit measurement from a rendered one.
    private func makeSurrogateFont(
        _ device: G.GraphicsDevice
    ) throws -> (G.SpriteFont, G.Texture2D) {
        let atlas = try G.Texture2D(graphicsDevice: device, width: 64, height: 8)
        func glyph(_ unit: UInt16, _ x: Int32, _ width: Float) -> CNASwift_SpriteFontGlyph {
            var g = CNASwift_SpriteFontGlyph()
            g.struct_size = UInt32(MemoryLayout<CNASwift_SpriteFontGlyph>.size)
            g.struct_version = 1
            g.glyph_bounds = CNASwift_Rectangle(x: x, y: 0, width: 8, height: 8)
            g.cropping = CNASwift_Rectangle(x: 0, y: 0, width: 8, height: 6)
            g.character = unit
            g.kerning = CNASwift_Vector3(x: 0, y: width, z: 0)
            return g
        }
        // Sorted ascending, which SpriteFont's binary search requires.
        let glyphs = [
            glyph(UInt16(UInt8(ascii: "A")), 0, 5),
            glyph(UInt16(UInt8(ascii: "B")), 8, 4),
            glyph(0xD83D, 16, 3),
            glyph(0xDE00, 24, 2),
        ]
        let runtime = device.runtimeState
        var handle: UInt64 = 0
        try glyphs.withUnsafeBufferPointer { buffer in
            var info = CNASwift_SpriteFontCreateInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_SpriteFontCreateInfo>.size)
            info.struct_version = 1
            info.texture = try atlas.validatedHandle("atlas")
            info.glyphs = buffer.baseAddress
            info.glyph_count = UInt64(buffer.count)
            info.line_spacing = 10
            info.spacing = 0
            info.has_default_character = 0
            try runtime.functions.check(
                runtime.functions.spriteFontCreate(&info, &handle),
                operation: "cna_sprite_font_create")
        }
        let font = try G.SpriteFont(
            box: SpriteFontBox(handle: handle, runtime: runtime), texture: atlas)
        return (font, atlas)
    }

    /// `MeasureString(StringBuilder)` answers what `MeasureString(String)`
    /// answers for the same contents — one measurement body, two overloads,
    /// exactly as XNA's `StringProxy` arranges.
    func testMeasureStringAgreesAcrossTheTwoOverloads() throws {
        let game = try run { game, device in
            let (font, atlas) = try self.makeSurrogateFont(device)
            let builder = CNAStringBuilder("AB")
            game.observations["builder"] = "\(try font.MeasureString(builder))"
            game.observations["string"] = "\(try font.MeasureString("AB"))"
            font.box.release()
            try atlas.Dispose()
        }
        // 0 + 5, then 0 + 4 with no spacing and no bearings -> 9 wide.
        XCTAssertEqual(game.observations["builder"], "\(F.Vector2(9, 10))")
        XCTAssertEqual(game.observations["string"], game.observations["builder"])
    }

    /// **The overload measures the buffer's CODE UNITS**, which is the whole
    /// reason `CNAStringBuilder` stores `[UInt16]`.
    ///
    /// The font has a glyph for each half of `U+1F600`, so a measurement that
    /// went through a rendered `String` and lost the surrogates would answer a
    /// different width — and would be indistinguishable from correct on any
    /// text that happens to be all BMP.
    func testMeasureStringReadsTheBuffersCodeUnits() throws {
        let game = try run { game, device in
            let (font, atlas) = try self.makeSurrogateFont(device)
            let builder = CNAStringBuilder("A\u{1F600}")
            game.observations["length"] = "\(builder.Length)"
            game.observations["measured"] = "\(try font.MeasureString(builder))"
            font.box.release()
            try atlas.Dispose()
        }
        // Three code units: 'A' and the surrogate pair.
        XCTAssertEqual(game.observations["length"], "3")
        // 5 + 3 + 2 = 10. Dropping the pair would answer 5.
        XCTAssertEqual(game.observations["measured"], "\(F.Vector2(10, 10))")
    }

    /// The three `DrawString(StringBuilder)` overloads reach the device, and
    /// carry XNA's begin/end rule with them.
    ///
    /// As in Foundation 68 and 70: a draw that returns is a draw the device
    /// accepted, and nothing here claims a glyph arrived anywhere.
    func testTheThreeStringBuilderDrawStringOverloads() throws {
        let game = try run { game, device in
            let (font, atlas) = try self.makeSurrogateFont(device)
            let batch = try G.SpriteBatch(graphicsDevice: device)
            let builder = CNAStringBuilder("AB")

            // Outside a pair, a builder that would draw a glyph is refused
            // exactly as a String is.
            do {
                try batch.DrawString(
                    font, text: builder, position: .Zero, color: .White)
                game.observations["outside"] = "accepted"
            } catch let error as CNAInvalidOperationException {
                game.observations["outside"] = error.Message
            }
            // An empty builder draws nothing and raises nothing.
            try batch.DrawString(
                font, text: CNAStringBuilder(), position: .Zero, color: .White)
            game.observations["empty"] = "accepted"

            try batch.Begin()
            try batch.DrawString(
                font, text: builder, position: F.Vector2(1, 2), color: .White)
            try batch.DrawString(
                font, text: builder, position: F.Vector2(1, 2), color: .White,
                rotation: 0.5, origin: F.Vector2(1, 1), scale: 2,
                effects: .FlipHorizontally, layerDepth: 0.25)
            try batch.DrawString(
                font, text: builder, position: F.Vector2(1, 2), color: .White,
                rotation: 0.5, origin: F.Vector2(1, 1),
                scale: F.Vector2(2, 3), effects: .FlipVertically, layerDepth: 0.25)
            try batch.End()
            game.observations["inside"] = "accepted"

            try batch.Dispose()
            font.box.release()
            try atlas.Dispose()
        }
        XCTAssertEqual(
            game.observations["outside"],
            "Begin must be called successfully before a Draw can be called.")
        XCTAssertEqual(game.observations["empty"], "accepted")
        XCTAssertEqual(game.observations["inside"], "accepted")
    }

    // ------------------------------------------------------------------

    /// **`Length` counts UTF-16 code units, not `Character`s.** An astral
    /// scalar is one Swift `Character` and two code units, and every index in
    /// this API is a code-unit index — which is why the store is `[UInt16]`
    /// and not a `String`.
    func testLengthCountsCodeUnitsNotCharacters() throws {
        let builder = CNAStringBuilder("a\u{1F600}b")
        XCTAssertEqual("a\u{1F600}b".count, 3)          // three Characters
        XCTAssertEqual(builder.Length, 4)               // four code units
        // The surrogate pair is addressable a half at a time, exactly as a
        // CLR `char` indexer addresses it.
        XCTAssertEqual(try builder.Item(1), 0xD83D)
        XCTAssertEqual(try builder.Item(2), 0xDE00)
        XCTAssertEqual(builder.ToString(), "a\u{1F600}b")
    }
}
