/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeInputExtensionRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadCopyGuidExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamepad_copy_guid_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadCopyNameExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamepad_copy_name_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadCopyPathExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamepad_copy_path_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadCopySerialExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.gamepad_copy_serial_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetAccelerometerExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jfloatArray out_accelerationFloating, jbooleanArray out_available)
{
    (void)environment;
    (void)declaring_class;
    if (out_accelerationFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 out_acceleration_value;
    memset(&out_acceleration_value, 0, sizeof out_acceleration_value);
    CNA_Bool out_available_value = 0;
    CNA_Result call_result = cna.gamepad_get_accelerometer_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_acceleration_value, &out_available_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_accelerationFloating_values[] = {
            (jfloat)out_acceleration_value.x,
            (jfloat)out_acceleration_value.y,
            (jfloat)out_acceleration_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_accelerationFloating, 0,
            (jsize)(sizeof out_accelerationFloating_values / sizeof out_accelerationFloating_values[0]),
            out_accelerationFloating_values);
        jboolean out_available_element = (jboolean)out_available_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_available, 0, 1, &out_available_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetButtonLabelExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jint button, jintArray out_label)
{
    (void)environment;
    (void)declaring_class;
    CNA_GamePadButtonLabel out_label_value = 0;
    CNA_Result call_result = cna.gamepad_get_button_label_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (CNA_GamePadButtonFlags)button, &out_label_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_label_element = (jint)out_label_value;
        (*environment)->SetIntArrayRegion(
            environment, out_label, 0, 1, &out_label_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetConnectionStateExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jintArray out_state)
{
    (void)environment;
    (void)declaring_class;
    CNA_GamePadConnectionState out_state_value = 0;
    CNA_Result call_result = cna.gamepad_get_connection_state_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetFirmwareVersionExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jintArray out_version)
{
    (void)environment;
    (void)declaring_class;
    uint16_t out_version_value = 0;
    CNA_Result call_result = cna.gamepad_get_firmware_version_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_version_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_version_element = (jint)out_version_value;
        (*environment)->SetIntArrayRegion(
            environment, out_version, 0, 1, &out_version_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetGuidSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamepad_get_guid_size_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetGyroExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jfloatArray out_gyroFloating, jbooleanArray out_available)
{
    (void)environment;
    (void)declaring_class;
    if (out_gyroFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 out_gyro_value;
    memset(&out_gyro_value, 0, sizeof out_gyro_value);
    CNA_Bool out_available_value = 0;
    CNA_Result call_result = cna.gamepad_get_gyro_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_gyro_value, &out_available_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_gyroFloating_values[] = {
            (jfloat)out_gyro_value.x,
            (jfloat)out_gyro_value.y,
            (jfloat)out_gyro_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_gyroFloating, 0,
            (jsize)(sizeof out_gyroFloating_values / sizeof out_gyroFloating_values[0]),
            out_gyroFloating_values);
        jboolean out_available_element = (jboolean)out_available_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_available, 0, 1, &out_available_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetNameSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamepad_get_name_size_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetPathSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamepad_get_path_size_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetPlayerIndexExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jintArray out_index)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_index_value = 0;
    CNA_Result call_result = cna.gamepad_get_player_index_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_index_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_index_element = (jint)out_index_value;
        (*environment)->SetIntArrayRegion(
            environment, out_index, 0, 1, &out_index_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetPowerInfoExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jintArray out_state, jintArray out_percent)
{
    (void)environment;
    (void)declaring_class;
    CNA_PowerState out_state_value = 0;
    int32_t out_percent_value = 0;
    CNA_Result call_result = cna.gamepad_get_power_info_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_state_value, &out_percent_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
        jint out_percent_element = (jint)out_percent_value;
        (*environment)->SetIntArrayRegion(
            environment, out_percent, 0, 1, &out_percent_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetSerialSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gamepad_get_serial_size_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetSteamHandleExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jlongArray out_handle)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_handle_value = 0;
    CNA_Result call_result = cna.gamepad_get_steam_handle_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_handle_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_handle_element = (jlong)out_handle_value;
        (*environment)->SetLongArrayRegion(
            environment, out_handle, 0, 1, &out_handle_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetTouchpadCountExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.gamepad_get_touchpad_count_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetTouchpadFingerCountExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jint touchpad, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.gamepad_get_touchpad_finger_count_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (int32_t)touchpad, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadGetTouchpadFingerExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jint touchpad, jint finger, jbyteArray out_fingerBytes, jlongArray out_fingerIntegral, jfloatArray out_fingerFloating, jbooleanArray out_available)
{
    (void)environment;
    (void)declaring_class;
    if (out_fingerBytes == NULL || out_fingerIntegral == NULL || out_fingerFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GamePadTouchpadFinger out_finger_value;
    memset(&out_finger_value, 0, sizeof out_finger_value);
    CNA_Bool out_available_value = 0;
    CNA_Result call_result = cna.gamepad_get_touchpad_finger_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (int32_t)touchpad, (int32_t)finger, &out_finger_value, &out_available_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_fingerBytes_values[] = {
            (jbyte)out_finger_value.reserved[0],
            (jbyte)out_finger_value.reserved[1],
            (jbyte)out_finger_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_fingerBytes, 0,
            (jsize)(sizeof out_fingerBytes_values / sizeof out_fingerBytes_values[0]),
            out_fingerBytes_values);
        jlong out_fingerIntegral_values[] = {
            (jlong)out_finger_value.is_down
        };
        (*environment)->SetLongArrayRegion(environment, out_fingerIntegral, 0,
            (jsize)(sizeof out_fingerIntegral_values / sizeof out_fingerIntegral_values[0]),
            out_fingerIntegral_values);
        jfloat out_fingerFloating_values[] = {
            (jfloat)out_finger_value.x,
            (jfloat)out_finger_value.y,
            (jfloat)out_finger_value.pressure
        };
        (*environment)->SetFloatArrayRegion(environment, out_fingerFloating, 0,
            (jsize)(sizeof out_fingerFloating_values / sizeof out_fingerFloating_values[0]),
            out_fingerFloating_values);
        jboolean out_available_element = (jboolean)out_available_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_available, 0, 1, &out_available_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadSetLightBarExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jlongArray colorIntegral)
{
    (void)environment;
    (void)declaring_class;
    CNA_Color color_value;
    memset(&color_value, 0, sizeof color_value);
    {
        jlong colorIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, colorIntegral, 0, 4, colorIntegral_values);
        color_value.r = (uint8_t)colorIntegral_values[0];
        color_value.g = (uint8_t)colorIntegral_values[1];
        color_value.b = (uint8_t)colorIntegral_values[2];
        color_value.a = (uint8_t)colorIntegral_values[3];
    }
    CNA_Result call_result = cna.gamepad_set_light_bar_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, color_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadSetPlayerIndexExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jint index, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.gamepad_set_player_index_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (int32_t)index, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_gamepadSetTriggerVibrationExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint player_index, jfloat left_trigger, jfloat right_trigger, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.gamepad_set_trigger_vibration_ext((CNA_Handle)game, (CNA_PlayerIndex)player_index, (float)left_trigger, (float)right_trigger, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceCopyName(
    JNIEnv* environment, jclass declaring_class, jlong device, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.haptic_device_copy_name((CNA_HapticDeviceHandle)device, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceCreateEffect(
    JNIEnv* environment, jclass declaring_class, jlong device, jlongArray effectIntegral, jintArray custom_data, jintArray out_effect_id)
{
    (void)environment;
    (void)declaring_class;
    if (effectIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_HapticEffect effect_value;
    memset(&effect_value, 0, sizeof effect_value);
    effect_value.struct_size = (uint32_t)(sizeof effect_value);
    effect_value.struct_version = UINT32_C(1);
    {
        jlong effectIntegral_values[44];
        (*environment)->GetLongArrayRegion(environment, effectIntegral, 0, 44, effectIntegral_values);
        effect_value.type = (CNA_HapticEffectType)effectIntegral_values[0];
        effect_value.reserved = (uint32_t)effectIntegral_values[1];
        effect_value.direction.type = (CNA_HapticDirectionType)effectIntegral_values[2];
        effect_value.direction.values[0] = (int32_t)effectIntegral_values[3];
        effect_value.direction.values[1] = (int32_t)effectIntegral_values[4];
        effect_value.direction.values[2] = (int32_t)effectIntegral_values[5];
        effect_value.length = (uint32_t)effectIntegral_values[6];
        effect_value.delay = (uint16_t)effectIntegral_values[7];
        effect_value.button = (uint16_t)effectIntegral_values[8];
        effect_value.interval = (uint16_t)effectIntegral_values[9];
        effect_value.level = (int16_t)effectIntegral_values[10];
        effect_value.period = (uint16_t)effectIntegral_values[11];
        effect_value.magnitude = (int16_t)effectIntegral_values[12];
        effect_value.offset = (int16_t)effectIntegral_values[13];
        effect_value.phase = (uint16_t)effectIntegral_values[14];
        effect_value.ramp_start = (int16_t)effectIntegral_values[15];
        effect_value.ramp_end = (int16_t)effectIntegral_values[16];
        effect_value.right_saturation[0] = (uint16_t)effectIntegral_values[17];
        effect_value.right_saturation[1] = (uint16_t)effectIntegral_values[18];
        effect_value.right_saturation[2] = (uint16_t)effectIntegral_values[19];
        effect_value.left_saturation[0] = (uint16_t)effectIntegral_values[20];
        effect_value.left_saturation[1] = (uint16_t)effectIntegral_values[21];
        effect_value.left_saturation[2] = (uint16_t)effectIntegral_values[22];
        effect_value.right_coefficient[0] = (int16_t)effectIntegral_values[23];
        effect_value.right_coefficient[1] = (int16_t)effectIntegral_values[24];
        effect_value.right_coefficient[2] = (int16_t)effectIntegral_values[25];
        effect_value.left_coefficient[0] = (int16_t)effectIntegral_values[26];
        effect_value.left_coefficient[1] = (int16_t)effectIntegral_values[27];
        effect_value.left_coefficient[2] = (int16_t)effectIntegral_values[28];
        effect_value.deadband[0] = (uint16_t)effectIntegral_values[29];
        effect_value.deadband[1] = (uint16_t)effectIntegral_values[30];
        effect_value.deadband[2] = (uint16_t)effectIntegral_values[31];
        effect_value.center[0] = (int16_t)effectIntegral_values[32];
        effect_value.center[1] = (int16_t)effectIntegral_values[33];
        effect_value.center[2] = (int16_t)effectIntegral_values[34];
        effect_value.large_magnitude = (uint16_t)effectIntegral_values[35];
        effect_value.small_magnitude = (uint16_t)effectIntegral_values[36];
        effect_value.custom_period = (uint16_t)effectIntegral_values[37];
        effect_value.custom_channels = (uint8_t)effectIntegral_values[38];
        effect_value.reserved2 = (uint8_t)effectIntegral_values[39];
        effect_value.attack_length = (uint16_t)effectIntegral_values[40];
        effect_value.attack_level = (uint16_t)effectIntegral_values[41];
        effect_value.fade_length = (uint16_t)effectIntegral_values[42];
        effect_value.fade_level = (uint16_t)effectIntegral_values[43];
    }
    jsize custom_data_size = (*environment)->GetArrayLength(environment, custom_data);
    jint* custom_data_elements = (*environment)->GetIntArrayElements(environment, custom_data, NULL);
    if (custom_data_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint16_t* custom_data_values = (uint16_t*)malloc(
        ((size_t)custom_data_size + 1U) * sizeof(*custom_data_values));
    if (custom_data_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, custom_data, custom_data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < custom_data_size; ++index) {
        custom_data_values[index] = (uint16_t)custom_data_elements[index];
    }
    int32_t out_effect_id_value = 0;
    CNA_Result call_result = cna.haptic_device_create_effect((CNA_HapticDeviceHandle)device, &effect_value, custom_data_values, (uint64_t)custom_data_size, &out_effect_id_value);
    free(custom_data_values);
    (*environment)->ReleaseIntArrayElements(
        environment, custom_data, custom_data_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_effect_id_element = (jint)out_effect_id_value;
        (*environment)->SetIntArrayRegion(
            environment, out_effect_id, 0, 1, &out_effect_id_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceDestroy(
    JNIEnv* environment, jclass declaring_class, jlong device)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.haptic_device_destroy((CNA_HapticDeviceHandle)device);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceDestroyEffect(
    JNIEnv* environment, jclass declaring_class, jlong device, jint effect_id)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.haptic_device_destroy_effect((CNA_HapticDeviceHandle)device, (int32_t)effect_id);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceDispose(
    JNIEnv* environment, jclass declaring_class, jlong device)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.haptic_device_dispose((CNA_HapticDeviceHandle)device);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceGetCapabilities(
    JNIEnv* environment, jclass declaring_class, jlong device, jbyteArray out_capabilitiesBytes, jlongArray out_capabilitiesIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_capabilitiesBytes == NULL || out_capabilitiesIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_HapticCapabilities out_capabilities_value;
    memset(&out_capabilities_value, 0, sizeof out_capabilities_value);
    out_capabilities_value.struct_size = (uint32_t)(sizeof out_capabilities_value);
    out_capabilities_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.haptic_device_get_capabilities((CNA_HapticDeviceHandle)device, &out_capabilities_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_capabilitiesBytes_values[] = {
            (jbyte)out_capabilities_value.reserved[0],
            (jbyte)out_capabilities_value.reserved[1]
        };
        (*environment)->SetByteArrayRegion(environment, out_capabilitiesBytes, 0,
            (jsize)(sizeof out_capabilitiesBytes_values / sizeof out_capabilitiesBytes_values[0]),
            out_capabilitiesBytes_values);
        jlong out_capabilitiesIntegral_values[] = {
            (jlong)out_capabilities_value.features,
            (jlong)out_capabilities_value.axis_count,
            (jlong)out_capabilities_value.max_effects,
            (jlong)out_capabilities_value.max_effects_playing,
            (jlong)out_capabilities_value.is_open,
            (jlong)out_capabilities_value.rumble_supported
        };
        (*environment)->SetLongArrayRegion(environment, out_capabilitiesIntegral, 0,
            (jsize)(sizeof out_capabilitiesIntegral_values / sizeof out_capabilitiesIntegral_values[0]),
            out_capabilitiesIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceGetEffectStatus(
    JNIEnv* environment, jclass declaring_class, jlong device, jint effect_id, jbooleanArray out_playing)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_playing_value = 0;
    CNA_Result call_result = cna.haptic_device_get_effect_status((CNA_HapticDeviceHandle)device, (int32_t)effect_id, &out_playing_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_playing_element = (jboolean)out_playing_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_playing, 0, 1, &out_playing_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceGetIsEffectSupported(
    JNIEnv* environment, jclass declaring_class, jlong device, jlongArray effectIntegral, jintArray custom_data, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    if (effectIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_HapticEffect effect_value;
    memset(&effect_value, 0, sizeof effect_value);
    effect_value.struct_size = (uint32_t)(sizeof effect_value);
    effect_value.struct_version = UINT32_C(1);
    {
        jlong effectIntegral_values[44];
        (*environment)->GetLongArrayRegion(environment, effectIntegral, 0, 44, effectIntegral_values);
        effect_value.type = (CNA_HapticEffectType)effectIntegral_values[0];
        effect_value.reserved = (uint32_t)effectIntegral_values[1];
        effect_value.direction.type = (CNA_HapticDirectionType)effectIntegral_values[2];
        effect_value.direction.values[0] = (int32_t)effectIntegral_values[3];
        effect_value.direction.values[1] = (int32_t)effectIntegral_values[4];
        effect_value.direction.values[2] = (int32_t)effectIntegral_values[5];
        effect_value.length = (uint32_t)effectIntegral_values[6];
        effect_value.delay = (uint16_t)effectIntegral_values[7];
        effect_value.button = (uint16_t)effectIntegral_values[8];
        effect_value.interval = (uint16_t)effectIntegral_values[9];
        effect_value.level = (int16_t)effectIntegral_values[10];
        effect_value.period = (uint16_t)effectIntegral_values[11];
        effect_value.magnitude = (int16_t)effectIntegral_values[12];
        effect_value.offset = (int16_t)effectIntegral_values[13];
        effect_value.phase = (uint16_t)effectIntegral_values[14];
        effect_value.ramp_start = (int16_t)effectIntegral_values[15];
        effect_value.ramp_end = (int16_t)effectIntegral_values[16];
        effect_value.right_saturation[0] = (uint16_t)effectIntegral_values[17];
        effect_value.right_saturation[1] = (uint16_t)effectIntegral_values[18];
        effect_value.right_saturation[2] = (uint16_t)effectIntegral_values[19];
        effect_value.left_saturation[0] = (uint16_t)effectIntegral_values[20];
        effect_value.left_saturation[1] = (uint16_t)effectIntegral_values[21];
        effect_value.left_saturation[2] = (uint16_t)effectIntegral_values[22];
        effect_value.right_coefficient[0] = (int16_t)effectIntegral_values[23];
        effect_value.right_coefficient[1] = (int16_t)effectIntegral_values[24];
        effect_value.right_coefficient[2] = (int16_t)effectIntegral_values[25];
        effect_value.left_coefficient[0] = (int16_t)effectIntegral_values[26];
        effect_value.left_coefficient[1] = (int16_t)effectIntegral_values[27];
        effect_value.left_coefficient[2] = (int16_t)effectIntegral_values[28];
        effect_value.deadband[0] = (uint16_t)effectIntegral_values[29];
        effect_value.deadband[1] = (uint16_t)effectIntegral_values[30];
        effect_value.deadband[2] = (uint16_t)effectIntegral_values[31];
        effect_value.center[0] = (int16_t)effectIntegral_values[32];
        effect_value.center[1] = (int16_t)effectIntegral_values[33];
        effect_value.center[2] = (int16_t)effectIntegral_values[34];
        effect_value.large_magnitude = (uint16_t)effectIntegral_values[35];
        effect_value.small_magnitude = (uint16_t)effectIntegral_values[36];
        effect_value.custom_period = (uint16_t)effectIntegral_values[37];
        effect_value.custom_channels = (uint8_t)effectIntegral_values[38];
        effect_value.reserved2 = (uint8_t)effectIntegral_values[39];
        effect_value.attack_length = (uint16_t)effectIntegral_values[40];
        effect_value.attack_level = (uint16_t)effectIntegral_values[41];
        effect_value.fade_length = (uint16_t)effectIntegral_values[42];
        effect_value.fade_level = (uint16_t)effectIntegral_values[43];
    }
    jsize custom_data_size = (*environment)->GetArrayLength(environment, custom_data);
    jint* custom_data_elements = (*environment)->GetIntArrayElements(environment, custom_data, NULL);
    if (custom_data_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint16_t* custom_data_values = (uint16_t*)malloc(
        ((size_t)custom_data_size + 1U) * sizeof(*custom_data_values));
    if (custom_data_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, custom_data, custom_data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < custom_data_size; ++index) {
        custom_data_values[index] = (uint16_t)custom_data_elements[index];
    }
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.haptic_device_get_is_effect_supported((CNA_HapticDeviceHandle)device, &effect_value, custom_data_values, (uint64_t)custom_data_size, &out_supported_value);
    free(custom_data_values);
    (*environment)->ReleaseIntArrayElements(
        environment, custom_data, custom_data_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceGetIsOpen(
    JNIEnv* environment, jclass declaring_class, jlong device, jbooleanArray out_open)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_open_value = 0;
    CNA_Result call_result = cna.haptic_device_get_is_open((CNA_HapticDeviceHandle)device, &out_open_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_open_element = (jboolean)out_open_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_open, 0, 1, &out_open_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceGetNameSize(
    JNIEnv* environment, jclass declaring_class, jlong device, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.haptic_device_get_name_size((CNA_HapticDeviceHandle)device, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceInitRumble(
    JNIEnv* environment, jclass declaring_class, jlong device, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_init_rumble((CNA_HapticDeviceHandle)device, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDevicePause(
    JNIEnv* environment, jclass declaring_class, jlong device, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_pause((CNA_HapticDeviceHandle)device, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDevicePlayRumble(
    JNIEnv* environment, jclass declaring_class, jlong device, jfloat strength, jint length_ms, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_play_rumble((CNA_HapticDeviceHandle)device, (float)strength, (uint32_t)length_ms, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceResume(
    JNIEnv* environment, jclass declaring_class, jlong device, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_resume((CNA_HapticDeviceHandle)device, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceRunEffect(
    JNIEnv* environment, jclass declaring_class, jlong device, jint effect_id, jint iterations, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_run_effect((CNA_HapticDeviceHandle)device, (int32_t)effect_id, (uint32_t)iterations, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceSetAutocenter(
    JNIEnv* environment, jclass declaring_class, jlong device, jint autocenter, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_set_autocenter((CNA_HapticDeviceHandle)device, (int32_t)autocenter, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceSetGain(
    JNIEnv* environment, jclass declaring_class, jlong device, jint gain, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_set_gain((CNA_HapticDeviceHandle)device, (int32_t)gain, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceStopAllEffects(
    JNIEnv* environment, jclass declaring_class, jlong device, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_stop_all_effects((CNA_HapticDeviceHandle)device, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceStopEffect(
    JNIEnv* environment, jclass declaring_class, jlong device, jint effect_id, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_stop_effect((CNA_HapticDeviceHandle)device, (int32_t)effect_id, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceStopRumble(
    JNIEnv* environment, jclass declaring_class, jlong device, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_stop_rumble((CNA_HapticDeviceHandle)device, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticDeviceUpdateEffect(
    JNIEnv* environment, jclass declaring_class, jlong device, jint effect_id, jlongArray effectIntegral, jintArray custom_data, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    if (effectIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_HapticEffect effect_value;
    memset(&effect_value, 0, sizeof effect_value);
    effect_value.struct_size = (uint32_t)(sizeof effect_value);
    effect_value.struct_version = UINT32_C(1);
    {
        jlong effectIntegral_values[44];
        (*environment)->GetLongArrayRegion(environment, effectIntegral, 0, 44, effectIntegral_values);
        effect_value.type = (CNA_HapticEffectType)effectIntegral_values[0];
        effect_value.reserved = (uint32_t)effectIntegral_values[1];
        effect_value.direction.type = (CNA_HapticDirectionType)effectIntegral_values[2];
        effect_value.direction.values[0] = (int32_t)effectIntegral_values[3];
        effect_value.direction.values[1] = (int32_t)effectIntegral_values[4];
        effect_value.direction.values[2] = (int32_t)effectIntegral_values[5];
        effect_value.length = (uint32_t)effectIntegral_values[6];
        effect_value.delay = (uint16_t)effectIntegral_values[7];
        effect_value.button = (uint16_t)effectIntegral_values[8];
        effect_value.interval = (uint16_t)effectIntegral_values[9];
        effect_value.level = (int16_t)effectIntegral_values[10];
        effect_value.period = (uint16_t)effectIntegral_values[11];
        effect_value.magnitude = (int16_t)effectIntegral_values[12];
        effect_value.offset = (int16_t)effectIntegral_values[13];
        effect_value.phase = (uint16_t)effectIntegral_values[14];
        effect_value.ramp_start = (int16_t)effectIntegral_values[15];
        effect_value.ramp_end = (int16_t)effectIntegral_values[16];
        effect_value.right_saturation[0] = (uint16_t)effectIntegral_values[17];
        effect_value.right_saturation[1] = (uint16_t)effectIntegral_values[18];
        effect_value.right_saturation[2] = (uint16_t)effectIntegral_values[19];
        effect_value.left_saturation[0] = (uint16_t)effectIntegral_values[20];
        effect_value.left_saturation[1] = (uint16_t)effectIntegral_values[21];
        effect_value.left_saturation[2] = (uint16_t)effectIntegral_values[22];
        effect_value.right_coefficient[0] = (int16_t)effectIntegral_values[23];
        effect_value.right_coefficient[1] = (int16_t)effectIntegral_values[24];
        effect_value.right_coefficient[2] = (int16_t)effectIntegral_values[25];
        effect_value.left_coefficient[0] = (int16_t)effectIntegral_values[26];
        effect_value.left_coefficient[1] = (int16_t)effectIntegral_values[27];
        effect_value.left_coefficient[2] = (int16_t)effectIntegral_values[28];
        effect_value.deadband[0] = (uint16_t)effectIntegral_values[29];
        effect_value.deadband[1] = (uint16_t)effectIntegral_values[30];
        effect_value.deadband[2] = (uint16_t)effectIntegral_values[31];
        effect_value.center[0] = (int16_t)effectIntegral_values[32];
        effect_value.center[1] = (int16_t)effectIntegral_values[33];
        effect_value.center[2] = (int16_t)effectIntegral_values[34];
        effect_value.large_magnitude = (uint16_t)effectIntegral_values[35];
        effect_value.small_magnitude = (uint16_t)effectIntegral_values[36];
        effect_value.custom_period = (uint16_t)effectIntegral_values[37];
        effect_value.custom_channels = (uint8_t)effectIntegral_values[38];
        effect_value.reserved2 = (uint8_t)effectIntegral_values[39];
        effect_value.attack_length = (uint16_t)effectIntegral_values[40];
        effect_value.attack_level = (uint16_t)effectIntegral_values[41];
        effect_value.fade_length = (uint16_t)effectIntegral_values[42];
        effect_value.fade_level = (uint16_t)effectIntegral_values[43];
    }
    jsize custom_data_size = (*environment)->GetArrayLength(environment, custom_data);
    jint* custom_data_elements = (*environment)->GetIntArrayElements(environment, custom_data, NULL);
    if (custom_data_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint16_t* custom_data_values = (uint16_t*)malloc(
        ((size_t)custom_data_size + 1U) * sizeof(*custom_data_values));
    if (custom_data_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, custom_data, custom_data_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < custom_data_size; ++index) {
        custom_data_values[index] = (uint16_t)custom_data_elements[index];
    }
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.haptic_device_update_effect((CNA_HapticDeviceHandle)device, (int32_t)effect_id, &effect_value, custom_data_values, (uint64_t)custom_data_size, &out_applied_value);
    free(custom_data_values);
    (*environment)->ReleaseIntArrayElements(
        environment, custom_data, custom_data_elements, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsCopyNameAt(
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
    CNA_Result call_result = cna.haptics_copy_name_at((CNA_Handle)game, (uint32_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsGetCount(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.haptics_get_count((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsGetIdAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jintArray out_id)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_id_value = 0;
    CNA_Result call_result = cna.haptics_get_id_at((CNA_Handle)game, (uint32_t)index, &out_id_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_id_element = (jint)out_id_value;
        (*environment)->SetIntArrayRegion(
            environment, out_id, 0, 1, &out_id_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsGetIsJoystickHaptic(
    JNIEnv* environment, jclass declaring_class, jlong game, jint joystick_id, jbooleanArray out_haptic)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_haptic_value = 0;
    CNA_Result call_result = cna.haptics_get_is_joystick_haptic((CNA_Handle)game, (uint32_t)joystick_id, &out_haptic_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_haptic_element = (jboolean)out_haptic_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_haptic, 0, 1, &out_haptic_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsGetIsMouseHaptic(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_haptic)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_haptic_value = 0;
    CNA_Result call_result = cna.haptics_get_is_mouse_haptic((CNA_Handle)game, &out_haptic_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_haptic_element = (jboolean)out_haptic_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_haptic, 0, 1, &out_haptic_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsGetNameSizeAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.haptics_get_name_size_at((CNA_Handle)game, (uint32_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsOpen(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id, jlongArray out_device)
{
    (void)environment;
    (void)declaring_class;
    CNA_HapticDeviceHandle out_device_value = 0;
    CNA_Result call_result = cna.haptics_open((CNA_Handle)game, (uint32_t)id, &out_device_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_device_element = (jlong)out_device_value;
        (*environment)->SetLongArrayRegion(
            environment, out_device, 0, 1, &out_device_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsOpenFromJoystick(
    JNIEnv* environment, jclass declaring_class, jlong game, jint joystick_id, jlongArray out_device)
{
    (void)environment;
    (void)declaring_class;
    CNA_HapticDeviceHandle out_device_value = 0;
    CNA_Result call_result = cna.haptics_open_from_joystick((CNA_Handle)game, (uint32_t)joystick_id, &out_device_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_device_element = (jlong)out_device_value;
        (*environment)->SetLongArrayRegion(
            environment, out_device, 0, 1, &out_device_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_hapticsOpenFromMouse(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_device)
{
    (void)environment;
    (void)declaring_class;
    CNA_HapticDeviceHandle out_device_value = 0;
    CNA_Result call_result = cna.haptics_open_from_mouse((CNA_Handle)game, &out_device_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_device_element = (jlong)out_device_value;
        (*environment)->SetLongArrayRegion(
            environment, out_device, 0, 1, &out_device_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateCopyAxes(
    JNIEnv* environment, jclass declaring_class, jlong state, jshortArray destination, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jshort* destination_elements = (*environment)->GetShortArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    int16_t* destination_values = (int16_t*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseShortArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (int16_t)destination_elements[index];
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.joystick_state_copy_axes((CNA_JoystickStateHandle)state, destination_values, (uint64_t)destination_size, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jshort)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseShortArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateCopyBalls(
    JNIEnv* environment, jclass declaring_class, jlong state, jlongArray destinationIntegral, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_count = (*environment)->GetArrayLength(environment, destinationIntegral) / 2;
    if ((*environment)->GetArrayLength(environment, destinationIntegral) != destination_count * 2) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Point* destination_values = (CNA_Point*)calloc(
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
            destination_values[element].x = (int32_t)destinationIntegral_values[element * 2 + 0];
            destination_values[element].y = (int32_t)destinationIntegral_values[element * 2 + 1];
        }
        free(destinationIntegral_values);
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.joystick_state_copy_balls((CNA_JoystickStateHandle)state, destination_values, (uint64_t)destination_count, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        {
            jlong* destinationIntegral_out = (jlong*)malloc(
                ((size_t)destination_count * 2 + 1U) * sizeof(*destinationIntegral_out));
            if (destinationIntegral_out != NULL) {
                for (jsize element = 0; element < destination_count; ++element) {
                    destinationIntegral_out[element * 2 + 0] = (jlong)destination_values[element].x;
                    destinationIntegral_out[element * 2 + 1] = (jlong)destination_values[element].y;
                }
                (*environment)->SetLongArrayRegion(
                    environment, destinationIntegral, 0,
                    (jsize)(destination_count * 2), destinationIntegral_out);
                free(destinationIntegral_out);
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
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateCopyButtons(
    JNIEnv* environment, jclass declaring_class, jlong state, jbooleanArray destination, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jboolean* destination_elements = (*environment)->GetBooleanArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_Bool* destination_values = (CNA_Bool*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseBooleanArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (CNA_Bool)destination_elements[index];
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.joystick_state_copy_buttons((CNA_JoystickStateHandle)state, destination_values, (uint64_t)destination_size, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS) {
        for (jsize index = 0; index < destination_size; ++index) {
            destination_elements[index] = (jboolean)destination_values[index];
        }
    }
    free(destination_values);
    (*environment)->ReleaseBooleanArrayElements(
        environment, destination, destination_elements, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_count_element = (jlong)out_count_value;
        (*environment)->SetLongArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateCopyHats(
    JNIEnv* environment, jclass declaring_class, jlong state, jintArray destination, jlongArray out_count)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jint* destination_elements = (*environment)->GetIntArrayElements(environment, destination, NULL);
    if (destination_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_JoystickHatPosition* destination_values = (CNA_JoystickHatPosition*)malloc(
        ((size_t)destination_size + 1U) * sizeof(*destination_values));
    if (destination_values == NULL) {
        (*environment)->ReleaseIntArrayElements(
            environment, destination, destination_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < destination_size; ++index) {
        destination_values[index] = (CNA_JoystickHatPosition)destination_elements[index];
    }
    uint64_t out_count_value = 0;
    CNA_Result call_result = cna.joystick_state_copy_hats((CNA_JoystickStateHandle)state, destination_values, (uint64_t)destination_size, &out_count_value);
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
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateDestroy(
    JNIEnv* environment, jclass declaring_class, jlong state)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.joystick_state_destroy((CNA_JoystickStateHandle)state);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateGetAxisCount(
    JNIEnv* environment, jclass declaring_class, jlong state, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.joystick_state_get_axis_count((CNA_JoystickStateHandle)state, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateGetBallCount(
    JNIEnv* environment, jclass declaring_class, jlong state, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.joystick_state_get_ball_count((CNA_JoystickStateHandle)state, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateGetButtonCount(
    JNIEnv* environment, jclass declaring_class, jlong state, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.joystick_state_get_button_count((CNA_JoystickStateHandle)state, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joystickStateGetHatCount(
    JNIEnv* environment, jclass declaring_class, jlong state, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.joystick_state_get_hat_count((CNA_JoystickStateHandle)state, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksCaptureState(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id, jlongArray out_state)
{
    (void)environment;
    (void)declaring_class;
    CNA_JoystickStateHandle out_state_value = 0;
    CNA_Result call_result = cna.joysticks_capture_state((CNA_Handle)game, (uint32_t)id, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_state_element = (jlong)out_state_value;
        (*environment)->SetLongArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksCopyCapabilitiesGuid(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.joysticks_copy_capabilities_guid((CNA_Handle)game, (uint32_t)id, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksCopyCapabilitiesName(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.joysticks_copy_capabilities_name((CNA_Handle)game, (uint32_t)id, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksCopyNameAt(
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
    CNA_Result call_result = cna.joysticks_copy_name_at((CNA_Handle)game, (uint32_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksGetCapabilities(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id, jbyteArray out_capabilitiesBytes, jlongArray out_capabilitiesIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_capabilitiesBytes == NULL || out_capabilitiesIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_JoystickCapabilities out_capabilities_value;
    memset(&out_capabilities_value, 0, sizeof out_capabilities_value);
    out_capabilities_value.struct_size = (uint32_t)(sizeof out_capabilities_value);
    out_capabilities_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.joysticks_get_capabilities((CNA_Handle)game, (uint32_t)id, &out_capabilities_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jbyte out_capabilitiesBytes_values[] = {
            (jbyte)out_capabilities_value.reserved[0],
            (jbyte)out_capabilities_value.reserved[1],
            (jbyte)out_capabilities_value.reserved[2]
        };
        (*environment)->SetByteArrayRegion(environment, out_capabilitiesBytes, 0,
            (jsize)(sizeof out_capabilitiesBytes_values / sizeof out_capabilitiesBytes_values[0]),
            out_capabilitiesBytes_values);
        jlong out_capabilitiesIntegral_values[] = {
            (jlong)out_capabilities_value.axis_count,
            (jlong)out_capabilities_value.button_count,
            (jlong)out_capabilities_value.hat_count,
            (jlong)out_capabilities_value.ball_count,
            (jlong)out_capabilities_value.type,
            (jlong)out_capabilities_value.power_state,
            (jlong)out_capabilities_value.power_percent,
            (jlong)out_capabilities_value.is_connected
        };
        (*environment)->SetLongArrayRegion(environment, out_capabilitiesIntegral, 0,
            (jsize)(sizeof out_capabilitiesIntegral_values / sizeof out_capabilitiesIntegral_values[0]),
            out_capabilitiesIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksGetCapabilitiesGuidSize(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.joysticks_get_capabilities_guid_size((CNA_Handle)game, (uint32_t)id, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksGetCapabilitiesNameSize(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.joysticks_get_capabilities_name_size((CNA_Handle)game, (uint32_t)id, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksGetCount(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.joysticks_get_count((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksGetInfoAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_JoystickInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.joysticks_get_info_at((CNA_Handle)game, (uint32_t)index, &out_info_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_infoIntegral_values[] = {
            (jlong)out_info_value.id,
            (jlong)out_info_value.type
        };
        (*environment)->SetLongArrayRegion(environment, out_infoIntegral, 0,
            (jsize)(sizeof out_infoIntegral_values / sizeof out_infoIntegral_values[0]),
            out_infoIntegral_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksGetNameSizeAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.joysticks_get_name_size_at((CNA_Handle)game, (uint32_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksRaiseConnectedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.joysticks_raise_connected_ext((CNA_Handle)game, (uint32_t)id);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksRaiseDisconnectedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint id)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.joysticks_raise_disconnected_ext((CNA_Handle)game, (uint32_t)id);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_joysticksResetForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.joysticks_reset_for_tests_ext((CNA_Handle)game);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_keyboardCopyKeyNameExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint key, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.keyboard_copy_key_name_ext((CNA_Handle)game, (CNA_Key)key, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_keyboardCopyScancodeNameExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint key, jbyteArray destination, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    jsize destination_size = (*environment)->GetArrayLength(environment, destination);
    jbyte* destination_bytes = (*environment)->GetByteArrayElements(environment, destination, NULL);
    if (destination_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    uint64_t destination_written = 0;
    CNA_Result call_result = cna.keyboard_copy_scancode_name_ext((CNA_Handle)game, (CNA_Key)key, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_keyboardGetKeyFromNameExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray name, jintArray out_key)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Key out_key_value = 0;
    CNA_Result call_result = cna.keyboard_get_key_from_name_ext((CNA_Handle)game, name_view, &out_key_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_key_element = (jint)out_key_value;
        (*environment)->SetIntArrayRegion(
            environment, out_key, 0, 1, &out_key_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_keyboardGetKeyFromScancodeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint scancode, jintArray out_key)
{
    (void)environment;
    (void)declaring_class;
    CNA_Key out_key_value = 0;
    CNA_Result call_result = cna.keyboard_get_key_from_scancode_ext((CNA_Handle)game, (CNA_Key)scancode, &out_key_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_key_element = (jint)out_key_value;
        (*environment)->SetIntArrayRegion(
            environment, out_key, 0, 1, &out_key_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_keyboardGetKeyNameSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint key, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.keyboard_get_key_name_size_ext((CNA_Handle)game, (CNA_Key)key, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_keyboardGetModStateExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_modifiers)
{
    (void)environment;
    (void)declaring_class;
    CNA_KeyModifiers out_modifiers_value = 0;
    CNA_Result call_result = cna.keyboard_get_mod_state_ext((CNA_Handle)game, &out_modifiers_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_modifiers_element = (jint)out_modifiers_value;
        (*environment)->SetIntArrayRegion(
            environment, out_modifiers, 0, 1, &out_modifiers_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_keyboardGetScancodeFromNameExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray name, jintArray out_key)
{
    (void)environment;
    (void)declaring_class;
    jsize name_size = (*environment)->GetArrayLength(environment, name);
    jbyte* name_bytes = (*environment)->GetByteArrayElements(environment, name, NULL);
    if (name_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView name_view = {(const char*)name_bytes, (uint64_t)name_size};
    CNA_Key out_key_value = 0;
    CNA_Result call_result = cna.keyboard_get_scancode_from_name_ext((CNA_Handle)game, name_view, &out_key_value);
    (*environment)->ReleaseByteArrayElements(environment, name, name_bytes, JNI_ABORT);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_key_element = (jint)out_key_value;
        (*environment)->SetIntArrayRegion(
            environment, out_key, 0, 1, &out_key_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_keyboardGetScancodeNameSizeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint key, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.keyboard_get_scancode_name_size_ext((CNA_Handle)game, (CNA_Key)key, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseCursorCreateExt(
    JNIEnv* environment, jclass declaring_class, jlongArray out_cursor)
{
    (void)environment;
    (void)declaring_class;
    CNA_MouseCursorHandle out_cursor_value = 0;
    CNA_Result call_result = cna.mouse_cursor_create_ext(&out_cursor_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_cursor_element = (jlong)out_cursor_value;
        (*environment)->SetLongArrayRegion(
            environment, out_cursor, 0, 1, &out_cursor_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseCursorCreateFromTexture2d(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong texture, jint origin_x, jint origin_y, jlongArray out_cursor)
{
    (void)environment;
    (void)declaring_class;
    CNA_MouseCursorHandle out_cursor_value = 0;
    CNA_Result call_result = cna.mouse_cursor_create_from_texture2d((CNA_Handle)game, (CNA_Handle)texture, (int32_t)origin_x, (int32_t)origin_y, &out_cursor_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_cursor_element = (jlong)out_cursor_value;
        (*environment)->SetLongArrayRegion(
            environment, out_cursor, 0, 1, &out_cursor_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseCursorDestroy(
    JNIEnv* environment, jclass declaring_class, jlong cursor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.mouse_cursor_destroy((CNA_MouseCursorHandle)cursor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseCursorDispose(
    JNIEnv* environment, jclass declaring_class, jlong cursor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.mouse_cursor_dispose((CNA_MouseCursorHandle)cursor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseCursorGetStockExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint stock, jlongArray out_cursor)
{
    (void)environment;
    (void)declaring_class;
    CNA_MouseCursorHandle out_cursor_value = 0;
    CNA_Result call_result = cna.mouse_cursor_get_stock_ext((CNA_Handle)game, (CNA_MouseCursorStock)stock, &out_cursor_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_cursor_element = (jlong)out_cursor_value;
        (*environment)->SetLongArrayRegion(
            environment, out_cursor, 0, 1, &out_cursor_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseGetGlobalPositionExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_x, jintArray out_y)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_x_value = 0;
    int32_t out_y_value = 0;
    CNA_Result call_result = cna.mouse_get_global_position_ext((CNA_Handle)game, &out_x_value, &out_y_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_x_element = (jint)out_x_value;
        (*environment)->SetIntArrayRegion(
            environment, out_x, 0, 1, &out_x_element);
        jint out_y_element = (jint)out_y_value;
        (*environment)->SetIntArrayRegion(
            environment, out_y, 0, 1, &out_y_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseGetIsRelativeMouseModeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_enabled_value = 0;
    CNA_Result call_result = cna.mouse_get_is_relative_mouse_mode_ext((CNA_Handle)game, &out_enabled_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_enabled_element = (jboolean)out_enabled_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_enabled, 0, 1, &out_enabled_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseRaiseClickedExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint button)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.mouse_raise_clicked_ext((CNA_Handle)game, (int32_t)button);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseSetCaptureExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jboolean enabled, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.mouse_set_capture_ext((CNA_Handle)game, (CNA_Bool)enabled, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseSetCursorExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong cursor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.mouse_set_cursor_ext((CNA_Handle)game, (CNA_MouseCursorHandle)cursor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseSetIsRelativeMouseModeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jboolean enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.mouse_set_is_relative_mouse_mode_ext((CNA_Handle)game, (CNA_Bool)enabled);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_mouseWarpGlobalExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint x, jint y, jbooleanArray out_applied)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_applied_value = 0;
    CNA_Result call_result = cna.mouse_warp_global_ext((CNA_Handle)game, (int32_t)x, (int32_t)y, &out_applied_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_applied_element = (jboolean)out_applied_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_applied, 0, 1, &out_applied_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputGetWindowHandleExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_window)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_window_value = 0;
    CNA_Result call_result = cna.text_input_get_window_handle_ext((CNA_Handle)game, &out_window_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_window_element = (jlong)out_window_value;
        (*environment)->SetLongArrayRegion(
            environment, out_window, 0, 1, &out_window_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputIsActiveExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_active)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_active_value = 0;
    CNA_Result call_result = cna.text_input_is_active_ext((CNA_Handle)game, &out_active_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_active_element = (jboolean)out_active_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_active, 0, 1, &out_active_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputIsScreenKeyboardShownExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_shown)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_shown_value = 0;
    CNA_Result call_result = cna.text_input_is_screen_keyboard_shown_ext((CNA_Handle)game, &out_shown_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_shown_element = (jboolean)out_shown_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_shown, 0, 1, &out_shown_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputIsScreenKeyboardShownForWindowExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong window, jbooleanArray out_shown)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_shown_value = 0;
    CNA_Result call_result = cna.text_input_is_screen_keyboard_shown_for_window_ext((CNA_Handle)game, (uint64_t)window, &out_shown_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_shown_element = (jboolean)out_shown_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_shown, 0, 1, &out_shown_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputRaiseTextEditingExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbyteArray text, jint start, jint length)
{
    (void)environment;
    (void)declaring_class;
    jsize text_size = (*environment)->GetArrayLength(environment, text);
    jbyte* text_bytes = (*environment)->GetByteArrayElements(environment, text, NULL);
    if (text_bytes == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_StringView text_view = {(const char*)text_bytes, (uint64_t)text_size};
    CNA_Result call_result = cna.text_input_raise_text_editing_ext((CNA_Handle)game, text_view, (int32_t)start, (int32_t)length);
    (*environment)->ReleaseByteArrayElements(environment, text, text_bytes, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputRaiseTextInputExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint code_unit)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.text_input_raise_text_input_ext((CNA_Handle)game, (uint16_t)code_unit);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputResetForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.text_input_reset_for_tests_ext((CNA_Handle)game);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputSetInputRectangleExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray rectangleIntegral)
{
    (void)environment;
    (void)declaring_class;
    CNA_Rectangle rectangle_value;
    memset(&rectangle_value, 0, sizeof rectangle_value);
    {
        jlong rectangleIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, rectangleIntegral, 0, 4, rectangleIntegral_values);
        rectangle_value.x = (int32_t)rectangleIntegral_values[0];
        rectangle_value.y = (int32_t)rectangleIntegral_values[1];
        rectangle_value.width = (int32_t)rectangleIntegral_values[2];
        rectangle_value.height = (int32_t)rectangleIntegral_values[3];
    }
    CNA_Result call_result = cna.text_input_set_input_rectangle_ext((CNA_Handle)game, rectangle_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputSetWindowHandleExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong window)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.text_input_set_window_handle_ext((CNA_Handle)game, (uint64_t)window);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputStartExt(
    JNIEnv* environment, jclass declaring_class, jlong game)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.text_input_start_ext((CNA_Handle)game);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputStartWithTypeExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint type)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.text_input_start_with_type_ext((CNA_Handle)game, (CNA_TextInputType)type);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_textInputStopExt(
    JNIEnv* environment, jclass declaring_class, jlong game)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.text_input_stop_ext((CNA_Handle)game);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_touchPanelGetMouseTouchEmulationEnabledExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_enabled_value = 0;
    CNA_Result call_result = cna.touch_panel_get_mouse_touch_emulation_enabled_ext((CNA_Handle)game, &out_enabled_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_enabled_element = (jboolean)out_enabled_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_enabled, 0, 1, &out_enabled_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_touchPanelGetTouchDeviceExistsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_exists)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_exists_value = 0;
    CNA_Result call_result = cna.touch_panel_get_touch_device_exists_ext((CNA_Handle)game, &out_exists_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_exists_element = (jboolean)out_exists_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_exists, 0, 1, &out_exists_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeInputExtensionRoutes_touchPanelSetMouseTouchEmulationEnabledExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jboolean enabled)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.touch_panel_set_mouse_touch_emulation_enabled_ext((CNA_Handle)game, (CNA_Bool)enabled);
    return (jint)call_result;
}
