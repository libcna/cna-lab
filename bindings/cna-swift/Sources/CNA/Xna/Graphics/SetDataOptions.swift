// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    public struct SetDataOptions: OptionSet {
        public let rawValue: Int32

        public init(rawValue: Int32) { self.rawValue = rawValue }

        public static let None = SetDataOptions([])
        public static let Discard = SetDataOptions(rawValue: 1)
        public static let NoOverwrite = SetDataOptions(rawValue: 2)
    }
}
