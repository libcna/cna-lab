// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework {
    public struct DisplayOrientation: OptionSet {
        public let rawValue: Int32

        public init(rawValue: Int32) { self.rawValue = rawValue }

        public static let Default = DisplayOrientation([])
        public static let LandscapeLeft = DisplayOrientation(rawValue: 1)
        public static let LandscapeRight = DisplayOrientation(rawValue: 2)
        public static let Portrait = DisplayOrientation(rawValue: 4)
    }
}
