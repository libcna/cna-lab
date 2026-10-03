/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeContentExtensionRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_contentManagerLoadObjectDictionaryExt(
    JNIEnv* environment, jclass declaring_class, jlong content_manager, jbyteArray asset_name, jlongArray out_dictionary)
{
    (void)environment;
    (void)declaring_class;
    jsize asset_name_size = (*environment)->GetArrayLength(environment, asset_name);
    jbyte* asset_name_bytes = (*environment)->GetByteArrayElements(environment, asset_name, NULL);
    if (asset_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView asset_name_view = {(const char*)asset_name_bytes, (uint64_t)asset_name_size};
    CNA_ObjectDictionaryHandle out_dictionary_value = 0;
    CNA_Result call_result = cna.content_manager_load_object_dictionary_ext((CNA_Handle)content_manager, asset_name_view, &out_dictionary_value);
    (*environment)->ReleaseByteArrayElements(environment, asset_name, asset_name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_dictionary_element = (jlong)out_dictionary_value;
        (*environment)->SetLongArrayRegion(
            environment, out_dictionary, 0, 1, &out_dictionary_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryContainsKey(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbooleanArray out_contains)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_Bool out_contains_value = 0;
    CNA_Result call_result = cna.object_dictionary_ext_contains_key((CNA_ObjectDictionaryHandle)dictionary, key_view, &out_contains_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_contains_element = (jboolean)out_contains_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_contains, 0, 1, &out_contains_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryCopyArray(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jint kind, jbyteArray destination, jlong capacity, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    jsize destination_length = (*environment)->GetArrayLength(environment, destination);
    jlong destination_required = 1;
    int destination_refused = 0;
    if ((jlong)capacity < 0) {
        destination_refused = 1;
    } else if ((jlong)capacity != 0 && destination_required > (jlong)destination_length / (jlong)capacity) {
        destination_refused = 1;
    } else {
        destination_required *= (jlong)capacity;
    }
    if (destination_refused || destination_required > (jlong)destination_length) {
        (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.object_dictionary_ext_copy_array((CNA_ObjectDictionaryHandle)dictionary, key_view, (CNA_ObjectDictionaryValueKind)kind, (void*)destination_bytes, (uint64_t)capacity, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryCopyKeyAt(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jlong index, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.object_dictionary_ext_copy_key_at((CNA_ObjectDictionaryHandle)dictionary, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryCopyRuntimeTypeName(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.object_dictionary_ext_copy_runtime_type_name((CNA_ObjectDictionaryHandle)dictionary, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryCopyString(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.object_dictionary_ext_copy_string((CNA_ObjectDictionaryHandle)dictionary, key_view, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryCopyTypeName(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.object_dictionary_ext_copy_type_name((CNA_ObjectDictionaryHandle)dictionary, key_view, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryCopyValue(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jint kind, jbyteArray destination, jlong capacity)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    jsize destination_length = (*environment)->GetArrayLength(environment, destination);
    jlong destination_required = 1;
    int destination_refused = 0;
    if ((jlong)capacity < 0) {
        destination_refused = 1;
    } else if ((jlong)capacity != 0 && destination_required > (jlong)destination_length / (jlong)capacity) {
        destination_refused = 1;
    } else {
        destination_required *= (jlong)capacity;
    }
    if (destination_refused || destination_required > (jlong)destination_length) {
        (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_Result call_result = cna.object_dictionary_ext_copy_value((CNA_ObjectDictionaryHandle)dictionary, key_view, (CNA_ObjectDictionaryValueKind)kind, (void*)destination_bytes, (uint64_t)capacity);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(
        environment, destination, destination_bytes, 0);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryDestroy(
    JNIEnv* environment, jclass declaring_class, jlong dictionary)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.object_dictionary_ext_destroy((CNA_ObjectDictionaryHandle)dictionary);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryGetCount(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.object_dictionary_ext_get_count((CNA_ObjectDictionaryHandle)dictionary, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryGetEntry(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlongArray out_entryIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_entryIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    CNA_ObjectDictionaryEntry out_entry_value;
    memset(&out_entry_value, 0, sizeof out_entry_value);
    out_entry_value.struct_size = (uint32_t)(sizeof out_entry_value);
    out_entry_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.object_dictionary_ext_get_entry((CNA_ObjectDictionaryHandle)dictionary, key_view, &out_entry_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_entryIntegral_values[] = {
            (jlong)out_entry_value.kind,
            (jlong)out_entry_value.is_array,
            (jlong)out_entry_value.element_count
        };
        (*environment)->SetLongArrayRegion(environment, out_entryIntegral, 0,
            (jsize)(sizeof out_entryIntegral_values / sizeof out_entryIntegral_values[0]),
            out_entryIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryGetKeySizeAt(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jlong index, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.object_dictionary_ext_get_key_size_at((CNA_ObjectDictionaryHandle)dictionary, (uint64_t)index, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryGetRuntimeTypeNameSize(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.object_dictionary_ext_get_runtime_type_name_size((CNA_ObjectDictionaryHandle)dictionary, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryGetStringSize(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.object_dictionary_ext_get_string_size((CNA_ObjectDictionaryHandle)dictionary, key_view, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeContentExtensionRoutes_objectDictionaryGetTypeNameSize(
    JNIEnv* environment, jclass declaring_class, jlong dictionary, jbyteArray key, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize key_size = (*environment)->GetArrayLength(environment, key);
    jbyte* key_bytes = (*environment)->GetByteArrayElements(environment, key, NULL);
    if (key_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView key_view = {(const char*)key_bytes, (uint64_t)key_size};
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.object_dictionary_ext_get_type_name_size((CNA_ObjectDictionaryHandle)dictionary, key_view, &out_byte_count_value);
    (*environment)->ReleaseByteArrayElements(environment, key, key_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}
