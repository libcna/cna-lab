// SPDX-License-Identifier: MIT

import Foundation

/// The `System.Text.StringBuilder` projection.
///
/// `sealed` in `mscorlib`, so `final`. A BCL support class: it lives outside
/// `Microsoft.Xna.Framework`, is counted in no XNA scoreboard, and exists
/// because four XNA members take one — `SpriteFont.MeasureString(StringBuilder)`
/// and `SpriteBatch.DrawString`'s three `StringBuilder` overloads.
///
/// **The store is UTF-16 code units, not a `String`.** Every index in this
/// API — `Length`, `Item`, `Insert`, `Remove`, `ToString(startIndex:length:)`
/// — is a code-unit index, which is the same reason Foundation 70 mapped
/// `System.Char` to `UInt16`. A Swift `String` indexes by `Character`, a
/// grapheme cluster, and would silently change every one of them.
///
/// The projected surface is a **measured subset** of the pinned sixty-five, as
/// `CNAList` is sixteen of `List<T>`'s fifty-two: the pinned shape in
/// `bcl40-selected-shape.json` is the authority record of what the family is,
/// and `bclSupportContract` measures this class structurally. `AppendFormat`
/// and the seventeen numeric `Append` overloads are not here; they bring .NET
/// composite formatting with them and no XNA member reaches them.
public final class CNAStringBuilder {
    /// `DefaultCapacity`, `ldc.i4.s 16` in every constructor that does not
    /// name one.
    internal static let defaultCapacity: Int32 = 16

    private var units: [UInt16]
    private var capacity: Int32
    private let maxCapacity: Int32

    /// `StringBuilder()` — capacity 16, `MaxCapacity = Int32.MaxValue`.
    public init() {
        units = []
        capacity = CNAStringBuilder.defaultCapacity
        maxCapacity = Int32.max
    }

    /// `StringBuilder(String value)`.
    public init(_ value: String) {
        units = Array(value.utf16)
        capacity = max(CNAStringBuilder.defaultCapacity, Int32(units.count))
        maxCapacity = Int32.max
    }

    /// `StringBuilder(Int32 capacity)`.
    public convenience init(capacity: Int32) throws {
        try self.init(capacity: capacity, maxCapacity: Int32.max)
    }

