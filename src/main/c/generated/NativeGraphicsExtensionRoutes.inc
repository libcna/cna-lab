/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeGraphicsExtensionRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_asciiPostProcessEffectCreate(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_AsciiPostProcessEffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.ascii_post_process_effect_create((CNA_Handle)graphics_device, &out_effect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_asciiPostProcessEffectDestroy(
    JNIEnv* environment, jclass declaring_class, jlong effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.ascii_post_process_effect_destroy((CNA_AsciiPostProcessEffectHandle)effect);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_asciiPostProcessEffectDraw(
    JNIEnv* environment, jclass declaring_class, jlong effect, jlong source, jlongArray destination_rectangleIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (destination_rectangleIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Rectangle destination_rectangle_value;
    memset(&destination_rectangle_value, 0, sizeof destination_rectangle_value);
    {
        jlong destination_rectangleIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, destination_rectangleIntegral, 0, 4, destination_rectangleIntegral_values);
        destination_rectangle_value.x = (int32_t)destination_rectangleIntegral_values[0];
        destination_rectangle_value.y = (int32_t)destination_rectangleIntegral_values[1];
        destination_rectangle_value.width = (int32_t)destination_rectangleIntegral_values[2];
        destination_rectangle_value.height = (int32_t)destination_rectangleIntegral_values[3];
    }
    CNA_Result call_result = cna.ascii_post_process_effect_draw((CNA_AsciiPostProcessEffectHandle)effect, (CNA_Handle)source, &destination_rectangle_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_asciiPostProcessEffectGetCellSize(
    JNIEnv* environment, jclass declaring_class, jlong effect, jintArray out_width, jintArray out_height)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_width_value = 0;
    int32_t out_height_value = 0;
    CNA_Result call_result = cna.ascii_post_process_effect_get_cell_size((CNA_AsciiPostProcessEffectHandle)effect, &out_width_value, &out_height_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_width_element = (jint)out_width_value;
        (*environment)->SetIntArrayRegion(
            environment, out_width, 0, 1, &out_width_element);
        jint out_height_element = (jint)out_height_value;
        (*environment)->SetIntArrayRegion(
            environment, out_height, 0, 1, &out_height_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_asciiPostProcessEffectGetLastGridDimensions(
    JNIEnv* environment, jclass declaring_class, jlong effect, jintArray out_columns, jintArray out_rows)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_columns_value = 0;
    int32_t out_rows_value = 0;
    CNA_Result call_result = cna.ascii_post_process_effect_get_last_grid_dimensions((CNA_AsciiPostProcessEffectHandle)effect, &out_columns_value, &out_rows_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_columns_element = (jint)out_columns_value;
        (*environment)->SetIntArrayRegion(
            environment, out_columns, 0, 1, &out_columns_element);
        jint out_rows_element = (jint)out_rows_value;
        (*environment)->SetIntArrayRegion(
            environment, out_rows, 0, 1, &out_rows_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_asciiPostProcessEffectGetQuantizeMode(
    JNIEnv* environment, jclass declaring_class, jlong effect, jintArray out_mode)
{
    (void)environment;
    (void)declaring_class;
    CNA_AsciiQuantizeMode out_mode_value = 0;
    CNA_Result call_result = cna.ascii_post_process_effect_get_quantize_mode((CNA_AsciiPostProcessEffectHandle)effect, &out_mode_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_mode_element = (jint)out_mode_value;
        (*environment)->SetIntArrayRegion(
            environment, out_mode, 0, 1, &out_mode_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_asciiPostProcessEffectSetCellSize(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint width, jint height)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.ascii_post_process_effect_set_cell_size((CNA_AsciiPostProcessEffectHandle)effect, (int32_t)width, (int32_t)height);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_asciiPostProcessEffectSetQuantizeMode(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint mode)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.ascii_post_process_effect_set_quantize_mode((CNA_AsciiPostProcessEffectHandle)effect, (CNA_AsciiQuantizeMode)mode);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectCreate(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_EffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.crt_effect_create((CNA_Handle)graphics_device, &out_effect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectGetCurvature(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.crt_effect_get_curvature((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectGetMaskIntensity(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.crt_effect_get_mask_intensity((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectGetMaskType(
    JNIEnv* environment, jclass declaring_class, jlong effect, jintArray out_mask_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_CRTMaskType out_mask_type_value = 0;
    CNA_Result call_result = cna.crt_effect_get_mask_type((CNA_EffectHandle)effect, &out_mask_type_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_mask_type_element = (jint)out_mask_type_value;
        (*environment)->SetIntArrayRegion(
            environment, out_mask_type, 0, 1, &out_mask_type_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectGetScanlineIntensity(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.crt_effect_get_scanline_intensity((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectGetVignetteIntensity(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloatArray out_value)
{
    (void)environment;
    (void)declaring_class;
    float out_value_value = 0;
    CNA_Result call_result = cna.crt_effect_get_vignette_intensity((CNA_EffectHandle)effect, &out_value_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_value_element = (jfloat)out_value_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_value, 0, 1, &out_value_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectSetCurvature(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.crt_effect_set_curvature((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectSetMaskIntensity(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.crt_effect_set_mask_intensity((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectSetMaskType(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint mask_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.crt_effect_set_mask_type((CNA_EffectHandle)effect, (CNA_CRTMaskType)mask_type);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectSetScanlineIntensity(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.crt_effect_set_scanline_intensity((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_crtEffectSetVignetteIntensity(
    JNIEnv* environment, jclass declaring_class, jlong effect, jfloat value)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.crt_effect_set_vignette_intensity((CNA_EffectHandle)effect, (float)value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawAddBoundingSphere(
    JNIEnv* environment, jclass declaring_class, jlong debug, jfloatArray sphereFloating, jlongArray colourIntegral, jint segments)
{
    (void)environment;
    (void)declaring_class;
    if (sphereFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_BoundingSphere sphere_value;
    memset(&sphere_value, 0, sizeof sphere_value);
    {
        jfloat sphereFloating_values[4];
        (*environment)->GetFloatArrayRegion(environment, sphereFloating, 0, 4, sphereFloating_values);
        sphere_value.center.x = (float)sphereFloating_values[0];
        sphere_value.center.y = (float)sphereFloating_values[1];
        sphere_value.center.z = (float)sphereFloating_values[2];
        sphere_value.radius = (float)sphereFloating_values[3];
    }
    CNA_Color colour_value;
    memset(&colour_value, 0, sizeof colour_value);
    {
        jlong colourIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, colourIntegral, 0, 4, colourIntegral_values);
        colour_value.r = (uint8_t)colourIntegral_values[0];
        colour_value.g = (uint8_t)colourIntegral_values[1];
        colour_value.b = (uint8_t)colourIntegral_values[2];
        colour_value.a = (uint8_t)colourIntegral_values[3];
    }
    CNA_Result call_result = cna.debug_draw_add_bounding_sphere((CNA_DebugDrawHandle)debug, &sphere_value, colour_value, (int32_t)segments);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawAddBox(
    JNIEnv* environment, jclass declaring_class, jlong debug, jfloatArray boundsFloating, jlongArray colourIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (boundsFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_BoundingBox bounds_value;
    memset(&bounds_value, 0, sizeof bounds_value);
    {
        jfloat boundsFloating_values[6];
        (*environment)->GetFloatArrayRegion(environment, boundsFloating, 0, 6, boundsFloating_values);
        bounds_value.min.x = (float)boundsFloating_values[0];
        bounds_value.min.y = (float)boundsFloating_values[1];
        bounds_value.min.z = (float)boundsFloating_values[2];
        bounds_value.max.x = (float)boundsFloating_values[3];
        bounds_value.max.y = (float)boundsFloating_values[4];
        bounds_value.max.z = (float)boundsFloating_values[5];
    }
    CNA_Color colour_value;
    memset(&colour_value, 0, sizeof colour_value);
    {
        jlong colourIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, colourIntegral, 0, 4, colourIntegral_values);
        colour_value.r = (uint8_t)colourIntegral_values[0];
        colour_value.g = (uint8_t)colourIntegral_values[1];
        colour_value.b = (uint8_t)colourIntegral_values[2];
        colour_value.a = (uint8_t)colourIntegral_values[3];
    }
    CNA_Result call_result = cna.debug_draw_add_box((CNA_DebugDrawHandle)debug, &bounds_value, colour_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawAddCross(
    JNIEnv* environment, jclass declaring_class, jlong debug, jfloatArray positionFloating, jfloat size, jlongArray colourIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (positionFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 position_value;
    memset(&position_value, 0, sizeof position_value);
    {
        jfloat positionFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, positionFloating, 0, 3, positionFloating_values);
        position_value.x = (float)positionFloating_values[0];
        position_value.y = (float)positionFloating_values[1];
        position_value.z = (float)positionFloating_values[2];
    }
    CNA_Color colour_value;
    memset(&colour_value, 0, sizeof colour_value);
    {
        jlong colourIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, colourIntegral, 0, 4, colourIntegral_values);
        colour_value.r = (uint8_t)colourIntegral_values[0];
        colour_value.g = (uint8_t)colourIntegral_values[1];
        colour_value.b = (uint8_t)colourIntegral_values[2];
        colour_value.a = (uint8_t)colourIntegral_values[3];
    }
    CNA_Result call_result = cna.debug_draw_add_cross((CNA_DebugDrawHandle)debug, &position_value, (float)size, colour_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawAddFrustum(
    JNIEnv* environment, jclass declaring_class, jlong debug, jfloatArray frustumFloating, jlongArray colourIntegral)
{
    (void)environment;
    (void)declaring_class;
    CNA_BoundingFrustum frustum_value;
    memset(&frustum_value, 0, sizeof frustum_value);
    {
        jfloat frustumFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, frustumFloating, 0, 16, frustumFloating_values);
        frustum_value.matrix.m11 = (float)frustumFloating_values[0];
        frustum_value.matrix.m12 = (float)frustumFloating_values[1];
        frustum_value.matrix.m13 = (float)frustumFloating_values[2];
        frustum_value.matrix.m14 = (float)frustumFloating_values[3];
        frustum_value.matrix.m21 = (float)frustumFloating_values[4];
        frustum_value.matrix.m22 = (float)frustumFloating_values[5];
        frustum_value.matrix.m23 = (float)frustumFloating_values[6];
        frustum_value.matrix.m24 = (float)frustumFloating_values[7];
        frustum_value.matrix.m31 = (float)frustumFloating_values[8];
        frustum_value.matrix.m32 = (float)frustumFloating_values[9];
        frustum_value.matrix.m33 = (float)frustumFloating_values[10];
        frustum_value.matrix.m34 = (float)frustumFloating_values[11];
        frustum_value.matrix.m41 = (float)frustumFloating_values[12];
        frustum_value.matrix.m42 = (float)frustumFloating_values[13];
        frustum_value.matrix.m43 = (float)frustumFloating_values[14];
        frustum_value.matrix.m44 = (float)frustumFloating_values[15];
    }
    CNA_Color colour_value;
    memset(&colour_value, 0, sizeof colour_value);
    {
        jlong colourIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, colourIntegral, 0, 4, colourIntegral_values);
        colour_value.r = (uint8_t)colourIntegral_values[0];
        colour_value.g = (uint8_t)colourIntegral_values[1];
        colour_value.b = (uint8_t)colourIntegral_values[2];
        colour_value.a = (uint8_t)colourIntegral_values[3];
    }
    CNA_Result call_result = cna.debug_draw_add_frustum((CNA_DebugDrawHandle)debug, frustum_value, colour_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawAddLine(
    JNIEnv* environment, jclass declaring_class, jlong debug, jfloatArray fromFloating, jfloatArray toFloating, jlongArray colourIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (fromFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (toFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 from_value;
    memset(&from_value, 0, sizeof from_value);
    {
        jfloat fromFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, fromFloating, 0, 3, fromFloating_values);
        from_value.x = (float)fromFloating_values[0];
        from_value.y = (float)fromFloating_values[1];
        from_value.z = (float)fromFloating_values[2];
    }
    CNA_Vector3 to_value;
    memset(&to_value, 0, sizeof to_value);
    {
        jfloat toFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, toFloating, 0, 3, toFloating_values);
        to_value.x = (float)toFloating_values[0];
        to_value.y = (float)toFloating_values[1];
        to_value.z = (float)toFloating_values[2];
    }
    CNA_Color colour_value;
    memset(&colour_value, 0, sizeof colour_value);
    {
        jlong colourIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, colourIntegral, 0, 4, colourIntegral_values);
        colour_value.r = (uint8_t)colourIntegral_values[0];
        colour_value.g = (uint8_t)colourIntegral_values[1];
        colour_value.b = (uint8_t)colourIntegral_values[2];
        colour_value.a = (uint8_t)colourIntegral_values[3];
    }
    CNA_Result call_result = cna.debug_draw_add_line((CNA_DebugDrawHandle)debug, &from_value, &to_value, colour_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawAddSphere(
    JNIEnv* environment, jclass declaring_class, jlong debug, jfloatArray centreFloating, jfloat radius, jlongArray colourIntegral, jint segments)
{
    (void)environment;
    (void)declaring_class;
    if (centreFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 centre_value;
    memset(&centre_value, 0, sizeof centre_value);
    {
        jfloat centreFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, centreFloating, 0, 3, centreFloating_values);
        centre_value.x = (float)centreFloating_values[0];
        centre_value.y = (float)centreFloating_values[1];
        centre_value.z = (float)centreFloating_values[2];
    }
    CNA_Color colour_value;
    memset(&colour_value, 0, sizeof colour_value);
    {
        jlong colourIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, colourIntegral, 0, 4, colourIntegral_values);
        colour_value.r = (uint8_t)colourIntegral_values[0];
        colour_value.g = (uint8_t)colourIntegral_values[1];
        colour_value.b = (uint8_t)colourIntegral_values[2];
        colour_value.a = (uint8_t)colourIntegral_values[3];
    }
    CNA_Result call_result = cna.debug_draw_add_sphere((CNA_DebugDrawHandle)debug, &centre_value, (float)radius, colour_value, (int32_t)segments);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawBegin(
    JNIEnv* environment, jclass declaring_class, jlong debug, jfloatArray viewFloating, jfloatArray projectionFloating)
{
    (void)environment;
    (void)declaring_class;
    if (viewFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    if (projectionFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Matrix view_value;
    memset(&view_value, 0, sizeof view_value);
    {
        jfloat viewFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, viewFloating, 0, 16, viewFloating_values);
        view_value.m11 = (float)viewFloating_values[0];
        view_value.m12 = (float)viewFloating_values[1];
        view_value.m13 = (float)viewFloating_values[2];
        view_value.m14 = (float)viewFloating_values[3];
        view_value.m21 = (float)viewFloating_values[4];
        view_value.m22 = (float)viewFloating_values[5];
        view_value.m23 = (float)viewFloating_values[6];
        view_value.m24 = (float)viewFloating_values[7];
        view_value.m31 = (float)viewFloating_values[8];
        view_value.m32 = (float)viewFloating_values[9];
        view_value.m33 = (float)viewFloating_values[10];
        view_value.m34 = (float)viewFloating_values[11];
        view_value.m41 = (float)viewFloating_values[12];
        view_value.m42 = (float)viewFloating_values[13];
        view_value.m43 = (float)viewFloating_values[14];
        view_value.m44 = (float)viewFloating_values[15];
    }
    CNA_Matrix projection_value;
    memset(&projection_value, 0, sizeof projection_value);
    {
        jfloat projectionFloating_values[16];
        (*environment)->GetFloatArrayRegion(environment, projectionFloating, 0, 16, projectionFloating_values);
        projection_value.m11 = (float)projectionFloating_values[0];
        projection_value.m12 = (float)projectionFloating_values[1];
        projection_value.m13 = (float)projectionFloating_values[2];
        projection_value.m14 = (float)projectionFloating_values[3];
        projection_value.m21 = (float)projectionFloating_values[4];
        projection_value.m22 = (float)projectionFloating_values[5];
        projection_value.m23 = (float)projectionFloating_values[6];
        projection_value.m24 = (float)projectionFloating_values[7];
        projection_value.m31 = (float)projectionFloating_values[8];
        projection_value.m32 = (float)projectionFloating_values[9];
        projection_value.m33 = (float)projectionFloating_values[10];
        projection_value.m34 = (float)projectionFloating_values[11];
        projection_value.m41 = (float)projectionFloating_values[12];
        projection_value.m42 = (float)projectionFloating_values[13];
        projection_value.m43 = (float)projectionFloating_values[14];
        projection_value.m44 = (float)projectionFloating_values[15];
    }
    CNA_Result call_result = cna.debug_draw_begin((CNA_DebugDrawHandle)debug, &view_value, &projection_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawClear(
    JNIEnv* environment, jclass declaring_class, jlong debug)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.debug_draw_clear((CNA_DebugDrawHandle)debug);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawCopyVertices(
    JNIEnv* environment, jclass declaring_class, jlong debug, jboolean depth_tested, jlongArray destinationIntegral, jfloatArray destinationFloating, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationIntegral) / 4;
    if ((*environment)->GetArrayLength(environment, destinationIntegral) != destination_count * 4 ||
        (*environment)->GetArrayLength(environment, destinationFloating) != destination_count * 3) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_VertexPositionColor* destination_values = (CNA_VertexPositionColor*)calloc(
        (size_t)destination_count + 1U, sizeof(*destination_values));
    if (destination_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize destinationIntegral_length = (*environment)->GetArrayLength(environment, destinationIntegral);
        jlong* destinationIntegral_values = (jlong*)malloc(
            ((size_t)destinationIntegral_length + 1U) * sizeof(*destinationIntegral_values));
        if (destinationIntegral_values == NULL) {
            free(destination_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetLongArrayRegion(environment, destinationIntegral, 0, destinationIntegral_length, destinationIntegral_values);
        for (jsize element = 0; element < destination_count; ++element) {
            destination_values[element].color.r = (uint8_t)destinationIntegral_values[element * 4 + 0];
            destination_values[element].color.g = (uint8_t)destinationIntegral_values[element * 4 + 1];
            destination_values[element].color.b = (uint8_t)destinationIntegral_values[element * 4 + 2];
            destination_values[element].color.a = (uint8_t)destinationIntegral_values[element * 4 + 3];
        }
        free(destinationIntegral_values);
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
            destination_values[element].position.x = (float)destinationFloating_values[element * 3 + 0];
            destination_values[element].position.y = (float)destinationFloating_values[element * 3 + 1];
            destination_values[element].position.z = (float)destinationFloating_values[element * 3 + 2];
        }
        free(destinationFloating_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.debug_draw_copy_vertices((CNA_DebugDrawHandle)debug, (CNA_Bool)depth_tested, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jlong* destinationIntegral_out = (jlong*)malloc(
                ((size_t)destination_count * 4 + 1U) * sizeof(*destinationIntegral_out));
            if (destinationIntegral_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationIntegral_out[element * 4 + 0] = (jlong)destination_values[element].color.r;
                    destinationIntegral_out[element * 4 + 1] = (jlong)destination_values[element].color.g;
                    destinationIntegral_out[element * 4 + 2] = (jlong)destination_values[element].color.b;
                    destinationIntegral_out[element * 4 + 3] = (jlong)destination_values[element].color.a;
                }
                (*environment)->SetLongArrayRegion(
                    environment, destinationIntegral, 0,
                    (jsize)(destination_count * 4), destinationIntegral_out);
                free(destinationIntegral_out);
            }
        }
        {
            jfloat* destinationFloating_out = (jfloat*)malloc(
                ((size_t)destination_count * 3 + 1U) * sizeof(*destinationFloating_out));
            if (destinationFloating_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationFloating_out[element * 3 + 0] = (jfloat)destination_values[element].position.x;
                    destinationFloating_out[element * 3 + 1] = (jfloat)destination_values[element].position.y;
                    destinationFloating_out[element * 3 + 2] = (jfloat)destination_values[element].position.z;
                }
                (*environment)->SetFloatArrayRegion(
                    environment, destinationFloating, 0,
                    (jsize)(destination_count * 3), destinationFloating_out);
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
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawCreate(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_debug)
{
    (void)environment;
    (void)declaring_class;
    CNA_DebugDrawHandle out_debug_value = 0;
    CNA_Result call_result = cna.debug_draw_create((CNA_Handle)graphics_device, &out_debug_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_debug_element = (jlong)out_debug_value;
        (*environment)->SetLongArrayRegion(
            environment, out_debug, 0, 1, &out_debug_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawDestroy(
    JNIEnv* environment, jclass declaring_class, jlong debug)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.debug_draw_destroy((CNA_DebugDrawHandle)debug);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawEnd(
    JNIEnv* environment, jclass declaring_class, jlong debug)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.debug_draw_end((CNA_DebugDrawHandle)debug);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawGetLineCount(
    JNIEnv* environment, jclass declaring_class, jlong debug, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.debug_draw_get_line_count((CNA_DebugDrawHandle)debug, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawIsDepthTested(
    JNIEnv* environment, jclass declaring_class, jlong debug, jbooleanArray out_depth_tested)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_depth_tested_value = 0;
    CNA_Result call_result = cna.debug_draw_is_depth_tested((CNA_DebugDrawHandle)debug, &out_depth_tested_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_depth_tested_element = (jboolean)out_depth_tested_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_depth_tested, 0, 1, &out_depth_tested_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_debugDrawSetDepthTested(
    JNIEnv* environment, jclass declaring_class, jlong debug, jboolean depth_tested)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.debug_draw_set_depth_tested((CNA_DebugDrawHandle)debug, (CNA_Bool)depth_tested);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_depthEffectCreate(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_effect)
{
    (void)environment;
    (void)declaring_class;
    CNA_EffectHandle out_effect_value = 0;
    CNA_Result call_result = cna.depth_effect_create((CNA_Handle)graphics_device, &out_effect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_effect_element = (jlong)out_effect_value;
        (*environment)->SetLongArrayRegion(
            environment, out_effect, 0, 1, &out_effect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_depthEffectGetDitherMode(
    JNIEnv* environment, jclass declaring_class, jlong effect, jintArray out_dither_mode)
{
    (void)environment;
    (void)declaring_class;
    CNA_DitherMode out_dither_mode_value = 0;
    CNA_Result call_result = cna.depth_effect_get_dither_mode((CNA_EffectHandle)effect, &out_dither_mode_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_dither_mode_element = (jint)out_dither_mode_value;
        (*environment)->SetIntArrayRegion(
            environment, out_dither_mode, 0, 1, &out_dither_mode_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_depthEffectGetMode(
    JNIEnv* environment, jclass declaring_class, jlong effect, jintArray out_mode)
{
    (void)environment;
    (void)declaring_class;
    CNA_DepthEffectMode out_mode_value = 0;
    CNA_Result call_result = cna.depth_effect_get_mode((CNA_EffectHandle)effect, &out_mode_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_mode_element = (jint)out_mode_value;
        (*environment)->SetIntArrayRegion(
            environment, out_mode, 0, 1, &out_mode_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_depthEffectSetDitherMode(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint dither_mode)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.depth_effect_set_dither_mode((CNA_EffectHandle)effect, (CNA_DitherMode)dither_mode);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_depthEffectSetMode(
    JNIEnv* environment, jclass declaring_class, jlong effect, jint mode)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.depth_effect_set_mode((CNA_EffectHandle)effect, (CNA_DepthEffectMode)mode);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_graphicsDeviceCopyCapabilityReportExt(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.graphics_device_copy_capability_report_ext((CNA_Handle)graphics_device, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_graphicsDeviceCopyRendererName(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.graphics_device_copy_renderer_name((CNA_Handle)graphics_device, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_graphicsDeviceGetCapabilityReportSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.graphics_device_get_capability_report_size_ext((CNA_Handle)graphics_device, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_graphicsDeviceGetRendererNameSize(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.graphics_device_get_renderer_name_size((CNA_Handle)graphics_device, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_graphicsDeviceGetShaderDialectExt(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jintArray out_dialect)
{
    (void)environment;
    (void)declaring_class;
    CNA_ShaderDialect out_dialect_value = 0;
    CNA_Result call_result = cna.graphics_device_get_shader_dialect_ext((CNA_Handle)graphics_device, &out_dialect_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_dialect_element = (jint)out_dialect_value;
        (*environment)->SetIntArrayRegion(
            environment, out_dialect, 0, 1, &out_dialect_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_graphicsDeviceSupportsCapability(
    JNIEnv* environment, jclass declaring_class, jlong graphics_device, jint capability, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.graphics_device_supports_capability((CNA_Handle)graphics_device, (CNA_GraphicsCapability)capability, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_graphicsExtIsAvailable(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_available)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_available_value = 0;
    CNA_Result call_result = cna.graphics_ext_is_available(&out_available_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_available_element = (jboolean)out_available_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_available, 0, 1, &out_available_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_imageBasedLightExtInit(
    JNIEnv* environment, jclass declaring_class, jlongArray out_lightIntegral, jfloatArray out_lightFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_lightIntegral == NULL || out_lightFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_ImageBasedLightEXT out_light_value;
    memset(&out_light_value, 0, sizeof out_light_value);
    out_light_value.struct_size = (uint32_t)(sizeof out_light_value);
    out_light_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.image_based_light_ext_init(&out_light_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_lightIntegral_values[] = {
            (jlong)out_light_value.irradiance,
            (jlong)out_light_value.prefiltered_specular,
            (jlong)out_light_value.brdf_lut,
            (jlong)out_light_value.prefiltered_mip_count
        };
        (*environment)->SetLongArrayRegion(environment, out_lightIntegral, 0,
            (jsize)(sizeof out_lightIntegral_values / sizeof out_lightIntegral_values[0]),
            out_lightIntegral_values);
        jfloat out_lightFloating_values[] = {
            (jfloat)out_light_value.intensity
        };
        (*environment)->SetFloatArrayRegion(environment, out_lightFloating, 0,
            (jsize)(sizeof out_lightFloating_values / sizeof out_lightFloating_values[0]),
            out_lightFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_imageBasedLightExtIsValid(
    JNIEnv* environment, jclass declaring_class, jlongArray lightIntegral, jfloatArray lightFloating, jbooleanArray out_valid)
{
    (void)environment;
    (void)declaring_class;
    if (lightIntegral == NULL || lightFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_ImageBasedLightEXT light_value;
    memset(&light_value, 0, sizeof light_value);
    light_value.struct_size = (uint32_t)(sizeof light_value);
    light_value.struct_version = UINT32_C(1);
    {
        jlong lightIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, lightIntegral, 0, 4, lightIntegral_values);
        light_value.irradiance = (CNA_Handle)lightIntegral_values[0];
        light_value.prefiltered_specular = (CNA_Handle)lightIntegral_values[1];
        light_value.brdf_lut = (CNA_Handle)lightIntegral_values[2];
        light_value.prefiltered_mip_count = (int32_t)lightIntegral_values[3];
    }
    {
        jfloat lightFloating_values[1];
        (*environment)->GetFloatArrayRegion(environment, lightFloating, 0, 1, lightFloating_values);
        light_value.intensity = (float)lightFloating_values[0];
    }
    CNA_Bool out_valid_value = 0;
    CNA_Result call_result = cna.image_based_light_ext_is_valid(&light_value, &out_valid_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_valid_element = (jboolean)out_valid_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_valid, 0, 1, &out_valid_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_indirectDrawArgumentsInit(
    JNIEnv* environment, jclass declaring_class, jlongArray out_argumentsIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_argumentsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_IndirectDrawArguments out_arguments_value;
    memset(&out_arguments_value, 0, sizeof out_arguments_value);
    CNA_Result call_result = cna.indirect_draw_arguments_init(&out_arguments_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_argumentsIntegral_values[] = {
            (jlong)out_arguments_value.vertex_count,
            (jlong)out_arguments_value.instance_count,
            (jlong)out_arguments_value.first_vertex,
            (jlong)out_arguments_value.base_instance
        };
        (*environment)->SetLongArrayRegion(environment, out_argumentsIntegral, 0,
            (jsize)(sizeof out_argumentsIntegral_values / sizeof out_argumentsIntegral_values[0]),
            out_argumentsIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeGraphicsExtensionRoutes_indirectDrawIndexedArgumentsInit(
    JNIEnv* environment, jclass declaring_class, jlongArray out_argumentsIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_argumentsIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_IndirectDrawIndexedArguments out_arguments_value;
    memset(&out_arguments_value, 0, sizeof out_arguments_value);
    CNA_Result call_result = cna.indirect_draw_indexed_arguments_init(&out_arguments_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_argumentsIntegral_values[] = {
            (jlong)out_arguments_value.index_count,
            (jlong)out_arguments_value.instance_count,
            (jlong)out_arguments_value.first_index,
            (jlong)out_arguments_value.base_vertex,
            (jlong)out_arguments_value.base_instance
        };
        (*environment)->SetLongArrayRegion(environment, out_argumentsIntegral, 0,
            (jsize)(sizeof out_argumentsIntegral_values / sizeof out_argumentsIntegral_values[0]),
            out_argumentsIntegral_values);
    }
    return (jint)call_result;
}
