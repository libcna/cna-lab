package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.generated.NativeSensorExtensionRoutes;

/**
 * The host's accelerometer, reporting acceleration per axis in g.
 *
 * <p>A CNA extension. A device at rest reads about one g along whichever axis points down, so a
 * reading of exactly zero on every axis is almost always the default rather than a measurement;
 * {@link #getIsDataValid()} is what tells them apart.
 */
public final class Accelerometer extends Sensor<AccelerometerReading> {

    private static final Routes ROUTES = new Routes() {

        @Override
        public int state(long sensor, int[] outState) {
            return NativeSensorExtensionRoutes.accelerometerGetState(sensor, outState);
        }

        @Override
        public int start(long sensor) {
            return NativeSensorExtensionRoutes.accelerometerStart(sensor);
        }

        @Override
        public int stop(long sensor) {
            return NativeSensorExtensionRoutes.accelerometerStop(sensor);
        }

        @Override
        public int dataValid(long sensor, boolean[] outValid) {
            return NativeSensorExtensionRoutes.accelerometerGetIsDataValid(sensor, outValid);
        }

        @Override
        public int updateTicks(long sensor, long[] outTicks) {
            return NativeSensorExtensionRoutes
                    .accelerometerGetTimeBetweenUpdatesTicks(sensor, outTicks);
        }

        @Override
        public int setUpdateTicks(long sensor, long ticks) {
            return NativeSensorExtensionRoutes
                    .accelerometerSetTimeBetweenUpdatesTicks(sensor, ticks);
        }

        @Override
        public int dispose(long sensor) {
            return NativeSensorExtensionRoutes.accelerometerDispose(sensor);
        }

        @Override
        public int destroy(long sensor) {
            return NativeSensorExtensionRoutes.accelerometerDestroy(sensor);
        }

        @Override
        public int currentValueKind() {
            return org.openeggbert.cna.internal.NativeBindings.SENSOR_ACCELEROMETER_CURRENT;
        }
    };

    private Accelerometer(long handle) {
        super("Accelerometer", ROUTES, handle);
    }

    /**
     * Reports whether this host has an accelerometer.
     *
     * <p>{@code false} on a machine without one is an ordinary answer, and the usual one on a
     * desktop.
     */
    public static boolean getIsSupported() {
        boolean[] supported = new boolean[1];
        SensorExtension.check("Accelerometer.getIsSupported",
                NativeSensorExtensionRoutes.accelerometerGetIsSupported(
                        SensorExtension.game("Accelerometer"), supported));
        return supported[0];
    }

    /** Creates an accelerometer. Creation succeeds even where the host has no such sensor. */
    public static Accelerometer Create() {
        long[] sensor = new long[1];
        SensorExtension.check("Accelerometer.Create",
                NativeSensorExtensionRoutes.accelerometerCreate(
                        SensorExtension.game("Accelerometer"), sensor));
        return new Accelerometer(sensor[0]);
    }

    /**
     * Returns the most recent sample.
     *
     * @throws IllegalStateException when the host has no accelerometer, which CNA refuses rather
     *     than answering with a default, or when this sensor is closed
     */
    @Override
    public AccelerometerReading getCurrentValue() {
        long[] integral = new long[2];
        float[] floating = new float[3];
        check("getCurrentValue", NativeSensorExtensionRoutes
                .accelerometerGetCurrentValue(open(), integral, floating));
        return new AccelerometerReading(
                SensorExtension.timestamp(integral[0], integral[1]),
                new Vector3(floating[0], floating[1], floating[2]));
    }

    @Override
    AccelerometerReading readingOf(double[] leaves) {
        return new AccelerometerReading(
                SensorExtension.timestamp((long) leaves[0], (long) leaves[1]),
                new Vector3((float) leaves[2], (float) leaves[3], (float) leaves[4]));
    }

    /**
     * Feeds the sensor a synthetic reading, in the host's own units.
     *
     * <p><strong>The units are the platform's and the reading that comes back is canonical.</strong>
     * That is the header's contract and it is measured rather than assumed: injecting 9.80665
     * metres per second squared into an accelerometer reads back as one g.
     *
     * <p>This is CNA's canonical test-support injector, and it exists because a game's own event
     * wiring has to be exercisable on a machine with no sensor -- which is every desktop. A
     * sensor only delivers it once acquisition has started, which on such a machine means
     * {@link SensorTestBackends}.
     *
     * @param x the first axis, in platform units
     * @param y the second axis
     * @param z the third axis
     */
    public void injectSyntheticUpdate(float x, float y, float z) {
        check("injectSyntheticUpdate", NativeSensorExtensionRoutes
                .accelerometerInjectSyntheticUpdateExt(open(), x, y, z));
    }

    /**
     * Calls a handler with every reading, through the obsolete event rather than the current one.
     *
     * <p>The canonical event this projects is superseded by
     * {@link #addCurrentValueChangedListener}, which is why it delivers a description of its own
     * rather than a reading. Both are raised for the same reading and the order is fixed: the
     * current-value handlers run first and this one second. Unlike the current-value event,
     * <strong>this one is raised only when the reading is valid</strong>.
     *
     * @param handler what to do with each reading
     * @return the subscription, which the caller closes
     */
    public SensorSubscription addReadingChangedListener(
            java.util.function.Consumer<AccelerometerReading> handler) {
        java.util.Objects.requireNonNull(handler, "handler");
        return subscribe(org.openeggbert.cna.internal.NativeBindings.SENSOR_ACCELEROMETER_READING,
                leaves -> handler.accept(readingOf(leaves)));
    }
}
