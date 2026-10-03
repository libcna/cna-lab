package org.openeggbert.cna.internal;

import java.lang.ref.Cleaner;
import java.util.ArrayDeque;
import java.util.Deque;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.function.LongToIntFunction;

/**
 * Releases a thread-affine native handle whose Java owner is not disposable.
 *
 * <p>Some XNA types own a CNA handle and are not {@code IDisposable}: a game writes
 * {@code new NetworkSessionProperties()} and never closes it, because XNA gave it nothing to
 * close. The handle still has to go back, and there is exactly one place it can go back from.
 *
 * <p><strong>A {@link Cleaner} alone cannot do it.</strong> CNA refuses the release from any
 * thread but the one that created the handle -- measured, not assumed:
 * {@code cna_network_session_properties_destroy} answers {@code CNA_RESULT_THREAD} off-thread --
 * and a cleaner runs on its own thread. So the cleaner never calls CNA. It only records the
 * handle on the queue belonging to the thread that created it, and that thread releases it the
 * next time it pumps the framework dispatcher, which is where this binding already does its
 * deferred native work.
 *
 * <p>The consequence is worth stating plainly: a handle whose owning thread never pumps again is
 * never released. That is a leak bounded by the life of that thread, and it is the honest
 * trade against calling CNA from a thread it refuses.
 *
 * <p>This class is not application API.
 */
public final class NativeDeferredRelease {

    private static final Cleaner CLEANER = Cleaner.create();

    /** One queue per owning thread, so a drain only releases what its own thread created. */
    private static final Map<Long, Deque<Pending>> PENDING = new HashMap<>();

    private NativeDeferredRelease() {
    }

    /** One handle waiting for its owning thread to come back and release it. */
    private record Pending(long handle, LongToIntFunction releaser, String operation) {
    }

    /**
     * Arranges for a handle to be released on this thread once its owner is unreachable.
     *
     * @param owner the Java object whose lifetime the handle follows; never referenced by the
     *     cleaning action, or it could never become unreachable
     * @param handle the native handle
     * @param releaser the CNA release route, which returns a CNA result
     * @param operation what to call the release in a diagnostic
     */
    public static void onOwningThread(
            Object owner, long handle, LongToIntFunction releaser, String operation) {
        Objects.requireNonNull(owner, "owner");
        Objects.requireNonNull(releaser, "releaser");
        Objects.requireNonNull(operation, "operation");
        if (handle == 0L) {
            return;
        }
        long thread = Thread.currentThread().getId();
        // Everything the action touches is captured by value. Capturing `owner` would make the
        // object reachable from its own cleaner and it would never be collected.
        CLEANER.register(owner, () -> enqueue(thread, new Pending(handle, releaser, operation)));
    }

    /**
     * Releases every handle this thread owns whose Java object has been collected.
     *
     * <p>Called from the framework-dispatcher pump, so it runs on the game thread during
     * {@code Update} -- the same place every other deferred native action in this binding runs.
     * A release that fails is reported once the queue is empty, so one bad handle does not strand
     * the rest.
     */
    public static void drain() {
        long thread = Thread.currentThread().getId();
        RuntimeException failure = null;
        while (true) {
            Pending pending;
            synchronized (PENDING) {
                Deque<Pending> queue = PENDING.get(thread);
                pending = queue == null ? null : queue.poll();
                if (pending == null) {
                    PENDING.remove(thread);
                    break;
                }
            }
            try {
                int result = pending.releaser().applyAsInt(pending.handle());
                if (result != 0) {
                    throw NativeBindings.failure(pending.operation(), result);
                }
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        }
        if (failure != null) {
            throw failure;
        }
    }

    /** Returns how many handles this thread still owes CNA, for a test to assert against. */
    public static int pendingCount() {
        synchronized (PENDING) {
            Deque<Pending> queue = PENDING.get(Thread.currentThread().getId());
            return queue == null ? 0 : queue.size();
        }
    }

    private static void enqueue(long thread, Pending pending) {
        synchronized (PENDING) {
            PENDING.computeIfAbsent(thread, ignored -> new ArrayDeque<>()).add(pending);
        }
    }
}
