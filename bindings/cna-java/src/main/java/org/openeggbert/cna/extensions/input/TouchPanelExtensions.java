package org.openeggbert.cna.extensions.input;

import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

/**
 * Touch behaviour CNA offers and XNA's {@code TouchPanel} does not.
 *
 * <p>A CNA extension. XNA's {@code TouchPanelCapabilities.IsConnected} answers whether touch is
 * usable; these two say something different, and both matter to a desktop developer building for
 * a touch device: whether real touch hardware exists at all, and whether the host is turning
 * mouse input into touch events so a touch path can be exercised without it.
 */
public final class TouchPanelExtensions {

    private TouchPanelExtensions() {
    }

    /**
     * Reports whether the host has real touch hardware.
     *
     * <p>This is not {@code TouchPanel.GetCapabilities().IsConnected}: a host emulating touch
     * from the mouse reports touch as usable there while answering false here.
     */
    public static boolean getTouchDeviceExists() {
        boolean[] exists = new boolean[1];
        check("getTouchDeviceExists", NativeInputExtensionRoutes
                .touchPanelGetTouchDeviceExistsExt(game(), exists));
        return exists[0];
    }

    /** Reports whether the host is turning mouse input into touch events. */
    public static boolean getMouseTouchEmulationEnabled() {
        boolean[] enabled = new boolean[1];
        check("getMouseTouchEmulationEnabled", NativeInputExtensionRoutes
                .touchPanelGetMouseTouchEmulationEnabledExt(game(), enabled));
        return enabled[0];
    }

    /**
     * Turns mouse-driven touch emulation on or off.
     *
     * @param enabled whether the mouse should produce touch events
     */
    public static void setMouseTouchEmulationEnabled(boolean enabled) {
        check("setMouseTouchEmulationEnabled", NativeInputExtensionRoutes
                .touchPanelSetMouseTouchEmulationEnabledExt(game(), enabled));
    }

    private static long game() {
        return InputExtension.game("TouchPanelExtensions");
    }

    private static void check(String operation, int result) {
        InputExtension.check("TouchPanelExtensions." + operation, result);
    }
}
