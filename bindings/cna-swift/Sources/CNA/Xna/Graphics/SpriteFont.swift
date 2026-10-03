// SPDX-License-Identifier: MIT

import CNAShim

/// One owned native `SpriteFont` handle, released exactly once.
internal final class SpriteFontBox {
    internal private(set) var handle: UInt64
    internal let runtime: RuntimeState

    init(handle: UInt64, runtime: RuntimeState) {
        self.handle = handle
        self.runtime = runtime
    }

    func release() {
        guard handle != 0 else { return }
        _ = runtime.functions.spriteFontDestroy(handle)
        handle = 0
    }

    func validated(_ operation: String) throws -> UInt64 {
        guard handle != 0 else {
            throw CNAError.producerInvariant("\(operation) on a released SpriteFont")
        }
        return handle
    }

    deinit { release() }
}

extension Microsoft.Xna.Framework.Graphics {
    /// The `Microsoft.Xna.Framework.Graphics.SpriteFont` projection.
    ///
    /// `sealed` in the IL, so `final`, and its only constructor is `assembly`
    /// — a font is built by the content pipeline, never by a caller. The Swift
    /// initializers are `internal` for that reason and no `init` appears in
    /// the public surface.
    ///
    /// **The measurement is reproduced, not forwarded.** CNA publishes
    /// `cna_sprite_font_measure_utf8`, and `MeasureString`'s answer is XNA
    /// behaviour: it comes from `SpriteFont.InternalMeasure`'s own arithmetic
    /// over the glyph table, exactly as `EnableDefaultLighting` comes from
    /// `EffectHelpers`' table rather than from the native route that agrees
    /// with it. The glyph table is CNA's — `cna_sprite_font_copy_glyphs` is
    /// documented as "the inverse of `cna_sprite_font_create`" — and the
    /// arithmetic over it is XNA's.
    public final class SpriteFont {
        internal let box: SpriteFontBox
        /// The atlas the font draws from, kept alive because CNA says so:
        /// "the source texture remains owned by the caller and cannot be
        /// destroyed until this SpriteFont is destroyed".
        private let texture: Texture2D?

        /// The glyph table, read once. XNA holds three parallel `List<T>`s
        /// filled by its constructor and never refilled; CNA hands back the
        /// same three things in one array, in the same order as `Characters`.
        private let glyphs: [CNASwift_SpriteFontGlyph]
        /// `characterMap`, which XNA requires to be **sorted** because
        /// `GetIndexForCharacter` binary-searches it.
        private let characterMap: [UInt16]

        private var lineSpacing: Int32
        private var spacing: Float
        private var defaultCharacter: UInt16?
        private var charactersView: CNAReadOnlyCollection<UInt16>?
        /// What a setter changed and the device has not been told about yet.
        /// XNA's setters are `stfld` and cannot fail; CNA's routes can, and a
        /// Swift setter cannot report it — so the write is deferred to
        /// `DrawString`, which is where the native font is actually used and
        /// which is allowed to throw. The same shape as the stock effects'
        /// `OnApply`.
        private var pendingLineSpacing = false
        private var pendingSpacing = false

        internal init(box: SpriteFontBox, texture: Texture2D?) throws {
            self.box = box
            self.texture = texture
            let handle = try box.validated("SpriteFont.init")
            let functions = box.runtime.functions

            var info = CNASwift_SpriteFontInfo()
            info.struct_size = UInt32(MemoryLayout<CNASwift_SpriteFontInfo>.size)
            info.struct_version = 1
            try functions.check(
                functions.spriteFontGetInfo(handle, &info),
                operation: "cna_sprite_font_get_info")
            lineSpacing = info.line_spacing
            spacing = info.spacing
            defaultCharacter =
                info.has_default_character != 0 ? info.default_character : nil

            var count = UInt64(info.character_count)
            var table = [CNASwift_SpriteFontGlyph](
                repeating: CNASwift_SpriteFontGlyph(), count: Int(count))
            if count > 0 {
                try table.withUnsafeMutableBufferPointer { destination in
                    try functions.check(
                        functions.spriteFontCopyGlyphs(
                            handle, destination.baseAddress,
                            UInt64(destination.count), &count),
                        operation: "cna_sprite_font_copy_glyphs")
                }
            }
            glyphs = table
            characterMap = table.map { $0.character }

            // XNA's `GetIndexForCharacter` BINARY-SEARCHES `characterMap`,
            // which is safe there because the content pipeline writes it
            // sorted and nothing else builds one. CNA takes a caller-supplied
            // glyph array and **neither sorts it nor refuses an unsorted
            // one** — `build-probe/f70_sorted.c` builds a font from `C, A, B`
            // and `cna_sprite_font_copy_characters` answers `CAB`. It accepts
            // a duplicated character too, and its own `measure_utf8` copes
            // with both.
            //
            // Reproducing XNA means reproducing the binary search, and a
            // binary search over an unsorted map answers "not in this font"
            // for a character that is. No public route can reach that state —
            // this type has no public constructor, so every font a consumer
            // holds came from content — but a wrong answer wearing the shape
            // of a legitimate refusal is the worst form a defect takes, so the
            // invariant the search depends on is checked once, here, where it
            // can still be reported as what it is.
            guard zip(characterMap, characterMap.dropFirst()).allSatisfy(<) else {
                throw CNAError.producerInvariant(
                    "SpriteFont's character map is not sorted ascending, which "
                    + "GetIndexForCharacter's binary search requires")
            }
        }

