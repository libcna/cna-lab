// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Audio {
    // Pinned as `.class public auto ansi sealed ... extends
    // [mscorlib]System.Enum` with an `int32 value__` and two literals, and no
    // `[Flags]` attribute. `Started` is the zero literal and `Stopped` is 1,
    // which is the opposite of the ordering a reader might assume; both are
    // transcribed from their own pinned `int32(...)` literal.
    public enum MicrophoneState: Int32 {
        case Started = 0
        case Stopped = 1
    }
}
