package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeSensorExtensionRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeSensorExtensionRoutes {

    private NativeSensorExtensionRoutes() {
    }

    /**
     * cna_accelerometer_copy_last_dispatch_exception_message_for_tests_ext (sensors.h).
     */
    public static native int accelerometerCopyLastDispatchExceptionMessageForTestsExt(long game, byte[] destination, long[] outBytes);

    /**
     * cna_accelerometer_create (sensors.h).
     */
    public static native int accelerometerCreate(long game, long[] outSensor);

    /**
     * cna_accelerometer_destroy (sensors.h).
     */
    public static native int accelerometerDestroy(long sensor);

    /**
     * cna_accelerometer_dispatch_to_instances_for_tests_ext (sensors.h).
     */
    public static native int accelerometerDispatchToInstancesForTestsExt(long game, long[] sensors, float x, float y, float z);

    /**
     * cna_accelerometer_dispose (sensors.h).
     */
    public static native int accelerometerDispose(long sensor);

    /**
     * cna_accelerometer_get_current_value (sensors.h).
     *
     * <p>outReadingIntegral carries CNA_AccelerometerReading in this order:
     * <ol start="0">
     *   <li>{@code timestamp.ticks} (int64_t)</li>
     *   <li>{@code timestamp.offset_ticks} (int64_t)</li>
     * </ol>
     *
     * <p>outReadingFloating carries CNA_AccelerometerReading in this order:
     * <ol start="0">
     *   <li>{@code acceleration.x} (float)</li>
     *   <li>{@code acceleration.y} (float)</li>
     *   <li>{@code acceleration.z} (float)</li>
     * </ol>
     */
    public static native int accelerometerGetCurrentValue(long sensor, long[] outReadingIntegral, float[] outReadingFloating);

    /**
     * cna_accelerometer_get_dispatch_exception_count_for_tests_ext (sensors.h).
     */
    public static native int accelerometerGetDispatchExceptionCountForTestsExt(long game, int[] outCount);

    /**
     * cna_accelerometer_get_is_data_valid (sensors.h).
     */
    public static native int accelerometerGetIsDataValid(long sensor, boolean[] outValid);

    /**
     * cna_accelerometer_get_is_supported (sensors.h).
     */
    public static native int accelerometerGetIsSupported(long game, boolean[] outSupported);

    /**
     * cna_accelerometer_get_last_dispatch_exception_message_size_for_tests_ext (sensors.h).
     */
    public static native int accelerometerGetLastDispatchExceptionMessageSizeForTestsExt(long game, long[] outBytes);

    /**
     * cna_accelerometer_get_state (sensors.h).
     */
    public static native int accelerometerGetState(long sensor, int[] outState);

    /**
     * cna_accelerometer_get_subsystem_held_for_tests_ext (sensors.h).
     */
    public static native int accelerometerGetSubsystemHeldForTestsExt(long sensor, boolean[] outHeld);

    /**
     * cna_accelerometer_get_time_between_updates_ticks (sensors.h).
     */
    public static native int accelerometerGetTimeBetweenUpdatesTicks(long sensor, long[] outTicks);

    /**
     * cna_accelerometer_inject_synthetic_update_ext (sensors.h).
     */
    public static native int accelerometerInjectSyntheticUpdateExt(long sensor, float x, float y, float z);

    /**
     * cna_accelerometer_is_sensor_connected_for_tests_ext (sensors.h).
     */
    public static native int accelerometerIsSensorConnectedForTestsExt(long game, long sensorId, boolean[] outConnected);

    /**
     * cna_accelerometer_register_started_instance_for_tests_ext (sensors.h).
     */
    public static native int accelerometerRegisterStartedInstanceForTestsExt(long sensor);

    /**
     * cna_accelerometer_set_event_watch_registration_failure_for_tests_ext (sensors.h).
     */
    public static native int accelerometerSetEventWatchRegistrationFailureForTestsExt(long game, boolean shouldFail);

    /**
     * cna_accelerometer_set_started_for_tests_ext (sensors.h).
     */
    public static native int accelerometerSetStartedForTestsExt(long sensor, boolean started);

    /**
     * cna_accelerometer_set_supported_for_tests_ext (sensors.h).
     */
    public static native int accelerometerSetSupportedForTestsExt(long sensor, boolean supported);

    /**
     * cna_accelerometer_set_time_between_updates_ticks (sensors.h).
     */
    public static native int accelerometerSetTimeBetweenUpdatesTicks(long sensor, long ticks);

    /**
     * cna_accelerometer_start (sensors.h).
     */
    public static native int accelerometerStart(long sensor);

    /**
     * cna_accelerometer_stop (sensors.h).
     */
    public static native int accelerometerStop(long sensor);

    /**
     * cna_accelerometer_unregister_started_instance_for_tests_ext (sensors.h).
     */
    public static native int accelerometerUnregisterStartedInstanceForTestsExt(long sensor);

    /**
     * cna_compass_create (sensors.h).
     */
    public static native int compassCreate(long game, long[] outSensor);

    /**
     * cna_compass_destroy (sensors.h).
     */
    public static native int compassDestroy(long sensor);

    /**
     * cna_compass_dispose (sensors.h).
     */
    public static native int compassDispose(long sensor);

    /**
     * cna_compass_get_current_value (sensors.h).
     *
     * <p>outReadingIntegral carries CNA_CompassReading in this order:
     * <ol start="0">
     *   <li>{@code timestamp.ticks} (int64_t)</li>
     *   <li>{@code timestamp.offset_ticks} (int64_t)</li>
     * </ol>
     *
     * <p>outReadingFloating carries CNA_CompassReading in this order:
     * <ol start="0">
     *   <li>{@code magnetometer_reading.x} (float)</li>
     *   <li>{@code magnetometer_reading.y} (float)</li>
     *   <li>{@code magnetometer_reading.z} (float)</li>
     * </ol>
     *
     * <p>outReadingDoubles carries CNA_CompassReading in this order:
     * <ol start="0">
     *   <li>{@code heading_accuracy} (double)</li>
     *   <li>{@code magnetic_heading} (double)</li>
     *   <li>{@code true_heading} (double)</li>
     * </ol>
     */
    public static native int compassGetCurrentValue(long sensor, long[] outReadingIntegral, float[] outReadingFloating, double[] outReadingDoubles);

    /**
     * cna_compass_get_is_data_valid (sensors.h).
     */
    public static native int compassGetIsDataValid(long sensor, boolean[] outValid);

    /**
     * cna_compass_get_is_supported (sensors.h).
     */
    public static native int compassGetIsSupported(long game, boolean[] outSupported);

    /**
     * cna_compass_get_state (sensors.h).
     */
    public static native int compassGetState(long sensor, int[] outState);

    /**
     * cna_compass_get_time_between_updates_ticks (sensors.h).
     */
    public static native int compassGetTimeBetweenUpdatesTicks(long sensor, long[] outTicks);

    /**
     * cna_compass_inject_calibration_request_ext (sensors.h).
     */
    public static native int compassInjectCalibrationRequestExt(long sensor);

    /**
     * cna_compass_inject_synthetic_update_ext (sensors.h).
     *
     * <p>readingIntegral carries CNA_CompassReading in this order:
     * <ol start="0">
     *   <li>{@code timestamp.ticks} (int64_t)</li>
     *   <li>{@code timestamp.offset_ticks} (int64_t)</li>
     * </ol>
     *
     * <p>readingFloating carries CNA_CompassReading in this order:
     * <ol start="0">
     *   <li>{@code magnetometer_reading.x} (float)</li>
     *   <li>{@code magnetometer_reading.y} (float)</li>
     *   <li>{@code magnetometer_reading.z} (float)</li>
     * </ol>
     *
     * <p>readingDoubles carries CNA_CompassReading in this order:
     * <ol start="0">
     *   <li>{@code heading_accuracy} (double)</li>
     *   <li>{@code magnetic_heading} (double)</li>
     *   <li>{@code true_heading} (double)</li>
     * </ol>
     */
    public static native int compassInjectSyntheticUpdateExt(long sensor, long[] readingIntegral, float[] readingFloating, double[] readingDoubles);

    /**
     * cna_compass_set_test_backend_ext (sensors.h).
     */
    public static native int compassSetTestBackendExt(long sensor, boolean installed, boolean supported);

    /**
     * cna_compass_set_time_between_updates_ticks (sensors.h).
     */
    public static native int compassSetTimeBetweenUpdatesTicks(long sensor, long ticks);

    /**
     * cna_compass_start (sensors.h).
     */
    public static native int compassStart(long sensor);

    /**
     * cna_compass_stop (sensors.h).
     */
    public static native int compassStop(long sensor);

    /**
     * cna_gyroscope_copy_last_dispatch_exception_message_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeCopyLastDispatchExceptionMessageForTestsExt(long game, byte[] destination, long[] outBytes);

    /**
     * cna_gyroscope_create (sensors.h).
     */
    public static native int gyroscopeCreate(long game, long[] outSensor);

    /**
     * cna_gyroscope_destroy (sensors.h).
     */
    public static native int gyroscopeDestroy(long sensor);

    /**
     * cna_gyroscope_dispatch_to_instances_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeDispatchToInstancesForTestsExt(long game, long[] sensors, float x, float y, float z);

    /**
     * cna_gyroscope_dispose (sensors.h).
     */
    public static native int gyroscopeDispose(long sensor);

    /**
     * cna_gyroscope_get_current_value (sensors.h).
     *
     * <p>outReadingIntegral carries CNA_GyroscopeReading in this order:
     * <ol start="0">
     *   <li>{@code timestamp.ticks} (int64_t)</li>
     *   <li>{@code timestamp.offset_ticks} (int64_t)</li>
     * </ol>
     *
     * <p>outReadingFloating carries CNA_GyroscopeReading in this order:
     * <ol start="0">
     *   <li>{@code rotation_rate.x} (float)</li>
     *   <li>{@code rotation_rate.y} (float)</li>
     *   <li>{@code rotation_rate.z} (float)</li>
     * </ol>
     */
    public static native int gyroscopeGetCurrentValue(long sensor, long[] outReadingIntegral, float[] outReadingFloating);

    /**
     * cna_gyroscope_get_dispatch_exception_count_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeGetDispatchExceptionCountForTestsExt(long game, int[] outCount);

    /**
     * cna_gyroscope_get_is_data_valid (sensors.h).
     */
    public static native int gyroscopeGetIsDataValid(long sensor, boolean[] outValid);

    /**
     * cna_gyroscope_get_is_supported (sensors.h).
     */
    public static native int gyroscopeGetIsSupported(long game, boolean[] outSupported);

    /**
     * cna_gyroscope_get_last_dispatch_exception_message_size_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeGetLastDispatchExceptionMessageSizeForTestsExt(long game, long[] outBytes);

    /**
     * cna_gyroscope_get_state (sensors.h).
     */
    public static native int gyroscopeGetState(long sensor, int[] outState);

    /**
     * cna_gyroscope_get_subsystem_held_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeGetSubsystemHeldForTestsExt(long sensor, boolean[] outHeld);

    /**
     * cna_gyroscope_get_time_between_updates_ticks (sensors.h).
     */
    public static native int gyroscopeGetTimeBetweenUpdatesTicks(long sensor, long[] outTicks);

    /**
     * cna_gyroscope_inject_synthetic_update_ext (sensors.h).
     */
    public static native int gyroscopeInjectSyntheticUpdateExt(long sensor, float x, float y, float z);

    /**
     * cna_gyroscope_is_sensor_connected_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeIsSensorConnectedForTestsExt(long game, long sensorId, boolean[] outConnected);

    /**
     * cna_gyroscope_register_started_instance_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeRegisterStartedInstanceForTestsExt(long sensor);

    /**
     * cna_gyroscope_set_event_watch_registration_failure_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeSetEventWatchRegistrationFailureForTestsExt(long game, boolean shouldFail);

    /**
     * cna_gyroscope_set_started_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeSetStartedForTestsExt(long sensor, boolean started);

    /**
     * cna_gyroscope_set_supported_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeSetSupportedForTestsExt(long sensor, boolean supported);

    /**
     * cna_gyroscope_set_time_between_updates_ticks (sensors.h).
     */
    public static native int gyroscopeSetTimeBetweenUpdatesTicks(long sensor, long ticks);

    /**
     * cna_gyroscope_start (sensors.h).
     */
    public static native int gyroscopeStart(long sensor);

    /**
     * cna_gyroscope_stop (sensors.h).
     */
    public static native int gyroscopeStop(long sensor);

    /**
     * cna_gyroscope_unregister_started_instance_for_tests_ext (sensors.h).
     */
    public static native int gyroscopeUnregisterStartedInstanceForTestsExt(long sensor);

    /**
     * cna_motion_create (sensors.h).
     */
    public static native int motionCreate(long game, long[] outSensor);

    /**
     * cna_motion_destroy (sensors.h).
     */
    public static native int motionDestroy(long sensor);

    /**
     * cna_motion_dispose (sensors.h).
     */
    public static native int motionDispose(long sensor);

    /**
     * cna_motion_get_current_value (sensors.h).
     *
     * <p>outReadingIntegral carries CNA_MotionReading in this order:
     * <ol start="0">
     *   <li>{@code timestamp.ticks} (int64_t)</li>
     *   <li>{@code timestamp.offset_ticks} (int64_t)</li>
     *   <li>{@code attitude.timestamp.ticks} (int64_t)</li>
     *   <li>{@code attitude.timestamp.offset_ticks} (int64_t)</li>
     * </ol>
     *
     * <p>outReadingFloating carries CNA_MotionReading in this order:
     * <ol start="0">
     *   <li>{@code attitude.pitch} (float)</li>
     *   <li>{@code attitude.roll} (float)</li>
     *   <li>{@code attitude.yaw} (float)</li>
     *   <li>{@code attitude.quaternion.x} (float)</li>
     *   <li>{@code attitude.quaternion.y} (float)</li>
     *   <li>{@code attitude.quaternion.z} (float)</li>
     *   <li>{@code attitude.quaternion.w} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m11} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m12} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m13} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m14} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m21} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m22} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m23} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m24} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m31} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m32} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m33} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m34} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m41} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m42} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m43} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m44} (float)</li>
     *   <li>{@code device_acceleration.x} (float)</li>
     *   <li>{@code device_acceleration.y} (float)</li>
     *   <li>{@code device_acceleration.z} (float)</li>
     *   <li>{@code device_rotation_rate.x} (float)</li>
     *   <li>{@code device_rotation_rate.y} (float)</li>
     *   <li>{@code device_rotation_rate.z} (float)</li>
     *   <li>{@code gravity.x} (float)</li>
     *   <li>{@code gravity.y} (float)</li>
     *   <li>{@code gravity.z} (float)</li>
     * </ol>
     */
    public static native int motionGetCurrentValue(long sensor, long[] outReadingIntegral, float[] outReadingFloating);

    /**
     * cna_motion_get_is_attitude_north_referenced_ext (sensors.h).
     */
    public static native int motionGetIsAttitudeNorthReferencedExt(long sensor, boolean[] outNorthReferenced);

    /**
     * cna_motion_get_is_data_valid (sensors.h).
     */
    public static native int motionGetIsDataValid(long sensor, boolean[] outValid);

    /**
     * cna_motion_get_is_supported (sensors.h).
     */
    public static native int motionGetIsSupported(long game, boolean[] outSupported);

    /**
     * cna_motion_get_state (sensors.h).
     */
    public static native int motionGetState(long sensor, int[] outState);

    /**
     * cna_motion_get_time_between_updates_ticks (sensors.h).
     */
    public static native int motionGetTimeBetweenUpdatesTicks(long sensor, long[] outTicks);

    /**
     * cna_motion_inject_calibration_request_ext (sensors.h).
     */
    public static native int motionInjectCalibrationRequestExt(long sensor);

    /**
     * cna_motion_inject_synthetic_update_ext (sensors.h).
     *
     * <p>readingIntegral carries CNA_MotionReading in this order:
     * <ol start="0">
     *   <li>{@code timestamp.ticks} (int64_t)</li>
     *   <li>{@code timestamp.offset_ticks} (int64_t)</li>
     *   <li>{@code attitude.timestamp.ticks} (int64_t)</li>
     *   <li>{@code attitude.timestamp.offset_ticks} (int64_t)</li>
     * </ol>
     *
     * <p>readingFloating carries CNA_MotionReading in this order:
     * <ol start="0">
     *   <li>{@code attitude.pitch} (float)</li>
     *   <li>{@code attitude.roll} (float)</li>
     *   <li>{@code attitude.yaw} (float)</li>
     *   <li>{@code attitude.quaternion.x} (float)</li>
     *   <li>{@code attitude.quaternion.y} (float)</li>
     *   <li>{@code attitude.quaternion.z} (float)</li>
     *   <li>{@code attitude.quaternion.w} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m11} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m12} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m13} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m14} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m21} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m22} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m23} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m24} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m31} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m32} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m33} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m34} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m41} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m42} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m43} (float)</li>
     *   <li>{@code attitude.rotation_matrix.m44} (float)</li>
     *   <li>{@code device_acceleration.x} (float)</li>
     *   <li>{@code device_acceleration.y} (float)</li>
     *   <li>{@code device_acceleration.z} (float)</li>
     *   <li>{@code device_rotation_rate.x} (float)</li>
     *   <li>{@code device_rotation_rate.y} (float)</li>
     *   <li>{@code device_rotation_rate.z} (float)</li>
     *   <li>{@code gravity.x} (float)</li>
     *   <li>{@code gravity.y} (float)</li>
     *   <li>{@code gravity.z} (float)</li>
     * </ol>
     */
    public static native int motionInjectSyntheticUpdateExt(long sensor, long[] readingIntegral, float[] readingFloating);

    /**
     * cna_motion_set_test_backend_ext (sensors.h).
     */
    public static native int motionSetTestBackendExt(long sensor, boolean installed, boolean supported, boolean northReferenced);

    /**
     * cna_motion_set_time_between_updates_ticks (sensors.h).
     */
    public static native int motionSetTimeBetweenUpdatesTicks(long sensor, long ticks);

    /**
     * cna_motion_start (sensors.h).
     */
    public static native int motionStart(long sensor);

    /**
     * cna_motion_stop (sensors.h).
     */
    public static native int motionStop(long sensor);

    /**
     * cna_sensor_unsubscribe_ext (sensors.h).
     */
    public static native int sensorUnsubscribeExt(long registration);

    /**
     * cna_sensors_copy_name_at (input_devices.h).
     */
    public static native int sensorsCopyNameAt(long game, int index, byte[] destination, long[] outBytes);

    /**
     * cna_sensors_get_accelerometer (input_devices.h).
     *
     * <p>outAccelerationFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int sensorsGetAccelerometer(long game, float[] outAccelerationFloating, boolean[] outAvailable);

    /**
     * cna_sensors_get_count (input_devices.h).
     */
    public static native int sensorsGetCount(long game, int[] outCount);

    /**
     * cna_sensors_get_gyroscope (input_devices.h).
     *
     * <p>outAngularVelocityFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int sensorsGetGyroscope(long game, float[] outAngularVelocityFloating, boolean[] outAvailable);

    /**
     * cna_sensors_get_info_at (input_devices.h).
     *
     * <p>outInfoIntegral carries CNA_SensorInfo in this order:
     * <ol start="0">
     *   <li>{@code id} (uint32_t)</li>
     *   <li>{@code type} (CNA_SensorType)</li>
     * </ol>
     */
    public static native int sensorsGetInfoAt(long game, int index, long[] outInfoIntegral);

    /**
     * cna_sensors_get_last_error_id_ext (sensors.h).
     */
    public static native int sensorsGetLastErrorIdExt(int[] outErrorId, boolean[] outHasErrorId);

    /**
     * cna_sensors_get_name_size_at (input_devices.h).
     */
    public static native int sensorsGetNameSizeAt(long game, int index, long[] outBytes);
}
