// SPDX-License-Identifier: MIT

/// `System.IO.FileMode`.
///
/// Pinned in `reference/bcl40-selected-shape.json` with its six values and
/// their numbers. It carries no `FlagsAttribute`, so it is a Swift enum and
/// not an `OptionSet` — the difference is not cosmetic: `Open|Append` is a
/// meaningless value the CLR type cannot express and an OptionSet would.
public enum CNAFileMode: Int32 {
    case CreateNew = 1
    case Create = 2
    case Open = 3
    case OpenOrCreate = 4
    case Truncate = 5
    case Append = 6
}

/// `System.IO.FileAccess`.
///
/// Carries `FlagsAttribute` in the assembly, so it projects as an `OptionSet`
/// with the CLR's `Int32` as `rawValue`. `ReadWrite` is `Read|Write` and is
/// spelled as the union rather than as a third literal, which is what the
/// pinned value 3 says it is.
public struct CNAFileAccess: OptionSet {
    public let rawValue: Int32

    public init(rawValue: Int32) { self.rawValue = rawValue }

    public static let Read = CNAFileAccess(rawValue: 1)
    public static let Write = CNAFileAccess(rawValue: 2)
    public static let ReadWrite: CNAFileAccess = [.Read, .Write]
}

/// `System.IO.FileShare`.
///
/// Also `[Flags]`. `None` is the empty set, and `Inheritable` is 16 rather
/// than the 8 a reader expects from the sequence — there is no value 8, and
/// the gap is in the assembly, so it is here.
public struct CNAFileShare: OptionSet {
    public let rawValue: Int32

    public init(rawValue: Int32) { self.rawValue = rawValue }

    public static let None = CNAFileShare([])
    public static let Read = CNAFileShare(rawValue: 1)
    public static let Write = CNAFileShare(rawValue: 2)
    public static let ReadWrite: CNAFileShare = [.Read, .Write]
    public static let Delete = CNAFileShare(rawValue: 4)
    public static let Inheritable = CNAFileShare(rawValue: 16)
}
