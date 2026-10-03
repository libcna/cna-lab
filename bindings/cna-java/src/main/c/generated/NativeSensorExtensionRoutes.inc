/* SPDX-License-Identifier: MS-PL */
/*
 * Generated JNI adapter for NativeSensorExtensionRoutes.
 *
 * Produced by tools/native-abi/generate_jni.py from the live CNA C headers, and
 * included by cna_java_jni.c so it shares the one dispatch table. Do not edit.
 */

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerCopyLastDispatchExceptionMessageForTestsExt(
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
    CNA_Result call_result = cna.accelerometer_copy_last_dispatch_exception_message_for_tests_ext((CNA_Handle)game, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerCreate(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_AccelerometerHandle out_sensor_value = 0;
    CNA_Result call_result = cna.accelerometer_create((CNA_Handle)game, &out_sensor_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sensor_element = (jlong)out_sensor_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sensor, 0, 1, &out_sensor_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerDestroy(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_destroy((CNA_AccelerometerHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerDispatchToInstancesForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray sensors, jfloat x, jfloat y, jfloat z)
{
    (void)environment;
    (void)declaring_class;
    jsize sensors_size = (*environment)->GetArrayLength(environment, sensors);
    jlong* sensors_elements = (*environment)->GetLongArrayElements(environment, sensors, NULL);
    if (sensors_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_AccelerometerHandle* sensors_values = (CNA_AccelerometerHandle*)malloc(
        ((size_t)sensors_size + 1U) * sizeof(*sensors_values));
    if (sensors_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, sensors, sensors_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < sensors_size; ++index) {
        sensors_values[index] = (CNA_AccelerometerHandle)sensors_elements[index];
    }
    CNA_Result call_result = cna.accelerometer_dispatch_to_instances_for_tests_ext((CNA_Handle)game, sensors_values, (uint64_t)sensors_size, (float)x, (float)y, (float)z);
    free(sensors_values);
    (*environment)->ReleaseLongArrayElements(
        environment, sensors, sensors_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerDispose(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_dispose((CNA_AccelerometerHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerGetCurrentValue(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray out_readingIntegral, jfloatArray out_readingFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_readingIntegral == NULL || out_readingFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_AccelerometerReading out_reading_value;
    memset(&out_reading_value, 0, sizeof out_reading_value);
    out_reading_value.struct_size = (uint32_t)(sizeof out_reading_value);
    out_reading_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.accelerometer_get_current_value((CNA_AccelerometerHandle)sensor, &out_reading_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_readingIntegral_values[] = {
            (jlong)out_reading_value.timestamp.ticks,
            (jlong)out_reading_value.timestamp.offset_ticks
        };
        (*environment)->SetLongArrayRegion(environment, out_readingIntegral, 0,
            (jsize)(sizeof out_readingIntegral_values / sizeof out_readingIntegral_values[0]),
            out_readingIntegral_values);
        jfloat out_readingFloating_values[] = {
            (jfloat)out_reading_value.acceleration.x,
            (jfloat)out_reading_value.acceleration.y,
            (jfloat)out_reading_value.acceleration.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_readingFloating, 0,
            (jsize)(sizeof out_readingFloating_values / sizeof out_readingFloating_values[0]),
            out_readingFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerGetDispatchExceptionCountForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.accelerometer_get_dispatch_exception_count_for_tests_ext((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerGetIsDataValid(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jbooleanArray out_valid)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_valid_value = 0;
    CNA_Result call_result = cna.accelerometer_get_is_data_valid((CNA_AccelerometerHandle)sensor, &out_valid_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_valid_element = (jboolean)out_valid_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_valid, 0, 1, &out_valid_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerGetIsSupported(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.accelerometer_get_is_supported((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerGetLastDispatchExceptionMessageSizeForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.accelerometer_get_last_dispatch_exception_message_size_for_tests_ext((CNA_Handle)game, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerGetState(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jintArray out_state)
{
    (void)environment;
    (void)declaring_class;
    CNA_SensorState out_state_value = 0;
    CNA_Result call_result = cna.accelerometer_get_state((CNA_AccelerometerHandle)sensor, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerGetSubsystemHeldForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jbooleanArray out_held)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_held_value = 0;
    CNA_Result call_result = cna.accelerometer_get_subsystem_held_for_tests_ext((CNA_AccelerometerHandle)sensor, &out_held_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_held_element = (jboolean)out_held_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_held, 0, 1, &out_held_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerGetTimeBetweenUpdatesTicks(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray out_ticks)
{
    (void)environment;
    (void)declaring_class;
    int64_t out_ticks_value = 0;
    CNA_Result call_result = cna.accelerometer_get_time_between_updates_ticks((CNA_AccelerometerHandle)sensor, &out_ticks_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_ticks_element = (jlong)out_ticks_value;
        (*environment)->SetLongArrayRegion(
            environment, out_ticks, 0, 1, &out_ticks_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerInjectSyntheticUpdateExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jfloat x, jfloat y, jfloat z)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_inject_synthetic_update_ext((CNA_AccelerometerHandle)sensor, (float)x, (float)y, (float)z);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerIsSensorConnectedForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong sensor_id, jbooleanArray out_connected)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_connected_value = 0;
    CNA_Result call_result = cna.accelerometer_is_sensor_connected_for_tests_ext((CNA_Handle)game, (int64_t)sensor_id, &out_connected_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_connected_element = (jboolean)out_connected_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_connected, 0, 1, &out_connected_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerRegisterStartedInstanceForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_register_started_instance_for_tests_ext((CNA_AccelerometerHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerSetEventWatchRegistrationFailureForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jboolean should_fail)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_set_event_watch_registration_failure_for_tests_ext((CNA_Handle)game, (CNA_Bool)should_fail);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerSetStartedForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jboolean started)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_set_started_for_tests_ext((CNA_AccelerometerHandle)sensor, (CNA_Bool)started);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerSetSupportedForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jboolean supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_set_supported_for_tests_ext((CNA_AccelerometerHandle)sensor, (CNA_Bool)supported);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerSetTimeBetweenUpdatesTicks(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlong ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_set_time_between_updates_ticks((CNA_AccelerometerHandle)sensor, (int64_t)ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerStart(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_start((CNA_AccelerometerHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerStop(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_stop((CNA_AccelerometerHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_accelerometerUnregisterStartedInstanceForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.accelerometer_unregister_started_instance_for_tests_ext((CNA_AccelerometerHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassCreate(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_CompassHandle out_sensor_value = 0;
    CNA_Result call_result = cna.compass_create((CNA_Handle)game, &out_sensor_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sensor_element = (jlong)out_sensor_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sensor, 0, 1, &out_sensor_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassDestroy(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.compass_destroy((CNA_CompassHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassDispose(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.compass_dispose((CNA_CompassHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassGetCurrentValue(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray out_readingIntegral, jfloatArray out_readingFloating, jdoubleArray out_readingDoubles)
{
    (void)environment;
    (void)declaring_class;
    if (out_readingIntegral == NULL || out_readingFloating == NULL || out_readingDoubles == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CompassReading out_reading_value;
    memset(&out_reading_value, 0, sizeof out_reading_value);
    out_reading_value.struct_size = (uint32_t)(sizeof out_reading_value);
    out_reading_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.compass_get_current_value((CNA_CompassHandle)sensor, &out_reading_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_readingIntegral_values[] = {
            (jlong)out_reading_value.timestamp.ticks,
            (jlong)out_reading_value.timestamp.offset_ticks
        };
        (*environment)->SetLongArrayRegion(environment, out_readingIntegral, 0,
            (jsize)(sizeof out_readingIntegral_values / sizeof out_readingIntegral_values[0]),
            out_readingIntegral_values);
        jfloat out_readingFloating_values[] = {
            (jfloat)out_reading_value.magnetometer_reading.x,
            (jfloat)out_reading_value.magnetometer_reading.y,
            (jfloat)out_reading_value.magnetometer_reading.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_readingFloating, 0,
            (jsize)(sizeof out_readingFloating_values / sizeof out_readingFloating_values[0]),
            out_readingFloating_values);
        jdouble out_readingDoubles_values[] = {
            (jdouble)out_reading_value.heading_accuracy,
            (jdouble)out_reading_value.magnetic_heading,
            (jdouble)out_reading_value.true_heading
        };
        (*environment)->SetDoubleArrayRegion(environment, out_readingDoubles, 0,
            (jsize)(sizeof out_readingDoubles_values / sizeof out_readingDoubles_values[0]),
            out_readingDoubles_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassGetIsDataValid(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jbooleanArray out_valid)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_valid_value = 0;
    CNA_Result call_result = cna.compass_get_is_data_valid((CNA_CompassHandle)sensor, &out_valid_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_valid_element = (jboolean)out_valid_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_valid, 0, 1, &out_valid_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassGetIsSupported(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.compass_get_is_supported((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassGetState(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jintArray out_state)
{
    (void)environment;
    (void)declaring_class;
    CNA_SensorState out_state_value = 0;
    CNA_Result call_result = cna.compass_get_state((CNA_CompassHandle)sensor, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassGetTimeBetweenUpdatesTicks(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray out_ticks)
{
    (void)environment;
    (void)declaring_class;
    int64_t out_ticks_value = 0;
    CNA_Result call_result = cna.compass_get_time_between_updates_ticks((CNA_CompassHandle)sensor, &out_ticks_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_ticks_element = (jlong)out_ticks_value;
        (*environment)->SetLongArrayRegion(
            environment, out_ticks, 0, 1, &out_ticks_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassInjectCalibrationRequestExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.compass_inject_calibration_request_ext((CNA_CompassHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassInjectSyntheticUpdateExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray readingIntegral, jfloatArray readingFloating, jdoubleArray readingDoubles)
{
    (void)environment;
    (void)declaring_class;
    if (readingIntegral == NULL || readingFloating == NULL || readingDoubles == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_CompassReading reading_value;
    memset(&reading_value, 0, sizeof reading_value);
    reading_value.struct_size = (uint32_t)(sizeof reading_value);
    reading_value.struct_version = UINT32_C(1);
    {
        jlong readingIntegral_values[2];
        (*environment)->GetLongArrayRegion(environment, readingIntegral, 0, 2, readingIntegral_values);
        reading_value.timestamp.ticks = (int64_t)readingIntegral_values[0];
        reading_value.timestamp.offset_ticks = (int64_t)readingIntegral_values[1];
    }
    {
        jfloat readingFloating_values[3];
        (*environment)->GetFloatArrayRegion(environment, readingFloating, 0, 3, readingFloating_values);
        reading_value.magnetometer_reading.x = (float)readingFloating_values[0];
        reading_value.magnetometer_reading.y = (float)readingFloating_values[1];
        reading_value.magnetometer_reading.z = (float)readingFloating_values[2];
    }
    {
        jdouble readingDoubles_values[3];
        (*environment)->GetDoubleArrayRegion(environment, readingDoubles, 0, 3, readingDoubles_values);
        reading_value.heading_accuracy = (double)readingDoubles_values[0];
        reading_value.magnetic_heading = (double)readingDoubles_values[1];
        reading_value.true_heading = (double)readingDoubles_values[2];
    }
    CNA_Result call_result = cna.compass_inject_synthetic_update_ext((CNA_CompassHandle)sensor, &reading_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassSetTestBackendExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jboolean installed, jboolean supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.compass_set_test_backend_ext((CNA_CompassHandle)sensor, (CNA_Bool)installed, (CNA_Bool)supported);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassSetTimeBetweenUpdatesTicks(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlong ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.compass_set_time_between_updates_ticks((CNA_CompassHandle)sensor, (int64_t)ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassStart(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.compass_start((CNA_CompassHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_compassStop(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.compass_stop((CNA_CompassHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeCopyLastDispatchExceptionMessageForTestsExt(
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
    CNA_Result call_result = cna.gyroscope_copy_last_dispatch_exception_message_for_tests_ext((CNA_Handle)game, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeCreate(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_GyroscopeHandle out_sensor_value = 0;
    CNA_Result call_result = cna.gyroscope_create((CNA_Handle)game, &out_sensor_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sensor_element = (jlong)out_sensor_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sensor, 0, 1, &out_sensor_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeDestroy(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_destroy((CNA_GyroscopeHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeDispatchToInstancesForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray sensors, jfloat x, jfloat y, jfloat z)
{
    (void)environment;
    (void)declaring_class;
    jsize sensors_size = (*environment)->GetArrayLength(environment, sensors);
    jlong* sensors_elements = (*environment)->GetLongArrayElements(environment, sensors, NULL);
    if (sensors_elements == NULL) {
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    CNA_GyroscopeHandle* sensors_values = (CNA_GyroscopeHandle*)malloc(
        ((size_t)sensors_size + 1U) * sizeof(*sensors_values));
    if (sensors_values == NULL) {
        (*environment)->ReleaseLongArrayElements(
            environment, sensors, sensors_elements, JNI_ABORT);
        return (jint)CNA_RESULT_OUT_OF_MEMORY;
    }
    for (jsize index = 0; index < sensors_size; ++index) {
        sensors_values[index] = (CNA_GyroscopeHandle)sensors_elements[index];
    }
    CNA_Result call_result = cna.gyroscope_dispatch_to_instances_for_tests_ext((CNA_Handle)game, sensors_values, (uint64_t)sensors_size, (float)x, (float)y, (float)z);
    free(sensors_values);
    (*environment)->ReleaseLongArrayElements(
        environment, sensors, sensors_elements, JNI_ABORT);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeDispose(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_dispose((CNA_GyroscopeHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeGetCurrentValue(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray out_readingIntegral, jfloatArray out_readingFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_readingIntegral == NULL || out_readingFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_GyroscopeReading out_reading_value;
    memset(&out_reading_value, 0, sizeof out_reading_value);
    out_reading_value.struct_size = (uint32_t)(sizeof out_reading_value);
    out_reading_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.gyroscope_get_current_value((CNA_GyroscopeHandle)sensor, &out_reading_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_readingIntegral_values[] = {
            (jlong)out_reading_value.timestamp.ticks,
            (jlong)out_reading_value.timestamp.offset_ticks
        };
        (*environment)->SetLongArrayRegion(environment, out_readingIntegral, 0,
            (jsize)(sizeof out_readingIntegral_values / sizeof out_readingIntegral_values[0]),
            out_readingIntegral_values);
        jfloat out_readingFloating_values[] = {
            (jfloat)out_reading_value.rotation_rate.x,
            (jfloat)out_reading_value.rotation_rate.y,
            (jfloat)out_reading_value.rotation_rate.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_readingFloating, 0,
            (jsize)(sizeof out_readingFloating_values / sizeof out_readingFloating_values[0]),
            out_readingFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeGetDispatchExceptionCountForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_count_value = 0;
    CNA_Result call_result = cna.gyroscope_get_dispatch_exception_count_for_tests_ext((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeGetIsDataValid(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jbooleanArray out_valid)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_valid_value = 0;
    CNA_Result call_result = cna.gyroscope_get_is_data_valid((CNA_GyroscopeHandle)sensor, &out_valid_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_valid_element = (jboolean)out_valid_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_valid, 0, 1, &out_valid_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeGetIsSupported(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.gyroscope_get_is_supported((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeGetLastDispatchExceptionMessageSizeForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.gyroscope_get_last_dispatch_exception_message_size_for_tests_ext((CNA_Handle)game, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeGetState(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jintArray out_state)
{
    (void)environment;
    (void)declaring_class;
    CNA_SensorState out_state_value = 0;
    CNA_Result call_result = cna.gyroscope_get_state((CNA_GyroscopeHandle)sensor, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeGetSubsystemHeldForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jbooleanArray out_held)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_held_value = 0;
    CNA_Result call_result = cna.gyroscope_get_subsystem_held_for_tests_ext((CNA_GyroscopeHandle)sensor, &out_held_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_held_element = (jboolean)out_held_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_held, 0, 1, &out_held_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeGetTimeBetweenUpdatesTicks(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray out_ticks)
{
    (void)environment;
    (void)declaring_class;
    int64_t out_ticks_value = 0;
    CNA_Result call_result = cna.gyroscope_get_time_between_updates_ticks((CNA_GyroscopeHandle)sensor, &out_ticks_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_ticks_element = (jlong)out_ticks_value;
        (*environment)->SetLongArrayRegion(
            environment, out_ticks, 0, 1, &out_ticks_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeInjectSyntheticUpdateExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jfloat x, jfloat y, jfloat z)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_inject_synthetic_update_ext((CNA_GyroscopeHandle)sensor, (float)x, (float)y, (float)z);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeIsSensorConnectedForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jlong sensor_id, jbooleanArray out_connected)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_connected_value = 0;
    CNA_Result call_result = cna.gyroscope_is_sensor_connected_for_tests_ext((CNA_Handle)game, (int64_t)sensor_id, &out_connected_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_connected_element = (jboolean)out_connected_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_connected, 0, 1, &out_connected_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeRegisterStartedInstanceForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_register_started_instance_for_tests_ext((CNA_GyroscopeHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeSetEventWatchRegistrationFailureForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong game, jboolean should_fail)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_set_event_watch_registration_failure_for_tests_ext((CNA_Handle)game, (CNA_Bool)should_fail);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeSetStartedForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jboolean started)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_set_started_for_tests_ext((CNA_GyroscopeHandle)sensor, (CNA_Bool)started);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeSetSupportedForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jboolean supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_set_supported_for_tests_ext((CNA_GyroscopeHandle)sensor, (CNA_Bool)supported);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeSetTimeBetweenUpdatesTicks(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlong ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_set_time_between_updates_ticks((CNA_GyroscopeHandle)sensor, (int64_t)ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeStart(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_start((CNA_GyroscopeHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeStop(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_stop((CNA_GyroscopeHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_gyroscopeUnregisterStartedInstanceForTestsExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.gyroscope_unregister_started_instance_for_tests_ext((CNA_GyroscopeHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionCreate(
    JNIEnv* environment, jclass declaring_class, jlong game, jlongArray out_sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_MotionHandle out_sensor_value = 0;
    CNA_Result call_result = cna.motion_create((CNA_Handle)game, &out_sensor_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_sensor_element = (jlong)out_sensor_value;
        (*environment)->SetLongArrayRegion(
            environment, out_sensor, 0, 1, &out_sensor_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionDestroy(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.motion_destroy((CNA_MotionHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionDispose(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.motion_dispose((CNA_MotionHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionGetCurrentValue(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray out_readingIntegral, jfloatArray out_readingFloating)
{
    (void)environment;
    (void)declaring_class;
    if (out_readingIntegral == NULL || out_readingFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_MotionReading out_reading_value;
    memset(&out_reading_value, 0, sizeof out_reading_value);
    out_reading_value.struct_size = (uint32_t)(sizeof out_reading_value);
    out_reading_value.struct_version = UINT32_C(1);
    out_reading_value.attitude.struct_size = (uint32_t)(sizeof out_reading_value.attitude);
    out_reading_value.attitude.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.motion_get_current_value((CNA_MotionHandle)sensor, &out_reading_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_readingIntegral_values[] = {
            (jlong)out_reading_value.timestamp.ticks,
            (jlong)out_reading_value.timestamp.offset_ticks,
            (jlong)out_reading_value.attitude.timestamp.ticks,
            (jlong)out_reading_value.attitude.timestamp.offset_ticks
        };
        (*environment)->SetLongArrayRegion(environment, out_readingIntegral, 0,
            (jsize)(sizeof out_readingIntegral_values / sizeof out_readingIntegral_values[0]),
            out_readingIntegral_values);
        jfloat out_readingFloating_values[] = {
            (jfloat)out_reading_value.attitude.pitch,
            (jfloat)out_reading_value.attitude.roll,
            (jfloat)out_reading_value.attitude.yaw,
            (jfloat)out_reading_value.attitude.quaternion.x,
            (jfloat)out_reading_value.attitude.quaternion.y,
            (jfloat)out_reading_value.attitude.quaternion.z,
            (jfloat)out_reading_value.attitude.quaternion.w,
            (jfloat)out_reading_value.attitude.rotation_matrix.m11,
            (jfloat)out_reading_value.attitude.rotation_matrix.m12,
            (jfloat)out_reading_value.attitude.rotation_matrix.m13,
            (jfloat)out_reading_value.attitude.rotation_matrix.m14,
            (jfloat)out_reading_value.attitude.rotation_matrix.m21,
            (jfloat)out_reading_value.attitude.rotation_matrix.m22,
            (jfloat)out_reading_value.attitude.rotation_matrix.m23,
            (jfloat)out_reading_value.attitude.rotation_matrix.m24,
            (jfloat)out_reading_value.attitude.rotation_matrix.m31,
            (jfloat)out_reading_value.attitude.rotation_matrix.m32,
            (jfloat)out_reading_value.attitude.rotation_matrix.m33,
            (jfloat)out_reading_value.attitude.rotation_matrix.m34,
            (jfloat)out_reading_value.attitude.rotation_matrix.m41,
            (jfloat)out_reading_value.attitude.rotation_matrix.m42,
            (jfloat)out_reading_value.attitude.rotation_matrix.m43,
            (jfloat)out_reading_value.attitude.rotation_matrix.m44,
            (jfloat)out_reading_value.device_acceleration.x,
            (jfloat)out_reading_value.device_acceleration.y,
            (jfloat)out_reading_value.device_acceleration.z,
            (jfloat)out_reading_value.device_rotation_rate.x,
            (jfloat)out_reading_value.device_rotation_rate.y,
            (jfloat)out_reading_value.device_rotation_rate.z,
            (jfloat)out_reading_value.gravity.x,
            (jfloat)out_reading_value.gravity.y,
            (jfloat)out_reading_value.gravity.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_readingFloating, 0,
            (jsize)(sizeof out_readingFloating_values / sizeof out_readingFloating_values[0]),
            out_readingFloating_values);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionGetIsAttitudeNorthReferencedExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jbooleanArray out_north_referenced)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_north_referenced_value = 0;
    CNA_Result call_result = cna.motion_get_is_attitude_north_referenced_ext((CNA_MotionHandle)sensor, &out_north_referenced_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_north_referenced_element = (jboolean)out_north_referenced_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_north_referenced, 0, 1, &out_north_referenced_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionGetIsDataValid(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jbooleanArray out_valid)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_valid_value = 0;
    CNA_Result call_result = cna.motion_get_is_data_valid((CNA_MotionHandle)sensor, &out_valid_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_valid_element = (jboolean)out_valid_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_valid, 0, 1, &out_valid_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionGetIsSupported(
    JNIEnv* environment, jclass declaring_class, jlong game, jbooleanArray out_supported)
{
    (void)environment;
    (void)declaring_class;
    CNA_Bool out_supported_value = 0;
    CNA_Result call_result = cna.motion_get_is_supported((CNA_Handle)game, &out_supported_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jboolean out_supported_element = (jboolean)out_supported_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_supported, 0, 1, &out_supported_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionGetState(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jintArray out_state)
{
    (void)environment;
    (void)declaring_class;
    CNA_SensorState out_state_value = 0;
    CNA_Result call_result = cna.motion_get_state((CNA_MotionHandle)sensor, &out_state_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_state_element = (jint)out_state_value;
        (*environment)->SetIntArrayRegion(
            environment, out_state, 0, 1, &out_state_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionGetTimeBetweenUpdatesTicks(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray out_ticks)
{
    (void)environment;
    (void)declaring_class;
    int64_t out_ticks_value = 0;
    CNA_Result call_result = cna.motion_get_time_between_updates_ticks((CNA_MotionHandle)sensor, &out_ticks_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_ticks_element = (jlong)out_ticks_value;
        (*environment)->SetLongArrayRegion(
            environment, out_ticks, 0, 1, &out_ticks_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionInjectCalibrationRequestExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.motion_inject_calibration_request_ext((CNA_MotionHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionInjectSyntheticUpdateExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlongArray readingIntegral, jfloatArray readingFloating)
{
    (void)environment;
    (void)declaring_class;
    if (readingIntegral == NULL || readingFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_MotionReading reading_value;
    memset(&reading_value, 0, sizeof reading_value);
    reading_value.struct_size = (uint32_t)(sizeof reading_value);
    reading_value.struct_version = UINT32_C(1);
    reading_value.attitude.struct_size = (uint32_t)(sizeof reading_value.attitude);
    reading_value.attitude.struct_version = UINT32_C(1);
    {
        jlong readingIntegral_values[4];
        (*environment)->GetLongArrayRegion(environment, readingIntegral, 0, 4, readingIntegral_values);
        reading_value.timestamp.ticks = (int64_t)readingIntegral_values[0];
        reading_value.timestamp.offset_ticks = (int64_t)readingIntegral_values[1];
        reading_value.attitude.timestamp.ticks = (int64_t)readingIntegral_values[2];
        reading_value.attitude.timestamp.offset_ticks = (int64_t)readingIntegral_values[3];
    }
    {
        jfloat readingFloating_values[32];
        (*environment)->GetFloatArrayRegion(environment, readingFloating, 0, 32, readingFloating_values);
        reading_value.attitude.pitch = (float)readingFloating_values[0];
        reading_value.attitude.roll = (float)readingFloating_values[1];
        reading_value.attitude.yaw = (float)readingFloating_values[2];
        reading_value.attitude.quaternion.x = (float)readingFloating_values[3];
        reading_value.attitude.quaternion.y = (float)readingFloating_values[4];
        reading_value.attitude.quaternion.z = (float)readingFloating_values[5];
        reading_value.attitude.quaternion.w = (float)readingFloating_values[6];
        reading_value.attitude.rotation_matrix.m11 = (float)readingFloating_values[7];
        reading_value.attitude.rotation_matrix.m12 = (float)readingFloating_values[8];
        reading_value.attitude.rotation_matrix.m13 = (float)readingFloating_values[9];
        reading_value.attitude.rotation_matrix.m14 = (float)readingFloating_values[10];
        reading_value.attitude.rotation_matrix.m21 = (float)readingFloating_values[11];
        reading_value.attitude.rotation_matrix.m22 = (float)readingFloating_values[12];
        reading_value.attitude.rotation_matrix.m23 = (float)readingFloating_values[13];
        reading_value.attitude.rotation_matrix.m24 = (float)readingFloating_values[14];
        reading_value.attitude.rotation_matrix.m31 = (float)readingFloating_values[15];
        reading_value.attitude.rotation_matrix.m32 = (float)readingFloating_values[16];
        reading_value.attitude.rotation_matrix.m33 = (float)readingFloating_values[17];
        reading_value.attitude.rotation_matrix.m34 = (float)readingFloating_values[18];
        reading_value.attitude.rotation_matrix.m41 = (float)readingFloating_values[19];
        reading_value.attitude.rotation_matrix.m42 = (float)readingFloating_values[20];
        reading_value.attitude.rotation_matrix.m43 = (float)readingFloating_values[21];
        reading_value.attitude.rotation_matrix.m44 = (float)readingFloating_values[22];
        reading_value.device_acceleration.x = (float)readingFloating_values[23];
        reading_value.device_acceleration.y = (float)readingFloating_values[24];
        reading_value.device_acceleration.z = (float)readingFloating_values[25];
        reading_value.device_rotation_rate.x = (float)readingFloating_values[26];
        reading_value.device_rotation_rate.y = (float)readingFloating_values[27];
        reading_value.device_rotation_rate.z = (float)readingFloating_values[28];
        reading_value.gravity.x = (float)readingFloating_values[29];
        reading_value.gravity.y = (float)readingFloating_values[30];
        reading_value.gravity.z = (float)readingFloating_values[31];
    }
    CNA_Result call_result = cna.motion_inject_synthetic_update_ext((CNA_MotionHandle)sensor, &reading_value);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionSetTestBackendExt(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jboolean installed, jboolean supported, jboolean north_referenced)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.motion_set_test_backend_ext((CNA_MotionHandle)sensor, (CNA_Bool)installed, (CNA_Bool)supported, (CNA_Bool)north_referenced);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionSetTimeBetweenUpdatesTicks(
    JNIEnv* environment, jclass declaring_class, jlong sensor, jlong ticks)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.motion_set_time_between_updates_ticks((CNA_MotionHandle)sensor, (int64_t)ticks);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionStart(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.motion_start((CNA_MotionHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_motionStop(
    JNIEnv* environment, jclass declaring_class, jlong sensor)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.motion_stop((CNA_MotionHandle)sensor);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_sensorUnsubscribeExt(
    JNIEnv* environment, jclass declaring_class, jlong registration)
{
    (void)environment;
    (void)declaring_class;
    CNA_Result call_result = cna.sensor_unsubscribe_ext((CNA_SensorEventRegistrationHandle)registration);
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_sensorsCopyNameAt(
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
    CNA_Result call_result = cna.sensors_copy_name_at((CNA_Handle)game, (uint32_t)index, (char*)destination_bytes, (uint64_t)destination_size, &destination_written);
    (*environment)->ReleaseByteArrayElements(environment, destination, destination_bytes, 0);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong destination_element = (jlong)destination_written;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &destination_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_sensorsGetAccelerometer(
    JNIEnv* environment, jclass declaring_class, jlong game, jfloatArray out_accelerationFloating, jbooleanArray out_available)
{
    (void)environment;
    (void)declaring_class;
    if (out_accelerationFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 out_acceleration_value;
    memset(&out_acceleration_value, 0, sizeof out_acceleration_value);
    CNA_Bool out_available_value = 0;
    CNA_Result call_result = cna.sensors_get_accelerometer((CNA_Handle)game, &out_acceleration_value, &out_available_value);
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
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_sensorsGetCount(
    JNIEnv* environment, jclass declaring_class, jlong game, jintArray out_count)
{
    (void)environment;
    (void)declaring_class;
    uint32_t out_count_value = 0;
    CNA_Result call_result = cna.sensors_get_count((CNA_Handle)game, &out_count_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_count_element = (jint)out_count_value;
        (*environment)->SetIntArrayRegion(
            environment, out_count, 0, 1, &out_count_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_sensorsGetGyroscope(
    JNIEnv* environment, jclass declaring_class, jlong game, jfloatArray out_angular_velocityFloating, jbooleanArray out_available)
{
    (void)environment;
    (void)declaring_class;
    if (out_angular_velocityFloating == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_Vector3 out_angular_velocity_value;
    memset(&out_angular_velocity_value, 0, sizeof out_angular_velocity_value);
    CNA_Bool out_available_value = 0;
    CNA_Result call_result = cna.sensors_get_gyroscope((CNA_Handle)game, &out_angular_velocity_value, &out_available_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jfloat out_angular_velocityFloating_values[] = {
            (jfloat)out_angular_velocity_value.x,
            (jfloat)out_angular_velocity_value.y,
            (jfloat)out_angular_velocity_value.z
        };
        (*environment)->SetFloatArrayRegion(environment, out_angular_velocityFloating, 0,
            (jsize)(sizeof out_angular_velocityFloating_values / sizeof out_angular_velocityFloating_values[0]),
            out_angular_velocityFloating_values);
        jboolean out_available_element = (jboolean)out_available_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_available, 0, 1, &out_available_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_sensorsGetInfoAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_infoIntegral)
{
    (void)environment;
    (void)declaring_class;
    if (out_infoIntegral == NULL) {
        return (jint)CNA_RESULT_INVALID_ARGUMENT;
    }
    CNA_SensorInfo out_info_value;
    memset(&out_info_value, 0, sizeof out_info_value);
    out_info_value.struct_size = (uint32_t)(sizeof out_info_value);
    out_info_value.struct_version = UINT32_C(1);
    CNA_Result call_result = cna.sensors_get_info_at((CNA_Handle)game, (uint32_t)index, &out_info_value);
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
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_sensorsGetLastErrorIdExt(
    JNIEnv* environment, jclass declaring_class, jintArray out_error_id, jbooleanArray out_has_error_id)
{
    (void)environment;
    (void)declaring_class;
    int32_t out_error_id_value = 0;
    CNA_Bool out_has_error_id_value = 0;
    CNA_Result call_result = cna.sensors_get_last_error_id_ext(&out_error_id_value, &out_has_error_id_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jint out_error_id_element = (jint)out_error_id_value;
        (*environment)->SetIntArrayRegion(
            environment, out_error_id, 0, 1, &out_error_id_element);
        jboolean out_has_error_id_element = (jboolean)out_has_error_id_value;
        (*environment)->SetBooleanArrayRegion(
            environment, out_has_error_id, 0, 1, &out_has_error_id_element);
    }
    return (jint)call_result;
}

JNIEXPORT jint JNICALL
Java_org_openeggbert_cna_internal_generated_NativeSensorExtensionRoutes_sensorsGetNameSizeAt(
    JNIEnv* environment, jclass declaring_class, jlong game, jint index, jlongArray out_bytes)
{
    (void)environment;
    (void)declaring_class;
    uint64_t out_bytes_value = 0;
    CNA_Result call_result = cna.sensors_get_name_size_at((CNA_Handle)game, (uint32_t)index, &out_bytes_value);
    if (call_result == CNA_RESULT_SUCCESS || call_result == CNA_RESULT_BUFFER_TOO_SMALL) {
        jlong out_bytes_element = (jlong)out_bytes_value;
        (*environment)->SetLongArrayRegion(
            environment, out_bytes, 0, 1, &out_bytes_element);
    }
    return (jint)call_result;
}
