package Microsoft.Xna.Framework.Content;

import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;

/** Strongly typed derivation point for user-defined XNB readers. */
public abstract class ContentTypeReaderOfT<T> extends ContentTypeReader {

    @SuppressWarnings("this-escape")
    protected ContentTypeReaderOfT() {
        super(Object.class);
        setInferredTargetType(inferTargetType(getClass()));
    }

    @Override
    @SuppressWarnings("unchecked")
    protected Object Read(ContentReader input, Object existingInstance) {
        T typedExisting = existingInstance == null
                ? null
                : (T) getTargetType().cast(existingInstance);
        return ReadTyped(input, typedExisting);
    }

    /** Typed Java extension point corresponding to XNA's colliding generic Read overload. */
    protected abstract T ReadTyped(ContentReader input, T existingInstance);

    private static Class<?> inferTargetType(Class<?> readerClass) {
        Class<?> current = readerClass;
        while (current != null && current != Object.class) {
            Type generic = current.getGenericSuperclass();
            if (generic instanceof ParameterizedType parameterized
                    && parameterized.getRawType() == ContentTypeReaderOfT.class) {
                Type target = parameterized.getActualTypeArguments()[0];
                if (target instanceof Class<?> concrete) {
                    return concrete;
                }
                if (target instanceof ParameterizedType nested
                        && nested.getRawType() instanceof Class<?> raw) {
                    return raw;
                }
                throw new IllegalStateException(
                        "ContentTypeReaderOfT target must resolve to a concrete Java class");
            }
            current = current.getSuperclass();
        }
        throw new IllegalStateException(
                "ContentTypeReaderOfT subclass does not declare a concrete target type");
    }
}
