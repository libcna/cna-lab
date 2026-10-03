package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeEffectExtensionRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeEffectExtensionRoutes {

    private NativeEffectExtensionRoutes() {
    }

    /**
     * cna_color_matrix_effect_create (effects.h).
     */
    public static native int colorMatrixEffectCreate(long graphicsDevice, long[] outEffect);

    /**
     * cna_color_matrix_effect_get_matrix (effects.h).
     *
     * <p>outValueFloating carries CNA_ColorMatrix4x4 in this order:
     * <ol start="0">
     *   <li>{@code values[0]} (float)</li>
     *   <li>{@code values[1]} (float)</li>
     *   <li>{@code values[2]} (float)</li>
     *   <li>{@code values[3]} (float)</li>
     *   <li>{@code values[4]} (float)</li>
     *   <li>{@code values[5]} (float)</li>
     *   <li>{@code values[6]} (float)</li>
     *   <li>{@code values[7]} (float)</li>
     *   <li>{@code values[8]} (float)</li>
     *   <li>{@code values[9]} (float)</li>
     *   <li>{@code values[10]} (float)</li>
     *   <li>{@code values[11]} (float)</li>
     *   <li>{@code values[12]} (float)</li>
     *   <li>{@code values[13]} (float)</li>
     *   <li>{@code values[14]} (float)</li>
     *   <li>{@code values[15]} (float)</li>
     * </ol>
     */
    public static native int colorMatrixEffectGetMatrix(long effect, float[] outValueFloating);

    /**
     * cna_color_matrix_effect_get_offset (effects.h).
     *
     * <p>outValueFloating carries CNA_Vector4 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     *   <li>{@code w} (float)</li>
     * </ol>
     */
    public static native int colorMatrixEffectGetOffset(long effect, float[] outValueFloating);

    /**
     * cna_color_matrix_effect_reset (effects.h).
     */
    public static native int colorMatrixEffectReset(long effect);

    /**
     * cna_color_matrix_effect_set_grayscale (effects.h).
     */
    public static native int colorMatrixEffectSetGrayscale(long effect);

    /**
     * cna_color_matrix_effect_set_matrix (effects.h).
     */
    public static native int colorMatrixEffectSetMatrix(long effect, float[] valueFloating);

    /**
     * cna_color_matrix_effect_set_offset (effects.h).
     */
    public static native int colorMatrixEffectSetOffset(long effect, float[] valueFloating);
}
