// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Content {
    /// `Microsoft.Xna.Framework.Content.ContentSerializerIgnoreAttribute`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.dll` as a public **sealed** class on
    /// `System.Attribute` declaring a single constructor whose IL body is
    /// exactly `base..ctor()`. It carries no state at all: its whole meaning
    /// is its presence, which is precisely why the base identity is the thing
    /// that had to be projected.
    ///
    /// CLR `sealed` maps to Swift `final`.
    public final class ContentSerializerIgnoreAttribute: CNAAttribute {
        /// `.ctor()`.
        public override init() {
            super.init()
        }
    }

    /// `Microsoft.Xna.Framework.Content.ContentSerializerCollectionItemNameAttribute`.
    ///
    /// Public **sealed** on `System.Attribute`, with one constructor and one
    /// get-only property.
    public final class ContentSerializerCollectionItemNameAttribute: CNAAttribute {
        private let collectionItemName: String

        /// `.ctor(String collectionItemName)`.
        ///
        /// The IL calls `base..ctor()` **first**, then raises
        /// `ArgumentNullException("collectionItemName")` when the argument is
        /// null **or empty**, and only then stores it. So an empty string is
        /// refused here, unlike `ContentSerializerAttribute`, which quietly
        /// substitutes a default for an unset one.
        public init(collectionItemName: String) throws {
            guard !collectionItemName.isEmpty else {
                throw CNAArgumentNullException(paramName: "collectionItemName")
            }
            self.collectionItemName = collectionItemName
            super.init()
        }

        /// `CollectionItemName` — the stored field, and non-Optional because
        /// the constructor refuses every value that could have made it null.
        public var CollectionItemName: String { collectionItemName }
    }

    /// `Microsoft.Xna.Framework.Content.ContentSerializerRuntimeTypeAttribute`.
    ///
    /// Public **sealed** on `System.Attribute`. Identical in shape to
    /// `ContentSerializerCollectionItemNameAttribute`, including the
    /// null-or-empty refusal.
    public final class ContentSerializerRuntimeTypeAttribute: CNAAttribute {
        private let runtimeType: String

        /// `.ctor(String runtimeType)`.
        ///
        /// `base..ctor()`, then `ArgumentNullException("runtimeType")` for a
        /// null or empty argument, then the store.
        ///
        /// The value is the assembly-qualified NAME of a type, carried as a
        /// string. Nothing here resolves it: doing that would need
        /// `System.Type`, and inventing a resolution would be worse than
        /// carrying the string XNA carries.
        public init(runtimeType: String) throws {
            guard !runtimeType.isEmpty else {
                throw CNAArgumentNullException(paramName: "runtimeType")
            }
            self.runtimeType = runtimeType
            super.init()
        }

        /// `RuntimeType` — the stored field, non-Optional for the same reason.
        public var RuntimeType: String { runtimeType }
    }

    /// `Microsoft.Xna.Framework.Content.ContentSerializerTypeVersionAttribute`.
    ///
    /// Public **sealed** on `System.Attribute`. The only one of the five whose
    /// constructor validates nothing: the IL is `base..ctor()` then a plain
    /// store, so a negative version is accepted exactly as XNA accepts it.
    public final class ContentSerializerTypeVersionAttribute: CNAAttribute {
        private let typeVersion: Int32

        /// `.ctor(Int32 typeVersion)`.
        public init(typeVersion: Int32) {
            self.typeVersion = typeVersion
            super.init()
        }

        /// `TypeVersion` — the stored field.
        public var TypeVersion: Int32 { typeVersion }
    }
}
