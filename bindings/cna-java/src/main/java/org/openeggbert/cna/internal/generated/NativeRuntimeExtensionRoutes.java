package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeRuntimeExtensionRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeRuntimeExtensionRoutes {

    private NativeRuntimeExtensionRoutes() {
    }

    /**
     * cna_assembly_copy_title_ext (core_ext.h).
     */
    public static native int assemblyCopyTitleExt(byte[] destination, long[] outBytes);

    /**
     * cna_assembly_set_title_ext (core_ext.h).
     */
    public static native int assemblySetTitleExt(byte[] title);

    /**
     * cna_desktop_os_get_current (core_ext.h).
     */
    public static native int desktopOsGetCurrent(int[] outOs);

    /**
     * cna_graphics_backend_category_copy_name (core_ext.h).
     */
    public static native int graphicsBackendCategoryCopyName(int category, byte[] destination, long[] outBytes);

    /**
     * cna_graphics_backend_category_get_name_size (core_ext.h).
     */
    public static native int graphicsBackendCategoryGetNameSize(int category, long[] outBytes);

    /**
     * cna_graphics_backend_get_category (core_ext.h).
     */
    public static native int graphicsBackendGetCategory(int type, int[] outCategory);

    /**
     * cna_graphics_backend_get_current_category (core_ext.h).
     */
    public static native int graphicsBackendGetCurrentCategory(int[] outCategory);

    /**
     * cna_graphics_backend_get_current_maturity (core_ext.h).
     */
    public static native int graphicsBackendGetCurrentMaturity(int[] outMaturity);

    /**
     * cna_graphics_backend_get_maturity (core_ext.h).
     */
    public static native int graphicsBackendGetMaturity(int type, int[] outMaturity);

    /**
     * cna_graphics_backend_maturity_copy_name (core_ext.h).
     */
    public static native int graphicsBackendMaturityCopyName(int maturity, byte[] destination, long[] outBytes);

    /**
     * cna_graphics_backend_maturity_get_name_size (core_ext.h).
     */
    public static native int graphicsBackendMaturityGetNameSize(int maturity, long[] outBytes);

    /**
     * cna_graphics_renderer_copy_available_ext (core_ext.h).
     */
    public static native int graphicsRendererCopyAvailableExt(int[] destination, long[] outCount);

    /**
     * cna_graphics_renderer_copy_current_name (core_ext.h).
     */
    public static native int graphicsRendererCopyCurrentName(byte[] destination, long[] outBytes);

    /**
     * cna_graphics_renderer_fallback_copy_message_ext (core_ext.h).
     */
    public static native int graphicsRendererFallbackCopyMessageExt(long index, byte[] destination, long[] outBytes);

    /**
     * cna_graphics_renderer_fallback_get_message_size_ext (core_ext.h).
     */
    public static native int graphicsRendererFallbackGetMessageSizeExt(long index, long[] outBytes);

    /**
     * cna_graphics_renderer_fallback_reason_copy_name_ext (core_ext.h).
     */
    public static native int graphicsRendererFallbackReasonCopyNameExt(int reason, byte[] destination, long[] outBytes);

    /**
     * cna_graphics_renderer_fallback_reason_get_name_size_ext (core_ext.h).
     */
    public static native int graphicsRendererFallbackReasonGetNameSizeExt(int reason, long[] outBytes);

    /**
     * cna_graphics_renderer_get_active_ext (core_ext.h).
     */
    public static native int graphicsRendererGetActiveExt(int[] outType);

    /**
     * cna_graphics_renderer_get_automatic_fallback_ext (core_ext.h).
     */
    public static native int graphicsRendererGetAutomaticFallbackExt(boolean[] outEnabled);

    /**
     * cna_graphics_renderer_get_available_count_ext (core_ext.h).
     */
    public static native int graphicsRendererGetAvailableCountExt(long[] outCount);

    /**
     * cna_graphics_renderer_get_current_name_size (core_ext.h).
     */
    public static native int graphicsRendererGetCurrentNameSize(long[] outBytes);

    /**
     * cna_graphics_renderer_get_fallback_at_ext (core_ext.h).
     *
     * <p>outRecordIntegral carries CNA_GraphicsRendererFallbackRecord in this order:
     * <ol start="0">
     *   <li>{@code type} (CNA_GraphicsRendererType)</li>
     *   <li>{@code reason} (CNA_GraphicsRendererFallbackReason)</li>
     * </ol>
     */
    public static native int graphicsRendererGetFallbackAtExt(long index, long[] outRecordIntegral);

    /**
     * cna_graphics_renderer_get_fallback_count_ext (core_ext.h).
     */
    public static native int graphicsRendererGetFallbackCountExt(long[] outCount);

    /**
     * cna_graphics_renderer_get_is_available_ext (core_ext.h).
     */
    public static native int graphicsRendererGetIsAvailableExt(int type, boolean[] outAvailable);

    /**
     * cna_graphics_renderer_get_is_latched_ext (core_ext.h).
     */
    public static native int graphicsRendererGetIsLatchedExt(boolean[] outLatched);

    /**
     * cna_graphics_renderer_get_selected_ext (core_ext.h).
     */
    public static native int graphicsRendererGetSelectedExt(int[] outType);

    /**
     * cna_graphics_renderer_reset_selection_for_tests_ext (core_ext.h).
     */
    public static native int graphicsRendererResetSelectionForTestsExt();

    /**
     * cna_graphics_renderer_set_automatic_fallback_ext (core_ext.h).
     */
    public static native int graphicsRendererSetAutomaticFallbackExt(boolean enabled);

    /**
     * cna_graphics_renderer_set_fallback_chain_ext (core_ext.h).
     */
    public static native int graphicsRendererSetFallbackChainExt(int[] types);

    /**
     * cna_graphics_renderer_set_preferred_by_name_ext (core_ext.h).
     */
    public static native int graphicsRendererSetPreferredByNameExt(byte[] name);

    /**
     * cna_graphics_renderer_set_preferred_ext (core_ext.h).
     */
    public static native int graphicsRendererSetPreferredExt(int type);

    /**
     * cna_graphics_renderer_try_parse_name_ext (core_ext.h).
     */
    public static native int graphicsRendererTryParseNameExt(byte[] name, int[] outType, boolean[] outRecognized);

    /**
     * cna_logger_get_minimum_level (core_ext.h).
     */
    public static native int loggerGetMinimumLevel(int[] outLevel);

    /**
     * cna_logger_log (core_ext.h).
     */
    public static native int loggerLog(int level, byte[] message, int category, boolean condition);

    /**
     * cna_logger_set_minimum_level (core_ext.h).
     */
    public static native int loggerSetMinimumLevel(int level);

    /**
     * cna_platform_copy_current_name_ext (core_ext.h).
     */
    public static native int platformCopyCurrentNameExt(byte[] destination, long[] outBytes);

    /**
     * cna_platform_get_current (core_ext.h).
     */
    public static native int platformGetCurrent(int[] outPlatform);

    /**
     * cna_platform_get_current_name_size_ext (core_ext.h).
     */
    public static native int platformGetCurrentNameSizeExt(long[] outBytes);

    /**
     * cna_platform_get_is_apple_ext (core_ext.h).
     */
    public static native int platformGetIsAppleExt(boolean[] outApple);

    /**
     * cna_platform_get_is_mobile_ext (core_ext.h).
     */
    public static native int platformGetIsMobileExt(boolean[] outMobile);
}
