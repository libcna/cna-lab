package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.BoundingBox;
import Microsoft.Xna.Framework.BoundingSphere;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Content.ContentManager;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeContentExtensionRoutes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * The {@code Dictionary<string, object>} a custom {@code ContentProcessor} attaches to an asset.
 *
 * <p>This is how an XNA game is handed data the stock content pipeline has no type for. XNA's own
 * {@code TrianglePickingSample} tags every model with its world-space triangle vertices and a
 * bounding sphere exactly this way, and the game casts the tag back to read them.
 *
 * <p><strong>Each entry keeps the type its own reader produced.</strong> A {@code Vector3[]} stays
 * a {@code Vector3[]} and a {@code BoundingSphere} stays a {@code BoundingSphere}, which is what
 * makes the dictionary worth having rather than a bag of bytes. Ask
 * {@link #getEntryKind(String)} what an entry holds before reading it, exactly as the canonical
 * {@code Get<T>} makes a caller name the type.
 *
 * <p><strong>Naming the wrong type is refused, not reinterpreted.</strong> Every read here names
 * the kind it expects and CNA answers {@code INVALID_ARGUMENT} for a mismatch -- which is the C
 * form of the {@code InvalidCastException} the canonical {@code Get<T>} throws.
 *
 * <p>The handle is <strong>owned</strong>; {@link #close()} releases it. The asset behind it is
 * cached by the content manager exactly as every other load is, so a second load of the same name
 * does not re-read the file -- but it does produce a second independent dictionary over a second
 * copy, and each one must be closed.
 *
 * <p><strong>This is CNA's reader, not the managed one.</strong> A Java {@code ContentManager}
 * reads most {@code .xnb} assets in managed code, and its reader table has no
 * {@code DictionaryReader}; this route reaches CNA's, through the same manager and therefore the
 * same root directory and the same cache.
 */
public final class CnaObjectDictionary implements AutoCloseable {

    /** CNA's own {@code CNA_RESULT_INVALID_ARGUMENT}, which a wrong kind or an absent key gives. */
    private static final int RESULT_INVALID_ARGUMENT = 1;

    private final long handle;
    private boolean closed;

    private CnaObjectDictionary(long handle) {
        this.handle = handle;
    }

    /** Takes ownership of a dictionary handle another CNA route returned. */
    static CnaObjectDictionary adopt(long handle) {
        return new CnaObjectDictionary(handle);
    }

    /**
     * Loads an asset whose root object is a {@code Dictionary<string, object>}.
     *
     * @param contentManager the manager whose root directory and cache to use
     * @param assetName the asset name, without extension
     * @return the dictionary, which the caller closes
     * @throws ContentNotSupportedException when this build has no content extension layer
     */
    public static CnaObjectDictionary Load(ContentManager contentManager, String assetName) {
        Objects.requireNonNull(contentManager, "contentManager");
        Objects.requireNonNull(assetName, "assetName");
        long[] loaded = new long[1];
        CnbExtension.check("CnaObjectDictionary.Load",
                NativeContentExtensionRoutes.contentManagerLoadObjectDictionaryExt(
                        NativeBindings.nativeContentManagerHandle(contentManager),
                        utf8(assetName), loaded));
        return new CnaObjectDictionary(loaded[0]);
    }

    /**
     * Returns how many entries the dictionary holds.
     *
     * @return the entry count
     */
    public int size() {
        long[] count = new long[1];
        CnbExtension.check("CnaObjectDictionary.size",
                NativeContentExtensionRoutes.objectDictionaryGetCount(open(), count));
        return Math.toIntExact(count[0]);
    }

    /**
     * Returns the dictionary's own managed runtime type name.
     *
     * <p>Always {@code System.Collections.Generic.Dictionary`2[System.String,System.Object]}, and
     * read from the asset rather than assumed -- an asset whose root is something else does not
     * load at all.
     *
     * @return the type name
     */
    public String getRuntimeTypeName() {
        long[] bytes = new long[1];
        CnbExtension.check("CnaObjectDictionary.getRuntimeTypeName",
                NativeContentExtensionRoutes.objectDictionaryGetRuntimeTypeNameSize(open(), bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        CnbExtension.check("CnaObjectDictionary.getRuntimeTypeName",
                NativeContentExtensionRoutes.objectDictionaryCopyRuntimeTypeName(
                        open(), destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /**
     * Returns the keys, in the dictionary's own stable order.
     *
     * <p>The canonical container is an ordered map, so the order is the keys' own and is the same
     * on every call -- which is what makes reading by index meaningful rather than a snapshot.
     *
     * @return the keys
     */
    public List<String> keys() {
        int count = size();
        List<String> names = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            long[] bytes = new long[1];
            CnbExtension.check("CnaObjectDictionary.keys", NativeContentExtensionRoutes
                    .objectDictionaryGetKeySizeAt(open(), index, bytes));
            byte[] destination = new byte[Math.toIntExact(bytes[0])];
            CnbExtension.check("CnaObjectDictionary.keys", NativeContentExtensionRoutes
                    .objectDictionaryCopyKeyAt(open(), index, destination, bytes));
            names.add(new String(destination, 0, Math.toIntExact(bytes[0]),
                    StandardCharsets.UTF_8));
        }
        return Collections.unmodifiableList(names);
    }

    /**
     * Reports whether an entry with this key exists.
     *
     * @param key the entry's name, as the processor wrote it
     * @return whether it is present
     */
    public boolean containsKey(String key) {
        boolean[] present = new boolean[1];
        CnbExtension.check("CnaObjectDictionary.containsKey", NativeContentExtensionRoutes
                .objectDictionaryContainsKey(open(), utf8(key), present));
        return present[0];
    }

    /**
     * Returns what one entry holds, and how many of them.
     *
     * @param key the entry's name
     * @return the entry's description
     * @throws IllegalArgumentException when no entry has that key
     */
    public Entry getEntry(String key) {
        long[] values = new long[3];
        int result = NativeContentExtensionRoutes.objectDictionaryGetEntry(
                open(), utf8(key), values);
        if (result == RESULT_INVALID_ARGUMENT) {
            throw new IllegalArgumentException("no dictionary entry named " + key);
        }
        CnbExtension.check("CnaObjectDictionary.getEntry", result);
        return new Entry(ObjectDictionaryValueKind.of(Math.toIntExact(values[0])),
                values[1] != 0L, Math.toIntExact(values[2]));
    }

    /**
     * Returns what one entry holds, without its shape.
     *
     * @param key the entry's name
     * @return the kind
     */
    public ObjectDictionaryValueKind getEntryKind(String key) {
        return getEntry(key).kind();
    }

    /**
     * Returns the implementation's own name for an entry's stored type.
     *
     * <p>A <strong>diagnostic, not an identity</strong>: the spelling is the C++ toolchain's and
     * is not part of CNA's compatibility promise. Print it; do not branch on it. It is what makes
     * an {@link ObjectDictionaryValueKind#Unknown} entry reportable rather than merely unreadable.
     *
     * @param key the entry's name
     * @return the type name
     */
    public String getEntryTypeName(String key) {
        long[] bytes = new long[1];
        CnbExtension.check("CnaObjectDictionary.getEntryTypeName", NativeContentExtensionRoutes
                .objectDictionaryGetTypeNameSize(open(), utf8(key), bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        CnbExtension.check("CnaObjectDictionary.getEntryTypeName", NativeContentExtensionRoutes
                .objectDictionaryCopyTypeName(open(), utf8(key), destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /**
     * Reads a string entry.
     *
     * @param key the entry's name
     * @return the text
     * @throws IllegalArgumentException when the entry is not a string
     */
    public String getString(String key) {
        long[] bytes = new long[1];
        int probe = NativeContentExtensionRoutes.objectDictionaryGetStringSize(
                open(), utf8(key), bytes);
        if (probe == RESULT_INVALID_ARGUMENT) {
            throw new IllegalArgumentException(
                    "dictionary entry " + key + " is not a string");
        }
        CnbExtension.check("CnaObjectDictionary.getString", probe);
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        CnbExtension.check("CnaObjectDictionary.getString", NativeContentExtensionRoutes
                .objectDictionaryCopyString(open(), utf8(key), destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    /** Reads a {@code boolean} entry. */
    public boolean getBoolean(String key) {
        return scalar(key, ObjectDictionaryValueKind.Boolean).get() != 0;
    }

    /** Reads a signed 32-bit integer entry. */
    public int getInt32(String key) {
        return scalar(key, ObjectDictionaryValueKind.Int32).getInt();
    }

    /** Reads a 32-bit float entry. */
    public float getSingle(String key) {
        return scalar(key, ObjectDictionaryValueKind.Single).getFloat();
    }

    /** Reads a 64-bit float entry. */
    public double getDouble(String key) {
        return scalar(key, ObjectDictionaryValueKind.Double).getDouble();
    }

    /** Reads a {@code Vector2} entry. */
    public Vector2 getVector2(String key) {
        ByteBuffer values = scalar(key, ObjectDictionaryValueKind.Vector2);
        return new Vector2(values.getFloat(), values.getFloat());
    }

    /** Reads a {@code Vector3} entry. */
    public Vector3 getVector3(String key) {
        ByteBuffer values = scalar(key, ObjectDictionaryValueKind.Vector3);
        return new Vector3(values.getFloat(), values.getFloat(), values.getFloat());
    }

    /** Reads a {@code Vector4} entry. */
    public Vector4 getVector4(String key) {
        ByteBuffer values = scalar(key, ObjectDictionaryValueKind.Vector4);
        return new Vector4(values.getFloat(), values.getFloat(), values.getFloat(),
                values.getFloat());
    }

    /** Reads a {@code Quaternion} entry. */
    public Quaternion getQuaternion(String key) {
        ByteBuffer values = scalar(key, ObjectDictionaryValueKind.Quaternion);
        return new Quaternion(values.getFloat(), values.getFloat(), values.getFloat(),
                values.getFloat());
    }

    /** Reads a {@code Matrix} entry, in CNA's row-major order. */
    public Matrix getMatrix(String key) {
        ByteBuffer values = scalar(key, ObjectDictionaryValueKind.Matrix);
        float[] elements = new float[16];
        for (int index = 0; index < elements.length; index++) {
            elements[index] = values.getFloat();
        }
        return new Matrix(elements[0], elements[1], elements[2], elements[3],
                elements[4], elements[5], elements[6], elements[7],
                elements[8], elements[9], elements[10], elements[11],
                elements[12], elements[13], elements[14], elements[15]);
    }

    /** Reads a {@code Color} entry. */
    public Color getColor(String key) {
        ByteBuffer values = scalar(key, ObjectDictionaryValueKind.Color);
        return new Color(values.get() & 0xFF, values.get() & 0xFF, values.get() & 0xFF,
                values.get() & 0xFF);
    }

    /** Reads a {@code BoundingSphere} entry. */
    public BoundingSphere getBoundingSphere(String key) {
        ByteBuffer values = scalar(key, ObjectDictionaryValueKind.BoundingSphere);
        return new BoundingSphere(
                new Vector3(values.getFloat(), values.getFloat(), values.getFloat()),
                values.getFloat());
    }

    /** Reads a {@code BoundingBox} entry. */
    public BoundingBox getBoundingBox(String key) {
        ByteBuffer values = scalar(key, ObjectDictionaryValueKind.BoundingBox);
        return new BoundingBox(
                new Vector3(values.getFloat(), values.getFloat(), values.getFloat()),
                new Vector3(values.getFloat(), values.getFloat(), values.getFloat()));
    }

    /**
     * Reads an array entry of {@code Vector3}, the shape a triangle list is stored in.
     *
     * @param key the entry's name
     * @return the elements, in the order the processor wrote them
     * @throws IllegalArgumentException when the entry is not an array of {@code Vector3}
     */
    public List<Vector3> getVector3Array(String key) {
        ByteBuffer values = array(key, ObjectDictionaryValueKind.Vector3);
        List<Vector3> elements = new ArrayList<>(values.remaining() / 12);
        while (values.remaining() >= 12) {
            elements.add(new Vector3(values.getFloat(), values.getFloat(), values.getFloat()));
        }
        return Collections.unmodifiableList(elements);
    }

    /**
     * Reads an array entry of {@code float}.
     *
     * @param key the entry's name
     * @return the elements
     */
    public float[] getSingleArray(String key) {
        ByteBuffer values = array(key, ObjectDictionaryValueKind.Single);
        float[] elements = new float[values.remaining() / 4];
        for (int index = 0; index < elements.length; index++) {
            elements[index] = values.getFloat();
        }
        return elements;
    }

    /**
     * Reads an array entry of signed 32-bit integers.
     *
     * @param key the entry's name
     * @return the elements
     */
    public int[] getInt32Array(String key) {
        ByteBuffer values = array(key, ObjectDictionaryValueKind.Int32);
        int[] elements = new int[values.remaining() / 4];
        for (int index = 0; index < elements.length; index++) {
            elements[index] = values.getInt();
        }
        return elements;
    }

    /** Releases the dictionary. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnaObjectDictionary.close",
                NativeContentExtensionRoutes.objectDictionaryDestroy(handle));
    }

    private ByteBuffer scalar(String key, ObjectDictionaryValueKind kind) {
        byte[] destination = new byte[kind.byteCount()];
        int result = NativeContentExtensionRoutes.objectDictionaryCopyValue(
                open(), utf8(key), kind.ordinal(), destination, destination.length);
        if (result == RESULT_INVALID_ARGUMENT) {
            // CNA folds the canonical KeyNotFoundException and InvalidCastException into one
            // result code, and its message is what separates them -- so the message is carried
            // rather than replaced with a guess about which one it was.
            throw new IllegalArgumentException("dictionary entry " + key + " is absent or is not "
                    + kind + ": " + NativeBindings.failure(
                            "cna_object_dictionary_ext_copy_value", result).getMessage());
        }
        CnbExtension.check("CnaObjectDictionary.get" + kind, result);
        return ByteBuffer.wrap(destination).order(ByteOrder.LITTLE_ENDIAN);
    }

    private ByteBuffer array(String key, ObjectDictionaryValueKind kind) {
        long[] bytes = new long[1];
        // Zero capacity first: CNA always reports the byte count it needs and writes nothing, so
        // the read is sized once and done once rather than guessed at.
        int probe = NativeContentExtensionRoutes.objectDictionaryCopyArray(
                open(), utf8(key), kind.ordinal(), new byte[0], 0L, bytes);
        if (probe == RESULT_INVALID_ARGUMENT) {
            throw new IllegalArgumentException(
                    "dictionary entry " + key + " is absent or is not an array of " + kind);
        }
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("CnaObjectDictionary.get" + kind + "Array", probe);
        }
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        CnbExtension.check("CnaObjectDictionary.get" + kind + "Array",
                NativeContentExtensionRoutes.objectDictionaryCopyArray(
                        open(), utf8(key), kind.ordinal(), destination, destination.length,
                        bytes));
        return ByteBuffer.wrap(destination, 0, Math.toIntExact(bytes[0]))
                .order(ByteOrder.LITTLE_ENDIAN);
    }

    private static byte[] utf8(String value) {
        return Objects.requireNonNull(value, "key").getBytes(StandardCharsets.UTF_8);
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this CnaObjectDictionary is closed");
        }
        return handle;
    }

    /**
     * What one entry holds, and how many of them.
     *
     * @param kind the value's kind, or the element's kind when {@code isArray} is true
     * @param isArray whether the entry holds an array rather than one value
     * @param elementCount the array length, or one for a scalar
     */
    public record Entry(ObjectDictionaryValueKind kind, boolean isArray, int elementCount) {
    }
}
