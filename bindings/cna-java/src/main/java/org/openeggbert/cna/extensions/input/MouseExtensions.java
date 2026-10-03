package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Point;
import org.openeggbert.cna.internal.GamerEventPump;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.function.IntConsumer;

/**
 * Mouse behaviour XNA's {@code Mouse} has no way to ask for.
 *
 * <p>A CNA extension. XNA can read the pointer's position inside the window and move it there,
 * and that is all. It cannot capture the pointer so a drag keeps reporting once it leaves the
 * window, cannot switch to relative motion for a first-person camera, and cannot see the desktop
 * as a whole.
 *
 * <p><strong>Relative mode is the one a camera wants.</strong> With it on, the host stops
 * reporting where the pointer is and starts reporting how far it moved, so the pointer never
 * reaches the edge of the screen and stops.
 */
public final class MouseExtensions {

    private static final int KIND_MOUSE_CLICKED = 37;

    private static final List<IntConsumer> CLICKED = new CopyOnWriteArrayList<>();

    static {
        GamerEventPump.addInputHandler(MouseExtensions::dispatch);
    }

    private MouseExtensions() {
    }

    /** Reports whether the host is delivering relative motion rather than absolute positions. */
    public static boolean getIsRelativeMouseMode() {
        boolean[] enabled = new boolean[1];
        check("getIsRelativeMouseMode", NativeInputExtensionRoutes
                .mouseGetIsRelativeMouseModeExt(game(), enabled));
        return enabled[0];
    }

    /**
     * Turns relative motion on or off.
     *
     * @param enabled whether to report motion instead of position
     */
    public static void setIsRelativeMouseMode(boolean enabled) {
        check("setIsRelativeMouseMode", NativeInputExtensionRoutes
                .mouseSetIsRelativeMouseModeExt(game(), enabled));
    }

    /**
     * Captures the pointer, so motion keeps arriving after it leaves the window.
     *
     * @param enabled whether to capture
     * @return whether the host applied it; a host that cannot capture reports false rather than
     *     failing
     */
    public static boolean setCapture(boolean enabled) {
        boolean[] applied = new boolean[1];
        check("setCapture",
                NativeInputExtensionRoutes.mouseSetCaptureExt(game(), enabled, applied));
        return applied[0];
    }

    /** Returns where the pointer is on the desktop, not inside the window. */
    public static Point getGlobalPosition() {
        int[] x = new int[1];
        int[] y = new int[1];
        check("getGlobalPosition",
                NativeInputExtensionRoutes.mouseGetGlobalPositionExt(game(), x, y));
        return new Point(x[0], y[0]);
    }

    /**
     * Moves the pointer to a point on the desktop, not inside the window.
     *
     * @param position where to put it, in desktop coordinates
     * @return whether the host moved it; one that will not reports false rather than failing
     */
    public static boolean WarpGlobal(Point position) {
        Objects.requireNonNull(position, "position");
        boolean[] applied = new boolean[1];
        check("WarpGlobal", NativeInputExtensionRoutes
                .mouseWarpGlobalExt(game(), position.X, position.Y, applied));
        return applied[0];
    }

    /**
     * Adds a listener for a mouse click.
     *
     * <p>XNA has no click event: a game polls {@code Mouse.GetState} and works out for itself
     * that a button went down and came up. This is the host's own click, delivered on the game
     * thread during {@code FrameworkDispatcher.Update}.
     *
     * @param listener called with the button index the host reported
     */
    public static void addClickedListener(IntConsumer listener) {
        CLICKED.add(Objects.requireNonNull(listener, "listener"));
        NativeBindings.requireAvailable();
        GamerEventPump.ensureMouseClickedSubscribed();
    }

    /** Removes a click listener. */
    public static void removeClickedListener(IntConsumer listener) {
        CLICKED.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Raises the host's own click event for one button.
     *
     * <p>CNA's route, not a Java simulation: it invokes the same event the platform layer
     * invokes, so a game can exercise its click wiring with no pointer to click.
     *
     * @param button the button index to report
     */
    public static void RaiseClicked(int button) {
        check("RaiseClicked",
                NativeInputExtensionRoutes.mouseRaiseClickedExt(game(), button));
    }

    private static void dispatch(long kind, long session, long first, long second, long flag) {
        if ((int) kind != KIND_MOUSE_CLICKED) {
            return;
        }
        for (IntConsumer listener : CLICKED) {
            listener.accept((int) first);
        }
    }

    private static long game() {
        return InputExtension.game("MouseExtensions");
    }

    private static void check(String operation, int result) {
        InputExtension.check("MouseExtensions." + operation, result);
    }
}
