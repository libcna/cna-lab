package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** XNA GPU occlusion query backed by CNA where the selected renderer supports queries. */
@SuppressWarnings("this-escape")
public class OcclusionQuery extends GraphicsResource {

    public OcclusionQuery(GraphicsDevice graphicsDevice) {
        super(Objects.requireNonNull(graphicsDevice, "graphicsDevice"));
        NativeBindings.createOcclusionQuery(this, graphicsDevice);
    }

    public final void Begin() {
        ensureNotDisposed();
        NativeBindings.beginOcclusionQuery(this);
    }

    public final void End() {
        ensureNotDisposed();
        NativeBindings.endOcclusionQuery(this);
    }

    public final boolean getIsComplete() {
        ensureNotDisposed();
        return NativeBindings.getOcclusionQueryComplete(this);
    }

    public final int getPixelCount() {
        ensureNotDisposed();
        return NativeBindings.getOcclusionQueryPixelCount(this);
    }

    @Override
    protected void Dispose(boolean arg0) {
        if (arg0 && !getIsDisposed()) {
            NativeBindings.closeGraphicsResource(this);
        }
        super.Dispose(arg0);
    }
}
