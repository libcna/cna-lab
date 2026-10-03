package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Base for device resources whose CNA ownership is released deterministically. */
public abstract class GraphicsResource implements AutoCloseable {

    private GraphicsDevice graphicsDevice;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposingListeners =
            new CopyOnWriteArrayList<>();
    private String name;
    private Object tag;
    private boolean disposed;

    GraphicsResource() {
    }

    GraphicsResource(GraphicsDevice graphicsDevice) {
        this.graphicsDevice = Objects.requireNonNull(graphicsDevice, "graphicsDevice");
    }

    public final void addDisposingListener(EventHandler<EventArgs> listener) {
        disposingListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDisposingListener(EventHandler<EventArgs> listener) {
        disposingListeners.remove(listener);
    }

    public final GraphicsDevice getGraphicsDevice() {
        return graphicsDevice;
    }

    public final boolean getIsDisposed() {
        return disposed;
    }

    public final String getName() {
        return name;
    }

    public final void setName(String value) {
        name = value;
    }

    public final Object getTag() {
        return tag;
    }

    public final void setTag(Object value) {
        tag = value;
    }

    protected void Dispose(boolean arg0) {
    }

    public final void Dispose() {
        if (disposed) {
            return;
        }
        Dispose(true);
        disposed = true;
        Throwable failure = null;
        for (EventHandler<EventArgs> listener : disposingListeners) {
            try {
                listener.invoke(this, EventArgs.Empty);
            } catch (Throwable exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        disposingListeners.clear();
        try {
            NativeBindings.rethrowGraphicsDeviceListenerFailure(graphicsDevice);
        } catch (Throwable exception) {
            if (failure == null) {
                failure = exception;
            } else {
                failure.addSuppressed(exception);
            }
        }
        if (failure != null) {
            rethrow(failure);
        }
    }

    @Override
    public final void close() {
        Dispose();
    }

    @Override
    public String toString() {
        return name == null || name.isEmpty() ? super.toString() : name;
    }

    final void ensureNotDisposed() {
        if (disposed) {
            throw new IllegalStateException(getClass().getSimpleName() + " is already disposed");
        }
    }

    final void attachGraphicsDevice(GraphicsDevice value) {
        graphicsDevice = Objects.requireNonNull(value, "graphicsDevice");
    }

    private static void rethrow(Throwable failure) {
        if (failure instanceof RuntimeException runtime) {
            throw runtime;
        }
        if (failure instanceof Error error) {
            throw error;
        }
        throw new IllegalStateException("GraphicsResource listener failed", failure);
    }
}