        /// `SpriteFont.LineSpacing` — `ldfld`/`stfld`, infallible both ways.
        public var LineSpacing: Int32 {
            get { lineSpacing }
            set { lineSpacing = newValue; pendingLineSpacing = true }
        }

        /// `SpriteFont.Spacing` — the same shape.
        public var Spacing: Float {
            get { spacing }
            set { spacing = newValue; pendingSpacing = true }
        }

        /// `SpriteFont.DefaultCharacter`.
        ///
        /// The getter is `ldfld`. The setter is not:
        ///
        /// ```text
        /// if (value.HasValue && !characterMap.Contains(value.Value))
        ///     throw new ArgumentException(
        ///         Format(CharacterNotInFont, value.Value, (int)value.Value));
        /// ```
        ///
        /// — an `ArgumentException` with **no `ParamName`**, unlike the one
        /// `GetIndexForCharacter` raises with the same message. Clearing it to
        /// nil is always accepted, because the test is guarded by `HasValue`.
        public var DefaultCharacter: UInt16? { defaultCharacter }

        /// `set_DefaultCharacter`.
        public func SetDefaultCharacter(_ value: UInt16?) throws {
            if let value {
                guard characterMap.contains(value) else {
                    throw CNAArgumentException(
                        message: SpriteFont.characterNotInFontMessage(value))
                }
            }
            let handle = try box.validated("SpriteFont.DefaultCharacter")
            try box.runtime.functions.check(
                box.runtime.functions.spriteFontSetDefaultCharacter(
                    handle, value == nil ? 0 : 1, value ?? 0),
                operation: "cna_sprite_font_set_default_character")
            defaultCharacter = value
        }

        /// `SpriteFont.Characters`.
        ///
        /// Built once on first read and cached in a field, so
        /// `font.Characters === font.Characters` — XNA's getter is a
        /// null-check and a lazy `newobj`, and the identity is observable.
        ///
        /// Optional because the pinned return-nullability reference records it
        /// so: the getter's last instruction is `ldfld characters; ret`, and
        /// what the analysis can prove about a field load is what the
        /// projection follows. That this projection always fills it before
        /// returning does not make XNA's declaration non-nullable.
        public var Characters: CNAReadOnlyCollection<UInt16>? {
            if let charactersView { return charactersView }
            let list = CNAList<UInt16>()
            for unit in characterMap { list.Add(unit) }
            let view = CNAReadOnlyCollection<UInt16>(list: list)
            charactersView = view
            return view
        }

        /// `SpriteFont.MeasureString(String)`.
        ///
        /// `InternalMeasure`, instruction for instruction. Both public
        /// overloads are one line each — a null test and a `StringProxy` —
        /// and `StringProxy` exists only so one measurement body can serve a
        /// `String` and a `StringBuilder`. Swift has one string type, so the
        /// proxy has nothing to abstract over and the two overloads differ
        /// only in the type they accept.
        ///
        /// ```text
        /// if (text.Length == 0) return Vector2.Zero;
        /// size = (0, lineSpacing);  width = 0;  lines = 0
        /// rightBearing = 0;  firstOfLine = true
        /// for each char c:
        ///     '\r' -> skip entirely, not even counted
        ///     '\n' -> size.X += max(rightBearing, 0)
        ///             width = max(size.X, width)
        ///             size = (0, lineSpacing); firstOfLine = true; lines++
        ///     else -> k = kerning[index(c)]
        ///             firstOfLine ? k.X = max(k.X, 0)
        ///                         : size.X += spacing + rightBearing
        ///             size.X += k.X + k.Y
        ///             rightBearing = k.Z
        ///             size.Y = max(size.Y, cropping[index(c)].Height)
        ///             firstOfLine = false
        /// size.X += max(rightBearing, 0)
        /// size.Y += lines * lineSpacing
        /// size.X = max(size.X, width)
        /// ```
        ///
        /// Three details a plausible reimplementation gets wrong. A carriage
        /// return is skipped **before** anything else, so `"a\r\nb"` and
        /// `"a\nb"` measure identically. The first glyph of a line has its
        /// left bearing **clamped at zero** rather than added, which is what
        /// stops a negative bearing pulling the first glyph off the left edge.
        /// And the height is the running `max` of the *cropping* heights, not
        /// of the glyph bounds — plus one `lineSpacing` per newline seen.
        public func MeasureString(_ text: String) throws
            -> Microsoft.Xna.Framework.Vector2 {
            try measure(Array(text.utf16))
        }

