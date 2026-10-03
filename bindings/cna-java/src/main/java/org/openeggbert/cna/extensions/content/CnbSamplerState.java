package org.openeggbert.cna.extensions.content;

/**
 * How one texture slot is sampled.
 *
 * <p>{@link #Declared()} is the field that matters most: a compiled material can leave a slot's
 * sampler unstated, meaning the renderer picks, and that is different from a slot that explicitly
 * asks for point filtering. A sampler read back with {@code Declared} false carries no decision,
 * and its other three fields say nothing.
 *
 * @param Filter CNA's own texture filter identifier
 * @param AddressU CNA's own addressing mode for the horizontal axis
 * @param AddressV CNA's own addressing mode for the vertical axis
 * @param Declared whether the file states a sampler for this slot at all
 */
public record CnbSamplerState(int Filter, int AddressU, int AddressV, boolean Declared) {

    /** Returns a sampler that states nothing, leaving the choice to the renderer. */
    public static CnbSamplerState undeclared() {
        return new CnbSamplerState(0, 0, 0, false);
    }
}
