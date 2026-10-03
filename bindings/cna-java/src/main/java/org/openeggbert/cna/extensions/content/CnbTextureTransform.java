package org.openeggbert.cna.extensions.content;

/**
 * How one texture slot's coordinates are transformed before sampling.
 *
 * <p>glTF's {@code KHR_texture_transform}: the coordinates are scaled, rotated and offset, in
 * that order.
 *
 * @param OffsetX the horizontal offset
 * @param OffsetY the vertical offset
 * @param ScaleX the horizontal scale
 * @param ScaleY the vertical scale
 * @param Rotation the rotation in radians
 */
public record CnbTextureTransform(
        float OffsetX, float OffsetY, float ScaleX, float ScaleY, float Rotation) {

    /** Returns the transform that changes nothing: no offset, unit scale, no rotation. */
    public static CnbTextureTransform identity() {
        return new CnbTextureTransform(0f, 0f, 1f, 1f, 0f);
    }
}
