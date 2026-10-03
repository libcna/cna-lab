// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Graphics {
    // Pinned in Microsoft.Xna.Framework.Graphics.dll as a public **sealed**
    // class extending `System.EventArgs`, whose only constructor is `assembly`
    // accessible: `.ctor(object resource)`. It therefore maps to a Swift
    // `final class` inheriting `CNAEventArgs` with no public initializer; the
    // internal initializer below is construction infrastructure, not an XNA
    // public member. This is the `DisplayModeCollection` precedent.
    //
    // The single public identity is the get-only `Resource`, whose IL returns
    // the stored `object` field verbatim. `System.Object` maps to `Any?`, so a
    // null resource is `nil`.
    //
    // Completing this type claims no graphics capability. XNA's only producer
    // is `GraphicsDevice.ResourceCreated`, which is a member of an untouched
    // runtime partial and is not implemented.
    public final class ResourceCreatedEventArgs: CNAEventArgs {
        private let resource: Any?

        internal init(resource: Any?) {
            self.resource = resource
            super.init()
        }

        public var Resource: Any? {
            resource
        }
    }

    // Pinned in the same assembly as a public **sealed** class extending
    // `System.EventArgs` with a single `assembly` constructor
    // `.ctor(string name, object tag)`. Both public identities are get-only
    // and return their stored fields verbatim.
    //
    // The IL stores `tag` before `name`, which is invisible in the public
    // surface; the Swift initializer keeps the CLR parameter order because it
    // is the order the pinned signature declares.
    //
    // As above, the only XNA producer is `GraphicsDevice.ResourceDestroyed` on
    // an untouched runtime partial.
    public final class ResourceDestroyedEventArgs: CNAEventArgs {
        private let name: String
        private let tag: Any?

        internal init(name: String, tag: Any?) {
            self.name = name
            self.tag = tag
            super.init()
        }

        public var Name: String {
            name
        }

        public var Tag: Any? {
            tag
        }
    }
}
