package Microsoft.Xna.Framework.Content;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import System.Action;
import System.IO.BinaryReader;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UncheckedIOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/** XNA XNB object-graph reader implemented entirely in managed Java. */
public final class ContentReader extends BinaryReader {

    private final ContentManager contentManager;
    private final String assetName;
    private final Action<AutoCloseable> recordDisposableObject;
    private ContentTypeReader[] typeReaders = new ContentTypeReader[0];
    private int[] typeReaderVersions = new int[0];
    private List<Action<Object>>[] sharedResourceFixups;

    private ContentReader(
            ContentManager contentManager,
            InputStream input,
            String assetName,
            Action<AutoCloseable> recordDisposableObject) {
        super(input);
        this.contentManager = Objects.requireNonNull(contentManager, "contentManager");
        this.assetName = Objects.requireNonNull(assetName, "assetName");
        this.recordDisposableObject = recordDisposableObject;
    }

    static ContentReader create(
            ContentManager manager,
            InputStream input,
            String assetName,
            Action<AutoCloseable> recordDisposableObject) {
        Objects.requireNonNull(input, "input");
        byte[] bytes;
        try {
            bytes = input.readAllBytes();
        } catch (IOException exception) {
            throw new ContentLoadException(
                    "Could not read content asset '" + assetName + "'",
                    new UncheckedIOException(exception));
        }
        if (bytes.length < 10) {
            throw failure(assetName, "truncated XNB header");
        }
        if (bytes[0] != 'X' || bytes[1] != 'N' || bytes[2] != 'B') {
            throw failure(assetName, "invalid XNB magic");
        }
        if (bytes[3] != 'w') {
            throw failure(assetName, "unsupported XNB platform '"
                    + (char)Byte.toUnsignedInt(bytes[3]) + "'");
        }
        int version = Byte.toUnsignedInt(bytes[4]);
        int flags = Byte.toUnsignedInt(bytes[5]);
        if (version != 5) {
            throw failure(assetName, "unsupported XNB version " + version);
        }
        int declaredSize = Byte.toUnsignedInt(bytes[6])
                | Byte.toUnsignedInt(bytes[7]) << 8
                | Byte.toUnsignedInt(bytes[8]) << 16
                | Byte.toUnsignedInt(bytes[9]) << 24;
        if (declaredSize != bytes.length) {
            throw failure(assetName, "declared XNB size " + declaredSize
                    + " does not match stream size " + bytes.length);
        }
        if ((flags & 0x40) != 0) {
            throw failure(assetName,
                    "LZ4-compressed XNB is not part of the selected XNA 4.0 format");
        }
        byte[] payload;
        if ((flags & 0x80) != 0) {
            if (bytes.length < 14) {
                throw failure(assetName, "truncated LZX payload header");
            }
            int decompressedSize = Byte.toUnsignedInt(bytes[10])
                    | Byte.toUnsignedInt(bytes[11]) << 8
                    | Byte.toUnsignedInt(bytes[12]) << 16
                    | Byte.toUnsignedInt(bytes[13]) << 24;
            payload = XnbLzxDecompression.decompress(
                    java.util.Arrays.copyOfRange(bytes, 14, bytes.length),
                    decompressedSize, assetName);
        } else {
            payload = java.util.Arrays.copyOfRange(bytes, 10, bytes.length);
        }
        return new ContentReader(manager,
                new ByteArrayInputStream(payload),
                assetName, recordDisposableObject);
    }

    public final String getAssetName() {
        return assetName;
    }

    public final ContentManager getContentManager() {
        return contentManager;
    }

    @Override
    public double ReadDouble() {
        return super.ReadDouble();
    }

    @Override
    public float ReadSingle() {
        return super.ReadSingle();
    }

    public Color ReadColor() {
        return new Color(ReadByte(), ReadByte(), ReadByte(), ReadByte());
    }

    public Matrix ReadMatrix() {
        return new Matrix(
                ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle(),
                ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle(),
                ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle(),
                ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle());
    }

    public Quaternion ReadQuaternion() {
        return new Quaternion(ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle());
    }

    public Vector2 ReadVector2() {
        return new Vector2(ReadSingle(), ReadSingle());
    }

    public Vector3 ReadVector3() {
        return new Vector3(ReadSingle(), ReadSingle(), ReadSingle());
    }

    public Vector4 ReadVector4() {
        return new Vector4(ReadSingle(), ReadSingle(), ReadSingle(), ReadSingle());
    }

    public <T> T ReadObject(Class<T> targetType) {
        return readObjectInternal(targetType, null, false);
    }

