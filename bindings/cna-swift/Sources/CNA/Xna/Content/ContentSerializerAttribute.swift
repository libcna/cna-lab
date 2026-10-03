// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Content {
    /// `Microsoft.Xna.Framework.Content.ContentSerializerAttribute`.
    ///
    /// Pinned in `Microsoft.Xna.Framework.dll` as a public **sealed** class
    /// whose direct base is `System.Attribute`. The largest of the five
    /// serializer attributes: six settable knobs, a derived `CollectionItemName`
    /// with a substituted default, a `HasCollectionItemName` that reports
    /// whether the raw field was ever set, and a `Clone`.
    ///
    /// CLR `sealed` maps to Swift `final`.
    public final class ContentSerializerAttribute: CNAAttribute {
        // The six private fields, held exactly as the CLR holds them. The
        // distinction between the RAW field and what `CollectionItemName`
        // reports is the whole behaviour of this type, so the field is kept
        // separate from the property rather than folded into it.
        private var elementName: String?
        private var flattenContent = false
        private var optional = false
        private var allowNull = true
        private var sharedResource = false
        private var collectionItemName: String?

        /// `.ctor()`.
        ///
        /// The IL sets `allowNull = true` **before** calling
        /// `Attribute::.ctor()`, and leaves every other field at its zero
        /// value. `AllowNull` therefore starts `true` and the other three
        /// Booleans start `false` — a default a plausible reimplementation
        /// gets wrong by making them uniform.
        public override init() {
            super.init()
        }

        /// `ElementName` — a plain field, and the only string here that stays
        /// nullable: nothing substitutes a default for it and no constructor
        /// assigns it.
        public var ElementName: String? {
            get { elementName }
            set { elementName = newValue }
        }

        /// `FlattenContent` — a plain field, `false` by default.
        public var FlattenContent: Bool {
            get { flattenContent }
            set { flattenContent = newValue }
        }

        /// `Optional` — a plain field, `false` by default.
        public var Optional: Bool {
            get { optional }
            set { optional = newValue }
        }

        /// `AllowNull` — a plain field, and the one the constructor sets to
        /// `true`.
        public var AllowNull: Bool {
            get { allowNull }
            set { allowNull = newValue }
        }

        /// `SharedResource` — a plain field, `false` by default.
        public var SharedResource: Bool {
            get { sharedResource }
            set { sharedResource = newValue }
        }

        /// `CollectionItemName` getter.
        ///
        /// `IsNullOrEmpty(field) ? "Item" : field` — so it **never** returns
        /// null or an empty string, which is why the projection is a
        /// non-Optional `String`. That verdict is the pinned inventory's, and
        /// deriving it is what found the analyser's `IsNullOrEmpty` blind spot.
        public var CollectionItemName: String {
            guard let name = collectionItemName, !name.isEmpty else {
                return ContentSerializerAttribute.defaultCollectionItemName
            }
            return name
        }

        /// The literal `get_CollectionItemName` substitutes.
        private static let defaultCollectionItemName = "Item"

        /// `CollectionItemName` setter, as the writer method the accessor rule
        /// requires for a fallible setter.
        ///
        /// The IL raises `ArgumentNullException("value")` when the incoming
        /// value is null **or empty**, and only then stores it. An empty
        /// string is therefore refused, not stored — which is a different
        /// thing from the getter substituting a default for one.
        public func SetCollectionItemName(_ value: String) throws {
            guard !value.isEmpty else {
                throw CNAArgumentNullException(paramName: "value")
            }
            collectionItemName = value
        }

        /// `HasCollectionItemName`.
        ///
        /// `!IsNullOrEmpty(field)` — it reports the RAW field, so it is
        /// `false` on a fresh instance even though `CollectionItemName`
        /// already answers `"Item"`.
        public var HasCollectionItemName: Bool {
            guard let name = collectionItemName else { return false }
            return !name.isEmpty
        }

        /// `Clone()`.
        ///
        /// A fresh instance with the six **fields** copied directly — not the
        /// properties. So a clone of an instance whose `collectionItemName`
        /// was never set carries the null field on, and its
        /// `HasCollectionItemName` is `false` too; cloning through the
        /// property would have turned the substituted `"Item"` into a real
        /// stored value and flipped that flag.
        public func Clone() -> ContentSerializerAttribute {
            let copy = ContentSerializerAttribute()
            copy.elementName = elementName
            copy.flattenContent = flattenContent
            copy.optional = optional
            copy.allowNull = allowNull
            copy.sharedResource = sharedResource
            copy.collectionItemName = collectionItemName
            return copy
        }
    }
}
