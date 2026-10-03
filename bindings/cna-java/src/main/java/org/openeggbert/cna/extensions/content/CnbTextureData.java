package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.SurfaceFormat;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.Objects;

/**
 * A texture as {@code .cnb} carries it, before it is a GPU resource.
 *
 * <p><strong>One image, several representations.</strong> A {@code .cnb} texture may hold the
 * same picture in more than one format -- an uncompressed copy and a block-compressed one, say --
 * so a game can pick whichever its hardware supports. That is why this is not simply a
 * {@link Texture2D}: choosing the representation is the caller's decision, and
 * {@link #toTexture2D(GraphicsDevice)} makes the ordinary one for them.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op. The bytes
 * are CNA's copy, so the array a caller passed in stays theirs and a level read out is a fresh
 * copy with no lifetime.
 */
public final class CnbTextureData implements AutoCloseable {

    private final long handle;
    private boolean closed;

    CnbTextureData(long handle) {
        this.handle = handle;
    }

    /**
     * Creates a texture of one size with no representations yet.
     *
     * @param width the widest level's width
     * @param height the widest level's height
     * @param depth the widest level's depth, one for a two-dimensional texture
     * @param faceCount six for a cube texture, one otherwise
     * @param mipCount how many mip levels each representation will have
     * @return the texture data, which the caller closes
     */
    public static CnbTextureData create(
            int width, int height, int depth, int faceCount, int mipCount) {
        CnbExtension.requireAvailable();
        long[] texture = new long[1];
        CnbExtension.check("CnbTextureData.create", NativeCnbRoutes.cnbTextureDataCreate(
                width, height, depth, faceCount, mipCount, texture));
        return new CnbTextureData(texture[0]);
    }

