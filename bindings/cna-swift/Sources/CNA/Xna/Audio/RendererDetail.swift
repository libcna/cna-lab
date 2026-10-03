// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Audio {

    /// `Microsoft.Xna.Framework.Audio.RendererDetail`.
    ///
    /// One audio renderer's identity: a name to show and an id to pass back.
    /// Entirely managed -- two strings, value equality over them, and the
    /// `ToString` that renders the friendly name.
    ///
    /// XNA produces these from `AudioEngine.RendererDetails`; the projected
    /// engine now does the same by copying CNA's renderer name and id into this
    /// managed value.
    ///
    /// Both getters are `IL_NO_FAILURE_PATH` and neither return is *proven*
    /// nullable, so both are non-Optional -- the same rule
    /// `GraphicsAdapter.DefaultAdapter` was decided by. A default-constructed
    /// value therefore reads two empty strings rather than two nils.
    public struct RendererDetail {

        private let friendlyName: String
        private let rendererId: String

        /// **Internal, and the reference metadata is why.** C# gives every
        /// struct an implicit parameterless constructor, but the pinned
        /// contract declares none -- so a public Swift `init()` is a member
        /// XNA does not have, and the strict comparison says so by name. The
        /// engine fills values through the internal two-string initializer.
        internal init() {
            self.init(friendlyName: "", rendererId: "")
        }

        internal init(friendlyName: String, rendererId: String) {
            self.friendlyName = friendlyName
            self.rendererId = rendererId
        }

        /// `RendererDetail.FriendlyName`.
        public var FriendlyName: String { friendlyName }

        /// `RendererDetail.RendererId`.
        public var RendererId: String { rendererId }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? RendererDetail else { return false }
            return self == other
        }

        /// `rendererId.GetHashCode()` -- the **id** alone, not the pair: two
        /// details naming the same renderer are the same renderer whatever
        /// they are called.
        public func GetHashCode() -> Int32 {
            Microsoft.Xna.Framework.Audio.RendererDetail.stringHash(rendererId)
        }

        /// `friendlyName`, which is what XNA's `ToString` returns -- not a
        /// braced field list like the geometry types.
        public func ToString() -> String { friendlyName }

        public static func == (lhs: RendererDetail, rhs: RendererDetail) -> Bool {
            lhs.rendererId == rhs.rendererId
        }

        public static func != (lhs: RendererDetail, rhs: RendererDetail) -> Bool {
            !(lhs == rhs)
        }

        /// The CLR's `String.GetHashCode` is not specified across runtimes, so
        /// this reproduces a stable hash rather than claiming to be Microsoft's
        /// -- what the contract requires is that equal values hash equally.
        private static func stringHash(_ value: String) -> Int32 {
            var hash: Int32 = 0
            for unit in value.utf16 {
                hash = hash &* 31 &+ Int32(unit)
            }
            return hash
        }
    }
}
