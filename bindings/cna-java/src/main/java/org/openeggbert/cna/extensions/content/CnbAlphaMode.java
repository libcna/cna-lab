package org.openeggbert.cna.extensions.content;

/**
 * How a compiled material's alpha is treated.
 *
 * <p>glTF's three modes, and the numbers are wire format.
 */
public enum CnbAlphaMode {

    /** Alpha is ignored and the surface is drawn opaque. */
    Opaque,

    /** Alpha is compared against the material's cutoff and the fragment is kept or dropped. */
    Mask,

    /** Alpha blends the surface with what is behind it. */
    Blend;

    static CnbAlphaMode fromValue(long value) {
        CnbAlphaMode[] values = values();
        if (value < 0 || value >= values.length) {
            throw new CnbFormatException("the file names alpha mode " + value
                    + ", which this build has no constant for");
        }
        return values[(int) value];
    }
}
