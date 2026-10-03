package org.openeggbert.cna.extensions.content;

/**
 * The shape of a texture a {@code .cnb} file carries.
 *
 * @param Width the widest level's width in pixels
 * @param Height the widest level's height in pixels
 * @param Depth the widest level's depth, one for a two-dimensional texture
 * @param FaceCount how many faces there are, six for a cube texture and one otherwise
 * @param MipCount how many mip levels each representation has
 * @param RepresentationCount how many format-specific copies of the same image the file carries
 */
public record CnbTextureInfo(
        int Width, int Height, int Depth, int FaceCount, int MipCount, int RepresentationCount) {
}
