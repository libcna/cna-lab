// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Media {
    // Pinned as `.class public auto ansi sealed ... extends
    // [mscorlib]System.Enum` with an `int32 value__` and two literals, and no
    // `[Flags]` attribute. The two values are not contiguous: the pinned
    // literals are 0x00000000 and 0x00000004, and no intervening literal
    // exists. No `None` or `All` literal is invented to close the gap.
    public enum MediaSourceType: Int32 {
        case LocalDevice = 0
        case WindowsMediaConnect = 4
    }
}
