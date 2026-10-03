// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Audio {
    // Pinned in Microsoft.Xna.Framework.Xact.dll as
    // `.class public auto ansi sealed ... extends [mscorlib]System.Enum` with
    // an `int32 value__` and two literals.
    //
    // It carries no `[Flags]` attribute. That absence is explicitly
    // deliberate: the pinned binary instead carries
    // `SuppressMessageAttribute("Microsoft.Design",
    //  "CA1027:MarkEnumsWithFlags")`, which is the analyzer suggestion to mark
    // it as flags being suppressed. So this is an ordinary enum, not an
    // OptionSet.
    public enum AudioStopOptions: Int32 {
        case AsAuthored = 0
        case Immediate = 1
    }
}
