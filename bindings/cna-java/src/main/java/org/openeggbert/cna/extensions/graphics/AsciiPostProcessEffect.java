package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Rectangle;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

import java.util.Objects;

/**
 * Redraws a rendered frame as a grid of ASCII glyphs.
 *
 * <p>A CNA extension with no XNA 4.0 counterpart. The effect owns a native object, so it is
 * closed explicitly; closing it twice is a no-op, as it is for every CNA-owned resource in this
 * binding.
 */
public final class AsciiPostProcessEffect implements AutoCloseable {

    private final long handle;
    private boolean closed;

    /**
     * Creates the effect for one graphics device.
     *
     * @throws ExtensionNotSupportedException when this CNA build has no extended graphics layer
     */
    public AsciiPostProcessEffect(GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        GraphicsExtension.requireBackend();
        long[] effect = new long[1];
        GraphicsExtension.check("AsciiPostProcessEffect",
                NativeGraphicsExtensionRoutes.asciiPostProcessEffectCreate(
                        NativeBindings.nativeDeviceGameHandle(graphicsDevice), effect));
        handle = effect[0];
    }

    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        GraphicsExtension.check("AsciiPostProcessEffect.close",
                NativeGraphicsExtensionRoutes.asciiPostProcessEffectDestroy(handle));
    }

    /** Draws one source texture as ASCII into a destination rectangle. */
    public void Draw(Texture2D source, Rectangle destinationRectangle) {
        requireOpen();
        Objects.requireNonNull(source, "source");
        Objects.requireNonNull(destinationRectangle, "destinationRectangle");
        GraphicsExtension.check("AsciiPostProcessEffect.Draw",
                NativeGraphicsExtensionRoutes.asciiPostProcessEffectDraw(handle,
                        NativeBindings.nativeResourceHandle(source),
                        new long[] {
                            destinationRectangle.X, destinationRectangle.Y,
                            destinationRectangle.Width, destinationRectangle.Height,
                        }));
    }

    /** Returns the glyph cell size in pixels, as width and height. */
    public int[] getCellSize() {
        requireOpen();
        int[] width = new int[1];
        int[] height = new int[1];
        GraphicsExtension.check("AsciiPostProcessEffect.CellSize",
                NativeGraphicsExtensionRoutes.asciiPostProcessEffectGetCellSize(
                        handle, width, height));
        return new int[] {width[0], height[0]};
    }

    public void setCellSize(int width, int height) {
        requireOpen();
        GraphicsExtension.check("AsciiPostProcessEffect.CellSize",
                NativeGraphicsExtensionRoutes.asciiPostProcessEffectSetCellSize(
                        handle, width, height));
    }

    /** Returns the columns and rows of the last drawn grid, as columns and rows. */
    public int[] getLastGridDimensions() {
        requireOpen();
        int[] columns = new int[1];
        int[] rows = new int[1];
        GraphicsExtension.check("AsciiPostProcessEffect.LastGridDimensions",
                NativeGraphicsExtensionRoutes.asciiPostProcessEffectGetLastGridDimensions(
                        handle, columns, rows));
        return new int[] {columns[0], rows[0]};
    }

    public AsciiQuantizeMode getQuantizeMode() {
        requireOpen();
        int[] mode = new int[1];
        GraphicsExtension.check("AsciiPostProcessEffect.QuantizeMode",
                NativeGraphicsExtensionRoutes.asciiPostProcessEffectGetQuantizeMode(handle, mode));
        return AsciiQuantizeMode.values()[mode[0]];
    }

    public void setQuantizeMode(AsciiQuantizeMode value) {
        requireOpen();
        GraphicsExtension.check("AsciiPostProcessEffect.QuantizeMode",
                NativeGraphicsExtensionRoutes.asciiPostProcessEffectSetQuantizeMode(
                        handle, Objects.requireNonNull(value, "value").ordinal()));
    }

    private void requireOpen() {
        if (closed) {
            throw new IllegalStateException("This AsciiPostProcessEffect is closed");
        }
    }
}
