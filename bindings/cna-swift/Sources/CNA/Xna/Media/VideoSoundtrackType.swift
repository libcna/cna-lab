// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Media {
    // Pinned in Microsoft.Xna.Framework.Video.dll as
    // `.class public auto ansi sealed ... extends [mscorlib]System.Enum` with
    // an `int32 value__`, three literals, and no `[Flags]` attribute.
    public enum VideoSoundtrackType: Int32 {
        case Music = 0
        case Dialog = 1
        case MusicAndDialog = 2
    }
}
