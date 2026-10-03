package org.openeggbert.cna.extensions.graphics;

/**
 * Raised when a CNA extension route exists in the ABI but not in this build.
 *
 * <p>CNA declares its extension surface in every build so the exported ABI never changes shape,
 * and answers {@code CNA_RESULT_NOT_SUPPORTED} where the implementation is compiled out. This
 * exception is that answer, kept distinct from an ordinary failure so a game can fall back
 * without swallowing a real error.
 */
public class ExtensionNotSupportedException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    public ExtensionNotSupportedException(String message) {
        super(message);
    }
}
