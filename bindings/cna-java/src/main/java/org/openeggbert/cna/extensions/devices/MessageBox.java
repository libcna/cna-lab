package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.util.List;
import java.util.Objects;

/**
 * A host message box, either dismiss-only or answering with one of several buttons.
 *
 * <p>A CNA extension: XNA 4.0 has {@code Guide.BeginShowMessageBox} on Xbox and Windows Phone and
 * nothing at all on Windows. This is the host's own dialog, on whatever platform the game runs.
 *
 * <p><strong>Showing one blocks the calling thread until a person answers.</strong> That is the
 * host's behaviour and this class does not soften it. A game that wants a message box in a test
 * installs {@link DeviceTestBackends#installMessageBoxBackend(int)}, which answers immediately
 * and records what it was asked to show.
 */
public final class MessageBox {

    private MessageBox() {
    }

    /** Reports whether this platform can show a message box at all. */
    public static boolean getIsSupported() {
        boolean[] supported = new boolean[1];
        DeviceExtension.check("MessageBox.getIsSupported",
                NativeDeviceExtensionRoutes.messageBoxGetIsSupportedExt(
                        DeviceExtension.game("MessageBox"), supported));
        return supported[0];
    }

    /**
     * Shows a message box the reader can only dismiss, and waits for them to do it.
     *
     * @param type the severity
     * @param title the dialog title
     * @param message the dialog body
     */
    public static void Show(MessageBoxType type, String title, String message) {
        Objects.requireNonNull(type, "type");
        Objects.requireNonNull(title, "title");
        Objects.requireNonNull(message, "message");
        DeviceExtension.check("MessageBox.Show",
                NativeDeviceExtensionRoutes.messageBoxShowSimpleExt(
                        DeviceExtension.game("MessageBox"), type.ordinal(),
                        NativeGamerServices.utf8(title), NativeGamerServices.utf8(message)));
    }

    /**
     * Shows a message box with buttons and waits for the reader to pick one.
     *
     * @param type the severity
     * @param title the dialog title
     * @param message the dialog body
     * @param buttonLabels the button labels, left to right; must not be empty
     * @return the zero-based index of the button chosen, or -1 when the dialog was closed
     *         without choosing one
     */
    public static int Show(MessageBoxType type, String title, String message,
            List<String> buttonLabels) {
        Objects.requireNonNull(type, "type");
        Objects.requireNonNull(title, "title");
        Objects.requireNonNull(message, "message");
        Objects.requireNonNull(buttonLabels, "buttonLabels");
        if (buttonLabels.isEmpty()) {
            throw new IllegalArgumentException("a message box with buttons needs at least one");
        }
        byte[][] labels = new byte[buttonLabels.size()][];
        for (int index = 0; index < labels.length; index++) {
            labels[index] = NativeGamerServices.utf8(
                    Objects.requireNonNull(buttonLabels.get(index), "buttonLabel"));
        }
        int[] chosen = new int[1];
        DeviceExtension.check("MessageBox.Show",
                NativeDeviceExtensionRoutes.messageBoxShowExt(
                        DeviceExtension.game("MessageBox"), type.ordinal(),
                        NativeGamerServices.utf8(title), NativeGamerServices.utf8(message),
                        labels, chosen));
        return chosen[0];
    }
}
