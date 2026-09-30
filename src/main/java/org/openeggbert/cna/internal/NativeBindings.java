package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.WindowHandle;
import Microsoft.Xna.Framework.Content.ContentManager;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.GraphicsResource;
import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.EffectAnnotation;
import Microsoft.Xna.Framework.Graphics.EffectParameter;
import Microsoft.Xna.Framework.Graphics.EffectPass;
import Microsoft.Xna.Framework.Graphics.EffectTechnique;
import Microsoft.Xna.Framework.Graphics.DirectionalLight;
import Microsoft.Xna.Framework.Graphics.CubeMapFace;
import Microsoft.Xna.Framework.Graphics.Texture;
import Microsoft.Xna.Framework.Graphics.PresentationParameters;
import Microsoft.Xna.Framework.Graphics.RenderTarget2D;
import Microsoft.Xna.Framework.Graphics.RenderTargetBinding;
import Microsoft.Xna.Framework.Graphics.RenderTargetCube;
import Microsoft.Xna.Framework.Graphics.SpriteBatch;
import Microsoft.Xna.Framework.Graphics.SpriteEffects;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Graphics.SpriteSortMode;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.Texture3D;
import Microsoft.Xna.Framework.Graphics.TextureCube;
import Microsoft.Xna.Framework.Graphics.IndexBuffer;
import Microsoft.Xna.Framework.Graphics.VertexBuffer;
import Microsoft.Xna.Framework.Graphics.VertexBufferBinding;

import java.nio.file.Path;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.WeakHashMap;

/** JNI entry point for CNA's stable C ABI. This class is not application API. */
public final class NativeBindings {

    /**
     * CNA C headers used to compile this binding: ABI 0.35.0.
     *
     * <p>Kept in step with {@code tools/native-abi/bindings.json}'s {@code compiledAbi} by
     * {@code nativeAbiCheck}, because this constant is what a game actually hits at load time
     * and a manifest that moved without it would pass every gate and then refuse the library.
     */
    public static final int COMPILED_ABI_VERSION = encodeVersion(0, 35, 0);

    private static boolean bridgeLoaded;
    private static int runtimeAbiVersion;
    private static final Map<Game, NativeGameHandle> GAMES = new WeakHashMap<>();
    private static final Map<GraphicsDevice, Game> DEVICES = new WeakHashMap<>();
    private static final Map<GraphicsResource, NativeResourceHandle> RESOURCES =
            new WeakHashMap<>();
    private static final Map<GraphicsResource, Game> RESOURCE_OWNERS = new WeakHashMap<>();
    private static final Map<Game, List<GraphicsResource>> GAME_RESOURCES = new WeakHashMap<>();
    private static final Map<Object, NativeResourceHandle> EFFECT_MEMBERS = new WeakHashMap<>();
    private static final Map<Effect, List<Object>> EFFECT_MEMBER_OWNERS = new WeakHashMap<>();
    private static final Map<GraphicsDevice, List<VertexBuffer>> DEVICE_VERTEX_BINDINGS =
            new WeakHashMap<>();
    private static final Map<GraphicsDevice, IndexBuffer> DEVICE_INDEX_BINDINGS =
            new WeakHashMap<>();
    private static final Map<SpriteFont, NativeResourceHandle> SPRITE_FONTS =
            new WeakHashMap<>();
    private static final Map<SpriteFont, Game> SPRITE_FONT_OWNERS = new WeakHashMap<>();
    private static final Map<Game, List<SpriteFont>> GAME_SPRITE_FONTS = new WeakHashMap<>();
    private static final Map<SpriteFont, Texture2D> SPRITE_FONT_ATLASES = new WeakHashMap<>();
    private static final Map<ContentManager, NativeResourceHandle> CONTENT_MANAGERS =
            new WeakHashMap<>();
    private static final Map<ContentManager, Game> CONTENT_MANAGER_OWNERS = new WeakHashMap<>();
    private static final Map<Game, List<ContentManager>> GAME_CONTENT_MANAGERS =
            new WeakHashMap<>();
    private static final Map<WindowHandle, Long> WINDOW_HANDLES = new WeakHashMap<>();
    private static final Map<GraphicsDevice, Throwable> DEVICE_CALLBACK_FAILURES =
            new WeakHashMap<>();
    private static final ThreadLocal<GraphicsResource> GRAPHICS_RESOURCE_EVENT =
            new ThreadLocal<>();
    private static Game currentGame;

    private NativeBindings() {
    }

    /** Loads the JNI adapter and the configured CNA C ABI library, then checks compatibility. */
    public static synchronized int requireAvailable() {
        if (runtimeAbiVersion != 0) {
            return runtimeAbiVersion;
        }
        loadBridge();

        String cnaLibrary = configuredCnaLibrary();
        int version = nativeLoadCna(cnaLibrary);
        int compiledMajor = versionMajor(COMPILED_ABI_VERSION);
        int compiledMinor = (COMPILED_ABI_VERSION >>> 8) & 0xff;
        int runtimeMinor = (version >>> 8) & 0xff;
        if (versionMajor(version) != compiledMajor || runtimeMinor != compiledMinor) {
            throw new UnsatisfiedLinkError(
                    "CNA C ABI mismatch: cna-java requires ABI " + compiledMajor + "."
                            + compiledMinor + ".x exactly"
                            + ", but loaded " + formatVersion(version));
        }
        runtimeAbiVersion = version;
        return version;
    }

    /** Returns whether a compatible native backend can be loaded without throwing. */
    public static boolean isAvailable() {
        try {
            requireAvailable();
            return true;
        } catch (LinkageError | RuntimeException unavailable) {
            return false;
        }
    }

    /** Returns a human-readable encoded ABI version. */
    public static String formatVersion(int version) {
        return versionMajor(version) + "." + ((version >>> 8) & 0xff) + "." + (version & 0xff);
    }

    public static NativeGameHandle createGame(
            Game game, String title, boolean fixedTimeStep, long targetTicks) {
        requireAvailable();
        long handle = nativeCreateGame(
                game, title.getBytes(StandardCharsets.UTF_8), fixedTimeStep, targetTicks);
        if (handle == 0L) {
            throw failure("cna_game_create", -1);
        }
        NativeGameHandle nativeGame = new NativeGameHandle(handle, game);
        synchronized (GAMES) {
            GAMES.put(game, nativeGame);
            currentGame = game;
        }
        NativeMedia.beginGameLifetime();
        NativeStorage.beginGameLifetime();
        return nativeGame;
    }

    public static void run(NativeGameHandle game) {
        check("cna_game_run", nativeRun(game.requireValue()));
    }

    public static void runOneFrame(NativeGameHandle game) {
        check("cna_game_run_one_frame", nativeRunOneFrame(game.requireValue()));
    }

    public static void resetElapsedTime(NativeGameHandle game) {
        check("cna_game_reset_elapsed_time", nativeResetElapsedTime(game.requireValue()));
    }

    public static void suppressDraw(NativeGameHandle game) {
        check("cna_game_suppress_draw", nativeSuppressDraw(game.requireValue()));
    }

    public static void tick(NativeGameHandle game) {
        check("cna_game_tick", nativeTick(game.requireValue()));
    }

    public static void requestExit(NativeGameHandle game) {
        check("cna_game_request_exit", nativeRequestExit(game.requireValue()));
    }

    public static void clear(NativeGameHandle game, int red, int green, int blue, int alpha) {
        check("cna_game_clear", nativeClear(game.requireValue(), red, green, blue, alpha));
    }

    /** Internal adapter used by strict facades without exposing a native handle. */
    public static void clear(Game game, int red, int green, int blue, int alpha) {
        clear(gameHandle(game, "GraphicsDevice"), red, green, blue, alpha);
    }

    public static void setMouseVisible(NativeGameHandle game, boolean visible) {
        check("cna_game_set_is_mouse_visible", nativeSetMouseVisible(game.requireValue(), visible));
    }

    public static boolean getMouseVisible(NativeGameHandle game) {
        return booleanResult(
                "cna_game_get_is_mouse_visible", nativeGetMouseVisible(game.requireValue()));
    }

    public static boolean getIsActive(NativeGameHandle game) {
        return booleanResult("cna_game_get_is_active", nativeGetIsActive(game.requireValue()));
    }

    public static void setFixedTimeStep(NativeGameHandle game, boolean value) {
        check("cna_game_set_is_fixed_time_step", nativeSetFixedTimeStep(game.requireValue(), value));
    }

    public static boolean getFixedTimeStep(NativeGameHandle game) {
        return booleanResult(
                "cna_game_get_is_fixed_time_step", nativeGetFixedTimeStep(game.requireValue()));
    }

    public static void setTargetElapsedTime(NativeGameHandle game, long ticks) {
        check("cna_game_set_target_elapsed_time_ticks",
                nativeSetTargetElapsedTime(game.requireValue(), ticks));
    }

    public static long getTargetElapsedTime(NativeGameHandle game) {
        return longResult("cna_game_get_target_elapsed_time_ticks",
                nativeGetTargetElapsedTime(game.requireValue()));
    }

    public static void setInactiveSleepTime(NativeGameHandle game, long ticks) {
        check("cna_game_set_inactive_sleep_time_ticks",
                nativeSetInactiveSleepTime(game.requireValue(), ticks));
    }

    public static long getInactiveSleepTime(NativeGameHandle game) {
        return longResult("cna_game_get_inactive_sleep_time_ticks",
                nativeGetInactiveSleepTime(game.requireValue()));
    }

    public static boolean getWindowAllowUserResizing(Game game) {
        return booleanResult("cna_game_window_get_allow_user_resizing",
                nativeGetWindowAllowUserResizing(gameHandle(game, "GameWindow").requireValue()));
    }

    public static void setWindowAllowUserResizing(Game game, boolean value) {
        check("cna_game_window_set_allow_user_resizing",
                nativeSetWindowAllowUserResizing(
                        gameHandle(game, "GameWindow").requireValue(), value));
    }

    public static int[] getWindowClientBounds(Game game) {
        int[] value = new int[4];
        check("cna_game_window_get_client_bounds",
                nativeGetWindowClientBounds(gameHandle(game, "GameWindow").requireValue(), value));
        return value;
    }

    public static int getWindowCurrentOrientation(Game game) {
        long value = longResult("cna_game_window_get_current_orientation",
                nativeGetWindowCurrentOrientation(gameHandle(game, "GameWindow").requireValue()));
        return Math.toIntExact(value);
    }

    public static long getWindowHandle(Game game) {
        long[] value = new long[1];
        check("cna_game_window_get_native_handle_ext",
                nativeGetWindowHandle(gameHandle(game, "GameWindow").requireValue(), value));
        return value[0];
    }

    public static String getWindowScreenDeviceName(Game game) {
        long gameValue = gameHandle(game, "GameWindow").requireValue();
        int size = Math.toIntExact(longResult("cna_game_window_get_screen_device_name_size",
                nativeGetWindowScreenDeviceNameSize(gameValue)));
        if (size == 0) {
            return "";
        }
        byte[] utf8 = new byte[size];
        check("cna_game_window_copy_screen_device_name",
                nativeCopyWindowScreenDeviceName(gameValue, utf8));
        return new String(utf8, StandardCharsets.UTF_8);
    }

    public static void setWindowTitle(Game game, String title) {
        check("cna_game_set_window_title",
                nativeSetWindowTitle(gameHandle(game, "GameWindow").requireValue(),
                        Objects.requireNonNull(title, "title").getBytes(StandardCharsets.UTF_8)));
    }

    public static void beginWindowScreenDeviceChange(Game game, boolean willBeFullScreen) {
        check("cna_game_window_begin_screen_device_change",
                nativeBeginWindowScreenDeviceChange(
                        gameHandle(game, "GameWindow").requireValue(), willBeFullScreen));
    }

    public static void endWindowScreenDeviceChange(
            Game game,
            String screenDeviceName,
            int clientWidth,
            int clientHeight) {
        check("cna_game_window_end_screen_device_change",
                nativeEndWindowScreenDeviceChange(
                        gameHandle(game, "GameWindow").requireValue(),
                        Objects.requireNonNull(screenDeviceName, "screenDeviceName")
                                .getBytes(StandardCharsets.UTF_8),
                        clientWidth, clientHeight));
    }

    /** Captures a copy of the current XNA keyboard bit set. A negative player selects no slot. */
    public static long[] getKeyboardState(int playerIndex) {
        NativeGameHandle game = currentGameHandle("Keyboard.GetState");
        long[] words = new long[4];
        check(playerIndex < 0 ? "cna_keyboard_get_state" : "cna_keyboard_get_state_for_player",
                nativeGetKeyboardState(game.requireValue(), playerIndex, words));
        return words;
    }

    public static int[] getMouseState() {
        int[] state = new int[4];
        check("cna_mouse_get_state",
                nativeGetMouseState(currentGameHandle("Mouse.GetState").requireValue(), state));
        return state;
    }

    public static void getGamePadState(
            int playerIndex, int deadZone, int[] discrete, float[] analog) {
        check(deadZone < 0 ? "cna_gamepad_get_state" : "cna_gamepad_get_state_with_dead_zone",
                nativeGetGamePadState(
                        currentGameHandle("GamePad.GetState").requireValue(),
                        playerIndex, deadZone, discrete, analog));
    }

    public static int[] getGamePadCapabilities(int playerIndex) {
        int[] capabilities = new int[26];
        check("cna_gamepad_get_capabilities", nativeGetGamePadCapabilities(
                currentGameHandle("GamePad.GetCapabilities").requireValue(),
                playerIndex, capabilities));
        return capabilities;
    }

    public static boolean setGamePadVibration(
            int playerIndex, float leftMotor, float rightMotor) {
        return booleanResult("cna_gamepad_set_vibration", nativeSetGamePadVibration(
                currentGameHandle("GamePad.SetVibration").requireValue(),
                playerIndex, leftMotor, rightMotor));
    }

    public static int[] getTouchCapabilities() {
        int[] output = new int[2];
        check("cna_touch_get_capabilities", nativeGetTouchCapabilities(
                currentGameHandle("TouchPanel.GetCapabilities").requireValue(), output));
        return output;
    }

    public static void getTouchState(int[] discrete, float[] positions) {
        requireOutputLength(discrete, 26, "TouchPanel state integer");
        requireOutputLength(positions, 32, "TouchPanel state position");
        check("cna_touch_get_state", nativeGetTouchState(
                currentGameHandle("TouchPanel.GetState").requireValue(),
                discrete, positions));
    }

    public static int getTouchPanelDisplayWidth() {
        int[] output = new int[1];
        check("cna_touch_panel_get_display_width", nativeTouchPanelGetDisplayWidth(
                currentGameHandle("TouchPanel.DisplayWidth").requireValue(), output));
        return output[0];
    }

    public static void setTouchPanelDisplayWidth(int value) {
        check("cna_touch_panel_set_display_width", nativeTouchPanelSetDisplayWidth(
                currentGameHandle("TouchPanel.DisplayWidth").requireValue(), value));
    }

    public static int getTouchPanelDisplayHeight() {
        int[] output = new int[1];
        check("cna_touch_panel_get_display_height", nativeTouchPanelGetDisplayHeight(
                currentGameHandle("TouchPanel.DisplayHeight").requireValue(), output));
        return output[0];
    }

    public static void setTouchPanelDisplayHeight(int value) {
        check("cna_touch_panel_set_display_height", nativeTouchPanelSetDisplayHeight(
                currentGameHandle("TouchPanel.DisplayHeight").requireValue(), value));
    }

    public static int getTouchPanelDisplayOrientation() {
        int[] output = new int[1];
        check("cna_touch_panel_get_display_orientation", nativeTouchPanelGetDisplayOrientation(
                currentGameHandle("TouchPanel.DisplayOrientation").requireValue(), output));
        return output[0];
    }

    public static void setTouchPanelDisplayOrientation(int value) {
        check("cna_touch_panel_set_display_orientation", nativeTouchPanelSetDisplayOrientation(
                currentGameHandle("TouchPanel.DisplayOrientation").requireValue(), value));
    }

    public static int getTouchPanelEnabledGestures() {
        int[] output = new int[1];
        check("cna_touch_panel_get_enabled_gestures", nativeTouchPanelGetEnabledGestures(
                currentGameHandle("TouchPanel.EnabledGestures").requireValue(), output));
        return output[0];
    }

    public static void setTouchPanelEnabledGestures(int value) {
        check("cna_touch_panel_set_enabled_gestures", nativeTouchPanelSetEnabledGestures(
                currentGameHandle("TouchPanel.EnabledGestures").requireValue(), value));
    }

    public static boolean getTouchPanelIsGestureAvailable() {
        return booleanResult("cna_touch_panel_get_is_gesture_available",
                nativeTouchPanelGetIsGestureAvailable(
                        currentGameHandle("TouchPanel.IsGestureAvailable").requireValue()));
    }

    public static WindowHandle getTouchPanelWindowHandle() {
        long[] output = new long[1];
        check("cna_touch_panel_get_window_handle", nativeTouchPanelGetWindowHandle(
                currentGameHandle("TouchPanel.WindowHandle").requireValue(), output));
        return knownWindowHandle(output[0], "TouchPanel.WindowHandle");
    }

    public static void setTouchPanelWindowHandle(WindowHandle window) {
        check("cna_touch_panel_set_window_handle", nativeTouchPanelSetWindowHandle(
                currentGameHandle("TouchPanel.WindowHandle").requireValue(),
                knownWindowValue(window, "TouchPanel.WindowHandle")));
    }

    public static void readTouchGesture(
            int[] type, long[] timestamp, float[] vectors) {
        requireOutputLength(type, 1, "GestureSample type");
        Objects.requireNonNull(timestamp, "timestamp");
        if (timestamp.length != 1) {
            throw new IllegalArgumentException("GestureSample timestamp output must contain 1 value");
        }
        requireOutputLength(vectors, 8, "GestureSample vector");
        check("cna_touch_panel_read_gesture", nativeReadTouchGesture(
                currentGameHandle("TouchPanel.ReadGesture").requireValue(),
                type, timestamp, vectors));
    }

    public static void setTouchDeviceExistsForTests(boolean value) {
        check("cna_touch_panel_set_touch_device_exists_ext",
                nativeSetTouchDeviceExists(
                        currentGameHandle("TouchPanel test device").requireValue(), value));
    }

    public static void setTouchFingerForTests(
            int index, int fingerId, float x, float y) {
        check("cna_touch_panel_set_finger_ext", nativeSetTouchFinger(
                currentGameHandle("TouchPanel test finger").requireValue(),
                index, fingerId, x, y));
    }

    public static void raiseTouchEventForTests(
            int fingerId, int state, float x, float y, float deltaX, float deltaY) {
        check("cna_touch_panel_raise_touch_event_ext", nativeRaiseTouchEvent(
                currentGameHandle("TouchPanel test event").requireValue(),
                fingerId, state, x, y, deltaX, deltaY));
    }

    public static void enqueueTouchGestureForTests(
            int type, long timestampTicks, float[] vectors) {
        Objects.requireNonNull(vectors, "vectors");
        if (vectors.length != 8) {
            throw new IllegalArgumentException("GestureSample vectors must contain 8 values");
        }
        check("cna_touch_panel_enqueue_gesture_ext", nativeEnqueueTouchGesture(
                currentGameHandle("TouchPanel test gesture").requireValue(),
                type, timestampTicks, vectors));
    }

    public static void updateTouchPanelForTests() {
        check("cna_touch_panel_update_ext", nativeUpdateTouchPanel(
                currentGameHandle("TouchPanel test update").requireValue()));
    }

    /**
     * Clears the text-input subsystem's own state.
     *
     * <p>A CNA test seam, reached only from this projection's tests: the composition buffer, the
     * candidate list and the bound window survive a game, so a suite that raised a composition
     * would otherwise leave it for the next one to find.
     */
    public static void resetTextInputForTests() {
        check("cna_text_input_reset_for_tests_ext",
                org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes.textInputResetForTestsExt(
                        currentGameHandle("TextInput test reset").requireValue()));
    }

    /**
     * Clears the joystick subsystem's enumerated devices.
     *
     * <p>The same kind of seam: the enumerated joysticks survive a game, so a suite that raised
     * a hot-plug event would otherwise leave the device for the next one to find.
     */
    public static void resetJoysticksForTests() {
        check("cna_joysticks_reset_for_tests_ext",
                org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes
                        .joysticksResetForTestsExt(
                                currentGameHandle("Joysticks test reset").requireValue()));
    }

    public static void resetTouchPanelForTests() {
        check("cna_touch_panel_reset_for_tests_ext", nativeResetTouchPanel(
                currentGameHandle("TouchPanel test reset").requireValue()));
    }

    public static NativeGraphicsDeviceManagerHandle createGraphicsDeviceManager(
            NativeGameHandle game,
            GraphicsDeviceManager manager) {
        long[] output = new long[1];
        check("cna_graphics_device_manager_create", nativeCreateGraphicsDeviceManager(
                game.requireValue(), Objects.requireNonNull(manager, "manager"), output));
        return new NativeGraphicsDeviceManagerHandle(output[0], manager);
    }

    public static int getGraphicsDeviceManagerGraphicsProfile(
            NativeGraphicsDeviceManagerHandle manager) {
        return Math.toIntExact(longResult("cna_graphics_device_manager_get_graphics_profile",
                nativeGetGraphicsDeviceManagerGraphicsProfile(manager.requireValue())));
    }

    public static void setGraphicsDeviceManagerGraphicsProfile(
            NativeGraphicsDeviceManagerHandle manager,
            int value) {
        check("cna_graphics_device_manager_set_graphics_profile",
                nativeSetGraphicsDeviceManagerGraphicsProfile(manager.requireValue(), value));
    }

    public static boolean getGraphicsDeviceManagerIsFullScreen(
            NativeGraphicsDeviceManagerHandle manager) {
        return booleanResult("cna_graphics_device_manager_get_is_full_screen",
                nativeGetGraphicsDeviceManagerIsFullScreen(manager.requireValue()));
    }

    public static void setGraphicsDeviceManagerIsFullScreen(
            NativeGraphicsDeviceManagerHandle manager,
            boolean value) {
        check("cna_graphics_device_manager_set_is_full_screen",
                nativeSetGraphicsDeviceManagerIsFullScreen(manager.requireValue(), value));
    }

    public static boolean getGraphicsDeviceManagerPreferMultiSampling(
            NativeGraphicsDeviceManagerHandle manager) {
        return booleanResult("cna_graphics_device_manager_get_prefer_multi_sampling",
                nativeGetGraphicsDeviceManagerPreferMultiSampling(manager.requireValue()));
    }

    public static void setGraphicsDeviceManagerPreferMultiSampling(
            NativeGraphicsDeviceManagerHandle manager,
            boolean value) {
        check("cna_graphics_device_manager_set_prefer_multi_sampling",
                nativeSetGraphicsDeviceManagerPreferMultiSampling(manager.requireValue(), value));
    }

    public static int getGraphicsDeviceManagerPreferredBackBufferFormat(
            NativeGraphicsDeviceManagerHandle manager) {
        return Math.toIntExact(longResult(
                "cna_graphics_device_manager_get_preferred_back_buffer_format",
                nativeGetGraphicsDeviceManagerPreferredBackBufferFormat(manager.requireValue())));
    }

    public static void setGraphicsDeviceManagerPreferredBackBufferFormat(
            NativeGraphicsDeviceManagerHandle manager,
            int value) {
        check("cna_graphics_device_manager_set_preferred_back_buffer_format",
                nativeSetGraphicsDeviceManagerPreferredBackBufferFormat(
                        manager.requireValue(), value));
    }

    public static int getGraphicsDeviceManagerPreferredBackBufferWidth(
            NativeGraphicsDeviceManagerHandle manager) {
        return Math.toIntExact(longResult(
                "cna_graphics_device_manager_get_preferred_back_buffer_width",
                nativeGetGraphicsDeviceManagerPreferredBackBufferWidth(manager.requireValue())));
    }

    public static void setGraphicsDeviceManagerPreferredBackBufferWidth(
            NativeGraphicsDeviceManagerHandle manager,
            int value) {
        check("cna_graphics_device_manager_set_preferred_back_buffer_width",
                nativeSetGraphicsDeviceManagerPreferredBackBufferWidth(
                        manager.requireValue(), value));
    }

    public static int getGraphicsDeviceManagerPreferredBackBufferHeight(
            NativeGraphicsDeviceManagerHandle manager) {
        return Math.toIntExact(longResult(
                "cna_graphics_device_manager_get_preferred_back_buffer_height",
                nativeGetGraphicsDeviceManagerPreferredBackBufferHeight(manager.requireValue())));
    }

    public static void setGraphicsDeviceManagerPreferredBackBufferHeight(
            NativeGraphicsDeviceManagerHandle manager,
            int value) {
        check("cna_graphics_device_manager_set_preferred_back_buffer_height",
                nativeSetGraphicsDeviceManagerPreferredBackBufferHeight(
                        manager.requireValue(), value));
    }

    public static int getGraphicsDeviceManagerPreferredDepthStencilFormat(
            NativeGraphicsDeviceManagerHandle manager) {
        return Math.toIntExact(longResult(
                "cna_graphics_device_manager_get_preferred_depth_stencil_format",
                nativeGetGraphicsDeviceManagerPreferredDepthStencilFormat(manager.requireValue())));
    }

    public static void setGraphicsDeviceManagerPreferredDepthStencilFormat(
            NativeGraphicsDeviceManagerHandle manager,
            int value) {
        check("cna_graphics_device_manager_set_preferred_depth_stencil_format",
                nativeSetGraphicsDeviceManagerPreferredDepthStencilFormat(
                        manager.requireValue(), value));
    }

    public static boolean getGraphicsDeviceManagerSynchronizeWithVerticalRetrace(
            NativeGraphicsDeviceManagerHandle manager) {
        return booleanResult(
                "cna_graphics_device_manager_get_synchronize_with_vertical_retrace",
                nativeGetGraphicsDeviceManagerSynchronizeWithVerticalRetrace(
                        manager.requireValue()));
    }

