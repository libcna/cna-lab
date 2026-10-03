package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.function.Consumer;

/**
 * The host's own file and folder chooser.
 *
 * <p>A CNA extension: XNA 4.0 has {@code StorageDevice} and nothing that asks a person where a
 * file should go.
 *
 * <p><strong>Every one of these is asynchronous, and the handler is where the answer arrives.</strong>
 * The call returns as soon as the request has been made; the host answers through its own event
 * loop, which may be many frames later and may be never if the process exits first. That is
 * CNA's contract and it is reproduced rather than hidden behind a blocking wrapper -- a blocking
 * wrapper would have to pump the game's own event loop from inside a game callback.
 *
 * <p><strong>Cancelling is an empty list, not a separate signal.</strong> The canonical dialog
 * reports it that way and so does this: a handler that receives no paths was cancelled, and a
 * platform with no file dialogs answers the same way immediately.
 */
public final class FileDialog {

    private static final int OPEN_FILE = 0;
    private static final int SAVE_FILE = 1;
    private static final int OPEN_FOLDER = 2;

    private FileDialog() {
    }

    /** Reports whether this platform can show file dialogs at all. */
    public static boolean getIsSupported() {
        boolean[] supported = new boolean[1];
        DeviceExtension.check("FileDialog.getIsSupported",
                NativeDeviceExtensionRoutes.fileDialogGetIsSupportedExt(
                        DeviceExtension.game("FileDialog"), supported));
        return supported[0];
    }

    /**
     * Asks the host for one or more files to open.
     *
     * @param filters the file types offered, or an empty list for every file
     * @param defaultLocation the directory to start in, or an empty string for the host's default
     * @param allowMultiple whether more than one file may be chosen
     * @param onResult receives the chosen paths, or an empty list when the dialog was cancelled
     */
    public static void ShowOpenFile(List<FileDialogFilter> filters, String defaultLocation,
            boolean allowMultiple, Consumer<List<String>> onResult) {
        show(OPEN_FILE, filters, defaultLocation, allowMultiple, onResult, "ShowOpenFile");
    }

    /**
     * Asks the host where a file should be saved.
     *
     * @param filters the file types offered, or an empty list for every file
     * @param defaultLocation the directory to start in, or an empty string for the host's default
     * @param onResult receives the chosen path, or an empty list when the dialog was cancelled
     */
    public static void ShowSaveFile(List<FileDialogFilter> filters, String defaultLocation,
            Consumer<List<String>> onResult) {
        show(SAVE_FILE, filters, defaultLocation, false, onResult, "ShowSaveFile");
    }

    /**
     * Asks the host for one or more folders.
     *
     * <p>Folder dialogs take no filters. That is the canonical shape rather than an omission
     * here, which is why this overload has no filter parameter to ignore.
     *
     * @param defaultLocation the directory to start in, or an empty string for the host's default
     * @param allowMultiple whether more than one folder may be chosen
     * @param onResult receives the chosen paths, or an empty list when the dialog was cancelled
     */
    public static void ShowOpenFolder(String defaultLocation, boolean allowMultiple,
            Consumer<List<String>> onResult) {
        show(OPEN_FOLDER, List.of(), defaultLocation, allowMultiple, onResult, "ShowOpenFolder");
    }

    private static void show(int kind, List<FileDialogFilter> filters, String defaultLocation,
            boolean allowMultiple, Consumer<List<String>> onResult, String operation) {
        Objects.requireNonNull(filters, "filters");
        Objects.requireNonNull(defaultLocation, "defaultLocation");
        Objects.requireNonNull(onResult, "onResult");
        byte[][] names = new byte[filters.size()][];
        byte[][] patterns = new byte[filters.size()][];
        for (int index = 0; index < names.length; index++) {
            FileDialogFilter filter = Objects.requireNonNull(filters.get(index), "filter");
            names[index] = NativeGamerServices.utf8(filter.name());
            patterns[index] = NativeGamerServices.utf8(filter.pattern());
        }
        // The paths arrive as UTF-8 bytes rather than as strings, because the JVM's own
        // NewStringUTF takes modified UTF-8 and would corrupt anything past the basic
        // multilingual plane. Decoding here is what keeps a path with an emoji in it intact.
        Consumer<byte[][]> sink = paths -> {
            List<String> decoded = new ArrayList<>(paths.length);
            for (byte[] path : paths) {
                decoded.add(new String(path, StandardCharsets.UTF_8));
            }
            onResult.accept(Collections.unmodifiableList(decoded));
        };
        // The native trampoline owns this token and deletes it when the handler runs, which
        // happens exactly once; the adapter releases it itself when CNA refuses the request,
        // because then the handler never runs at all.
        long token = NativeBindings.newCallbackToken(sink);
        DeviceExtension.check("FileDialog." + operation,
                NativeBindings.fileDialogShow(DeviceExtension.game("FileDialog"), kind, token,
                        names, patterns, NativeGamerServices.utf8(defaultLocation),
                        allowMultiple));
    }
}
