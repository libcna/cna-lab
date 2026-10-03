package Microsoft.Xna.Framework;

import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Managed implementation of XNA's base updateable game component. */
public class GameComponent implements IGameComponent, IUpdateable, AutoCloseable {

    private final Game game;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> enabledChangedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> updateOrderChangedListeners =
            new CopyOnWriteArrayList<>();
    private boolean enabled = true;
    private int updateOrder;
    private boolean disposed;

    public GameComponent(Game game) {
        this.game = Objects.requireNonNull(game, "game");
    }

    public final Game getGame() {
        return game;
    }

    @Override
    public final boolean getEnabled() {
        return enabled;
    }

    public final void setEnabled(boolean value) {
        ensureOpen();
        if (enabled != value) {
            enabled = value;
            OnEnabledChanged(this, EventArgs.Empty);
        }
    }

    @Override
    public final int getUpdateOrder() {
        return updateOrder;
    }

    public final void setUpdateOrder(int value) {
        ensureOpen();
        if (updateOrder != value) {
            updateOrder = value;
            OnUpdateOrderChanged(this, EventArgs.Empty);
        }
    }

    @Override
    public void Initialize() {
    }

    @Override
    public void Update(GameTime gameTime) {
        Objects.requireNonNull(gameTime, "gameTime");
    }

    public final void addDisposedListener(EventHandler<EventArgs> listener) {
        disposedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDisposedListener(EventHandler<EventArgs> listener) {
        disposedListeners.remove(listener);
    }

    @Override
    public final void addEnabledChangedListener(EventHandler<EventArgs> listener) {
        enabledChangedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    @Override
    public final void removeEnabledChangedListener(EventHandler<EventArgs> listener) {
        enabledChangedListeners.remove(listener);
    }

    @Override
    public final void addUpdateOrderChangedListener(EventHandler<EventArgs> listener) {
        updateOrderChangedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    @Override
    public final void removeUpdateOrderChangedListener(EventHandler<EventArgs> listener) {
        updateOrderChangedListeners.remove(listener);
    }

    protected void OnEnabledChanged(Object sender, EventArgs args) {
        for (EventHandler<EventArgs> listener : enabledChangedListeners) {
            listener.invoke(sender, args);
        }
    }

    protected void OnUpdateOrderChanged(Object sender, EventArgs args) {
        for (EventHandler<EventArgs> listener : updateOrderChangedListeners) {
            listener.invoke(sender, args);
        }
    }

    protected void Dispose(boolean disposing) {
    }

    public final void Dispose() {
        if (disposed) {
            return;
        }
        RuntimeException failure = null;
        try {
            Dispose(true);
        } catch (RuntimeException exception) {
            failure = exception;
        } finally {
            disposed = true;
        }
        try {
            for (EventHandler<EventArgs> listener : disposedListeners) {
                listener.invoke(this, EventArgs.Empty);
            }
        } catch (RuntimeException exception) {
            if (failure == null) {
                failure = exception;
            } else {
                failure.addSuppressed(exception);
            }
        } finally {
            disposedListeners.clear();
            enabledChangedListeners.clear();
            updateOrderChangedListeners.clear();
        }
        if (failure != null) {
            throw failure;
        }
    }

    @Override
    public final void close() {
        Dispose();
    }

    private void ensureOpen() {
        if (disposed) {
            throw new IllegalStateException("GameComponent is already closed");
        }
    }
}
