package org.openeggbert.cna.extensions.graphics;

import Microsoft.Xna.Framework.Vector2;

/**
 * How one of a material's textures is placed on the surface.
 *
 * <p>A CNA extension, and glTF's {@code KHR_texture_transform}: the same texture atlas page used at
 * two scales, or a scrolling detail map, without a second copy of the texture. XNA has no
 * counterpart -- a texture is sampled at the coordinates the vertices carry and that is all.
 *
 * <p>The identity transform is offset zero, scale one, rotation zero, and {@link #identity()}
 * is it.
 *
 * @param offset added to the texture coordinates
 * @param scale multiplied into the texture coordinates
 * @param rotation in radians, applied about the origin
 */
public record TextureTransform(Vector2 offset, Vector2 scale, float rotation) {

    /** How many floats one transform occupies in CNA's flattened material. */
    static final int LEAVES = 5;

    /**
     * Returns the transform that changes nothing.
     *
     * @return the identity transform
     */
    public static TextureTransform identity() {
        return new TextureTransform(new Vector2(0f, 0f), new Vector2(1f, 1f), 0f);
    }

    /** Reads one transform out of flat leaves at a given offset. */
    static TextureTransform fromLeaves(float[] leaves, int base) {
        return new TextureTransform(new Vector2(leaves[base], leaves[base + 1]),
                new Vector2(leaves[base + 2], leaves[base + 3]), leaves[base + 4]);
    }

    /** Writes this transform into flat leaves at a given offset. */
    void writeTo(float[] leaves, int base) {
        leaves[base] = offset.X;
        leaves[base + 1] = offset.Y;
        leaves[base + 2] = scale.X;
        leaves[base + 3] = scale.Y;
        leaves[base + 4] = rotation;
    }

    /** The five floats CNA's structure declares, in declaration order. */
    float[] floating() {
        float[] leaves = new float[LEAVES];
        writeTo(leaves, 0);
        return leaves;
    }
}
