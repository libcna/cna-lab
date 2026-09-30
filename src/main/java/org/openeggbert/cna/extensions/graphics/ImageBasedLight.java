package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.TextureCube;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeGraphicsExtensionRoutes;

/**
 * The three textures a PBR shader needs to light a surface from an environment.
 *
 * <p>A CNA extension: an irradiance cube for the diffuse half, a prefiltered specular cube whose
 * mip levels are roughness levels, and the BRDF lookup that combines them. {@link #intensity()}
 * scales the lot. CNA ABI 0.30 removed the environment processor that produced these and the
 * effect routes that consumed them; {@code graphics_ext.h} still declares the structure, its
 * initializer and its validity check, so it is kept as a value.
 *
 * <p><strong>The textures are borrowed and retained here.</strong> CNA's structure records them
 * and never owns them, so this holds Java references to keep them alive and disposes none of
 * them.
 *
 * @param irradiance the irradiance cube, or {@code null}
 * @param prefilteredSpecular the prefiltered specular cube, or {@code null}
 * @param brdfLut the BRDF lookup texture, or {@code null}
 * @param prefilteredMipCount how many mip levels the prefiltered cube has; at least one
 * @param intensity the scalar multiplier on the light
 */
public record ImageBasedLight(TextureCube irradiance, TextureCube prefilteredSpecular,
        Texture2D brdfLut, int prefilteredMipCount, float intensity) {

    /**
     * Returns the light CNA itself defaults to, which names no textures.
     *
     * @return the default light
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public static ImageBasedLight createDefault() {
        GraphicsExtension.requireBackend();
        long[] integral = new long[4];
        float[] floating = new float[1];
        GraphicsExtension.check("ImageBasedLight.createDefault",
                NativeGraphicsExtensionRoutes.imageBasedLightExtInit(integral, floating));
        return new ImageBasedLight(null, null, null, Math.toIntExact(integral[3]), floating[0]);
    }

    /**
     * Reports whether an effect would accept this light.
     *
     * <p>CNA's own answer: a light missing one of its three textures, or claiming fewer than one
     * mip level, cannot be sampled.
     *
     * @return whether the light is usable
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public boolean isValid() {
        GraphicsExtension.requireBackend();
        boolean[] valid = new boolean[1];
        GraphicsExtension.check("ImageBasedLight.isValid",
                NativeGraphicsExtensionRoutes.imageBasedLightExtIsValid(integral(), floating(),
                        valid));
        return valid[0];
    }

    /** The integral leaves CNA's structure declares, in declaration order. */
    long[] integral() {
        return new long[] {
            irradiance == null ? 0L : NativeBindings.nativeResourceHandle(irradiance),
            prefilteredSpecular == null ? 0L
                    : NativeBindings.nativeResourceHandle(prefilteredSpecular),
            brdfLut == null ? 0L : NativeBindings.nativeResourceHandle(brdfLut),
            prefilteredMipCount,
        };
    }

    /** The floating leaves CNA's structure declares, in declaration order. */
    float[] floating() {
        return new float[] {intensity};
    }
}
