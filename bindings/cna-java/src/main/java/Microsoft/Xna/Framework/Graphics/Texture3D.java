package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Device-owned volume texture backed by CNA's stable texture-volume ABI. */
@SuppressWarnings("this-escape")
public class Texture3D extends Texture {

    private int width;
    private int height;
    private int depth;

    Texture3D(GraphicsDevice graphicsDevice) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
    }

    public Texture3D(
            GraphicsDevice graphicsDevice,
            int width,
            int height,
            int depth,
            boolean mipMap,
            SurfaceFormat format) {
        this(graphicsDevice);
        if (width <= 0 || height <= 0 || depth <= 0) {
            throw new IllegalArgumentException("Texture3D dimensions must be positive");
        }
        initialize(NativeBindings.createTexture3D(
                this, graphicsDevice, width, height, depth, mipMap,
                Objects.requireNonNull(format, "format").ordinal()));
    }

    public final <T> void SetData(T[] data) {
        Objects.requireNonNull(data, "data");
        SetData(data, 0, data.length);
    }

    public final <T> void SetData(T[] data, int startIndex, int elementCount) {
        SetData(0, 0, 0, getWidth(), getHeight(), 0, getDepth(),
                data, startIndex, elementCount);
    }

    public final <T> void SetData(
            int level,
            int left,
            int top,
            int right,
            int bottom,
            int front,
            int back,
            T[] data,
            int startIndex,
            int elementCount) {
        ensureNotDisposed();
        Objects.requireNonNull(data, "data");
        if (!(data instanceof Color[] colors)) {
            throw new UnsupportedOperationException(
                    "CNA-Java's Texture3D route currently exposes Color transfers only");
        }
        validateTransfer(level, left, top, right, bottom, front, back,
                data.length, startIndex, elementCount);
        Color[] snapshot = new Color[data.length];
        for (int index = startIndex; index < startIndex + elementCount; index++) {
            snapshot[index] = new Color(Objects.requireNonNull(colors[index], "data[" + index + "]"));
        }
        NativeBindings.setTexture3DData(this, level, left, top, right, bottom, front, back,
                snapshot, startIndex, elementCount);
    }

    public final <T> void GetData(T[] data) {
        Objects.requireNonNull(data, "data");
        GetData(data, 0, data.length);
    }

    public final <T> void GetData(T[] data, int startIndex, int elementCount) {
        GetData(0, 0, 0, getWidth(), getHeight(), 0, getDepth(),
                data, startIndex, elementCount);
    }

    public final <T> void GetData(
            int level,
            int left,
            int top,
            int right,
            int bottom,
            int front,
            int back,
            T[] data,
            int startIndex,
            int elementCount) {
        ensureNotDisposed();
        Objects.requireNonNull(data, "data");
        if (!(data instanceof Color[] colors)) {
            throw new UnsupportedOperationException(
                    "CNA-Java's Texture3D route currently exposes Color transfers only");
        }
        validateTransfer(level, left, top, right, bottom, front, back,
                data.length, startIndex, elementCount);
        Color[] snapshot = NativeBindings.getTexture3DData(
                this, level, left, top, right, bottom, front, back,
                data.length, startIndex, elementCount);
        System.arraycopy(snapshot, startIndex, colors, startIndex, elementCount);
    }

    public final int getWidth() {
        ensureNotDisposed();
        return width;
    }

    public final int getHeight() {
        ensureNotDisposed();
        return height;
    }

    public final int getDepth() {
        ensureNotDisposed();
        return depth;
    }

    @Override
    protected void Dispose(boolean arg0) {
        if (arg0 && !getIsDisposed()) {
            NativeBindings.closeGraphicsResource(this);
        }
        super.Dispose(arg0);
    }

    final void initialize(int[] info) {
        if (info.length != 5 || info[0] <= 0 || info[1] <= 0 || info[2] <= 0
                || info[3] <= 0 || info[4] < 0 || info[4] >= SurfaceFormat.values().length) {
            NativeBindings.closeGraphicsResource(this);
            throw new IllegalStateException("CNA returned invalid Texture3D metadata");
        }
        width = info[0];
        height = info[1];
        depth = info[2];
        setTextureInfo(SurfaceFormat.values()[info[4]], info[3]);
    }

    private void validateTransfer(
            int level,
            int left,
            int top,
            int right,
            int bottom,
            int front,
            int back,
            int arrayLength,
            int startIndex,
            int elementCount) {
        if (level < 0 || level >= getLevelCount()) {
            throw new IllegalArgumentException("Texture3D mip level is outside the allocated chain");
        }
        int levelWidth = Math.max(1, width >> level);
        int levelHeight = Math.max(1, height >> level);
        int levelDepth = Math.max(1, depth >> level);
        if (left < 0 || top < 0 || front < 0 || right <= left || bottom <= top || back <= front
                || right > levelWidth || bottom > levelHeight || back > levelDepth) {
            throw new IllegalArgumentException("Texture3D box is outside the selected mip level");
        }
        if (startIndex < 0 || startIndex > arrayLength || elementCount <= 0
                || elementCount > arrayLength - startIndex) {
            throw new IndexOutOfBoundsException("Texture3D data window is outside the array");
        }
        long expected = Math.multiplyExact(
                Math.multiplyExact((long)(right - left), bottom - top), back - front);
        if (elementCount != expected) {
            throw new IllegalArgumentException(
                    "Texture3D transfer element count must be exactly " + expected);
        }
    }
}
