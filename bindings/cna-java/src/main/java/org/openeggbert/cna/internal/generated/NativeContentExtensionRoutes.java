package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeContentExtensionRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeContentExtensionRoutes {

    private NativeContentExtensionRoutes() {
    }

    /**
     * cna_content_manager_load_object_dictionary_ext (content_readers.h).
     */
    public static native int contentManagerLoadObjectDictionaryExt(long contentManager, byte[] assetName, long[] outDictionary);

    /**
     * cna_object_dictionary_ext_contains_key (content_readers.h).
     */
    public static native int objectDictionaryContainsKey(long dictionary, byte[] key, boolean[] outContains);

    /**
     * cna_object_dictionary_ext_copy_array (content_readers.h).
     */
    public static native int objectDictionaryCopyArray(long dictionary, byte[] key, int kind, byte[] destination, long capacity, long[] outByteCount);

    /**
     * cna_object_dictionary_ext_copy_key_at (content_readers.h).
     */
    public static native int objectDictionaryCopyKeyAt(long dictionary, long index, byte[] destination, long[] outByteCount);

    /**
     * cna_object_dictionary_ext_copy_runtime_type_name (content_readers.h).
     */
    public static native int objectDictionaryCopyRuntimeTypeName(long dictionary, byte[] destination, long[] outByteCount);

    /**
     * cna_object_dictionary_ext_copy_string (content_readers.h).
     */
    public static native int objectDictionaryCopyString(long dictionary, byte[] key, byte[] destination, long[] outByteCount);

    /**
     * cna_object_dictionary_ext_copy_type_name (content_readers.h).
     */
    public static native int objectDictionaryCopyTypeName(long dictionary, byte[] key, byte[] destination, long[] outByteCount);

    /**
     * cna_object_dictionary_ext_copy_value (content_readers.h).
     */
    public static native int objectDictionaryCopyValue(long dictionary, byte[] key, int kind, byte[] destination, long capacity);

    /**
     * cna_object_dictionary_ext_destroy (content_readers.h).
     */
    public static native int objectDictionaryDestroy(long dictionary);

    /**
     * cna_object_dictionary_ext_get_count (content_readers.h).
     */
    public static native int objectDictionaryGetCount(long dictionary, long[] outCount);

    /**
     * cna_object_dictionary_ext_get_entry (content_readers.h).
     *
     * <p>outEntryIntegral carries CNA_ObjectDictionaryEntry in this order:
     * <ol start="0">
     *   <li>{@code kind} (CNA_ObjectDictionaryValueKind)</li>
     *   <li>{@code is_array} (CNA_Bool)</li>
     *   <li>{@code element_count} (uint64_t)</li>
     * </ol>
     */
    public static native int objectDictionaryGetEntry(long dictionary, byte[] key, long[] outEntryIntegral);

    /**
     * cna_object_dictionary_ext_get_key_size_at (content_readers.h).
     */
    public static native int objectDictionaryGetKeySizeAt(long dictionary, long index, long[] outByteCount);

    /**
     * cna_object_dictionary_ext_get_runtime_type_name_size (content_readers.h).
     */
    public static native int objectDictionaryGetRuntimeTypeNameSize(long dictionary, long[] outByteCount);

    /**
     * cna_object_dictionary_ext_get_string_size (content_readers.h).
     */
    public static native int objectDictionaryGetStringSize(long dictionary, byte[] key, long[] outByteCount);

    /**
     * cna_object_dictionary_ext_get_type_name_size (content_readers.h).
     */
    public static native int objectDictionaryGetTypeNameSize(long dictionary, byte[] key, long[] outByteCount);
}
