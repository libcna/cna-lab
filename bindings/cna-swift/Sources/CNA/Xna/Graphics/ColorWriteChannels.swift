// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    public struct ColorWriteChannels: OptionSet {
        public let rawValue: Int32

        public init(rawValue: Int32) { self.rawValue = rawValue }

        public static let None = ColorWriteChannels([])
        public static let Red = ColorWriteChannels(rawValue: 1)
        public static let Green = ColorWriteChannels(rawValue: 2)
        public static let Blue = ColorWriteChannels(rawValue: 4)
        public static let Alpha = ColorWriteChannels(rawValue: 8)
        public static let All = ColorWriteChannels(rawValue: 15)
    }
}
