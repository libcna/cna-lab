package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * An icon in the host's notification area, with a menu a person can pick from.
 *
 * <p>A CNA extension: XNA 4.0 has no counterpart. The icon is real host state, so a tray that is
 * never closed stays on the desktop after the game stops drawing.
 *
 * <p>The handle is <strong>owned</strong>; {@link #close()} releases it and removes the icon.
 * Each entry's click handler is pinned for as long as the tray lives -- a person may pick an
 * entry at any moment -- and closing the tray releases every one of them.
 *
 * <p>A click handler runs on <strong>whatever thread the host delivers the activation on</strong>,
 * which is not necessarily the game thread. A handler that touches game state should hand the
 * work to the game rather than doing it where it lands.
 */
public final class SystemTray implements AutoCloseable {

    private final long handle;
    private final List<Long> tokens = new ArrayList<>();
    private boolean closed;

    private SystemTray(long handle) {
        this.handle = handle;
    }

    /** Reports whether this platform has a notification area at all. */
    public static boolean getIsSupported() {
        boolean[] supported = new boolean[1];
        DeviceExtension.check("SystemTray.getIsSupported",
                NativeDeviceExtensionRoutes.systemTrayGetIsSupportedExt(
                        DeviceExtension.game("SystemTray"), supported));
        return supported[0];
    }

    /**
     * Puts a real icon in the host's notification area.
     *
     * @param tooltip the text shown when the pointer rests on the icon
     * @return the new tray, which the caller owns and must close
     */
    public static SystemTray Create(String tooltip) {
        Objects.requireNonNull(tooltip, "tooltip");
        long[] tray = new long[1];
        DeviceExtension.check("SystemTray.Create",
                NativeDeviceExtensionRoutes.systemTrayCreate(
                        DeviceExtension.game("SystemTray"), NativeGamerServices.utf8(tooltip),
                        tray));
        return new SystemTray(tray[0]);
    }

    /**
     * Creates a tray backed by CNA's own test backend, which reaches no desktop.
     *
     * <p>CNA takes the backend as a construction argument rather than through a switch, so this
     * is a second factory rather than a mode flag -- and that is why it is here rather than in
     * {@link DeviceTestBackends}. Entries added to such a tray can be activated with
     * {@link #clickEntryForTests(int)}, which is the only way to exercise a click handler on a
     * machine nobody is sitting at.
     *
     * @param tooltip the tooltip the backend records
     * @return the new tray, which the caller owns and must close
     */
    public static SystemTray CreateForTests(String tooltip) {
        Objects.requireNonNull(tooltip, "tooltip");
        long[] tray = new long[1];
        DeviceExtension.check("SystemTray.CreateForTests",
                NativeDeviceExtensionRoutes.systemTrayCreateWithTestBackendExt(
                        DeviceExtension.game("SystemTray"), NativeGamerServices.utf8(tooltip),
                        tray));
        return new SystemTray(tray[0]);
    }

    /** Changes the text shown when the pointer rests on the icon. */
    public void setTooltip(String tooltip) {
        Objects.requireNonNull(tooltip, "tooltip");
        DeviceExtension.check("SystemTray.setTooltip",
                NativeDeviceExtensionRoutes.systemTraySetTooltip(
                        open(), NativeGamerServices.utf8(tooltip)));
    }

    /**
     * Adds an entry to the tray's menu.
     *
     * @param label the entry's label
     * @param checkable whether the entry carries a check mark
     * @param initiallyChecked whether a checkable entry starts checked
     * @param initiallyEnabled whether the entry starts enabled
     * @param onClick what to run when a person picks it, or {@code null} for an entry that does
     *        nothing on its own
     * @return the entry's zero-based index, which every other entry operation takes
     */
    public int addEntry(String label, boolean checkable, boolean initiallyChecked,
            boolean initiallyEnabled, Runnable onClick) {
        Objects.requireNonNull(label, "label");
        long token = onClick == null ? 0L : NativeBindings.newCallbackToken(onClick);
        long[] index = new long[1];
        int result = NativeBindings.systemTrayAddEntry(open(), NativeGamerServices.utf8(label),
                checkable, initiallyChecked, initiallyEnabled, token, index);
        if (result != 0) {
            // The entry was not added, so nothing will ever call the handler and the token has
            // no other owner. Released here rather than kept, which is the difference between
            // a refused call and a leak per refusal.
            NativeBindings.releaseCallbackToken(token);
            DeviceExtension.check("SystemTray.addEntry", result);
        }
        if (token != 0L) {
            tokens.add(token);
        }
        return (int) index[0];
    }

    /**
     * Changes one entry's label.
     *
     * <p>An index past the last entry is <strong>ignored</strong> rather than refused. That is
     * the host backend's own behaviour and it is reported rather than tightened here, because a
     * projection that refused it would disagree with CNA about what happened.
     */
    public void setEntryLabel(int index, String label) {
        Objects.requireNonNull(label, "label");
        DeviceExtension.check("SystemTray.setEntryLabel",
                NativeDeviceExtensionRoutes.systemTraySetEntryLabel(
                        open(), index, NativeGamerServices.utf8(label)));
    }

    /** Sets one entry's check mark. An index past the last entry is ignored. */
    public void setEntryChecked(int index, boolean checked) {
        DeviceExtension.check("SystemTray.setEntryChecked",
                NativeDeviceExtensionRoutes.systemTraySetEntryChecked(open(), index, checked));
    }

    /** Reports whether one entry is checked. An index past the last entry answers false. */
    public boolean getEntryChecked(int index) {
        boolean[] checked = new boolean[1];
        DeviceExtension.check("SystemTray.getEntryChecked",
                NativeDeviceExtensionRoutes.systemTrayGetEntryChecked(open(), index, checked));
        return checked[0];
    }

    /** Enables or disables one entry. An index past the last entry is ignored. */
    public void setEntryEnabled(int index, boolean enabled) {
        DeviceExtension.check("SystemTray.setEntryEnabled",
                NativeDeviceExtensionRoutes.systemTraySetEntryEnabled(open(), index, enabled));
    }

    /** Reports whether one entry is enabled. An index past the last entry answers false. */
    public boolean getEntryEnabled(int index) {
        boolean[] enabled = new boolean[1];
        DeviceExtension.check("SystemTray.getEntryEnabled",
                NativeDeviceExtensionRoutes.systemTrayGetEntryEnabled(open(), index, enabled));
        return enabled[0];
    }

    /**
     * Activates one entry as though a person had picked it.
     *
     * <p>Only a tray from {@link #CreateForTests(String)} can do this; a real tray answers
     * {@code INVALID_STATE}, because activating a host menu entry from inside the process is
     * not something the host offers.
     *
     * @param index the entry to activate
     */
    public void clickEntryForTests(int index) {
        DeviceExtension.check("SystemTray.clickEntryForTests",
                NativeDeviceExtensionRoutes.systemTrayClickEntryForTestsExt(open(), index));
    }

    /** Removes the icon and releases the handle and every pinned click handler. */
    @Override
    public void close() {
        if (closed) {
            return;
        }
        closed = true;
        try {
            DeviceExtension.check("SystemTray.close",
                    NativeDeviceExtensionRoutes.systemTrayDestroy(handle));
        } finally {
            // After the tray is gone nothing can deliver a click, so every handler's reference
            // is released -- including when destroy itself failed, because a tray that refused
            // to close is not a reason to keep a handler alive that can no longer run.
            for (long token : tokens) {
                NativeBindings.releaseCallbackToken(token);
            }
            tokens.clear();
        }
    }

    private long open() {
        if (closed) {
            throw new IllegalStateException("this SystemTray is closed");
        }
        return handle;
    }
}