    /**
     * Creates a single-level RGBA texture from pixel bytes.
     *
     * @param width the width in pixels
     * @param height the height in pixels
     * @param rgba four bytes per pixel, row by row; CNA copies them
     * @return the texture data, which the caller closes
     */
    public static CnbTextureData ofRgba8(int width, int height, byte[] rgba) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(rgba, "rgba");
        long[] texture = new long[1];
        CnbExtension.check("CnbTextureData.ofRgba8",
                NativeCnbRoutes.cnbTextureDataCreateRgba8(width, height, rgba, texture));
        return new CnbTextureData(texture[0]);
    }

    /** Returns the texture's shape. */
    public CnbTextureInfo getInfo() {
        long[] values = new long[6];
        CnbExtension.check("CnbTextureData.getInfo",
                NativeCnbRoutes.cnbTextureDataGetInfo(open(), values));
        return new CnbTextureInfo((int) values[0], (int) values[1], (int) values[2],
                (int) values[3], (int) values[4], (int) values[5]);
    }

    /** Returns how many format-specific copies of the image the texture carries. */
    public int getRepresentationCount() {
        long[] count = new long[1];
        CnbExtension.check("CnbTextureData.getRepresentationCount",
                NativeCnbRoutes.cnbTextureDataGetRepresentationCount(open(), count));
        return (int) count[0];
    }

    /**
     * Returns one representation's format.
     *
     * @param representation the zero-based representation index
     * @return the format its levels are stored in
     */
    public CnbTextureFormat getRepresentationFormat(int representation) {
        int[] format = new int[1];
        CnbExtension.check("CnbTextureData.getRepresentationFormat", NativeCnbRoutes
                .cnbTextureDataGetRepresentationFormat(open(), representation, format));
        return CnbTextureFormat.fromValue(format[0]);
    }

    /**
     * Adds a representation in one format.
     *
     * @param format the format its levels will be stored in
     * @return the new representation's index
     */
    public int addRepresentation(CnbTextureFormat format) {
        Objects.requireNonNull(format, "format");
        long[] index = new long[1];
        CnbExtension.check("CnbTextureData.addRepresentation", NativeCnbRoutes
                .cnbTextureDataAddRepresentation(open(), format.ordinal(), index));
        return (int) index[0];
    }

    /**
     * Returns how many mip levels one representation has.
     *
     * @param representation the zero-based representation index
     * @return the level count
     */
    public int getLevelCount(int representation) {
        long[] count = new long[1];
        CnbExtension.check("CnbTextureData.getLevelCount",
                NativeCnbRoutes.cnbTextureDataGetLevelCount(open(), representation, count));
        return (int) count[0];
    }

    /**
     * Returns one mip level's dimensions.
     *
     * @param level the zero-based mip level
     * @return width, height and depth in that order
     */
    public int[] getLevelDimensions(int level) {
        int[] width = new int[1];
        int[] height = new int[1];
        int[] depth = new int[1];
        CnbExtension.check("CnbTextureData.getLevelDimensions", NativeCnbRoutes
                .cnbTextureDataGetLevelDimensions(open(), level, width, height, depth));
        return new int[] {width[0], height[0], depth[0]};
    }

    /**
     * Replaces one mip level's bytes.
     *
     * @param representation the zero-based representation index
     * @param level the zero-based mip level
     * @param bytes the level's payload; CNA copies it
     */
    public void setLevel(int representation, int level, byte[] bytes) {
        Objects.requireNonNull(bytes, "bytes");
        CnbExtension.check("CnbTextureData.setLevel", NativeCnbRoutes
                .cnbTextureDataSetLevel(open(), representation, level, bytes));
    }

    /**
     * Returns one mip level's bytes.
     *
     * @param representation the zero-based representation index
     * @param level the zero-based mip level
     * @return a fresh copy of the level's payload
     */
    public byte[] readLevel(int representation, int level) {
        long handle = open();
        int[] dimensions = getLevelDimensions(level);
        CnbTextureFormat format = getRepresentationFormat(representation);
        long size = format.getLevelByteSize(dimensions[0], dimensions[1], dimensions[2]);
        byte[] destination = new byte[Math.toIntExact(size)];
        long[] written = new long[1];
        CnbExtension.check("CnbTextureData.readLevel", NativeCnbRoutes
                .cnbTextureDataCopyLevel(handle, representation, level, destination, written));
        if (written[0] != destination.length) {
            byte[] exact = new byte[(int) written[0]];
            System.arraycopy(destination, 0, exact, 0, exact.length);
            return exact;
        }
        return destination;
    }

    /**
     * Uploads the texture to the graphics device as an ordinary XNA {@link Texture2D}.
     *
     * <p>The first representation whose format XNA can name is the one used, because a format
     * CNB carries and XNA has no {@link SurfaceFormat} for cannot become a Texture2D at all.
     * Every mip level is uploaded.
     *
     * @param graphicsDevice the device to create the texture on
     * @return the texture, owned by the device as any other is
     * @throws ContentNotSupportedException when no representation has a format XNA can name
     */
    public Texture2D toTexture2D(GraphicsDevice graphicsDevice) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        CnbTextureInfo info = getInfo();
        for (int representation = 0; representation < info.RepresentationCount();
                representation++) {
            CnbTextureFormat format = getRepresentationFormat(representation);
            SurfaceFormat surface;
            try {
                surface = format.toSurfaceFormat();
            } catch (ContentNotSupportedException | CnbFormatException unnameable) {
                continue;
            }
            int levels = getLevelCount(representation);
            Texture2D texture = new Texture2D(graphicsDevice, info.Width(), info.Height(),
                    levels > 1, surface);
            try {
                for (int level = 0; level < levels; level++) {
                    // The level's bytes go up as bytes. XNA's own SetData is generic over the
                    // element type a game chose, which would mean boxing every pixel only for
                    // XNA to encode it straight back; the transfer still goes through the codec
                    // this surface format declares.
                    org.openeggbert.cna.internal.FacadeFactory.setTexture2DLevelBytes(
                            texture, level, readLevel(representation, level));
                }
                return texture;
            } catch (RuntimeException failure) {
                texture.close();
                throw failure;
            }
        }
        throw new ContentNotSupportedException(
                "no representation of this .cnb texture has a format XNA can name");
    }

    long handle() {
        return open();
    }

    /** Releases the texture data. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbTextureData.close",
                NativeCnbRoutes.cnbTextureDataDestroy(handle));
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnbTextureData is closed");
            }
        }
        return handle;
    }
}