    public static void setGraphicsDeviceManagerSynchronizeWithVerticalRetrace(
            NativeGraphicsDeviceManagerHandle manager,
            boolean value) {
        check("cna_graphics_device_manager_set_synchronize_with_vertical_retrace",
                nativeSetGraphicsDeviceManagerSynchronizeWithVerticalRetrace(
                        manager.requireValue(), value));
    }

    public static int getGraphicsDeviceManagerSupportedOrientations(
            NativeGraphicsDeviceManagerHandle manager) {
        return Math.toIntExact(longResult(
                "cna_graphics_device_manager_get_supported_orientations",
                nativeGetGraphicsDeviceManagerSupportedOrientations(manager.requireValue())));
    }

    public static void setGraphicsDeviceManagerSupportedOrientations(
            NativeGraphicsDeviceManagerHandle manager,
            int value) {
        check("cna_graphics_device_manager_set_supported_orientations",
                nativeSetGraphicsDeviceManagerSupportedOrientations(
                        manager.requireValue(), value));
    }

    public static void applyGraphicsDeviceManagerChanges(
            NativeGraphicsDeviceManagerHandle manager) {
        check("cna_graphics_device_manager_apply_changes",
                nativeApplyGraphicsDeviceManagerChanges(manager.requireValue()));
    }

    public static void toggleGraphicsDeviceManagerFullScreen(
            NativeGraphicsDeviceManagerHandle manager) {
        check("cna_graphics_device_manager_toggle_full_screen",
                nativeToggleGraphicsDeviceManagerFullScreen(manager.requireValue()));
    }

    public static void createGraphicsDeviceManagerDevice(
            NativeGraphicsDeviceManagerHandle manager) {
        check("cna_graphics_device_manager_create_device",
                nativeCreateGraphicsDeviceManagerDevice(manager.requireValue()));
    }

    public static boolean beginGraphicsDeviceManagerDraw(
            NativeGraphicsDeviceManagerHandle manager) {
        return booleanResult("cna_graphics_device_manager_begin_draw",
                nativeBeginGraphicsDeviceManagerDraw(manager.requireValue()));
    }

    public static void endGraphicsDeviceManagerDraw(
            NativeGraphicsDeviceManagerHandle manager) {
        check("cna_graphics_device_manager_end_draw",
                nativeEndGraphicsDeviceManagerDraw(manager.requireValue()));
    }

    public static void disposeGraphicsDeviceManager(
            NativeGraphicsDeviceManagerHandle manager) {
        check("cna_graphics_device_manager_dispose",
                nativeDisposeGraphicsDeviceManager(manager.requireValue()));
    }

    public static int getGraphicsAdapterCount() {
        return Math.toIntExact(longResult("cna_graphics_adapter_get_count",
                nativeGetGraphicsAdapterCount(
                        currentGameHandle("GraphicsAdapter.Adapters").requireValue())));
    }

    public static long[] getGraphicsAdapterInfo(int adapterIndex) {
        long[] output = new long[10];
        check("cna_graphics_adapter_get_info", nativeGetGraphicsAdapterInfo(
                currentGameHandle("GraphicsAdapter").requireValue(), adapterIndex, output));
        return output;
    }

    public static String getGraphicsAdapterDescription(int adapterIndex, long byteLength) {
        return getGraphicsAdapterString(adapterIndex, byteLength, true);
    }

    public static String getGraphicsAdapterDeviceName(int adapterIndex, long byteLength) {
        return getGraphicsAdapterString(adapterIndex, byteLength, false);
    }

    public static int[] getGraphicsAdapterCurrentDisplayMode(int adapterIndex) {
        int[] output = new int[4];
        check("cna_graphics_adapter_get_current_display_mode",
                nativeGetGraphicsAdapterCurrentDisplayMode(
                        currentGameHandle("GraphicsAdapter.CurrentDisplayMode").requireValue(),
                        adapterIndex, output));
        return output;
    }

    public static int[][] getGraphicsAdapterDisplayModes(int adapterIndex) {
        int count = Math.toIntExact(longResult("cna_graphics_adapter_get_display_mode_count",
                nativeGetGraphicsAdapterDisplayModeCount(
                        currentGameHandle("GraphicsAdapter.SupportedDisplayModes").requireValue(),
                        adapterIndex)));
        int[] packed = new int[Math.multiplyExact(count, 4)];
        check("cna_graphics_adapter_copy_display_modes", nativeCopyGraphicsAdapterDisplayModes(
                currentGameHandle("GraphicsAdapter.SupportedDisplayModes").requireValue(),
                adapterIndex, packed));
        int[][] output = new int[count][4];
        for (int index = 0; index < count; index++) {
            System.arraycopy(packed, index * 4, output[index], 0, 4);
        }
        return output;
    }

    public static void setGraphicsAdapterDevicePreferences(
            int adapterIndex,
            boolean useNullDevice,
            boolean useReferenceDevice) {
        check("cna_graphics_adapter_set_device_preferences",
                nativeSetGraphicsAdapterDevicePreferences(
                        currentGameHandle("GraphicsAdapter device preferences").requireValue(),
                        adapterIndex, useNullDevice, useReferenceDevice));
    }

    public static boolean isGraphicsAdapterProfileSupported(int adapterIndex, int profile) {
        return booleanResult("cna_graphics_adapter_is_profile_supported",
                nativeIsGraphicsAdapterProfileSupported(
                        currentGameHandle("GraphicsAdapter.IsProfileSupported").requireValue(),
                        adapterIndex, profile));
    }

    public static int[] queryGraphicsAdapterFormat(
            int adapterIndex,
            boolean backBuffer,
            int profile,
            int format,
            int depthFormat,
            int multiSampleCount) {
        int[] output = new int[4];
        check(backBuffer
                        ? "cna_graphics_adapter_query_backbuffer_format"
                        : "cna_graphics_adapter_query_render_target_format",
                nativeQueryGraphicsAdapterFormat(
                        currentGameHandle("GraphicsAdapter format query").requireValue(),
                        adapterIndex, backBuffer, profile, format, depthFormat,
                        multiSampleCount, output));
        return output;
    }

    public static WindowHandle getGraphicsAdapterMonitorHandle(int adapterIndex) {
        long[] output = new long[1];
        check("cna_graphics_adapter_get_native_monitor_handle",
                nativeGetGraphicsAdapterMonitorHandle(
                        currentGameHandle("GraphicsAdapter.MonitorHandle").requireValue(),
                        adapterIndex, output));
        if (output[0] == 0L) {
            return WindowHandle.Zero;
        }
        synchronized (GAMES) {
            WindowHandle known = findWindowHandle(output[0]);
            if (known != null) {
                return known;
            }
        }
        throw new IllegalStateException("CNA returned an unrecognized opaque monitor token");
    }

    public static void setMousePosition(int x, int y) {
        check("cna_mouse_set_position",
                nativeSetMousePosition(currentGameHandle("Mouse.SetPosition").requireValue(), x, y));
    }

    public static WindowHandle getMouseWindowHandle() {
        long[] output = new long[1];
        check("cna_mouse_get_window_handle", nativeGetMouseWindowHandle(
                currentGameHandle("Mouse.WindowHandle").requireValue(), output));
        if (output[0] == 0L) {
            return WindowHandle.Zero;
        }
        synchronized (GAMES) {
            WindowHandle known = findWindowHandle(output[0]);
            if (known != null) {
                return known;
            }
        }

        Game game;
        synchronized (GAMES) {
            game = currentGame;
        }
        WindowHandle gameWindow = game.getWindow().getHandle();
        synchronized (GAMES) {
            Long value = WINDOW_HANDLES.get(gameWindow);
            if (value != null && value == output[0]) {
                return gameWindow;
            }
        }
        throw new IllegalStateException("CNA returned an unrecognized opaque mouse window token");
    }

    public static void setMouseWindowHandle(WindowHandle window) {
        long value;
        synchronized (GAMES) {
            Long registered = WINDOW_HANDLES.get(Objects.requireNonNull(window, "window"));
            if (registered == null && !window.getIsZero()) {
                throw new IllegalArgumentException(
                        "Mouse.WindowHandle accepts only an opaque token issued by CNA-Java");
            }
            value = registered == null ? 0L : registered;
        }
        check("cna_mouse_set_window_handle", nativeSetMouseWindowHandle(
                currentGameHandle("Mouse.WindowHandle").requireValue(), value));
    }

    public static void updateFrameworkDispatcher() {
        check("cna_framework_dispatcher_update", nativeUpdateFrameworkDispatcher(
                currentGameHandle("FrameworkDispatcher.Update").requireValue()));
        NativeMedia.dispatchPendingEvents();
        NativeStorage.dispatchPendingEvents();
        // XNA pumps its framework services here, and CNA's typed-character events are one of
        // them: a game that never touches gamer services still expects text it typed to arrive.
        GamerEventPump.drain();
        // And this is where a thread-affine handle whose Java owner has been collected goes
        // back to CNA, because this is the thread CNA will accept it from.
        NativeDeferredRelease.drain();
    }

    /** Initializes the selected-profile GamerServices component against its owning game. */
    public static void initializeGamerServices(Game game) {
        Objects.requireNonNull(game, "game").getWindow().getHandle();
        long gameValue = gameHandle(game, "GamerServicesComponent.Initialize").requireValue();
        long windowValue = getWindowHandle(game);
        check("cna_gamer_services_dispatcher_set_window_handle",
                nativeSetGamerServicesWindowHandle(windowValue));
        initializedGamerServices(nativeInitializeGamerServices(gameValue));
    }

    /** Pumps the selected-profile GamerServices dispatcher for a live owning game. */
    public static void updateGamerServices(Game game) {
        gameHandle(game, "GamerServicesComponent.Update").requireValue();
        check("cna_gamer_services_dispatcher_update", nativeUpdateGamerServices());
    }

    /** Initializes the process-wide GamerServices dispatcher against the running game. */
    public static void initializeGamerServicesDispatcher() {
        initializedGamerServices(nativeInitializeGamerServices(
                currentGameHandle("GamerServicesDispatcher.Initialize").requireValue()));
    }

    /**
     * Maps the dispatcher's initialize result. The dispatcher is process-wide and XNA refuses a
     * second initialization with {@code InvalidOperationException}; CNA answers that case with
     * {@code INVALID_STATE}, which is Java's {@link IllegalStateException}.
     */
    private static void initializedGamerServices(int result) {
        if (result == 3) {
            throw new IllegalStateException("Gamer services are already initialized.",
                    failure("cna_gamer_services_dispatcher_initialize", result));
        }
        check("cna_gamer_services_dispatcher_initialize", result);
    }

    /** Pumps the process-wide GamerServices dispatcher. */
    public static void updateGamerServicesDispatcher() {
        check("cna_gamer_services_dispatcher_update", nativeUpdateGamerServices());
    }

    /** Publishes the window token the Guide draws over. */
    public static void setGamerServicesWindowHandle(WindowHandle window) {
        check("cna_gamer_services_dispatcher_set_window_handle", nativeSetGamerServicesWindowHandle(
                knownWindowValue(window, "GamerServicesDispatcher.WindowHandle")));
    }

    /**
     * Returns the running game's CNA handle, for the CNA extension surface.
     *
     * <p>The adapter's own routes take its wrapper token and reach the CNA handle inside it; a
     * generated route takes the CNA handle directly, so it is converted here rather than
     * handing CNA a pointer it would reject.
     */
    public static long currentGameHandleValue(String owner) {
        return NativeGamerServices.nativeCnaGameHandle(currentGameValue(owner));
    }

    /** Resolves an opaque native window token back to its registered Java identity. */
    public static WindowHandle windowHandle(long value) {
        return knownWindowHandle(value, "GamerServicesDispatcher.WindowHandle");
    }

