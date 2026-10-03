package Microsoft.Xna.Framework.Graphics;

/** Base class for XNA texture resources. */
public abstract class Texture extends GraphicsResource {

    private SurfaceFormat format;
    private int levelCount;

    Texture(GraphicsDevice graphicsDevice) {
        super(graphicsDevice);
    }

    public final SurfaceFormat getFormat() {
        ensureNotDisposed();
        return format;
    }

    public final int getLevelCount() {
        ensureNotDisposed();
        return levelCount;
    }

    final void setTextureInfo(SurfaceFormat format, int levelCount) {
        this.format = format;
        this.levelCount = levelCount;
    }
}
