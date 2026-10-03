package Microsoft.Xna.Framework;

import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Strict XNA window facade; CNA ownership and platform handles remain hidden. */
public abstract class GameWindow {

    private final Game game;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> clientSizeChangedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> orientationChangedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> screenDeviceNameChangedListeners =
            new CopyOnWriteArrayList<>();
    private Throwable pendingListenerFailure;
    private String title;

    GameWindow(Game game, String title) {
        this.game = Objects.requireNonNull(game, "game");
        this.title = Objects.requireNonNull(title, "title");
    }

    public abstract boolean getAllowUserResizing();

    public abstract void setAllowUserResizing(boolean value);

    public abstract Rectangle getClientBounds();

    public abstract DisplayOrientation getCurrentOrientation();

    public abstract WindowHandle getHandle();

    public abstract String getScreenDeviceName();

    public final String getTitle() {
        return title;
    }

    public final void setTitle(String value) {
        String next = Objects.requireNonNull(value, "value");
        if (!title.equals(next)) {
            SetTitle(next);
            title = next;
        }
    }

    public final void addClientSizeChangedListener(EventHandler<EventArgs> listener) {
        clientSizeChangedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeClientSizeChangedListener(EventHandler<EventArgs> listener) {
        clientSizeChangedListeners.remove(listener);
    }

    public final void addOrientationChangedListener(EventHandler<EventArgs> listener) {
        orientationChangedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeOrientationChangedListener(EventHandler<EventArgs> listener) {
        orientationChangedListeners.remove(listener);
    }

    public final void addScreenDeviceNameChangedListener(EventHandler<EventArgs> listener) {
        screenDeviceNameChangedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeScreenDeviceNameChangedListener(EventHandler<EventArgs> listener) {
        screenDeviceNameChangedListeners.remove(listener);
    }

    public abstract void BeginScreenDeviceChange(boolean willBeFullScreen);

    public final void EndScreenDeviceChange(String screenDeviceName) {
        EndScreenDeviceChange(screenDeviceName, 0, 0);
    }

    public abstract void EndScreenDeviceChange(
            String screenDeviceName,
            int clientWidth,
            int clientHeight);

    protected final void OnActivated() {
    }

    protected final void OnClientSizeChanged() {
        invoke(clientSizeChangedListeners);
    }

    protected final void OnDeactivated() {
    }

    protected final void OnOrientationChanged() {
        invoke(orientationChangedListeners);
    }

    protected final void OnPaint() {
    }

    protected final void OnScreenDeviceNameChanged() {
        invoke(screenDeviceNameChangedListeners);
    }

    protected abstract void SetSupportedOrientations(DisplayOrientation orientations);

    protected abstract void SetTitle(String title);

    final Game game() {
        return game;
    }

    final void dispatchNativeEvent(int event) {
        try {
            switch (event) {
                case 0 -> OnClientSizeChanged();
                case 1 -> OnOrientationChanged();
                case 2 -> OnScreenDeviceNameChanged();
                default -> throw new IllegalArgumentException("Unknown native window event " + event);
            }
        } catch (Throwable failure) {
            synchronized (this) {
                if (pendingListenerFailure == null) {
                    pendingListenerFailure = failure;
                } else {
                    pendingListenerFailure.addSuppressed(failure);
                }
            }
        }
    }

    final void rethrowPendingListenerFailure() {
        Throwable failure;
        synchronized (this) {
            failure = pendingListenerFailure;
            pendingListenerFailure = null;
        }
        if (failure instanceof RuntimeException runtime) {
            throw runtime;
        }
        if (failure instanceof Error error) {
            throw error;
        }
        if (failure != null) {
            throw new IllegalStateException("Window listener failed", failure);
        }
    }

    final void clearEventListeners() {
        clientSizeChangedListeners.clear();
        orientationChangedListeners.clear();
        screenDeviceNameChangedListeners.clear();
    }

    private void invoke(CopyOnWriteArrayList<EventHandler<EventArgs>> listeners) {
        for (EventHandler<EventArgs> listener : listeners) {
            listener.invoke(this, EventArgs.Empty);
        }
    }
}
