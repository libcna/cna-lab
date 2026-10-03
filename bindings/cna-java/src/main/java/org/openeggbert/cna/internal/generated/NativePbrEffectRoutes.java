package org.openeggbert.cna.internal.generated;

/**
 * Generated CNA C ABI declarations for NativePbrEffectRoutes.
 *
 * <p>Produced by {@code tools/native-abi/generate_jni.py} from the live CNA C headers.
 * Do not edit: every signature here is the header's own declaration, and regenerating
 * is how a change upstream reaches Java. This class is not application API.
 */
public final class NativePbrEffectRoutes {

    private NativePbrEffectRoutes() {
    }

    /**
     * cna_pbr_effect_create (effects.h).
     */
    public static native int pbrEffectCreate(long graphicsDevice, long[] outEffect);

    /**
     * cna_pbr_effect_get_alpha (effects.h).
     */
    public static native int pbrEffectGetAlpha(long effect, float[] outValue);

    /**
     * cna_pbr_effect_get_alpha_cutoff_ext (effects.h).
     */
    public static native int pbrEffectGetAlphaCutoffExt(long effect, float[] outValue);

    /**
     * cna_pbr_effect_get_alpha_mode_ext (effects.h).
     */
    public static native int pbrEffectGetAlphaModeExt(long effect, int[] outValue);

    /**
     * cna_pbr_effect_get_diffuse_color (effects.h).
     *
     * <p>outValueFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int pbrEffectGetDiffuseColor(long effect, float[] outValueFloating);

    /**
     * cna_pbr_effect_get_double_sided_ext (effects.h).
     */
    public static native int pbrEffectGetDoubleSidedExt(long effect, boolean[] outValue);

    /**
     * cna_pbr_effect_get_emissive_factor (effects.h).
     *
     * <p>outValueFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int pbrEffectGetEmissiveFactor(long effect, float[] outValueFloating);

    /**
     * cna_pbr_effect_get_encode_output_to_srgb_ext (effects.h).
     */
    public static native int pbrEffectGetEncodeOutputToSrgbExt(long effect, boolean[] outValue);

    /**
     * cna_pbr_effect_get_ior_ext (effects.h).
     */
    public static native int pbrEffectGetIorExt(long effect, float[] outValue);

    /**
     * cna_pbr_effect_get_metallic_factor (effects.h).
     */
    public static native int pbrEffectGetMetallicFactor(long effect, float[] outValue);

    /**
     * cna_pbr_effect_get_normal_scale_ext (effects.h).
     */
    public static native int pbrEffectGetNormalScaleExt(long effect, float[] outValue);

    /**
     * cna_pbr_effect_get_occlusion_strength_ext (effects.h).
     */
    public static native int pbrEffectGetOcclusionStrengthExt(long effect, float[] outValue);

    /**
     * cna_pbr_effect_get_roughness_factor (effects.h).
     */
    public static native int pbrEffectGetRoughnessFactor(long effect, float[] outValue);

    /**
     * cna_pbr_effect_get_specular_color_factor_ext (effects.h).
     *
     * <p>outValueFloating carries CNA_Vector3 in this order:
     * <ol start="0">
     *   <li>{@code x} (float)</li>
     *   <li>{@code y} (float)</li>
     *   <li>{@code z} (float)</li>
     * </ol>
     */
    public static native int pbrEffectGetSpecularColorFactorExt(long effect, float[] outValueFloating);

    /**
     * cna_pbr_effect_get_specular_factor_ext (effects.h).
     */
    public static native int pbrEffectGetSpecularFactorExt(long effect, float[] outValue);

    /**
     * cna_pbr_effect_get_texture (effects.h).
     */
    public static native int pbrEffectGetTexture(long effect, int slot, boolean[] outHasTexture, long[] outTexture);

    /**
     * cna_pbr_effect_get_texture_coordinate_set_ext (effects.h).
     */
    public static native int pbrEffectGetTextureCoordinateSetExt(long effect, int slot, int[] outValue);

    /**
     * cna_pbr_effect_get_texture_is_srgb_ext (effects.h).
     */
    public static native int pbrEffectGetTextureIsSrgbExt(long effect, int slot, boolean[] outValue);

    /**
     * cna_pbr_effect_get_texture_transform_ext (effects.h).
     *
     * <p>outTransformFloating carries CNA_TextureTransformEXT in this order:
     * <ol start="0">
     *   <li>{@code offset.x} (float)</li>
     *   <li>{@code offset.y} (float)</li>
     *   <li>{@code scale.x} (float)</li>
     *   <li>{@code scale.y} (float)</li>
     *   <li>{@code rotation} (float)</li>
     * </ol>
     */
    public static native int pbrEffectGetTextureTransformExt(long effect, int slot, float[] outTransformFloating);

