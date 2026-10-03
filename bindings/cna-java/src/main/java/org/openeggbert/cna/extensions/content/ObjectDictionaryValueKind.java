package org.openeggbert.cna.extensions.content;

/**
 * Which value one {@link CnaObjectDictionary} entry holds.
 *
 * <p>The entry keeps whatever type its own content type reader produced -- a {@code Vector3[]}
 * stays a {@code Vector3[]}, a {@code BoundingSphere} a {@code BoundingSphere} -- which is what
 * makes the dictionary worth having rather than a bag of bytes. The ordinals are CNA's own
 * {@code CNA_OBJECT_DICTIONARY_VALUE_*} values.
 */
public enum ObjectDictionaryValueKind {

    /**
     * A type this ABI does not express.
     *
     * <p>Not "unreadable": {@link CnaObjectDictionary#getEntryTypeName(String)} still reports the
     * implementation's own name for it, so a game can say what it found.
     */
    Unknown(0),

    /** A {@code boolean}. */
    Boolean(1),

    /** A signed 32-bit integer. */
    Int32(4),

    /** A 32-bit float. */
    Single(4),

    /** A 64-bit float. */
    Double(8),

    /** A {@code String}; read it with {@link CnaObjectDictionary#getString(String)}. */
    String(0),

    /** A {@code Vector2}. */
    Vector2(8),

    /** A {@code Vector3}. */
    Vector3(12),

    /** A {@code Vector4}. */
    Vector4(16),

    /** A {@code Matrix}. */
    Matrix(64),

    /** A {@code Quaternion}. */
    Quaternion(16),

    /** A {@code Color}, four bytes in RGBA order. */
    Color(4),

    /** A {@code BoundingSphere}: a centre and a radius. */
    BoundingSphere(16),

    /** A {@code BoundingBox}: two corners. */
    BoundingBox(24),

    /**
     * An object a caller's own reflective reader produced.
     *
     * <p>No entry in a dictionary this projection loads can hold one, because CNA's reflective
     * reader builder is not projected -- see the package documentation for why. The identity is
     * named so an entry that somehow carries one is reported rather than mistaken for something
     * else.
     */
    ForeignObject(0);

    /** How many bytes one element occupies in CNA's packed form; zero when it has none. */
    private final int byteCount;

    ObjectDictionaryValueKind(int byteCount) {
        this.byteCount = byteCount;
    }

    /** The packed size of one element, for the fixed-layout kinds. */
    int byteCount() {
        return byteCount;
    }

    static ObjectDictionaryValueKind of(int value) {
        ObjectDictionaryValueKind[] all = values();
        if (value < 0 || value >= all.length) {
            throw new IllegalStateException("CNA reported dictionary value kind " + value
                    + ", which this ABI does not name");
        }
        return all[value];
    }
}
