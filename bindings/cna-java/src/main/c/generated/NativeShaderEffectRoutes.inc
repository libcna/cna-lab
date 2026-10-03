/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeShaderEffectRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_effectCopyFragmentSource(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.effect_copy_fragment_source((CNA_EffectHandle)effect, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_effectCopyVertexSource(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray destination, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.effect_copy_vertex_source((CNA_EffectHandle)effect, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_effectGetFragmentSourceByteCount(
    JNIEnv* environment, jclass declaring_class, jlong effect, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.effect_get_fragment_source_byte_count((CNA_EffectHandle)effect, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_effectGetIsCompiledExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbooleanArray out_is_compiled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_compiled_value = 0;
    CNA_Result call_result = cna.effect_get_is_compiled_ext((CNA_EffectHandle)effect, &out_is_compiled_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_compiled_element = (jboolean)out_is_compiled_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_compiled, 0, 1, &out_is_compiled_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_effectGetVertexSourceByteCount(
    JNIEnv* environment, jclass declaring_class, jlong effect, jlongArray out_byte_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_byte_count_value = 0;
    CNA_Result call_result = cna.effect_get_vertex_source_byte_count((CNA_EffectHandle)effect, &out_byte_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_byte_count_element = (jlong)out_byte_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_byte_count, 0, 1, &out_byte_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_effectHasRenderer(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbooleanArray out_has_renderer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_renderer_value = 0;
    CNA_Result call_result = cna.effect_has_renderer((CNA_EffectHandle)effect, &out_has_renderer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_renderer_element = (jboolean)out_has_renderer_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_renderer, 0, 1, &out_has_renderer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectCopyCompileErrorExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.shader_effect_copy_compile_error_ext((CNA_EffectHandle)effect, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectCreate(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jbyteArray vertex_source, jbyteArray fragment_source, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    jsize vertex_source_size = (*environment)->GetArrayLength(environment, vertex_source);
    jbyte* vertex_source_bytes = (*environment)->GetByteArrayElements(environment, vertex_source, NULL);
    if (vertex_source_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView vertex_source_view = {(const char*)vertex_source_bytes, (uint64_t)vertex_source_size};
    jsize fragment_source_size = (*environment)->GetArrayLength(environment, fragment_source);
    jbyte* fragment_source_bytes = (*environment)->GetByteArrayElements(environment, fragment_source, NULL);
    if (fragment_source_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, vertex_source, vertex_source_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView fragment_source_view = {(const char*)fragment_source_bytes, (uint64_t)fragment_source_size};
    CNA_EffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.shader_effect_create((CNA_Handle)graphics_device, vertex_source_view, fragment_source_view, &out_effect_value);
    (*environment)->ReleaseByteArrayElements(environment, vertex_source, vertex_source_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, fragment_source, fragment_source_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectDeclareUniformBlockExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint block_size_bytes, jobjectArray names, jintArray offsets)
{
    (void)environment;
    (void)declaring_class;
    if ((*environment)->GetArrayLength(environment, names) !=
            (*environment)->GetArrayLength(environment, offsets)) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CnaJniStringViews names_views;
    const CNA_Result names_borrowed = cna_jni_borrow_string_views(
        environment, names, &names_views);
    if (names_borrowed != CNA_RESULT_SUCCESS) {
        return (jint)names_borrowed;
    }
    jsize offsets_size = (*environment)->GetArrayLength(environment, offsets);
    jint* offsets_elements = (*environment)->GetIntArrayElements(environment, offsets, NULL);
    if (offsets_elements == NULL) {
    cna_jni_free_string_views(&names_views);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    int32_t* offsets_values = (int32_t*)malloc(
        ((size_t)offsets_size + 1U) * sizeof(*offsets_values));
    if (offsets_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, offsets, offsets_elements, JNI_ABORT);
    cna_jni_free_string_views(&names_views);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < offsets_size; ++index) {
        offsets_values[index] = (int32_t)offsets_elements[index];
    }
    CNA_Result call_result = cna.shader_effect_declare_uniform_block_ext((CNA_EffectHandle)effect, (int32_t)block_size_bytes, (const CNA_StringView*)names_views.views, offsets_values, (uint64_t)(*environment)->GetArrayLength(environment, names));
    cna_jni_free_string_views(&names_views);
    free(offsets_values);
    (*environment)->ReleaseIntArrayElements(
        environment, offsets, offsets_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectGetProjection(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.shader_effect_get_projection((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.m11,
            (jfloat)out_value_value.m12,
            (jfloat)out_value_value.m13,
            (jfloat)out_value_value.m14,
            (jfloat)out_value_value.m21,
            (jfloat)out_value_value.m22,
            (jfloat)out_value_value.m23,
            (jfloat)out_value_value.m24,
            (jfloat)out_value_value.m31,
            (jfloat)out_value_value.m32,
            (jfloat)out_value_value.m33,
            (jfloat)out_value_value.m34,
            (jfloat)out_value_value.m41,
            (jfloat)out_value_value.m42,
            (jfloat)out_value_value.m43,
            (jfloat)out_value_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectGetView(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.shader_effect_get_view((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.m11,
            (jfloat)out_value_value.m12,
            (jfloat)out_value_value.m13,
            (jfloat)out_value_value.m14,
            (jfloat)out_value_value.m21,
            (jfloat)out_value_value.m22,
            (jfloat)out_value_value.m23,
            (jfloat)out_value_value.m24,
            (jfloat)out_value_value.m31,
            (jfloat)out_value_value.m32,
            (jfloat)out_value_value.m33,
            (jfloat)out_value_value.m34,
            (jfloat)out_value_value.m41,
            (jfloat)out_value_value.m42,
            (jfloat)out_value_value.m43,
            (jfloat)out_value_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectGetWorld(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.shader_effect_get_world((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.m11,
            (jfloat)out_value_value.m12,
            (jfloat)out_value_value.m13,
            (jfloat)out_value_value.m14,
            (jfloat)out_value_value.m21,
            (jfloat)out_value_value.m22,
            (jfloat)out_value_value.m23,
            (jfloat)out_value_value.m24,
            (jfloat)out_value_value.m31,
            (jfloat)out_value_value.m32,
            (jfloat)out_value_value.m33,
            (jfloat)out_value_value.m34,
            (jfloat)out_value_value.m41,
            (jfloat)out_value_value.m42,
            (jfloat)out_value_value.m43,
            (jfloat)out_value_value.m44
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectHasRenderer(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbooleanArray out_has_renderer)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_renderer_value = 0;
    CNA_Result call_result = cna.shader_effect_has_renderer((CNA_EffectHandle)effect, &out_has_renderer_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_renderer_element = (jboolean)out_has_renderer_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_renderer, 0, 1, &out_has_renderer_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectIsValid(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbooleanArray out_is_valid)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_is_valid_value = 0;
    CNA_Result call_result = cna.shader_effect_is_valid((CNA_EffectHandle)effect, &out_is_valid_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_is_valid_element = (jboolean)out_is_valid_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_is_valid, 0, 1, &out_is_valid_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetProjection(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_Matrix value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 16, valueFloating_values);
        value_value.m11 = (float)valueFloating_values[0];
        value_value.m12 = (float)valueFloating_values[1];
        value_value.m13 = (float)valueFloating_values[2];
        value_value.m14 = (float)valueFloating_values[3];
        value_value.m21 = (float)valueFloating_values[4];
        value_value.m22 = (float)valueFloating_values[5];
        value_value.m23 = (float)valueFloating_values[6];
        value_value.m24 = (float)valueFloating_values[7];
        value_value.m31 = (float)valueFloating_values[8];
        value_value.m32 = (float)valueFloating_values[9];
        value_value.m33 = (float)valueFloating_values[10];
        value_value.m34 = (float)valueFloating_values[11];
        value_value.m41 = (float)valueFloating_values[12];
        value_value.m42 = (float)valueFloating_values[13];
        value_value.m43 = (float)valueFloating_values[14];
        value_value.m44 = (float)valueFloating_values[15];
    }
    CNA_Result call_result = cna.shader_effect_set_projection((CNA_EffectHandle)effect, value_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetTexture2d(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint unit, jlong texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.shader_effect_set_texture2d((CNA_EffectHandle)effect, (int32_t)unit, (CNA_Handle)texture);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetTexture3d(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint unit, jlong texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.shader_effect_set_texture3d((CNA_EffectHandle)effect, (int32_t)unit, (CNA_Handle)texture);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetTextureCube(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint unit, jlong texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.shader_effect_set_texture_cube((CNA_EffectHandle)effect, (int32_t)unit, (CNA_Handle)texture);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformFloat(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Result call_result = cna.shader_effect_set_uniform_float((CNA_EffectHandle)effect, name_view, (float)value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformFloatArray(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloatArray values)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize values_size = (*environment)->GetArrayLength(environment, values);
    jfloat* values_elements = (*environment)->GetFloatArrayElements(environment, values, NULL);
    if (values_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* values_values = (float*)malloc(
        ((size_t)values_size + 1U) * sizeof(*values_values));
    if (values_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, values, values_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < values_size; ++index) {
        values_values[index] = (float)values_elements[index];
    }
    CNA_Result call_result = cna.shader_effect_set_uniform_float_array((CNA_EffectHandle)effect, name_view, values_values, (uint64_t)values_size);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    free(values_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, values, values_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformInt32(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jint value)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Result call_result = cna.shader_effect_set_uniform_int32((CNA_EffectHandle)effect, name_view, (int32_t)value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformMat4Array(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloatArray matrices)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize matrices_size = (*environment)->GetArrayLength(environment, matrices);
    jfloat* matrices_elements = (*environment)->GetFloatArrayElements(environment, matrices, NULL);
    if (matrices_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* matrices_values = (float*)malloc(
        ((size_t)matrices_size + 1U) * sizeof(*matrices_values));
    if (matrices_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, matrices, matrices_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < matrices_size; ++index) {
        matrices_values[index] = (float)matrices_elements[index];
    }
    if (matrices_size % 16 != 0) {
        free(matrices_values);
        (*environment)->ReleaseFloatArrayElements(
            environment, matrices, matrices_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Result call_result = cna.shader_effect_set_uniform_mat4_array((CNA_EffectHandle)effect, name_view, matrices_values, (int32_t)(matrices_size / 16));
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    free(matrices_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, matrices, matrices_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformMatrix(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Matrix value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 16, valueFloating_values);
        value_value.m11 = (float)valueFloating_values[0];
        value_value.m12 = (float)valueFloating_values[1];
        value_value.m13 = (float)valueFloating_values[2];
        value_value.m14 = (float)valueFloating_values[3];
        value_value.m21 = (float)valueFloating_values[4];
        value_value.m22 = (float)valueFloating_values[5];
        value_value.m23 = (float)valueFloating_values[6];
        value_value.m24 = (float)valueFloating_values[7];
        value_value.m31 = (float)valueFloating_values[8];
        value_value.m32 = (float)valueFloating_values[9];
        value_value.m33 = (float)valueFloating_values[10];
        value_value.m34 = (float)valueFloating_values[11];
        value_value.m41 = (float)valueFloating_values[12];
        value_value.m42 = (float)valueFloating_values[13];
        value_value.m43 = (float)valueFloating_values[14];
        value_value.m44 = (float)valueFloating_values[15];
    }
    CNA_Result call_result = cna.shader_effect_set_uniform_matrix((CNA_EffectHandle)effect, name_view, value_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformVec3Array(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloatArray values)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize values_size = (*environment)->GetArrayLength(environment, values);
    jfloat* values_elements = (*environment)->GetFloatArrayElements(environment, values, NULL);
    if (values_elements == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    float* values_values = (float*)malloc(
        ((size_t)values_size + 1U) * sizeof(*values_values));
    if (values_values == NULL) {
        (*environment)->ReleaseFloatArrayElements(
            environment, values, values_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < values_size; ++index) {
        values_values[index] = (float)values_elements[index];
    }
    if (values_size % 3 != 0) {
        free(values_values);
        (*environment)->ReleaseFloatArrayElements(
            environment, values, values_elements, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Result call_result = cna.shader_effect_set_uniform_vec3_array((CNA_EffectHandle)effect, name_view, values_values, (int32_t)(values_size / 3));
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    free(values_values);
    (*environment)->ReleaseFloatArrayElements(
        environment, values, values_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformVector2(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Vector2 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[2];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 2, valueFloating_values);
        value_value.x = (float)valueFloating_values[0];
        value_value.y = (float)valueFloating_values[1];
    }
    CNA_Result call_result = cna.shader_effect_set_uniform_vector2((CNA_EffectHandle)effect, name_view, value_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformVector2Array(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloatArray valuesFloating)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    jsize values_count = (*environment)->GetArrayLength(environment, valuesFloating) / 2;
    if ((*environment)->GetArrayLength(environment, valuesFloating) != values_count * 2) {
        (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector2* values_values = (CNA_Vector2*)calloc(
        (size_t)values_count + 1U, sizeof(*values_values));
    if (values_values == NULL) {
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize valuesFloating_length = (*environment)->GetArrayLength(environment, valuesFloating);
        jfloat* valuesFloating_values = (jfloat*)malloc(
            ((size_t)valuesFloating_length + 1U) * sizeof(*valuesFloating_values));
        if (valuesFloating_values == NULL) {
            free(values_values);
            (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, valuesFloating, 0, valuesFloating_length, valuesFloating_values);
        for (jsize element = 0; element < values_count; ++element) {
            values_values[element].x = (float)valuesFloating_values[element * 2 + 0];
            values_values[element].y = (float)valuesFloating_values[element * 2 + 1];
        }
        free(valuesFloating_values);
    }
    CNA_Result call_result = cna.shader_effect_set_uniform_vector2_array((CNA_EffectHandle)effect, name_view, values_values, (uint64_t)values_count);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    free(values_values);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformVector3(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Vector3 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 3, valueFloating_values);
        value_value.x = (float)valueFloating_values[0];
        value_value.y = (float)valueFloating_values[1];
        value_value.z = (float)valueFloating_values[2];
    }
    CNA_Result call_result = cna.shader_effect_set_uniform_vector3((CNA_EffectHandle)effect, name_view, value_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetUniformVector4(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbyteArray name, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Vector4 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[4];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 4, valueFloating_values);
        value_value.x = (float)valueFloating_values[0];
        value_value.y = (float)valueFloating_values[1];
        value_value.z = (float)valueFloating_values[2];
        value_value.w = (float)valueFloating_values[3];
    }
    CNA_Result call_result = cna.shader_effect_set_uniform_vector4((CNA_EffectHandle)effect, name_view, value_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetView(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_Matrix value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 16, valueFloating_values);
        value_value.m11 = (float)valueFloating_values[0];
        value_value.m12 = (float)valueFloating_values[1];
        value_value.m13 = (float)valueFloating_values[2];
        value_value.m14 = (float)valueFloating_values[3];
        value_value.m21 = (float)valueFloating_values[4];
        value_value.m22 = (float)valueFloating_values[5];
        value_value.m23 = (float)valueFloating_values[6];
        value_value.m24 = (float)valueFloating_values[7];
        value_value.m31 = (float)valueFloating_values[8];
        value_value.m32 = (float)valueFloating_values[9];
        value_value.m33 = (float)valueFloating_values[10];
        value_value.m34 = (float)valueFloating_values[11];
        value_value.m41 = (float)valueFloating_values[12];
        value_value.m42 = (float)valueFloating_values[13];
        value_value.m43 = (float)valueFloating_values[14];
        value_value.m44 = (float)valueFloating_values[15];
    }
    CNA_Result call_result = cna.shader_effect_set_view((CNA_EffectHandle)effect, value_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeShaderEffectRoutes_shaderEffectSetWorld(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_Matrix value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 16, valueFloating_values);
        value_value.m11 = (float)valueFloating_values[0];
        value_value.m12 = (float)valueFloating_values[1];
        value_value.m13 = (float)valueFloating_values[2];
        value_value.m14 = (float)valueFloating_values[3];
        value_value.m21 = (float)valueFloating_values[4];
        value_value.m22 = (float)valueFloating_values[5];
        value_value.m23 = (float)valueFloating_values[6];
        value_value.m24 = (float)valueFloating_values[7];
        value_value.m31 = (float)valueFloating_values[8];
        value_value.m32 = (float)valueFloating_values[9];
        value_value.m33 = (float)valueFloating_values[10];
        value_value.m34 = (float)valueFloating_values[11];
        value_value.m41 = (float)valueFloating_values[12];
        value_value.m42 = (float)valueFloating_values[13];
        value_value.m43 = (float)valueFloating_values[14];
        value_value.m44 = (float)valueFloating_values[15];
    }
    CNA_Result call_result = cna.shader_effect_set_world((CNA_EffectHandle)effect, value_value);
    return (jint)call_result;
}
