package org.openeggbert.cna.extensions.input;

/**
 * What is physically printed on a game pad's face button, in CNA's own identity order.
 *
 * <p>A CNA extension, and one that matters for a prompt: XNA names the four face buttons A, B, X
 * and Y because that is what an Xbox pad has. On a PlayStation pad the button in the A position
 * is marked with a cross, and telling the player to "press A" is then wrong. The XNA name stays
 * the identity; this is the label to draw.
 */
public enum GamePadButtonLabel {

    /** The host does not know what is printed on the button. */
    Unknown,

    /** Marked A. */
    A,

    /** Marked B. */
    B,

    /** Marked X. */
    X,

    /** Marked Y. */
    Y,

    /** Marked with a cross. */
    Cross,

    /** Marked with a circle. */
    Circle,

    /** Marked with a square. */
    Square,

    /** Marked with a triangle. */
    Triangle
}
