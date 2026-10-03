package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.Objects;

/**
 * The host clipboard.
 *
 * <p>A CNA extension: XNA 4.0 has no clipboard at all. The clipboard is process-external state
 * another application can change at any moment, so a write reports that the request was made,
 * not that the clipboard now holds the text -- and a read is a snapshot that may already be
 * stale by the time the caller looks at it.
 */
public final class Clipboard {

    /** CNA's own {@code CNA_RESULT_BUFFER_TOO_SMALL}, which a growing clipboard produces. */
    private static final int RESULT_BUFFER_TOO_SMALL = 14;

    private Clipboard() {
    }

    /**
     * Asks the host to put this text on the clipboard.
     *
     * @return whether the host accepted the request
     */
    public static boolean SetText(String text) {
        Objects.requireNonNull(text, "text");
        boolean[] accepted = new boolean[1];
        DeviceExtension.check("Clipboard.SetText",
                NativeDeviceExtensionRoutes.devicesClipboardSetTextExt(
                        DeviceExtension.game("Clipboard"),
                        NativeGamerServices.utf8(text), accepted));
        return accepted[0];
    }

    /**
     * Reports whether the clipboard currently holds non-empty text.
     *
     * <p>Cheaper than reading it, and the answer another application can invalidate a moment
     * later: this is a snapshot, not a lock.
     *
     * @return whether the clipboard holds non-empty text
     */
    public static boolean getHasText() {
        boolean[] present = new boolean[1];
        DeviceExtension.check("Clipboard.getHasText",
                NativeDeviceExtensionRoutes.clipboardGetHasText(
                        DeviceExtension.game("Clipboard"), present));
        return present[0];
    }

    /**
     * Returns the clipboard's current text, empty when it holds none.
     *
     * <p>Two calls, as CNA's count-then-copy pair requires. Between them another application may
     * replace the contents with something longer, which CNA answers by reporting the size it
     * needs rather than by writing part of it -- so this asks again rather than truncating.
     *
     * @return the text, never null
     */
    public static String GetText() {
        String owner = "Clipboard.GetText";
        long game = DeviceExtension.game("Clipboard");
        long[] bytes = new long[1];
        DeviceExtension.check(owner,
                NativeDeviceExtensionRoutes.clipboardGetTextSize(game, bytes));
        for (int attempt = 0; attempt < 4; attempt++) {
            byte[] destination = new byte[(int) bytes[0]];
            int result = NativeDeviceExtensionRoutes.clipboardCopyText(game, destination, bytes);
            if (result == RESULT_BUFFER_TOO_SMALL) {
                // CNA always writes the size it needs, even when it refuses, so the retry is
                // over a buffer that is right rather than over a guess.
                continue;
            }
            DeviceExtension.check(owner, result);
            return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
        }
        throw new IllegalStateException("the clipboard changed under every attempt to read it");
    }
}
