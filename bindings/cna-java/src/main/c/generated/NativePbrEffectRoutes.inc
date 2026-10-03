/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativePbrEffectRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectCreate(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_EffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.pbr_effect_create((CNA_Handle)graphics_device, &out_effect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetAlpha(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_alpha((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetAlphaCutoffExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_alpha_cutoff_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetAlphaModeExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_AlphaModeEXT out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_alpha_mode_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetDiffuseColor(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.pbr_effect_get_diffuse_color((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.x,
            (jfloat)out_value_value.y,
            (jfloat)out_value_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetDoubleSidedExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_double_sided_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetEmissiveFactor(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.pbr_effect_get_emissive_factor((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.x,
            (jfloat)out_value_value.y,
            (jfloat)out_value_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetEncodeOutputToSrgbExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_encode_output_to_srgb_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetIorExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_ior_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetMetallicFactor(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_metallic_factor((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetNormalScaleExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_normal_scale_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetOcclusionStrengthExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_occlusion_strength_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetRoughnessFactor(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_roughness_factor((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetSpecularColorFactorExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.pbr_effect_get_specular_color_factor_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.x,
            (jfloat)out_value_value.y,
            (jfloat)out_value_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetSpecularFactorExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_specular_factor_ext((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetTexture(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint slot, jbooleanArray out_has_texture, jlongArray out_texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_texture_value = 0;
    CNA_Handle out_texture_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_texture((CNA_EffectHandle)effect, (CNA_PbrTextureSlot)slot, &out_has_texture_value, &out_texture_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_texture_element = (jboolean)out_has_texture_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_texture, 0, 1, &out_has_texture_element);
        jlong out_texture_element = (jlong)out_texture_value;
        (*environment)->SetLongArrayRegion(
            environment, out_texture, 0, 1, &out_texture_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetTextureCoordinateSetExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint slot, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_texture_coordinate_set_ext((CNA_EffectHandle)effect, (CNA_PbrTextureSlot)slot, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetTextureIsSrgbExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint slot, jbooleanArray out_value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_value_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_texture_is_srgb_ext((CNA_EffectHandle)effect, (CNA_PbrTextureSlot)slot, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_value_element = (jboolean)out_value_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetTextureTransformExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint slot, jfloatArray out_transformFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_transformFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_TextureTransformEXT out_transform_value;
    memset(&out_transform_value, 0, sizeof out_transform_value);
    out_transform_value.struct_size = (uint32_t)(sizeof out_transform_value);
    out_transform_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.pbr_effect_get_texture_transform_ext((CNA_EffectHandle)effect, (CNA_PbrTextureSlot)slot, &out_transform_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_transformFloating_values[] = {
            (jfloat)out_transform_value.offset.x,
            (jfloat)out_transform_value.offset.y,
            (jfloat)out_transform_value.scale.x,
            (jfloat)out_transform_value.scale.y,
            (jfloat)out_transform_value.rotation
        };
        (*environment)->SetFloatArrayRegion(environment, out_transformFloating, 0,
            (jsize)(sizeof out_transformFloating_values / sizeof out_transformFloating_values[0]),
            out_transformFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectGetVertexColorEnabledExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jbooleanArray out_enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_enabled_value = 0;
    CNA_Result call_result = cna.pbr_effect_get_vertex_color_enabled_ext((CNA_EffectHandle)effect, &out_enabled_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_enabled_element = (jboolean)out_enabled_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_enabled, 0, 1, &out_enabled_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetAlpha(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_alpha((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetAlphaCutoffExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_alpha_cutoff_ext((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetAlphaModeExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_alpha_mode_ext((CNA_EffectHandle)effect, (CNA_AlphaModeEXT)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetDiffuseColor(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_Vector3 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 3, valueFloating_values);
        value_value.x = (float)valueFloating_values[0];
        value_value.y = (float)valueFloating_values[1];
        value_value.z = (float)valueFloating_values[2];
    }
    CNA_Result call_result = cna.pbr_effect_set_diffuse_color((CNA_EffectHandle)effect, value_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetDoubleSidedExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jboolean value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_double_sided_ext((CNA_EffectHandle)effect, (CNA_Bool)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetEmissiveFactor(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_Vector3 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 3, valueFloating_values);
        value_value.x = (float)valueFloating_values[0];
        value_value.y = (float)valueFloating_values[1];
        value_value.z = (float)valueFloating_values[2];
    }
    CNA_Result call_result = cna.pbr_effect_set_emissive_factor((CNA_EffectHandle)effect, value_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetEncodeOutputToSrgbExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jboolean value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_encode_output_to_srgb_ext((CNA_EffectHandle)effect, (CNA_Bool)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetIorExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_ior_ext((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetMetallicFactor(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_metallic_factor((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetNormalScaleExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_normal_scale_ext((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetOcclusionStrengthExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_occlusion_strength_ext((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetRoughnessFactor(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_roughness_factor((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetSpecularColorFactorExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_Vector3 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 3, valueFloating_values);
        value_value.x = (float)valueFloating_values[0];
        value_value.y = (float)valueFloating_values[1];
        value_value.z = (float)valueFloating_values[2];
    }
    CNA_Result call_result = cna.pbr_effect_set_specular_color_factor_ext((CNA_EffectHandle)effect, value_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetSpecularFactorExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_specular_factor_ext((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetTexture(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint slot, jlong texture)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_texture((CNA_EffectHandle)effect, (CNA_PbrTextureSlot)slot, (CNA_Handle)texture);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetTextureCoordinateSetExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint slot, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_texture_coordinate_set_ext((CNA_EffectHandle)effect, (CNA_PbrTextureSlot)slot, (int32_t)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetTextureIsSrgbExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint slot, jboolean value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_texture_is_srgb_ext((CNA_EffectHandle)effect, (CNA_PbrTextureSlot)slot, (CNA_Bool)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetTextureTransformExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint slot, jfloatArray transformFloating)
{
    (void)environment;
    (void)declaring_class;
    if (transformFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_TextureTransformEXT transform_value;
    memset(&transform_value, 0, sizeof transform_value);
    transform_value.struct_size = (uint32_t)(sizeof transform_value);
    transform_value.struct_version = UINT32_C(1);
    {
        jfloat transformFloating_values[5];
        (*environment)->GetFloatArrayRegion(environment, transformFloating, 0, 5, transformFloating_values);
        transform_value.offset.x = (float)transformFloating_values[0];
        transform_value.offset.y = (float)transformFloating_values[1];
        transform_value.scale.x = (float)transformFloating_values[2];
        transform_value.scale.y = (float)transformFloating_values[3];
        transform_value.rotation = (float)transformFloating_values[4];
    }
    CNA_Result call_result = cna.pbr_effect_set_texture_transform_ext((CNA_EffectHandle)effect, (CNA_PbrTextureSlot)slot, &transform_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_pbrEffectSetVertexColorEnabledExt(
    JNIEnv* environment, jclass declaring_class, jlong effect, jboolean enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.pbr_effect_set_vertex_color_enabled_ext((CNA_EffectHandle)effect, (CNA_Bool)enabled);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_skinnedPbrEffectCopyBoneTransforms(
    JNIEnv* environment, jclass declaring_class, jlong effect, jlong requested_count, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationFloating) / 16;
    if ((*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* destination_values = (CNA_Matrix*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationFloating_length = (*environment)->GetArrayLength(environment, destinationFloating);
        jfloat* destinationFloating_values = (jfloat*)malloc(
            ((size_t)destinationFloating_length + 1U) * sizeof(*destinationFloating_values));
        if (destinationFloating_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, destinationFloating, 0, destinationFloating_length, destinationFloating_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].m11 = (float)destinationFloating_values[element * 16 + 0];
            destination_values[element].m12 = (float)destinationFloating_values[element * 16 + 1];
            destination_values[element].m13 = (float)destinationFloating_values[element * 16 + 2];
            destination_values[element].m14 = (float)destinationFloating_values[element * 16 + 3];
            destination_values[element].m21 = (float)destinationFloating_values[element * 16 + 4];
            destination_values[element].m22 = (float)destinationFloating_values[element * 16 + 5];
            destination_values[element].m23 = (float)destinationFloating_values[element * 16 + 6];
            destination_values[element].m24 = (float)destinationFloating_values[element * 16 + 7];
            destination_values[element].m31 = (float)destinationFloating_values[element * 16 + 8];
            destination_values[element].m32 = (float)destinationFloating_values[element * 16 + 9];
            destination_values[element].m33 = (float)destinationFloating_values[element * 16 + 10];
            destination_values[element].m34 = (float)destinationFloating_values[element * 16 + 11];
            destination_values[element].m41 = (float)destinationFloating_values[element * 16 + 12];
            destination_values[element].m42 = (float)destinationFloating_values[element * 16 + 13];
            destination_values[element].m43 = (float)destinationFloating_values[element * 16 + 14];
            destination_values[element].m44 = (float)destinationFloating_values[element * 16 + 15];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.skinned_pbr_effect_copy_bone_transforms((CNA_EffectHandle)effect, (uint64_t)requested_count, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 16 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 16 + 0] = (jfloat)destination_values[element].m11;
                    destinationFloating_out[element * 16 + 1] = (jfloat)destination_values[element].m12;
                    destinationFloating_out[element * 16 + 2] = (jfloat)destination_values[element].m13;
                    destinationFloating_out[element * 16 + 3] = (jfloat)destination_values[element].m14;
                    destinationFloating_out[element * 16 + 4] = (jfloat)destination_values[element].m21;
                    destinationFloating_out[element * 16 + 5] = (jfloat)destination_values[element].m22;
                    destinationFloating_out[element * 16 + 6] = (jfloat)destination_values[element].m23;
                    destinationFloating_out[element * 16 + 7] = (jfloat)destination_values[element].m24;
                    destinationFloating_out[element * 16 + 8] = (jfloat)destination_values[element].m31;
                    destinationFloating_out[element * 16 + 9] = (jfloat)destination_values[element].m32;
                    destinationFloating_out[element * 16 + 10] = (jfloat)destination_values[element].m33;
                    destinationFloating_out[element * 16 + 11] = (jfloat)destination_values[element].m34;
                    destinationFloating_out[element * 16 + 12] = (jfloat)destination_values[element].m41;
                    destinationFloating_out[element * 16 + 13] = (jfloat)destination_values[element].m42;
                    destinationFloating_out[element * 16 + 14] = (jfloat)destination_values[element].m43;
                    destinationFloating_out[element * 16 + 15] = (jfloat)destination_values[element].m44;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 16), destinationFloating_out);
                free(destinationFloating_out);
            }
        }
    }
    free(destination_values);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_skinnedPbrEffectCreate(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_EffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.skinned_pbr_effect_create((CNA_Handle)graphics_device, &out_effect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_skinnedPbrEffectGetWeightsPerVertex(
    JNIEnv* environment, jclass declaring_class, jlong effect, jintArray out_value)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_value_value = 0;
    CNA_Result call_result = cna.skinned_pbr_effect_get_weights_per_vertex((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_value_element = (jint)out_value_value;
        (*environment)->SetIntArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_skinnedPbrEffectSetBoneTransforms(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray transformsFloating)
{
    (void)environment;
    (void)declaring_class;
    jsize transforms_count = (*environment)->GetArrayLength(environment, transformsFloating) / 16;
    if ((*environment)->GetArrayLength(environment, transformsFloating) != transforms_count * 16) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix* transforms_values = (CNA_Matrix*)calloc(
        (size_t)transforms_count + 1U, sizeof(*transforms_values));
    if (transforms_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize transformsFloating_length = (*environment)->GetArrayLength(environment, transformsFloating);
        jfloat* transformsFloating_values = (jfloat*)malloc(
            ((size_t)transformsFloating_length + 1U) * sizeof(*transformsFloating_values));
        if (transformsFloating_values == NULL) {
            free(transforms_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetFloatArrayRegion(environment, transformsFloating, 0, transformsFloating_length, transformsFloating_values);
        for (jsize element = 0; element < transforms_count; ++element) {
            transforms_values[element].m11 = (float)transformsFloating_values[element * 16 + 0];
            transforms_values[element].m12 = (float)transformsFloating_values[element * 16 + 1];
            transforms_values[element].m13 = (float)transformsFloating_values[element * 16 + 2];
            transforms_values[element].m14 = (float)transformsFloating_values[element * 16 + 3];
            transforms_values[element].m21 = (float)transformsFloating_values[element * 16 + 4];
            transforms_values[element].m22 = (float)transformsFloating_values[element * 16 + 5];
            transforms_values[element].m23 = (float)transformsFloating_values[element * 16 + 6];
            transforms_values[element].m24 = (float)transformsFloating_values[element * 16 + 7];
            transforms_values[element].m31 = (float)transformsFloating_values[element * 16 + 8];
            transforms_values[element].m32 = (float)transformsFloating_values[element * 16 + 9];
            transforms_values[element].m33 = (float)transformsFloating_values[element * 16 + 10];
            transforms_values[element].m34 = (float)transformsFloating_values[element * 16 + 11];
            transforms_values[element].m41 = (float)transformsFloating_values[element * 16 + 12];
            transforms_values[element].m42 = (float)transformsFloating_values[element * 16 + 13];
            transforms_values[element].m43 = (float)transformsFloating_values[element * 16 + 14];
            transforms_values[element].m44 = (float)transformsFloating_values[element * 16 + 15];
        }
        free(transformsFloating_values);
    }
    CNA_Result call_result = cna.skinned_pbr_effect_set_bone_transforms((CNA_EffectHandle)effect, transforms_values, (uint64_t)transforms_count);
    free(transforms_values);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativePbrEffectRoutes_skinnedPbrEffectSetWeightsPerVertex(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.skinned_pbr_effect_set_weights_per_vertex((CNA_EffectHandle)effect, (int32_t)value);
    return (jint)call_result;
}
