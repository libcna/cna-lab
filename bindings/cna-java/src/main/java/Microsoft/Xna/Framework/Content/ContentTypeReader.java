package Microsoft.Xna.Framework.Content;

import java.util.Objects;

/** Base class for a strongly typed managed XNB content reader. */
public abstract class ContentTypeReader {

    private Class<?> targetType;

    protected ContentTypeReader(Class<?> targetType) {
        this.targetType = Objects.requireNonNull(targetType, "targetType");
    }

    public boolean getCanDeserializeIntoExistingObject() {
        return false;
    }

    public final Class<?> getTargetType() {
        return targetType;
    }

    public int getTypeVersion() {
        return 0;
    }

    protected void Initialize(ContentTypeReaderManager manager) {
        Objects.requireNonNull(manager, "manager");
    }

    protected abstract Object Read(ContentReader input, Object existingInstance);

    final Object readValue(ContentReader input, Object existingInstance) {
        return Read(input, existingInstance);
    }

    final void initializeReader(ContentTypeReaderManager manager) {
        Initialize(manager);
    }

    final void setInferredTargetType(Class<?> value) {
        targetType = Objects.requireNonNull(value, "value");
    }
}