        /// `SpriteFont.MeasureString(StringBuilder)`.
        ///
        /// One line, exactly as the `String` overload is one line. XNA needs a
        /// `StringProxy` to let a single measurement body serve both; the
        /// proxy has nothing to abstract over here, because both overloads
        /// hand `measure` the same UTF-16 code units.
        ///
        /// This reads the buffer's contents rather than copying it into a
        /// `String` first — which is the whole reason the overload exists.
        public func MeasureString(_ text: CNAStringBuilder) throws
            -> Microsoft.Xna.Framework.Vector2 {
            try measure(text.codeUnits)
        }

        internal func measure(
            _ units: [UInt16]
        ) throws -> Microsoft.Xna.Framework.Vector2 {
            guard !units.isEmpty else { return .Zero }
            var size = Microsoft.Xna.Framework.Vector2(0, Float(lineSpacing))
            var widest: Float = 0
            var lines: Int32 = 0
            var rightBearing: Float = 0
            var firstOfLine = true

            for unit in units {
                if unit == 13 { continue }          // '\r'
                if unit == 10 {                     // '\n'
                    size.X += max(rightBearing, 0)
                    rightBearing = 0
                    widest = max(size.X, widest)
                    size = Microsoft.Xna.Framework.Vector2(0, Float(lineSpacing))
                    firstOfLine = true
                    lines += 1
                    continue
                }
                let index = try indexForCharacter(unit)
                let glyph = glyphs[index]
                var left = glyph.kerning.x
                if firstOfLine {
                    left = max(left, 0)
                } else {
                    size.X += spacing + rightBearing
                }
                size.X += left + glyph.kerning.y
                rightBearing = glyph.kerning.z
                size.Y = max(size.Y, Float(glyph.cropping.height))
                firstOfLine = false
            }

            size.X += max(rightBearing, 0)
            size.Y += Float(lines * lineSpacing)
            size.X = max(size.X, widest)
            return size
        }

        /// `SpriteFont.GetIndexForCharacter`.
        ///
        /// A binary search over the character map — whose sortedness the
        /// initializer checks, because CNA does not guarantee it — then
        /// **one** fallback
        /// to `DefaultCharacter` — the recursion cannot go deeper, because the
        /// fallback is not attempted when the character already *is* the
        /// default. Without that guard a font whose default is itself missing
        /// would recurse for ever instead of reporting the character.
        ///
        /// The refusal is `ArgumentException(message, "character")`, **with**
        /// a parameter name, where `set_DefaultCharacter`'s carries none.
        internal func indexForCharacter(_ unit: UInt16) throws -> Int {
            var low = 0
            var high = characterMap.count - 1
            while low <= high {
                let middle = low + ((high - low) >> 1)
                let candidate = characterMap[middle]
                if candidate == unit { return middle }
                if candidate < unit { low = middle + 1 } else { high = middle - 1 }
            }
            if let defaultCharacter, defaultCharacter != unit {
                return try indexForCharacter(defaultCharacter)
            }
            throw CNAArgumentException(
                message: SpriteFont.characterNotInFontMessage(unit),
                paramName: "character")
        }

        /// `FrameworkResources.CharacterNotInFont`, whose second placeholder
        /// is `0x{1:x4}` — four lowercase hex digits, which is why the
        /// numeric argument is formatted rather than interpolated.
        internal static let characterNotInFontMessage =
            "The character '{0}' (0x{1:x4}) is not available in this SpriteFont. "
            + "If applicable, adjust the font's start and end CharacterRegions to "
            + "include this character."

        internal static func characterNotInFontMessage(_ value: UInt16) -> String {
            let scalar = Unicode.Scalar(value).map(String.init) ?? ""
            return characterNotInFontMessage
                .replacingOccurrences(of: "{0}", with: scalar)
                .replacingOccurrences(
                    of: "{1:x4}", with: String(format: "%04x", Int(value)))
        }

        /// Flushes what an infallible setter could not report, and answers the
        /// handle a draw needs. Called from `SpriteBatch.DrawString`.
        internal func validatedHandleForDraw() throws -> UInt64 {
            let handle = try box.validated("SpriteBatch.DrawString font")
            let functions = box.runtime.functions
            if pendingLineSpacing {
                try functions.check(
                    functions.spriteFontSetLineSpacing(handle, lineSpacing),
                    operation: "cna_sprite_font_set_line_spacing")
                pendingLineSpacing = false
            }
            if pendingSpacing {
                try functions.check(
                    functions.spriteFontSetSpacing(handle, spacing),
                    operation: "cna_sprite_font_set_spacing")
                pendingSpacing = false
            }
            return handle
        }

        /// The atlas, so a caller-built font's texture outlives it.
        internal var atlas: Texture2D? { texture }
    }
}
