package org.openeggbert.cna.extensions.input;

/**
 * Raised when an input capability exists in the ABI but not in this build or on this host.
 *
 * <p>Kept distinct from an ordinary failure so a game can fall back -- to key snapshots, to the
 * default cursor -- without swallowing a real error.
 */
public class InputNotSupportedException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public InputNotSupportedException(String message) {
        super(message);
    }
}
