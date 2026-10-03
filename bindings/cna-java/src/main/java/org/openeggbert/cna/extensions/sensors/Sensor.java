package org.openeggbert.cna.extensions.sensors;

import org.openeggbert.cna.internal.NativeBindings;

import java.time.Duration;
import java.util.Objects;
import java.util.function.Consumer;

/**
 * A host motion sensor: what every sensor family in this package has in common.
 *
 * <p>A CNA extension. A sensor is created even on a host that has none, which is CNA's own
 * contract: creation succeeds and {@link #getState()} says whether the hardware is there. Asking
 * an unsupported sensor for a reading is refused rather than answered with zeros, so a caller
 * that wants a number without a state check should ask its {@code getIsSupported()} first.
 *
 * <p>The handle is <strong>owned</strong>. {@link #close()} releases it, and CNA disposes the
 * sensor as part of releasing it. Closing twice is a no-op here even though CNA refuses a second
 * disposal, because {@link AutoCloseable} requires it; the second call never reaches CNA.
 *
 * <p>A sensor belongs to the game that created it and is created, read and closed on that game's
 * thread, which is the thread ownership CNA's game handle already imposes.
 *
 * @param <TReading> the sample type this sensor produces
 */
public abstract class Sensor<TReading> implements AutoCloseable {

    /** The dispatch a concrete sensor supplies, so the shared members reach its own routes. */
    interface Routes {

        int state(long sensor, int[] outState);

        int start(long sensor);

        int stop(long sensor);

        int dataValid(long sensor, boolean[] outValid);

        int updateTicks(long sensor, long[] outTicks);

        int setUpdateTicks(long sensor, long ticks);

        int dispose(long sensor);

        int destroy(long sensor);

        /**
         * Which of the adapter's subscription kinds delivers this sensor's readings.
         *
         * <p>Seven routes with six different callback types cannot be selected by a handle, so
         * the kind is the adapter's own constant rather than anything CNA names.
         */
        int currentValueKind();
    }

    private final String owner;
    private final Routes routes;
    private final long handle;
    private boolean closed;

    Sensor(String owner, Routes routes, long handle) {
        this.owner = owner;
        this.routes = routes;
        this.handle = handle;
    }

    /** Returns the most recent sample. */
    public abstract TReading getCurrentValue();

    /** Returns what the sensor is currently doing, including whether the host has it at all. */
    public final SensorState getState() {
        int[] state = new int[1];
        check("getState", routes.state(open(), state));
        return SensorState.values()[state[0]];
    }

    /** Starts acquisition. Starting an already started sensor is refused, as CNA refuses it. */
    public final void Start() {
        check("Start", routes.start(open()));
    }

    /** Stops acquisition. */
    public final void Stop() {
        check("Stop", routes.stop(open()));
    }

    /**
     * Reports whether the last reading is a real measurement.
     *
     * <p>This is what tells a genuine zero from the zeroed default a supported sensor answers
     * before it has produced anything.
     */
    public final boolean getIsDataValid() {
        boolean[] valid = new boolean[1];
        check("getIsDataValid", routes.dataValid(open(), valid));
        return valid[0];
    }

    /** Returns the interval the host was asked to sample at. */
    public final Duration getTimeBetweenUpdates() {
        long[] ticks = new long[1];
        check("getTimeBetweenUpdates", routes.updateTicks(open(), ticks));
        return Duration.ofNanos(Math.multiplyExact(
                ticks[0], SensorExtension.NANOSECONDS_PER_TICK));
    }

    /**
     * Asks the host to sample at this interval.
     *
     * <p>The host is free to sample slower or faster; this is a request, and the interval read
     * back is what CNA recorded, not what the hardware achieved.
     *
     * @param interval the requested sampling interval, never negative
     */
    public final void setTimeBetweenUpdates(Duration interval) {
        Objects.requireNonNull(interval, "interval");
        if (interval.isNegative()) {
            throw new IllegalArgumentException("interval must not be negative: " + interval);
        }
        check("setTimeBetweenUpdates", routes.setUpdateTicks(open(),
                interval.toNanos() / SensorExtension.NANOSECONDS_PER_TICK));
    }

    /**
     * Reads one reading out of the flattened leaves a subscription delivers.
     *
     * <p>A callback's reading crosses as one {@code double[]} of its leaves in declaration order:
     * a {@code float} widens to a {@code double} without loss and a {@code double} is already
     * one, so one array costs nothing in precision and saves six near-identical trampolines from
     * becoming twelve.
     *
     * @param leaves the reading's leaves, in the order the header declares them
     * @return the reading
     */
    abstract TReading readingOf(double[] leaves);

    /**
     * Calls a handler with every reading this sensor produces, until the subscription is closed.
     *
     * <p><strong>The handler runs on whatever thread CNA raises the event on</strong>, which for
     * a host sensor is the platform's own and for an injected reading is the injecting thread. A
     * handler that touches game state should hand the work to the game rather than doing it where
     * it lands.
     *
     * <p>The returned subscription is what keeps the handler alive; closing it detaches from the
     * sensor and releases the handler. <strong>Close it before the sensor</strong>: CNA's
     * registration detaches from the sensor it was made for, which must still exist.
     *
     * @param handler what to do with each reading
     * @return the subscription, which the caller closes
     */
    public final SensorSubscription addCurrentValueChangedListener(Consumer<TReading> handler) {
        Objects.requireNonNull(handler, "handler");
        return subscribe(routes.currentValueKind(),
                leaves -> handler.accept(readingOf(leaves)));
    }

    /**
     * Stops acquisition without releasing the handle.
     *
     * <p>The canonical disposal, and not {@link #close()}: this leaves the object in hand and
     * every route on it answering {@code INVALID_STATE} afterwards. <strong>It is not
     * idempotent</strong> -- a second call is refused, which is CNA's behaviour and is measured
     * rather than smoothed over.
     */
    public final void dispose() {
        check("dispose", routes.dispose(open()));
    }

    /** Registers one handler over the flattened leaves, for this sensor's own kind. */
    final SensorSubscription subscribe(int kind, Consumer<double[]> handler) {
        long sensor = open();
        long token = NativeBindings.newCallbackToken(handler);
        long[] registration = new long[1];
        int result = NativeBindings.sensorSubscribe(kind, sensor, token, registration);
        if (result != 0) {
            // The subscription was refused, so nothing will ever call the handler and the token
            // has no other owner.
            NativeBindings.releaseCallbackToken(token);
            check("subscribe", result);
        }
        return new SensorSubscription(owner, registration[0], token);
    }

    /** Releases the sensor. CNA disposes it as part of releasing it; closing twice is a no-op. */
    @Override
    public final void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        check("close", routes.destroy(handle));
    }

    /** Returns the owned handle, refusing to use it after the sensor is closed. */
    final long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This " + owner + " is closed");
            }
        }
        return handle;
    }

    final void check(String operation, int result) {
        SensorExtension.check(owner + "." + operation, result);
    }
}
