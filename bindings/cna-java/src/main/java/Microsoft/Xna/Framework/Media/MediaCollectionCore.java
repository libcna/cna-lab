package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.NoSuchElementException;

/** Package-private read-only collection implementation with stable per-index facades. */
final class MediaCollectionCore<T> {
    private final int kind;
    private final ItemFactory<T> factory;
    private final Map<Integer, T> items = new LinkedHashMap<>();
    private long handle;

    MediaCollectionCore(long handle, int kind, ItemFactory<T> factory) {
        if (handle == 0L) throw new IllegalArgumentException("Collection handle must not be zero");
        this.handle = handle;
        this.kind = kind;
        this.factory = factory;
    }

    synchronized long value() {
        if (handle == 0L) throw new IllegalStateException("Media collection is already closed");
        return handle;
    }

    synchronized int count() {
        return NativeMedia.getCollectionCount(value(), kind);
    }

    synchronized boolean isDisposed() {
        return handle == 0L || NativeMedia.getCollectionIsDisposed(handle, kind);
    }

    synchronized T get(int index) {
        int count = count();
        if (index < 0 || index >= count) {
            throw new IndexOutOfBoundsException("index " + index + " is outside [0, " + count + ")");
        }
        T cached = items.get(index);
        if (cached != null) return cached;
        T created = factory.create(NativeMedia.getCollectionAt(handle, kind, index));
        items.put(index, created);
        return created;
    }

    Iterator<T> iterator() {
        int upperBound = count();
        return new Iterator<>() {
            private int index;
            @Override public boolean hasNext() { return index < upperBound; }
            @Override public T next() {
                if (!hasNext()) throw new NoSuchElementException();
                return get(index++);
            }
        };
    }

    synchronized void close() {
        if (handle == 0L) return;
        Throwable failure = releaseItems();
        if (failure == null) {
            try {
                NativeMedia.closeCollection(handle, kind);
                handle = 0L;
            } catch (Throwable exception) {
                failure = exception;
            }
        }
        MediaObjectCoreFailure.rethrow(failure, "media collection");
    }

    synchronized void releaseHandleOnly() {
        if (handle == 0L) return;
        Throwable failure = releaseItems();
        if (failure == null) {
            try {
                NativeMedia.releaseCollection(handle, kind);
                handle = 0L;
            } catch (Throwable exception) { failure = exception; }
        }
        MediaObjectCoreFailure.rethrow(failure, "media collection");
    }

    private Throwable releaseItems() {
        Throwable failure = null;
        Object[] values = items.values().toArray();
        for (int index = values.length - 1; index >= 0; index--) {
            try { factory.release(values[index]); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        if (failure == null) items.clear();
        return failure;
    }

    interface ItemFactory<T> {
        T create(long handle);
        void release(Object value);
    }
}
