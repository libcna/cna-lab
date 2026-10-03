// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Media {
    // Pinned as `.class public auto ansi sealed beforefieldinit ... extends
    // [mscorlib]System.Enum` with an `int32 value__` and three literals. The
    // pinned binary carries no `[Flags]` attribute, so this is an ordinary
    // enum and not an OptionSet. The literals are deliberately not in
    // declaration order in the metadata; each raw value is transcribed from
    // its own `int32(...)` literal.
    public enum MediaState: Int32 {
        case Stopped = 0
        case Playing = 1
        case Paused = 2
    }
}
