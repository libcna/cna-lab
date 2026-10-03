package org.openeggbert.cna.extensions.graphics;

/**
 * How a material's alpha is interpreted.
 *
 * <p>A CNA extension, and glTF's own three-way distinction: XNA has a blend state and no notion of
 * a material declaring what its alpha means, so a mask and a blend look the same to it and are
 * drawn the same wrong way.
 */
public enum AlphaMode {

    /** The output is fully opaque and any alpha is ignored. */
    Opaque,
    /** Alpha is compared against the cutoff, and the fragment is kept or discarded. */
    Mask,
    /** Alpha blends the fragment with what is already there. */
    Blend;

    /**
     * Returns the mode CNA's identity names.
     *
     * @param value the identity
     * @return the mode
     * @throws IllegalArgumentException for a value outside the three
     */
    public static AlphaMode fromValue(long value) {
        AlphaMode[] modes = values();
        if (value < 0 || value >= modes.length) {
            throw new IllegalArgumentException("Unknown alpha mode " + value);
        }
        return modes[(int) value];
    }
}