    /** Creates a non-owning facade for a VideoPlayer-owned transient frame texture. */
    public static Texture2D createBorrowedVideoTexture(
            GraphicsDevice graphicsDevice, long nativeTexture) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        if (nativeTexture == 0L) throw new IllegalArgumentException("nativeTexture");
        Texture2D texture = FacadeFactory.createUninitializedTexture2D(graphicsDevice);
        Game game = deviceGame(graphicsDevice);
        NativeResourceHandle handle = new NativeResourceHandle(
                nativeTexture, NativeHandle.Ownership.PARENT_OWNED, ignored -> { });
        synchronized (GAMES) {
            RESOURCES.put(texture, handle);
            RESOURCE_OWNERS.put(texture, game);
            GAME_RESOURCES.computeIfAbsent(game, ignored -> new ArrayList<>()).add(texture);
        }
        try {
            FacadeFactory.initializeTexture2D(texture, textureInfoOrClose(texture));
            return texture;
        } catch (RuntimeException failure) {
            closeAfterFailedFacade(texture, failure);
            throw failure;
        }
    }

    /**
     * Pins a Java object so native code may call it after the registering call returns.
     *
     * <p>A callback registered once and run later -- a tray entry's click handler, a sensor
     * subscription -- needs a reference that outlives the registering call. CNA offers no
     * unregistration hook beyond passing a null callback, so the reference is made explicit rather
     * than hidden: this returns it as an opaque token, the caller stores it beside the
     * registration, and {@link #releaseCallbackToken(long)} deletes it.
     *
     * @param callback the object to pin
     * @return the token, or zero for a null callback
     */
    public static long newCallbackToken(Object callback) {
        requireAvailable();
        return nativeCallbackTokenCreate(callback);
    }

    /**
     * Releases a token from {@link #newCallbackToken(Object)}.
     *
     * <p>Releasing zero is a no-op, and releasing a token twice would delete a reference that is
     * already gone, so a caller keeps one owner per token.
     *
     * @param token the token to release
     */
    public static void releaseCallbackToken(long token) {
        if (token == 0L) {
            return;
        }
        requireAvailable();
        nativeCallbackTokenRelease(token);
    }

    private static native long nativeCallbackTokenCreate(Object callback);

    private static native void nativeCallbackTokenRelease(long token);

    /**
     * Adds a menu entry to a system tray icon, with an optional click handler.
     *
     * <p>Hand-written rather than generated because CNA takes a C function pointer for the
     * handler. The handler's lifetime is the tray's: a person may pick the entry at any time
     * until the tray is closed, so the token is held by the tray and released when it closes.
     *
     * @param tray the native tray handle
     * @param label the entry's label as UTF-8 bytes
     * @param checkable whether the entry carries a check mark
     * @param initiallyChecked whether a checkable entry starts checked
     * @param initiallyEnabled whether the entry starts enabled
     * @param token a token from {@link #newCallbackToken(Object)} over a {@link Runnable}, or
     *        zero for an entry with no handler
     * @param outIndex receives the entry's zero-based index
     * @return CNA's result
     */
    public static int systemTrayAddEntry(long tray, byte[] label, boolean checkable,
            boolean initiallyChecked, boolean initiallyEnabled, long token, long[] outIndex) {
        requireAvailable();
        return nativeSystemTrayAddEntry(tray, label, checkable, initiallyChecked,
                initiallyEnabled, token, outIndex);
    }

    /**
     * Shows a file dialog and reports the chosen paths to a one-shot handler.
     *
     * <p>Hand-written for the function pointer, and one-shot for a reason the header states:
     * CNA's dialog is asynchronous, so the handler may run long after this returns, or never if
     * the process exits first. The native trampoline therefore owns the token and deletes it the
     * moment it dispatches -- there is nothing for Java to release, and nothing it could
     * sensibly release it at. A refused request is the one case the handler never runs, and the
     * adapter releases the token itself when CNA answers anything but success.
     *
     * <p>The handler receives {@code byte[][]}: CNA hands over UTF-8 and the JVM's own
     * {@code NewStringUTF} takes modified UTF-8, so a path outside the basic multilingual plane
     * would be corrupted silently by the shorter route.
     *
     * @param game the native game handle
     * @param kind 0 to open files, 1 to save a file, 2 to open folders
     * @param token a token from {@link #newCallbackToken(Object)} over a
     *        {@code Consumer<byte[][]>}; zero passes no handler, which CNA refuses
     * @param filterNames each filter's display name as UTF-8 bytes, or null for none
     * @param filterPatterns each filter's pattern, one per name
     * @param defaultLocation the starting directory as UTF-8 bytes, empty for the default
     * @param allowMultiple whether more than one path may be chosen; ignored when saving
     * @return CNA's result
     */
    public static int fileDialogShow(long game, int kind, long token, byte[][] filterNames,
            byte[][] filterPatterns, byte[] defaultLocation, boolean allowMultiple) {
        requireAvailable();
        return nativeFileDialogShow(game, kind, token, filterNames, filterPatterns,
                defaultLocation, allowMultiple);
    }

    private static native int nativeSystemTrayAddEntry(long tray, byte[] label, boolean checkable,
            boolean initiallyChecked, boolean initiallyEnabled, long token, long[] outIndex);

    private static native int nativeFileDialogShow(long game, int kind, long token,
            byte[][] filterNames, byte[][] filterPatterns, byte[] defaultLocation,
            boolean allowMultiple);

    /**
     * Encodes one animation clip as a whole {@code .cnb} file.
     *
     * <p>Hand-written rather than generated because CNA takes the clip as a pointer graph -- a
     * descriptor pointing at track descriptors, each pointing at keyframes -- and nothing in the
     * C says which keyframes belong to which track. The generator refuses that, correctly. The
     * lifetimes are not unknown though, only underivable: every array is borrowed for the call,
     * so the graph is built for its duration and freed after it.
     *
     * <p>The flattening is the only one that loses nothing: one bone index and one keyframe count
     * per track, then every keyframe's time and its ten floats end to end. The adapter checks
     * that the counts sum to exactly the keyframes given before it allocates anything, because a
     * sum that does not add up would make CNA read past an array's end from inside C.
     *
     * @param durationSeconds how long the clip runs
     * @param boneIndices which bone each track drives, one per track
     * @param keyframeCounts how many keyframes each track has, one per track
     * @param times each keyframe's time, in track order
     * @param values ten floats per keyframe: translation, rotation, scale
     * @param targetSpace which index space the bone indices live in
     * @param contentName the logical content name to record, as UTF-8 bytes
     * @param destination receives the file; may be empty to ask only for the size
     * @param outByteCount receives the size the file needs, written either way
     * @return CNA's result
     */
    public static int cnbEncodeAnimationClip(double durationSeconds, int[] boneIndices,
            int[] keyframeCounts, double[] times, float[] values, int targetSpace,
            byte[] contentName, byte[] destination, long[] outByteCount) {
        requireAvailable();
        return nativeCnbEncodeAnimationClip(durationSeconds, boneIndices, keyframeCounts, times,
                values, targetSpace, contentName, destination, outByteCount);
    }

    /**
     * Adds one named animation clip to a {@code .cnb} model being built.
     *
     * <p>The same pointer graph, and the same reason it is hand-written. The model deeply copies
     * the clip, so nothing here outlives the call.
     *
     * @param model the native model-data handle
     * @param clipName the clip's name as UTF-8 bytes
     * @param durationSeconds how long the clip runs
     * @param boneIndices which bone each track drives, one per track
     * @param keyframeCounts how many keyframes each track has, one per track
     * @param times each keyframe's time, in track order
     * @param values ten floats per keyframe
     * @param targetSpace which index space the bone indices live in
     * @param outIndex receives the clip's index in the model
     * @return CNA's result
     */
    public static int cnbModelAddAnimation(long model, byte[] clipName, double durationSeconds,
            int[] boneIndices, int[] keyframeCounts, double[] times, float[] values,
            int targetSpace, long[] outIndex) {
        requireAvailable();
        return nativeCnbModelAddAnimation(model, clipName, durationSeconds, boneIndices,
                keyframeCounts, times, values, targetSpace, outIndex);
    }

    private static native int nativeCnbEncodeAnimationClip(double durationSeconds,
            int[] boneIndices, int[] keyframeCounts, double[] times, float[] values,
            int targetSpace, byte[] contentName, byte[] destination, long[] outByteCount);

    private static native int nativeCnbModelAddAnimation(long model, byte[] clipName,
            double durationSeconds, int[] boneIndices, int[] keyframeCounts, double[] times,
            float[] values, int targetSpace, long[] outIndex);

    /**
     * Creates a named set of animation clips from the flattened clip graph.
     *
     * <p>The same pointer graph one level deeper -- named clips to clips to tracks to keyframes
     * -- and hand-written for the same reason. CNA copies every clip, so nothing here outlives
     * the call.
     *
     * @param names each clip's name as UTF-8 bytes
     * @param durations each clip's duration
     * @param clipTrackCounts how many tracks each clip has
     * @param boneIndices which bone each track drives, across every clip in order
     * @param keyframeCounts how many keyframes each track has
     * @param times each keyframe's time, in track order
     * @param values ten floats per keyframe
     * @param outAnimations receives the owned handle
     * @return CNA's result
     */
    public static int modelAnimationsCreate(byte[][] names, double[] durations,
            int[] clipTrackCounts, int[] boneIndices, int[] keyframeCounts, double[] times,
            float[] values, long[] outAnimations) {
        requireAvailable();
        return nativeModelAnimationsCreate(names, durations, clipTrackCounts, boneIndices,
                keyframeCounts, times, values, outAnimations);
    }

    private static native int nativeModelAnimationsCreate(byte[][] names, double[] durations,
            int[] clipTrackCounts, int[] boneIndices, int[] keyframeCounts, double[] times,
            float[] values, long[] outAnimations);

    /** The adapter's own subscription kinds; seven routes with six callback types. */
    public static final int SENSOR_ACCELEROMETER_CURRENT = 0;

    /** The accelerometer's obsolete reading-changed event, which carries its own description. */
    public static final int SENSOR_ACCELEROMETER_READING = 1;

    /** The gyroscope's current-value event. */
    public static final int SENSOR_GYROSCOPE_CURRENT = 2;

    /** The compass's current-value event. */
    public static final int SENSOR_COMPASS_CURRENT = 3;

    /** The compass's calibration-requested event, which carries no reading. */
    public static final int SENSOR_COMPASS_CALIBRATE = 4;

    /** The motion sensor's current-value event. */
    public static final int SENSOR_MOTION_CURRENT = 5;

    /** The motion sensor's calibration-requested event, which carries no reading. */
    public static final int SENSOR_MOTION_CALIBRATE = 6;

    /**
     * Subscribes a Java handler to one sensor event.
     *
     * <p>Hand-written rather than generated because each route takes a C function pointer, and
     * the six reading shapes cannot be selected by a handle -- so the kind is this adapter's own
     * constant. The registration outlives the call that made it, so the handler is pinned by a
     * token the registration owns and released when it unsubscribes.
     *
     * <p>The handler receives the reading's leaves as one {@code double[]} in declaration order.
     *
     * @param kind which of the {@code SENSOR_*} subscriptions to make
     * @param sensor the native sensor handle
     * @param token a token from {@link #newCallbackToken(Object)} over a
     *        {@code Consumer<double[]>}
     * @param outRegistration receives the owned registration handle
     * @return CNA's result
     */
    public static int sensorSubscribe(int kind, long sensor, long token, long[] outRegistration) {
        requireAvailable();
        return nativeSensorSubscribe(kind, sensor, token, outRegistration);
    }

    private static native int nativeSensorSubscribe(int kind, long sensor, long token,
            long[] outRegistration);

    /**
     * Creates skinning data from a skeleton and a set of named clips.
     *
     * <p>Hand-written because CNA takes a descriptor whose fields are pointers to arrays of
     * descriptors -- skeleton matrices, and named clips holding tracks holding keyframes -- which
     * the generator refuses. It is right to: nothing in the C says which keyframes belong to
     * which track, nor how many matrices sit behind a bone count. The lifetimes are not unknown,
     * though. CNA documents every array as borrowed for the call and the whole descriptor as
     * deeply copied, so the graph is built here for one call and freed after it.
     *
     * @param parents each bone's parent index, one per bone; its length is the bone count
     * @param bindPose sixteen floats per bone, in CNA's row order
     * @param inverseBindPose sixteen floats per bone
     * @param rootPrefix sixteen floats per bone, or null or empty for none
     * @param names each clip's name as UTF-8 bytes
     * @param durations each clip's duration
     * @param clipTrackCounts how many tracks each clip has
     * @param boneIndices which bone each track drives, across every clip in order
     * @param keyframeCounts how many keyframes each track has
     * @param times each keyframe's time
     * @param values ten floats per keyframe
     * @param outData receives the owned handle
     * @return CNA's result
     */
    public static int skinningDataCreate(int[] parents, float[] bindPose,
            float[] inverseBindPose, float[] rootPrefix, byte[][] names, double[] durations,
            int[] clipTrackCounts, int[] boneIndices, int[] keyframeCounts, double[] times,
            float[] values, long[] outData) {
        requireAvailable();
        return nativeSkinningDataCreate(parents, bindPose, inverseBindPose, rootPrefix, names,
                durations, clipTrackCounts, boneIndices, keyframeCounts, times, values, outData);
    }

    /**
     * Sets a skinned model's skeleton from three parallel arrays.
     *
     * @param model the native skinned-model handle
     * @param parents each bone's parent index; its length is the bone count
     * @param bindPose sixteen floats per bone
     * @param inverseBindPose sixteen floats per bone
     * @return CNA's result
     */
    public static int skinnedModelSetSkeleton(long model, int[] parents, float[] bindPose,
            float[] inverseBindPose) {
        requireAvailable();
        return nativeSkinnedModelSetSkeleton(model, parents, bindPose, inverseBindPose);
    }

    /**
     * Sets one named clip on a skinned model.
     *
     * @param model the native skinned-model handle
     * @param clipName the clip's name as UTF-8 bytes
     * @param durationSeconds how long the clip runs
     * @param boneIndices which bone each track drives
     * @param keyframeCounts how many keyframes each track has
     * @param times each keyframe's time
     * @param values ten floats per keyframe
     * @return CNA's result
     */
    public static int skinnedModelSetClip(long model, byte[] clipName, double durationSeconds,
            int[] boneIndices, int[] keyframeCounts, double[] times, float[] values) {
        requireAvailable();
        return nativeSkinnedModelSetClip(model, clipName, durationSeconds, boneIndices,
                keyframeCounts, times, values);
    }

    private static native int nativeSkinningDataCreate(int[] parents, float[] bindPose,
            float[] inverseBindPose, float[] rootPrefix, byte[][] names, double[] durations,
            int[] clipTrackCounts, int[] boneIndices, int[] keyframeCounts, double[] times,
            float[] values, long[] outData);

    private static native int nativeSkinnedModelSetSkeleton(long model, int[] parents,
            float[] bindPose, float[] inverseBindPose);

    private static native int nativeSkinnedModelSetClip(long model, byte[] clipName,
            double durationSeconds, int[] boneIndices, int[] keyframeCounts, double[] times,
            float[] values);

    /**
     * Creates morph-target data from its whole descriptor graph.
     *
     * <p>The third and widest graph this adapter marshals by hand, and hand-written for the same
     * reason as the other two: C states every count as a separate parameter and Java carries each
     * in its own array, so nothing derives which deltas belong to which target or which weights
     * to which keyframe. Every relationship is checked before anything is built.
     *
     * @param baseVertexBytes the base pose's vertex bytes
     * @param stride one base vertex's byte stride
     * @param positionCounts how many position deltas each target has
     * @param positionDeltas three floats per delta, across every target in order
     * @param normalCounts how many normal deltas each target has
     * @param normalDeltas three floats per delta
     * @param weights the current weight per target
     * @param trackTimes each weight keyframe's time
     * @param trackWeightCounts how many weights each keyframe carries
     * @param trackWeights the weights, across every keyframe in order
     * @param trackInCounts how many incoming tangents each keyframe carries
     * @param trackInTangents the incoming tangents
     * @param trackOutCounts how many outgoing tangents each keyframe carries
     * @param trackOutTangents the outgoing tangents
     * @param stepInterpolation whether evaluation holds the lower keyframe's value
     * @param cubicSpline whether evaluation uses the tangents
     * @param outData receives the owned handle
     * @return CNA's result
     */
    public static int morphTargetDataCreate(byte[] baseVertexBytes, int stride,
            int[] positionCounts, float[] positionDeltas, int[] normalCounts,
            float[] normalDeltas, float[] weights, double[] trackTimes, int[] trackWeightCounts,
            float[] trackWeights, int[] trackInCounts, float[] trackInTangents,
            int[] trackOutCounts, float[] trackOutTangents, boolean stepInterpolation,
            boolean cubicSpline, long[] outData) {
        requireAvailable();
        return nativeMorphTargetDataCreate(baseVertexBytes, stride, positionCounts,
                positionDeltas, normalCounts, normalDeltas, weights, trackTimes,
                trackWeightCounts, trackWeights, trackInCounts, trackInTangents, trackOutCounts,
                trackOutTangents, stepInterpolation, cubicSpline, outData);
    }

    /**
     * Replaces the weight track morph data animates its weights from.
     *
     * @param data the native morph-data handle
     * @param times each keyframe's time
     * @param weightCounts how many weights each keyframe carries
     * @param weights the weights, across every keyframe in order
     * @param inCounts how many incoming tangents each keyframe carries
     * @param inTangents the incoming tangents
     * @param outCounts how many outgoing tangents each keyframe carries
     * @param outTangents the outgoing tangents
     * @param stepInterpolation whether evaluation holds the lower keyframe's value
     * @param cubicSpline whether evaluation uses the tangents
     * @return CNA's result
     */
    public static int morphTargetDataSetWeightTrack(long data, double[] times, int[] weightCounts,
            float[] weights, int[] inCounts, float[] inTangents, int[] outCounts,
            float[] outTangents, boolean stepInterpolation, boolean cubicSpline) {
        requireAvailable();
        return nativeMorphTargetDataSetWeightTrack(data, times, weightCounts, weights, inCounts,
                inTangents, outCounts, outTangents, stepInterpolation, cubicSpline);
    }

    /**
     * Evaluates a standalone weight track at a time, without any morph data.
     *
     * @param times each keyframe's time
     * @param weightCounts how many weights each keyframe carries
     * @param weights the weights, across every keyframe in order
     * @param inCounts how many incoming tangents each keyframe carries
     * @param inTangents the incoming tangents
     * @param outCounts how many outgoing tangents each keyframe carries
     * @param outTangents the outgoing tangents
     * @param stepInterpolation whether evaluation holds the lower keyframe's value
     * @param cubicSpline whether evaluation uses the tangents
     * @param timeSeconds when to evaluate
     * @param destination receives the weights; may be empty to ask only for the count
     * @param outWeightCount receives the count, written either way
     * @return CNA's result
     */
    public static int morphWeightTrackEvaluate(double[] times, int[] weightCounts,
            float[] weights, int[] inCounts, float[] inTangents, int[] outCounts,
            float[] outTangents, boolean stepInterpolation, boolean cubicSpline,
            double timeSeconds, float[] destination, long[] outWeightCount) {
        requireAvailable();
        return nativeMorphWeightTrackEvaluate(times, weightCounts, weights, inCounts, inTangents,
                outCounts, outTangents, stepInterpolation, cubicSpline, timeSeconds, destination,
                outWeightCount);
    }

    private static native int nativeMorphTargetDataCreate(byte[] baseVertexBytes, int stride,
            int[] positionCounts, float[] positionDeltas, int[] normalCounts,
            float[] normalDeltas, float[] weights, double[] trackTimes, int[] trackWeightCounts,
            float[] trackWeights, int[] trackInCounts, float[] trackInTangents,
            int[] trackOutCounts, float[] trackOutTangents, boolean stepInterpolation,
            boolean cubicSpline, long[] outData);

    private static native int nativeMorphTargetDataSetWeightTrack(long data, double[] times,
            int[] weightCounts, float[] weights, int[] inCounts, float[] inTangents,
            int[] outCounts, float[] outTangents, boolean stepInterpolation, boolean cubicSpline);

    private static native int nativeMorphWeightTrackEvaluate(double[] times, int[] weightCounts,
            float[] weights, int[] inCounts, float[] inTangents, int[] outCounts,
            float[] outTangents, boolean stepInterpolation, boolean cubicSpline,
            double timeSeconds, float[] destination, long[] outWeightCount);

    /**
     * Records one glTF import diagnostic on a model.
     *
     * <p>Hand-written for a shape the generator refuses inside a structure rather than beside
     * one: the descriptor carries two string views, which it handles, and a pointer to an array
     * of them, which it does not -- those would have to outlive one element's marshalling. Here
     * they do, and only for the call.
     *
     * @param model the native model handle
     * @param code the stable diagnostic identifier as UTF-8 bytes
     * @param severity one CNA severity identity
     * @param kind one CNA diagnostic-kind identity
     * @param subject what the entry concerns, as UTF-8 bytes; may be empty
     * @param count how many occurrences the entry represents
     * @param worstMagnitude the largest measured magnitude, or zero
     * @param details each affected name as UTF-8 bytes
     * @param message the human-readable message as UTF-8 bytes
     * @return CNA's result
     */
    public static int modelAddGltfImportDiagnostic(long model, byte[] code, int severity,
            int kind, byte[] subject, long count, double worstMagnitude, byte[][] details,
            byte[] message) {
        requireAvailable();
        return nativeModelAddGltfImportDiagnostic(model, code, severity, kind, subject, count,
                worstMagnitude, details, message);
    }

    private static native int nativeModelAddGltfImportDiagnostic(long model, byte[] code,
            int severity, int kind, byte[] subject, long count, double worstMagnitude,
            byte[][] details, byte[] message);

    /**
     * Installs or clears the process-wide sink CNA writes its log lines to.
     *
     * <p>Hand-written because CNA's sink is a C function pointer, which the generator refuses
     * rather than guesses at, and because the reference discipline is the whole risk here: the
     * adapter holds one global reference to the sink and deletes it only after CNA has stopped
     * calling through it.
     *
     * @param sink an object with {@code void acceptNativeLine(int, int, byte[])}, or null to
     *     restore CNA's own stderr sink
     * @return CNA's result
     */
    public static int loggerSetSink(Object sink) {
        requireAvailable();
        return nativeLoggerSetSink(sink);
    }

    private static native int nativeLoggerSetSink(Object sink);


    /**
     * Gives up ownership of a resource's native handle without releasing it.
     *
     * <p>For the one shape CNA has where a call <em>consumes</em> a handle: after a successful
     * transfer the native object belongs to whatever took it, and the Java facade must stop
     * being its owner or the next {@code Dispose} is a double free. The facade is unregistered
     * and behaves as closed; the native object lives on under its new owner.
     *
     * <p>Called only after the transfer has succeeded. A transfer that failed leaves the Java
     * object owning what it always owned, which is what makes the failure recoverable.
     *
     * @param resource the facade to unregister
     */
    public static void surrenderResource(GraphicsResource resource) {
        Objects.requireNonNull(resource, "resource");
        NativeResourceHandle handle;
        synchronized (GAMES) {
            handle = RESOURCES.remove(resource);
            Game owner = RESOURCE_OWNERS.remove(resource);
            List<GraphicsResource> children = GAME_RESOURCES.get(owner);
            if (children != null) {
                children.remove(resource);
            }
        }
        if (handle != null) {
            handle.surrender();
        }
    }

    public static void registerWindowHandle(WindowHandle window, long value) {
        if (value != 0L) {
            synchronized (GAMES) {
                WINDOW_HANDLES.put(Objects.requireNonNull(window, "window"), value);
            }
        }
    }

    public static void registerGraphicsDevice(GraphicsDevice device, Game game) {
        synchronized (GAMES) {
            DEVICES.put(Objects.requireNonNull(device, "device"),
                    Objects.requireNonNull(game, "game"));
        }
    }

    /** Returns whether this facade currently has a live native game parent. */
    public static boolean isGraphicsDeviceNative(GraphicsDevice device) {
        Game game = deviceGame(device);
        synchronized (GAMES) {
            NativeGameHandle handle = GAMES.get(game);
            return handle != null && !handle.isClosed();
        }
    }

    /** Installs all native device callbacks while called from a valid game lifecycle callback. */
    public static void ensureGraphicsDeviceEvents(GraphicsDevice device) {
        Game game = deviceGame(device);
        check("cna_graphics_device_subscribe_*", nativeEnsureGraphicsDeviceEvents(
                gameHandle(game, "GraphicsDevice events").requireValue(), device));
    }

    public static boolean getGraphicsDeviceIsDisposed(GraphicsDevice device) {
        int result = nativeGetGraphicsDeviceIsDisposed(deviceGameValue(device));
        boolean value = booleanResult("cna_graphics_device_get_is_disposed", result);
        rethrowGraphicsDeviceListenerFailure(device);
        return value;
    }

    public static int getGraphicsDeviceStatus(GraphicsDevice device) {
        return deviceIntResult(
                device, "cna_graphics_device_get_status",
                nativeGetGraphicsDeviceStatus(deviceGameValue(device)));
    }

    public static int getGraphicsDeviceAdapterIndex(GraphicsDevice device) {
        return deviceIntResult(
                device, "cna_graphics_device_get_adapter_index",
                nativeGetGraphicsDeviceAdapterIndex(deviceGameValue(device)));
    }

    public static int getGraphicsDeviceProfile(GraphicsDevice device) {
        return deviceIntResult(
                device, "cna_graphics_device_get_graphics_profile",
                nativeGetGraphicsDeviceProfile(deviceGameValue(device)));
    }

    public static void setGraphicsDeviceProfile(GraphicsDevice device, int profile) {
        checkDevice(
                device, "cna_graphics_device_set_graphics_profile_ext",
                nativeSetGraphicsDeviceProfile(deviceGameValue(device), profile));
    }

    public static int[] getGraphicsDeviceScissorRectangle(GraphicsDevice device) {
        int[] output = new int[4];
        checkDevice(
                device, "cna_graphics_device_get_scissor_rectangle",
                nativeGetGraphicsDeviceScissorRectangle(deviceGameValue(device), output));
        return output;
    }

    public static void setGraphicsDeviceScissorRectangle(
            GraphicsDevice device, int x, int y, int width, int height) {
        checkDevice(
                device, "cna_graphics_device_set_scissor_rectangle",
                nativeSetGraphicsDeviceScissorRectangle(
                        deviceGameValue(device), x, y, width, height));
    }

    public static void getGraphicsDeviceViewport(
            GraphicsDevice device, int[] bounds, float[] depth) {
        checkDevice(
                device, "cna_graphics_device_get_viewport",
                nativeGetGraphicsDeviceViewport(
                        deviceGameValue(device),
                        Objects.requireNonNull(bounds, "bounds"),
                        Objects.requireNonNull(depth, "depth")));
    }

    public static void setGraphicsDeviceViewport(
            GraphicsDevice device,
            int x,
            int y,
            int width,
            int height,
            float minDepth,
            float maxDepth) {
        checkDevice(
                device, "cna_graphics_device_set_viewport",
                nativeSetGraphicsDeviceViewport(
                        deviceGameValue(device), x, y, width, height, minDepth, maxDepth));
    }

    public static int getGraphicsDeviceBlendFactor(GraphicsDevice device) {
        long value = deviceLongResult(
                device, "cna_graphics_device_get_blend_factor",
                nativeGetGraphicsDeviceBlendFactor(deviceGameValue(device)));
        return (int)value;
    }

    public static void setGraphicsDeviceBlendFactor(GraphicsDevice device, int packedColor) {
        checkDevice(
                device, "cna_graphics_device_set_blend_factor",
                nativeSetGraphicsDeviceBlendFactor(deviceGameValue(device), packedColor));
    }

    public static int[] getGraphicsDeviceBlendState(GraphicsDevice device) {
        int[] output = new int[12];
        checkDevice(
                device, "cna_graphics_device_get_blend_state",
                nativeGetGraphicsDeviceBlendState(deviceGameValue(device), output));
        return output;
    }

    public static void setGraphicsDeviceBlendState(GraphicsDevice device, int[] values) {
        int[] snapshot = requireLength(values, 12, "BlendState");
        checkDevice(
                device, "cna_graphics_device_set_blend_state",
                nativeSetGraphicsDeviceBlendState(deviceGameValue(device), snapshot));
    }

    public static int[] getGraphicsDeviceDepthStencilState(GraphicsDevice device) {
        int[] output = new int[16];
        checkDevice(
                device, "cna_graphics_device_get_depth_stencil_state",
                nativeGetGraphicsDeviceDepthStencilState(deviceGameValue(device), output));
        return output;
    }

    public static void setGraphicsDeviceDepthStencilState(
            GraphicsDevice device, int[] values) {
        int[] snapshot = requireLength(values, 16, "DepthStencilState");
        checkDevice(
                device, "cna_graphics_device_set_depth_stencil_state",
                nativeSetGraphicsDeviceDepthStencilState(deviceGameValue(device), snapshot));
    }

    public static void getGraphicsDeviceRasterizerState(
            GraphicsDevice device, int[] integers, float[] floats) {
        requireOutputLength(integers, 4, "RasterizerState integer");
        requireOutputLength(floats, 2, "RasterizerState float");
        checkDevice(
                device, "cna_graphics_device_get_rasterizer_state",
                nativeGetGraphicsDeviceRasterizerState(
                        deviceGameValue(device), integers, floats));
    }

    public static void setGraphicsDeviceRasterizerState(
            GraphicsDevice device, int[] integers, float[] floats) {
        int[] integerSnapshot = requireLength(integers, 4, "RasterizerState integer");
        float[] floatSnapshot = requireLength(floats, 2, "RasterizerState float");
        checkDevice(
                device, "cna_graphics_device_set_rasterizer_state",
                nativeSetGraphicsDeviceRasterizerState(
                        deviceGameValue(device), integerSnapshot, floatSnapshot));
    }

    public static void getGraphicsDeviceSamplerState(
            GraphicsDevice device,
            int shaderStage,
            int slot,
            int[] integers,
            float[] bias) {
        requireOutputLength(integers, 6, "SamplerState integer");
        requireOutputLength(bias, 1, "SamplerState bias");
        checkDevice(
                device, "cna_graphics_device_get_sampler_state",
                nativeGetGraphicsDeviceSamplerState(
                        deviceGameValue(device), shaderStage, slot, integers, bias));
    }

    public static void setGraphicsDeviceSamplerState(
            GraphicsDevice device,
            int shaderStage,
            int slot,
            int[] integers,
            float bias) {
        int[] snapshot = requireLength(integers, 6, "SamplerState integer");
        checkDevice(
                device, "cna_graphics_device_set_sampler_state",
                nativeSetGraphicsDeviceSamplerState(
                        deviceGameValue(device), shaderStage, slot, snapshot, bias));
    }

    public static Texture getGraphicsDeviceTexture(
            GraphicsDevice device,
            int shaderStage,
            int slot,
            Texture cached) {
        long[] output = new long[2];
        checkDevice(
                device, "cna_graphics_device_get_texture",
                nativeGetGraphicsDeviceTexture(
                        deviceGameValue(device), shaderStage, slot, output));
        if (output[0] == 0L) {
            return null;
        }
        if (output[0] != 1L) {
            throw new IllegalStateException(
                    "CNA returned invalid texture-slot bound value " + output[0]);
        }
        if (output[1] == 0L) {
            throw new UnsupportedOperationException(
                    "The texture slot is occupied by canonical CNA code without a C resource handle");
        }
        Texture result = findTexture(output[1], cached);
        if (result == null) {
            throw new IllegalStateException(
                    "CNA returned a texture handle not owned by this Java binding");
        }
        return result;
    }

    public static void setGraphicsDeviceTexture(
            GraphicsDevice device,
            int shaderStage,
            int slot,
            Texture texture) {
        long textureHandle = texture == null ? 0L : resourceValue(texture);
        checkDevice(
                device, "cna_graphics_device_set_texture",
                nativeSetGraphicsDeviceTexture(
                        deviceGameValue(device), shaderStage, slot, textureHandle));
    }

    public static int getGraphicsDeviceMultiSampleMask(GraphicsDevice device) {
        return deviceIntResult(
                device, "cna_graphics_device_get_multi_sample_mask",
                nativeGetGraphicsDeviceMultiSampleMask(deviceGameValue(device)));
    }

    public static void setGraphicsDeviceMultiSampleMask(GraphicsDevice device, int value) {
        checkDevice(
                device, "cna_graphics_device_set_multi_sample_mask",
                nativeSetGraphicsDeviceMultiSampleMask(deviceGameValue(device), value));
    }

    public static int getGraphicsDeviceReferenceStencil(GraphicsDevice device) {
        return deviceIntResult(
                device, "cna_graphics_device_get_reference_stencil",
                nativeGetGraphicsDeviceReferenceStencil(deviceGameValue(device)));
    }

    public static void setGraphicsDeviceReferenceStencil(GraphicsDevice device, int value) {
        checkDevice(
                device, "cna_graphics_device_set_reference_stencil",
                nativeSetGraphicsDeviceReferenceStencil(deviceGameValue(device), value));
    }

    public static int[] getGraphicsDevicePresentationParameters(GraphicsDevice device) {
        int[] output = new int[10];
        checkDevice(
                device, "cna_graphics_device_get_presentation_parameters",
                nativeGetGraphicsDevicePresentationParameters(deviceGameValue(device), output));
        return output;
    }

    public static int[] getGraphicsDeviceDisplayMode(GraphicsDevice device) {
        int[] output = new int[4];
        checkDevice(
                device, "cna_graphics_device_get_display_mode",
                nativeGetGraphicsDeviceDisplayMode(deviceGameValue(device), output));
        return output;
    }

    public static int[] getGraphicsDeviceBackBufferInfo(GraphicsDevice device) {
        int[] output = new int[3];
        checkDevice(
                device, "cna_graphics_device_get_backbuffer_info",
                nativeGetGraphicsDeviceBackBufferInfo(deviceGameValue(device), output));
        return output;
    }

    public static int[] getGraphicsDeviceBackBufferData(
            GraphicsDevice device,
            Rectangle rectangle,
            int capacity,
            int startIndex,
            int elementCount) {
        if (capacity < 0) {
            throw new IllegalArgumentException("Backbuffer capacity must not be negative");
        }
        int[] output = new int[capacity];
        boolean hasRectangle = rectangle != null;
        int x = hasRectangle ? rectangle.X : 0;
        int y = hasRectangle ? rectangle.Y : 0;
        int width = hasRectangle ? rectangle.Width : 0;
        int height = hasRectangle ? rectangle.Height : 0;
        checkDevice(
                device, "cna_graphics_device_get_backbuffer_data_window",
                nativeGetGraphicsDeviceBackBufferData(
                        deviceGameValue(device), hasRectangle,
                        x, y, width, height,
                        startIndex, elementCount, output));
        return output;
    }

    public static void clearGraphicsDevice(
            GraphicsDevice device, int options, int packedColor, float depth, int stencil) {
        checkDevice(
                device, "cna_graphics_device_clear_options",
                nativeClearGraphicsDevice(
                        deviceGameValue(device), options, packedColor, depth, stencil));
    }

    public static void presentGraphicsDevice(GraphicsDevice device) {
        checkDevice(
                device, "cna_graphics_device_present",
                nativePresentGraphicsDevice(deviceGameValue(device)));
    }

    public static void resetGraphicsDevice(GraphicsDevice device) {
        checkDevice(
                device, "cna_graphics_device_reset",
                nativeResetGraphicsDevice(deviceGameValue(device)));
    }

    public static void resetGraphicsDevice(
            GraphicsDevice device,
            PresentationParameters parameters,
            int adapterIndex) {
        PresentationParameters snapshot =
                Objects.requireNonNull(parameters, "parameters").Clone();
        Game game = deviceGame(device);
        WindowHandle requestedWindow = snapshot.getDeviceWindowHandle();
        if (!requestedWindow.getIsZero()
                && !requestedWindow.equals(game.getWindow().getHandle())) {
            throw new UnsupportedOperationException(
                    "CNA's C ABI cannot reset a GraphicsDevice onto a different native window");
        }
        int[] values = {
                snapshot.getBackBufferFormat().ordinal(),
                snapshot.getBackBufferWidth(),
                snapshot.getBackBufferHeight(),
                snapshot.getDepthStencilFormat().ordinal(),
                snapshot.getMultiSampleCount(),
                snapshot.getPresentationInterval().ordinal(),
                snapshot.getDisplayOrientation().getValue(),
                snapshot.getRenderTargetUsage().ordinal(),
                snapshot.getIsFullScreen() ? 1 : 0
        };
        checkDevice(
                device, "cna_graphics_device_reset_with_parameters",
                nativeResetGraphicsDeviceWithParameters(
                        gameHandle(game, "GraphicsDevice.Reset").requireValue(),
                        values,
                        adapterIndex));
    }

    public static void setGraphicsDeviceVertexBuffer(
            GraphicsDevice device, VertexBuffer vertexBuffer, int vertexOffset) {
        long handle = 0L;
        if (vertexBuffer != null) {
            requireResourceOwner(device, vertexBuffer);
            handle = resourceValue(vertexBuffer);
        }
        int result = nativeSetGraphicsDeviceVertexBuffer(
                deviceGameValue(device), handle, vertexOffset);
        try {
            checkDevice(device,
                    vertexOffset == 0
                            ? "cna_graphics_device_set_vertex_buffer"
                            : "cna_graphics_device_set_vertex_buffer_offset",
                    result);
        } finally {
            if (result == 0) {
                recordVertexBindings(
                        device, vertexBuffer == null ? List.of() : List.of(vertexBuffer));
            }
        }
    }

    public static void setGraphicsDeviceVertexBuffers(
            GraphicsDevice device, VertexBufferBinding[] bindings) {
        Objects.requireNonNull(bindings, "bindings");
        long[] handles = new long[bindings.length];
        int[] offsets = new int[bindings.length];
        int[] frequencies = new int[bindings.length];
        for (int index = 0; index < bindings.length; index++) {
            VertexBufferBinding binding = Objects.requireNonNull(
                    bindings[index], "bindings[" + index + "]");
            VertexBuffer buffer = Objects.requireNonNull(
                    binding.getVertexBuffer(), "bindings[" + index + "].vertexBuffer");
            requireResourceOwner(device, buffer);
            handles[index] = resourceValue(buffer);
            offsets[index] = binding.getVertexOffset();
            frequencies[index] = binding.getInstanceFrequency();
        }
        int result = nativeSetGraphicsDeviceVertexBuffers(
                deviceGameValue(device), handles, offsets, frequencies);
        try {
            checkDevice(device, "cna_graphics_device_set_vertex_buffers", result);
        } finally {
            if (result == 0) {
                List<VertexBuffer> buffers = new ArrayList<>(bindings.length);
                for (VertexBufferBinding binding : bindings) {
                    buffers.add(binding.getVertexBuffer());
                }
                recordVertexBindings(device, buffers);
            }
        }
    }

    public static VertexBufferBinding[] getGraphicsDeviceVertexBuffers(GraphicsDevice device) {
        int count = Math.toIntExact(longResult(
                "cna_graphics_device_get_vertex_buffer_count",
                nativeGetGraphicsDeviceVertexBufferCount(deviceGameValue(device))));
        long[] handles = new long[count];
        int[] offsets = new int[count];
        int[] frequencies = new int[count];
        checkDevice(device, "cna_graphics_device_copy_vertex_buffers",
                nativeCopyGraphicsDeviceVertexBuffers(
                        deviceGameValue(device), handles, offsets, frequencies));
        VertexBufferBinding[] result = new VertexBufferBinding[count];
        for (int index = 0; index < count; index++) {
            if (handles[index] == 0L) {
                throw new UnsupportedOperationException(
                        "A vertex buffer was bound by canonical CNA code without a C resource handle");
            }
            VertexBuffer buffer = findVertexBuffer(handles[index]);
            if (buffer == null) {
                throw new IllegalStateException(
                        "CNA returned a vertex-buffer handle not owned by CNA-Java");
            }
            result[index] = new VertexBufferBinding(
                    buffer, offsets[index], frequencies[index]);
        }
        return result;
    }

    public static void setGraphicsDeviceIndexBuffer(
            GraphicsDevice device, IndexBuffer indexBuffer) {
        long handle = 0L;
        if (indexBuffer != null) {
            requireResourceOwner(device, indexBuffer);
            handle = resourceValue(indexBuffer);
        }
        int result = nativeSetGraphicsDeviceIndexBuffer(deviceGameValue(device), handle);
        try {
            checkDevice(device, "cna_graphics_device_set_index_buffer", result);
        } finally {
            if (result == 0) {
                synchronized (GAMES) {
                    if (indexBuffer == null) {
                        DEVICE_INDEX_BINDINGS.remove(device);
                    } else {
                        DEVICE_INDEX_BINDINGS.put(device, indexBuffer);
                    }
                }
            }
        }
    }

    public static IndexBuffer getGraphicsDeviceIndexBuffer(GraphicsDevice device) {
        long[] output = new long[1];
        checkDevice(device, "cna_graphics_device_get_index_buffer",
                nativeGetGraphicsDeviceIndexBuffer(deviceGameValue(device), output));
        if (output[0] == 0L) {
            return null;
        }
        IndexBuffer result = findIndexBuffer(output[0]);
        if (result == null) {
            throw new UnsupportedOperationException(
                    "The index buffer was bound by canonical CNA code without a C resource handle");
        }
        return result;
    }

    public static void drawPrimitives(
            GraphicsDevice device, int primitiveType, int startVertex, int primitiveCount) {
        checkDevice(device, "cna_graphics_device_draw_primitives",
                nativeDrawPrimitives(
                        deviceGameValue(device), primitiveType, startVertex, primitiveCount));
    }

    public static void drawIndexedPrimitives(
            GraphicsDevice device,
            int primitiveType,
            int baseVertex,
            int minVertexIndex,
            int numVertices,
            int startIndex,
            int primitiveCount) {
        checkDevice(device, "cna_graphics_device_draw_indexed_primitives",
                nativeDrawIndexedPrimitives(
                        deviceGameValue(device), primitiveType, baseVertex,
                        minVertexIndex, numVertices, startIndex, primitiveCount));
    }

    public static void drawInstancedPrimitives(
            GraphicsDevice device,
            int primitiveType,
            int baseVertex,
            int minVertexIndex,
            int numVertices,
            int startIndex,
            int primitiveCount,
            int instanceCount) {
        checkDevice(device, "cna_graphics_device_draw_instanced_primitives",
                nativeDrawInstancedPrimitives(
                        deviceGameValue(device), primitiveType, baseVertex,
                        minVertexIndex, numVertices, startIndex,
                        primitiveCount, instanceCount));
    }

    public static void drawUserPrimitives(
            GraphicsDevice device,
            int primitiveType,
            int vertexSource,
            byte[] vertexData,
            int vertexStride,
            int vertexOffset,
            int numVertices,
            int primitiveCount,
            int[] declaration) {
        checkDevice(device, "cna_graphics_device_draw_user_primitives",
                nativeDrawUserPrimitives(
                        deviceGameValue(device), primitiveType, vertexSource,
                        Objects.requireNonNull(vertexData, "vertexData"), vertexStride,
                        vertexOffset, numVertices, primitiveCount,
                        Objects.requireNonNull(declaration, "declaration")));
    }

    public static void drawUserIndexedPrimitives(
            GraphicsDevice device,
            int primitiveType,
            int vertexSource,
            byte[] vertexData,
            int vertexStride,
            int vertexOffset,
            int numVertices,
            short[] indexData,
            int indexOffset,
            int primitiveCount,
            int[] declaration) {
        checkDevice(device, "cna_graphics_device_draw_user_indexed_primitives",
                nativeDrawUserIndexedPrimitives16(
                        deviceGameValue(device), primitiveType, vertexSource,
                        Objects.requireNonNull(vertexData, "vertexData"), vertexStride,
                        vertexOffset, numVertices,
                        Objects.requireNonNull(indexData, "indexData"), indexOffset,
                        primitiveCount, Objects.requireNonNull(declaration, "declaration")));
    }

    public static void drawUserIndexedPrimitives(
            GraphicsDevice device,
            int primitiveType,
            int vertexSource,
            byte[] vertexData,
            int vertexStride,
            int vertexOffset,
            int numVertices,
            int[] indexData,
            int indexOffset,
            int primitiveCount,
            int[] declaration) {
        checkDevice(device, "cna_graphics_device_draw_user_indexed_primitives",
                nativeDrawUserIndexedPrimitives32(
                        deviceGameValue(device), primitiveType, vertexSource,
                        Objects.requireNonNull(vertexData, "vertexData"), vertexStride,
                        vertexOffset, numVertices,
                        Objects.requireNonNull(indexData, "indexData"), indexOffset,
                        primitiveCount, Objects.requireNonNull(declaration, "declaration")));
    }

    /** Returns the Java resource currently crossing a synchronous native create/destroy event. */
    public static GraphicsResource currentGraphicsResourceEvent() {
        return GRAPHICS_RESOURCE_EVENT.get();
    }

    /** Records a Java listener failure without allowing it to unwind through native C/C++. */
    public static void recordGraphicsDeviceListenerFailure(
            GraphicsDevice device, Throwable failure) {
        synchronized (GAMES) {
            Throwable previous = DEVICE_CALLBACK_FAILURES.get(device);
            if (previous == null) {
                DEVICE_CALLBACK_FAILURES.put(device, failure);
            } else {
                previous.addSuppressed(failure);
            }
        }
    }

    /** Rethrows and clears a listener failure at the next Java/native call boundary. */
    public static void rethrowGraphicsDeviceListenerFailure(GraphicsDevice device) {
        Throwable failure;
        synchronized (GAMES) {
            failure = DEVICE_CALLBACK_FAILURES.remove(device);
        }
        if (failure instanceof RuntimeException runtime) {
            throw runtime;
        }
        if (failure instanceof Error error) {
            throw error;
        }
        if (failure != null) {
            throw new IllegalStateException("GraphicsDevice listener failed", failure);
        }
    }

    public static void createEffect(
            Effect effect,
            GraphicsDevice graphicsDevice,
            byte[] effectCode,
            boolean empty) {
        Objects.requireNonNull(effect, "effect");
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        check(empty ? "cna_effect_create_empty" : "cna_effect_create_compiled",
                nativeCreateEffect(
                        gameHandle(game, "Effect").requireValue(),
                        Objects.requireNonNull(effectCode, "effectCode"), empty, output));
        registerResource(game, effect, output[0], NativeBindings::destroyEffect);
    }

    /**
     * Registers an effect handle the extended layer already made as a Java facade.
     *
     * <p>Nothing is created here: the handle exists, and this is the Java name for it. Disposing
     * the facade calls {@code cna_effect_destroy}, which releases the handle the extended layer
     * created.
     *
     * @param effect the facade to register
     * @param graphicsDevice the device the effect belongs to
     * @param nativeEffect the handle the extended layer returned
     */
    public static void adoptEffect(
            Effect effect, GraphicsDevice graphicsDevice, long nativeEffect) {
        Objects.requireNonNull(effect, "effect");
        if (nativeEffect == 0L) {
            throw new IllegalArgumentException("nativeEffect");
        }
        registerResource(deviceGame(graphicsDevice), effect, nativeEffect,
                NativeBindings::destroyEffect);
    }

    public static void createBasicEffect(Effect effect, GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(effect, "effect");
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        check("cna_basic_effect_create",
                nativeCreateBasicEffect(gameHandle(game, "BasicEffect").requireValue(), output));
        registerResource(game, effect, output[0], NativeBindings::destroyEffect);
    }

    public static void createStockEffect(
            Effect effect, GraphicsDevice graphicsDevice, int effectKind) {
        Objects.requireNonNull(effect, "effect");
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        check(stockEffectCreateOperation(effectKind), nativeCreateStockEffect(
                gameHandle(game, stockEffectName(effectKind)).requireValue(), effectKind, output));
        registerResource(game, effect, output[0], NativeBindings::destroyEffect);
    }

    /**
     * Creates one of CNA's extended-layer effects and registers it against its game.
     *
     * <p>The effect is an ordinary graphics resource from here on: the game owns it, disposing
     * the game disposes it, and {@code SpriteBatch.Begin} takes it like any other effect.
     */
    public static void createExtensionEffect(
            Effect effect, GraphicsDevice graphicsDevice, int extensionEffect) {
        Objects.requireNonNull(effect, "effect");
        Game game = deviceGame(graphicsDevice);
        long[] resolved = new long[1];
        check("cna_game_get_graphics_device", NativeGamerServices.nativeGraphicsDeviceHandle(
                gameHandle(game, "CNA extension effect").requireValue(), resolved));
        long device = resolved[0];
        long[] output = new long[1];
        int result = extensionEffect == EXTENSION_EFFECT_CRT
                ? org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes
                        .crtEffectCreate(device, output)
                : org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes
                        .depthEffectCreate(device, output);
        check(extensionEffect == EXTENSION_EFFECT_CRT
                ? "cna_crt_effect_create" : "cna_depth_effect_create", result);
        registerResource(game, effect, output[0], NativeBindings::destroyEffect);
    }

    /** The CRT post-process effect of CNA's extended graphics layer. */
    public static final int EXTENSION_EFFECT_CRT = 0;

    /** The colour-depth reduction effect of CNA's extended graphics layer. */
    public static final int EXTENSION_EFFECT_DEPTH = 1;

    public static void cloneEffect(Effect effect, Effect source) {
        Objects.requireNonNull(effect, "effect");
        long[] output = new long[1];
        check("cna_effect_clone", nativeCloneEffect(resourceValue(source), output));
        registerResource(resourceOwner(source), effect, output[0], NativeBindings::destroyEffect);
    }

    public static void createEffectMaterial(Effect effect, Effect source) {
        Objects.requireNonNull(effect, "effect");
        Objects.requireNonNull(source, "source");
        long[] output = new long[1];
        check("cna_effect_material_create",
                nativeCreateEffectMaterial(resourceValue(source), output));
        registerResource(resourceOwner(source), effect, output[0], NativeBindings::destroyEffect);
    }

    public static void applyEffect(Effect effect) {
        check("cna_effect_apply", nativeApplyEffect(resourceValue(effect)));
    }

    public static long getEffectCollection(Effect effect, int kind) {
        long[] output = new long[2];
        check(kind == 0 ? "cna_effect_get_parameters" : "cna_effect_get_techniques",
                nativeGetEffectChild(resourceValue(effect), kind, output));
        return requireNativeEffectHandle(output[0]);
    }

    public static long[] getEffectCurrentTechnique(Effect effect) {
        long[] output = new long[2];
        check("cna_effect_get_current_technique",
                nativeGetEffectChild(resourceValue(effect), 2, output));
        return output;
    }

    public static void setEffectCurrentTechnique(Effect effect, EffectTechnique technique) {
        check("cna_effect_set_current_technique", nativeSetEffectCurrentTechnique(
                resourceValue(effect), effectMemberValue(technique)));
    }

    public static void registerEffectMember(
            Effect owner, Object member, long value, int destroyKind) {
        Objects.requireNonNull(owner, "owner");
        Objects.requireNonNull(member, "member");
        NativeResourceHandle handle = new NativeResourceHandle(
                requireNativeEffectHandle(value),
                closing -> destroyEffectMember(closing, destroyKind));
        synchronized (GAMES) {
            if (!RESOURCES.containsKey(owner)) {
                throw new IllegalStateException("Effect has no live CNA resource");
            }
            if (EFFECT_MEMBERS.put(member, handle) != null) {
                throw new IllegalStateException("Effect member was registered twice");
            }
            EFFECT_MEMBER_OWNERS.computeIfAbsent(owner, ignored -> new ArrayList<>()).add(member);
        }
    }

    public static void requireEffectMember(Object member) {
        effectMemberValue(member);
    }

    public static void releaseDuplicateEffectMember(long value, int destroyKind) {
        destroyEffectMember(requireNativeEffectHandle(value), destroyKind);
    }

    public static void closeEffectMembers(Effect owner) {
        List<Object> snapshot;
        synchronized (GAMES) {
            List<Object> members = EFFECT_MEMBER_OWNERS.get(owner);
            snapshot = members == null ? List.of() : new ArrayList<>(members);
        }
        Collections.reverse(snapshot);
        RuntimeException failure = null;
        for (Object member : snapshot) {
            NativeResourceHandle handle;
            synchronized (GAMES) {
                handle = EFFECT_MEMBERS.get(member);
            }
            if (handle == null) {
                continue;
            }
            try {
                handle.close();
                synchronized (GAMES) {
                    EFFECT_MEMBERS.remove(member);
                    List<Object> members = EFFECT_MEMBER_OWNERS.get(owner);
                    if (members != null) {
                        members.remove(member);
                    }
                }
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        synchronized (GAMES) {
            List<Object> members = EFFECT_MEMBER_OWNERS.get(owner);
            if (members != null && members.isEmpty()) {
                EFFECT_MEMBER_OWNERS.remove(owner);
            }
        }
        if (failure != null) {
            throw failure;
        }
    }

    public static long getEffectMemberCollection(Object member, int childKind) {
        long[] output = new long[2];
        check(effectChildOperation(childKind), nativeGetEffectChild(
                effectMemberValue(member), childKind, output));
        return requireNativeEffectHandle(output[0]);
    }

    public static int getEffectCollectionCount(Object collection, int collectionKind) {
        return Math.toIntExact(longResult(
                effectCollectionOperation(collectionKind, "get_count"),
                nativeGetEffectCollectionCount(effectMemberValue(collection), collectionKind)));
    }

    public static long getEffectCollectionElement(
            Object collection, int collectionKind, int index) {
        long[] output = new long[1];
        check(effectCollectionOperation(collectionKind, "get_at"),
                nativeGetEffectCollectionElement(
                        effectMemberValue(collection), collectionKind, index, output));
        return requireNativeEffectHandle(output[0]);
    }

    public static String getEffectString(Object member, int stringKind) {
        long handle = effectMemberValue(member);
        int size = Math.toIntExact(longResult(
                effectStringOperation(stringKind, true),
                nativeGetEffectStringSize(handle, stringKind)));
        if (size == 0) {
            return "";
        }
        byte[] output = new byte[size];
        check(effectStringOperation(stringKind, false),
                nativeCopyEffectString(handle, stringKind, output));
        return new String(output, StandardCharsets.UTF_8);
    }

    public static int[] getEffectInfo(Object member, int infoKind) {
        int[] output = new int[4];
        check(infoKind == 0
                        ? "cna_effect_parameter_get_info"
                        : "cna_effect_annotation_get_info",
                nativeGetEffectInfo(effectMemberValue(member), infoKind, output));
        return output;
    }

    public static int[] getEffectInts(Object member, int valueType, int count) {
        if (count < 0) {
            throw new IllegalArgumentException("Effect value count must not be negative");
        }
        int[] output = new int[count];
        int written = Math.toIntExact(longResult(
                member instanceof EffectAnnotation
                        ? annotationValueOperation(valueType)
                        : "cna_effect_parameter_get_values",
                nativeGetEffectInts(
                        effectMemberValue(member), member instanceof EffectAnnotation,
                        valueType, count, output)));
        return written == output.length ? output : Arrays.copyOf(output, written);
    }

    public static int getEffectIntValue(Object parameter, int valueType) {
        int[] output = new int[1];
        check("cna_effect_parameter_get_value", nativeGetEffectIntValue(
                effectMemberValue(parameter), valueType, output));
        return output[0];
    }

    public static float[] getEffectFloatValue(
            Object parameter, int valueType, int width) {
        if (width <= 0) {
            throw new IllegalArgumentException("Effect value width must be positive");
        }
        float[] output = new float[width];
        check("cna_effect_parameter_get_value", nativeGetEffectFloatValue(
                effectMemberValue(parameter), valueType, output));
        return output;
    }

    public static float[] getEffectFloats(
            Object member, int valueType, int count, int width) {
        if (count < 0 || width <= 0) {
            throw new IllegalArgumentException("Effect value shape is invalid");
        }
        float[] output = new float[Math.multiplyExact(count, width)];
        int written = Math.toIntExact(longResult(
                member instanceof EffectAnnotation
                        ? annotationValueOperation(valueType)
                        : "cna_effect_parameter_get_values",
                nativeGetEffectFloats(
                        effectMemberValue(member), member instanceof EffectAnnotation,
                        valueType, count, output)));
        int floatCount = Math.multiplyExact(written, width);
        return floatCount == output.length ? output : Arrays.copyOf(output, floatCount);
    }

    public static void setEffectInts(Object parameter, int valueType, int[] values) {
        check("cna_effect_parameter_set_values", nativeSetEffectInts(
                effectMemberValue(parameter), valueType,
                Objects.requireNonNull(values, "values")));
    }

    public static void setEffectIntValue(Object parameter, int valueType, int value) {
        check("cna_effect_parameter_set_value", nativeSetEffectIntValue(
                effectMemberValue(parameter), valueType, value));
    }

    public static void setEffectFloatValue(
            Object parameter, int valueType, float[] value) {
        check("cna_effect_parameter_set_value", nativeSetEffectFloatValue(
                effectMemberValue(parameter), valueType,
                Objects.requireNonNull(value, "value")));
    }

    public static void setEffectFloats(
            Object parameter, int valueType, float[] values, int count) {
        check("cna_effect_parameter_set_values", nativeSetEffectFloats(
                effectMemberValue(parameter), valueType,
                Objects.requireNonNull(values, "values"), count));
    }

    public static void setEffectString(Object parameter, String value) {
        check("cna_effect_parameter_set_value_string", nativeSetEffectString(
                effectMemberValue(parameter),
                Objects.requireNonNull(value, "value").getBytes(StandardCharsets.UTF_8)));
    }

    public static Texture2D getEffectTexture2D(Effect owner, EffectParameter parameter) {
        long handle = getEffectTextureHandle(parameter, 1);
        if (handle == 0L) {
            return null;
        }
        Texture2D texture = FacadeFactory.createUninitializedTexture2D(owner.getGraphicsDevice());
        registerResource(resourceOwner(owner), texture, handle, NativeBindings::destroyTexture2D);
        try {
            FacadeFactory.initializeTexture2D(texture, textureInfoOrClose(texture));
            return texture;
        } catch (RuntimeException failure) {
            closeAfterFailedFacade(texture, failure);
            throw failure;
        }
    }

    public static Texture3D getEffectTexture3D(Effect owner, EffectParameter parameter) {
        long handle = getEffectTextureHandle(parameter, 2);
        if (handle == 0L) {
            return null;
        }
        Texture3D texture = FacadeFactory.createUninitializedTexture3D(owner.getGraphicsDevice());
        registerResource(resourceOwner(owner), texture, handle, NativeBindings::destroyTexture3D);
        try {
            FacadeFactory.initializeTexture3D(texture, texture3DInfoOrClose(texture));
            return texture;
        } catch (RuntimeException failure) {
            closeAfterFailedFacade(texture, failure);
            throw failure;
        }
    }

    public static TextureCube getEffectTextureCube(Effect owner, EffectParameter parameter) {
        long handle = getEffectTextureHandle(parameter, 3);
        if (handle == 0L) {
            return null;
        }
        TextureCube texture = FacadeFactory.createUninitializedTextureCube(owner.getGraphicsDevice());
        registerResource(resourceOwner(owner), texture, handle, NativeBindings::destroyTextureCube);
        try {
            FacadeFactory.initializeTextureCube(texture, textureCubeInfoOrClose(texture));
            return texture;
        } catch (RuntimeException failure) {
            closeAfterFailedFacade(texture, failure);
            throw failure;
        }
    }

    public static void setEffectTexture(Effect owner, EffectParameter parameter, Texture value) {
        long texture = 0L;
        if (value != null) {
            if (value.getGraphicsDevice() != owner.getGraphicsDevice()) {
                throw new IllegalArgumentException("Texture belongs to a different GraphicsDevice");
            }
            texture = resourceValue(value);
        }
        check("cna_effect_parameter_set_value_texture", nativeSetEffectTexture(
                effectMemberValue(parameter), 0, texture));
    }

    public static void applyEffectPass(EffectPass pass) {
        check("cna_effect_pass_apply", nativeApplyEffectPass(effectMemberValue(pass)));
    }

    public static boolean getBasicEffectBoolean(Effect effect, int kind) {
        return booleanResult(basicEffectBooleanOperation(kind, false),
                nativeGetBasicEffectBoolean(resourceValue(effect), kind));
    }

    public static void setBasicEffectBoolean(Effect effect, int kind, boolean value) {
        check(basicEffectBooleanOperation(kind, true),
                nativeSetBasicEffectBoolean(resourceValue(effect), kind, value));
    }

    public static float getBasicEffectFloat(Effect effect, int kind) {
        float[] output = new float[1];
        check(basicEffectFloatOperation(kind, false),
                nativeGetBasicEffectFloat(resourceValue(effect), kind, output));
        return output[0];
    }

    public static void setBasicEffectFloat(Effect effect, int kind, float value) {
        check(basicEffectFloatOperation(kind, true),
                nativeSetBasicEffectFloat(resourceValue(effect), kind, value));
    }

    public static Vector3 getBasicEffectVector(Effect effect, int kind) {
        float[] output = new float[3];
        check(basicEffectVectorOperation(kind, false),
                nativeGetBasicEffectVector(resourceValue(effect), kind, output));
        return new Vector3(output[0], output[1], output[2]);
    }

    public static void setBasicEffectVector(Effect effect, int kind, Vector3 value) {
        Vector3 snapshot = new Vector3(Objects.requireNonNull(value, "value"));
        check(basicEffectVectorOperation(kind, true),
                nativeSetBasicEffectVector(
                        resourceValue(effect), kind,
                        new float[]{snapshot.X, snapshot.Y, snapshot.Z}));
    }

    public static Matrix getBasicEffectMatrix(Effect effect, int kind) {
        float[] output = new float[16];
        check(basicEffectMatrixOperation(kind, false),
                nativeGetBasicEffectMatrix(resourceValue(effect), kind, output));
        return matrix(output);
    }

    public static void setBasicEffectMatrix(Effect effect, int kind, Matrix value) {
        check(basicEffectMatrixOperation(kind, true),
                nativeSetBasicEffectMatrix(
                        resourceValue(effect), kind,
                        matrixValues(new Matrix(Objects.requireNonNull(value, "value")))));
    }

    public static long getBasicEffectDirectionalLightHandle(Effect effect, int index) {
        if (index < 0 || index > 2) {
            throw new IndexOutOfBoundsException("Directional-light index must be between 0 and 2");
        }
        long[] output = new long[1];
        check("cna_effect_lights_get_directional_light",
                nativeGetBasicEffectDirectionalLight(resourceValue(effect), index, output));
        return requireNativeEffectHandle(output[0]);
    }

    public static void enableDefaultLighting(Effect effect) {
        check("cna_effect_lights_enable_default",
                nativeEnableDefaultLighting(resourceValue(effect)));
    }

    public static Vector3 getDirectionalLightVector(DirectionalLight light, int kind) {
        float[] output = new float[3];
        check(directionalLightVectorOperation(kind, false),
                nativeGetDirectionalLightVector(effectMemberValue(light), kind, output));
        return new Vector3(output[0], output[1], output[2]);
    }

    public static void setDirectionalLightVector(
            DirectionalLight light, int kind, Vector3 value) {
        Vector3 snapshot = new Vector3(Objects.requireNonNull(value, "value"));
        check(directionalLightVectorOperation(kind, true),
                nativeSetDirectionalLightVector(
                        effectMemberValue(light), kind,
                        new float[]{snapshot.X, snapshot.Y, snapshot.Z}));
    }

    public static boolean getDirectionalLightEnabled(DirectionalLight light) {
        return booleanResult("cna_directional_light_get_enabled",
                nativeGetDirectionalLightEnabled(effectMemberValue(light)));
    }

    public static void setDirectionalLightEnabled(DirectionalLight light, boolean value) {
        check("cna_directional_light_set_enabled",
                nativeSetDirectionalLightEnabled(effectMemberValue(light), value));
    }

    public static void setBasicEffectTexture(Effect effect, Texture2D texture) {
        long textureHandle = 0L;
        if (texture != null) {
            if (texture.getGraphicsDevice() != effect.getGraphicsDevice()) {
                throw new IllegalArgumentException("Texture belongs to a different GraphicsDevice");
            }
            textureHandle = resourceValue(texture);
        }
        check("cna_basic_effect_set_texture",
                nativeSetBasicEffectTexture(resourceValue(effect), textureHandle));
    }

    public static boolean getStockEffectBoolean(Effect effect, int effectKind, int kind) {
        return booleanResult(stockEffectBooleanOperation(effectKind, kind, false),
                nativeGetStockEffectBoolean(resourceValue(effect), effectKind, kind));
    }

    public static void setStockEffectBoolean(
            Effect effect, int effectKind, int kind, boolean value) {
        check(stockEffectBooleanOperation(effectKind, kind, true),
                nativeSetStockEffectBoolean(resourceValue(effect), effectKind, kind, value));
    }

    public static float getStockEffectFloat(Effect effect, int effectKind, int kind) {
        float[] output = new float[1];
        check(stockEffectFloatOperation(effectKind, kind, false),
                nativeGetStockEffectFloat(resourceValue(effect), effectKind, kind, output));
        return output[0];
    }

    public static void setStockEffectFloat(
            Effect effect, int effectKind, int kind, float value) {
        check(stockEffectFloatOperation(effectKind, kind, true),
                nativeSetStockEffectFloat(resourceValue(effect), effectKind, kind, value));
    }

    public static int getStockEffectInt(Effect effect, int effectKind, int kind) {
        int[] output = new int[1];
        check(stockEffectIntOperation(effectKind, kind, false),
                nativeGetStockEffectInt(resourceValue(effect), effectKind, kind, output));
        return output[0];
    }

    public static void setStockEffectInt(
            Effect effect, int effectKind, int kind, int value) {
        check(stockEffectIntOperation(effectKind, kind, true),
                nativeSetStockEffectInt(resourceValue(effect), effectKind, kind, value));
    }

    public static Vector3 getStockEffectVector(Effect effect, int effectKind, int kind) {
        float[] output = new float[3];
        check(stockEffectVectorOperation(effectKind, kind, false),
                nativeGetStockEffectVector(resourceValue(effect), effectKind, kind, output));
        return new Vector3(output[0], output[1], output[2]);
    }

    public static void setStockEffectVector(
            Effect effect, int effectKind, int kind, Vector3 value) {
        Vector3 snapshot = new Vector3(Objects.requireNonNull(value, "value"));
        check(stockEffectVectorOperation(effectKind, kind, true),
                nativeSetStockEffectVector(resourceValue(effect), effectKind, kind,
                        new float[]{snapshot.X, snapshot.Y, snapshot.Z}));
    }

    public static void setStockEffectTexture(
            Effect effect, int effectKind, int slot, Texture texture) {
        long textureHandle = 0L;
        if (texture != null) {
            if (texture.getGraphicsDevice() != effect.getGraphicsDevice()) {
                throw new IllegalArgumentException("Texture belongs to a different GraphicsDevice");
            }
            textureHandle = resourceValue(texture);
        }
        check(stockEffectTextureOperation(effectKind, slot), nativeSetStockEffectTexture(
                resourceValue(effect), effectKind, slot, textureHandle));
    }

    public static void createOcclusionQuery(
            GraphicsResource query, GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(query, "query");
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        check("cna_occlusion_query_create", nativeCreateOcclusionQuery(
                gameHandle(game, "OcclusionQuery").requireValue(), output));
        registerResource(game, query, output[0], NativeBindings::destroyOcclusionQuery);
    }

    public static void beginOcclusionQuery(GraphicsResource query) {
        check("cna_occlusion_query_begin", nativeBeginOcclusionQuery(resourceValue(query)));
    }

    public static void endOcclusionQuery(GraphicsResource query) {
        check("cna_occlusion_query_end", nativeEndOcclusionQuery(resourceValue(query)));
    }

    public static boolean getOcclusionQueryComplete(GraphicsResource query) {
        return booleanResult("cna_occlusion_query_get_is_complete",
                nativeGetOcclusionQueryComplete(resourceValue(query)));
    }

    public static int getOcclusionQueryPixelCount(GraphicsResource query) {
        int[] output = new int[1];
        check("cna_occlusion_query_get_pixel_count",
                nativeGetOcclusionQueryPixelCount(resourceValue(query), output));
        return output[0];
    }

    public static void setSkinnedEffectBoneTransforms(Effect effect, Matrix[] transforms) {
        float[] values = new float[Math.multiplyExact(transforms.length, 16)];
        for (int index = 0; index < transforms.length; index++) {
            Matrix snapshot = new Matrix(Objects.requireNonNull(
                    transforms[index], "boneTransforms[" + index + "]"));
            System.arraycopy(matrixValues(snapshot), 0, values, index * 16, 16);
        }
        check("cna_skinned_effect_set_bone_transforms",
                nativeSetSkinnedEffectBoneTransforms(resourceValue(effect), values));
    }

    public static Matrix[] getSkinnedEffectBoneTransforms(Effect effect, int count) {
        float[] values = new float[Math.multiplyExact(count, 16)];
        check("cna_skinned_effect_copy_bone_transforms",
                nativeGetSkinnedEffectBoneTransforms(resourceValue(effect), count, values));
        Matrix[] output = new Matrix[count];
        for (int index = 0; index < count; index++) {
            float[] value = new float[16];
            System.arraycopy(values, index * 16, value, 0, 16);
            output[index] = matrix(value);
        }
        return output;
    }

    public static int[] createTexture2D(
            Texture2D texture,
            GraphicsDevice graphicsDevice,
            int width,
            int height,
            boolean mipMap,
            int format) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(texture);
        try {
            check("cna_texture2d_create", nativeCreateTexture2D(
                    gameHandle(game, "Texture2D").requireValue(),
                    width, height, mipMap, format, output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, texture, output[0], NativeBindings::destroyTexture2D);
        int[] info = textureInfoOrClose(texture);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        return info;
    }

    public static Texture2D loadContentTexture2D(
            ContentManager manager,
            GraphicsDevice graphicsDevice,
            String rootDirectory,
            String assetName) {
        Game game = deviceGame(graphicsDevice);
        long managerValue = contentManagerValue(
                manager, graphicsDevice, Objects.requireNonNull(rootDirectory, "rootDirectory"));
        Texture2D texture = FacadeFactory.createUninitializedTexture2D(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(texture);
        try {
            check("cna_content_manager_load_texture2d", nativeLoadContentTexture2D(
                    managerValue,
                    Objects.requireNonNull(assetName, "assetName")
                            .getBytes(StandardCharsets.UTF_8),
                    output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, texture, output[0], NativeBindings::destroyTexture2D);
        try {
            FacadeFactory.initializeTexture2D(texture, textureInfoOrClose(texture));
            rethrowGraphicsDeviceListenerFailure(graphicsDevice);
            return texture;
        } catch (RuntimeException failure) {
            try {
                texture.close();
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    public static SpriteFont loadContentSpriteFont(
            ContentManager manager,
            GraphicsDevice graphicsDevice,
            String rootDirectory,
            String assetName) {
        Game game = deviceGame(graphicsDevice);
        long managerValue = contentManagerValue(
                manager, graphicsDevice, Objects.requireNonNull(rootDirectory, "rootDirectory"));
        SpriteFont font = FacadeFactory.createSpriteFont();
        Texture2D atlas = FacadeFactory.createUninitializedTexture2D(graphicsDevice);
        long[] output = new long[2];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(atlas);
        try {
            check("cna_content_manager_load_sprite_font", nativeLoadContentSpriteFont(
                    managerValue,
                    Objects.requireNonNull(assetName, "assetName")
                            .getBytes(StandardCharsets.UTF_8),
                    output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, atlas, output[1], NativeBindings::destroyTexture2D);
        registerSpriteFont(game, font, output[0], atlas);
        try {
            FacadeFactory.initializeTexture2D(atlas, textureInfoOrClose(atlas));
            spriteFontInfo(font);
            rethrowGraphicsDeviceListenerFailure(graphicsDevice);
            return font;
        } catch (RuntimeException failure) {
            try {
                closeSpriteFont(font);
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    public static SpriteFont createSpriteFont(
            Texture2D texture,
            List<Rectangle> glyphBounds,
            List<Rectangle> cropping,
            List<Character> characters,
            int lineSpacing,
            float spacing,
            List<Vector3> kerning,
            Character defaultCharacter) {
        Objects.requireNonNull(texture, "texture");
        Objects.requireNonNull(glyphBounds, "glyphBounds");
        Objects.requireNonNull(cropping, "cropping");
        Objects.requireNonNull(characters, "characters");
        Objects.requireNonNull(kerning, "kerning");
        int count = characters.size();
        if (count == 0 || glyphBounds.size() != count || cropping.size() != count
                || kerning.size() != count || !Float.isFinite(spacing)) {
            throw new IllegalArgumentException("Invalid SpriteFont glyph table");
        }
        int[] rectangles = new int[Math.multiplyExact(count, 8)];
        char[] characterValues = new char[count];
        float[] kerningValues = new float[Math.multiplyExact(count, 3)];
        for (int index = 0; index < count; index++) {
            Rectangle glyph = new Rectangle(Objects.requireNonNull(
                    glyphBounds.get(index), "glyphBounds[" + index + "]"));
            Rectangle crop = new Rectangle(Objects.requireNonNull(
                    cropping.get(index), "cropping[" + index + "]"));
            Character character = Objects.requireNonNull(
                    characters.get(index), "characters[" + index + "]");
            Vector3 bearing = new Vector3(Objects.requireNonNull(
                    kerning.get(index), "kerning[" + index + "]"));
            int rectangleOffset = index * 8;
            rectangles[rectangleOffset] = glyph.X;
            rectangles[rectangleOffset + 1] = glyph.Y;
            rectangles[rectangleOffset + 2] = glyph.Width;
            rectangles[rectangleOffset + 3] = glyph.Height;
            rectangles[rectangleOffset + 4] = crop.X;
            rectangles[rectangleOffset + 5] = crop.Y;
            rectangles[rectangleOffset + 6] = crop.Width;
            rectangles[rectangleOffset + 7] = crop.Height;
            characterValues[index] = character;
            int kerningOffset = index * 3;
            kerningValues[kerningOffset] = bearing.X;
            kerningValues[kerningOffset + 1] = bearing.Y;
            kerningValues[kerningOffset + 2] = bearing.Z;
        }

        Game game = resourceOwner(texture);
        SpriteFont font = FacadeFactory.createSpriteFont();
        long[] output = new long[1];
        check("cna_sprite_font_create", nativeCreateSpriteFont(
                resourceValue(texture), rectangles, characterValues, kerningValues,
                lineSpacing, spacing, defaultCharacter != null,
                defaultCharacter == null ? 0 : defaultCharacter, output));
        registerSpriteFont(game, font, output[0], texture);
        try {
            spriteFontInfo(font);
            return font;
        } catch (RuntimeException failure) {
            try {
                closeSpriteFont(font);
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    public static List<Character> getSpriteFontCharacters(SpriteFont font) {
        int count = spriteFontInfo(font).integers()[0];
        char[] characters = new char[count];
        check("cna_sprite_font_copy_characters",
                nativeCopySpriteFontCharacters(spriteFontValue(font), characters));
        List<Character> result = new ArrayList<>(count);
        for (char character : characters) {
            result.add(character);
        }
        return List.copyOf(result);
    }

    public static Character getSpriteFontDefaultCharacter(SpriteFont font) {
        int[] info = spriteFontInfo(font).integers();
        return info[3] == 0 ? null : (char)info[2];
    }

    public static void setSpriteFontDefaultCharacter(SpriteFont font, Character value) {
        check("cna_sprite_font_set_default_character",
                nativeSetSpriteFontDefaultCharacter(
                        spriteFontValue(font), value != null,
                        value == null ? 0 : value));
    }

    public static int getSpriteFontLineSpacing(SpriteFont font) {
        return spriteFontInfo(font).integers()[1];
    }

    public static void setSpriteFontLineSpacing(SpriteFont font, int value) {
        check("cna_sprite_font_set_line_spacing",
                nativeSetSpriteFontLineSpacing(spriteFontValue(font), value));
    }

    public static float getSpriteFontSpacing(SpriteFont font) {
        return spriteFontInfo(font).spacing();
    }

    public static void setSpriteFontSpacing(SpriteFont font, float value) {
        check("cna_sprite_font_set_spacing",
                nativeSetSpriteFontSpacing(spriteFontValue(font), value));
    }

    public static Vector2 measureSpriteFont(SpriteFont font, String text) {
        float[] output = new float[2];
        int result = nativeMeasureSpriteFont(
                spriteFontValue(font),
                Objects.requireNonNull(text, "text").getBytes(StandardCharsets.UTF_8),
                output);
        if (result == 1) {
            throw new IllegalArgumentException(
                    "Text contains a character that is absent from the SpriteFont");
        }
        check("cna_sprite_font_measure_utf8", result);
        return new Vector2(output[0], output[1]);
    }

    public static int[] createVertexBuffer(
            VertexBuffer buffer,
            GraphicsDevice graphicsDevice,
            int vertexStride,
            int[] declaration,
            int vertexCount,
            int usage,
            boolean dynamic) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(buffer);
        try {
            check("cna_vertex_buffer_create", nativeCreateVertexBuffer(
                    gameHandle(game, "VertexBuffer").requireValue(),
                    vertexStride, Objects.requireNonNull(declaration, "declaration"),
                    vertexCount, usage, dynamic, output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, buffer, output[0], NativeBindings::destroyVertexBuffer);
        int[] info = vertexBufferInfoOrClose(buffer);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        return info;
    }

    public static void setVertexBufferData(
            VertexBuffer buffer,
            int offsetInBytes,
            int vertexType,
            byte[] payload,
            int vertexCount,
            int vertexStride,
            int options) {
        check(offsetInBytes < 0
                        ? (options == 0
                                ? "cna_vertex_buffer_set_data"
                                : "cna_vertex_buffer_set_data(options)")
                        : "cna_vertex_buffer_set_data_raw_at",
                nativeSetVertexBufferData(
                        resourceValue(buffer), offsetInBytes, vertexType,
                        Objects.requireNonNull(payload, "payload"),
                        vertexCount, vertexStride, options));
    }

    public static void setVertexBufferRawBytes(
            VertexBuffer buffer, byte[] payload, int vertexCount, int vertexStride) {
        byte[] snapshot = Objects.requireNonNull(payload, "payload").clone();
        if (snapshot.length != Math.multiplyExact(vertexCount, vertexStride)) {
            throw new IllegalArgumentException("Raw vertex payload length does not match the buffer");
        }
        check("cna_vertex_buffer_set_data_raw_at", nativeSetVertexBufferData(
                resourceValue(buffer), 0, 0, snapshot,
                vertexCount, vertexStride, 0));
    }

    public static boolean getVertexBufferIsContentLost(VertexBuffer buffer) {
        return vertexBufferInfo(buffer)[5] != 0;
    }

    public static long subscribeVertexBufferContentLost(
            VertexBuffer buffer, Object callbackTarget) {
        long[] output = new long[1];
        check("cna_vertex_buffer_subscribe_content_lost",
                nativeSubscribeVertexBufferContentLost(
                        resourceValue(buffer),
                        Objects.requireNonNull(callbackTarget, "callbackTarget"), output));
        if (output[0] == 0L) {
            throw new IllegalStateException("CNA returned an invalid vertex-buffer subscription");
        }
        return output[0];
    }

    public static void unsubscribeVertexBufferContentLost(long registration) {
        check("cna_vertex_buffer_unsubscribe_content_lost",
                nativeUnsubscribeVertexBufferContentLost(registration));
    }

    public static byte[] getVertexBufferData(
            VertexBuffer buffer,
            int offsetInBytes,
            int vertexCount,
            int vertexStride) {
        byte[] output = new byte[Math.multiplyExact(vertexCount, vertexStride)];
        check("cna_vertex_buffer_get_data_raw",
                nativeGetVertexBufferData(
                        resourceValue(buffer), offsetInBytes,
                        vertexCount, vertexStride, output));
        return output;
    }

    public static int[] createIndexBuffer(
            IndexBuffer buffer,
            GraphicsDevice graphicsDevice,
            int indexElementSize,
            int indexCount,
            int usage,
            boolean dynamic) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(buffer);
        try {
            check("cna_index_buffer_create", nativeCreateIndexBuffer(
                    gameHandle(game, "IndexBuffer").requireValue(),
                    indexElementSize, indexCount, usage, dynamic, output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, buffer, output[0], NativeBindings::destroyIndexBuffer);
        int[] info = indexBufferInfoOrClose(buffer);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        return info;
    }

    public static void setIndexBufferData(
            IndexBuffer buffer,
            int offsetInBytes,
            int indexElementSize,
            int[] values,
            int options) {
        check(offsetInBytes < 0
                        ? "cna_index_buffer_set_data"
                        : "cna_index_buffer_set_data_at",
                nativeSetIndexBufferData(
                        resourceValue(buffer), offsetInBytes, indexElementSize,
                        Objects.requireNonNull(values, "values"), options));
    }

    public static boolean getIndexBufferIsContentLost(IndexBuffer buffer) {
        return indexBufferInfo(buffer)[4] != 0;
    }

    public static long subscribeIndexBufferContentLost(
            IndexBuffer buffer, Object callbackTarget) {
        long[] output = new long[1];
        check("cna_index_buffer_subscribe_content_lost",
                nativeSubscribeIndexBufferContentLost(
                        resourceValue(buffer),
                        Objects.requireNonNull(callbackTarget, "callbackTarget"), output));
        if (output[0] == 0L) {
            throw new IllegalStateException("CNA returned an invalid index-buffer subscription");
        }
        return output[0];
    }

    public static void unsubscribeIndexBufferContentLost(long registration) {
        check("cna_index_buffer_unsubscribe_content_lost",
                nativeUnsubscribeIndexBufferContentLost(registration));
    }

    public static int[] getIndexBufferData(
            IndexBuffer buffer, int indexElementSize, int elementCount) {
        int[] output = new int[elementCount];
        check("cna_index_buffer_get_data",
                nativeGetIndexBufferData(
                        resourceValue(buffer), indexElementSize, output));
        return output;
    }

    public static int[] createTexture2DFromEncoded(
            Texture2D texture,
            GraphicsDevice graphicsDevice,
            byte[] encoded,
            int width,
            int height,
            boolean zoom,
            boolean resize) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(texture);
        try {
            check("cna_texture2d_create_from_encoded_memory", nativeCreateTexture2DFromEncoded(
                    gameHandle(game, "Texture2D.FromStream").requireValue(),
                    Objects.requireNonNull(encoded, "encoded"),
                    width, height, zoom, resize, output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, texture, output[0], NativeBindings::destroyTexture2D);
        int[] info = textureInfoOrClose(texture);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        return info;
    }

    public static void setTexture2DData(Texture2D texture, Color[] data) {
        Objects.requireNonNull(data, "data");
        int[] packed = new int[data.length];
        for (int index = 0; index < data.length; index++) {
            packed[index] = Objects.requireNonNull(data[index], "data[" + index + "]")
                    .getPackedValue().intValue();
        }
        check("cna_texture2d_set_data_rgba8",
                nativeSetTexture2DData(resourceValue(texture), packed));
    }

    public static Color[] getTexture2DData(Texture2D texture, int pixelCount) {
        int[] packed = new int[pixelCount];
        check("cna_texture2d_get_data_rgba8",
                nativeGetTexture2DData(resourceValue(texture), packed));
        Color[] result = new Color[pixelCount];
        for (int index = 0; index < packed.length; index++) {
            Color color = new Color(0, 0, 0, 0);
            color.setPackedValue(Integer.toUnsignedLong(packed[index]));
            result[index] = color;
        }
        return result;
    }

    public static void setTexture2DData(
            Texture2D texture,
            int dataType,
            int level,
            Rectangle rectangle,
            int startIndex,
            int elementCount,
            byte[] payload) {
        Objects.requireNonNull(payload, "payload");
        boolean hasRectangle = rectangle != null;
        int x = hasRectangle ? rectangle.X : 0;
        int y = hasRectangle ? rectangle.Y : 0;
        int width = hasRectangle ? rectangle.Width : 0;
        int height = hasRectangle ? rectangle.Height : 0;
        check("cna_texture2d_set_data", nativeSetTexture2DTypedData(
                resourceValue(texture), dataType, level,
                hasRectangle, x, y, width, height,
                startIndex, elementCount, payload));
    }

    public static void setTexture2DRawBytes(Texture2D texture, int level, byte[] payload) {
        byte[] snapshot = Objects.requireNonNull(payload, "payload").clone();
        if (snapshot.length == 0 || snapshot.length % 4 != 0) {
            throw new IllegalArgumentException("Color texture payload must contain whole RGBA texels");
        }
        check("cna_texture2d_set_data", nativeSetTexture2DTypedData(
                resourceValue(texture), 0, level,
                false, 0, 0, 0, 0,
                0, snapshot.length / 4, snapshot));
    }

    public static byte[] getTexture2DData(
            Texture2D texture,
            int dataType,
            int level,
            Rectangle rectangle,
            int startIndex,
            int elementCount,
            int payloadBytes) {
        if (payloadBytes < 0) {
            throw new IllegalArgumentException("Texture payload capacity must not be negative");
        }
        byte[] payload = new byte[payloadBytes];
        boolean hasRectangle = rectangle != null;
        int x = hasRectangle ? rectangle.X : 0;
        int y = hasRectangle ? rectangle.Y : 0;
        int width = hasRectangle ? rectangle.Width : 0;
        int height = hasRectangle ? rectangle.Height : 0;
        check("cna_texture2d_get_data", nativeGetTexture2DTypedData(
                resourceValue(texture), dataType, level,
                hasRectangle, x, y, width, height,
                startIndex, elementCount, payload));
        return payload;
    }

    public static byte[] encodeTexture2D(Texture2D texture, int format, int width, int height) {
        long handle = resourceValue(texture);
        int size = Math.toIntExact(longResult("cna_texture2d_get_encoded_byte_count",
                nativeGetTexture2DEncodedSize(handle, format, width, height)));
        byte[] output = new byte[size];
        check("cna_texture2d_copy_encoded",
                nativeCopyTexture2DEncoded(handle, format, width, height, output));
        return output;
    }

    public static int[] createTexture3D(
            Texture3D texture,
            GraphicsDevice graphicsDevice,
            int width,
            int height,
            int depth,
            boolean mipMap,
            int format) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        check("cna_texture3d_create", nativeCreateTexture3D(
                gameHandle(game, "Texture3D").requireValue(),
                width, height, depth, mipMap, format, output));
        registerResource(game, texture, output[0], NativeBindings::destroyTexture3D);
        int[] info = texture3DInfoOrClose(texture);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        return info;
    }

    public static void setTexture3DData(
            Texture3D texture,
            int level,
            int left,
            int top,
            int right,
            int bottom,
            int front,
            int back,
            Color[] data,
            int startIndex,
            int elementCount) {
        int[] packed = new int[data.length];
        for (int index = startIndex; index < startIndex + elementCount; index++) {
            packed[index] = Objects.requireNonNull(data[index], "data[" + index + "]")
                    .getPackedValue().intValue();
        }
        check("cna_texture3d_set_data", nativeSetTexture3DData(
                resourceValue(texture), level, left, top, right, bottom, front, back,
                startIndex, elementCount, packed));
    }

    public static Color[] getTexture3DData(
            Texture3D texture,
            int level,
            int left,
            int top,
            int right,
            int bottom,
            int front,
            int back,
            int capacity,
            int startIndex,
            int elementCount) {
        int[] packed = new int[capacity];
        check("cna_texture3d_get_data", nativeGetTexture3DData(
                resourceValue(texture), level, left, top, right, bottom, front, back,
                startIndex, elementCount, packed));
        Color[] output = new Color[capacity];
        for (int index = startIndex; index < startIndex + elementCount; index++) {
            Color color = new Color();
            color.setPackedValue(Integer.toUnsignedLong(packed[index]));
            output[index] = color;
        }
        return output;
    }

    public static int[] createTextureCube(
            TextureCube texture,
            GraphicsDevice graphicsDevice,
            int size,
            boolean mipMap,
            int format) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(texture);
        try {
            check("cna_texturecube_create", nativeCreateTextureCube(
                    gameHandle(game, "TextureCube").requireValue(),
                    size, mipMap, format, output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, texture, output[0], NativeBindings::destroyTextureCube);
        int[] info = textureCubeInfoOrClose(texture);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        return info;
    }

    public static void setTextureCubeData(
            TextureCube texture,
            int face,
            int level,
            Rectangle rectangle,
            Color[] data,
            int startIndex,
            int elementCount) {
        Objects.requireNonNull(data, "data");
        int[] packed = new int[data.length];
        for (int index = startIndex; index < startIndex + elementCount; index++) {
            packed[index] = Objects.requireNonNull(data[index], "data[" + index + "]")
                    .getPackedValue().intValue();
        }
        boolean hasRectangle = rectangle != null;
        check("cna_texturecube_set_data", nativeSetTextureCubeData(
                resourceValue(texture), face, level, hasRectangle,
                hasRectangle ? rectangle.X : 0,
                hasRectangle ? rectangle.Y : 0,
                hasRectangle ? rectangle.Width : 0,
                hasRectangle ? rectangle.Height : 0,
                startIndex, elementCount, packed));
    }

    public static Color[] getTextureCubeData(
            TextureCube texture,
            int face,
            int level,
            Rectangle rectangle,
            int capacity,
            int startIndex,
            int elementCount) {
        int[] packed = new int[capacity];
        boolean hasRectangle = rectangle != null;
        check("cna_texturecube_get_data", nativeGetTextureCubeData(
                resourceValue(texture), face, level, hasRectangle,
                hasRectangle ? rectangle.X : 0,
                hasRectangle ? rectangle.Y : 0,
                hasRectangle ? rectangle.Width : 0,
                hasRectangle ? rectangle.Height : 0,
                startIndex, elementCount, packed));
        Color[] result = new Color[capacity];
        for (int index = startIndex; index < startIndex + elementCount; index++) {
            Color color = new Color();
            color.setPackedValue(Integer.toUnsignedLong(packed[index]));
            result[index] = color;
        }
        return result;
    }

    public static int[] createRenderTarget2D(
            RenderTarget2D renderTarget,
            GraphicsDevice graphicsDevice,
            int width,
            int height,
            boolean mipMap,
            int format,
            int depthFormat,
            int multiSampleCount,
            int usage) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(renderTarget);
        try {
            check("cna_render_target2d_create", nativeCreateRenderTarget2D(
                    gameHandle(game, "RenderTarget2D").requireValue(),
                    width, height, mipMap, format, depthFormat,
                    multiSampleCount, usage, output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, renderTarget, output[0], NativeBindings::destroyRenderTarget);
        int[] info = renderTargetInfoOrClose(renderTarget);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        return info;
    }

    public static int[] createRenderTargetCube(
            RenderTargetCube renderTarget,
            GraphicsDevice graphicsDevice,
            int size,
            boolean mipMap,
            int format,
            int depthFormat,
            int multiSampleCount,
            int usage) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(renderTarget);
        try {
            check("cna_render_target_cube_create", nativeCreateRenderTargetCube(
                    gameHandle(game, "RenderTargetCube").requireValue(),
                    size, mipMap, format, depthFormat,
                    multiSampleCount, usage, output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, renderTarget, output[0], NativeBindings::destroyRenderTarget);
        int[] info = renderTargetInfoOrClose(renderTarget);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        return info;
    }

    public static boolean getRenderTargetIsContentLost(GraphicsResource renderTarget) {
        return renderTargetInfo(renderTarget)[7] != 0;
    }

    public static void setGraphicsDeviceRenderTarget2D(
            GraphicsDevice device, RenderTarget2D renderTarget) {
        checkDevice(device, "cna_graphics_device_set_render_target2d",
                nativeSetGraphicsDeviceRenderTarget2D(
                        deviceGameValue(device),
                        renderTarget == null ? 0L : resourceValue(renderTarget)));
    }

    public static void setGraphicsDeviceRenderTargetCube(
            GraphicsDevice device,
            RenderTargetCube renderTarget,
            CubeMapFace face) {
        checkDevice(device, "cna_graphics_device_set_render_target_cube",
                nativeSetGraphicsDeviceRenderTargetCube(
                        deviceGameValue(device),
                        renderTarget == null ? 0L : resourceValue(renderTarget),
                        Objects.requireNonNull(face, "face").ordinal()));
    }

    public static void setGraphicsDeviceRenderTargets(
            GraphicsDevice device, RenderTargetBinding[] bindings) {
        Objects.requireNonNull(bindings, "bindings");
        long[] handles = new long[bindings.length];
        int[] faces = new int[bindings.length];
        for (int index = 0; index < bindings.length; index++) {
            RenderTargetBinding binding = Objects.requireNonNull(
                    bindings[index], "bindings[" + index + "]");
            Texture target = binding.getRenderTarget();
            if (!(target instanceof RenderTarget2D)
                    && !(target instanceof RenderTargetCube)) {
                throw new IllegalArgumentException(
                        "A render-target binding must contain RenderTarget2D or RenderTargetCube");
            }
            if (target.getGraphicsDevice() != device) {
                throw new IllegalArgumentException(
                        "Every render target must belong to this GraphicsDevice");
            }
            handles[index] = resourceValue(target);
            faces[index] = binding.getCubeMapFace().ordinal();
        }
        checkDevice(device, "cna_graphics_device_set_render_targets",
                nativeSetGraphicsDeviceRenderTargets(
                        deviceGameValue(device), handles, faces));
    }

    public static RenderTargetBinding[] getGraphicsDeviceRenderTargets(GraphicsDevice device) {
        int count = Math.toIntExact(longResult(
                "cna_graphics_device_get_render_target_count",
                nativeGetGraphicsDeviceRenderTargetCount(deviceGameValue(device))));
        long[] handles = new long[count];
        int[] faces = new int[count];
        checkDevice(device, "cna_graphics_device_copy_render_targets",
                nativeCopyGraphicsDeviceRenderTargets(
                        deviceGameValue(device), handles, faces));
        RenderTargetBinding[] result = new RenderTargetBinding[count];
        for (int index = 0; index < count; index++) {
            Texture target = findTexture(handles[index], null);
            if (target instanceof RenderTarget2D renderTarget2D) {
                result[index] = new RenderTargetBinding(renderTarget2D);
            } else if (target instanceof RenderTargetCube renderTargetCube) {
                if (faces[index] < 0 || faces[index] >= CubeMapFace.values().length) {
                    throw new IllegalStateException(
                            "CNA returned an unknown CubeMapFace " + faces[index]);
                }
                result[index] = new RenderTargetBinding(
                        renderTargetCube,
                        CubeMapFace.values()[faces[index]]);
            } else {
                throw new IllegalStateException(
                        "CNA returned a render target not owned by this Java binding");
            }
        }
        return result;
    }

    public static void createSpriteBatch(SpriteBatch spriteBatch, GraphicsDevice graphicsDevice) {
        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(spriteBatch);
        try {
            check("cna_sprite_batch_create", nativeCreateSpriteBatch(
                    gameHandle(game, "SpriteBatch").requireValue(), output));
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        registerResource(game, spriteBatch, output[0], NativeBindings::destroySpriteBatch);
        rethrowGraphicsDeviceListenerFailure(graphicsDevice);
    }

    public static void beginSpriteBatch(SpriteBatch spriteBatch, SpriteSortMode sortMode) {
        check("cna_sprite_batch_begin", nativeBeginSpriteBatch(
                resourceValue(spriteBatch), Objects.requireNonNull(sortMode, "sortMode").ordinal()));
    }

    public static void beginSpriteBatchWithStates(
            SpriteBatch spriteBatch,
            SpriteSortMode sortMode,
            int[] blend,
            int[] sampler,
            float samplerBias,
            int[] depthStencil,
            int[] rasterizer,
            float[] rasterizerFloats) {
        check("cna_sprite_batch_begin_with_states", nativeBeginSpriteBatchWithStates(
                resourceValue(spriteBatch),
                Objects.requireNonNull(sortMode, "sortMode").ordinal(),
                requireLength(blend, 12, "BlendState"),
                requireLength(sampler, 6, "SamplerState integer"),
                samplerBias,
                requireLength(depthStencil, 16, "DepthStencilState"),
                requireLength(rasterizer, 4, "RasterizerState integer"),
                requireLength(rasterizerFloats, 2, "RasterizerState float")));
    }

    public static void beginSpriteBatchWithEffect(
            SpriteBatch spriteBatch,
            SpriteSortMode sortMode,
            int[] blend,
            int[] sampler,
            float samplerBias,
            int[] depthStencil,
            int[] rasterizer,
            float[] rasterizerFloats,
            Effect effect,
            Matrix transform) {
        if (effect != null && effect.getGraphicsDevice() != spriteBatch.getGraphicsDevice()) {
            throw new IllegalArgumentException("Effect belongs to a different GraphicsDevice");
        }
        float[] matrix = null;
        if (transform != null) {
            Matrix value = new Matrix(transform);
            matrix = new float[] {
                    value.M11, value.M12, value.M13, value.M14,
                    value.M21, value.M22, value.M23, value.M24,
                    value.M31, value.M32, value.M33, value.M34,
                    value.M41, value.M42, value.M43, value.M44};
        }
        check("cna_sprite_batch_begin_with_effect", nativeBeginSpriteBatchWithEffect(
                resourceValue(spriteBatch),
                Objects.requireNonNull(sortMode, "sortMode").ordinal(),
                requireLength(blend, 12, "BlendState"),
                requireLength(sampler, 6, "SamplerState integer"),
                samplerBias,
                requireLength(depthStencil, 16, "DepthStencilState"),
                requireLength(rasterizer, 4, "RasterizerState integer"),
                requireLength(rasterizerFloats, 2, "RasterizerState float"),
                effect == null ? 0L : resourceValue(effect), matrix));
    }

    public static void drawSpriteRectangle(
            SpriteBatch spriteBatch,
            Texture2D texture,
            Rectangle destination,
            Rectangle source,
            Color color,
            float rotation,
            Vector2 origin,
            SpriteEffects effects,
            float layerDepth) {
        Rectangle destinationValue = new Rectangle(Objects.requireNonNull(destination, "destination"));
        Rectangle sourceValue = source == null ? new Rectangle() : new Rectangle(source);
        Color colorValue = new Color(Objects.requireNonNull(color, "color"));
        Vector2 originValue = new Vector2(Objects.requireNonNull(origin, "origin"));
        check("cna_sprite_batch_submit_many", nativeDrawSpriteRectangle(
                resourceValue(spriteBatch), resourceValue(texture),
                destinationValue.X, destinationValue.Y, destinationValue.Width, destinationValue.Height,
                sourceValue.X, sourceValue.Y, sourceValue.Width, sourceValue.Height,
                colorValue.getPackedValue().intValue(), rotation, originValue.X, originValue.Y,
                Objects.requireNonNull(effects, "effects").getValue(), layerDepth));
    }

    public static void drawSpriteScaled(
            SpriteBatch spriteBatch,
            Texture2D texture,
            Vector2 position,
            Rectangle source,
            Color color,
            float rotation,
            Vector2 origin,
            Vector2 scale,
            SpriteEffects effects,
            float layerDepth) {
        Vector2 positionValue = new Vector2(Objects.requireNonNull(position, "position"));
        Rectangle sourceValue = source == null ? new Rectangle() : new Rectangle(source);
        Color colorValue = new Color(Objects.requireNonNull(color, "color"));
        Vector2 originValue = new Vector2(Objects.requireNonNull(origin, "origin"));
        Vector2 scaleValue = new Vector2(Objects.requireNonNull(scale, "scale"));
        check("cna_sprite_batch_submit_scaled_many", nativeDrawSpriteScaled(
                resourceValue(spriteBatch), resourceValue(texture), positionValue.X, positionValue.Y,
                sourceValue.X, sourceValue.Y, sourceValue.Width, sourceValue.Height,
                colorValue.getPackedValue().intValue(), rotation, originValue.X, originValue.Y,
                scaleValue.X, scaleValue.Y, Objects.requireNonNull(effects, "effects").getValue(),
                layerDepth));
    }

    public static void drawSpriteString(
            SpriteBatch spriteBatch,
            SpriteFont spriteFont,
            String text,
            Vector2 position,
            Color color,
            float rotation,
            Vector2 origin,
            Vector2 scale,
            SpriteEffects effects,
            float layerDepth) {
        Vector2 positionValue = new Vector2(Objects.requireNonNull(position, "position"));
        Color colorValue = new Color(Objects.requireNonNull(color, "color"));
        Vector2 originValue = new Vector2(Objects.requireNonNull(origin, "origin"));
        Vector2 scaleValue = new Vector2(Objects.requireNonNull(scale, "scale"));
        check("cna_sprite_batch_draw_string", nativeDrawSpriteString(
                resourceValue(spriteBatch),
                spriteFontValue(Objects.requireNonNull(spriteFont, "spriteFont")),
                Objects.requireNonNull(text, "text").getBytes(StandardCharsets.UTF_8),
                positionValue.X,
                positionValue.Y,
                colorValue.getPackedValue().intValue(),
                rotation,
                originValue.X,
                originValue.Y,
                scaleValue.X,
                scaleValue.Y,
                Objects.requireNonNull(effects, "effects").getValue(),
                layerDepth));
    }

    public static void endSpriteBatch(SpriteBatch spriteBatch) {
        check("cna_sprite_batch_end", nativeEndSpriteBatch(resourceValue(spriteBatch)));
    }

    public static void closeGraphicsResource(GraphicsResource resource) {
        NativeResourceHandle handle;
        synchronized (GAMES) {
            handle = RESOURCES.get(resource);
        }
        if (handle == null) {
            return;
        }
        unbindBufferBeforeRelease(resource);
        GraphicsResource previous = GRAPHICS_RESOURCE_EVENT.get();
        GRAPHICS_RESOURCE_EVENT.set(resource);
        try {
            handle.close();
        } finally {
            restoreGraphicsResourceEvent(previous);
        }
        synchronized (GAMES) {
            RESOURCES.remove(resource);
            Game owner = RESOURCE_OWNERS.remove(resource);
            List<GraphicsResource> children = GAME_RESOURCES.get(owner);
            if (children != null) {
                children.remove(resource);
            }
        }
    }

    private static void unbindBufferBeforeRelease(GraphicsResource resource) {
        GraphicsDevice device = resource.getGraphicsDevice();
        if (resource instanceof VertexBuffer) {
            boolean bound;
            synchronized (GAMES) {
                List<VertexBuffer> bindings = DEVICE_VERTEX_BINDINGS.get(device);
                bound = bindings != null && bindings.stream()
                        .anyMatch(buffer -> buffer == resource);
            }
            if (bound) {
                int result = nativeSetGraphicsDeviceVertexBuffers(
                        deviceGameValue(device), new long[0], new int[0], new int[0]);
                try {
                    checkDevice(device, "cna_graphics_device_set_vertex_buffers", result);
                } finally {
                    if (result == 0) {
                        recordVertexBindings(device, List.of());
                    }
                }
            }
        } else if (resource instanceof IndexBuffer) {
            boolean bound;
            synchronized (GAMES) {
                bound = DEVICE_INDEX_BINDINGS.get(device) == resource;
            }
            if (bound) {
                int result = nativeSetGraphicsDeviceIndexBuffer(deviceGameValue(device), 0L);
                try {
                    checkDevice(device, "cna_graphics_device_set_index_buffer", result);
                } finally {
                    if (result == 0) {
                        synchronized (GAMES) {
                            if (DEVICE_INDEX_BINDINGS.get(device) == resource) {
                                DEVICE_INDEX_BINDINGS.remove(device);
                            }
                        }
                    }
                }
            }
        }
    }

    private static void recordVertexBindings(
            GraphicsDevice device, List<VertexBuffer> bindings) {
        synchronized (GAMES) {
            if (bindings.isEmpty()) {
                DEVICE_VERTEX_BINDINGS.remove(device);
            } else {
                DEVICE_VERTEX_BINDINGS.put(device, List.copyOf(bindings));
            }
        }
    }

    public static void closeSpriteFont(SpriteFont font) {
        Objects.requireNonNull(font, "font");
        NativeResourceHandle handle;
        Texture2D atlas;
        synchronized (GAMES) {
            handle = SPRITE_FONTS.get(font);
            atlas = SPRITE_FONT_ATLASES.get(font);
        }
        if (handle != null) {
            handle.close();
            synchronized (GAMES) {
                SPRITE_FONTS.remove(font);
                Game owner = SPRITE_FONT_OWNERS.remove(font);
                List<SpriteFont> children = GAME_SPRITE_FONTS.get(owner);
                if (children != null) {
                    children.remove(font);
                }
            }
        }
        if (atlas != null) {
            atlas.close();
            synchronized (GAMES) {
                SPRITE_FONT_ATLASES.remove(font);
            }
        }
    }

    public static void unloadContentManager(ContentManager manager) {
        NativeResourceHandle handle;
        synchronized (GAMES) {
            handle = CONTENT_MANAGERS.get(Objects.requireNonNull(manager, "manager"));
        }
        if (handle != null) {
            check("cna_content_manager_unload",
                    nativeUnloadContentManager(handle.requireValue()));
        }
    }

    public static void closeContentManager(ContentManager manager) {
        Objects.requireNonNull(manager, "manager");
        NativeResourceHandle handle;
        synchronized (GAMES) {
            handle = CONTENT_MANAGERS.get(manager);
        }
        if (handle == null) {
            return;
        }
        handle.close();
        synchronized (GAMES) {
            CONTENT_MANAGERS.remove(manager);
            Game owner = CONTENT_MANAGER_OWNERS.remove(manager);
            List<ContentManager> children = GAME_CONTENT_MANAGERS.get(owner);
            if (children != null) {
                children.remove(manager);
            }
        }
    }

    public static void closeGraphicsResources(Game game) {
        RuntimeException failure = closeSpriteFonts(game, null);
        failure = closeContentManagers(game, failure);
        List<GraphicsResource> snapshot;
        synchronized (GAMES) {
            List<GraphicsResource> resources = GAME_RESOURCES.get(game);
            snapshot = resources == null ? List.of() : new ArrayList<>(resources);
        }
        Collections.reverse(snapshot);
        for (GraphicsResource resource : snapshot) {
            try {
                resource.close();
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        if (failure != null) {
            throw failure;
        }
    }

    static void destroyGame(Game game, long handle) {
        RuntimeException audioFailure = null;
        RuntimeException mediaFailure = null;
        RuntimeException storageFailure = null;
        try {
            NativeStorage.closeAllForGameShutdown();
        } catch (RuntimeException exception) {
            storageFailure = exception;
        }
        try {
            NativeMedia.closeAllForGameShutdown();
        } catch (RuntimeException exception) {
            mediaFailure = exception;
        }
        try {
            NativeAudio.closeAllForGameShutdown();
        } catch (RuntimeException exception) {
            audioFailure = exception;
        }
        int result = nativeDestroyGame(handle);
        // CNA documents CALLBACK as released: shutdown completed but a callback reported failure.
        if (result != 0 && result != 9) {
            CnaNativeException nativeFailure = failure("cna_game_destroy", result);
            if (audioFailure != null) nativeFailure.addSuppressed(audioFailure);
            if (mediaFailure != null) nativeFailure.addSuppressed(mediaFailure);
            if (storageFailure != null) nativeFailure.addSuppressed(storageFailure);
            throw nativeFailure;
        }
        synchronized (GAMES) {
            GAMES.remove(game);
            GAME_RESOURCES.remove(game);
            GAME_SPRITE_FONTS.remove(game);
            GAME_CONTENT_MANAGERS.remove(game);
            DEVICES.entrySet().removeIf(entry -> {
                if (entry.getValue() != game) {
                    return false;
                }
                DEVICE_CALLBACK_FAILURES.remove(entry.getKey());
                DEVICE_VERTEX_BINDINGS.remove(entry.getKey());
                DEVICE_INDEX_BINDINGS.remove(entry.getKey());
                return true;
            });
            if (currentGame == game) {
                currentGame = null;
            }
        }
        if (mediaFailure != null) {
            if (audioFailure != null) mediaFailure.addSuppressed(audioFailure);
            if (storageFailure != null) mediaFailure.addSuppressed(storageFailure);
            throw mediaFailure;
        }
        if (audioFailure != null) {
            if (storageFailure != null) audioFailure.addSuppressed(storageFailure);
            throw audioFailure;
        }
        if (storageFailure != null) throw storageFailure;
    }

    static void destroyGraphicsDeviceManager(
            GraphicsDeviceManager manager,
            long handle) {
        Objects.requireNonNull(manager, "manager");
        check("cna_graphics_device_manager_destroy",
                nativeDestroyGraphicsDeviceManager(handle));
    }

    private static void check(String operation, int result) {
        if (result != 0) {
            throw failure(operation, result);
        }
    }

    private static NativeGameHandle gameHandle(Game game, String owner) {
        NativeGameHandle handle;
        synchronized (GAMES) {
            handle = GAMES.get(Objects.requireNonNull(game, "game"));
        }
        if (handle == null || handle.isClosed()) {
            throw new IllegalStateException(owner + " is unavailable before native Game creation");
        }
        return handle;
    }

    private static NativeGameHandle currentGameHandle(String owner) {
        NativeGameHandle handle;
        synchronized (GAMES) {
            handle = currentGame == null ? null : GAMES.get(currentGame);
        }
        if (handle == null || handle.isClosed()) {
            throw new IllegalStateException(
                    owner + " requires a live CNA Game on the current process");
        }
        return handle;
    }

    static long currentGameValue(String owner) {
        return currentGameHandle(owner).requireValue();
    }

    private static Game deviceGame(GraphicsDevice device) {
        Game game;
        synchronized (GAMES) {
            game = DEVICES.get(Objects.requireNonNull(device, "graphicsDevice"));
        }
        if (game == null) {
            throw new IllegalArgumentException("GraphicsDevice was not created by CNA-Java");
        }
        return game;
    }

    private static long deviceGameValue(GraphicsDevice device) {
        return gameHandle(deviceGame(device), "GraphicsDevice").requireValue();
    }

    private static void checkDevice(GraphicsDevice device, String operation, int result) {
        try {
            check(operation, result);
        } catch (RuntimeException nativeFailure) {
            try {
                rethrowGraphicsDeviceListenerFailure(device);
            } catch (Throwable callbackFailure) {
                nativeFailure.addSuppressed(callbackFailure);
            }
            throw nativeFailure;
        }
        rethrowGraphicsDeviceListenerFailure(device);
    }

    private static long deviceLongResult(
            GraphicsDevice device, String operation, long result) {
        try {
            long value = longResult(operation, result);
            rethrowGraphicsDeviceListenerFailure(device);
            return value;
        } catch (RuntimeException nativeFailure) {
            try {
                rethrowGraphicsDeviceListenerFailure(device);
            } catch (Throwable callbackFailure) {
                nativeFailure.addSuppressed(callbackFailure);
            }
            throw nativeFailure;
        }
    }

    private static int deviceIntResult(
            GraphicsDevice device, String operation, long result) {
        return Math.toIntExact(deviceLongResult(device, operation, result));
    }

    private static void restoreGraphicsResourceEvent(GraphicsResource previous) {
        if (previous == null) {
            GRAPHICS_RESOURCE_EVENT.remove();
        } else {
            GRAPHICS_RESOURCE_EVENT.set(previous);
        }
    }

    private static void registerResource(
            Game game,
            GraphicsResource resource,
            long value,
            java.util.function.LongConsumer releaser) {
        NativeResourceHandle handle = new NativeResourceHandle(value, releaser);
        synchronized (GAMES) {
            RESOURCES.put(resource, handle);
            RESOURCE_OWNERS.put(resource, game);
            GAME_RESOURCES.computeIfAbsent(game, ignored -> new ArrayList<>()).add(resource);
        }
    }

    private static void registerSpriteFont(
            Game game, SpriteFont font, long value, Texture2D atlas) {
        NativeResourceHandle handle = new NativeResourceHandle(
                value, NativeBindings::destroySpriteFont);
        synchronized (GAMES) {
            SPRITE_FONTS.put(font, handle);
            SPRITE_FONT_OWNERS.put(font, game);
            SPRITE_FONT_ATLASES.put(font, atlas);
            GAME_SPRITE_FONTS.computeIfAbsent(game, ignored -> new ArrayList<>()).add(font);
        }
    }

    /**
     * Returns the CNA content manager behind one Java {@link ContentManager}.
     *
     * <p>Reached from {@code org.openeggbert.cna.extensions.content}, which loads CNA's own view
     * of an asset through the same manager the XNA loads use, so both see one root directory and
     * one asset cache.
     */
    public static long nativeContentManagerHandle(ContentManager manager) {
        Objects.requireNonNull(manager, "manager");
        return contentManagerValue(manager,
                FacadeFactory.contentManagerGraphicsDevice(manager), manager.getRootDirectory());
    }

    private static long contentManagerValue(
            ContentManager manager,
            GraphicsDevice graphicsDevice,
            String rootDirectory) {
        Objects.requireNonNull(manager, "manager");
        NativeResourceHandle existing;
        synchronized (GAMES) {
            existing = CONTENT_MANAGERS.get(manager);
        }
        if (existing != null) {
            check("cna_content_manager_set_root_directory", nativeSetContentManagerRootDirectory(
                    existing.requireValue(), rootDirectory.getBytes(StandardCharsets.UTF_8)));
            return existing.requireValue();
        }

        Game game = deviceGame(graphicsDevice);
        long[] output = new long[1];
        check("cna_content_manager_create", nativeCreateContentManager(
                gameHandle(game, "ContentManager").requireValue(),
                rootDirectory.getBytes(StandardCharsets.UTF_8),
                output));
        int registrationResult = nativeRegisterContentManagerBuiltinLoaders(output[0]);
        if (registrationResult != 0) {
            CnaNativeException registrationFailure = failure(
                    "cna_content_manager_register_builtin_loaders", registrationResult);
            int destroyResult = nativeDestroyContentManager(output[0]);
            if (destroyResult != 0) {
                registrationFailure.addSuppressed(
                        failure("cna_content_manager_destroy", destroyResult));
            }
            throw registrationFailure;
        }
        NativeResourceHandle created = new NativeResourceHandle(
                output[0], NativeBindings::destroyContentManager);
        synchronized (GAMES) {
            CONTENT_MANAGERS.put(manager, created);
            CONTENT_MANAGER_OWNERS.put(manager, game);
            GAME_CONTENT_MANAGERS.computeIfAbsent(game, ignored -> new ArrayList<>()).add(manager);
        }
        return created.requireValue();
    }

    /**
     * Returns the CNA handle behind one {@link SpriteFont}.
     *
     * <p>Reached from {@code org.openeggbert.cna.extensions.gamerservices}, which hands CNA the
     * surfaces it draws its own on-screen keyboard and message box with.
     */
    public static long nativeSpriteFontHandle(SpriteFont font) {
        return spriteFontValue(font);
    }

    private static long spriteFontValue(SpriteFont font) {
        NativeResourceHandle handle;
        synchronized (GAMES) {
            handle = SPRITE_FONTS.get(Objects.requireNonNull(font, "font"));
        }
        if (handle == null) {
            throw new IllegalStateException("SpriteFont is not backed by an open CNA resource");
        }
        return handle.requireValue();
    }

    private static SpriteFontInfo spriteFontInfo(SpriteFont font) {
        int[] integers = new int[4];
        float[] spacing = new float[1];
        check("cna_sprite_font_get_info",
                nativeGetSpriteFontInfo(spriteFontValue(font), integers, spacing));
        if (integers[0] < 0 || integers[3] < 0 || integers[3] > 1
                || integers[2] < Character.MIN_VALUE || integers[2] > Character.MAX_VALUE
                || !Float.isFinite(spacing[0])) {
            throw new IllegalStateException("CNA returned invalid SpriteFont metadata");
        }
        return new SpriteFontInfo(integers, spacing[0]);
    }

    private static RuntimeException closeSpriteFonts(Game game, RuntimeException failure) {
        List<SpriteFont> snapshot;
        synchronized (GAMES) {
            List<SpriteFont> resources = GAME_SPRITE_FONTS.get(game);
            snapshot = resources == null ? List.of() : new ArrayList<>(resources);
        }
        Collections.reverse(snapshot);
        for (SpriteFont font : snapshot) {
            try {
                closeSpriteFont(font);
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        return failure;
    }

    private static RuntimeException closeContentManagers(
            Game game, RuntimeException failure) {
        List<ContentManager> snapshot;
        synchronized (GAMES) {
            List<ContentManager> resources = GAME_CONTENT_MANAGERS.get(game);
            snapshot = resources == null ? List.of() : new ArrayList<>(resources);
        }
        Collections.reverse(snapshot);
        for (ContentManager manager : snapshot) {
            try {
                closeContentManager(manager);
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        return failure;
    }

    private record SpriteFontInfo(int[] integers, float spacing) {
    }

    private static int[] textureInfoOrClose(Texture2D texture) {
        int[] info = new int[4];
        try {
            check("cna_texture2d_get_info", nativeGetTexture2DInfo(resourceValue(texture), info));
            return info;
        } catch (RuntimeException failure) {
            try {
                closeGraphicsResource(texture);
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    private static int[] vertexBufferInfoOrClose(VertexBuffer buffer) {
        try {
            return vertexBufferInfo(buffer);
        } catch (RuntimeException failure) {
            try {
                closeGraphicsResource(buffer);
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    private static int[] vertexBufferInfo(VertexBuffer buffer) {
        int[] info = new int[7];
        try {
            check("cna_vertex_buffer_get_info",
                    nativeGetVertexBufferInfo(resourceValue(buffer), info));
            return info;
        } catch (RuntimeException failure) {
            throw failure;
        }
    }

    private static int[] indexBufferInfoOrClose(IndexBuffer buffer) {
        try {
            return indexBufferInfo(buffer);
        } catch (RuntimeException failure) {
            try {
                closeGraphicsResource(buffer);
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    private static int[] indexBufferInfo(IndexBuffer buffer) {
        int[] info = new int[6];
        try {
            check("cna_index_buffer_get_info",
                    nativeGetIndexBufferInfo(resourceValue(buffer), info));
            return info;
        } catch (RuntimeException failure) {
            throw failure;
        }
    }

    private static int[] textureCubeInfoOrClose(TextureCube texture) {
        int[] info = new int[3];
        try {
            check("cna_texturecube_get_info",
                    nativeGetTextureCubeInfo(resourceValue(texture), info));
            return info;
        } catch (RuntimeException failure) {
            try {
                closeGraphicsResource(texture);
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    private static int[] texture3DInfoOrClose(Texture3D texture) {
        int[] info = new int[5];
        try {
            check("cna_texture3d_get_info", nativeGetTexture3DInfo(resourceValue(texture), info));
            return info;
        } catch (RuntimeException failure) {
            closeAfterFailedFacade(texture, failure);
            throw failure;
        }
    }

    private static int[] renderTargetInfoOrClose(GraphicsResource renderTarget) {
        try {
            return renderTargetInfo(renderTarget);
        } catch (RuntimeException failure) {
            try {
                closeGraphicsResource(renderTarget);
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    private static int[] renderTargetInfo(GraphicsResource renderTarget) {
        int[] info = new int[10];
        check("cna_render_target_get_info",
                nativeGetRenderTargetInfo(resourceValue(renderTarget), info));
        return info;
    }

    /**
     * Returns the live native handle behind one graphics resource.
     *
     * <p>For the CNA extension surface, which owns its own native objects but has to hand CNA
     * an XNA-projected texture or device. It stays inside
     * {@code org.openeggbert.cna.internal}; no public or protected signature exposes it.
     */
    public static long nativeResourceHandle(GraphicsResource resource) {
        return resourceValue(resource);
    }

    /**
     * Returns the CNA graphics-device handle behind one Java {@link GraphicsDevice}.
     *
     * <p>CNA scopes the device to a frame callback, so this resolves it from the owning game
     * every time rather than caching a handle that would be stale outside the frame.
     */
    public static long nativeGraphicsDeviceValue(GraphicsDevice device) {
        Game game = deviceGame(device);
        long[] resolved = new long[1];
        check("cna_game_get_graphics_device", NativeGamerServices.nativeGraphicsDeviceHandle(
                gameHandle(game, "CNA extension").requireValue(), resolved));
        return resolved[0];
    }

    /** Returns the live native handle of the game that owns a graphics device. */
    public static long nativeDeviceGameHandle(GraphicsDevice device) {
        Game game = deviceGame(device);
        return gameHandle(game, "CNA extension").requireValue();
    }


    private static long resourceValue(GraphicsResource resource) {
        NativeResourceHandle handle;
        synchronized (GAMES) {
            handle = RESOURCES.get(Objects.requireNonNull(resource, "resource"));
        }
        if (handle == null) {
            throw new IllegalStateException(
                    resource.getClass().getSimpleName() + " has no live CNA resource");
        }
        return handle.requireValue();
    }

    private static Game resourceOwner(GraphicsResource resource) {
        synchronized (GAMES) {
            Game owner = RESOURCE_OWNERS.get(Objects.requireNonNull(resource, "resource"));
            if (owner == null) {
                throw new IllegalStateException(
                        resource.getClass().getSimpleName() + " has no live CNA owner");
            }
            return owner;
        }
    }

    private static long effectMemberValue(Object member) {
        NativeResourceHandle handle;
        synchronized (GAMES) {
            handle = EFFECT_MEMBERS.get(Objects.requireNonNull(member, "member"));
        }
        if (handle == null) {
            throw new IllegalStateException("Effect member is unavailable after parent disposal");
        }
        return handle.requireValue();
    }

    private static long requireNativeEffectHandle(long value) {
        if (value == 0L) {
            throw new IllegalStateException("CNA returned an invalid Effect reflection handle");
        }
        return value;
    }

    private static long getEffectTextureHandle(EffectParameter parameter, int textureType) {
        long[] output = new long[1];
        check("cna_effect_parameter_get_value_texture", nativeGetEffectTexture(
                effectMemberValue(parameter), textureType, output));
        return output[0];
    }

    private static void closeAfterFailedFacade(
            GraphicsResource resource, RuntimeException failure) {
        try {
            resource.close();
        } catch (RuntimeException closeFailure) {
            failure.addSuppressed(closeFailure);
        }
    }

    private static String effectChildOperation(int kind) {
        return switch (kind) {
            case 3 -> "cna_effect_technique_get_passes";
            case 4 -> "cna_effect_technique_get_annotations";
            case 5 -> "cna_effect_pass_get_annotations";
            case 6 -> "cna_effect_parameter_get_elements";
            case 7 -> "cna_effect_parameter_get_structure_members";
            case 8 -> "cna_effect_parameter_get_annotations";
            default -> throw new IllegalArgumentException("Unknown Effect child kind " + kind);
        };
    }

    private static String effectCollectionOperation(int kind, String suffix) {
        String stem = switch (kind) {
            case 0 -> "cna_effect_parameter_collection_";
            case 1 -> "cna_effect_technique_collection_";
            case 2 -> "cna_effect_pass_collection_";
            case 3 -> "cna_effect_annotation_collection_";
            default -> throw new IllegalArgumentException("Unknown Effect collection kind " + kind);
        };
        return stem + suffix;
    }

    private static String effectStringOperation(int kind, boolean size) {
        String operation = switch (kind) {
            case 0 -> "cna_effect_technique_";
            case 1 -> "cna_effect_pass_";
            case 2 -> "cna_effect_parameter_";
            case 3 -> "cna_effect_parameter_";
            case 4 -> "cna_effect_annotation_";
            case 5 -> "cna_effect_annotation_";
            case 6 -> "cna_effect_parameter_";
            case 7 -> "cna_effect_annotation_";
            default -> throw new IllegalArgumentException("Unknown Effect string kind " + kind);
        };
        String field = switch (kind) {
            case 0, 1, 2, 4 -> "name";
            case 3, 5 -> "semantic";
            case 6, 7 -> "value_string";
            default -> throw new AssertionError();
        };
        return operation + (size ? "get_" + field + "_byte_count" : "copy_" + field);
    }

    private static String annotationValueOperation(int valueType) {
        return switch (valueType) {
            case 0 -> "cna_effect_annotation_get_value_boolean";
            case 1 -> "cna_effect_annotation_get_value_int32";
            case 2 -> "cna_effect_annotation_get_value_single";
            case 3 -> "cna_effect_annotation_get_value_matrix";
            case 6 -> "cna_effect_annotation_get_value_vector2";
            case 7 -> "cna_effect_annotation_get_value_vector3";
            case 8 -> "cna_effect_annotation_get_value_vector4";
            default -> throw new IllegalArgumentException(
                    "Unsupported Effect annotation value kind " + valueType);
        };
    }

    private static String basicEffectBooleanOperation(int kind, boolean setter) {
        String stem = switch (kind) {
            case 0 -> "cna_basic_effect_" + (setter ? "set" : "get")
                    + "_vertex_color_enabled";
            case 1 -> "cna_basic_effect_" + (setter ? "set" : "get")
                    + "_prefer_per_pixel_lighting";
            case 2 -> "cna_basic_effect_" + (setter ? "set" : "get")
                    + "_texture_enabled";
            case 3 -> "cna_effect_lights_" + (setter ? "set" : "get") + "_enabled";
            case 4 -> "cna_effect_fog_" + (setter ? "set" : "get") + "_enabled";
            default -> throw new IllegalArgumentException("Unknown BasicEffect Boolean kind " + kind);
        };
        return stem;
    }

    private static String stockEffectName(int effectKind) {
        return switch (effectKind) {
            case 0 -> "AlphaTestEffect";
            case 1 -> "DualTextureEffect";
            case 2 -> "EnvironmentMapEffect";
            case 3 -> "SkinnedEffect";
            default -> throw new IllegalArgumentException("Unknown stock Effect kind " + effectKind);
        };
    }

    private static String stockEffectCreateOperation(int effectKind) {
        return switch (effectKind) {
            case 0 -> "cna_alpha_test_effect_create";
            case 1 -> "cna_dual_texture_effect_create";
            case 2 -> "cna_environment_map_effect_create";
            case 3 -> "cna_skinned_effect_create";
            default -> throw new IllegalArgumentException("Unknown stock Effect kind " + effectKind);
        };
    }

    private static String stockEffectBooleanOperation(
            int effectKind, int kind, boolean setter) {
        String action = setter ? "set" : "get";
        String operation = switch (effectKind) {
            case 0 -> kind == 0
                    ? "cna_alpha_test_effect_" + action + "_vertex_color_enabled" : null;
            case 1 -> kind == 0
                    ? "cna_dual_texture_effect_" + action + "_vertex_color_enabled" : null;
            case 3 -> kind == 0
                    ? "cna_skinned_effect_" + action + "_prefer_per_pixel_lighting" : null;
            default -> throw new IllegalArgumentException(
                    "Unknown stock Effect Boolean kind " + effectKind + ":" + kind);
        };
        if (operation == null) {
            throw new IllegalArgumentException(
                    "Unknown stock Effect Boolean kind " + effectKind + ":" + kind);
        }
        return operation;
    }

    private static String stockEffectFloatOperation(
            int effectKind, int kind, boolean setter) {
        String action = setter ? "set" : "get";
        String operation = switch (effectKind) {
            case 0 -> kind == 0 ? "cna_alpha_test_effect_" + action + "_alpha" : null;
            case 1 -> kind == 0 ? "cna_dual_texture_effect_" + action + "_alpha" : null;
            case 2 -> switch (kind) {
                case 0 -> "cna_environment_map_effect_" + action + "_alpha";
                case 1 -> "cna_environment_map_effect_" + action + "_amount";
                case 2 -> "cna_environment_map_effect_" + action + "_fresnel_factor";
                default -> null;
            };
            case 3 -> switch (kind) {
                case 0 -> "cna_skinned_effect_" + action + "_specular_power";
                case 1 -> "cna_skinned_effect_" + action + "_alpha";
                default -> null;
            };
            default -> throw new IllegalArgumentException(
                    "Unknown stock Effect float kind " + effectKind + ":" + kind);
        };
        if (operation == null) {
            throw new IllegalArgumentException(
                    "Unknown stock Effect float kind " + effectKind + ":" + kind);
        }
        return operation;
    }

    private static String stockEffectIntOperation(int effectKind, int kind, boolean setter) {
        String action = setter ? "set" : "get";
        String operation = switch (effectKind) {
            case 0 -> switch (kind) {
                case 0 -> "cna_alpha_test_effect_" + action + "_alpha_function";
                case 1 -> "cna_alpha_test_effect_" + action + "_reference_alpha";
                default -> null;
            };
            case 3 -> kind == 0
                    ? "cna_skinned_effect_" + action + "_weights_per_vertex" : null;
            default -> throw new IllegalArgumentException(
                    "Unknown stock Effect int kind " + effectKind + ":" + kind);
        };
        if (operation == null) {
            throw new IllegalArgumentException(
                    "Unknown stock Effect int kind " + effectKind + ":" + kind);
        }
        return operation;
    }

    private static String stockEffectVectorOperation(
            int effectKind, int kind, boolean setter) {
        String action = setter ? "set" : "get";
        String operation = switch (effectKind) {
            case 0 -> kind == 0
                    ? "cna_alpha_test_effect_" + action + "_diffuse_color" : null;
            case 1 -> kind == 0
                    ? "cna_dual_texture_effect_" + action + "_diffuse_color" : null;
            case 2 -> switch (kind) {
                case 0 -> "cna_environment_map_effect_" + action + "_diffuse_color";
                case 1 -> "cna_environment_map_effect_" + action + "_emissive_color";
                case 2 -> "cna_environment_map_effect_" + action + "_specular";
                default -> null;
            };
            case 3 -> switch (kind) {
                case 0 -> "cna_skinned_effect_" + action + "_diffuse_color";
                case 1 -> "cna_skinned_effect_" + action + "_emissive_color";
                case 2 -> "cna_skinned_effect_" + action + "_specular_color";
                default -> null;
            };
            default -> throw new IllegalArgumentException(
                    "Unknown stock Effect vector kind " + effectKind + ":" + kind);
        };
        if (operation == null) {
            throw new IllegalArgumentException(
                    "Unknown stock Effect vector kind " + effectKind + ":" + kind);
        }
        return operation;
    }

    private static String stockEffectTextureOperation(int effectKind, int slot) {
        String operation = switch (effectKind) {
            case 0 -> slot == 0 ? "cna_alpha_test_effect_set_texture" : null;
            case 1 -> slot == 0 || slot == 1 ? "cna_dual_texture_effect_set_texture" : null;
            case 2 -> switch (slot) {
                case 0 -> "cna_environment_map_effect_set_texture";
                case 1 -> "cna_environment_map_effect_set_environment_map";
                default -> null;
            };
            case 3 -> slot == 0 ? "cna_skinned_effect_set_texture" : null;
            default -> throw new IllegalArgumentException(
                    "Unknown stock Effect texture kind " + effectKind + ":" + slot);
        };
        if (operation == null) {
            throw new IllegalArgumentException(
                    "Unknown stock Effect texture kind " + effectKind + ":" + slot);
        }
        return operation;
    }

    private static String basicEffectFloatOperation(int kind, boolean setter) {
        String action = setter ? "set" : "get";
        return switch (kind) {
            case 0 -> "cna_basic_effect_" + action + "_specular_power";
            case 1 -> "cna_basic_effect_" + action + "_alpha";
            case 2 -> "cna_effect_fog_" + action + "_start";
            case 3 -> "cna_effect_fog_" + action + "_end";
            default -> throw new IllegalArgumentException("Unknown BasicEffect float kind " + kind);
        };
    }

    private static String basicEffectVectorOperation(int kind, boolean setter) {
        String action = setter ? "set" : "get";
        return switch (kind) {
            case 0 -> "cna_basic_effect_" + action + "_diffuse_color";
            case 1 -> "cna_basic_effect_" + action + "_emissive_color";
            case 2 -> "cna_basic_effect_" + action + "_specular_color";
            case 3 -> "cna_effect_lights_" + action + "_ambient_color";
            case 4 -> "cna_effect_fog_" + action + "_color";
            default -> throw new IllegalArgumentException("Unknown BasicEffect vector kind " + kind);
        };
    }

    private static String basicEffectMatrixOperation(int kind, boolean setter) {
        String action = setter ? "set" : "get";
        return switch (kind) {
            case 0 -> "cna_effect_matrices_" + action + "_world";
            case 1 -> "cna_effect_matrices_" + action + "_view";
            case 2 -> "cna_effect_matrices_" + action + "_projection";
            default -> throw new IllegalArgumentException("Unknown BasicEffect matrix kind " + kind);
        };
    }

    private static String directionalLightVectorOperation(int kind, boolean setter) {
        String action = setter ? "set" : "get";
        return switch (kind) {
            case 0 -> "cna_directional_light_" + action + "_diffuse_color";
            case 1 -> "cna_directional_light_" + action + "_direction";
            case 2 -> "cna_directional_light_" + action + "_specular_color";
            default -> throw new IllegalArgumentException("Unknown DirectionalLight vector kind " + kind);
        };
    }

    private static float[] matrixValues(Matrix value) {
        return new float[]{
                value.M11, value.M12, value.M13, value.M14,
                value.M21, value.M22, value.M23, value.M24,
                value.M31, value.M32, value.M33, value.M34,
                value.M41, value.M42, value.M43, value.M44};
    }

    private static Matrix matrix(float[] value) {
        return new Matrix(
                value[0], value[1], value[2], value[3],
                value[4], value[5], value[6], value[7],
                value[8], value[9], value[10], value[11],
                value[12], value[13], value[14], value[15]);
    }

    private static Texture findTexture(long value, Texture cached) {
        synchronized (GAMES) {
            if (cached != null) {
                NativeResourceHandle cachedHandle = RESOURCES.get(cached);
                if (cachedHandle != null && !cachedHandle.isClosed()
                        && cachedHandle.requireValue() == value) {
                    return cached;
                }
            }
            for (Map.Entry<GraphicsResource, NativeResourceHandle> entry : RESOURCES.entrySet()) {
                if (entry.getKey() instanceof Texture texture
                        && !entry.getValue().isClosed()
                        && entry.getValue().requireValue() == value) {
                    return texture;
                }
            }
        }
        return null;
    }

    private static VertexBuffer findVertexBuffer(long value) {
        GraphicsResource result = findResource(value, VertexBuffer.class);
        return result instanceof VertexBuffer buffer ? buffer : null;
    }

    private static IndexBuffer findIndexBuffer(long value) {
        GraphicsResource result = findResource(value, IndexBuffer.class);
        return result instanceof IndexBuffer buffer ? buffer : null;
    }

    private static GraphicsResource findResource(
            long value, Class<? extends GraphicsResource> expectedType) {
        synchronized (GAMES) {
            for (Map.Entry<GraphicsResource, NativeResourceHandle> entry : RESOURCES.entrySet()) {
                if (expectedType.isInstance(entry.getKey())
                        && !entry.getValue().isClosed()
                        && entry.getValue().requireValue() == value) {
                    return entry.getKey();
                }
            }
        }
        return null;
    }

    private static void requireResourceOwner(
            GraphicsDevice device, GraphicsResource resource) {
        if (resource.getGraphicsDevice() != device) {
            throw new IllegalArgumentException(
                    resource.getClass().getSimpleName()
                            + " belongs to a different GraphicsDevice");
        }
    }

    private static int[] requireLength(int[] values, int length, String name) {
        Objects.requireNonNull(values, "values");
        if (values.length != length) {
            throw new IllegalArgumentException(name + " descriptor must contain " + length + " values");
        }
        return Arrays.copyOf(values, values.length);
    }

    private static float[] requireLength(float[] values, int length, String name) {
        Objects.requireNonNull(values, "values");
        if (values.length != length) {
            throw new IllegalArgumentException(name + " descriptor must contain " + length + " values");
        }
        return Arrays.copyOf(values, values.length);
    }

    private static void requireOutputLength(int[] values, int length, String name) {
        Objects.requireNonNull(values, "values");
        if (values.length != length) {
            throw new IllegalArgumentException(name + " output must contain " + length + " values");
        }
    }

    private static void requireOutputLength(float[] values, int length, String name) {
        Objects.requireNonNull(values, "values");
        if (values.length != length) {
            throw new IllegalArgumentException(name + " output must contain " + length + " values");
        }
    }

    private static void destroyTexture2D(long handle) {
        check("cna_texture2d_destroy", nativeDestroyTexture2D(handle));
    }

    private static void destroyTexture3D(long handle) {
        check("cna_texture3d_destroy", nativeDestroyTexture3D(handle));
    }

    private static void destroyEffect(long handle) {
        check("cna_effect_destroy", nativeDestroyEffectObject(handle, 0));
    }

    private static void destroyOcclusionQuery(long handle) {
        check("cna_occlusion_query_destroy", nativeDestroyOcclusionQuery(handle));
    }

    private static void destroyEffectMember(long handle, int kind) {
        check(effectDestroyOperation(kind), nativeDestroyEffectObject(handle, kind));
    }

    private static String effectDestroyOperation(int kind) {
        return switch (kind) {
            case 0 -> "cna_effect_destroy";
            case 1 -> "cna_effect_parameter_collection_destroy";
            case 2 -> "cna_effect_technique_collection_destroy";
            case 3 -> "cna_effect_pass_collection_destroy";
            case 4 -> "cna_effect_annotation_collection_destroy";
            case 5 -> "cna_effect_parameter_destroy";
            case 6 -> "cna_effect_technique_destroy";
            case 7 -> "cna_effect_pass_destroy";
            case 8 -> "cna_effect_annotation_destroy";
            case 9 -> "cna_directional_light_destroy";
            default -> throw new IllegalArgumentException("Unknown Effect handle kind " + kind);
        };
    }

    private static void destroyVertexBuffer(long handle) {
        check("cna_vertex_buffer_destroy", nativeDestroyVertexBuffer(handle));
    }

    private static void destroyIndexBuffer(long handle) {
        check("cna_index_buffer_destroy", nativeDestroyIndexBuffer(handle));
    }

    private static void destroyTextureCube(long handle) {
        check("cna_texturecube_destroy", nativeDestroyTextureCube(handle));
    }

    private static void destroyRenderTarget(long handle) {
        check("cna_render_target_destroy", nativeDestroyRenderTarget(handle));
    }

    private static void destroySpriteBatch(long handle) {
        check("cna_sprite_batch_destroy", nativeDestroySpriteBatch(handle));
    }

    private static void destroySpriteFont(long handle) {
        check("cna_sprite_font_destroy", nativeDestroySpriteFont(handle));
    }

    private static void destroyContentManager(long handle) {
        check("cna_content_manager_destroy", nativeDestroyContentManager(handle));
    }

    private static WindowHandle findWindowHandle(long value) {
        for (Map.Entry<WindowHandle, Long> entry : WINDOW_HANDLES.entrySet()) {
            if (entry.getValue() == value) {
                return entry.getKey();
            }
        }
        return null;
    }

    /** Wraps an opaque host window value the extensions hand back to a game. */
    public static WindowHandle knownWindowHandle(long value, String operation) {
        if (value == 0L) {
            return WindowHandle.Zero;
        }
        synchronized (GAMES) {
            WindowHandle known = findWindowHandle(value);
            if (known != null) {
                return known;
            }
        }
        throw new IllegalStateException(
                "CNA returned an unrecognized opaque token for " + operation);
    }

    /** Unwraps a host window value the extensions pass into CNA. */
    public static long knownWindowValue(WindowHandle window, String operation) {
        synchronized (GAMES) {
            Long registered = WINDOW_HANDLES.get(Objects.requireNonNull(window, "window"));
            if (registered == null && !window.getIsZero()) {
                throw new IllegalArgumentException(
                        operation + " accepts only an opaque token issued by CNA-Java");
            }
            return registered == null ? 0L : registered;
        }
    }

    private static String getGraphicsAdapterString(
            int adapterIndex,
            long byteLength,
            boolean description) {
        int size = Math.toIntExact(byteLength);
        byte[] utf8 = new byte[size];
        check(description
                        ? "cna_graphics_adapter_copy_description"
                        : "cna_graphics_adapter_copy_device_name",
                nativeCopyGraphicsAdapterString(
                        currentGameHandle("GraphicsAdapter metadata").requireValue(),
                        adapterIndex, description, utf8));
        return new String(utf8, StandardCharsets.UTF_8);
    }

    private static boolean booleanResult(String operation, int result) {
        if (result < 0) {
            throw failure(operation, -result);
        }
        return result != 0;
    }

    private static long longResult(String operation, long result) {
        if (result < 0L) {
            throw failure(operation, Math.toIntExact(-result));
        }
        return result;
    }

    public static CnaNativeException failure(String operation, int result) {
        String diagnostic;
        try {
            diagnostic = nativeLastErrorMessage();
        } catch (LinkageError ignored) {
            diagnostic = "native diagnostic unavailable";
        }
        return new CnaNativeException(operation, result, diagnostic);
    }

    private static void loadBridge() {
        if (bridgeLoaded) {
            return;
        }
        String explicit = firstNonBlank(
                System.getProperty("cna.java.jniLibrary"),
                System.getenv("CNA_JNI_LIBRARY"));
        if (explicit != null) {
            System.load(Path.of(explicit).toAbsolutePath().normalize().toString());
        } else {
            System.loadLibrary("cna_java_jni");
        }
        bridgeLoaded = true;
    }

    private static String configuredCnaLibrary() {
        String explicit = firstNonBlank(
                System.getProperty("cna.native.library"),
                System.getenv("CNA_NATIVE_LIBRARY"));
        if (explicit != null) {
            return Path.of(explicit).toAbsolutePath().normalize().toString();
        }

        String directory = firstNonBlank(
                System.getProperty("cna.native.dir"),
                System.getenv("CNA_NATIVE_DIR"));
        if (directory == null) {
            return null;
        }
        return Path.of(directory, platformCnaLibraryName()).toAbsolutePath().normalize().toString();
    }

    private static String platformCnaLibraryName() {
        String os = System.getProperty("os.name", "").toLowerCase(Locale.ROOT);
        if (os.contains("win")) {
            return "cna_c_api.dll";
        }
        if (os.contains("mac") || os.contains("darwin")) {
            return "libcna_c_api.dylib";
        }
        return "libcna_c_api.so";
    }

    private static String firstNonBlank(String first, String second) {
        if (first != null && !first.isBlank()) {
            return first;
        }
        return second != null && !second.isBlank() ? second : null;
    }

    private static int encodeVersion(int major, int minor, int patch) {
        return ((major & 0xffff) << 16) | ((minor & 0xff) << 8) | (patch & 0xff);
    }

    private static int versionMajor(int version) {
        return version >>> 16;
    }

    private static native int nativeLoadCna(String path);

    private static native long nativeCreateGame(
            Game game, byte[] titleUtf8, boolean fixedTimeStep, long targetElapsedTimeTicks);

    private static native int nativeRun(long game);

    private static native int nativeRunOneFrame(long game);

    private static native int nativeResetElapsedTime(long game);

    private static native int nativeSuppressDraw(long game);

    private static native int nativeTick(long game);

    private static native int nativeRequestExit(long game);

    private static native int nativeClear(long game, int red, int green, int blue, int alpha);

    private static native int nativeSetMouseVisible(long game, boolean visible);

    private static native int nativeGetMouseVisible(long game);

    private static native int nativeGetIsActive(long game);

    private static native int nativeSetFixedTimeStep(long game, boolean value);

    private static native int nativeGetFixedTimeStep(long game);

    private static native int nativeSetTargetElapsedTime(long game, long ticks);

    private static native long nativeGetTargetElapsedTime(long game);

    private static native int nativeSetInactiveSleepTime(long game, long ticks);

    private static native long nativeGetInactiveSleepTime(long game);

    private static native int nativeGetWindowAllowUserResizing(long game);

    private static native int nativeSetWindowAllowUserResizing(long game, boolean value);

    private static native int nativeGetWindowClientBounds(long game, int[] value);

    private static native long nativeGetWindowCurrentOrientation(long game);

    private static native int nativeGetWindowHandle(long game, long[] value);

    private static native long nativeGetWindowScreenDeviceNameSize(long game);

    private static native int nativeCopyWindowScreenDeviceName(long game, byte[] destination);

    private static native int nativeSetWindowTitle(long game, byte[] titleUtf8);

    private static native int nativeBeginWindowScreenDeviceChange(
            long game, boolean willBeFullScreen);

    private static native int nativeEndWindowScreenDeviceChange(
            long game, byte[] screenDeviceNameUtf8, int clientWidth, int clientHeight);

    private static native int nativeGetKeyboardState(
            long game, int playerIndex, long[] words);

    private static native int nativeGetGamePadState(
            long game, int playerIndex, int deadZone, int[] discrete, float[] analog);

    private static native int nativeGetGamePadCapabilities(
            long game, int playerIndex, int[] capabilities);

    private static native int nativeSetGamePadVibration(
            long game, int playerIndex, float leftMotor, float rightMotor);

    private static native int nativeGetTouchCapabilities(long game, int[] output);

    private static native int nativeGetTouchState(
            long game, int[] discrete, float[] positions);

    private static native int nativeTouchPanelGetDisplayWidth(long game, int[] output);

    private static native int nativeTouchPanelSetDisplayWidth(long game, int value);

    private static native int nativeTouchPanelGetDisplayHeight(long game, int[] output);

    private static native int nativeTouchPanelSetDisplayHeight(long game, int value);

    private static native int nativeTouchPanelGetDisplayOrientation(long game, int[] output);

    private static native int nativeTouchPanelSetDisplayOrientation(long game, int value);

    private static native int nativeTouchPanelGetEnabledGestures(long game, int[] output);

    private static native int nativeTouchPanelSetEnabledGestures(long game, int value);

    private static native int nativeTouchPanelGetIsGestureAvailable(long game);

    private static native int nativeTouchPanelGetWindowHandle(long game, long[] output);

    private static native int nativeTouchPanelSetWindowHandle(long game, long value);

    private static native int nativeReadTouchGesture(
            long game, int[] type, long[] timestamp, float[] vectors);

    private static native int nativeSetTouchDeviceExists(long game, boolean value);

    private static native int nativeSetTouchFinger(
            long game, int index, int fingerId, float x, float y);

    private static native int nativeRaiseTouchEvent(
            long game, int fingerId, int state,
            float x, float y, float deltaX, float deltaY);

    private static native int nativeEnqueueTouchGesture(
            long game, int type, long timestampTicks, float[] vectors);

    private static native int nativeUpdateTouchPanel(long game);

    private static native int nativeResetTouchPanel(long game);

    private static native int nativeCreateGraphicsDeviceManager(
            long game, GraphicsDeviceManager manager, long[] output);

    private static native long nativeGetGraphicsDeviceManagerGraphicsProfile(long manager);

    private static native int nativeSetGraphicsDeviceManagerGraphicsProfile(
            long manager, int value);

    private static native int nativeGetGraphicsDeviceManagerIsFullScreen(long manager);

    private static native int nativeSetGraphicsDeviceManagerIsFullScreen(
            long manager, boolean value);

    private static native int nativeGetGraphicsDeviceManagerPreferMultiSampling(long manager);

    private static native int nativeSetGraphicsDeviceManagerPreferMultiSampling(
            long manager, boolean value);

    private static native long nativeGetGraphicsDeviceManagerPreferredBackBufferFormat(
            long manager);

    private static native int nativeSetGraphicsDeviceManagerPreferredBackBufferFormat(
            long manager, int value);

    private static native long nativeGetGraphicsDeviceManagerPreferredBackBufferWidth(long manager);

    private static native int nativeSetGraphicsDeviceManagerPreferredBackBufferWidth(
            long manager, int value);

    private static native long nativeGetGraphicsDeviceManagerPreferredBackBufferHeight(long manager);

    private static native int nativeSetGraphicsDeviceManagerPreferredBackBufferHeight(
            long manager, int value);

    private static native long nativeGetGraphicsDeviceManagerPreferredDepthStencilFormat(
            long manager);

    private static native int nativeSetGraphicsDeviceManagerPreferredDepthStencilFormat(
            long manager, int value);

    private static native int nativeGetGraphicsDeviceManagerSynchronizeWithVerticalRetrace(
            long manager);

    private static native int nativeSetGraphicsDeviceManagerSynchronizeWithVerticalRetrace(
            long manager, boolean value);

    private static native long nativeGetGraphicsDeviceManagerSupportedOrientations(long manager);

    private static native int nativeSetGraphicsDeviceManagerSupportedOrientations(
            long manager, int value);

    private static native int nativeApplyGraphicsDeviceManagerChanges(long manager);

    private static native int nativeToggleGraphicsDeviceManagerFullScreen(long manager);

    private static native int nativeCreateGraphicsDeviceManagerDevice(long manager);

    private static native int nativeBeginGraphicsDeviceManagerDraw(long manager);

    private static native int nativeEndGraphicsDeviceManagerDraw(long manager);

    private static native int nativeDisposeGraphicsDeviceManager(long manager);

    private static native int nativeDestroyGraphicsDeviceManager(long manager);

    private static native long nativeGetGraphicsAdapterCount(long game);

    private static native int nativeGetGraphicsAdapterInfo(
            long game, int adapterIndex, long[] output);

    private static native int nativeCopyGraphicsAdapterString(
            long game, int adapterIndex, boolean description, byte[] output);

    private static native int nativeGetGraphicsAdapterCurrentDisplayMode(
            long game, int adapterIndex, int[] output);

    private static native long nativeGetGraphicsAdapterDisplayModeCount(
            long game, int adapterIndex);

    private static native int nativeCopyGraphicsAdapterDisplayModes(
            long game, int adapterIndex, int[] output);

    private static native int nativeSetGraphicsAdapterDevicePreferences(
            long game, int adapterIndex, boolean useNullDevice, boolean useReferenceDevice);

    private static native int nativeIsGraphicsAdapterProfileSupported(
            long game, int adapterIndex, int profile);

    private static native int nativeQueryGraphicsAdapterFormat(
            long game, int adapterIndex, boolean backBuffer, int profile,
            int format, int depthFormat, int multiSampleCount, int[] output);

    private static native int nativeGetGraphicsAdapterMonitorHandle(
            long game, int adapterIndex, long[] output);

    private static native int nativeEnsureGraphicsDeviceEvents(
            long game, GraphicsDevice device);

    private static native int nativeGetGraphicsDeviceIsDisposed(long game);

    private static native long nativeGetGraphicsDeviceStatus(long game);

    private static native long nativeGetGraphicsDeviceAdapterIndex(long game);

    private static native long nativeGetGraphicsDeviceProfile(long game);

    private static native int nativeSetGraphicsDeviceProfile(long game, int profile);

    private static native int nativeGetGraphicsDeviceScissorRectangle(
            long game, int[] output);

    private static native int nativeSetGraphicsDeviceScissorRectangle(
            long game, int x, int y, int width, int height);

    private static native int nativeGetGraphicsDeviceViewport(
            long game, int[] bounds, float[] depth);

    private static native int nativeSetGraphicsDeviceViewport(
            long game, int x, int y, int width, int height,
            float minDepth, float maxDepth);

    private static native long nativeGetGraphicsDeviceBlendFactor(long game);

    private static native int nativeSetGraphicsDeviceBlendFactor(
            long game, int packedColor);

    private static native int nativeGetGraphicsDeviceBlendState(
            long game, int[] output);

    private static native int nativeSetGraphicsDeviceBlendState(
            long game, int[] values);

    private static native int nativeGetGraphicsDeviceDepthStencilState(
            long game, int[] output);

    private static native int nativeSetGraphicsDeviceDepthStencilState(
            long game, int[] values);

    private static native int nativeGetGraphicsDeviceRasterizerState(
            long game, int[] integers, float[] floats);

    private static native int nativeSetGraphicsDeviceRasterizerState(
            long game, int[] integers, float[] floats);

    private static native int nativeGetGraphicsDeviceSamplerState(
            long game, int shaderStage, int slot, int[] integers, float[] bias);

    private static native int nativeSetGraphicsDeviceSamplerState(
            long game, int shaderStage, int slot, int[] integers, float bias);

    private static native int nativeGetGraphicsDeviceTexture(
            long game, int shaderStage, int slot, long[] output);

    private static native int nativeSetGraphicsDeviceTexture(
            long game, int shaderStage, int slot, long texture);

    private static native long nativeGetGraphicsDeviceMultiSampleMask(long game);

    private static native int nativeSetGraphicsDeviceMultiSampleMask(long game, int value);

    private static native long nativeGetGraphicsDeviceReferenceStencil(long game);

    private static native int nativeSetGraphicsDeviceReferenceStencil(long game, int value);

    private static native int nativeGetGraphicsDevicePresentationParameters(
            long game, int[] output);

    private static native int nativeGetGraphicsDeviceDisplayMode(long game, int[] output);

    private static native int nativeGetGraphicsDeviceBackBufferInfo(long game, int[] output);

    private static native int nativeGetGraphicsDeviceBackBufferData(
            long game,
            boolean hasRectangle,
            int x,
            int y,
            int width,
            int height,
            int startIndex,
            int elementCount,
            int[] output);

    private static native int nativeClearGraphicsDevice(
            long game, int options, int packedColor, float depth, int stencil);

    private static native int nativePresentGraphicsDevice(long game);

    private static native int nativeResetGraphicsDevice(long game);

    private static native int nativeResetGraphicsDeviceWithParameters(
            long game, int[] parameters, int adapterIndex);

    private static native int nativeSetGraphicsDeviceVertexBuffer(
            long game, long vertexBuffer, int vertexOffset);

    private static native int nativeSetGraphicsDeviceVertexBuffers(
            long game, long[] vertexBuffers, int[] vertexOffsets, int[] instanceFrequencies);

    private static native long nativeGetGraphicsDeviceVertexBufferCount(long game);

    private static native int nativeCopyGraphicsDeviceVertexBuffers(
            long game, long[] vertexBuffers, int[] vertexOffsets, int[] instanceFrequencies);

    private static native int nativeSetGraphicsDeviceIndexBuffer(long game, long indexBuffer);

    private static native int nativeGetGraphicsDeviceIndexBuffer(long game, long[] output);

    private static native int nativeDrawPrimitives(
            long game, int primitiveType, int startVertex, int primitiveCount);

    private static native int nativeDrawIndexedPrimitives(
            long game,
            int primitiveType,
            int baseVertex,
            int minVertexIndex,
            int numVertices,
            int startIndex,
            int primitiveCount);

    private static native int nativeDrawInstancedPrimitives(
            long game,
            int primitiveType,
            int baseVertex,
            int minVertexIndex,
            int numVertices,
            int startIndex,
            int primitiveCount,
            int instanceCount);

    private static native int nativeDrawUserPrimitives(
            long game,
            int primitiveType,
            int vertexSource,
            byte[] vertexData,
            int vertexStride,
            int vertexOffset,
            int numVertices,
            int primitiveCount,
            int[] declaration);

    private static native int nativeDrawUserIndexedPrimitives16(
            long game,
            int primitiveType,
            int vertexSource,
            byte[] vertexData,
            int vertexStride,
            int vertexOffset,
            int numVertices,
            short[] indexData,
            int indexOffset,
            int primitiveCount,
            int[] declaration);

    private static native int nativeDrawUserIndexedPrimitives32(
            long game,
            int primitiveType,
            int vertexSource,
            byte[] vertexData,
            int vertexStride,
            int vertexOffset,
            int numVertices,
            int[] indexData,
            int indexOffset,
            int primitiveCount,
            int[] declaration);

    private static native int nativeGetMouseState(long game, int[] state);

    private static native int nativeSetMousePosition(long game, int x, int y);

    private static native int nativeGetMouseWindowHandle(long game, long[] window);

    private static native int nativeSetMouseWindowHandle(long game, long window);

    private static native int nativeUpdateFrameworkDispatcher(long game);

    private static native int nativeSetGamerServicesWindowHandle(long window);

    private static native int nativeInitializeGamerServices(long game);

    private static native int nativeUpdateGamerServices();

    private static native int nativeCreateVertexBuffer(
            long game,
            int vertexStride,
            int[] declaration,
            int vertexCount,
            int usage,
            boolean dynamic,
            long[] output);

    private static native int nativeGetVertexBufferInfo(long vertexBuffer, int[] output);

    private static native int nativeSetVertexBufferData(
            long vertexBuffer,
            int offsetInBytes,
            int vertexType,
            byte[] payload,
            int vertexCount,
            int vertexStride,
            int options);

    private static native int nativeSubscribeVertexBufferContentLost(
            long vertexBuffer, Object callbackTarget, long[] output);

    private static native int nativeUnsubscribeVertexBufferContentLost(long registration);

    private static native int nativeGetVertexBufferData(
            long vertexBuffer,
            int offsetInBytes,
            int vertexCount,
            int vertexStride,
            byte[] output);

    private static native int nativeDestroyVertexBuffer(long vertexBuffer);

    private static native int nativeCreateIndexBuffer(
            long game,
            int indexElementSize,
            int indexCount,
            int usage,
            boolean dynamic,
            long[] output);

    private static native int nativeGetIndexBufferInfo(long indexBuffer, int[] output);

    private static native int nativeSetIndexBufferData(
            long indexBuffer, int offsetInBytes, int indexElementSize,
            int[] values, int options);

    private static native int nativeSubscribeIndexBufferContentLost(
            long indexBuffer, Object callbackTarget, long[] output);

    private static native int nativeUnsubscribeIndexBufferContentLost(long registration);

    private static native int nativeGetIndexBufferData(
            long indexBuffer, int indexElementSize, int[] output);

    private static native int nativeDestroyIndexBuffer(long indexBuffer);

    private static native int nativeCreateContentManager(
            long game, byte[] rootDirectory, long[] output);

    private static native int nativeSetContentManagerRootDirectory(
            long contentManager, byte[] rootDirectory);

    private static native int nativeLoadContentTexture2D(
            long contentManager, byte[] assetName, long[] output);

    private static native int nativeLoadContentSpriteFont(
            long contentManager, byte[] assetName, long[] output);

    private static native int nativeUnloadContentManager(long contentManager);

    private static native int nativeRegisterContentManagerBuiltinLoaders(long contentManager);

    private static native int nativeDestroyContentManager(long contentManager);

    private static native int nativeCreateEffect(
            long game, byte[] effectCode, boolean empty, long[] output);

    private static native int nativeCreateBasicEffect(long game, long[] output);

    private static native int nativeCreateStockEffect(
            long game, int effectKind, long[] output);

    private static native int nativeCreateEffectMaterial(long source, long[] output);

    private static native int nativeCloneEffect(long effect, long[] output);

    private static native int nativeApplyEffect(long effect);

    private static native int nativeGetEffectChild(long handle, int kind, long[] output);

    private static native int nativeSetEffectCurrentTechnique(long effect, long technique);

    private static native int nativeDestroyEffectObject(long handle, int kind);

    private static native long nativeGetEffectCollectionCount(long collection, int kind);

    private static native int nativeGetEffectCollectionElement(
            long collection, int kind, int index, long[] output);

    private static native long nativeGetEffectStringSize(long handle, int kind);

    private static native int nativeCopyEffectString(long handle, int kind, byte[] output);

    private static native int nativeGetEffectInfo(long handle, int kind, int[] output);

    private static native long nativeGetEffectInts(
            long handle, boolean annotation, int valueType, int count, int[] output);

    private static native long nativeGetEffectFloats(
            long handle, boolean annotation, int valueType, int count, float[] output);

    private static native int nativeGetEffectIntValue(
            long parameter, int valueType, int[] output);

    private static native int nativeGetEffectFloatValue(
            long parameter, int valueType, float[] output);

    private static native int nativeSetEffectInts(
            long parameter, int valueType, int[] values);

    private static native int nativeSetEffectFloats(
            long parameter, int valueType, float[] values, int count);

    private static native int nativeSetEffectIntValue(
            long parameter, int valueType, int value);

    private static native int nativeSetEffectFloatValue(
            long parameter, int valueType, float[] value);

    private static native int nativeSetEffectString(long parameter, byte[] value);

    private static native int nativeGetEffectTexture(
            long parameter, int textureType, long[] output);

    private static native int nativeSetEffectTexture(
            long parameter, int textureType, long texture);

    private static native int nativeApplyEffectPass(long pass);

    private static native int nativeGetBasicEffectBoolean(long effect, int kind);

    private static native int nativeSetBasicEffectBoolean(
            long effect, int kind, boolean value);

    private static native int nativeGetBasicEffectFloat(
            long effect, int kind, float[] output);

    private static native int nativeSetBasicEffectFloat(long effect, int kind, float value);

    private static native int nativeGetBasicEffectVector(
            long effect, int kind, float[] output);

    private static native int nativeSetBasicEffectVector(
            long effect, int kind, float[] value);

    private static native int nativeGetBasicEffectMatrix(
            long effect, int kind, float[] output);

    private static native int nativeSetBasicEffectMatrix(
            long effect, int kind, float[] value);

    private static native int nativeGetBasicEffectDirectionalLight(
            long effect, int index, long[] output);

    private static native int nativeEnableDefaultLighting(long effect);

    private static native int nativeGetDirectionalLightVector(
            long light, int kind, float[] output);

    private static native int nativeSetDirectionalLightVector(
            long light, int kind, float[] value);

    private static native int nativeGetDirectionalLightEnabled(long light);

    private static native int nativeSetDirectionalLightEnabled(long light, boolean value);

    private static native int nativeSetBasicEffectTexture(long effect, long texture);

    private static native int nativeGetStockEffectBoolean(
            long effect, int effectKind, int kind);

    private static native int nativeSetStockEffectBoolean(
            long effect, int effectKind, int kind, boolean value);

    private static native int nativeGetStockEffectFloat(
            long effect, int effectKind, int kind, float[] output);

    private static native int nativeSetStockEffectFloat(
            long effect, int effectKind, int kind, float value);

    private static native int nativeGetStockEffectInt(
            long effect, int effectKind, int kind, int[] output);

    private static native int nativeSetStockEffectInt(
            long effect, int effectKind, int kind, int value);

    private static native int nativeGetStockEffectVector(
            long effect, int effectKind, int kind, float[] output);

    private static native int nativeSetStockEffectVector(
            long effect, int effectKind, int kind, float[] value);

    private static native int nativeSetStockEffectTexture(
            long effect, int effectKind, int slot, long texture);

    private static native int nativeSetSkinnedEffectBoneTransforms(
            long effect, float[] transforms);

    private static native int nativeGetSkinnedEffectBoneTransforms(
            long effect, int count, float[] output);

    private static native int nativeCreateOcclusionQuery(long game, long[] output);

    private static native int nativeBeginOcclusionQuery(long query);

    private static native int nativeEndOcclusionQuery(long query);

    private static native int nativeGetOcclusionQueryComplete(long query);

    private static native int nativeGetOcclusionQueryPixelCount(long query, int[] output);

    private static native int nativeDestroyOcclusionQuery(long query);

    private static native int nativeCreateTexture2D(
            long game, int width, int height, boolean mipMap, int format, long[] output);

    private static native int nativeCreateTexture2DFromEncoded(
            long game, byte[] encoded, int width, int height, boolean zoom, boolean resize,
            long[] output);

    private static native int nativeGetTexture2DInfo(long texture, int[] output);

    private static native int nativeSetTexture2DData(long texture, int[] packedColors);

    private static native int nativeGetTexture2DData(long texture, int[] packedColors);

    private static native int nativeSetTexture2DTypedData(
            long texture,
            int dataType,
            int level,
            boolean hasRectangle,
            int x,
            int y,
            int width,
            int height,
            int startIndex,
            int elementCount,
            byte[] payload);

    private static native int nativeGetTexture2DTypedData(
            long texture,
            int dataType,
            int level,
            boolean hasRectangle,
            int x,
            int y,
            int width,
            int height,
            int startIndex,
            int elementCount,
            byte[] payload);

    private static native long nativeGetTexture2DEncodedSize(
            long texture, int format, int width, int height);

    private static native int nativeCopyTexture2DEncoded(
            long texture, int format, int width, int height, byte[] output);

    private static native int nativeDestroyTexture2D(long texture);

    private static native int nativeCreateTexture3D(
            long game,
            int width,
            int height,
            int depth,
            boolean mipMap,
            int format,
            long[] output);

    private static native int nativeGetTexture3DInfo(long texture, int[] output);

    private static native int nativeSetTexture3DData(
            long texture,
            int level,
            int left,
            int top,
            int right,
            int bottom,
            int front,
            int back,
            int startIndex,
            int elementCount,
            int[] packedColors);

    private static native int nativeGetTexture3DData(
            long texture,
            int level,
            int left,
            int top,
            int right,
            int bottom,
            int front,
            int back,
            int startIndex,
            int elementCount,
            int[] packedColors);

    private static native int nativeDestroyTexture3D(long texture);

    private static native int nativeCreateSpriteFont(
            long texture,
            int[] rectangles,
            char[] characters,
            float[] kerning,
            int lineSpacing,
            float spacing,
            boolean hasDefaultCharacter,
            int defaultCharacter,
            long[] output);

    private static native int nativeGetSpriteFontInfo(
            long spriteFont, int[] integers, float[] spacing);

    private static native int nativeCopySpriteFontCharacters(
            long spriteFont, char[] characters);

    private static native int nativeSetSpriteFontDefaultCharacter(
            long spriteFont, boolean hasValue, int value);

    private static native int nativeSetSpriteFontLineSpacing(long spriteFont, int value);

    private static native int nativeSetSpriteFontSpacing(long spriteFont, float value);

    private static native int nativeMeasureSpriteFont(
            long spriteFont, byte[] text, float[] output);

    private static native int nativeDestroySpriteFont(long spriteFont);

    private static native int nativeCreateTextureCube(
            long game, int size, boolean mipMap, int format, long[] output);

    private static native int nativeGetTextureCubeInfo(long texture, int[] output);

    private static native int nativeSetTextureCubeData(
            long texture,
            int face,
            int level,
            boolean hasRectangle,
            int x,
            int y,
            int width,
            int height,
            int startIndex,
            int elementCount,
            int[] packedColors);

    private static native int nativeGetTextureCubeData(
            long texture,
            int face,
            int level,
            boolean hasRectangle,
            int x,
            int y,
            int width,
            int height,
            int startIndex,
            int elementCount,
            int[] packedColors);

    private static native int nativeDestroyTextureCube(long texture);

    private static native int nativeCreateRenderTarget2D(
            long game,
            int width,
            int height,
            boolean mipMap,
            int format,
            int depthFormat,
            int multiSampleCount,
            int usage,
            long[] output);

    private static native int nativeCreateRenderTargetCube(
            long game,
            int size,
            boolean mipMap,
            int format,
            int depthFormat,
            int multiSampleCount,
            int usage,
            long[] output);

    private static native int nativeGetRenderTargetInfo(long renderTarget, int[] output);

    private static native int nativeSetGraphicsDeviceRenderTarget2D(
            long game, long renderTarget);

    private static native int nativeSetGraphicsDeviceRenderTargetCube(
            long game, long renderTarget, int face);

    private static native int nativeSetGraphicsDeviceRenderTargets(
            long game, long[] renderTargets, int[] faces);

    private static native long nativeGetGraphicsDeviceRenderTargetCount(long game);

    private static native int nativeCopyGraphicsDeviceRenderTargets(
            long game, long[] renderTargets, int[] faces);

    private static native int nativeDestroyRenderTarget(long renderTarget);

    private static native int nativeCreateSpriteBatch(long game, long[] output);

    private static native int nativeBeginSpriteBatch(long spriteBatch, int sortMode);

    private static native int nativeBeginSpriteBatchWithStates(
            long spriteBatch,
            int sortMode,
            int[] blend,
            int[] sampler,
            float samplerBias,
            int[] depthStencil,
            int[] rasterizer,
            float[] rasterizerFloats);

    private static native int nativeBeginSpriteBatchWithEffect(
            long spriteBatch,
            int sortMode,
            int[] blend,
            int[] sampler,
            float samplerBias,
            int[] depthStencil,
            int[] rasterizer,
            float[] rasterizerFloats,
            long effect,
            float[] transform);

    private static native int nativeDrawSpriteRectangle(
            long spriteBatch, long texture,
            int destinationX, int destinationY, int destinationWidth, int destinationHeight,
            int sourceX, int sourceY, int sourceWidth, int sourceHeight,
            int packedColor, float rotation, float originX, float originY,
            int effects, float layerDepth);

    private static native int nativeDrawSpriteScaled(
            long spriteBatch, long texture, float positionX, float positionY,
            int sourceX, int sourceY, int sourceWidth, int sourceHeight,
            int packedColor, float rotation, float originX, float originY,
            float scaleX, float scaleY, int effects, float layerDepth);

    private static native int nativeDrawSpriteString(
            long spriteBatch,
            long spriteFont,
            byte[] text,
            float positionX,
            float positionY,
            int packedColor,
            float rotation,
            float originX,
            float originY,
            float scaleX,
            float scaleY,
            int effects,
            float layerDepth);

    private static native int nativeEndSpriteBatch(long spriteBatch);

    private static native int nativeDestroySpriteBatch(long spriteBatch);

    private static native int nativeDestroyGame(long game);

    private static native String nativeLastErrorMessage();
}
