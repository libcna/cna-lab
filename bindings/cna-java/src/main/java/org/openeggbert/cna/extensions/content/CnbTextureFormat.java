package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Graphics.SurfaceFormat;
import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

/**
 * How one texture representation's bytes are laid out.
 *
 * <p>The identities are CNA's own and are wire format. They overlap XNA's {@link SurfaceFormat}
 * without being it -- CNB carries formats XNA never had -- so the two are converted rather than
 * conflated, and a format XNA cannot express says so instead of guessing.
 */
public enum CnbTextureFormat {

    /** Not a format this build knows. */
    Unknown,

    /** Eight bits per channel, red first. */
    Rgba8,

    /** Eight bits per channel, blue first. */
    Bgra8,

    /** Eight bits per channel, red first, in the sRGB transfer function. */
    Rgba8Srgb,

    /** Five, six and five bits, blue first. */
    Bgr565,

    /** Five bits per colour channel and one of alpha, blue first. */
    Bgra5551,

    /** Four bits per channel, blue first. */
    Bgra4444,

    /** Eight bits of alpha only. */
    Alpha8,

    /** Eight bits of red only. */
    R8,

    /** Sixteen bits of red only. */
    R16,

    /** Sixteen bits each of red and green. */
    Rg16,

    /** Sixteen bits per channel. */
    Rgba16,

    /** Eight signed-normalised bits each of red and green. */
    Rg8Snorm,

    /** Eight signed-normalised bits per channel. */
    Rgba8Snorm,

    /** Ten bits per colour channel and two of alpha. */
    Rgb10A2,

    /** One 32-bit float of red. */
    R32Float,

    /** Two 32-bit floats. */
    Rg32Float,

    /** Four 32-bit floats. */
    Rgba32Float,

    /** One 16-bit float of red. */
    R16Float,

    /** Two 16-bit floats. */
    Rg16Float,

    /** Four 16-bit floats. */
    Rgba16Float,

    /** The blendable high-dynamic-range format. */
    HdrBlendable,

    /** Block compression 1. */
    Bc1,

    /** Block compression 2. */
    Bc2,

    /** Block compression 3. */
    Bc3,

    /** Block compression 3 in the sRGB transfer function. */
    Bc3Srgb,

    /** Block compression 7. */
    Bc7,

    /** Block compression 7 in the sRGB transfer function. */
    Bc7Srgb;

    /** Returns CNA's own name for the format. */
    public String getName() {
        CnbExtension.requireAvailable();
        return CnbExtension.text("CnbTextureFormat.getName",
                bytes -> NativeCnbRoutes.cnbGetTextureFormatNameSize(ordinal(), bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbCopyTextureFormatName(ordinal(), destination, bytes));
    }

    /**
     * Reports whether the format stores blocks rather than pixels.
     *
     * <p>It matters for every size calculation: a block-compressed level is measured in
     * four-by-four blocks, so its byte size is not width times height times a unit.
     */
    public boolean isBlockCompressed() {
        CnbExtension.requireAvailable();
        boolean[] blockCompressed = new boolean[1];
        CnbExtension.check("CnbTextureFormat.isBlockCompressed",
                NativeCnbRoutes.cnbIsBlockCompressedTextureFormat(ordinal(), blockCompressed));
        return blockCompressed[0];
    }

    /** Returns how many bytes one unit takes: one pixel, or one block for a compressed format. */
    public int getUnitBytes() {
        CnbExtension.requireAvailable();
        int[] unitBytes = new int[1];
        CnbExtension.check("CnbTextureFormat.getUnitBytes",
                NativeCnbRoutes.cnbGetTextureFormatUnitBytes(ordinal(), unitBytes));
        return unitBytes[0];
    }

    /**
     * Returns how many bytes one mip level of these dimensions occupies.
     *
     * @param width the level's width in pixels
     * @param height the level's height in pixels
     * @param depth the level's depth in pixels, one for a two-dimensional texture
     * @return the byte size, computed by CNA rather than restated here
     */
    public long getLevelByteSize(int width, int height, int depth) {
        CnbExtension.requireAvailable();
        long[] size = new long[1];
        CnbExtension.check("CnbTextureFormat.getLevelByteSize", NativeCnbRoutes
                .cnbGetTextureLevelByteSize(ordinal(), width, height, depth, size));
        return size[0];
    }

    /**
     * Returns the XNA surface format this one corresponds to.
     *
     * @return the surface format
     * @throws ContentNotSupportedException when CNB carries a format XNA has no name for
     */
    public SurfaceFormat toSurfaceFormat() {
        CnbExtension.requireAvailable();
        int[] surface = new int[1];
        CnbExtension.check("CnbTextureFormat.toSurfaceFormat",
                NativeCnbRoutes.cnbTextureFormatToSurfaceFormat(ordinal(), surface));
        return SurfaceFormat.values()[surface[0]];
    }

    /**
     * Returns the CNB format one XNA surface format corresponds to.
     *
     * @param surfaceFormat the XNA format
     * @return the CNB format
     */
    public static CnbTextureFormat fromSurfaceFormat(SurfaceFormat surfaceFormat) {
        CnbExtension.requireAvailable();
        int[] format = new int[1];
        CnbExtension.check("CnbTextureFormat.fromSurfaceFormat", NativeCnbRoutes
                .cnbTextureFormatFromSurfaceFormat(surfaceFormat.ordinal(), format));
        return fromValue(format[0]);
    }

    /** Reports whether a wire value names a format this build knows. */
    public static boolean isKnown(int value) {
        CnbExtension.requireAvailable();
        boolean[] known = new boolean[1];
        CnbExtension.check("CnbTextureFormat.isKnown",
                NativeCnbRoutes.cnbIsKnownTextureFormat(value, known));
        return known[0];
    }

    static CnbTextureFormat fromValue(long value) {
        CnbTextureFormat[] values = values();
        if (value < 0 || value >= values.length) {
            throw new CnbFormatException(
                    "the file names texture format " + value + ", which this build has no "
                    + "constant for; the numbers are wire format, so this is a newer writer");
        }
        return values[(int) value];
    }
}
