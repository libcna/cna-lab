// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    public struct ClearOptions: OptionSet {
        public let rawValue: Int32

        public init(rawValue: Int32) { self.rawValue = rawValue }

        public static let Target = ClearOptions(rawValue: 1)
        public static let DepthBuffer = ClearOptions(rawValue: 2)
        public static let Stencil = ClearOptions(rawValue: 4)
    }
}
