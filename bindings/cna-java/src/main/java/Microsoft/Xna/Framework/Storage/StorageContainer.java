package Microsoft.Xna.Framework.Storage;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import System.IO.FileAccess;
import System.IO.FileMode;
import System.IO.FileShare;
import System.IO.Stream;
import org.openeggbert.cna.internal.NativeStorage;
import org.openeggbert.cna.internal.NativeStorageStream;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Named isolated storage container with owned native streams. */
public class StorageContainer implements AutoCloseable {
    private final StorageDevice storageDevice;
    private final List<NativeStorageStream> streams = new ArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposingListeners =
            new CopyOnWriteArrayList<>();
    private long handle;
    private long disposingRegistration;
    private long disposingContext;
    private boolean disposed;
    private boolean eventDelivered;
    private volatile boolean nativeDisposingObserved;

    StorageContainer(long handle, StorageDevice storageDevice) {
        if (handle == 0L) throw new IllegalArgumentException("Storage container handle is zero");
        this.handle = handle;
        this.storageDevice = Objects.requireNonNull(storageDevice, "storageDevice");
    }

    public final void addDisposingListener(EventHandler<EventArgs> listener) {
        EventHandler<EventArgs> selected = Objects.requireNonNull(listener, "listener");
        synchronized (this) {
            if (!disposed && disposingRegistration == 0L) {
                long[] registration = NativeStorage.subscribeContainerDisposing(requireHandle(), this);
                disposingRegistration = registration[0];
                disposingContext = registration[1];
            }
            disposingListeners.add(selected);
        }
    }

    public final void removeDisposingListener(EventHandler<EventArgs> listener) {
        disposingListeners.remove(listener);
    }

    public final String getDisplayName() {
        return NativeStorage.getContainerDisplayName(requireOpen());
    }

    public final boolean getIsDisposed() { return disposed; }

    public final StorageDevice getStorageDevice() {
        requireOpen();
        return storageDevice;
    }

    public final void CreateDirectory(String directory) {
        NativeStorage.pathOperation(requireOpen(), 0, validatePath(directory, "directory"));
    }

    public final Stream CreateFile(String file) {
        return stream(NativeStorage.openStream(requireOpen(), 0,
                validatePath(file, "file"), 0, 0, 0));
    }

    public final void DeleteDirectory(String directory) {
        NativeStorage.pathOperation(requireOpen(), 1, validatePath(directory, "directory"));
    }

    public final void DeleteFile(String file) {
        NativeStorage.pathOperation(requireOpen(), 2, validatePath(file, "file"));
    }

    public final boolean DirectoryExists(String directory) {
        return NativeStorage.pathQuery(requireOpen(), true,
                validatePath(directory, "directory"));
    }

    public final boolean FileExists(String file) {
        return NativeStorage.pathQuery(requireOpen(), false,
                validatePath(file, "file"));
    }

    public final String[] GetDirectoryNames() { return GetDirectoryNames(null); }
    public final String[] GetDirectoryNames(String searchPattern) {
        return NativeStorage.getNames(requireOpen(), true, searchPattern);
    }
    public final String[] GetFileNames() { return GetFileNames(null); }
    public final String[] GetFileNames(String searchPattern) {
        return NativeStorage.getNames(requireOpen(), false, searchPattern);
    }

    public final Stream OpenFile(String file, FileMode fileMode) {
        return stream(NativeStorage.openStream(requireOpen(), 1,
                validatePath(file, "file"),
                Objects.requireNonNull(fileMode, "fileMode").getValue(), 0, 0));
    }

    public final Stream OpenFile(String file, FileMode fileMode, FileAccess fileAccess) {
        return stream(NativeStorage.openStream(requireOpen(), 2,
                validatePath(file, "file"),
                Objects.requireNonNull(fileMode, "fileMode").getValue(),
                Objects.requireNonNull(fileAccess, "fileAccess").getValue(), 0));
    }

    public final Stream OpenFile(
            String file, FileMode fileMode, FileAccess fileAccess, FileShare fileShare) {
        return stream(NativeStorage.openStream(requireOpen(), 3,
                validatePath(file, "file"),
                Objects.requireNonNull(fileMode, "fileMode").getValue(),
                Objects.requireNonNull(fileAccess, "fileAccess").getValue(),
                Objects.requireNonNull(fileShare, "fileShare").getValue()));
    }