    /// `StringBuilder(Int32 capacity, Int32 maxCapacity)`, whose three
    /// refusals come in this order:
    ///
    /// ```text
    /// capacity > maxCapacity -> ArgumentOutOfRangeException("capacity",
    ///                               ArgumentOutOfRange_Capacity)
    /// maxCapacity < 1        -> ArgumentOutOfRangeException("maxCapacity",
    ///                               ArgumentOutOfRange_SmallMaxCapacity)
    /// capacity < 0           -> ArgumentOutOfRangeException("capacity",
    ///                               Format(ArgumentOutOfRange_MustBePositive,
    ///                                      "capacity"))
    /// ```
    ///
    /// The third is the only one whose message is FORMATTED, and the argument
    /// it formats in is the parameter name — so `'capacity' must be greater
    /// than zero.` names the argument twice.
    public init(capacity: Int32, maxCapacity: Int32) throws {
        guard capacity <= maxCapacity else {
            throw CNAArgumentOutOfRangeException(
                paramName: "capacity",
                message: CNAStringBuilder.capacityExceedsMaximum)
        }
        guard maxCapacity >= 1 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "maxCapacity",
                message: CNAStringBuilder.smallMaxCapacity)
        }
        guard capacity >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "capacity",
                message: CNAStringBuilder.mustBePositive
                    .replacingOccurrences(of: "{0}", with: "capacity"))
        }
        units = []
        // `if (capacity == 0) capacity = Math.Min(DefaultCapacity, maxCapacity)`
        // — the floor is the SMALLER of sixteen and the ceiling just accepted,
        // so a builder with `maxCapacity` 4 does not start at 16.
        self.capacity = capacity == 0
            ? min(CNAStringBuilder.defaultCapacity, maxCapacity)
            : capacity
        self.maxCapacity = maxCapacity
    }

    // MARK: - The three properties

    /// `StringBuilder.Length` — `ldfld`-shaped, infallible.
    public var Length: Int32 { Int32(units.count) }

    /// `set_Length`, whose two refusals both name `"value"` and carry two
    /// different messages:
    ///
    /// ```text
    /// value < 0            -> ArgumentOutOfRange_NegativeLength
    /// value > MaxCapacity  -> ArgumentOutOfRange_SmallCapacity
    /// ```
    ///
    /// Growing appends `'\0'` repeated; shrinking truncates.
    public func SetLength(_ value: Int32) throws {
        guard value >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "value", message: CNAStringBuilder.negativeLength)
        }
        guard value <= maxCapacity else {
            throw CNAArgumentOutOfRangeException(
                paramName: "value", message: CNAStringBuilder.smallCapacity)
        }
        let wanted = Int(value)
        if wanted < units.count {
            units.removeLast(units.count - wanted)
        } else if wanted > units.count {
            units.append(contentsOf: repeatElement(0, count: wanted - units.count))
        }
        capacity = max(capacity, value)
    }

    /// `StringBuilder.Capacity` — **a lower bound here, not .NET's chunk
    /// arithmetic**, and that is a decision rather than an oversight.
    ///
    /// .NET computes it as `m_ChunkOffset + m_ChunkChars.Length` over a chain
    /// of chunks, and `ExpandByABlock` sizes each new one as
    ///
    /// ```text
    /// newBlockLength = Max(requiredAdditionalLength, Min(Length, 8000))
    /// ```
    ///
    /// so the buffer grows by at least its current length, doubling-ish,
    /// capped at `MaxChunkSize`. This class holds one flat `[UInt16]` and
    /// grows `capacity` to exactly what was asked for, so appending twenty
    /// characters to a fresh builder reports `20` where .NET reports `16 +
    /// newBlockLength`.
    ///
    /// **Why the difference is admitted rather than removed.** Reproducing the
    /// number means reproducing the chunk chain — `chunkOffset`, per-chunk
    /// lengths, the `MakeRoom`/`ExpandByABlock` split — which is allocation
    /// strategy, not observable text. No XNA member reads `Capacity`; the four
    /// this family was admitted for measure and draw the buffer's contents.
    /// A half-modelled chunk chain would be a *plausible* wrong number, which
    /// is worse than an honest smaller one.
    ///
    /// What is guaranteed, and what `Foundation71StringBuilderTests` pins:
    /// `Capacity >= Length` always, `Clear()` leaves it alone, and
    /// `SetCapacity` behaves exactly as the IL says including all three of its
    /// refusals. What is **not** guaranteed is the value .NET would have
    /// chosen after a growth.
    public var Capacity: Int32 { capacity }

    /// `set_Capacity`, three refusals, all naming `"value"`.
    public func SetCapacity(_ value: Int32) throws {
        guard value >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "value", message: CNAStringBuilder.negativeCapacity)
        }
        guard value <= maxCapacity else {
            throw CNAArgumentOutOfRangeException(
                paramName: "value", message: CNAStringBuilder.capacityExceedsMaximum)
        }
        guard value >= Length else {
            throw CNAArgumentOutOfRangeException(
                paramName: "value", message: CNAStringBuilder.smallCapacity)
        }
        capacity = value
    }

    /// `StringBuilder.MaxCapacity` — get-only, and `Int32.MaxValue` unless a
    /// constructor named one.
    public var MaxCapacity: Int32 { maxCapacity }

    // MARK: - The indexer, whose two accessors raise DIFFERENT exceptions

    /// `get_Chars(Int32)`.
    ///
    /// A **bare `IndexOutOfRangeException()`** — no message, no parameter
    /// name — where the setter raises `ArgumentOutOfRangeException`. One
    /// indexer, two exception types, for the same kind of mistake; it is the
    /// first thing a reimplementation gets wrong.
    public func Item(_ index: Int32) throws -> UInt16 {
        guard index >= 0, index < Length else {
            throw CNAIndexOutOfRangeException()
        }
        return units[Int(index)]
    }

    /// `set_Chars(Int32, Char)` — `ArgumentOutOfRangeException("index",
    /// ArgumentOutOfRange_Index)`, twice, for two conditions.
    public func SetItem(_ index: Int32, _ value: UInt16) throws {
        guard index >= 0, index < Length else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNAStringBuilder.indexOutOfRange)
        }
        units[Int(index)] = value
    }

    // MARK: - Growing
    //
    // EVERY growing member is fallible, and not for a reason its own body
    // shows: `ExpandByABlock` and `MakeRoom` raise
    // `ArgumentOutOfRangeException("requiredLength",
    // ArgumentOutOfRange_SmallCapacity)` when the result would exceed
    // MaxCapacity. A member that only appends still reaches it.

    @discardableResult
    public func Append(_ value: String) throws -> CNAStringBuilder {
        try grow(by: Array(value.utf16))
        return self
    }

    @discardableResult
    public func Append(_ value: UInt16) throws -> CNAStringBuilder {
        try grow(by: [value])
        return self
    }

    /// `Append(Char value, Int32 repeatCount)` — the one growing overload with
    /// a refusal of its own, `ArgumentOutOfRangeException("repeatCount",
    /// ArgumentOutOfRange_NegativeCount)`, raised before anything is written.
    @discardableResult
    public func Append(_ value: UInt16, repeatCount: Int32) throws -> CNAStringBuilder {
        guard repeatCount >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "repeatCount", message: CNAStringBuilder.negativeCount)
        }
        try grow(by: Array(repeatElement(value, count: Int(repeatCount))))
        return self
    }

    /// `AppendLine()` — appends `Environment.NewLine`, which on the CLR this
    /// binding targets is `"\r\n"`. It is a Windows runtime; the separator is
    /// not the host's.
    @discardableResult
    public func AppendLine() throws -> CNAStringBuilder {
        try Append(CNAStringBuilder.newLine)
    }

    @discardableResult
    public func AppendLine(_ value: String) throws -> CNAStringBuilder {
        try Append(value)
        return try Append(CNAStringBuilder.newLine)
    }

    /// `Insert(Int32 index, String value)`.
    ///
    /// XNA's bound is **one unsigned comparison** — `ble.un.s` against
    /// `Length` — which rejects a negative index because it reads as an
    /// enormous unsigned one. Two signed guards say the same thing and say it
    /// legibly; the outcome is identical and the instruction is not.
    ///
    /// A null `value` returns `this` untouched (`brfalse.s`), which a
    /// non-Optional Swift `String` cannot reach.
    @discardableResult
    public func Insert(_ index: Int32, _ value: String) throws -> CNAStringBuilder {
        guard index >= 0, index <= Length else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNAStringBuilder.indexOutOfRange)
        }
        let inserted = Array(value.utf16)
        try requireRoom(units.count + inserted.count)
        units.insert(contentsOf: inserted, at: Int(index))
        return self
    }

    // MARK: - Shrinking and reading

    /// `Remove(Int32 startIndex, Int32 length)`, whose three refusals come in
    /// this order: a negative length, a negative start, and a range past the
    /// end — the last of which names `"index"`, not `"startIndex"`.
    @discardableResult
    public func Remove(_ startIndex: Int32, _ length: Int32) throws -> CNAStringBuilder {
        guard length >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "length", message: CNAStringBuilder.negativeLength)
        }
        guard startIndex >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "startIndex", message: CNAStringBuilder.startIndex)
        }
        guard length <= Length - startIndex else {
            throw CNAArgumentOutOfRangeException(
                paramName: "index", message: CNAStringBuilder.indexOutOfRange)
        }
        units.removeSubrange(Int(startIndex)..<Int(startIndex + length))
        return self
    }

    /// `Clear()` — literally `this.Length = 0`, three instructions, and
    /// nothing else. **The capacity survives**, which is the whole reason to
    /// clear rather than build a new one. It cannot fail: `set_Length`'s two
    /// refusals are a negative value and one past `MaxCapacity`, and zero is
    /// neither.
    @discardableResult
    public func Clear() -> CNAStringBuilder {
        units.removeAll(keepingCapacity: true)
        return self
    }

    /// `ToString()`.
    public func ToString() -> String {
        String(decoding: units, as: UTF16.self)
    }

    /// `ToString(Int32 startIndex, Int32 length)`, four refusals in order.
    public func ToString(_ startIndex: Int32, _ length: Int32) throws -> String {
        guard startIndex >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "startIndex", message: CNAStringBuilder.startIndex)
        }
        guard startIndex <= Length else {
            throw CNAArgumentOutOfRangeException(
                paramName: "startIndex",
                message: CNAStringBuilder.startIndexLargerThanLength)
        }
        guard length >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "length", message: CNAStringBuilder.negativeLength)
        }
        guard startIndex + length <= Length else {
            throw CNAArgumentOutOfRangeException(
                paramName: "length", message: CNAStringBuilder.indexLength)
        }
        return String(
            decoding: units[Int(startIndex)..<Int(startIndex + length)], as: UTF16.self)
    }

    /// `EnsureCapacity(Int32)` — returns the capacity afterwards.
    @discardableResult
    public func EnsureCapacity(_ capacity: Int32) throws -> Int32 {
        guard capacity >= 0 else {
            throw CNAArgumentOutOfRangeException(
                paramName: "capacity", message: CNAStringBuilder.negativeCapacity)
        }
        if capacity > self.capacity { try SetCapacity(capacity) }
        return self.capacity
    }

    /// The code units, for the XNA members that measure and draw them. Not a
    /// CLR member: XNA reaches the buffer through `StringProxy`, which exists
    /// only to let one body serve a `String` and a `StringBuilder`.
    internal var codeUnits: [UInt16] { units }

    // MARK: - Growth, and the refusal it shares

    private func grow(by added: [UInt16]) throws {
        try requireRoom(units.count + added.count)
        units.append(contentsOf: added)
    }

    /// `ExpandByABlock`'s refusal, which is where every growing member's
    /// fallibility actually comes from.
    private func requireRoom(_ required: Int) throws {
        guard required <= Int(maxCapacity) else {
            throw CNAArgumentOutOfRangeException(
                paramName: "requiredLength", message: CNAStringBuilder.smallCapacity)
        }
        capacity = max(capacity, Int32(required))
    }

    // MARK: - The admitted mscorlib messages

    internal static let negativeLength = "Length cannot be less than zero."
    internal static let smallCapacity = "capacity was less than the current size."
    internal static let negativeCapacity = "Capacity must be positive."
    internal static let capacityExceedsMaximum = "Capacity exceeds maximum capacity."
    internal static let indexOutOfRange =
        "Index was out of range. Must be non-negative and less than the size of "
        + "the collection."
    internal static let negativeCount = "Count cannot be less than zero."
    internal static let startIndex = "StartIndex cannot be less than zero."
    internal static let startIndexLargerThanLength =
        "startIndex cannot be larger than length of string."
    internal static let indexLength =
        "Index and length must refer to a location within the string."
    /// `ArgumentOutOfRange_MustBePositive` is FORMATTED: its `{0}` is the
    /// parameter name, so the message a caller sees names the argument twice
    /// — once in `ParamName` and once inside the sentence.
    internal static let mustBePositive = "'{0}' must be greater than zero."
    internal static let smallMaxCapacity = "MaxCapacity must be one or greater."
    /// `Environment.NewLine` on the Windows CLR this binding targets.
    internal static let newLine = "\r\n"
}
