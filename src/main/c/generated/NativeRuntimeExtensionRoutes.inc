/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeRuntimeExtensionRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_assemblyCopyTitleExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.assembly_copy_title_ext((char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_assemblySetTitleExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray title)
{
    (void)environment;
    (void)declaring_class;
    jsize title_size = (*environment)->GetArrayLength(environment, title);
    jbyte* title_bytes = (*environment)->GetByteArrayElements(environment, title, NULL);
    if (title_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView title_view = {(const char*)title_bytes, (uint64_t)title_size};
    CNA_Result call_result = cna.assembly_set_title_ext(title_view);
    (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_desktopOsGetCurrent(
    JNIEnv* environment, jclass declaring_class, jintArray out_os)
{
    (void)environment;
    (void)declaring_class;
    CNA_DesktopOS out_os_value = 0;
    CNA_Result call_result = cna.desktop_os_get_current(&out_os_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_os_element = (jint)out_os_value;
        (*environment)->SetIntArrayRegion(
            environment, out_os, 0, 1, &out_os_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsBackendCategoryCopyName(
    JNIEnv* environment, jclass declaring_class, jint category, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.graphics_backend_category_copy_name((CNA_GraphicsBackendCategory)category, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsBackendCategoryGetNameSize(
    JNIEnv* environment, jclass declaring_class, jint category, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.graphics_backend_category_get_name_size((CNA_GraphicsBackendCategory)category, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsBackendGetCategory(
    JNIEnv* environment, jclass declaring_class, jint type, jintArray out_category)
{
    (void)environment;
    (void)declaring_class;
    CNA_GraphicsBackendCategory out_category_value = 0;
    CNA_Result call_result = cna.graphics_backend_get_category((CNA_GraphicsRendererType)type, &out_category_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_category_element = (jint)out_category_value;
        (*environment)->SetIntArrayRegion(
            environment, out_category, 0, 1, &out_category_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsBackendGetCurrentCategory(
    JNIEnv* environment, jclass declaring_class, jintArray out_category)
{
    (void)environment;
    (void)declaring_class;
    CNA_GraphicsBackendCategory out_category_value = 0;
    CNA_Result call_result = cna.graphics_backend_get_current_category(&out_category_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_category_element = (jint)out_category_value;
        (*environment)->SetIntArrayRegion(
            environment, out_category, 0, 1, &out_category_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsBackendGetCurrentMaturity(
    JNIEnv* environment, jclass declaring_class, jintArray out_maturity)
{
    (void)environment;
    (void)declaring_class;
    CNA_GraphicsBackendMaturity out_maturity_value = 0;
    CNA_Result call_result = cna.graphics_backend_get_current_maturity(&out_maturity_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_maturity_element = (jint)out_maturity_value;
        (*environment)->SetIntArrayRegion(
            environment, out_maturity, 0, 1, &out_maturity_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsBackendGetMaturity(
    JNIEnv* environment, jclass declaring_class, jint type, jintArray out_maturity)
{
    (void)environment;
    (void)declaring_class;
    CNA_GraphicsBackendMaturity out_maturity_value = 0;
    CNA_Result call_result = cna.graphics_backend_get_maturity((CNA_GraphicsRendererType)type, &out_maturity_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_maturity_element = (jint)out_maturity_value;
        (*environment)->SetIntArrayRegion(
            environment, out_maturity, 0, 1, &out_maturity_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsBackendMaturityCopyName(
    JNIEnv* environment, jclass declaring_class, jint maturity, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.graphics_backend_maturity_copy_name((CNA_GraphicsBackendMaturity)maturity, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsBackendMaturityGetNameSize(
    JNIEnv* environment, jclass declaring_class, jint maturity, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.graphics_backend_maturity_get_name_size((CNA_GraphicsBackendMaturity)maturity, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererCopyAvailableExt(
    JNIEnv* environment, jclass declaring_class, jintArray destination, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jint* destination_elements = (*environment)->GetIntArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_GraphicsRendererType* destination_values = (CNA_GraphicsRendererType*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (CNA_GraphicsRendererType)destination_elements[index];
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.graphics_renderer_copy_available_ext(destination_values, (uint64_t)destination_size, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jint)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseIntArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererCopyCurrentName(
    JNIEnv* environment, jclass declaring_class, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.graphics_renderer_copy_current_name((char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererFallbackCopyMessageExt(
    JNIEnv* environment, jclass declaring_class, jlong index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.graphics_renderer_fallback_copy_message_ext((uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererFallbackGetMessageSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.graphics_renderer_fallback_get_message_size_ext((uint64_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererFallbackReasonCopyNameExt(
    JNIEnv* environment, jclass declaring_class, jint reason, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.graphics_renderer_fallback_reason_copy_name_ext((CNA_GraphicsRendererFallbackReason)reason, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererFallbackReasonGetNameSizeExt(
    JNIEnv* environment, jclass declaring_class, jint reason, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.graphics_renderer_fallback_reason_get_name_size_ext((CNA_GraphicsRendererFallbackReason)reason, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetActiveExt(
    JNIEnv* environment, jclass declaring_class, jintArray out_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_GraphicsRendererType out_type_value = 0;
    CNA_Result call_result = cna.graphics_renderer_get_active_ext(&out_type_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_type_element = (jint)out_type_value;
        (*environment)->SetIntArrayRegion(
            environment, out_type, 0, 1, &out_type_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetAutomaticFallbackExt(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_enabled_value = 0;
    CNA_Result call_result = cna.graphics_renderer_get_automatic_fallback_ext(&out_enabled_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_enabled_element = (jboolean)out_enabled_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_enabled, 0, 1, &out_enabled_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetAvailableCountExt(
    JNIEnv* environment, jclass declaring_class, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.graphics_renderer_get_available_count_ext(&out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetCurrentNameSize(
    JNIEnv* environment, jclass declaring_class, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.graphics_renderer_get_current_name_size(&out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetFallbackAtExt(
    JNIEnv* environment, jclass declaring_class, jlong index, jlongArray out_recordIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_recordIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GraphicsRendererFallbackRecord out_record_value;
    memset(&out_record_value, 0, sizeof out_record_value);
    out_record_value.struct_size = (uint32_t)(sizeof out_record_value);
    out_record_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.graphics_renderer_get_fallback_at_ext((uint64_t)index, &out_record_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_recordIntegral_values[] = {
            (jlong)out_record_value.type,
            (jlong)out_record_value.reason
        };
        (*environment)->SetLongArrayRegion(environment, out_recordIntegral, 0,
            (jsize)(sizeof out_recordIntegral_values / sizeof out_recordIntegral_values[0]),
            out_recordIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetFallbackCountExt(
    JNIEnv* environment, jclass declaring_class, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.graphics_renderer_get_fallback_count_ext(&out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetIsAvailableExt(
    JNIEnv* environment, jclass declaring_class, jint type, jbooleanArray out_available)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_available_value = 0;
    CNA_Result call_result = cna.graphics_renderer_get_is_available_ext((CNA_GraphicsRendererType)type, &out_available_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_available_element = (jboolean)out_available_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_available, 0, 1, &out_available_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetIsLatchedExt(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_latched)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_latched_value = 0;
    CNA_Result call_result = cna.graphics_renderer_get_is_latched_ext(&out_latched_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_latched_element = (jboolean)out_latched_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_latched, 0, 1, &out_latched_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererGetSelectedExt(
    JNIEnv* environment, jclass declaring_class, jintArray out_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_GraphicsRendererType out_type_value = 0;
    CNA_Result call_result = cna.graphics_renderer_get_selected_ext(&out_type_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_type_element = (jint)out_type_value;
        (*environment)->SetIntArrayRegion(
            environment, out_type, 0, 1, &out_type_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererResetSelectionForTestsExt(
    JNIEnv* environment, jclass declaring_class)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.graphics_renderer_reset_selection_for_tests_ext();
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererSetAutomaticFallbackExt(
    JNIEnv* environment, jclass declaring_class, jboolean enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.graphics_renderer_set_automatic_fallback_ext((CNA_Bool)enabled);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererSetFallbackChainExt(
    JNIEnv* environment, jclass declaring_class, jintArray types)
{
    (void)environment;
    (void)declaring_class;
    jsize types_size = (*environment)->GetArrayLength(environment, types);
    jint* types_elements = (*environment)->GetIntArrayElements(environment, types, NULL);
    if (types_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_GraphicsRendererType* types_values = (CNA_GraphicsRendererType*)malloc(
        ((size_t)types_size + 1U) * sizeof(*types_values));
    if (types_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, types, types_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < types_size; ++index) {
        types_values[index] = (CNA_GraphicsRendererType)types_elements[index];
    }
    CNA_Result call_result = cna.graphics_renderer_set_fallback_chain_ext(types_values, (uint64_t)types_size);
    free(types_values);
    (*environment)->ReleaseIntArrayElements(
        environment, types, types_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererSetPreferredByNameExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray name)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Result call_result = cna.graphics_renderer_set_preferred_by_name_ext(name_view);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererSetPreferredExt(
    JNIEnv* environment, jclass declaring_class, jint type)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.graphics_renderer_set_preferred_ext((CNA_GraphicsRendererType)type);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_graphicsRendererTryParseNameExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray name, jintArray out_type, jbooleanArray out_recognized)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_GraphicsRendererType out_type_value = 0;
    CNA_Bool out_recognized_value = 0;
    CNA_Result call_result = cna.graphics_renderer_try_parse_name_ext(name_view, &out_type_value, &out_recognized_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_type_element = (jint)out_type_value;
        (*environment)->SetIntArrayRegion(
            environment, out_type, 0, 1, &out_type_element);
        jboolean out_recognized_element = (jboolean)out_recognized_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_recognized, 0, 1, &out_recognized_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_loggerGetMinimumLevel(
    JNIEnv* environment, jclass declaring_class, jintArray out_level)
{
    (void)environment;
    (void)declaring_class;
    CNA_LogLevel out_level_value = 0;
    CNA_Result call_result = cna.logger_get_minimum_level(&out_level_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_level_element = (jint)out_level_value;
        (*environment)->SetIntArrayRegion(
            environment, out_level, 0, 1, &out_level_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_loggerLog(
    JNIEnv* environment, jclass declaring_class, jint level, jbyteArray message, jint category, jboolean condition)
{
    (void)environment;
    (void)declaring_class;
    jsize message_size = (*environment)->GetArrayLength(environment, message);
    jbyte* message_bytes = (*environment)->GetByteArrayElements(environment, message, NULL);
    if (message_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView message_view = {(const char*)message_bytes, (uint64_t)message_size};
    CNA_Result call_result = cna.logger_log((CNA_LogLevel)level, message_view, (CNA_LogCategory)category, (CNA_Bool)condition);
    (*environment)->ReleaseByteArrayElements(environment, message, message_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_loggerSetMinimumLevel(
    JNIEnv* environment, jclass declaring_class, jint level)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.logger_set_minimum_level((CNA_LogLevel)level);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_platformCopyCurrentNameExt(
    JNIEnv* environment, jclass declaring_class, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.platform_copy_current_name_ext((char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_platformGetCurrent(
    JNIEnv* environment, jclass declaring_class, jintArray out_platform)
{
    (void)environment;
    (void)declaring_class;
    CNA_Platform out_platform_value = 0;
    CNA_Result call_result = cna.platform_get_current(&out_platform_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_platform_element = (jint)out_platform_value;
        (*environment)->SetIntArrayRegion(
            environment, out_platform, 0, 1, &out_platform_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_platformGetCurrentNameSizeExt(
    JNIEnv* environment, jclass declaring_class, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.platform_get_current_name_size_ext(&out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_platformGetIsAppleExt(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_apple)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_apple_value = 0;
    CNA_Result call_result = cna.platform_get_is_apple_ext(&out_apple_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_apple_element = (jboolean)out_apple_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_apple, 0, 1, &out_apple_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeRuntimeExtensionRoutes_platformGetIsMobileExt(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_mobile)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_mobile_value = 0;
    CNA_Result call_result = cna.platform_get_is_mobile_ext(&out_mobile_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_mobile_element = (jboolean)out_mobile_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_mobile, 0, 1, &out_mobile_element);
    }
    return (jint)call_result;
}
