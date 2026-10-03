package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Dynamic index buffer whose streaming options and ContentLost contract are backed by CNA. */
@SuppressWarnings("this-escape")
public class DynamicIndexBuffer extends IndexBuffer {

    private final CopyOnWriteArrayList<EventHandler<EventArgs>> contentLostListeners =
            new CopyOnWriteArrayList<>();
    private long contentLostRegistration;
    private Throwable contentLostFailure;

    public DynamicIndexBuffer(
            GraphicsDevice graphicsDevice,
            IndexElementSize indexElementSize,
            int indexCount,
            BufferUsage usage) {
        super(graphicsDevice, indexElementSize, indexCount, usage, true);
    }

    public DynamicIndexBuffer(
            GraphicsDevice graphicsDevice,
            Class<?> indexType,
            int indexCount,
            BufferUsage usage) {
        this(graphicsDevice, sizeForType(indexType), indexCount, usage);
    }

    public void addContentLostListener(EventHandler<EventArgs> listener) {
        ensureNotDisposed();
        rethrowContentLostFailure();
        EventHandler<EventArgs> selected = Objects.requireNonNull(listener, "listener");
        if (contentLostRegistration == 0L) {
            contentLostRegistration = NativeBindings.subscribeIndexBufferContentLost(this, this);
        }
        contentLostListeners.add(selected);
    }

    public void removeContentLostListener(EventHandler<EventArgs> listener) {
        contentLostListeners.remove(listener);
    }

    public final boolean getIsContentLost() {
        ensureNotDisposed();
        rethrowContentLostFailure();
        return NativeBindings.getIndexBufferIsContentLost(this);
    }

    public final <T> void SetData(
            T[] data,
            int startIndex,
            int elementCount,
            SetDataOptions options) {
        rethrowContentLostFailure();
        setData(-1, data, startIndex, elementCount, options);
    }

    public final <T> void SetData(
            int offsetInBytes,
            T[] data,
            int startIndex,
            int elementCount,
            SetDataOptions options) {
        rethrowContentLostFailure();
        setData(offsetInBytes, data, startIndex, elementCount, options);
    }

    @Override
    void releaseDynamicSubscription() {
        long registration = contentLostRegistration;
        if (registration != 0L) {
            NativeBindings.unsubscribeIndexBufferContentLost(registration);
            contentLostRegistration = 0L;
        }
        contentLostListeners.clear();
        contentLostFailure = null;
    }

    @SuppressWarnings("unused")
    private void nativeContentLost() {
        Throwable failure = null;
        for (EventHandler<EventArgs> listener : contentLostListeners) {
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
        if (failure != null) {
            synchronized (this) {
                if (contentLostFailure == null) {
                    contentLostFailure = failure;
                } else {
                    contentLostFailure.addSuppressed(failure);
                }
            }
        }
    }

    private void rethrowContentLostFailure() {
        Throwable failure;
        synchronized (this) {
            failure = contentLostFailure;
            contentLostFailure = null;
        }
        if (failure instanceof RuntimeException runtime) {
            throw runtime;
        }
        if (failure instanceof Error error) {
            throw error;
        }
        if (failure != null) {
            throw new IllegalStateException("ContentLost listener failed", failure);
        }
    }
}
