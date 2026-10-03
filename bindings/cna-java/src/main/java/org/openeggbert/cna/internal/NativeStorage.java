package org.openeggbert.cna.internal;

import java.nio.charset.StandardCharsets;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/** Internal JNI surface and Game-scoped ownership root for CNA ABI 0.7 Storage. */
public final class NativeStorage {
    private static final Map<Object, Runnable> DEVICES = new LinkedHashMap<>();
    private static final Deque<Boolean> PENDING_DEVICE_EVENTS = new ArrayDeque<>();
    private static Runnable deviceChangedDispatcher;
    private static boolean deviceEventsSubscribed;
    private static volatile boolean acceptEvents = true;

    private NativeStorage() { }

    public static void requireGame(String operation) {
        NativeBindings.currentGameValue(operation);
    }

    public static void beginGameLifetime() {
        synchronized (PENDING_DEVICE_EVENTS) {
            PENDING_DEVICE_EVENTS.clear();
            acceptEvents = true;
        }
    }

    public static long selectDevice(
            int variant, int player, int sizeInBytes, int directoryCount) {
        requireGame("StorageDevice.EndShowSelector");
        long[] output = new long[1];
        check("cna_storage_device_show_selector", nativeSelectDevice(
                variant, player, sizeInBytes, directoryCount, output));
        return handle(output[0], "cna_storage_device_show_selector");
    }

    public static int getDeviceInt(long device) {
        int[] output = new int[1];
        check("cna_storage_device_get_is_connected", nativeGetDeviceInt(device, output));
        return output[0];
    }

    public static long getDeviceLong(long device, int property) {
        long[] output = new long[1];
        check(property == 0 ? "cna_storage_device_get_free_space"
                : "cna_storage_device_get_total_space",
                nativeGetDeviceLong(device, property, output));
        return output[0];
    }

    public static void deleteContainer(long device, String titleName) {
        check("cna_storage_device_delete_container",
                nativeDeleteContainer(device, utf8(titleName)));
    }

    public static long openContainer(long device, String displayName) {
        long[] output = new long[1];
        check("cna_storage_container_open",
                nativeOpenContainer(device, utf8(displayName), output));
        return handle(output[0], "cna_storage_container_open");
    }

    public static String getContainerDisplayName(long container) {
        return string(nativeGetContainerDisplayName(container),
                "cna_storage_container_copy_display_name");
    }

    public static void disposeContainer(long container) {
        check("cna_storage_container_dispose", nativeDisposeContainer(container));
    }

    public static void destroyContainer(long container) {
        check("cna_storage_container_destroy", nativeDestroyContainer(container));
    }

    public static long[] subscribeContainerDisposing(long container, Object target) {
        long[] output = new long[2];
        check("cna_storage_container_subscribe_disposing",
                nativeSubscribeContainerDisposing(container, target, output));
        handle(output[0], "cna_storage_container_subscribe_disposing");
        handle(output[1], "storage disposing JNI context");
        return output;
    }

    public static void unsubscribeContainerDisposing(long registration, long context) {
        check("cna_storage_container_unsubscribe_disposing",
                nativeUnsubscribeContainerDisposing(registration, context));
    }

    public static void pathOperation(long container, int operation, String path) {
        check("cna_storage_container path operation",
                nativePathOperation(container, operation, utf8(path)));
    }

    /**
     * Returns the CNA handle behind a container, for the qualification that asks CNA directly.
     *
     * <p>The projection refuses an escaping path before the JNI call, so a test going through
     * {@code StorageContainer} can never see whether CNA would refuse it too. This is how that
     * question gets asked; nothing in the product uses it.
     */
    public static long containerHandleForQualification(
            Microsoft.Xna.Framework.Storage.StorageContainer container) {
        return FacadeFactory.storageContainerHandle(container);
    }

    public static boolean pathQuery(long container, boolean directory, String path) {
        int[] output = new int[1];
        check("cna_storage_container path query",
                nativePathQuery(container, directory, utf8(path), output));
        return output[0] != 0;
    }

    public static String[] getNames(long container, boolean directories, String pattern) {
        byte[] encoded = utf8(pattern == null ? "" : pattern);
        long[] count = new long[1];
        check("cna_storage_container name count",
                nativeGetNameCount(container, directories, encoded, count));
        if (count[0] < 0L || count[0] > Integer.MAX_VALUE) {
            throw new IllegalStateException("Storage name count exceeds the Java array range");
        }
        String[] names = new String[(int)count[0]];
        for (int index = 0; index < names.length; index++) {
            names[index] = string(nativeGetName(
                    container, directories, encoded, index),
                    "cna_storage_container_copy name");
        }
        return names;
    }

    public static long openStream(
            long container, int variant, String path,
            int fileMode, int fileAccess, int fileShare) {
        long[] output = new long[1];
        check("cna_storage_container open stream", nativeOpenStream(
                container, variant, utf8(path), fileMode, fileAccess, fileShare, output));
        return handle(output[0], "cna_storage_container open stream");
    }

    public static int readStream(long stream, byte[] buffer, int offset, int count) {
        int[] output = new int[1];
        check("cna_storage_stream_read",
                nativeReadStream(stream, buffer, offset, count, output));
        return output[0];
    }

    public static void writeStream(long stream, byte[] buffer, int offset, int count) {
        check("cna_storage_stream_write",
                nativeWriteStream(stream, buffer, offset, count));
    }

    public static long seekStream(long stream, long offset, int origin) {
        long[] output = new long[1];
        check("cna_storage_stream_seek", nativeSeekStream(stream, offset, origin, output));
        return output[0];
    }