    public final void Dispose() {
        Throwable failure = closeStreams();
        if (failure != null) StorageDevice.rethrow(failure, "StorageContainer streams");

        synchronized (this) {
            if (handle == 0L) return;
            if (!disposed) {
                NativeStorage.disposeContainer(handle);
                disposed = true;
            }
        }

        if (!eventDelivered) {
            // XNA marks disposal before raising Disposing. Mark the Java delivery one-shot before
            // invoking handlers as well, so a handler may safely call close() recursively.
            eventDelivered = true;
            Throwable listenerFailure = null;
            for (EventHandler<EventArgs> listener : disposingListeners) {
                try { listener.invoke(this, EventArgs.Empty); }
                catch (Throwable exception) {
                    listenerFailure = append(listenerFailure, exception);
                }
            }
            failure = append(failure, listenerFailure);
        }

        synchronized (this) {
            if (disposingRegistration != 0L) {
                try {
                    NativeStorage.unsubscribeContainerDisposing(
                            disposingRegistration, disposingContext);
                    disposingRegistration = 0L;
                    disposingContext = 0L;
                } catch (Throwable exception) { failure = append(failure, exception); }
            }
            if (disposingRegistration == 0L && handle != 0L) {
                try {
                    NativeStorage.destroyContainer(handle);
                    handle = 0L;
                    storageDevice.removeContainer(this);
                    disposingListeners.clear();
                } catch (Throwable exception) { failure = append(failure, exception); }
            }
        }
        StorageDevice.rethrow(failure, "StorageContainer.close");
    }

    @Override
    public final void close() {
        Dispose();
    }

    /** JNI callback target: records native delivery without invoking user code in the C frame. */
    @SuppressWarnings("unused")
    private void nativeDisposingObserved() { nativeDisposingObserved = true; }

    boolean nativeDisposingWasObserved() { return nativeDisposingObserved; }

    private synchronized long requireHandle() {
        if (handle == 0L) throw new IllegalStateException("StorageContainer is already closed");
        return handle;
    }

    private synchronized long requireOpen() {
        if (disposed || handle == 0L) {
            throw new IllegalStateException("StorageContainer is already disposed");
        }
        return handle;
    }

    private synchronized Stream stream(long nativeHandle) {
        NativeStorageStream created = new NativeStorageStream(nativeHandle,
                () -> removeStreamByHandle(nativeHandle));
        streams.add(created);
        return created;
    }

    private synchronized void removeStreamByHandle(long nativeHandle) {
        streams.removeIf(value -> value.matchesReleasedHandle(nativeHandle));
    }

    private Throwable closeStreams() {
        NativeStorageStream[] snapshot;
        synchronized (this) { snapshot = streams.toArray(NativeStorageStream[]::new); }
        Throwable failure = null;
        for (int index = snapshot.length - 1; index >= 0; index--) {
            try { snapshot[index].close(); }
            catch (Throwable exception) { failure = append(failure, exception); }
        }
        return failure;
    }

    private static String validatePath(String value, String argument) {
        String selected = Objects.requireNonNull(value, argument);
        if (selected.isEmpty()) throw new NullPointerException(argument);
        String portable = selected.replace('\\', '/');
        if (portable.startsWith("/")
                || (portable.length() >= 2
                && Character.isLetter(portable.charAt(0))
                && portable.charAt(1) == ':')) {
            throw new IllegalArgumentException(argument + " escapes the storage container");
        }
        int depth = 0;
        for (String component : portable.split("/", -1)) {
            if (component.isEmpty() || component.equals(".")) continue;
            if (component.equals("..")) {
                if (depth == 0) {
                    throw new IllegalArgumentException(
                            argument + " escapes the storage container");
                }
                depth--;
            } else {
                depth++;
            }
        }
        return selected;
    }

    private static Throwable append(Throwable current, Throwable added) {
        if (added == null) return current;
        if (current == null) return added;
        current.addSuppressed(added);
        return current;
    }
}
