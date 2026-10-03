package org.openeggbert.cna.internal;

import System.AsyncCallback;
import System.IAsyncResult;

import java.util.Objects;
import java.util.function.Supplier;

/**
 * The shared {@code Begin}/{@code End} carrier for operations CNA completes synchronously.
 *
 * <p>XNA's gamer-services and networking APIs are written in the CLR's asynchronous
 * programming model, but the operation behind each one completes before {@code Begin}
 * returns. Reporting that honestly is what {@code CompletedSynchronously} is for, so this
 * carrier runs the work in {@code Begin}, invokes the callback before returning, and hands the
 * stored outcome back from {@code End}.
 *
 * <p>A failure is captured and rethrown from {@code End}, which is the CLR's contract: the
 * exception a caller would have seen from the synchronous method surfaces at {@code End}, not
 * inside the callback.
 *
 * <p>This class is not application API.
 */
public final class CompletedAsyncResult<T> implements IAsyncResult {

    private final Object state;
    private final T value;
    private final RuntimeException failure;
    private boolean ended;

    private CompletedAsyncResult(Object state, T value, RuntimeException failure) {
        this.state = state;
        this.value = value;
        this.failure = failure;
    }

    /** Runs the operation, notifies the callback, and returns its completed result. */
    public static <T> IAsyncResult begin(
            AsyncCallback callback, Object state, Supplier<T> operation) {
        T value = null;
        RuntimeException failure = null;
        try {
            value = operation.get();
        } catch (RuntimeException exception) {
            failure = exception;
        }
        CompletedAsyncResult<T> result = new CompletedAsyncResult<>(state, value, failure);
        if (callback != null) {
            callback.invoke(result);
        }
        return result;
    }

    /** Runs an operation with no result, notifies the callback, and returns it completed. */
    public static IAsyncResult begin(AsyncCallback callback, Object state, Runnable operation) {
        return begin(callback, state, () -> {
            operation.run();
            return null;
        });
    }

    /**
     * Completes one {@code Begin} exactly once and returns its value.
     *
     * @throws NullPointerException when the result did not come from the matching {@code Begin}
     * @throws IllegalStateException when the same result is ended twice, which the CLR forbids
     */
    public static <T> T end(IAsyncResult result, Class<T> type) {
        Objects.requireNonNull(type, "type");
        if (!(result instanceof CompletedAsyncResult<?> completed)) {
            throw new NullPointerException("result");
        }
        Object value = completed.finish();
        return value == null ? null : type.cast(value);
    }

    /** Completes one {@code Begin} that produced no value. */
    public static void endVoid(IAsyncResult result) {
        if (!(result instanceof CompletedAsyncResult<?> completed)) {
            throw new NullPointerException("result");
        }
        completed.finish();
    }

    @Override
    public Object getAsyncState() {
        return state;
    }

    @Override
    public boolean getCompletedSynchronously() {
        return true;
    }

    @Override
    public boolean getIsCompleted() {
        return true;
    }

    private synchronized Object finish() {
        if (ended) {
            throw new IllegalStateException("End cannot be called twice on one IAsyncResult");
        }
        ended = true;
        if (failure != null) {
            throw failure;
        }
        return value;
    }
}
