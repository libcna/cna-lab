package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;

import java.util.LinkedHashMap;
import java.util.Map;

/** Package-private ownership and child-cache core; never appears in the strict public API. */
final class MediaObjectCore {
    private final int kind;
    private final Map<Integer, ChildEntry<?>> children = new LinkedHashMap<>();
    private long handle;

    MediaObjectCore(long handle, int kind) {
        if (handle == 0L) throw new IllegalArgumentException("Media handle must not be zero");
        this.handle = handle;
        this.kind = kind;
    }

    synchronized long value() {
        if (handle == 0L) throw new IllegalStateException("Media object is already closed");
        return handle;
    }

    int kind() {
        return kind;
    }

    synchronized boolean isClosed() {
        return handle == 0L;
    }

    synchronized <T extends AutoCloseable> T child(int relation, ChildFactory<T> factory) {
        return child(relation, relation, factory);
    }

    synchronized <T extends AutoCloseable> T child(
            int cacheKey, int relation, ChildFactory<T> factory) {
        @SuppressWarnings("unchecked")
        ChildEntry<T> cached = (ChildEntry<T>)children.get(cacheKey);
        if (cached != null) return cached.value();
        long child = NativeMedia.getObjectChild(value(), kind, relation);
        if (child == 0L) return null;
        T created = factory.create(child);
        children.put(cacheKey, new ChildEntry<>(created, created));
        return created;
    }

    synchronized <T extends AutoCloseable> T relationship(
            int cacheKey, int relation, ChildFactory<T> factory, ChildReleaser<T> releaser) {
        @SuppressWarnings("unchecked")
        ChildEntry<T> cached = (ChildEntry<T>)children.get(cacheKey);
        if (cached != null) return cached.value();
        long child = NativeMedia.getObjectChild(value(), kind, relation);
        if (child == 0L) return null;
        T created = factory.create(child);
        children.put(cacheKey, new ChildEntry<>(created, () -> releaser.release(created)));
        return created;
    }

    synchronized void close() {
        if (handle == 0L) return;
        Throwable failure = closeChildren();
        if (failure == null) {
            try {
                NativeMedia.closeObject(handle, kind);
                handle = 0L;
            } catch (Throwable exception) {
                failure = exception;
            }
        }
        rethrow(failure);
    }

    synchronized void releaseHandleOnly() {
        if (handle == 0L) return;
        Throwable failure = closeChildren();
        if (failure == null) {
            try {
                NativeMedia.releaseObject(handle, kind);
                handle = 0L;
            } catch (Throwable exception) {
                failure = exception;
            }
        }
        rethrow(failure);
    }

    private Throwable closeChildren() {
        Throwable failure = null;
        AutoCloseable[] values = children.values().stream()
                .map(ChildEntry::cleanup).toArray(AutoCloseable[]::new);
        for (int index = values.length - 1; index >= 0; index--) {
            try { values[index].close(); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        if (failure == null) children.clear();
        return failure;
    }

    private static void rethrow(Throwable failure) {
        if (failure == null) return;
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        throw new IllegalStateException("Failed to release a media object", failure);
    }

    @FunctionalInterface
    interface ChildFactory<T> {
        T create(long handle);
    }

    @FunctionalInterface
    interface ChildReleaser<T> {
        void release(T value);
    }

    private record ChildEntry<T>(T value, AutoCloseable cleanup) {
    }
}
