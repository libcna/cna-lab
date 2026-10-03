package org.openeggbert.cna.extensions.content;

/**
 * Runs one body against the {@code .cnb} extension layer.
 *
 * <p>That layer is pure functions and self-owned handles over caller bytes, so unlike every other
 * extension here it needs no running game: a tool can read and write a file with no window, no
 * device and no frame. This exists so a test says that rather than opening a game it does not
 * need.
 */
final class CnbExtensionProbe {

    private CnbExtensionProbe() {
    }

    static void run(Runnable body) {
        body.run();
    }
}
