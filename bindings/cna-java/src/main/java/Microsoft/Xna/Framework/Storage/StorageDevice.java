package Microsoft.Xna.Framework.Storage;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import Microsoft.Xna.Framework.PlayerIndex;
import System.AsyncCallback;
import System.IAsyncResult;
import org.openeggbert.cna.internal.NativeStorage;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** XNA storage-device selection facade over CNA's process storage service. */
public final class StorageDevice {
    private static final CopyOnWriteArrayList<EventHandler<EventArgs>> DEVICE_LISTENERS =
            new CopyOnWriteArrayList<>();

    private final List<StorageContainer> containers = new ArrayList<>();
    private long handle;

    private StorageDevice(long handle) {
        if (handle == 0L) throw new StorageDeviceNotConnectedException();
        this.handle = handle;
        NativeStorage.registerDevice(this, this::releaseForGameShutdown);
    }

    public static void addDeviceChangedListener(EventHandler<EventArgs> listener) {
        EventHandler<EventArgs> selected = Objects.requireNonNull(listener, "listener");
        NativeStorage.ensureDeviceEvents(StorageDevice::dispatchDeviceChanged);
        DEVICE_LISTENERS.add(selected);
    }

    public static void removeDeviceChangedListener(EventHandler<EventArgs> listener) {
        DEVICE_LISTENERS.remove(listener);
    }

    public static IAsyncResult BeginShowSelector(
            AsyncCallback callback, Object state) {
        return beginSelector(-1, 0, 1, callback, state, 0);
    }

    public static IAsyncResult BeginShowSelector(
            PlayerIndex player, AsyncCallback callback, Object state) {
        return beginSelector(Objects.requireNonNull(player, "player").ordinal(),
                0, 1, callback, state, 1);
    }

    public static IAsyncResult BeginShowSelector(
            int sizeInBytes, int directoryCount, AsyncCallback callback, Object state) {
        return beginSelector(-1, sizeInBytes, directoryCount, callback, state, 2);
    }

    public static IAsyncResult BeginShowSelector(
            PlayerIndex player, int sizeInBytes, int directoryCount,
            AsyncCallback callback, Object state) {
        return beginSelector(Objects.requireNonNull(player, "player").ordinal(),
                sizeInBytes, directoryCount, callback, state, 3);
    }

    public static StorageDevice EndShowSelector(IAsyncResult result) {
        CompletedResult<?> completed = completed(result, ResultKind.SELECTOR, null);
        return (StorageDevice)completed.finish();
    }

    public final IAsyncResult BeginOpenContainer(
            String displayName, AsyncCallback callback, Object state) {
        requireHandle();
        CompletedResult<StorageContainer> result = new CompletedResult<>(
                ResultKind.CONTAINER, this, state,
                () -> openContainer(displayName));
        if (callback != null) callback.invoke(result);
        return result;
    }

    public final StorageContainer EndOpenContainer(IAsyncResult result) {
        CompletedResult<?> completed = completed(result, ResultKind.CONTAINER, this);
        return (StorageContainer)completed.finish();
    }

    public final void DeleteContainer(String titleName) {
        NativeStorage.deleteContainer(requireHandle(), Objects.requireNonNull(titleName, "titleName"));
    }

    public final long getFreeSpace() { return NativeStorage.getDeviceLong(requireHandle(), 0); }
    public final boolean getIsConnected() {
        return NativeStorage.getDeviceInt(requireHandle()) != 0;
    }
    public final long getTotalSpace() { return NativeStorage.getDeviceLong(requireHandle(), 1); }

    synchronized long requireHandle() {
        if (handle == 0L) {
            throw new StorageDeviceNotConnectedException("The storage device is not connected");
        }
        return handle;
    }

    synchronized void removeContainer(StorageContainer container) {
        containers.remove(container);
    }

    private static IAsyncResult beginSelector(
            int player, int sizeInBytes, int directoryCount,
            AsyncCallback callback, Object state, int variant) {
        if (sizeInBytes < 0) throw new IllegalArgumentException("sizeInBytes");
        NativeStorage.requireGame("StorageDevice.BeginShowSelector");
        CompletedResult<StorageDevice> result = new CompletedResult<>(
                ResultKind.SELECTOR, null, state,
                () -> new StorageDevice(NativeStorage.selectDevice(
                        variant, player, sizeInBytes, Math.max(0, directoryCount))));
        if (callback != null) callback.invoke(result);
        return result;
    }

    private synchronized StorageContainer openContainer(String displayName) {
        String selectedName = Objects.requireNonNull(displayName, "displayName");
        if (selectedName.isEmpty()) throw new NullPointerException("displayName");
        long nativeContainer = NativeStorage.openContainer(
                requireHandle(), selectedName);
        StorageContainer created = new StorageContainer(nativeContainer, this);
        containers.add(created);
        return created;
    }

    private void releaseForGameShutdown() {
        StorageContainer[] snapshot;
        synchronized (this) {
            if (handle == 0L) return;
            snapshot = containers.toArray(StorageContainer[]::new);
        }
        Throwable failure = null;
        for (int index = snapshot.length - 1; index >= 0; index--) {
            try { snapshot[index].close(); }
            catch (Throwable exception) { failure = append(failure, exception); }
        }
        synchronized (this) {
            if (containers.isEmpty()) {
                try {
                    NativeStorage.destroyDevice(handle);
                    handle = 0L;
                    NativeStorage.unregisterDevice(this);
                } catch (Throwable exception) { failure = append(failure, exception); }
            }
        }
        rethrow(failure, "StorageDevice shutdown");
    }

    private static void dispatchDeviceChanged() {
        Throwable failure = null;
        for (EventHandler<EventArgs> listener : DEVICE_LISTENERS) {
            try { listener.invoke(null, EventArgs.Empty); }
            catch (Throwable exception) { failure = append(failure, exception); }
        }
        rethrow(failure, "StorageDevice.DeviceChanged");
    }

    private static CompletedResult<?> completed(
            IAsyncResult result, ResultKind kind, StorageDevice owner) {
        if (!(result instanceof CompletedResult<?> completed)
                || completed.kind != kind || completed.owner != owner) {
            throw new NullPointerException("result");
        }
        return completed;
    }

    private static Throwable append(Throwable current, Throwable added) {
        if (current == null) return added;
        current.addSuppressed(added);
        return current;
    }

    static void rethrow(Throwable failure, String operation) {
        if (failure == null) return;
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        throw new IllegalStateException(operation + " failed", failure);
    }

    private enum ResultKind { SELECTOR, CONTAINER }

    @FunctionalInterface
    private interface Operation<T> { T execute(); }

    private static final class CompletedResult<T> implements IAsyncResult {
        private final ResultKind kind;
        private final StorageDevice owner;
        private final Object state;
        private final Operation<T> operation;
        private boolean ended;

        CompletedResult(ResultKind kind, StorageDevice owner, Object state, Operation<T> operation) {
            this.kind = kind;
            this.owner = owner;
            this.state = state;
            this.operation = operation;
        }

        @Override public Object getAsyncState() { return state; }
        @Override public boolean getCompletedSynchronously() { return true; }
        @Override public boolean getIsCompleted() { return true; }

        synchronized T finish() {
            if (ended) throw new IllegalStateException("End cannot be called twice");
            ended = true;
            return operation.execute();
        }
    }
}
