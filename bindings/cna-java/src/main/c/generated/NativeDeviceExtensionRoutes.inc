/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeDeviceExtensionRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraCopyNameAtExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.camera_copy_name_at_ext((CNA_Handle)game, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraCreate(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_camera)
{
    (void)environment;
    (void)declaring_class;
    CNA_CameraHandle out_camera_value = 0;
    CNA_Result call_result = cna.camera_create((CNA_Handle)game, &out_camera_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_camera_element = (jlong)out_camera_value;
        (*environment)->SetLongArrayRegion(
            environment, out_camera, 0, 1, &out_camera_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraCreateWithTestBackendExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_camera)
{
    (void)environment;
    (void)declaring_class;
    CNA_CameraHandle out_camera_value = 0;
    CNA_Result call_result = cna.camera_create_with_test_backend_ext((CNA_Handle)game, &out_camera_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_camera_element = (jlong)out_camera_value;
        (*environment)->SetLongArrayRegion(
            environment, out_camera, 0, 1, &out_camera_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraDestroy(
    JNIEnv* environment, jclass declaring_class, jlong camera)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.camera_destroy((CNA_CameraHandle)camera);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraDeviceInfoInit(
    JNIEnv* environment, jclass declaring_class, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CameraDeviceInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.camera_device_info_init(&out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.position
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraGetCountExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.camera_get_count_ext((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraGetFrameHeightExt(
    JNIEnv* environment, jclass declaring_class, jlong camera, jintArray out_height)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_height_value = 0;
    CNA_Result call_result = cna.camera_get_frame_height_ext((CNA_CameraHandle)camera, &out_height_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_height_element = (jint)out_height_value;
        (*environment)->SetIntArrayRegion(
            environment, out_height, 0, 1, &out_height_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraGetFrameWidthExt(
    JNIEnv* environment, jclass declaring_class, jlong camera, jintArray out_width)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_width_value = 0;
    CNA_Result call_result = cna.camera_get_frame_width_ext((CNA_CameraHandle)camera, &out_width_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_width_element = (jint)out_width_value;
        (*environment)->SetIntArrayRegion(
            environment, out_width, 0, 1, &out_width_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraGetInfoAtExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong index, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CameraDeviceInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.camera_get_info_at_ext((CNA_Handle)game, (uint64_t)index, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.position
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraGetIsSupportedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.camera_get_is_supported_ext((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraGetNameSizeAtExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.camera_get_name_size_at_ext((CNA_Handle)game, (uint64_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraGetStateExt(
    JNIEnv* environment, jclass declaring_class, jlong camera, jintArray out_state)
{
    (void)environment;
    (void)declaring_class;
    CNA_CameraState out_state_value = 0;
    CNA_Result call_result = cna.camera_get_state_ext((CNA_CameraHandle)camera, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraSetTestFrameExt(
    JNIEnv* environment, jclass declaring_class, jlong camera, jint width, jint height, jlongArray pixelsIntegral)
{
    (void)environment;
    (void)declaring_class;
    jsize pixels_count = (*environment)->GetArrayLength(environment, pixelsIntegral) / 4;
    if ((*environment)->GetArrayLength(environment, pixelsIntegral) != pixels_count * 4) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Color* pixels_values = (CNA_Color*)calloc(
        (size_t)pixels_count + 1U, sizeof(*pixels_values));
    if (pixels_values == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    {
        jsize pixelsIntegral_length = (*environment)->GetArrayLength(environment, pixelsIntegral);
        jlong* pixelsIntegral_values = (jlong*)malloc(
            ((size_t)pixelsIntegral_length + 1U) * sizeof(*pixelsIntegral_values));
        if (pixelsIntegral_values == NULL) {
            free(pixels_values);
            return (jint)CNA_RESULT_OUT_OF_MEMORY;
        }
        (*environment)->GetLongArrayRegion(environment, pixelsIntegral, 0, pixelsIntegral_length, pixelsIntegral_values);
        for (jsize element = 0; element < pixels_count; ++element) {
            pixels_values[element].r = (uint8_t)pixelsIntegral_values[element * 4 + 0];
            pixels_values[element].g = (uint8_t)pixelsIntegral_values[element * 4 + 1];
            pixels_values[element].b = (uint8_t)pixelsIntegral_values[element * 4 + 2];
            pixels_values[element].a = (uint8_t)pixelsIntegral_values[element * 4 + 3];
        }
        free(pixelsIntegral_values);
    }
    CNA_Result call_result = cna.camera_set_test_frame_ext((CNA_CameraHandle)camera, (int32_t)width, (int32_t)height, pixels_values, (uint64_t)pixels_count);
    free(pixels_values);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraSetTestStateExt(
    JNIEnv* environment, jclass declaring_class, jlong camera, jint state)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.camera_set_test_state_ext((CNA_CameraHandle)camera, (CNA_CameraState)state);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_cameraTryAcquireFrameExt(
    JNIEnv* environment, jclass declaring_class, jlong camera, jlong texture, jbooleanArray out_acquired)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_acquired_value = 0;
    CNA_Result call_result = cna.camera_try_acquire_frame_ext((CNA_CameraHandle)camera, (CNA_Handle)texture, &out_acquired_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_acquired_element = (jboolean)out_acquired_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_acquired, 0, 1, &out_acquired_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_clipboardCopyText(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.clipboard_copy_text((CNA_Handle)game, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_clipboardGetHasText(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_has_text)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_has_text_value = 0;
    CNA_Result call_result = cna.clipboard_get_has_text((CNA_Handle)game, &out_has_text_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_has_text_element = (jboolean)out_has_text_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_text, 0, 1, &out_has_text_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_clipboardGetTextSize(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.clipboard_get_text_size((CNA_Handle)game, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_devicesClipboardSetTextExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray text, jbooleanArray out_accepted)
{
    (void)environment;
    (void)declaring_class;
    jsize text_size = (*environment)->GetArrayLength(environment, text);
    jbyte* text_bytes = (*environment)->GetByteArrayElements(environment, text, NULL);
    if (text_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView text_view = {(const char*)text_bytes, (uint64_t)text_size};
    CNA_Bool out_accepted_value = 0;
    CNA_Result call_result = cna.devices_clipboard_set_text_ext((CNA_Handle)game, text_view, &out_accepted_value);
    (*environment)->ReleaseByteArrayElements(environment, text, text_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_accepted_element = (jboolean)out_accepted_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_accepted, 0, 1, &out_accepted_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_devicesExtIsAvailable(
    JNIEnv* environment, jclass declaring_class, jbooleanArray out_available)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_available_value = 0;
    CNA_Result call_result = cna.devices_ext_is_available(&out_available_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_available_element = (jboolean)out_available_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_available, 0, 1, &out_available_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_displayInfoGetContentScaleExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jfloatArray out_scale)
{
    (void)environment;
    (void)declaring_class;
    float out_scale_value = 0;
    CNA_Result call_result = cna.display_info_get_content_scale_ext((CNA_Handle)game, &out_scale_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_scale_element = (jfloat)out_scale_value;
        (*environment)->SetFloatArrayRegion(
            environment, out_scale, 0, 1, &out_scale_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_displayInfoGetSafeAreaExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_areaIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_areaIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Rectangle out_area_value;
    memset(&out_area_value, 0, sizeof out_area_value);
    CNA_Result call_result = cna.display_info_get_safe_area_ext((CNA_Handle)game, &out_area_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_areaIntegral_values[] = {
            (jlong)out_area_value.x,
            (jlong)out_area_value.y,
            (jlong)out_area_value.width,
            (jlong)out_area_value.height
        };
        (*environment)->SetLongArrayRegion(environment, out_areaIntegral, 0,
            (jsize)(sizeof out_areaIntegral_values / sizeof out_areaIntegral_values[0]),
            out_areaIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_environmentGetDeviceType(
    JNIEnv* environment, jclass declaring_class, jintArray out_device_type)
{
    (void)environment;
    (void)declaring_class;
    CNA_DeviceType out_device_type_value = 0;
    CNA_Result call_result = cna.environment_get_device_type(&out_device_type_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_device_type_element = (jint)out_device_type_value;
        (*environment)->SetIntArrayRegion(
            environment, out_device_type, 0, 1, &out_device_type_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_fileDialogGetIsSupportedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.file_dialog_get_is_supported_ext((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_fileDialogSetTestBackendExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jboolean installed, jobjectArray results)
{
    (void)environment;
    (void)declaring_class;
    CnaJniStringViews results_views;
    const CNA_Result results_borrowed = cna_jni_borrow_string_views(
        environment, results, &results_views);
    if (results_borrowed != CNA_RESULT_SUCCESS) {
        return (jint)results_borrowed;
    }
    CNA_Result call_result = cna.file_dialog_set_test_backend_ext((CNA_Handle)game, (CNA_Bool)installed, (const CNA_StringView*)results_views.views, (uint64_t)results_views.count);
    cna_jni_free_string_views(&results_views);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesCopyKeyboardNameAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.input_devices_copy_keyboard_name_at((CNA_Handle)game, (uint32_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesCopyMouseNameAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.input_devices_copy_mouse_name_at((CNA_Handle)game, (uint32_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesCopyTouchDeviceNameAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.input_devices_copy_touch_device_name_at((CNA_Handle)game, (uint32_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetKeyboardCount(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.input_devices_get_keyboard_count((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetKeyboardInfoAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_InputDeviceInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.input_devices_get_keyboard_info_at((CNA_Handle)game, (uint32_t)index, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.id
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetKeyboardNameSizeAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.input_devices_get_keyboard_name_size_at((CNA_Handle)game, (uint32_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetMouseCount(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.input_devices_get_mouse_count((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetMouseInfoAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_InputDeviceInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.input_devices_get_mouse_info_at((CNA_Handle)game, (uint32_t)index, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.id
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetMouseNameSizeAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.input_devices_get_mouse_name_size_at((CNA_Handle)game, (uint32_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetTouchDeviceCount(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.input_devices_get_touch_device_count((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetTouchDeviceInfoAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_InputDeviceInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.input_devices_get_touch_device_info_at((CNA_Handle)game, (uint32_t)index, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.id
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesGetTouchDeviceNameSizeAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.input_devices_get_touch_device_name_size_at((CNA_Handle)game, (uint32_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesRaiseKeyboardConnectedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.input_devices_raise_keyboard_connected_ext((CNA_Handle)game, (uint32_t)id);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesRaiseKeyboardDisconnectedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.input_devices_raise_keyboard_disconnected_ext((CNA_Handle)game, (uint32_t)id);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesRaiseMouseConnectedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.input_devices_raise_mouse_connected_ext((CNA_Handle)game, (uint32_t)id);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_inputDevicesRaiseMouseDisconnectedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.input_devices_raise_mouse_disconnected_ext((CNA_Handle)game, (uint32_t)id);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_localeCopyCountryAtExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.locale_copy_country_at_ext((CNA_Handle)game, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_localeCopyLanguageAtExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.locale_copy_language_at_ext((CNA_Handle)game, (uint64_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_localeGetCountrySizeAtExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.locale_get_country_size_at_ext((CNA_Handle)game, (uint64_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_localeGetLanguageSizeAtExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.locale_get_language_size_at_ext((CNA_Handle)game, (uint64_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_localeGetPreferredCountExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.locale_get_preferred_count_ext((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_messageBoxGetIsSupportedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.message_box_get_is_supported_ext((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_messageBoxGetTestLogExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_logIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_logIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_MessageBoxTestLog out_log_value;
    memset(&out_log_value, 0, sizeof out_log_value);
    out_log_value.struct_size = (uint32_t)(sizeof out_log_value);
    out_log_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.message_box_get_test_log_ext((CNA_Handle)game, &out_log_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_logIntegral_values[] = {
            (jlong)out_log_value.simple_calls,
            (jlong)out_log_value.choice_calls,
            (jlong)out_log_value.last_type,
            (jlong)out_log_value.last_button_count
        };
        (*environment)->SetLongArrayRegion(environment, out_logIntegral, 0,
            (jsize)(sizeof out_logIntegral_values / sizeof out_logIntegral_values[0]),
            out_logIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_messageBoxSetTestBackendExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jboolean installed, jint chosen_button)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.message_box_set_test_backend_ext((CNA_Handle)game, (CNA_Bool)installed, (int32_t)chosen_button);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_messageBoxShowExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint type, jbyteArray title, jbyteArray message, jobjectArray button_labels, jintArray out_chosen)
{
    (void)environment;
    (void)declaring_class;
    jsize title_size = (*environment)->GetArrayLength(environment, title);
    jbyte* title_bytes = (*environment)->GetByteArrayElements(environment, title, NULL);
    if (title_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView title_view = {(const char*)title_bytes, (uint64_t)title_size};
    jsize message_size = (*environment)->GetArrayLength(environment, message);
    jbyte* message_bytes = (*environment)->GetByteArrayElements(environment, message, NULL);
    if (message_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView message_view = {(const char*)message_bytes, (uint64_t)message_size};
    CnaJniStringViews button_labels_views;
    const CNA_Result button_labels_borrowed = cna_jni_borrow_string_views(
        environment, button_labels, &button_labels_views);
    if (button_labels_borrowed != CNA_RESULT_SUCCESS) {
    (*environment)->ReleaseByteArrayElements(environment, message, message_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
        return (jint)button_labels_borrowed;
    }
    int32_t out_chosen_value = 0;
    CNA_Result call_result = cna.message_box_show_ext((CNA_Handle)game, (CNA_MessageBoxType)type, title_view, message_view, (const CNA_StringView*)button_labels_views.views, (uint64_t)button_labels_views.count, &out_chosen_value);
    (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, message, message_bytes, JNI_ABORT);
    cna_jni_free_string_views(&button_labels_views);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_chosen_element = (jint)out_chosen_value;
        (*environment)->SetIntArrayRegion(
            environment, out_chosen, 0, 1, &out_chosen_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_messageBoxShowSimpleExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint type, jbyteArray title, jbyteArray message)
{
    (void)environment;
    (void)declaring_class;
    jsize title_size = (*environment)->GetArrayLength(environment, title);
    jbyte* title_bytes = (*environment)->GetByteArrayElements(environment, title, NULL);
    if (title_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView title_view = {(const char*)title_bytes, (uint64_t)title_size};
    jsize message_size = (*environment)->GetArrayLength(environment, message);
    jbyte* message_bytes = (*environment)->GetByteArrayElements(environment, message, NULL);
    if (message_bytes == NULL) {
        (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView message_view = {(const char*)message_bytes, (uint64_t)message_size};
    CNA_Result call_result = cna.message_box_show_simple_ext((CNA_Handle)game, (CNA_MessageBoxType)type, title_view, message_view);
    (*environment)->ReleaseByteArrayElements(environment, title, title_bytes, JNI_ABORT);
    (*environment)->ReleaseByteArrayElements(environment, message, message_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_powerGetBatteryPercentExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_percent)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_percent_value = 0;
    CNA_Result call_result = cna.power_get_battery_percent_ext((CNA_Handle)game, &out_percent_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_percent_element = (jint)out_percent_value;
        (*environment)->SetIntArrayRegion(
            environment, out_percent, 0, 1, &out_percent_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_powerGetInfo(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_state, jintArray out_seconds_left, jintArray out_percent)
{
    (void)environment;
    (void)declaring_class;
    CNA_PowerState out_state_value = 0;
    int32_t out_seconds_left_value = 0;
    int32_t out_percent_value = 0;
    CNA_Result call_result = cna.power_get_info((CNA_Handle)game, &out_state_value, &out_seconds_left_value, &out_percent_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
        jint out_seconds_left_element = (jint)out_seconds_left_value;
        (*environment)->SetIntArrayRegion(
            environment, out_seconds_left, 0, 1, &out_seconds_left_element);
        jint out_percent_element = (jint)out_percent_value;
        (*environment)->SetIntArrayRegion(
            environment, out_percent, 0, 1, &out_percent_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_powerGetSecondsRemainingExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_seconds)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_seconds_value = 0;
    CNA_Result call_result = cna.power_get_seconds_remaining_ext((CNA_Handle)game, &out_seconds_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_seconds_element = (jint)out_seconds_value;
        (*environment)->SetIntArrayRegion(
            environment, out_seconds, 0, 1, &out_seconds_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_powerGetStateExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_state)
{
    (void)environment;
    (void)declaring_class;
    CNA_PowerState out_state_value = 0;
    CNA_Result call_result = cna.power_get_state_ext((CNA_Handle)game, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemInfoGetLogicalCpuCoreCountExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.system_info_get_logical_cpu_core_count_ext((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemInfoGetSystemRamMegabytesExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_megabytes)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_megabytes_value = 0;
    CNA_Result call_result = cna.system_info_get_system_ram_megabytes_ext((CNA_Handle)game, &out_megabytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_megabytes_element = (jint)out_megabytes_value;
        (*environment)->SetIntArrayRegion(
            environment, out_megabytes, 0, 1, &out_megabytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTrayClickEntryForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong tray, jlong index)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.system_tray_click_entry_for_tests_ext((CNA_SystemTrayHandle)tray, (uint64_t)index);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTrayCreate(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray tooltip, jlongArray out_tray)
{
    (void)environment;
    (void)declaring_class;
    jsize tooltip_size = (*environment)->GetArrayLength(environment, tooltip);
    jbyte* tooltip_bytes = (*environment)->GetByteArrayElements(environment, tooltip, NULL);
    if (tooltip_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView tooltip_view = {(const char*)tooltip_bytes, (uint64_t)tooltip_size};
    CNA_SystemTrayHandle out_tray_value = 0;
    CNA_Result call_result = cna.system_tray_create((CNA_Handle)game, tooltip_view, &out_tray_value);
    (*environment)->ReleaseByteArrayElements(environment, tooltip, tooltip_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_tray_element = (jlong)out_tray_value;
        (*environment)->SetLongArrayRegion(
            environment, out_tray, 0, 1, &out_tray_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTrayCreateWithTestBackendExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray tooltip, jlongArray out_tray)
{
    (void)environment;
    (void)declaring_class;
    jsize tooltip_size = (*environment)->GetArrayLength(environment, tooltip);
    jbyte* tooltip_bytes = (*environment)->GetByteArrayElements(environment, tooltip, NULL);
    if (tooltip_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView tooltip_view = {(const char*)tooltip_bytes, (uint64_t)tooltip_size};
    CNA_SystemTrayHandle out_tray_value = 0;
    CNA_Result call_result = cna.system_tray_create_with_test_backend_ext((CNA_Handle)game, tooltip_view, &out_tray_value);
    (*environment)->ReleaseByteArrayElements(environment, tooltip, tooltip_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_tray_element = (jlong)out_tray_value;
        (*environment)->SetLongArrayRegion(
            environment, out_tray, 0, 1, &out_tray_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTrayDestroy(
    JNIEnv* environment, jclass declaring_class, jlong tray)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.system_tray_destroy((CNA_SystemTrayHandle)tray);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTrayGetEntryChecked(
    JNIEnv* environment, jclass declaring_class, jlong tray, jlong index, jbooleanArray out_checked)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_checked_value = 0;
    CNA_Result call_result = cna.system_tray_get_entry_checked((CNA_SystemTrayHandle)tray, (uint64_t)index, &out_checked_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_checked_element = (jboolean)out_checked_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_checked, 0, 1, &out_checked_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTrayGetEntryEnabled(
    JNIEnv* environment, jclass declaring_class, jlong tray, jlong index, jbooleanArray out_enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_enabled_value = 0;
    CNA_Result call_result = cna.system_tray_get_entry_enabled((CNA_SystemTrayHandle)tray, (uint64_t)index, &out_enabled_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_enabled_element = (jboolean)out_enabled_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_enabled, 0, 1, &out_enabled_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTrayGetIsSupportedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.system_tray_get_is_supported_ext((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTraySetEntryChecked(
    JNIEnv* environment, jclass declaring_class, jlong tray, jlong index, jboolean checked)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.system_tray_set_entry_checked((CNA_SystemTrayHandle)tray, (uint64_t)index, (CNA_Bool)checked);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTraySetEntryEnabled(
    JNIEnv* environment, jclass declaring_class, jlong tray, jlong index, jboolean enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.system_tray_set_entry_enabled((CNA_SystemTrayHandle)tray, (uint64_t)index, (CNA_Bool)enabled);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTraySetEntryLabel(
    JNIEnv* environment, jclass declaring_class, jlong tray, jlong index, jbyteArray label)
{
    (void)environment;
    (void)declaring_class;
    jsize label_size = (*environment)->GetArrayLength(environment, label);
    jbyte* label_bytes = (*environment)->GetByteArrayElements(environment, label, NULL);
    if (label_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView label_view = {(const char*)label_bytes, (uint64_t)label_size};
    CNA_Result call_result = cna.system_tray_set_entry_label((CNA_SystemTrayHandle)tray, (uint64_t)index, label_view);
    (*environment)->ReleaseByteArrayElements(environment, label, label_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_systemTraySetTooltip(
    JNIEnv* environment, jclass declaring_class, jlong tray, jbyteArray tooltip)
{
    (void)environment;
    (void)declaring_class;
    jsize tooltip_size = (*environment)->GetArrayLength(environment, tooltip);
    jbyte* tooltip_bytes = (*environment)->GetByteArrayElements(environment, tooltip, NULL);
    if (tooltip_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView tooltip_view = {(const char*)tooltip_bytes, (uint64_t)tooltip_size};
    CNA_Result call_result = cna.system_tray_set_tooltip((CNA_SystemTrayHandle)tray, tooltip_view);
    (*environment)->ReleaseByteArrayElements(environment, tooltip, tooltip_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_urlLauncherOpenExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray url, jbooleanArray out_opened)
{
    (void)environment;
    (void)declaring_class;
    jsize url_size = (*environment)->GetArrayLength(environment, url);
    jbyte* url_bytes = (*environment)->GetByteArrayElements(environment, url, NULL);
    if (url_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView url_view = {(const char*)url_bytes, (uint64_t)url_size};
    CNA_Bool out_opened_value = 0;
    CNA_Result call_result = cna.url_launcher_open_ext((CNA_Handle)game, url_view, &out_opened_value);
    (*environment)->ReleaseByteArrayElements(environment, url, url_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_opened_element = (jboolean)out_opened_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_opened, 0, 1, &out_opened_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerCopyDeviceNameExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.vibrate_controller_copy_device_name_ext((CNA_Handle)game, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerGetDeviceNameSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.vibrate_controller_get_device_name_size_ext((CNA_Handle)game, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerGetIsSupportedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.vibrate_controller_get_is_supported_ext((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerGetTestLogExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_logIntegral, jfloatArray out_logFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_logIntegral == NULL || out_logFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_VibrationTestLog out_log_value;
    memset(&out_log_value, 0, sizeof out_log_value);
    out_log_value.struct_size = (uint32_t)(sizeof out_log_value);
    out_log_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.vibrate_controller_get_test_log_ext((CNA_Handle)game, &out_log_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_logIntegral_values[] = {
            (jlong)out_log_value.start_calls,
            (jlong)out_log_value.stop_calls,
            (jlong)out_log_value.left_right_calls,
            (jlong)out_log_value.reserved,
            (jlong)out_log_value.last_duration_ticks
        };
        (*environment)->SetLongArrayRegion(environment, out_logIntegral, 0,
            (jsize)(sizeof out_logIntegral_values / sizeof out_logIntegral_values[0]),
            out_logIntegral_values);
        jfloat out_logFloating_values[] = {
            (jfloat)out_log_value.last_intensity,
            (jfloat)out_log_value.last_large_motor,
            (jfloat)out_log_value.last_small_motor,
            (jfloat)out_log_value.reserved_float
        };
        (*environment)->SetFloatArrayRegion(environment, out_logFloating, 0,
            (jsize)(sizeof out_logFloating_values / sizeof out_logFloating_values[0]),
            out_logFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerSetTestBackendExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jboolean installed, jboolean supported, jbyteArray device_name)
{
    (void)environment;
    (void)declaring_class;
    jsize device_name_size = (*environment)->GetArrayLength(environment, device_name);
    jbyte* device_name_bytes = (*environment)->GetByteArrayElements(environment, device_name, NULL);
    if (device_name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView device_name_view = {(const char*)device_name_bytes, (uint64_t)device_name_size};
    CNA_Result call_result = cna.vibrate_controller_set_test_backend_ext((CNA_Handle)game, (CNA_Bool)installed, (CNA_Bool)supported, device_name_view);
    (*environment)->ReleaseByteArrayElements(environment, device_name, device_name_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerStart(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong duration_ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.vibrate_controller_start((CNA_Handle)game, (int64_t)duration_ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerStartLeftRightExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jfloat large_motor, jfloat small_motor, jlong duration_ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.vibrate_controller_start_left_right_ext((CNA_Handle)game, (float)large_motor, (float)small_motor, (int64_t)duration_ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerStartWithIntensityExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong duration_ticks, jfloat intensity)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.vibrate_controller_start_with_intensity_ext((CNA_Handle)game, (int64_t)duration_ticks, (float)intensity);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeDeviceExtensionRoutes_vibrateControllerStop(
    JNIEnv* environment, jclass declaring_class, jlong game)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.vibrate_controller_stop((CNA_Handle)game);
    return (jint)call_result;
}
