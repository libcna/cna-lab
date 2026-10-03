package Microsoft.Xna.Framework.Content;

import System.Attribute;

import java.util.Objects;

/** Specifies the serialized item element name for a collection. */
public final class ContentSerializerCollectionItemNameAttribute extends Attribute {
    private final String collectionItemName;

    public ContentSerializerCollectionItemNameAttribute(String collectionItemName) {
        this.collectionItemName = Objects.requireNonNull(collectionItemName, "collectionItemName");
    }

    public String getCollectionItemName() {
        return collectionItemName;
    }
}
