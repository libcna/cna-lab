// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    // The pinned XNA 4.0 Windows metadata declares a public, non-sealed class
    // whose only constructor is `assembly` accessible:
    // `.ctor(List<DisplayMode> displayModes)`. A CLR class with no accessible
    // constructor cannot be constructed or derived from outside its own
    // assembly, so this projection is deliberately not `open`; metadata
    // sealed=false keeps it non-`final`. The Swift initializer below is the
    // corresponding internal construction path and is implementation
    // infrastructure, not an XNA public member.
    //
    // The public contract is exactly two identities: `GetEnumerator()` and the
    // read-only indexed property `Item[SurfaceFormat]`. The explicit
    // `System.Collections.IEnumerable.GetEnumerator` in the pinned IL is a
    // private explicit interface implementation and is not public contract.
    //
    // Completing this type claims **no** display or adapter capability. It has
    // no public constructor, so no consumer can obtain an instance; the only
    // producer in XNA is `GraphicsAdapter` mode enumeration, which is not
    // implemented. Nothing here queries a display, enumerates an adapter, or
    // invents a mode. This is the same footing on which `DisplayMode` itself
    // was completed in Foundation 12.
    public class DisplayModeCollection {
        private let displayModes: [Microsoft.Xna.Framework.Graphics.DisplayMode]

        internal init(
            displayModes: [Microsoft.Xna.Framework.Graphics.DisplayMode]
        ) {
            self.displayModes = displayModes
        }

        // `GetEnumerator` returns the backing `List<DisplayMode>` enumerator
        // directly, so enumeration is in stored order. The list is never
        // mutated after construction, so the CLR mutation-invalidation path
        // that `CNAEnumerator` preserves can never fire for this type; the
        // version is captured once and stays constant.
        public func GetEnumerator()
            -> CNAEnumerator<Microsoft.Xna.Framework.Graphics.DisplayMode>
        {
            let snapshot = displayModes
            return CNAEnumerator(expectedVersion: 0) { index, _ in
                index < snapshot.count ? snapshot[index] : nil
            }
        }

        // `get_Item` walks the backing list once and returns a **new** list
        // holding every mode whose `Format` equals the argument, in the
        // original order. It is a fresh filtered sequence, not a live view,
        // and an unmatched format yields an empty result rather than an error.
        public subscript(
            format: Microsoft.Xna.Framework.Graphics.SurfaceFormat
        ) -> [Microsoft.Xna.Framework.Graphics.DisplayMode] {
            displayModes.filter { $0.Format == format }
        }
    }
}
