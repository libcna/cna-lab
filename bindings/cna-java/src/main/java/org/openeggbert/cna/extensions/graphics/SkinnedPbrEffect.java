package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Matrix;
import org.openeggbert.cna.internal.generated.NativePbrEffectRoutes;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * The physically-based effect for animated geometry.
 *
 * <p>Everything {@link PbrEffect} is, plus the bone transforms a skinned mesh is deformed by --
 * the PBR counterpart of XNA's {@code SkinnedEffect}, which has the bones but not the material
 * model.
 *
 * <p>CNA carries at most {@value #MAX_BONES} bones, which is the shader's own constant-register
 * budget rather than a choice made here.
 */
public final class SkinnedPbrEffect extends PbrEffect {

    /** The greatest number of bone transforms this effect carries. */
    public static final int MAX_BONES = 72;

    /**
     * Creates the effect on one device.
     *
     * @param graphicsDevice the device to compile on
     * @throws ExtensionNotSupportedException when this build has no extended graphics layer
     */
    public SkinnedPbrEffect(GraphicsDevice graphicsDevice) {
        super(graphicsDevice, true);
    }

    /**
     * Returns how many bone weights each vertex carries.
     *
     * @return one, two or four
     */
    public int getWeightsPerVertex() {
        int[] value = new int[1];
        GraphicsExtension.check("SkinnedPbrEffect.getWeightsPerVertex",
                NativePbrEffectRoutes.skinnedPbrEffectGetWeightsPerVertex(handle(), value));
        return value[0];
    }

    /**
     * Sets how many bone weights each vertex carries.
     *
     * @param value one, two or four; anything else is refused
     */
    public void setWeightsPerVertex(int value) {
        GraphicsExtension.check("SkinnedPbrEffect.setWeightsPerVertex",
                NativePbrEffectRoutes.skinnedPbrEffectSetWeightsPerVertex(handle(), value));
    }

    /**
     * Replaces the leading bone transforms.
     *
     * <p>Leading rather than all: a mesh with fewer bones than the budget sets only those, and
     * the rest keep whatever they had. CNA copies the array, so the list is not retained.
     *
     * @param transforms between one and {@value #MAX_BONES} transforms
     */
    public void setBoneTransforms(List<Matrix> transforms) {
        Objects.requireNonNull(transforms, "transforms");
        GraphicsExtension.check("SkinnedPbrEffect.setBoneTransforms",
                NativePbrEffectRoutes.skinnedPbrEffectSetBoneTransforms(handle(),
                        EngineValues.matrices(transforms, "transforms")));
    }

    /**
     * Copies the leading bone transforms back out.
     *
     * @param count how many to read, between one and {@value #MAX_BONES}
     * @return the transforms
     */
    public List<Matrix> getBoneTransforms(int count) {
        float[] leaves = new float[Math.multiplyExact(Math.max(count, 0),
                EngineValues.MATRIX_LEAVES)];
        long[] written = new long[1];
        GraphicsExtension.check("SkinnedPbrEffect.getBoneTransforms",
                NativePbrEffectRoutes.skinnedPbrEffectCopyBoneTransforms(handle(), count, leaves,
                        written));
        List<Matrix> transforms = new ArrayList<>(count);
        for (int index = 0; index < Math.toIntExact(written[0]); index++) {
            transforms.add(EngineValues.matrix(leaves, index));
        }
        return List.copyOf(transforms);
    }

}
