package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Vector4;
import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeEffectExtensionRoutes;

import java.util.Objects;

/**
 * A four-by-four colour transform applied to everything the effect draws, plus an RGBA offset.
 *
 * <p>A CNA extension. XNA 4.0's five stock effects can tint by multiplying a diffuse colour and
 * that is the whole of their colour control -- there is no way to say "desaturate this",
 * "swap red and blue" or "shift the whole image towards sepia" without writing a shader, and XNA's
 * shader route needs a Content Pipeline that no longer runs on most machines. This is that
 * capability as an ordinary effect.
 *
 * <p>The transform is <strong>row-major, and each row is one output channel's weights</strong>
 * over the four input channels, with the offset added afterwards:
 *
 * <pre>{@code
 * out[i] = matrix[i*4 + 0] * r + matrix[i*4 + 1] * g
 *        + matrix[i*4 + 2] * b + matrix[i*4 + 3] * a + offset[i]
 * }</pre>
 *
 * <p>That is measured rather than assumed: the transpose is a different transform, and
 * {@link #setGrayscale()} is what settles which one CNA means -- it fills the first three rows
 * identically with the Rec. 709 weights, which only makes sense read this way.
 *
 * <p><strong>The effect is an ordinary graphics resource.</strong> {@link #getEffect()} hands back
 * an XNA {@link Effect} that {@code SpriteBatch.Begin} takes, and the game owns it: closing this
 * releases it.
 *
 * <p><strong>Only a CPU rasterizer executes the transform, and CNA says so rather than leaving
 * it to chance.</strong> The matrix travels through the shared CPU {@code SpriteBatch} path, and
 * CNA's renderer interface documents that every other renderer "deliberately leaves this false
 * rather than pretending to execute it" -- so on the EasyGL family the effect is accepted, the
 * draw succeeds, and the pixels are the source's own. Nothing in CNA's capability set predicts
 * which, so a game that depends on the transform reaching a pixel should draw once and look,
 * exactly as this projection's own test does.
 *
 * <p>The state itself is exact on every renderer: what is set is what is read back, and
 * {@link #setGrayscale()} fills the same weights everywhere. It is only the drawing that a
 * renderer may decline.
 */
public final class ColorMatrixEffect implements AutoCloseable {

    /** How many floats CNA's row-major transform carries. */
    private static final int ELEMENTS = 16;

    private final Effect effect;
    private boolean closed;

    private ColorMatrixEffect(Effect effect) {
        this.effect = effect;
    }

    /**
     * Creates a colour-matrix effect on a device, starting at the identity transform.
     *
     * @param graphicsDevice the device to create it on
     * @return the effect, which the caller closes
     */
    public static ColorMatrixEffect create(GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        long[] created = new long[1];
        GraphicsExtension.check("ColorMatrixEffect.create",
                NativeEffectExtensionRoutes.colorMatrixEffectCreate(
                        NativeBindings.nativeGraphicsDeviceValue(graphicsDevice), created));
        return new ColorMatrixEffect(
                FacadeFactory.createOwnedEffect(graphicsDevice, created[0]));
    }

    /**
     * Returns the effect a draw takes.
     *
     * @return the effect; it is this object's, not a fresh view, and closing this closes it
     */
    public Effect getEffect() {
        open();
        return effect;
    }

    /**
     * Returns the current transform, sixteen floats in row-major order.
     *
     * @return a fresh array; changing it changes nothing until it is set back
     */
    public float[] getMatrix() {
        float[] values = new float[ELEMENTS];
        GraphicsExtension.check("ColorMatrixEffect.getMatrix",
                NativeEffectExtensionRoutes.colorMatrixEffectGetMatrix(handle(), values));
        return values;
    }

    /**
     * Sets the transform.
     *
     * @param values sixteen finite floats in row-major order
     * @throws IllegalArgumentException when the array is not sixteen long
     */
    public void setMatrix(float[] values) {
        Objects.requireNonNull(values, "values");
        if (values.length != ELEMENTS) {
            throw new IllegalArgumentException(
                    "a colour matrix is " + ELEMENTS + " row-major floats, not " + values.length);
        }
        GraphicsExtension.check("ColorMatrixEffect.setMatrix",
                NativeEffectExtensionRoutes.colorMatrixEffectSetMatrix(handle(), values.clone()));
    }

    /**
     * Returns the RGBA offset added after the transform.
     *
     * @return the offset
     */
    public Vector4 getOffset() {
        float[] values = new float[4];
        GraphicsExtension.check("ColorMatrixEffect.getOffset",
                NativeEffectExtensionRoutes.colorMatrixEffectGetOffset(handle(), values));
        return new Vector4(values[0], values[1], values[2], values[3]);
    }

    /**
     * Sets the RGBA offset added after the transform.
     *
     * @param value the offset; every component must be finite
     */
    public void setOffset(Vector4 value) {
        Objects.requireNonNull(value, "value");
        GraphicsExtension.check("ColorMatrixEffect.setOffset",
                NativeEffectExtensionRoutes.colorMatrixEffectSetOffset(handle(),
                        new float[] {value.X, value.Y, value.Z, value.W}));
    }

    /**
     * Selects the Rec. 709 greyscale transform and a zero offset.
     *
     * <p>Rec. 709 rather than a flat third each: the weights are 0.2126 red, 0.7152 green and
     * 0.0722 blue, which is what makes a grey conversion look like the original's brightness
     * rather than like a wash.
     */
    public void setGrayscale() {
        GraphicsExtension.check("ColorMatrixEffect.setGrayscale",
                NativeEffectExtensionRoutes.colorMatrixEffectSetGrayscale(handle()));
    }

    /** Restores the identity transform and a zero offset. */
    public void reset() {
        GraphicsExtension.check("ColorMatrixEffect.reset",
                NativeEffectExtensionRoutes.colorMatrixEffectReset(handle()));
    }

    /** Releases the effect. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        effect.Dispose();
    }

    private long handle() {
        open();
        return NativeBindings.nativeResourceHandle(effect);
    }

    private void open() {
        if (closed) {
            throw new IllegalStateException("this ColorMatrixEffect is closed");
        }
    }
}
