// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Input.Touch {
    // Pinned in Microsoft.Xna.Framework.Input.Touch.dll as
    // `.class public auto ansi sealed ... extends [mscorlib]System.Enum` with
    // an `int32 value__`, four contiguous literals, and no `[Flags]`
    // attribute. `Invalid` is the zero literal.
    public enum TouchLocationState: Int32 {
        case Invalid = 0
        case Released = 1
        case Pressed = 2
        case Moved = 3
    }
}
