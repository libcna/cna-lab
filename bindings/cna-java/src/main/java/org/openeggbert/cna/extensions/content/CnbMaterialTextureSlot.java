package org.openeggbert.cna.extensions.content;

/**
 * One of the eight named texture slots a compiled material has.
 *
 * <p>The numbers are wire format. The names are glTF's, because that is the vocabulary the
 * material's factors use as well, and calling a metallic-roughness map "texture 3" would lose the
 * one thing a tool needs to know about it.
 */
public enum CnbMaterialTextureSlot {

    /** The albedo or diffuse map. */
    BaseColor,

    /** The second texture layer, used by the dual-texture effect. */
    Second,

    /** The tangent-space normal map. */
    Normal,

    /** Metalness in blue and roughness in green, as glTF packs them. */
    MetallicRoughness,

    /** The emissive map. */
    Emissive,

    /** The ambient occlusion map. */
    Occlusion,

    /** The specular strength map. */
    Specular,

    /** The specular colour map. */
    SpecularColor
}