    /**
     * cna_pbr_effect_get_vertex_color_enabled_ext (effects.h).
     */
    public static native int pbrEffectGetVertexColorEnabledExt(long effect, boolean[] outEnabled);

    /**
     * cna_pbr_effect_set_alpha (effects.h).
     */
    public static native int pbrEffectSetAlpha(long effect, float value);

    /**
     * cna_pbr_effect_set_alpha_cutoff_ext (effects.h).
     */
    public static native int pbrEffectSetAlphaCutoffExt(long effect, float value);

    /**
     * cna_pbr_effect_set_alpha_mode_ext (effects.h).
     */
    public static native int pbrEffectSetAlphaModeExt(long effect, int value);

    /**
     * cna_pbr_effect_set_diffuse_color (effects.h).
     */
    public static native int pbrEffectSetDiffuseColor(long effect, float[] valueFloating);

    /**
     * cna_pbr_effect_set_double_sided_ext (effects.h).
     */
    public static native int pbrEffectSetDoubleSidedExt(long effect, boolean value);

    /**
     * cna_pbr_effect_set_emissive_factor (effects.h).
     */
    public static native int pbrEffectSetEmissiveFactor(long effect, float[] valueFloating);

    /**
     * cna_pbr_effect_set_encode_output_to_srgb_ext (effects.h).
     */
    public static native int pbrEffectSetEncodeOutputToSrgbExt(long effect, boolean value);

    /**
     * cna_pbr_effect_set_ior_ext (effects.h).
     */
    public static native int pbrEffectSetIorExt(long effect, float value);

    /**
     * cna_pbr_effect_set_metallic_factor (effects.h).
     */
    public static native int pbrEffectSetMetallicFactor(long effect, float value);

    /**
     * cna_pbr_effect_set_normal_scale_ext (effects.h).
     */
    public static native int pbrEffectSetNormalScaleExt(long effect, float value);

    /**
     * cna_pbr_effect_set_occlusion_strength_ext (effects.h).
     */
    public static native int pbrEffectSetOcclusionStrengthExt(long effect, float value);

    /**
     * cna_pbr_effect_set_roughness_factor (effects.h).
     */
    public static native int pbrEffectSetRoughnessFactor(long effect, float value);

    /**
     * cna_pbr_effect_set_specular_color_factor_ext (effects.h).
     */
    public static native int pbrEffectSetSpecularColorFactorExt(long effect, float[] valueFloating);

    /**
     * cna_pbr_effect_set_specular_factor_ext (effects.h).
     */
    public static native int pbrEffectSetSpecularFactorExt(long effect, float value);

    /**
     * cna_pbr_effect_set_texture (effects.h).
     */
    public static native int pbrEffectSetTexture(long effect, int slot, long texture);

    /**
     * cna_pbr_effect_set_texture_coordinate_set_ext (effects.h).
     */
    public static native int pbrEffectSetTextureCoordinateSetExt(long effect, int slot, int value);

    /**
     * cna_pbr_effect_set_texture_is_srgb_ext (effects.h).
     */
    public static native int pbrEffectSetTextureIsSrgbExt(long effect, int slot, boolean value);

    /**
     * cna_pbr_effect_set_texture_transform_ext (effects.h).
     *
     * <p>transformFloating carries CNA_TextureTransformEXT in this order:
     * <ol start="0">
     *   <li>{@code offset.x} (float)</li>
     *   <li>{@code offset.y} (float)</li>
     *   <li>{@code scale.x} (float)</li>
     *   <li>{@code scale.y} (float)</li>
     *   <li>{@code rotation} (float)</li>
     * </ol>
     */
    public static native int pbrEffectSetTextureTransformExt(long effect, int slot, float[] transformFloating);

    /**
     * cna_pbr_effect_set_vertex_color_enabled_ext (effects.h).
     */
    public static native int pbrEffectSetVertexColorEnabledExt(long effect, boolean enabled);

    /**
     * cna_skinned_pbr_effect_copy_bone_transforms (effects.h).
     */
    public static native int skinnedPbrEffectCopyBoneTransforms(long effect, long requestedCount, float[] destinationFloating, long[] outCount);

    /**
     * cna_skinned_pbr_effect_create (effects.h).
     */
    public static native int skinnedPbrEffectCreate(long graphicsDevice, long[] outEffect);

    /**
     * cna_skinned_pbr_effect_get_weights_per_vertex (effects.h).
     */
    public static native int skinnedPbrEffectGetWeightsPerVertex(long effect, int[] outValue);

    /**
     * cna_skinned_pbr_effect_set_bone_transforms (effects.h).
     */
    public static native int skinnedPbrEffectSetBoneTransforms(long effect, float[] transformsFloating);

    /**
     * cna_skinned_pbr_effect_set_weights_per_vertex (effects.h).
     */
    public static native int skinnedPbrEffectSetWeightsPerVertex(long effect, int value);
}
