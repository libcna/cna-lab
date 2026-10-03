// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Input.Touch {
    // Pinned in Microsoft.Xna.Framework.Input.Touch.dll as
    // `.class public auto ansi sealed ... extends [mscorlib]System.Enum`
    // carrying `.custom instance void [mscorlib]System.FlagsAttribute::.ctor()`.
    // `[Flags]` presence is read out of the binary, so this maps to a Swift
    // OptionSet with the CLR underlying type as `rawValue`. The zero literal
    // `None` is spelled as the empty set; the other ten are single bits from
    // 0x001 to 0x200.
    public struct GestureType: OptionSet {
        public let rawValue: Int32

        public init(rawValue: Int32) { self.rawValue = rawValue }

        public static let None = GestureType([])
        public static let Tap = GestureType(rawValue: 1)
        public static let DoubleTap = GestureType(rawValue: 2)
        public static let Hold = GestureType(rawValue: 4)
        public static let HorizontalDrag = GestureType(rawValue: 8)
        public static let VerticalDrag = GestureType(rawValue: 16)
        public static let FreeDrag = GestureType(rawValue: 32)
        public static let Pinch = GestureType(rawValue: 64)
        public static let Flick = GestureType(rawValue: 128)
        public static let DragComplete = GestureType(rawValue: 256)
        public static let PinchComplete = GestureType(rawValue: 512)
    }
}
