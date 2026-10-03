package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeInputExtensionRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeInputExtensionRoutes {

    private NativeInputExtensionRoutes() {
    }

    /**
     * cna_gamepad_copy_guid_ext (input_gamepad.h).
     */
    public static native int gamepadCopyGuidExt(long game, int playerIndex, byte[] destination, long[] outBytes);

    /**
     * cna_gamepad_copy_name_ext (input_gamepad.h).
     */
    public static native int gamepadCopyNameExt(long game, int playerIndex, byte[] destination, long[] outBytes);

    /**
     * cna_gamepad_copy_path_ext (input_gamepad.h).
     */
    public static native int gamepadCopyPathExt(long game, int playerIndex, byte[] destination, long[] outBytes);

    /**
     * cna_gamepad_copy_serial_ext (input_gamepad.h).
     */
    public static native int gamepadCopySerialExt(long game, int playerIndex, byte[] destination, long[] outBytes);

    /**
     * cna_gamepad_get_accelerometer_ext (input_gamepad.h).
     *
     * <p>outAccelerationFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int gamepadGetAccelerometerExt(long game, int playerIndex, float[] outAccelerationFloating, boolean[] outAvailable);

    /**
     * cna_gamepad_get_button_label_ext (input_gamepad.h).
     */
    public static native int gamepadGetButtonLabelExt(long game, int playerIndex, int button, int[] outLabel);

    /**
     * cna_gamepad_get_connection_state_ext (input_gamepad.h).
     */
    public static native int gamepadGetConnectionStateExt(long game, int playerIndex, int[] outState);

    /**
     * cna_gamepad_get_firmware_version_ext (input_gamepad.h).
     */
    public static native int gamepadGetFirmwareVersionExt(long game, int playerIndex, int[] outVersion);

    /**
     * cna_gamepad_get_guid_size_ext (input_gamepad.h).
     */
    public static native int gamepadGetGuidSizeExt(long game, int playerIndex, long[] outBytes);

    /**
     * cna_gamepad_get_gyro_ext (input_gamepad.h).
     *
     * <p>outGyroFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int gamepadGetGyroExt(long game, int playerIndex, float[] outGyroFloating, boolean[] outAvailable);

    /**
     * cna_gamepad_get_name_size_ext (input_gamepad.h).
     */
    public static native int gamepadGetNameSizeExt(long game, int playerIndex, long[] outBytes);

    /**
     * cna_gamepad_get_path_size_ext (input_gamepad.h).
     */
    public static native int gamepadGetPathSizeExt(long game, int playerIndex, long[] outBytes);

    /**
     * cna_gamepad_get_player_index_ext (input_gamepad.h).
     */
    public static native int gamepadGetPlayerIndexExt(long game, int playerIndex, int[] outIndex);

    /**
     * cna_gamepad_get_power_info_ext (input_gamepad.h).
     */
    public static native int gamepadGetPowerInfoExt(long game, int playerIndex, int[] outState, int[] outPercent);

    /**
     * cna_gamepad_get_serial_size_ext (input_gamepad.h).
     */
    public static native int gamepadGetSerialSizeExt(long game, int playerIndex, long[] outBytes);

    /**
     * cna_gamepad_get_steam_handle_ext (input_gamepad.h).
     */
    public static native int gamepadGetSteamHandleExt(long game, int playerIndex, long[] outHandle);

    /**
     * cna_gamepad_get_touchpad_count_ext (input_gamepad.h).
     */
    public static native int gamepadGetTouchpadCountExt(long game, int playerIndex, int[] outCount);

    /**
     * cna_gamepad_get_touchpad_finger_count_ext (input_gamepad.h).
     */
    public static native int gamepadGetTouchpadFingerCountExt(long game, int playerIndex, int touchpad, int[] outCount);

    /**
     * cna_gamepad_get_touchpad_finger_ext (input_gamepad.h).
     *
     * <p>outFingerBytes carries CNA_GamePadTouchpadFinger in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     * </ol>
     *
     * <p>outFingerIntegral carries CNA_GamePadTouchpadFinger in this order:
     * <ol start="0">
     *   <li>{@code is_down} (CNA_Bool)</li>
     * </ol>
     *
     * <p>outFingerFloating carries CNA_GamePadTouchpadFinger in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code pressure} (float)</li>
     * </ol>
     */
    public static native int gamepadGetTouchpadFingerExt(long game, int playerIndex, int touchpad, int finger, byte[] outFingerBytes, long[] outFingerIntegral, float[] outFingerFloating, boolean[] outAvailable);

    /**
     * cna_gamepad_set_light_bar_ext (input_gamepad.h).
     */
    public static native int gamepadSetLightBarExt(long game, int playerIndex, long[] colorIntegral);

    /**
     * cna_gamepad_set_player_index_ext (input_gamepad.h).
     */
    public static native int gamepadSetPlayerIndexExt(long game, int playerIndex, int index, boolean[] outApplied);

    /**
     * cna_gamepad_set_trigger_vibration_ext (input_gamepad.h).
     */
    public static native int gamepadSetTriggerVibrationExt(long game, int playerIndex, float leftTrigger, float rightTrigger, boolean[] outApplied);

    /**
     * cna_haptic_device_copy_name (input_haptics.h).
     */
    public static native int hapticDeviceCopyName(long device, byte[] destination, long[] outBytes);

    /**
     * cna_haptic_device_create_effect (input_haptics.h).
     *
     * <p>effectIntegral carries CNA_HapticEffect in this order:
     * <ol start="0">
     *   <li>{@code type} (CNA_HapticEffectType)</li>
     *   <li>{@code reserved} (uint32_t)</li>
     *   <li>{@code direction.type} (CNA_HapticDirectionType)</li>
     *   <li>{@code direction.values[0]} (int32_t)</li>
     *   <li>{@code direction.values[1]} (int32_t)</li>
     *   <li>{@code direction.values[2]} (int32_t)</li>
     *   <li>{@code length} (uint32_t)</li>
     *   <li>{@code delay} (uint16_t)</li>
     *   <li>{@code button} (uint16_t)</li>
     *   <li>{@code interval} (uint16_t)</li>
     *   <li>{@code level} (int16_t)</li>
     *   <li>{@code period} (uint16_t)</li>
     *   <li>{@code magnitude} (int16_t)</li>
     *   <li>{@code offset} (int16_t)</li>
     *   <li>{@code phase} (uint16_t)</li>
     *   <li>{@code ramp_start} (int16_t)</li>
     *   <li>{@code ramp_end} (int16_t)</li>
     *   <li>{@code right_saturation[0]} (uint16_t)</li>
     *   <li>{@code right_saturation[1]} (uint16_t)</li>
     *   <li>{@code right_saturation[2]} (uint16_t)</li>
     *   <li>{@code left_saturation[0]} (uint16_t)</li>
     *   <li>{@code left_saturation[1]} (uint16_t)</li>
     *   <li>{@code left_saturation[2]} (uint16_t)</li>
     *   <li>{@code right_coefficient[0]} (int16_t)</li>
     *   <li>{@code right_coefficient[1]} (int16_t)</li>
     *   <li>{@code right_coefficient[2]} (int16_t)</li>
     *   <li>{@code left_coefficient[0]} (int16_t)</li>
     *   <li>{@code left_coefficient[1]} (int16_t)</li>
     *   <li>{@code left_coefficient[2]} (int16_t)</li>
     *   <li>{@code deadband[0]} (uint16_t)</li>
     *   <li>{@code deadband[1]} (uint16_t)</li>
     *   <li>{@code deadband[2]} (uint16_t)</li>
     *   <li>{@code center[0]} (int16_t)</li>
     *   <li>{@code center[1]} (int16_t)</li>
     *   <li>{@code center[2]} (int16_t)</li>
     *   <li>{@code large_magnitude} (uint16_t)</li>
     *   <li>{@code small_magnitude} (uint16_t)</li>
     *   <li>{@code custom_period} (uint16_t)</li>
     *   <li>{@code custom_channels} (uint8_t)</li>
     *   <li>{@code reserved2} (uint8_t)</li>
     *   <li>{@code attack_length} (uint16_t)</li>
     *   <li>{@code attack_level} (uint16_t)</li>
     *   <li>{@code fade_length} (uint16_t)</li>
     *   <li>{@code fade_level} (uint16_t)</li>
     * </ol>
     */
    public static native int hapticDeviceCreateEffect(long device, long[] effectIntegral, int[] customData, int[] outEffectId);

    /**
     * cna_haptic_device_destroy (input_haptics.h).
     */
    public static native int hapticDeviceDestroy(long device);

    /**
     * cna_haptic_device_destroy_effect (input_haptics.h).
     */
    public static native int hapticDeviceDestroyEffect(long device, int effectId);

    /**
     * cna_haptic_device_dispose (input_haptics.h).
     */
    public static native int hapticDeviceDispose(long device);

    /**
     * cna_haptic_device_get_capabilities (input_haptics.h).
     *
     * <p>outCapabilitiesBytes carries CNA_HapticCapabilities in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     * </ol>
     *
     * <p>outCapabilitiesIntegral carries CNA_HapticCapabilities in this order:
     * <ol start="0">
     *   <li>{@code features} (CNA_HapticFeature)</li>
     *   <li>{@code axis_count} (int32_t)</li>
     *   <li>{@code max_effects} (int32_t)</li>
     *   <li>{@code max_effects_playing} (int32_t)</li>
     *   <li>{@code is_open} (CNA_Bool)</li>
     *   <li>{@code rumble_supported} (CNA_Bool)</li>
     * </ol>
     */
    public static native int hapticDeviceGetCapabilities(long device, byte[] outCapabilitiesBytes, long[] outCapabilitiesIntegral);

    /**
     * cna_haptic_device_get_effect_status (input_haptics.h).
     */
    public static native int hapticDeviceGetEffectStatus(long device, int effectId, boolean[] outPlaying);

    /**
     * cna_haptic_device_get_is_effect_supported (input_haptics.h).
     *
     * <p>effectIntegral carries CNA_HapticEffect in this order:
     * <ol start="0">
     *   <li>{@code type} (CNA_HapticEffectType)</li>
     *   <li>{@code reserved} (uint32_t)</li>
     *   <li>{@code direction.type} (CNA_HapticDirectionType)</li>
     *   <li>{@code direction.values[0]} (int32_t)</li>
     *   <li>{@code direction.values[1]} (int32_t)</li>
     *   <li>{@code direction.values[2]} (int32_t)</li>
     *   <li>{@code length} (uint32_t)</li>
     *   <li>{@code delay} (uint16_t)</li>
     *   <li>{@code button} (uint16_t)</li>
     *   <li>{@code interval} (uint16_t)</li>
     *   <li>{@code level} (int16_t)</li>
     *   <li>{@code period} (uint16_t)</li>
     *   <li>{@code magnitude} (int16_t)</li>
     *   <li>{@code offset} (int16_t)</li>
     *   <li>{@code phase} (uint16_t)</li>
     *   <li>{@code ramp_start} (int16_t)</li>
     *   <li>{@code ramp_end} (int16_t)</li>
     *   <li>{@code right_saturation[0]} (uint16_t)</li>
     *   <li>{@code right_saturation[1]} (uint16_t)</li>
     *   <li>{@code right_saturation[2]} (uint16_t)</li>
     *   <li>{@code left_saturation[0]} (uint16_t)</li>
     *   <li>{@code left_saturation[1]} (uint16_t)</li>
     *   <li>{@code left_saturation[2]} (uint16_t)</li>
     *   <li>{@code right_coefficient[0]} (int16_t)</li>
     *   <li>{@code right_coefficient[1]} (int16_t)</li>
     *   <li>{@code right_coefficient[2]} (int16_t)</li>
     *   <li>{@code left_coefficient[0]} (int16_t)</li>
     *   <li>{@code left_coefficient[1]} (int16_t)</li>
     *   <li>{@code left_coefficient[2]} (int16_t)</li>
     *   <li>{@code deadband[0]} (uint16_t)</li>
     *   <li>{@code deadband[1]} (uint16_t)</li>
     *   <li>{@code deadband[2]} (uint16_t)</li>
     *   <li>{@code center[0]} (int16_t)</li>
     *   <li>{@code center[1]} (int16_t)</li>
     *   <li>{@code center[2]} (int16_t)</li>
     *   <li>{@code large_magnitude} (uint16_t)</li>
     *   <li>{@code small_magnitude} (uint16_t)</li>
     *   <li>{@code custom_period} (uint16_t)</li>
     *   <li>{@code custom_channels} (uint8_t)</li>
     *   <li>{@code reserved2} (uint8_t)</li>
     *   <li>{@code attack_length} (uint16_t)</li>
     *   <li>{@code attack_level} (uint16_t)</li>
     *   <li>{@code fade_length} (uint16_t)</li>
     *   <li>{@code fade_level} (uint16_t)</li>
     * </ol>
     */
    public static native int hapticDeviceGetIsEffectSupported(long device, long[] effectIntegral, int[] customData, boolean[] outSupported);

    /**
     * cna_haptic_device_get_is_open (input_haptics.h).
     */
    public static native int hapticDeviceGetIsOpen(long device, boolean[] outOpen);

    /**
     * cna_haptic_device_get_name_size (input_haptics.h).
     */
    public static native int hapticDeviceGetNameSize(long device, long[] outBytes);

    /**
     * cna_haptic_device_init_rumble (input_haptics.h).
     */
    public static native int hapticDeviceInitRumble(long device, boolean[] outApplied);

    /**
     * cna_haptic_device_pause (input_haptics.h).
     */
    public static native int hapticDevicePause(long device, boolean[] outApplied);

    /**
     * cna_haptic_device_play_rumble (input_haptics.h).
     */
    public static native int hapticDevicePlayRumble(long device, float strength, int lengthMs, boolean[] outApplied);

    /**
     * cna_haptic_device_resume (input_haptics.h).
     */
    public static native int hapticDeviceResume(long device, boolean[] outApplied);

    /**
     * cna_haptic_device_run_effect (input_haptics.h).
     */
    public static native int hapticDeviceRunEffect(long device, int effectId, int iterations, boolean[] outApplied);

    /**
     * cna_haptic_device_set_autocenter (input_haptics.h).
     */
    public static native int hapticDeviceSetAutocenter(long device, int autocenter, boolean[] outApplied);

    /**
     * cna_haptic_device_set_gain (input_haptics.h).
     */
    public static native int hapticDeviceSetGain(long device, int gain, boolean[] outApplied);

    /**
     * cna_haptic_device_stop_all_effects (input_haptics.h).
     */
    public static native int hapticDeviceStopAllEffects(long device, boolean[] outApplied);

    /**
     * cna_haptic_device_stop_effect (input_haptics.h).
     */
    public static native int hapticDeviceStopEffect(long device, int effectId, boolean[] outApplied);

    /**
     * cna_haptic_device_stop_rumble (input_haptics.h).
     */
    public static native int hapticDeviceStopRumble(long device, boolean[] outApplied);

    /**
     * cna_haptic_device_update_effect (input_haptics.h).
     *
     * <p>effectIntegral carries CNA_HapticEffect in this order:
     * <ol start="0">
     *   <li>{@code type} (CNA_HapticEffectType)</li>
     *   <li>{@code reserved} (uint32_t)</li>
     *   <li>{@code direction.type} (CNA_HapticDirectionType)</li>
     *   <li>{@code direction.values[0]} (int32_t)</li>
     *   <li>{@code direction.values[1]} (int32_t)</li>
     *   <li>{@code direction.values[2]} (int32_t)</li>
     *   <li>{@code length} (uint32_t)</li>
     *   <li>{@code delay} (uint16_t)</li>
     *   <li>{@code button} (uint16_t)</li>
     *   <li>{@code interval} (uint16_t)</li>
     *   <li>{@code level} (int16_t)</li>
     *   <li>{@code period} (uint16_t)</li>
     *   <li>{@code magnitude} (int16_t)</li>
     *   <li>{@code offset} (int16_t)</li>
     *   <li>{@code phase} (uint16_t)</li>
     *   <li>{@code ramp_start} (int16_t)</li>
     *   <li>{@code ramp_end} (int16_t)</li>
     *   <li>{@code right_saturation[0]} (uint16_t)</li>
     *   <li>{@code right_saturation[1]} (uint16_t)</li>
     *   <li>{@code right_saturation[2]} (uint16_t)</li>
     *   <li>{@code left_saturation[0]} (uint16_t)</li>
     *   <li>{@code left_saturation[1]} (uint16_t)</li>
     *   <li>{@code left_saturation[2]} (uint16_t)</li>
     *   <li>{@code right_coefficient[0]} (int16_t)</li>
     *   <li>{@code right_coefficient[1]} (int16_t)</li>
     *   <li>{@code right_coefficient[2]} (int16_t)</li>
     *   <li>{@code left_coefficient[0]} (int16_t)</li>
     *   <li>{@code left_coefficient[1]} (int16_t)</li>
     *   <li>{@code left_coefficient[2]} (int16_t)</li>
     *   <li>{@code deadband[0]} (uint16_t)</li>
     *   <li>{@code deadband[1]} (uint16_t)</li>
     *   <li>{@code deadband[2]} (uint16_t)</li>
     *   <li>{@code center[0]} (int16_t)</li>
     *   <li>{@code center[1]} (int16_t)</li>
     *   <li>{@code center[2]} (int16_t)</li>
     *   <li>{@code large_magnitude} (uint16_t)</li>
     *   <li>{@code small_magnitude} (uint16_t)</li>
     *   <li>{@code custom_period} (uint16_t)</li>
     *   <li>{@code custom_channels} (uint8_t)</li>
     *   <li>{@code reserved2} (uint8_t)</li>
     *   <li>{@code attack_length} (uint16_t)</li>
     *   <li>{@code attack_level} (uint16_t)</li>
     *   <li>{@code fade_length} (uint16_t)</li>
     *   <li>{@code fade_level} (uint16_t)</li>
     * </ol>
     */
    public static native int hapticDeviceUpdateEffect(long device, int effectId, long[] effectIntegral, int[] customData, boolean[] outApplied);

    /**
     * cna_haptics_copy_name_at (input_haptics.h).
     */
    public static native int hapticsCopyNameAt(long game, int index, byte[] destination, long[] outBytes);

    /**
     * cna_haptics_get_count (input_haptics.h).
     */
    public static native int hapticsGetCount(long game, int[] outCount);

    /**
     * cna_haptics_get_id_at (input_haptics.h).
     */
    public static native int hapticsGetIdAt(long game, int index, int[] outId);

    /**
     * cna_haptics_get_is_joystick_haptic (input_haptics.h).
     */
    public static native int hapticsGetIsJoystickHaptic(long game, int joystickId, boolean[] outHaptic);

    /**
     * cna_haptics_get_is_mouse_haptic (input_haptics.h).
     */
    public static native int hapticsGetIsMouseHaptic(long game, boolean[] outHaptic);

    /**
     * cna_haptics_get_name_size_at (input_haptics.h).
     */
    public static native int hapticsGetNameSizeAt(long game, int index, long[] outBytes);

    /**
     * cna_haptics_open (input_haptics.h).
     */
    public static native int hapticsOpen(long game, int id, long[] outDevice);

    /**
     * cna_haptics_open_from_joystick (input_haptics.h).
     */
    public static native int hapticsOpenFromJoystick(long game, int joystickId, long[] outDevice);

    /**
     * cna_haptics_open_from_mouse (input_haptics.h).
     */
    public static native int hapticsOpenFromMouse(long game, long[] outDevice);

    /**
     * cna_joystick_state_copy_axes (input_joystick.h).
     */
    public static native int joystickStateCopyAxes(long state, short[] destination, long[] outCount);

    /**
     * cna_joystick_state_copy_balls (input_joystick.h).
     */
    public static native int joystickStateCopyBalls(long state, long[] destinationIntegral, long[] outCount);

    /**
     * cna_joystick_state_copy_buttons (input_joystick.h).
     */
    public static native int joystickStateCopyButtons(long state, boolean[] destination, long[] outCount);

    /**
     * cna_joystick_state_copy_hats (input_joystick.h).
     */
    public static native int joystickStateCopyHats(long state, int[] destination, long[] outCount);

    /**
     * cna_joystick_state_destroy (input_joystick.h).
     */
    public static native int joystickStateDestroy(long state);

    /**
     * cna_joystick_state_get_axis_count (input_joystick.h).
     */
    public static native int joystickStateGetAxisCount(long state, int[] outCount);

    /**
     * cna_joystick_state_get_ball_count (input_joystick.h).
     */
    public static native int joystickStateGetBallCount(long state, int[] outCount);

    /**
     * cna_joystick_state_get_button_count (input_joystick.h).
     */
    public static native int joystickStateGetButtonCount(long state, int[] outCount);

    /**
     * cna_joystick_state_get_hat_count (input_joystick.h).
     */
    public static native int joystickStateGetHatCount(long state, int[] outCount);

    /**
     * cna_joysticks_capture_state (input_joystick.h).
     */
    public static native int joysticksCaptureState(long game, int id, long[] outState);

    /**
     * cna_joysticks_copy_capabilities_guid (input_joystick.h).
     */
    public static native int joysticksCopyCapabilitiesGuid(long game, int id, byte[] destination, long[] outBytes);

    /**
     * cna_joysticks_copy_capabilities_name (input_joystick.h).
     */
    public static native int joysticksCopyCapabilitiesName(long game, int id, byte[] destination, long[] outBytes);

    /**
     * cna_joysticks_copy_name_at (input_joystick.h).
     */
    public static native int joysticksCopyNameAt(long game, int index, byte[] destination, long[] outBytes);

    /**
     * cna_joysticks_get_capabilities (input_joystick.h).
     *
     * <p>outCapabilitiesBytes carries CNA_JoystickCapabilities in this order:
     * <ol start="0">
     *   <li>{@code reserved[0]} (uint8_t)</li>
     *   <li>{@code reserved[1]} (uint8_t)</li>
     *   <li>{@code reserved[2]} (uint8_t)</li>
     * </ol>
     *
     * <p>outCapabilitiesIntegral carries CNA_JoystickCapabilities in this order:
     * <ol start="0">
     *   <li>{@code axis_count} (int32_t)</li>
     *   <li>{@code button_count} (int32_t)</li>
     *   <li>{@code hat_count} (int32_t)</li>
     *   <li>{@code ball_count} (int32_t)</li>
     *   <li>{@code type} (CNA_JoystickType)</li>
     *   <li>{@code power_state} (CNA_PowerState)</li>
     *   <li>{@code power_percent} (int32_t)</li>
     *   <li>{@code is_connected} (CNA_Bool)</li>
     * </ol>
     */
    public static native int joysticksGetCapabilities(long game, int id, byte[] outCapabilitiesBytes, long[] outCapabilitiesIntegral);

    /**
     * cna_joysticks_get_capabilities_guid_size (input_joystick.h).
     */
    public static native int joysticksGetCapabilitiesGuidSize(long game, int id, long[] outBytes);

    /**
     * cna_joysticks_get_capabilities_name_size (input_joystick.h).
     */
    public static native int joysticksGetCapabilitiesNameSize(long game, int id, long[] outBytes);

    /**
     * cna_joysticks_get_count (input_joystick.h).
     */
    public static native int joysticksGetCount(long game, int[] outCount);

    /**
     * cna_joysticks_get_info_at (input_joystick.h).
     *
     * <p>outInfoIntegral carries CNA_JoystickInfo in this order:
     * <ol start="0">
     *   <li>{@code id} (uint32_t)</li>
     *   <li>{@code type} (CNA_JoystickType)</li>
     * </ol>
     */
    public static native int joysticksGetInfoAt(long game, int index, long[] outInfoIntegral);

    /**
     * cna_joysticks_get_name_size_at (input_joystick.h).
     */
    public static native int joysticksGetNameSizeAt(long game, int index, long[] outBytes);

    /**
     * cna_joysticks_raise_connected_ext (input_joystick.h).
     */
    public static native int joysticksRaiseConnectedExt(long game, int id);

    /**
     * cna_joysticks_raise_disconnected_ext (input_joystick.h).
     */
    public static native int joysticksRaiseDisconnectedExt(long game, int id);

    /**
     * cna_joysticks_reset_for_tests_ext (input_joystick.h).
     */
    public static native int joysticksResetForTestsExt(long game);

    /**
     * cna_keyboard_copy_key_name_ext (input_keyboard.h).
     */
    public static native int keyboardCopyKeyNameExt(long game, int key, byte[] destination, long[] outBytes);

    /**
     * cna_keyboard_copy_scancode_name_ext (input_keyboard.h).
     */
    public static native int keyboardCopyScancodeNameExt(long game, int key, byte[] destination, long[] outBytes);

    /**
     * cna_keyboard_get_key_from_name_ext (input_keyboard.h).
     */
    public static native int keyboardGetKeyFromNameExt(long game, byte[] name, int[] outKey);

    /**
     * cna_keyboard_get_key_from_scancode_ext (input_keyboard.h).
     */
    public static native int keyboardGetKeyFromScancodeExt(long game, int scancode, int[] outKey);

    /**
     * cna_keyboard_get_key_name_size_ext (input_keyboard.h).
     */
    public static native int keyboardGetKeyNameSizeExt(long game, int key, long[] outBytes);

    /**
     * cna_keyboard_get_mod_state_ext (input_keyboard.h).
     */
    public static native int keyboardGetModStateExt(long game, int[] outModifiers);

    /**
     * cna_keyboard_get_scancode_from_name_ext (input_keyboard.h).
     */
    public static native int keyboardGetScancodeFromNameExt(long game, byte[] name, int[] outKey);

    /**
     * cna_keyboard_get_scancode_name_size_ext (input_keyboard.h).
     */
    public static native int keyboardGetScancodeNameSizeExt(long game, int key, long[] outBytes);

    /**
     * cna_mouse_cursor_create_ext (input_cursor.h).
     */
    public static native int mouseCursorCreateExt(long[] outCursor);

    /**
     * cna_mouse_cursor_create_from_texture2d (input_cursor.h).
     */
    public static native int mouseCursorCreateFromTexture2d(long game, long texture, int originX, int originY, long[] outCursor);

    /**
     * cna_mouse_cursor_destroy (input_cursor.h).
     */
    public static native int mouseCursorDestroy(long cursor);

    /**
     * cna_mouse_cursor_dispose (input_cursor.h).
     */
    public static native int mouseCursorDispose(long cursor);

    /**
     * cna_mouse_cursor_get_stock_ext (input_cursor.h).
     */
    public static native int mouseCursorGetStockExt(long game, int stock, long[] outCursor);

    /**
     * cna_mouse_get_global_position_ext (input_mouse.h).
     */
    public static native int mouseGetGlobalPositionExt(long game, int[] outX, int[] outY);

    /**
     * cna_mouse_get_is_relative_mouse_mode_ext (input_mouse.h).
     */
    public static native int mouseGetIsRelativeMouseModeExt(long game, boolean[] outEnabled);

    /**
     * cna_mouse_raise_clicked_ext (input_mouse.h).
     */
    public static native int mouseRaiseClickedExt(long game, int button);

    /**
     * cna_mouse_set_capture_ext (input_mouse.h).
     */
    public static native int mouseSetCaptureExt(long game, boolean enabled, boolean[] outApplied);

    /**
     * cna_mouse_set_cursor_ext (input_cursor.h).
     */
    public static native int mouseSetCursorExt(long game, long cursor);

    /**
     * cna_mouse_set_is_relative_mouse_mode_ext (input_mouse.h).
     */
    public static native int mouseSetIsRelativeMouseModeExt(long game, boolean enabled);

    /**
     * cna_mouse_warp_global_ext (input_mouse.h).
     */
    public static native int mouseWarpGlobalExt(long game, int x, int y, boolean[] outApplied);

    /**
     * cna_text_input_get_window_handle_ext (input_text.h).
     */
    public static native int textInputGetWindowHandleExt(long game, long[] outWindow);

    /**
     * cna_text_input_is_active_ext (input_text.h).
     */
    public static native int textInputIsActiveExt(long game, boolean[] outActive);

    /**
     * cna_text_input_is_screen_keyboard_shown_ext (input_text.h).
     */
    public static native int textInputIsScreenKeyboardShownExt(long game, boolean[] outShown);

    /**
     * cna_text_input_is_screen_keyboard_shown_for_window_ext (input_text.h).
     */
    public static native int textInputIsScreenKeyboardShownForWindowExt(long game, long window, boolean[] outShown);

    /**
     * cna_text_input_raise_text_editing_ext (input_text.h).
     */
    public static native int textInputRaiseTextEditingExt(long game, byte[] text, int start, int length);

    /**
     * cna_text_input_raise_text_input_ext (input_text.h).
     */
    public static native int textInputRaiseTextInputExt(long game, int codeUnit);

    /**
     * cna_text_input_reset_for_tests_ext (input_text.h).
     */
    public static native int textInputResetForTestsExt(long game);

    /**
     * cna_text_input_set_input_rectangle_ext (input_text.h).
     */
    public static native int textInputSetInputRectangleExt(long game, long[] rectangleIntegral);

    /**
     * cna_text_input_set_window_handle_ext (input_text.h).
     */
    public static native int textInputSetWindowHandleExt(long game, long window);

    /**
     * cna_text_input_start_ext (input_text.h).
     */
    public static native int textInputStartExt(long game);

    /**
     * cna_text_input_start_with_type_ext (input_text.h).
     */
    public static native int textInputStartWithTypeExt(long game, int type);

    /**
     * cna_text_input_stop_ext (input_text.h).
     */
    public static native int textInputStopExt(long game);

    /**
     * cna_touch_panel_get_mouse_touch_emulation_enabled_ext (input_touch.h).
     */
    public static native int touchPanelGetMouseTouchEmulationEnabledExt(long game, boolean[] outEnabled);

    /**
     * cna_touch_panel_get_touch_device_exists_ext (input_touch.h).
     */
    public static native int touchPanelGetTouchDeviceExistsExt(long game, boolean[] outExists);

    /**
     * cna_touch_panel_set_mouse_touch_emulation_enabled_ext (input_touch.h).
     */
    public static native int touchPanelSetMouseTouchEmulationEnabledExt(long game, boolean enabled);
}
