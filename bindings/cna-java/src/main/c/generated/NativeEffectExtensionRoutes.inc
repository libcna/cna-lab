/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeEffectExtensionRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeEffectExtensionRoutes_colorMatrixEffectCreate(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_EffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.color_matrix_effect_create((CNA_Handle)graphics_device, &out_effect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeEffectExtensionRoutes_colorMatrixEffectGetMatrix(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_ColorMatrix4x4 out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.color_matrix_effect_get_matrix((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.values[0],
            (jfloat)out_value_value.values[1],
            (jfloat)out_value_value.values[2],
            (jfloat)out_value_value.values[3],
            (jfloat)out_value_value.values[4],
            (jfloat)out_value_value.values[5],
            (jfloat)out_value_value.values[6],
            (jfloat)out_value_value.values[7],
            (jfloat)out_value_value.values[8],
            (jfloat)out_value_value.values[9],
            (jfloat)out_value_value.values[10],
            (jfloat)out_value_value.values[11],
            (jfloat)out_value_value.values[12],
            (jfloat)out_value_value.values[13],
            (jfloat)out_value_value.values[14],
            (jfloat)out_value_value.values[15]
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeEffectExtensionRoutes_colorMatrixEffectGetOffset(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_valueFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_valueFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector4 out_value_value;
    memset(&out_value_value, 0, sizeof out_value_value);
    CNA_Result call_result = cna.color_matrix_effect_get_offset((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_valueFloating_values[] = {
            (jfloat)out_value_value.x,
            (jfloat)out_value_value.y,
            (jfloat)out_value_value.z,
            (jfloat)out_value_value.w
        };
        (*environment)->SetFloatArrayRegion(environment, out_valueFloating, 0,
            (jsize)(sizeof out_valueFloating_values / sizeof out_valueFloating_values[0]),
            out_valueFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeEffectExtensionRoutes_colorMatrixEffectReset(
    JNIEnv* environment, jclass declaring_class, jlong effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.color_matrix_effect_reset((CNA_EffectHandle)effect);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeEffectExtensionRoutes_colorMatrixEffectSetGrayscale(
    JNIEnv* environment, jclass declaring_class, jlong effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.color_matrix_effect_set_grayscale((CNA_EffectHandle)effect);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeEffectExtensionRoutes_colorMatrixEffectSetMatrix(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
    CNA_ColorMatrix4x4 value_value;
    memset(&value_value, 0, sizeof value_value);
    {
        jfloat valueFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, valueFloating, 0, 16, valueFloating_values);
        value_value.values[0] = (float)valueFloating_values[0];
        value_value.values[1] = (float)valueFloating_values[1];
        value_value.values[2] = (float)valueFloating_values[2];
        value_value.values[3] = (float)valueFloating_values[3];
        value_value.values[4] = (float)valueFloating_values[4];
        value_value.values[5] = (float)valueFloating_values[5];
        value_value.values[6] = (float)valueFloating_values[6];
        value_value.values[7] = (float)valueFloating_values[7];
        value_value.values[8] = (float)valueFloating_values[8];
        value_value.values[9] = (float)valueFloating_values[9];
        value_value.values[10] = (float)valueFloating_values[10];
        value_value.values[11] = (float)valueFloating_values[11];
        value_value.values[12] = (float)valueFloating_values[12];
        value_value.values[13] = (float)valueFloating_values[13];
        value_value.values[14] = (float)valueFloating_values[14];
        value_value.values[15] = (float)valueFloating_values[15];
    }
    CNA_Result call_result = cna.color_matrix_effect_set_matrix((CNA_EffectHandle)effect, value_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeEffectExtensionRoutes_colorMatrixEffectSetOffset(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray valueFloating)
{
    (void)environment;
    (void)declaring_class;
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
    CNA_Result call_result = cna.color_matrix_effect_set_offset((CNA_EffectHandle)effect, value_value);
    return (jint)call_result;
}
