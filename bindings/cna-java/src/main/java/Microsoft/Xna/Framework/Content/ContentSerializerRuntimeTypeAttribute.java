package Microsoft.Xna.Framework.Content;

import System.Attribute;

import java.util.Objects;

/** Stores the runtime type name emitted for an intermediate content value. */
public final class ContentSerializerRuntimeTypeAttribute extends Attribute {
    private final String runtimeType;

    public ContentSerializerRuntimeTypeAttribute(String runtimeType) {
        this.runtimeType = Objects.requireNonNull(runtimeType, "runtimeType");
    }

    public String getRuntimeType() {
        return runtimeType;
    }
}
