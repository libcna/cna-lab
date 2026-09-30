package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeGraphicsExtensionRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeGraphicsExtensionRoutes {

    private NativeGraphicsExtensionRoutes() {
    }

    /**
     * cna_ascii_post_process_effect_create (graphics_ext.h).
     */
    public static native int asciiPostProcessEffectCreate(long graphicsDevice, long[] outEffect);

    /**
     * cna_ascii_post_process_effect_destroy (graphics_ext.h).
     */
    public static native int asciiPostProcessEffectDestroy(long effect);

    /**
     * cna_ascii_post_process_effect_draw (graphics_ext.h).
     *
     * <p>destinationRectangleIntegral carries CNA_Rectangle in this order:
     * <ol start="0">
     *   <li>{@code x} (int32_t)</li>
     *   <li>{@code y} (int32_t)</li>
     *   <li>{@code width} (int32_t)</li>
     *   <li>{@code height} (int32_t)</li>
     * </ol>
     */
    public static native int asciiPostProcessEffectDraw(long effect, long source, long[] destinationRectangleIntegral);

    /**
     * cna_ascii_post_process_effect_get_cell_size (graphics_ext.h).
     */
    public static native int asciiPostProcessEffectGetCellSize(long effect, int[] outWidth, int[] outHeight);

    /**
     * cna_ascii_post_process_effect_get_last_grid_dimensions (graphics_ext.h).
     */
    public static native int asciiPostProcessEffectGetLastGridDimensions(long effect, int[] outColumns, int[] outRows);

    /**
     * cna_ascii_post_process_effect_get_quantize_mode (graphics_ext.h).
     */
    public static native int asciiPostProcessEffectGetQuantizeMode(long effect, int[] outMode);

    /**
     * cna_ascii_post_process_effect_set_cell_size (graphics_ext.h).
     */
    public static native int asciiPostProcessEffectSetCellSize(long effect, int width, int height);

    /**
     * cna_ascii_post_process_effect_set_quantize_mode (graphics_ext.h).
     */
    public static native int asciiPostProcessEffectSetQuantizeMode(long effect, int mode);

    /**
     * cna_crt_effect_create (graphics_ext.h).
     */
    public static native int crtEffectCreate(long graphicsDevice, long[] outEffect);

    /**
     * cna_crt_effect_get_curvature (graphics_ext.h).
     */
    public static native int crtEffectGetCurvature(long effect, float[] outValue);

    /**
     * cna_crt_effect_get_mask_intensity (graphics_ext.h).
     */
    public static native int crtEffectGetMaskIntensity(long effect, float[] outValue);

    /**
     * cna_crt_effect_get_mask_type (graphics_ext.h).
     */
    public static native int crtEffectGetMaskType(long effect, int[] outMaskType);

    /**
     * cna_crt_effect_get_scanline_intensity (graphics_ext.h).
     */
    public static native int crtEffectGetScanlineIntensity(long effect, float[] outValue);

    /**
     * cna_crt_effect_get_vignette_intensity (graphics_ext.h).
     */
    public static native int crtEffectGetVignetteIntensity(long effect, float[] outValue);

    /**
     * cna_crt_effect_set_curvature (graphics_ext.h).
     */
    public static native int crtEffectSetCurvature(long effect, float value);

    /**
     * cna_crt_effect_set_mask_intensity (graphics_ext.h).
     */
    public static native int crtEffectSetMaskIntensity(long effect, float value);

    /**
     * cna_crt_effect_set_mask_type (graphics_ext.h).
     */
    public static native int crtEffectSetMaskType(long effect, int maskType);

    /**
     * cna_crt_effect_set_scanline_intensity (graphics_ext.h).
     */
    public static native int crtEffectSetScanlineIntensity(long effect, float value);

    /**
     * cna_crt_effect_set_vignette_intensity (graphics_ext.h).
     */
    public static native int crtEffectSetVignetteIntensity(long effect, float value);

    /**
     * cna_debug_draw_add_bounding_sphere (graphics_ext.h).
     *
     * <p>sphereFloating carries CNA_BoundingSphere in this order:
     * <ol start="0">
     *   <li>{@code center.x} (float)</li>
     *   <li>{@code center.y} (float)</li>
     *   <li>{@code center.z} (float)</li>
     *   <li>{@code radius} (float)</li>
     * </ol>
     */
    public static native int debugDrawAddBoundingSphere(long debug, float[] sphereFloating, long[] colourIntegral, int segments);

    /**
     * cna_debug_draw_add_box (graphics_ext.h).
     *
     * <p>boundsFloating carries CNA_BoundingBox in this order:
     * <ol start="0">
     *   <li>{@code min.x} (float)</li>
     *   <li>{@code min.y} (float)</li>
     *   <li>{@code min.z} (float)</li>
     *   <li>{@code max.x} (float)</li>
     *   <li>{@code max.y} (float)</li>
     *   <li>{@code max.z} (float)</li>
     * </ol>
     */
    public static native int debugDrawAddBox(long debug, float[] boundsFloating, long[] colourIntegral);

    /**
     * cna_debug_draw_add_cross (graphics_ext.h).
     *
     * <p>positionFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int debugDrawAddCross(long debug, float[] positionFloating, float size, long[] colourIntegral);

    /**
     * cna_debug_draw_add_frustum (graphics_ext.h).
     */
    public static native int debugDrawAddFrustum(long debug, float[] frustumFloating, long[] colourIntegral);

    /**
     * cna_debug_draw_add_line (graphics_ext.h).
     *
     * <p>fromFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     *
     * <p>toFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int debugDrawAddLine(long debug, float[] fromFloating, float[] toFloating, long[] colourIntegral);

    /**
     * cna_debug_draw_add_sphere (graphics_ext.h).
     *
     * <p>centreFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int debugDrawAddSphere(long debug, float[] centreFloating, float radius, long[] colourIntegral, int segments);

    /**
     * cna_debug_draw_begin (graphics_ext.h).
     *
     * <p>viewFloating carries CNA_Matrix in this order:
     * <ol start="0">
     *   <li>{@code m11} (float)</li>
     *   <li>{@code m12} (float)</li>
     *   <li>{@code m13} (float)</li>
     *   <li>{@code m14} (float)</li>
     *   <li>{@code m21} (float)</li>
     *   <li>{@code m22} (float)</li>
     *   <li>{@code m23} (float)</li>
     *   <li>{@code m24} (float)</li>
     *   <li>{@code m31} (float)</li>
     *   <li>{@code m32} (float)</li>
     *   <li>{@code m33} (float)</li>
     *   <li>{@code m34} (float)</li>
     *   <li>{@code m41} (float)</li>
     *   <li>{@code m42} (float)</li>
     *   <li>{@code m43} (float)</li>
     *   <li>{@code m44} (float)</li>
     * </ol>
     *
     * <p>projectionFloating carries CNA_Matrix in this order:
     * <ol start="0">
     *   <li>{@code m11} (float)</li>
     *   <li>{@code m12} (float)</li>
     *   <li>{@code m13} (float)</li>
     *   <li>{@code m14} (float)</li>
     *   <li>{@code m21} (float)</li>
     *   <li>{@code m22} (float)</li>
     *   <li>{@code m23} (float)</li>
     *   <li>{@code m24} (float)</li>
     *   <li>{@code m31} (float)</li>
     *   <li>{@code m32} (float)</li>
     *   <li>{@code m33} (float)</li>
     *   <li>{@code m34} (float)</li>
     *   <li>{@code m41} (float)</li>
     *   <li>{@code m42} (float)</li>
     *   <li>{@code m43} (float)</li>
     *   <li>{@code m44} (float)</li>
     * </ol>
     */
    public static native int debugDrawBegin(long debug, float[] viewFloating, float[] projectionFloating);

    /**
     * cna_debug_draw_clear (graphics_ext.h).
     */
    public static native int debugDrawClear(long debug);

    /**
     * cna_debug_draw_copy_vertices (graphics_ext.h).
     */
    public static native int debugDrawCopyVertices(long debug, boolean depthTested, long[] destinationIntegral, float[] destinationFloating, long[] outCount);

    /**
     * cna_debug_draw_create (graphics_ext.h).
     */
    public static native int debugDrawCreate(long graphicsDevice, long[] outDebug);

    /**
     * cna_debug_draw_destroy (graphics_ext.h).
     */
    public static native int debugDrawDestroy(long debug);

    /**
     * cna_debug_draw_end (graphics_ext.h).
     */
    public static native int debugDrawEnd(long debug);

    /**
     * cna_debug_draw_get_line_count (graphics_ext.h).
     */
    public static native int debugDrawGetLineCount(long debug, int[] outCount);

    /**
     * cna_debug_draw_is_depth_tested (graphics_ext.h).
     */
    public static native int debugDrawIsDepthTested(long debug, boolean[] outDepthTested);

    /**
     * cna_debug_draw_set_depth_tested (graphics_ext.h).
     */
    public static native int debugDrawSetDepthTested(long debug, boolean depthTested);

    /**
     * cna_depth_effect_create (graphics_ext.h).
     */
    public static native int depthEffectCreate(long graphicsDevice, long[] outEffect);

    /**
     * cna_depth_effect_get_dither_mode (graphics_ext.h).
     */
    public static native int depthEffectGetDitherMode(long effect, int[] outDitherMode);

    /**
     * cna_depth_effect_get_mode (graphics_ext.h).
     */
    public static native int depthEffectGetMode(long effect, int[] outMode);

    /**
     * cna_depth_effect_set_dither_mode (graphics_ext.h).
     */
    public static native int depthEffectSetDitherMode(long effect, int ditherMode);

    /**
     * cna_depth_effect_set_mode (graphics_ext.h).
     */
    public static native int depthEffectSetMode(long effect, int mode);

    /**
     * cna_graphics_device_copy_capability_report_ext (graphics.h).
     */
    public static native int graphicsDeviceCopyCapabilityReportExt(long graphicsDevice, byte[] destination, long[] outBytes);

    /**
     * cna_graphics_device_copy_renderer_name (graphics.h).
     */
    public static native int graphicsDeviceCopyRendererName(long graphicsDevice, byte[] destination, long[] outBytes);

    /**
     * cna_graphics_device_get_capability_report_size_ext (graphics.h).
     */
    public static native int graphicsDeviceGetCapabilityReportSizeExt(long graphicsDevice, long[] outBytes);

    /**
     * cna_graphics_device_get_renderer_name_size (graphics.h).
     */
    public static native int graphicsDeviceGetRendererNameSize(long graphicsDevice, long[] outBytes);

    /**
     * cna_graphics_device_get_shader_dialect_ext (graphics.h).
     */
    public static native int graphicsDeviceGetShaderDialectExt(long graphicsDevice, int[] outDialect);

    /**
     * cna_graphics_device_supports_capability (graphics.h).
     */
    public static native int graphicsDeviceSupportsCapability(long graphicsDevice, int capability, boolean[] outSupported);

    /**
     * cna_graphics_ext_is_available (graphics_ext.h).
     */
    public static native int graphicsExtIsAvailable(boolean[] outAvailable);

    /**
     * cna_image_based_light_ext_init (graphics_ext.h).
     *
     * <p>outLightIntegral carries CNA_ImageBasedLightEXT in this order:
     * <ol start="0">
     *   <li>{@code irradiance} (CNA_Handle)</li>
     *   <li>{@code prefiltered_specular} (CNA_Handle)</li>
     *   <li>{@code brdf_lut} (CNA_Handle)</li>
     *   <li>{@code prefiltered_mip_count} (int32_t)</li>
     * </ol>
     *
     * <p>outLightFloating carries CNA_ImageBasedLightEXT in this order:
     * <ol start="0">
     *   <li>{@code intensity} (float)</li>
     * </ol>
     */
    public static native int imageBasedLightExtInit(long[] outLightIntegral, float[] outLightFloating);

    /**
     * cna_image_based_light_ext_is_valid (graphics_ext.h).
     *
     * <p>lightIntegral carries CNA_ImageBasedLightEXT in this order:
     * <ol start="0">
     *   <li>{@code irradiance} (CNA_Handle)</li>
     *   <li>{@code prefiltered_specular} (CNA_Handle)</li>
     *   <li>{@code brdf_lut} (CNA_Handle)</li>
     *   <li>{@code prefiltered_mip_count} (int32_t)</li>
     * </ol>
     *
     * <p>lightFloating carries CNA_ImageBasedLightEXT in this order:
     * <ol start="0">
     *   <li>{@code intensity} (float)</li>
     * </ol>
     */
    public static native int imageBasedLightExtIsValid(long[] lightIntegral, float[] lightFloating, boolean[] outValid);

    /**
     * cna_indirect_draw_arguments_init (graphics_ext.h).
     *
     * <p>outArgumentsIntegral carries CNA_IndirectDrawArguments in this order:
     * <ol start="0">
     *   <li>{@code vertex_count} (uint32_t)</li>
     *   <li>{@code instance_count} (uint32_t)</li>
     *   <li>{@code first_vertex} (uint32_t)</li>
     *   <li>{@code base_instance} (uint32_t)</li>
     * </ol>
     */
    public static native int indirectDrawArgumentsInit(long[] outArgumentsIntegral);

    /**
     * cna_indirect_draw_indexed_arguments_init (graphics_ext.h).
     *
     * <p>outArgumentsIntegral carries CNA_IndirectDrawIndexedArguments in this order:
     * <ol start="0">
     *   <li>{@code index_count} (uint32_t)</li>
     *   <li>{@code instance_count} (uint32_t)</li>
     *   <li>{@code first_index} (uint32_t)</li>
     *   <li>{@code base_vertex} (int32_t)</li>
     *   <li>{@code base_instance} (uint32_t)</li>
     * </ol>
     */
    public static native int indirectDrawIndexedArgumentsInit(long[] outArgumentsIntegral);
}
