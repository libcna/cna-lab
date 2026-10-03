package org.openeggbert.cna.extensions.content;

/**
 * One of the seven slots a compiled material carries per-slot state for.
 *
 * <p><strong>Not the same thing as {@link CnbMaterialTextureSlot}, and CNA's own header calls the
 * difference "a real trap".</strong> There are eight texture <em>names</em>, because CNA's effect
 * slots include {@code DualTextureEffect}'s second layer, which glTF has no counterpart for. The
 * coordinate sets, transforms and samplers are seven-element arrays in the importer's own order,
 * which is a different order as well as a different length.
 *
 * <p>In C both are integers and confusing them silently addresses the wrong slot. Here they are
 * two types, so the mistake does not compile.
 */
public enum CnbImporterTextureSlot {

    /** The base-colour slot. */
    BaseColor,

    /** The normal-map slot. */
    Normal,

    /** The metallic-roughness slot. */
    MetallicRoughness,

    /** The occlusion slot. */
    Occlusion,

    /** The emissive slot. */
    Emissive,

    /** The specular-strength slot. */
    Specular,

    /** The specular-colour slot. */
    SpecularColor
}