    public <T> T ReadObject(Class<T> targetType, T existingInstance) {
        return readObjectInternal(targetType, existingInstance, existingInstance != null);
    }

    public <T> T ReadObject(Class<T> targetType, ContentTypeReader typeReader) {
        Objects.requireNonNull(typeReader, "typeReader");
        if (isValueType(typeReader.getTargetType())) {
            return readAndRecord(targetType, typeReader, null, false);
        }
        return readObjectInternal(targetType, null, false);
    }

    public <T> T ReadObject(
            Class<T> targetType, ContentTypeReader typeReader, T existingInstance) {
        Objects.requireNonNull(typeReader, "typeReader");
        if (isValueType(typeReader.getTargetType())) {
            return readAndRecord(
                    targetType, typeReader, existingInstance, existingInstance != null);
        }
        return readObjectInternal(targetType, existingInstance, existingInstance != null);
    }

    public <T> T ReadRawObject(Class<T> targetType) {
        return readAndRecord(targetType, findTypeReader(targetType), null, false);
    }

    public <T> T ReadRawObject(Class<T> targetType, T existingInstance) {
        return readAndRecord(targetType, findTypeReader(targetType),
                existingInstance, existingInstance != null);
    }

    public <T> T ReadRawObject(Class<T> targetType, ContentTypeReader typeReader) {
        return readAndRecord(targetType, Objects.requireNonNull(typeReader, "typeReader"),
                null, false);
    }

    public <T> T ReadRawObject(
            Class<T> targetType, ContentTypeReader typeReader, T existingInstance) {
        return readAndRecord(targetType, Objects.requireNonNull(typeReader, "typeReader"),
                existingInstance, existingInstance != null);
    }

    public <T> void ReadSharedResource(Class<T> targetType, Action<T> fixup) {
        Class<T> expectedType = Objects.requireNonNull(targetType, "targetType");
        Action<T> callback = Objects.requireNonNull(fixup, "fixup");
        int index = read7BitEncodedInt32();
        if (index == 0) {
            return;
        }
        if (sharedResourceFixups == null || index < 1 || index > sharedResourceFixups.length) {
            throw failure("invalid shared resource index " + index);
        }
        sharedResourceFixups[index - 1].add(value -> callback.invoke(
                checkedCast(expectedType, value, "shared resource " + index)));
    }

    public <T> T ReadExternalReference(Class<T> targetType) {
        Objects.requireNonNull(targetType, "targetType");
        String reference = ReadString();
        if (reference.isEmpty()) {
            return null;
        }
        String parent = "";
        int slash = Math.max(assetName.lastIndexOf('/'), assetName.lastIndexOf('\\'));
        if (slash >= 0) {
            parent = assetName.substring(0, slash);
        }
        String resolved = parent.isEmpty()
                ? reference : Path.of(parent).resolve(reference).normalize().toString();
        return contentManager.Load(targetType, resolved.replace('\\', '/'));
    }

    <T> T readAsset(Class<T> targetType, ContentTypeReaderManager readerManager) {
        typeReaders = readerManager.loadAssetReaders(this);
        int sharedCount = read7BitEncodedInt32();
        if (sharedCount < 0 || sharedCount > 1_000_000) {
            throw failure("invalid shared resource count " + sharedCount);
        }
        @SuppressWarnings("unchecked")
        List<Action<Object>>[] fixups = (List<Action<Object>>[])new List<?>[sharedCount];
        for (int index = 0; index < sharedCount; index++) {
            fixups[index] = new ArrayList<>();
        }
        sharedResourceFixups = fixups;
        T root = ReadObject(targetType);
        Object[] resources = new Object[sharedCount];
        for (int index = 0; index < sharedCount; index++) {
            resources[index] = ReadObject(Object.class);
        }
        for (int index = 0; index < sharedCount; index++) {
            if (resources[index] == null && !fixups[index].isEmpty()) {
                throw failure("shared resource " + (index + 1) + " is null");
            }
            for (Action<Object> fixup : fixups[index]) {
                fixup.invoke(resources[index]);
            }
        }
        return root;
    }

    int read7BitEncodedInt32() {
        try {
            return Read7BitEncodedInt();
        } catch (RuntimeException exception) {
            throw new ContentLoadException(
                    "Content asset '" + assetName + "' has an invalid 7-bit integer", exception);
        }
    }

    byte[] readByteBuffer(int count) {
        try {
            return ReadBytesExact(count);
        } catch (RuntimeException exception) {
            throw new ContentLoadException(
                    "Content asset '" + assetName + "' has a truncated byte payload", exception);
        }
    }

