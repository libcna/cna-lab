package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Cube-map render target backed by CNA's owned render-target resource. */
@SuppressWarnings("this-escape")
public class RenderTargetCube extends TextureCube {

    private final CopyOnWriteArrayList<EventHandler<EventArgs>> contentLostListeners =
            new CopyOnWriteArrayList<>();
    private DepthFormat depthStencilFormat;
    private int multiSampleCount;
    private RenderTargetUsage renderTargetUsage;

    public RenderTargetCube(
            GraphicsDevice graphicsDevice,
            int size,
            boolean mipMap,
            SurfaceFormat preferredFormat,
            DepthFormat preferredDepthFormat) {
        this(graphicsDevice, size, mipMap, preferredFormat,
                preferredDepthFormat, 0, RenderTargetUsage.DiscardContents);
    }

    public RenderTargetCube(
            GraphicsDevice graphicsDevice,
            int size,
            boolean mipMap,
            SurfaceFormat preferredFormat,
            DepthFormat preferredDepthFormat,
            int preferredMultiSampleCount,
            RenderTargetUsage usage) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
        if (size <= 0) {
            throw new IllegalArgumentException("RenderTargetCube size must be positive");
        }
        if (preferredMultiSampleCount < 0) {
            throw new IllegalArgumentException("RenderTargetCube multisample count must not be negative");
        }
        int[] info = NativeBindings.createRenderTargetCube(
                this, graphicsDevice, size, mipMap,
                Objects.requireNonNull(preferredFormat, "preferredFormat").ordinal(),
                Objects.requireNonNull(preferredDepthFormat, "preferredDepthFormat").ordinal(),
                preferredMultiSampleCount,
                Objects.requireNonNull(usage, "usage").ordinal());
        initialize(new int[]{info[0], info[2], info[3]});
        initializeRenderTarget(info);
    }

    public void addContentLostListener(EventHandler<EventArgs> listener) {
        contentLostListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeContentLostListener(EventHandler<EventArgs> listener) {
        contentLostListeners.remove(listener);
    }

    public final DepthFormat getDepthStencilFormat() {
        ensureNotDisposed();
        return depthStencilFormat;
    }

    public final boolean getIsContentLost() {
        ensureNotDisposed();
        return NativeBindings.getRenderTargetIsContentLost(this);
    }

    public final int getMultiSampleCount() {
        ensureNotDisposed();
        return multiSampleCount;
    }

    public final RenderTargetUsage getRenderTargetUsage() {
        ensureNotDisposed();
        return renderTargetUsage;
    }

    @Override
    protected void Dispose(boolean arg0) {
        if (arg0 && !getIsDisposed()) {
            NativeBindings.closeGraphicsResource(this);
            contentLostListeners.clear();
        }
        super.Dispose(false);
    }

    private void initializeRenderTarget(int[] info) {
        if (info.length != 10 || info[4] < 0 || info[4] >= DepthFormat.values().length
                || info[5] < 0 || info[6] < 0
                || info[6] >= RenderTargetUsage.values().length) {
            NativeBindings.closeGraphicsResource(this);
            throw new IllegalStateException("CNA returned invalid RenderTargetCube metadata");
        }
        depthStencilFormat = DepthFormat.values()[info[4]];
        multiSampleCount = info[5];
        renderTargetUsage = RenderTargetUsage.values()[info[6]];
    }
}
