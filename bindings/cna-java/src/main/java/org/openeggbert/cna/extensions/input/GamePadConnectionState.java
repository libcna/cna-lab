package org.openeggbert.cna.extensions.input;

/**
 * How a game pad is attached, in CNA's own identity order.
 *
 * <p>A CNA extension. XNA reports only whether a pad is connected; this says how.
 */
public enum GamePadConnectionState {

    /** The host does not know how the pad is attached. */
    Unknown,

    /** The pad is attached by cable. */
    Wired,

    /** The pad is attached over a wireless link. */
    Wireless
}
