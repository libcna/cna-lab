package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Rectangle;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Device-owned cube texture backed by CNA's stable texture-volume ABI. */
@SuppressWarnings("this-escape")
public class TextureCube extends Texture {

    private int size;

    TextureCube(GraphicsDevice graphicsDevice) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
    }

    public TextureCube(
            GraphicsDevice graphicsDevice,
            int size,
            boolean mipMap,
            SurfaceFormat format) {
        this(graphicsDevice);
        if (size <= 0) {
            throw new IllegalArgumentException("TextureCube size must be positive");
        }
        initialize(NativeBindings.createTextureCube(
                this, graphicsDevice, size, mipMap,
                Objects.requireNonNull(format, "format").ordinal()));
    }

    public final <T> void SetData(CubeMapFace cubeMapFace, T[] data) {
        Objects.requireNonNull(data, "data");
        SetData(cubeMapFace, 0, null, data, 0, data.length);
    }

    public final <T> void SetData(
            CubeMapFace cubeMapFace,
            T[] data,
            int startIndex,
            int elementCount) {
        SetData(cubeMapFace, 0, null, data, startIndex, elementCount);
    }

    public final <T> void SetData(
            CubeMapFace cubeMapFace,
            int level,
            Rectangle rect,
            T[] data,
            int startIndex,
            int elementCount) {
        ensureNotDisposed();
        Objects.requireNonNull(cubeMapFace, "cubeMapFace");
        Objects.requireNonNull(data, "data");
        if (!(data instanceof Color[] colors)) {
            throw new UnsupportedOperationException(
                    "CNA's TextureCube C ABI exposes Color transfers only");
        }
        Rectangle region = validateTransfer(level, rect, data.length, startIndex, elementCount);
        Color[] snapshot = new Color[data.length];
        for (int index = startIndex; index < startIndex + elementCount; index++) {
            snapshot[index] = new Color(Objects.requireNonNull(colors[index], "data[" + index + "]"));
        }
        NativeBindings.setTextureCubeData(
                this, cubeMapFace.ordinal(), level, region,
                snapshot, startIndex, elementCount);
    }

    public final <T> void GetData(CubeMapFace cubeMapFace, T[] data) {
        Objects.requireNonNull(data, "data");
        GetData(cubeMapFace, 0, null, data, 0, data.length);
    }

    public final <T> void GetData(
            CubeMapFace cubeMapFace,
            T[] data,
            int startIndex,
            int elementCount) {
        GetData(cubeMapFace, 0, null, data, startIndex, elementCount);
    }

    public final <T> void GetData(
            CubeMapFace cubeMapFace,
            int level,
            Rectangle rect,
            T[] data,
            int startIndex,
            int elementCount) {
        ensureNotDisposed();
        Objects.requireNonNull(cubeMapFace, "cubeMapFace");
        Objects.requireNonNull(data, "data");
        if (!(data instanceof Color[] colors)) {
            throw new UnsupportedOperationException(
                    "CNA's TextureCube C ABI exposes Color transfers only");
        }
        Rectangle region = validateTransfer(level, rect, data.length, startIndex, elementCount);
        Color[] snapshot = NativeBindings.getTextureCubeData(
                this, cubeMapFace.ordinal(), level, region,
                data.length, startIndex, elementCount);
        System.arraycopy(snapshot, startIndex, colors, startIndex, elementCount);
    }

    public final int getSize() {
        ensureNotDisposed();
        return size;
    }

    @Override
    protected void Dispose(boolean arg0) {
        if (arg0 && !getIsDisposed()) {
            NativeBindings.closeGraphicsResource(this);
        }
        super.Dispose(arg0);
    }

    final void initialize(int[] info) {
        if (info.length != 3 || info[0] <= 0 || info[1] <= 0
                || info[2] < 0 || info[2] >= SurfaceFormat.values().length) {
            NativeBindings.closeGraphicsResource(this);
            throw new IllegalStateException("CNA returned invalid TextureCube metadata");
        }
        size = info[0];
        setTextureInfo(SurfaceFormat.values()[info[2]], info[1]);
    }

    private Rectangle validateTransfer(
            int level,
            Rectangle rect,
            int arrayLength,
            int startIndex,
            int elementCount) {
        if (arrayLength == 0) {
            throw new IllegalArgumentException("TextureCube data array must not be empty");
        }
        if (level < 0 || level >= getLevelCount()) {
            throw new IllegalStateException("TextureCube mip level is outside the allocated chain");
        }
        if (startIndex < 0 || startIndex > arrayLength) {
            throw new IndexOutOfBoundsException("TextureCube data start index is outside the array");
        }
        if (elementCount <= 0 || elementCount > arrayLength - startIndex) {
            throw new IndexOutOfBoundsException("TextureCube data window is outside the array");
        }
        int levelSize = Math.max(1, size >> level);
        Rectangle region = rect == null ? null : new Rectangle(rect);
        long expected = (long)levelSize * levelSize;
        if (region != null) {
            if (region.X < 0 || region.Y < 0 || region.Width <= 0 || region.Height <= 0
                    || (long)region.X + region.Width > levelSize
                    || (long)region.Y + region.Height > levelSize) {
                throw new IllegalArgumentException(
                        "TextureCube rectangle is outside the selected mip level");
            }
            expected = (long)region.Width * region.Height;
        }
        if (elementCount != expected) {
            throw new IllegalArgumentException(
                    "TextureCube transfer element count must be exactly " + expected);
        }
        return region;
    }
}
