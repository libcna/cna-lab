package org.openeggbert.cna.extensions.content;

/**
 * Which effect a compiled part expects to be drawn with.
 *
 * <p>The numbers are wire format. {@link #External} is the one that means "not one of these":
 * the part names an effect asset of its own, which {@link CnbModelData#getPartExternalEffect}
 * reads.
 */
public enum CnbEffectKind {

    /** XNA's BasicEffect. */
    Basic,

    /** A skinned variant of the basic effect. */
    Skinned,

    /** Two texture layers. */
    DualTexture,

    /** Physically based rendering. */
    Pbr,

    /** A skinned variant of the physically based effect. */
    SkinnedPbr,

    /** An effect asset the part names itself. */
    External;

    static CnbEffectKind fromValue(long value) {
        CnbEffectKind[] values = values();
        if (value < 0 || value >= values.length) {
            throw new CnbFormatException("the file names effect kind " + value
                    + ", which this build has no constant for");
        }
        return values[(int) value];
    }
}
