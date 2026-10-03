package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativeShaderEffectRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativeShaderEffectRoutes {

    private NativeShaderEffectRoutes() {
    }

    /**
     * cna_effect_copy_fragment_source (effects.h).
     */
    public static native int effectCopyFragmentSource(long effect, byte[] destination, long[] outByteCount);

    /**
     * cna_effect_copy_vertex_source (effects.h).
     */
    public static native int effectCopyVertexSource(long effect, byte[] destination, long[] outByteCount);

    /**
     * cna_effect_get_fragment_source_byte_count (effects.h).
     */
    public static native int effectGetFragmentSourceByteCount(long effect, long[] outByteCount);

    /**
     * cna_effect_get_is_compiled_ext (effects.h).
     */
    public static native int effectGetIsCompiledExt(long effect, boolean[] outIsCompiled);

    /**
     * cna_effect_get_vertex_source_byte_count (effects.h).
     */
    public static native int effectGetVertexSourceByteCount(long effect, long[] outByteCount);

    /**
     * cna_effect_has_renderer (effects.h).
     */
    public static native int effectHasRenderer(long effect, boolean[] outHasRenderer);

    /**
     * cna_shader_effect_copy_compile_error_ext (effects.h).
     */
    public static native int shaderEffectCopyCompileErrorExt(long effect, byte[] destination, long[] outBytes);

    /**
     * cna_shader_effect_create (effects.h).
     */
    public static native int shaderEffectCreate(long graphicsDevice, byte[] vertexSource, byte[] fragmentSource, long[] outEffect);

    /**
     * cna_shader_effect_declare_uniform_block_ext (effects.h).
     */
    public static native int shaderEffectDeclareUniformBlockExt(long effect, int blockSizeBytes, byte[][] names, int[] offsets);

    /**
     * cna_shader_effect_get_projection (effects.h).
     *
     * <p>outValueFloating carries CNA_Matrix in this order:
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
    public static native int shaderEffectGetProjection(long effect, float[] outValueFloating);

    /**
     * cna_shader_effect_get_view (effects.h).
     *
     * <p>outValueFloating carries CNA_Matrix in this order:
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
    public static native int shaderEffectGetView(long effect, float[] outValueFloating);

    /**
     * cna_shader_effect_get_world (effects.h).
     *
     * <p>outValueFloating carries CNA_Matrix in this order:
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
    public static native int shaderEffectGetWorld(long effect, float[] outValueFloating);

    /**
     * cna_shader_effect_has_renderer (effects.h).
     */
    public static native int shaderEffectHasRenderer(long effect, boolean[] outHasRenderer);

    /**
     * cna_shader_effect_is_valid (effects.h).
     */
    public static native int shaderEffectIsValid(long effect, boolean[] outIsValid);

    /**
     * cna_shader_effect_set_projection (effects.h).
     */
    public static native int shaderEffectSetProjection(long effect, float[] valueFloating);

    /**
     * cna_shader_effect_set_texture2d (effects.h).
     */
    public static native int shaderEffectSetTexture2d(long effect, int unit, long texture);

    /**
     * cna_shader_effect_set_texture3d (effects.h).
     */
    public static native int shaderEffectSetTexture3d(long effect, int unit, long texture);

    /**
     * cna_shader_effect_set_texture_cube (effects.h).
     */
    public static native int shaderEffectSetTextureCube(long effect, int unit, long texture);

    /**
     * cna_shader_effect_set_uniform_float (effects.h).
     */
    public static native int shaderEffectSetUniformFloat(long effect, byte[] name, float value);

    /**
     * cna_shader_effect_set_uniform_float_array (effects.h).
     */
    public static native int shaderEffectSetUniformFloatArray(long effect, byte[] name, float[] values);

    /**
     * cna_shader_effect_set_uniform_int32 (effects.h).
     */
    public static native int shaderEffectSetUniformInt32(long effect, byte[] name, int value);

    /**
     * cna_shader_effect_set_uniform_mat4_array (effects.h).
     */
    public static native int shaderEffectSetUniformMat4Array(long effect, byte[] name, float[] matrices);

    /**
     * cna_shader_effect_set_uniform_matrix (effects.h).
     */
    public static native int shaderEffectSetUniformMatrix(long effect, byte[] name, float[] valueFloating);

    /**
     * cna_shader_effect_set_uniform_vec3_array (effects.h).
     */
    public static native int shaderEffectSetUniformVec3Array(long effect, byte[] name, float[] values);

    /**
     * cna_shader_effect_set_uniform_vector2 (effects.h).
     */
    public static native int shaderEffectSetUniformVector2(long effect, byte[] name, float[] valueFloating);

    /**
     * cna_shader_effect_set_uniform_vector2_array (effects.h).
     */
    public static native int shaderEffectSetUniformVector2Array(long effect, byte[] name, float[] valuesFloating);

    /**
     * cna_shader_effect_set_uniform_vector3 (effects.h).
     */
    public static native int shaderEffectSetUniformVector3(long effect, byte[] name, float[] valueFloating);

    /**
     * cna_shader_effect_set_uniform_vector4 (effects.h).
     */
    public static native int shaderEffectSetUniformVector4(long effect, byte[] name, float[] valueFloating);

    /**
     * cna_shader_effect_set_view (effects.h).
     */
    public static native int shaderEffectSetView(long effect, float[] valueFloating);

    /**
     * cna_shader_effect_set_world (effects.h).
     */
    public static native int shaderEffectSetWorld(long effect, float[] valueFloating);
}
