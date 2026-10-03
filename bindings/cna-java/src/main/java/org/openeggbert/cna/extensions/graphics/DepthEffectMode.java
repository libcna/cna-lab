package org.openeggbert.cna.extensions.graphics;

/**
 * The colour depth the depth-reduction effect targets.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. The constant names are CNA's own.
 */
public enum DepthEffectMode {
    Color16Bit,
    Color8Bit,
    Grayscale4Bit,
    Grayscale2Bit,
    Grayscale1Bit,
    Palette256,
    Palette16;
}
