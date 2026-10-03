package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeDeviceExtensionRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeDeviceExtensionRoutes {

    private NativeDeviceExtensionRoutes() {
    }

    /**
     * cna_camera_copy_name_at_ext (devices.h).
     */
    public static native int cameraCopyNameAtExt(long game, long index, byte[] destination, long[] outBytes);

    /**
     * cna_camera_create (devices.h).
     */
    public static native int cameraCreate(long game, long[] outCamera);

    /**
     * cna_camera_create_with_test_backend_ext (devices.h).
     */
    public static native int cameraCreateWithTestBackendExt(long game, long[] outCamera);

    /**
     * cna_camera_destroy (devices.h).
     */
    public static native int cameraDestroy(long camera);

    /**
     * cna_camera_device_info_init (devices.h).
     *
     * <p>outInfoIntegral carries CNA_CameraDeviceInfo in this order:
     * <ol start="0">
     *   <li>{@code position} (CNA_CameraPosition)</li>
     * </ol>
     */
    public static native int cameraDeviceInfoInit(long[] outInfoIntegral);

    /**
     * cna_camera_get_count_ext (devices.h).
     */
    public static native int cameraGetCountExt(long game, long[] outCount);

    /**
     * cna_camera_get_frame_height_ext (devices.h).
     */
    public static native int cameraGetFrameHeightExt(long camera, int[] outHeight);

    /**
     * cna_camera_get_frame_width_ext (devices.h).
     */
    public static native int cameraGetFrameWidthExt(long camera, int[] outWidth);

    /**
     * cna_camera_get_info_at_ext (devices.h).
     *
     * <p>outInfoIntegral carries CNA_CameraDeviceInfo in this order:
     * <ol start="0">
     *   <li>{@code position} (CNA_CameraPosition)</li>
     * </ol>
     */
    public static native int cameraGetInfoAtExt(long game, long index, long[] outInfoIntegral);

    /**
     * cna_camera_get_is_supported_ext (devices.h).
     */
    public static native int cameraGetIsSupportedExt(long game, boolean[] outSupported);

    /**
     * cna_camera_get_name_size_at_ext (devices.h).
     */
    public static native int cameraGetNameSizeAtExt(long game, long index, long[] outBytes);

    /**
     * cna_camera_get_state_ext (devices.h).
     */
    public static native int cameraGetStateExt(long camera, int[] outState);

    /**
     * cna_camera_set_test_frame_ext (devices.h).
     */
    public static native int cameraSetTestFrameExt(long camera, int width, int height, long[] pixelsIntegral);

    /**
     * cna_camera_set_test_state_ext (devices.h).
     */
    public static native int cameraSetTestStateExt(long camera, int state);

    /**
     * cna_camera_try_acquire_frame_ext (devices.h).
     */
    public static native int cameraTryAcquireFrameExt(long camera, long texture, boolean[] outAcquired);

    /**
     * cna_clipboard_copy_text (input_devices.h).
     */
    public static native int clipboardCopyText(long game, byte[] destination, long[] outBytes);

    /**
     * cna_clipboard_get_has_text (input_devices.h).
     */
    public static native int clipboardGetHasText(long game, boolean[] outHasText);

    /**
     * cna_clipboard_get_text_size (input_devices.h).
     */
    public static native int clipboardGetTextSize(long game, long[] outBytes);

    /**
     * cna_devices_clipboard_set_text_ext (devices.h).
     */
    public static native int devicesClipboardSetTextExt(long game, byte[] text, boolean[] outAccepted);

    /**
     * cna_devices_ext_is_available (devices.h).
     */
    public static native int devicesExtIsAvailable(boolean[] outAvailable);

    /**
     * cna_display_info_get_content_scale_ext (devices.h).
     */
    public static native int displayInfoGetContentScaleExt(long game, float[] outScale);

    /**
     * cna_display_info_get_safe_area_ext (devices.h).
     *
     * <p>outAreaIntegral carries CNA_Rectangle in this order:
     * <ol start="0">
     *   <li>{@code x} (int32_t)</li>
     *   <li>{@code y} (int32_t)</li>
     *   <li>{@code width} (int32_t)</li>
     *   <li>{@code height} (int32_t)</li>
     * </ol>
     */
    public static native int displayInfoGetSafeAreaExt(long game, long[] outAreaIntegral);

    /**
     * cna_environment_get_device_type (devices.h).
     */
    public static native int environmentGetDeviceType(int[] outDeviceType);

    /**
     * cna_file_dialog_get_is_supported_ext (devices.h).
     */
    public static native int fileDialogGetIsSupportedExt(long game, boolean[] outSupported);

    /**
     * cna_file_dialog_set_test_backend_ext (devices.h).
     */
    public static native int fileDialogSetTestBackendExt(long game, boolean installed, byte[][] results);

    /**
     * cna_input_devices_copy_keyboard_name_at (input_devices.h).
     */
    public static native int inputDevicesCopyKeyboardNameAt(long game, int index, byte[] destination, long[] outBytes);

    /**
     * cna_input_devices_copy_mouse_name_at (input_devices.h).
     */
    public static native int inputDevicesCopyMouseNameAt(long game, int index, byte[] destination, long[] outBytes);

    /**
     * cna_input_devices_copy_touch_device_name_at (input_devices.h).
     */
    public static native int inputDevicesCopyTouchDeviceNameAt(long game, int index, byte[] destination, long[] outBytes);

    /**
     * cna_input_devices_get_keyboard_count (input_devices.h).
     */
    public static native int inputDevicesGetKeyboardCount(long game, int[] outCount);

    /**
     * cna_input_devices_get_keyboard_info_at (input_devices.h).
     *
     * <p>outInfoIntegral carries CNA_InputDeviceInfo in this order:
     * <ol start="0">
     *   <li>{@code id} (uint64_t)</li>
     * </ol>
     */
    public static native int inputDevicesGetKeyboardInfoAt(long game, int index, long[] outInfoIntegral);

    /**
     * cna_input_devices_get_keyboard_name_size_at (input_devices.h).
     */
    public static native int inputDevicesGetKeyboardNameSizeAt(long game, int index, long[] outBytes);

    /**
     * cna_input_devices_get_mouse_count (input_devices.h).
     */
    public static native int inputDevicesGetMouseCount(long game, int[] outCount);

    /**
     * cna_input_devices_get_mouse_info_at (input_devices.h).
     *
     * <p>outInfoIntegral carries CNA_InputDeviceInfo in this order:
     * <ol start="0">
     *   <li>{@code id} (uint64_t)</li>
     * </ol>
     */
    public static native int inputDevicesGetMouseInfoAt(long game, int index, long[] outInfoIntegral);

    /**
     * cna_input_devices_get_mouse_name_size_at (input_devices.h).
     */
    public static native int inputDevicesGetMouseNameSizeAt(long game, int index, long[] outBytes);

    /**
     * cna_input_devices_get_touch_device_count (input_devices.h).
     */
    public static native int inputDevicesGetTouchDeviceCount(long game, int[] outCount);

    /**
     * cna_input_devices_get_touch_device_info_at (input_devices.h).
     *
     * <p>outInfoIntegral carries CNA_InputDeviceInfo in this order:
     * <ol start="0">
     *   <li>{@code id} (uint64_t)</li>
     * </ol>
     */
    public static native int inputDevicesGetTouchDeviceInfoAt(long game, int index, long[] outInfoIntegral);

    /**
     * cna_input_devices_get_touch_device_name_size_at (input_devices.h).
     */
    public static native int inputDevicesGetTouchDeviceNameSizeAt(long game, int index, long[] outBytes);

    /**
     * cna_input_devices_raise_keyboard_connected_ext (input_devices.h).
     */
    public static native int inputDevicesRaiseKeyboardConnectedExt(long game, int id);

    /**
     * cna_input_devices_raise_keyboard_disconnected_ext (input_devices.h).
     */
    public static native int inputDevicesRaiseKeyboardDisconnectedExt(long game, int id);

    /**
     * cna_input_devices_raise_mouse_connected_ext (input_devices.h).
     */
    public static native int inputDevicesRaiseMouseConnectedExt(long game, int id);

    /**
     * cna_input_devices_raise_mouse_disconnected_ext (input_devices.h).
     */
    public static native int inputDevicesRaiseMouseDisconnectedExt(long game, int id);

    /**
     * cna_locale_copy_country_at_ext (devices.h).
     */
    public static native int localeCopyCountryAtExt(long game, long index, byte[] destination, long[] outBytes);

    /**
     * cna_locale_copy_language_at_ext (devices.h).
     */
    public static native int localeCopyLanguageAtExt(long game, long index, byte[] destination, long[] outBytes);

    /**
     * cna_locale_get_country_size_at_ext (devices.h).
     */
    public static native int localeGetCountrySizeAtExt(long game, long index, long[] outBytes);

    /**
     * cna_locale_get_language_size_at_ext (devices.h).
     */
    public static native int localeGetLanguageSizeAtExt(long game, long index, long[] outBytes);

    /**
     * cna_locale_get_preferred_count_ext (devices.h).
     */
    public static native int localeGetPreferredCountExt(long game, long[] outCount);

    /**
     * cna_message_box_get_is_supported_ext (devices.h).
     */
    public static native int messageBoxGetIsSupportedExt(long game, boolean[] outSupported);

    /**
     * cna_message_box_get_test_log_ext (devices.h).
     *
     * <p>outLogIntegral carries CNA_MessageBoxTestLog in this order:
     * <ol start="0">
     *   <li>{@code simple_calls} (uint32_t)</li>
     *   <li>{@code choice_calls} (uint32_t)</li>
     *   <li>{@code last_type} (CNA_MessageBoxType)</li>
     *   <li>{@code last_button_count} (uint32_t)</li>
     * </ol>
     */
    public static native int messageBoxGetTestLogExt(long game, long[] outLogIntegral);

    /**
     * cna_message_box_set_test_backend_ext (devices.h).
     */
    public static native int messageBoxSetTestBackendExt(long game, boolean installed, int chosenButton);

    /**
     * cna_message_box_show_ext (devices.h).
     */
    public static native int messageBoxShowExt(long game, int type, byte[] title, byte[] message, byte[][] buttonLabels, int[] outChosen);

    /**
     * cna_message_box_show_simple_ext (devices.h).
     */
    public static native int messageBoxShowSimpleExt(long game, int type, byte[] title, byte[] message);

    /**
     * cna_power_get_battery_percent_ext (devices.h).
     */
    public static native int powerGetBatteryPercentExt(long game, int[] outPercent);

    /**
     * cna_power_get_info (input_devices.h).
     */
    public static native int powerGetInfo(long game, int[] outState, int[] outSecondsLeft, int[] outPercent);

    /**
     * cna_power_get_seconds_remaining_ext (devices.h).
     */
    public static native int powerGetSecondsRemainingExt(long game, int[] outSeconds);

    /**
     * cna_power_get_state_ext (devices.h).
     */
    public static native int powerGetStateExt(long game, int[] outState);

    /**
     * cna_system_info_get_logical_cpu_core_count_ext (devices.h).
     */
    public static native int systemInfoGetLogicalCpuCoreCountExt(long game, int[] outCount);

    /**
     * cna_system_info_get_system_ram_megabytes_ext (devices.h).
     */
    public static native int systemInfoGetSystemRamMegabytesExt(long game, int[] outMegabytes);

    /**
     * cna_system_tray_click_entry_for_tests_ext (devices.h).
     */
    public static native int systemTrayClickEntryForTestsExt(long tray, long index);

    /**
     * cna_system_tray_create (devices.h).
     */
    public static native int systemTrayCreate(long game, byte[] tooltip, long[] outTray);

    /**
     * cna_system_tray_create_with_test_backend_ext (devices.h).
     */
    public static native int systemTrayCreateWithTestBackendExt(long game, byte[] tooltip, long[] outTray);

    /**
     * cna_system_tray_destroy (devices.h).
     */
    public static native int systemTrayDestroy(long tray);

    /**
     * cna_system_tray_get_entry_checked (devices.h).
     */
    public static native int systemTrayGetEntryChecked(long tray, long index, boolean[] outChecked);

    /**
     * cna_system_tray_get_entry_enabled (devices.h).
     */
    public static native int systemTrayGetEntryEnabled(long tray, long index, boolean[] outEnabled);

    /**
     * cna_system_tray_get_is_supported_ext (devices.h).
     */
    public static native int systemTrayGetIsSupportedExt(long game, boolean[] outSupported);

    /**
     * cna_system_tray_set_entry_checked (devices.h).
     */
    public static native int systemTraySetEntryChecked(long tray, long index, boolean checked);

    /**
     * cna_system_tray_set_entry_enabled (devices.h).
     */
    public static native int systemTraySetEntryEnabled(long tray, long index, boolean enabled);

    /**
     * cna_system_tray_set_entry_label (devices.h).
     */
    public static native int systemTraySetEntryLabel(long tray, long index, byte[] label);

    /**
     * cna_system_tray_set_tooltip (devices.h).
     */
    public static native int systemTraySetTooltip(long tray, byte[] tooltip);

    /**
     * cna_url_launcher_open_ext (devices.h).
     */
    public static native int urlLauncherOpenExt(long game, byte[] url, boolean[] outOpened);

    /**
     * cna_vibrate_controller_copy_device_name_ext (devices.h).
     */
    public static native int vibrateControllerCopyDeviceNameExt(long game, byte[] destination, long[] outBytes);

    /**
     * cna_vibrate_controller_get_device_name_size_ext (devices.h).
     */
    public static native int vibrateControllerGetDeviceNameSizeExt(long game, long[] outBytes);

    /**
     * cna_vibrate_controller_get_is_supported_ext (devices.h).
     */
    public static native int vibrateControllerGetIsSupportedExt(long game, boolean[] outSupported);

    /**
     * cna_vibrate_controller_get_test_log_ext (devices.h).
     *
     * <p>outLogIntegral carries CNA_VibrationTestLog in this order:
     * <ol start="0">
     *   <li>{@code start_calls} (uint32_t)</li>
     *   <li>{@code stop_calls} (uint32_t)</li>
     *   <li>{@code left_right_calls} (uint32_t)</li>
     *   <li>{@code reserved} (uint32_t)</li>
     *   <li>{@code last_duration_ticks} (int64_t)</li>
     * </ol>
     *
     * <p>outLogFloating carries CNA_VibrationTestLog in this order:
     * <ol start="0">
     *   <li>{@code last_intensity} (float)</li>
     *   <li>{@code last_large_motor} (float)</li>
     *   <li>{@code last_small_motor} (float)</li>
     *   <li>{@code reserved_float} (float)</li>
     * </ol>
     */
    public static native int vibrateControllerGetTestLogExt(long game, long[] outLogIntegral, float[] outLogFloating);

    /**
     * cna_vibrate_controller_set_test_backend_ext (devices.h).
     */
    public static native int vibrateControllerSetTestBackendExt(long game, boolean installed, boolean supported, byte[] deviceName);

    /**
     * cna_vibrate_controller_start (devices.h).
     */
    public static native int vibrateControllerStart(long game, long durationTicks);

    /**
     * cna_vibrate_controller_start_left_right_ext (devices.h).
     */
    public static native int vibrateControllerStartLeftRightExt(long game, float largeMotor, float smallMotor, long durationTicks);

    /**
     * cna_vibrate_controller_start_with_intensity_ext (devices.h).
     */
    public static native int vibrateControllerStartWithIntensityExt(long game, long durationTicks, float intensity);

    /**
     * cna_vibrate_controller_stop (devices.h).
     */
    public static native int vibrateControllerStop(long game);
}
