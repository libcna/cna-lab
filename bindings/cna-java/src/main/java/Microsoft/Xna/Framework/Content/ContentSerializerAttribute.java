package Microsoft.Xna.Framework.Content;

import System.Attribute;

/** Mutable XNA content-serialization metadata value. */
public final class ContentSerializerAttribute extends Attribute {

    private boolean allowNull = true;
    private String collectionItemName;
    private String elementName;
    private boolean flattenContent;
    private boolean optional;
    private boolean sharedResource;

    public ContentSerializerAttribute() {
    }

    public ContentSerializerAttribute Clone() {
        ContentSerializerAttribute copy = new ContentSerializerAttribute();
        copy.allowNull = allowNull;
        copy.collectionItemName = collectionItemName;
        copy.elementName = elementName;
        copy.flattenContent = flattenContent;
        copy.optional = optional;
        copy.sharedResource = sharedResource;
        return copy;
    }

    public boolean getAllowNull() { return allowNull; }
    public void setAllowNull(boolean value) { allowNull = value; }
    public String getCollectionItemName() {
        return collectionItemName == null || collectionItemName.isEmpty()
                ? "Item" : collectionItemName;
    }
    public void setCollectionItemName(String value) {
        if (value == null || value.isEmpty()) {
            throw new IllegalArgumentException("CollectionItemName must not be empty");
        }
        collectionItemName = value;
    }
    public String getElementName() { return elementName; }
    public void setElementName(String value) { elementName = value; }
    public boolean getFlattenContent() { return flattenContent; }
    public void setFlattenContent(boolean value) { flattenContent = value; }
    public boolean getHasCollectionItemName() {
        return collectionItemName != null && !collectionItemName.isEmpty();
    }
    public boolean getOptional() { return optional; }
    public void setOptional(boolean value) { optional = value; }
    public boolean getSharedResource() { return sharedResource; }
    public void setSharedResource(boolean value) { sharedResource = value; }
}
