package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Content.ContentLoadException;
import Microsoft.Xna.Framework.Rectangle;
import org.openeggbert.cna.internal.NativeBindings;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Objects;

/** Device-owned two-dimensional texture backed by CNA's stable C resource ABI. */
@SuppressWarnings("this-escape")
public class Texture2D extends Texture {

    private int width;
    private int height;

    Texture2D(GraphicsDevice graphicsDevice) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
    }

    public Texture2D(GraphicsDevice graphicsDevice, int width, int height) {
        this(graphicsDevice, width, height, false, SurfaceFormat.Color);
    }

    public Texture2D(
            GraphicsDevice graphicsDevice,
            int width,
            int height,
            boolean mipMap,
            SurfaceFormat format) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
        if (width <= 0 || height <= 0) {
            throw new IllegalArgumentException("Texture dimensions must be positive");
        }
        initialize(NativeBindings.createTexture2D(
                this, graphicsDevice, width, height, mipMap,
                Objects.requireNonNull(format, "format").ordinal()));
    }

    private Texture2D(
            GraphicsDevice graphicsDevice,
            byte[] encoded,
            int width,
            int height,
            boolean zoom,
            boolean resize) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
        initialize(NativeBindings.createTexture2DFromEncoded(
                this, graphicsDevice, encoded, width, height, zoom, resize));
    }

    public static Texture2D FromStream(GraphicsDevice graphicsDevice, InputStream stream) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        return new Texture2D(graphicsDevice, readEncoded(stream), 0, 0, false, false);
    }

    public static Texture2D FromStream(
            GraphicsDevice graphicsDevice,
            InputStream stream,
            int width,
            int height,
            boolean zoom) {
        Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        if (width <= 0 || height <= 0) {
            throw new IllegalArgumentException("Decoded texture dimensions must be positive");
        }
        return new Texture2D(graphicsDevice, readEncoded(stream), width, height, zoom, true);
    }

    public final <T> void SetData(T[] data) {
        Objects.requireNonNull(data, "data");
        SetData(0, null, data, 0, data.length);
    }

    public final <T> void SetData(T[] data, int startIndex, int elementCount) {
        SetData(0, null, data, startIndex, elementCount);
    }

    public final <T> void SetData(
            int level,
            Rectangle rect,
            T[] data,
            int startIndex,
            int elementCount) {
        ensureNotDisposed();
        Objects.requireNonNull(data, "data");
        Rectangle region = rect == null ? null : new Rectangle(rect);
        TextureDataCodec codec = validateTransfer(
                level, region, data, startIndex, elementCount);
        byte[] snapshot = codec.encode(data, startIndex, elementCount);
        NativeBindings.setTexture2DData(
                this, codec.dataType(), level, region,
                startIndex, elementCount, snapshot);
    }

    /**
     * Uploads one mip level's bytes exactly as a content format stored them.
     *
     * <p>Package-private, and reached only from {@code org.openeggbert.cna.extensions.content}
     * through the internal facade. XNA's own {@code SetData} is generic over the element type a
     * game chose, which is right for a game that has a {@code Color[]}; a content format has
     * bytes, and re-boxing them into elements only to have XNA encode them straight back would
     * be a second conversion that can only lose. The transfer still goes through the codec this
     * surface format declares, so nothing about what CNA receives changes.
     */
    final void setLevelBytes(int level, byte[] bytes) {
        ensureNotDisposed();
        Objects.requireNonNull(bytes, "bytes");
        TextureDataCodec codec = TextureDataCodec.forFormat(getFormat());
        int elementSize = codec.elementSize();
        if (bytes.length % elementSize != 0) {
            throw new IllegalArgumentException(
                    "A " + getFormat() + " level is a whole number of " + elementSize
                    + "-byte elements; got " + bytes.length + " bytes");
        }
        NativeBindings.setTexture2DData(this, codec.dataType(), level, null,
                0, bytes.length / elementSize, bytes);
    }

    public final <T> void GetData(T[] data) {
        Objects.requireNonNull(data, "data");
        GetData(0, null, data, 0, data.length);
    }

    public final <T> void GetData(T[] data, int startIndex, int elementCount) {
        GetData(0, null, data, startIndex, elementCount);
    }

    public final <T> void GetData(
            int level,
            Rectangle rect,
            T[] data,
            int startIndex,
            int elementCount) {
        ensureNotDisposed();
        Objects.requireNonNull(data, "data");
        Rectangle region = rect == null ? null : new Rectangle(rect);
        TextureDataCodec codec = validateTransfer(
                level, region, data, startIndex, elementCount);
        int payloadBytes = Math.multiplyExact(data.length, codec.elementSize());
        byte[] snapshot = NativeBindings.getTexture2DData(
                this, codec.dataType(), level, region,
                startIndex, elementCount, payloadBytes);
        codec.decodeInto(snapshot, data, startIndex, elementCount);
    }

    public final void SaveAsPng(OutputStream stream, int width, int height) {
        saveAs(stream, 0, width, height, "SaveAsPng");
    }

    public final void SaveAsJpeg(OutputStream stream, int width, int height) {
        saveAs(stream, 1, width, height, "SaveAsJpeg");
    }

    public final Rectangle getBounds() {
        ensureNotDisposed();
        return new Rectangle(0, 0, width, height);
    }

    public final int getHeight() {
        ensureNotDisposed();
        return height;
    }

    public final int getWidth() {
        ensureNotDisposed();
        return width;
    }

    @Override
    protected void Dispose(boolean arg0) {
        if (arg0 && !getIsDisposed()) {
            NativeBindings.closeGraphicsResource(this);
        }
        super.Dispose(arg0);
    }

    final void initialize(int[] info) {
        if (info.length != 4 || info[0] <= 0 || info[1] <= 0 || info[2] <= 0
                || info[3] < 0 || info[3] >= SurfaceFormat.values().length) {
            NativeBindings.closeGraphicsResource(this);
            throw new IllegalStateException("CNA returned invalid Texture2D metadata");
        }
        width = info[0];
        height = info[1];
        setTextureInfo(SurfaceFormat.values()[info[3]], info[2]);
    }

    private <T> TextureDataCodec validateTransfer(
            int level,
            Rectangle rect,
            T[] data,
            int startIndex,
            int elementCount) {
        if (data.length == 0) {
            throw new IllegalArgumentException("Texture data array must not be empty");
        }
        if (level < 0 || level >= getLevelCount()) {
            throw new IllegalStateException("Texture mip level is outside the allocated chain");
        }
        validateArrayWindow(data.length, startIndex, elementCount);

        int levelWidth = Math.max(1, width >> level);
        int levelHeight = Math.max(1, height >> level);
        int transferWidth = levelWidth;
        int transferHeight = levelHeight;
        if (rect != null) {
            if (rect.X < 0 || rect.Y < 0 || rect.Width <= 0 || rect.Height <= 0
                    || (long)rect.X + rect.Width > levelWidth
                    || (long)rect.Y + rect.Height > levelHeight) {
                throw new IllegalArgumentException("Texture rectangle is outside the selected mip level");
            }
            transferWidth = rect.Width;
            transferHeight = rect.Height;
        }

        TextureDataCodec codec = TextureDataCodec.select(
                data.getClass().getComponentType(), getFormat());
        long expectedBytes = expectedByteCount(getFormat(), transferWidth, transferHeight);
        if (expectedBytes % codec.elementSize() != 0L) {
            throw new UnsupportedOperationException(
                    "The selected Java element type cannot exactly represent the texture payload");
        }
        long expected = expectedBytes / codec.elementSize();
        if (elementCount != expected) {
            throw new IllegalArgumentException(
                    "Texture transfer element count must be exactly " + expected);
        }
        return codec;
    }

    private static long expectedByteCount(SurfaceFormat format, int width, int height) {
        return switch (format) {
            case Dxt1 -> (long)((width + 3) >> 2) * ((height + 3) >> 2) * 8L;
            case Dxt3, Dxt5 -> (long)((width + 3) >> 2) * ((height + 3) >> 2) * 16L;
            case Bgr565, Bgra5551, Bgra4444, NormalizedByte2, HalfSingle ->
                    (long)width * height * 2L;
            case Color, NormalizedByte4, Rgba1010102, Rg32, Single, HalfVector2 ->
                    (long)width * height * 4L;
            case Rgba64, Vector2, HalfVector4, HdrBlendable ->
                    (long)width * height * 8L;
            case Vector4 -> (long)width * height * 16L;
            case Alpha8 -> (long)width * height;
        };
    }

    private void saveAs(OutputStream stream, int format, int width, int height, String operation) {
        ensureNotDisposed();
        Objects.requireNonNull(stream, "stream");
        if (width <= 0 || height <= 0) {
            throw new IllegalArgumentException("Encoded texture dimensions must be positive");
        }
        try {
            stream.write(NativeBindings.encodeTexture2D(this, format, width, height));
        } catch (IOException exception) {
            throw new ContentLoadException(
                    operation + " could not write the Java output stream", new RuntimeException(exception));
        }
    }

    private static byte[] readEncoded(InputStream stream) {
        try {
            byte[] encoded = Objects.requireNonNull(stream, "stream").readAllBytes();
            if (encoded.length == 0) {
                throw new IllegalArgumentException("Encoded image stream must not be empty");
            }
            return encoded;
        } catch (IOException exception) {
            throw new ContentLoadException(
                    "Texture2D.FromStream could not read the Java input stream",
                    new RuntimeException(exception));
        }
    }

    private static void validateArrayWindow(int length, int startIndex, int elementCount) {
        if (startIndex < 0 || startIndex > length) {
            throw new IndexOutOfBoundsException("Texture data start index is outside the array");
        }
        if (elementCount <= 0 || elementCount > length - startIndex) {
            throw new IndexOutOfBoundsException("Texture data array window is outside the source array");
        }
    }
}