    void setTypeReaderVersions(int[] versions) {
        typeReaderVersions = versions.clone();
    }

    int getTypeReaderVersion(ContentTypeReader reader) {
        for (int index = 0; index < typeReaders.length; index++) {
            if (typeReaders[index] == reader) {
                return typeReaderVersions[index];
            }
        }
        throw new IllegalArgumentException("Reader is not in this asset's type table");
    }

    void recordManagedNativeObject(Object value) {
        contentManager.recordManagedNativeObject(value);
    }

    ContentLoadException failure(String detail) {
        return failure(assetName, detail);
    }

    private <T> T readObjectInternal(
            Class<T> targetType, T existingInstance, boolean hasExistingInstance) {
        int readerIndex = read7BitEncodedInt32();
        if (readerIndex == 0) {
            if (targetType.isPrimitive()) {
                throw failure("null object cannot be read as " + targetType.getName());
            }
            return null;
        }
        if (readerIndex < 1 || readerIndex > typeReaders.length) {
            throw failure("invalid type reader index " + readerIndex);
        }
        return readAndRecord(targetType, typeReaders[readerIndex - 1],
                existingInstance, hasExistingInstance);
    }

    private <T> T readAndRecord(
            Class<T> targetType,
            ContentTypeReader reader,
            T existingInstance,
            boolean hasExistingInstance) {
        Objects.requireNonNull(targetType, "targetType");
        Object value;
        try {
            value = reader.readValue(this, existingInstance);
        } catch (ContentLoadException exception) {
            throw exception;
        } catch (RuntimeException exception) {
            throw new ContentLoadException(
                    "Content type reader '" + reader.getClass().getName()
                            + "' failed while loading '" + assetName + "'", exception);
        }
        T typed;
        try {
            typed = checkedCast(targetType, value, "reader result");
        } catch (RuntimeException failure) {
            if (!hasExistingInstance && value instanceof AutoCloseable closeable) {
                try {
                    closeable.close();
                } catch (Exception closeFailure) {
                    failure.addSuppressed(closeFailure);
                }
            }
            throw failure;
        }
        if (hasExistingInstance && existingInstance != typed) {
            throw new IllegalStateException(
                    "Content type reader constructed a new value instead of populating the existing instance");
        }
        if (!hasExistingInstance && typed instanceof AutoCloseable closeable) {
            if (recordDisposableObject == null) {
                contentManager.recordDisposableObject(closeable);
            } else {
                recordDisposableObject.invoke(closeable);
            }
        }
        return typed;
    }

    private ContentTypeReader findTypeReader(Class<?> targetType) {
        for (ContentTypeReader reader : typeReaders) {
            if (reader.getTargetType() == boxClass(targetType)) {
                return reader;
            }
        }
        throw failure("reader table has no reader for '" + targetType.getName() + "'");
    }

    private static <T> T checkedCast(Class<T> targetType, Object value, String context) {
        Class<?> boxed = boxClass(targetType);
        if (value == null) {
            if (targetType.isPrimitive()) {
                throw new ContentLoadException(context + " is null, not " + targetType.getName());
            }
            return null;
        }
        if (!boxed.isInstance(value)) {
            throw new ContentLoadException(context + " is " + value.getClass().getName()
                    + ", not " + targetType.getName());
        }
        @SuppressWarnings("unchecked")
        T typed = (T)value;
        return typed;
    }

    private static Class<?> boxClass(Class<?> type) {
        if (!type.isPrimitive()) return type;
        if (type == boolean.class) return Boolean.class;
        if (type == byte.class) return Byte.class;
        if (type == char.class) return Character.class;
        if (type == short.class) return Short.class;
        if (type == int.class) return Integer.class;
        if (type == long.class) return Long.class;
        if (type == float.class) return Float.class;
        if (type == double.class) return Double.class;
        return Void.class;
    }

    private static boolean isValueType(Class<?> type) {
        return type.isPrimitive() || type == Boolean.class || type == Byte.class
                || type == Character.class || type == Short.class || type == Integer.class
                || type == Long.class || type == Float.class || type == Double.class
                || type == Color.class || type == Matrix.class || type == Quaternion.class
                || type == Rectangle.class
                || type == Vector2.class || type == Vector3.class || type == Vector4.class;
    }

    private static ContentLoadException failure(String assetName, String detail) {
        return new ContentLoadException(
                "Content asset '" + assetName + "' is not a valid XNB: " + detail);
    }
}
