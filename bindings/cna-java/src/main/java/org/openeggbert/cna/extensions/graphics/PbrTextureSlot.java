package org.openeggbert.cna.extensions.graphics;

/**
 * Which map of a physically-based material a texture is.
 *
 * <p>A CNA extension: XNA's effects have one texture and no notion of a material split across
 * several maps. The order is CNA's own, and it is what {@code texture_coordinate_sets} and
 * {@code texture_transforms} are indexed by, so it is an enum rather than a set of names.
 */
public enum PbrTextureSlot {

    /** The base colour, sometimes called albedo. */
    BaseColor,
    /** The tangent-space normal map. */
    Normal,
    /** glTF's combined metallic-roughness map. */
    MetallicRoughness,
    /** The emissive map. */
    Emissive,
    /** The ambient-occlusion map. */
    Occlusion,
    /** {@code KHR_materials_specular}'s scalar strength map, sampled from the alpha channel. */
    Specular,
    /** {@code KHR_materials_specular}'s colour map, sRGB encoded by default. */
    SpecularColor;

    /** How many slots a material has, which is CNA's own {@code CNA_PBR_TEXTURE_SLOT_COUNT}. */
    public static final int COUNT = 7;

    /**
     * Returns the slot CNA's identity names.
     *
     * @param value the identity
     * @return the slot
     * @throws IllegalArgumentException for a value outside the seven
     */
    public static PbrTextureSlot fromValue(long value) {
        PbrTextureSlot[] slots = values();
        if (value < 0 || value >= slots.length) {
            throw new IllegalArgumentException("Unknown PBR texture slot " + value);
        }
        return slots[(int) value];
    }
}
