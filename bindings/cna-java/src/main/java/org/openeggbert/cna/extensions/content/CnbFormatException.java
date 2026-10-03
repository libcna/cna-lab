package org.openeggbert.cna.extensions.content;

/**
 * Thrown when a {@code .cnb} file is not one this build can read.
 *
 * <p>Its own type because the answer is about the file, not about the call: the magic is wrong,
 * a length does not add up, a mandatory chunk is not understood, a limit was exceeded, or the
 * asset is not the one the caller required. All of those are things a game can report to a
 * player or a tool can report to an artist.
 */
public final class CnbFormatException extends RuntimeException {

    private static final long serialVersionUID = 1L;

    CnbFormatException(String message) {
        super(message);
    }
}
