package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

import java.util.Objects;

/**
 * CNA's colour-depth reduction, for the look of an older display.
 *
 * <p>A CNA extension with no XNA 4.0 counterpart. What CNA creates is a real XNA {@link Effect},
 * so {@link #getEffect()} hands it to {@code SpriteBatch.Begin} like any other; this type is the
 * typed set of knobs that effect answers to.
 */
public final class DepthEffect implements AutoCloseable {

    private final Effect effect;

    /**
     * Creates the effect for one graphics device.
     *
     * @throws ExtensionNotSupportedException when this CNA build has no extended graphics layer
     */
    public DepthEffect(GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        GraphicsExtension.requireBackend();
        if (!GraphicsExtension.isAvailable()) {
            throw new ExtensionNotSupportedException(
                    "DepthEffect is not available: this build has no extended graphics layer");
        }
        effect = FacadeFactory.createExtensionEffect(
                graphicsDevice, NativeBindings.EXTENSION_EFFECT_DEPTH);
    }

    /** Returns the XNA effect, for {@code SpriteBatch.Begin} and the ordinary effect surface. */
    public Effect getEffect() {
        return effect;
    }

    @Override
    public void close() {
        effect.Dispose();
    }

    public DepthEffectMode getMode() {
        int[] value = new int[1];
        GraphicsExtension.check("DepthEffect.Mode",
                NativeGraphicsExtensionRoutes.depthEffectGetMode(handle(), value));
        return DepthEffectMode.values()[value[0]];
    }

    public void setMode(DepthEffectMode value) {
        GraphicsExtension.check("DepthEffect.Mode",
                NativeGraphicsExtensionRoutes.depthEffectSetMode(
                        handle(), Objects.requireNonNull(value, "value").ordinal()));
    }

    public DitherMode getDitherMode() {
        int[] value = new int[1];
        GraphicsExtension.check("DepthEffect.DitherMode",
                NativeGraphicsExtensionRoutes.depthEffectGetDitherMode(handle(), value));
        return DitherMode.values()[value[0]];
    }

    public void setDitherMode(DitherMode value) {
        GraphicsExtension.check("DepthEffect.DitherMode",
                NativeGraphicsExtensionRoutes.depthEffectSetDitherMode(
                        handle(), Objects.requireNonNull(value, "value").ordinal()));
    }

    private long handle() {
        return NativeBindings.nativeResourceHandle(effect);
    }
}