    public static long getStreamLong(long stream, int property) {
        long[] output = new long[1];
        check(property == 0 ? "cna_storage_stream_get_position"
                : "cna_storage_stream_get_length",
                nativeGetStreamLong(stream, property, output));
        return output[0];
    }

    public static void setStreamLength(long stream, long length) {
        check("cna_storage_stream_set_length", nativeSetStreamLength(stream, length));
    }

    public static boolean getStreamCapability(long stream, int capability) {
        int[] output = new int[1];
        check("cna_storage_stream capability",
                nativeGetStreamCapability(stream, capability, output));
        return output[0] != 0;
    }

    public static void flushStream(long stream) {
        check("cna_storage_stream_flush", nativeFlushStream(stream));
    }

    public static void closeStream(long stream) {
        check("cna_storage_stream_close", nativeCloseStream(stream));
    }

    public static void destroyDevice(long device) {
        check("cna_storage_device_destroy", nativeDestroyDevice(device));
    }

    public static void registerDevice(Object device, Runnable release) {
        synchronized (DEVICES) { DEVICES.put(device, release); }
    }

    public static void unregisterDevice(Object device) {
        synchronized (DEVICES) { DEVICES.remove(device); }
    }

    public static synchronized void ensureDeviceEvents(Runnable dispatcher) {
        requireGame("StorageDevice.DeviceChanged");
        if (deviceChangedDispatcher == null) deviceChangedDispatcher = dispatcher;
        if (!deviceEventsSubscribed) {
            check("cna_storage_device_subscribe_device_changed", nativeSubscribeDeviceChanged());
            deviceEventsSubscribed = true;
        }
    }

    @SuppressWarnings("unused")
    private static void nativeDeviceChanged() {
        synchronized (PENDING_DEVICE_EVENTS) {
            if (acceptEvents) PENDING_DEVICE_EVENTS.addLast(Boolean.TRUE);
        }
    }

    /** Enqueues the same owner-thread work item as the native process callback for qualification. */
    public static void enqueueDeviceChangedForQualification() {
        nativeDeviceChanged();
    }

    public static void dispatchPendingEvents() {
        while (true) {
            synchronized (PENDING_DEVICE_EVENTS) {
                if (PENDING_DEVICE_EVENTS.pollFirst() == null) return;
            }
            Runnable dispatcher = deviceChangedDispatcher;
            if (dispatcher != null) dispatcher.run();
        }
    }

    public static void closeAllForGameShutdown() {
        acceptEvents = false;
        List<Runnable> snapshot;
        synchronized (DEVICES) { snapshot = new ArrayList<>(DEVICES.values()); }
        Throwable failure = null;
        for (int index = snapshot.size() - 1; index >= 0; index--) {
            try { snapshot.get(index).run(); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        synchronized (PENDING_DEVICE_EVENTS) { PENDING_DEVICE_EVENTS.clear(); }
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        if (failure != null) throw new IllegalStateException("Storage shutdown failed", failure);
    }

    private static byte[] utf8(String value) {
        return value.getBytes(StandardCharsets.UTF_8);
    }

    private static String string(byte[] value, String operation) {
        if (value == null) throw NativeBindings.failure(operation, -1);
        return new String(value, StandardCharsets.UTF_8);
    }

    private static long handle(long value, String operation) {
        if (value == 0L) throw NativeBindings.failure(operation, -1);
        return value;
    }

    private static void check(String operation, int result) {
        if (result != 0) throw NativeBindings.failure(operation, result);
    }

    private static native int nativeSelectDevice(
            int variant, int player, int sizeInBytes, int directoryCount, long[] output);
    private static native int nativeGetDeviceInt(long device, int[] output);
    private static native int nativeGetDeviceLong(long device, int property, long[] output);
    private static native int nativeDeleteContainer(long device, byte[] titleName);
    private static native int nativeDestroyDevice(long device);
    private static native int nativeOpenContainer(long device, byte[] displayName, long[] output);
    private static native byte[] nativeGetContainerDisplayName(long container);
    private static native int nativeDisposeContainer(long container);
    private static native int nativeDestroyContainer(long container);
    private static native int nativeSubscribeContainerDisposing(
            long container, Object target, long[] output);
    private static native int nativeUnsubscribeContainerDisposing(
            long registration, long context);
    private static native int nativePathOperation(long container, int operation, byte[] path);
    private static native int nativePathQuery(
            long container, boolean directory, byte[] path, int[] output);
    private static native int nativeGetNameCount(
            long container, boolean directories, byte[] pattern, long[] output);
    private static native byte[] nativeGetName(
            long container, boolean directories, byte[] pattern, int index);
    private static native int nativeOpenStream(
            long container, int variant, byte[] path, int fileMode,
            int fileAccess, int fileShare, long[] output);
    private static native int nativeReadStream(
            long stream, byte[] buffer, int offset, int count, int[] output);
    private static native int nativeWriteStream(
            long stream, byte[] buffer, int offset, int count);
    private static native int nativeSeekStream(
            long stream, long offset, int origin, long[] output);
    private static native int nativeGetStreamLong(long stream, int property, long[] output);
    private static native int nativeSetStreamLength(long stream, long length);
    private static native int nativeGetStreamCapability(
            long stream, int capability, int[] output);
    private static native int nativeFlushStream(long stream);
    private static native int nativeCloseStream(long stream);
    private static native int nativeSubscribeDeviceChanged();
}
