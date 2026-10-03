package Microsoft.Xna.Framework.Content;

import System.Attribute;

/** Stores the content serializer version assigned to a type. */
public final class ContentSerializerTypeVersionAttribute extends Attribute {
    private final int typeVersion;

    public ContentSerializerTypeVersionAttribute(int typeVersion) {
        this.typeVersion = typeVersion;
    }

    public int getTypeVersion() {
        return typeVersion;
    }
}
