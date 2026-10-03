package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Graphics.Texture2D;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

import java.util.Objects;

/**
 * A mouse cursor: one of the host's stock shapes, an image of the game's own, or none.
 *
 * <p>A CNA extension: XNA 4.0 can only show or hide the cursor through
 * {@code Game.IsMouseVisible}.
 *
 * <p>A stock cursor is a borrowed view of a process-lifetime object, so closing one is a no-op
 * and never frees the shared cursor. A cursor built from a texture is owned and is freed when it
 * is closed.
 */
public final class MouseCursor implements AutoCloseable {

    private final long handle;
    private final boolean owned;
    private boolean closed;

    private MouseCursor(long handle, boolean owned) {
        this.handle = handle;
        this.owned = owned;
    }

    /** Returns the host's stock cursor for this shape. */
    public static MouseCursor FromStock(MouseCursorStock stock) {
        Objects.requireNonNull(stock, "stock");
        long[] cursor = new long[1];
        InputExtension.check("MouseCursor.FromStock",
                NativeInputExtensionRoutes.mouseCursorGetStockExt(
                        InputExtension.game("MouseCursor"), stock.ordinal(), cursor));
        return new MouseCursor(cursor[0], false);
    }

    /** Creates the empty cursor, which hides the pointer without hiding the system cursor. */
    public static MouseCursor CreateEmpty() {
        InputExtension.game("MouseCursor");
        long[] cursor = new long[1];
        InputExtension.check("MouseCursor.CreateEmpty",
                NativeInputExtensionRoutes.mouseCursorCreateExt(cursor));
        return new MouseCursor(cursor[0], true);
    }

    /** Creates a cursor from a texture, with the hotspot at the given pixel. */
    public static MouseCursor FromTexture(Texture2D texture, int originX, int originY) {
        Objects.requireNonNull(texture, "texture");
        long[] cursor = new long[1];
        InputExtension.check("MouseCursor.FromTexture",
                NativeInputExtensionRoutes.mouseCursorCreateFromTexture2d(
                        InputExtension.game("MouseCursor"),
                        NativeBindings.nativeResourceHandle(texture),
                        originX, originY, cursor));
        return new MouseCursor(cursor[0], true);
    }

    /** Makes this cursor the active one. */
    public void Apply() {
        requireOpen();
        InputExtension.check("MouseCursor.Apply",
                NativeInputExtensionRoutes.mouseSetCursorExt(
                        InputExtension.game("MouseCursor"), handle));
    }

    /**
     * Releases the cursor.
     *
     * <p>A stock cursor is a borrowed view of a process-lifetime object, so this is a no-op for
     * one. Closing twice is a no-op either way.
     */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        if (owned) {
            InputExtension.check("MouseCursor.close",
                    NativeInputExtensionRoutes.mouseCursorDestroy(handle));
        } else {
            InputExtension.check("MouseCursor.close",
                    NativeInputExtensionRoutes.mouseCursorDispose(handle));
        }
    }

    private void requireOpen() {
        if (closed) {
            throw new IllegalStateException("This MouseCursor is closed");
        }
    }
}
