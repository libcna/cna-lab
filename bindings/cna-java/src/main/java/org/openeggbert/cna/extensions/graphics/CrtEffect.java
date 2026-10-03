package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

import java.util.Objects;

/**
 * CNA's CRT emulation: curvature, scanlines, a sub-pixel mask and a vignette.
 *
 * <p>A CNA extension with no XNA 4.0 counterpart. What CNA creates is a real XNA
 * {@link Effect}, so {@link #getEffect()} hands it to {@code SpriteBatch.Begin} like any other;
 * this type is the typed set of knobs that effect answers to.
 *
 * <p>The effect belongs to its graphics device's game, like every other graphics resource:
 * disposing the game disposes it, and closing this closes the effect.
 */
public final class CrtEffect implements AutoCloseable {

    private final Effect effect;

    /**
     * Creates the effect for one graphics device.
     *
     * @throws ExtensionNotSupportedException when this CNA build has no extended graphics layer
     */
    public CrtEffect(GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        GraphicsExtension.requireBackend();
        if (!GraphicsExtension.isAvailable()) {
            throw new ExtensionNotSupportedException(
                    "CrtEffect is not available: this build has no extended graphics layer");
        }
        effect = FacadeFactory.createExtensionEffect(
                graphicsDevice, NativeBindings.EXTENSION_EFFECT_CRT);
    }

    /** Returns the XNA effect, for {@code SpriteBatch.Begin} and the ordinary effect surface. */
    public Effect getEffect() {
        return effect;
    }

    @Override
    public void close() {
        effect.Dispose();
    }

    public float getCurvature() {
        float[] value = new float[1];
        GraphicsExtension.check("CrtEffect.Curvature",
                NativeGraphicsExtensionRoutes.crtEffectGetCurvature(handle(), value));
        return value[0];
    }

    public void setCurvature(float value) {
        GraphicsExtension.check("CrtEffect.Curvature",
                NativeGraphicsExtensionRoutes.crtEffectSetCurvature(handle(), value));
    }

    public float getScanlineIntensity() {
        float[] value = new float[1];
        GraphicsExtension.check("CrtEffect.ScanlineIntensity",
                NativeGraphicsExtensionRoutes.crtEffectGetScanlineIntensity(handle(), value));
        return value[0];
    }

    public void setScanlineIntensity(float value) {
        GraphicsExtension.check("CrtEffect.ScanlineIntensity",
                NativeGraphicsExtensionRoutes.crtEffectSetScanlineIntensity(handle(), value));
    }

    public float getVignetteIntensity() {
        float[] value = new float[1];
        GraphicsExtension.check("CrtEffect.VignetteIntensity",
                NativeGraphicsExtensionRoutes.crtEffectGetVignetteIntensity(handle(), value));
        return value[0];
    }

    public void setVignetteIntensity(float value) {
        GraphicsExtension.check("CrtEffect.VignetteIntensity",
                NativeGraphicsExtensionRoutes.crtEffectSetVignetteIntensity(handle(), value));
    }

    public float getMaskIntensity() {
        float[] value = new float[1];
        GraphicsExtension.check("CrtEffect.MaskIntensity",
                NativeGraphicsExtensionRoutes.crtEffectGetMaskIntensity(handle(), value));
        return value[0];
    }

    public void setMaskIntensity(float value) {
        GraphicsExtension.check("CrtEffect.MaskIntensity",
                NativeGraphicsExtensionRoutes.crtEffectSetMaskIntensity(handle(), value));
    }

    public CrtMaskType getMaskType() {
        int[] value = new int[1];
        GraphicsExtension.check("CrtEffect.MaskType",
                NativeGraphicsExtensionRoutes.crtEffectGetMaskType(handle(), value));
        return CrtMaskType.values()[value[0]];
    }

    public void setMaskType(CrtMaskType value) {
        GraphicsExtension.check("CrtEffect.MaskType",
                NativeGraphicsExtensionRoutes.crtEffectSetMaskType(
                        handle(), Objects.requireNonNull(value, "value").ordinal()));
    }

    private long handle() {
        return NativeBindings.nativeResourceHandle(effect);
    }
}
