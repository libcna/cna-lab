// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    public struct BufferUsage: OptionSet {
        public let rawValue: Int32

        public init(rawValue: Int32) { self.rawValue = rawValue }

        public static let None = BufferUsage([])
        public static let WriteOnly = BufferUsage(rawValue: 1)
    }
}
