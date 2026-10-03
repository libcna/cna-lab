package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.generated.NativeSensorExtensionRoutes;

/**
 * The host's compass: magnetic and true heading in degrees, plus the raw magnetometer vector.
 *
 * <p>A CNA extension. The headings are doubles across the whole path, because that is what CNA
 * reports and narrowing them at the boundary would discard precision the host measured.
 */
public final class Compass extends Sensor<CompassReading> {

    private static final Routes ROUTES = new Routes() {

        @Override
        public int state(long sensor, int[] outState) {
            return NativeSensorExtensionRoutes.compassGetState(sensor, outState);
        }

        @Override
        public int start(long sensor) {
            return NativeSensorExtensionRoutes.compassStart(sensor);
        }

        @Override
        public int stop(long sensor) {
            return NativeSensorExtensionRoutes.compassStop(sensor);
        }

        @Override
        public int dataValid(long sensor, boolean[] outValid) {
            return NativeSensorExtensionRoutes.compassGetIsDataValid(sensor, outValid);
        }

        @Override
        public int updateTicks(long sensor, long[] outTicks) {
            return NativeSensorExtensionRoutes
                    .compassGetTimeBetweenUpdatesTicks(sensor, outTicks);
        }

        @Override
        public int setUpdateTicks(long sensor, long ticks) {
            return NativeSensorExtensionRoutes
                    .compassSetTimeBetweenUpdatesTicks(sensor, ticks);
        }

        @Override
        public int dispose(long sensor) {
            return NativeSensorExtensionRoutes.compassDispose(sensor);
        }

        @Override
        public int destroy(long sensor) {
            return NativeSensorExtensionRoutes.compassDestroy(sensor);
        }

        @Override
        public int currentValueKind() {
            return org.openeggbert.cna.internal.NativeBindings.SENSOR_COMPASS_CURRENT;
        }
    };

    private Compass(long handle) {
        super("Compass", ROUTES, handle);
    }

    /** Reports whether this host has a compass. */
    public static boolean getIsSupported() {
        boolean[] supported = new boolean[1];
        SensorExtension.check("Compass.getIsSupported",
                NativeSensorExtensionRoutes.compassGetIsSupported(
                        SensorExtension.game("Compass"), supported));
        return supported[0];
    }

    /** Creates a compass. Creation succeeds even where the host has no such sensor. */
    public static Compass Create() {
        long[] sensor = new long[1];
        SensorExtension.check("Compass.Create",
                NativeSensorExtensionRoutes.compassCreate(
                        SensorExtension.game("Compass"), sensor));
        return new Compass(sensor[0]);
    }

    /**
     * Returns the most recent sample.
     *
     * @throws IllegalStateException when the host has no compass, which CNA refuses rather than
     *     answering with a default, or when this sensor is closed
     */
    @Override
    public CompassReading getCurrentValue() {
        long[] integral = new long[2];
        float[] floating = new float[3];
        double[] doubles = new double[3];
        check("getCurrentValue", NativeSensorExtensionRoutes
                .compassGetCurrentValue(open(), integral, floating, doubles));
        return new CompassReading(
                SensorExtension.timestamp(integral[0], integral[1]),
                doubles[0], doubles[1], doubles[2],
                new Vector3(floating[0], floating[1], floating[2]));
    }

    @Override
    CompassReading readingOf(double[] leaves) {
        return new CompassReading(
                SensorExtension.timestamp((long) leaves[0], (long) leaves[1]),
                leaves[2], leaves[3], leaves[4],
                new Vector3((float) leaves[5], (float) leaves[6], (float) leaves[7]));
    }

    /**
     * Installs or removes CNA's own stand-in backend for this sensor.
     *
     * <p>The canonical hook takes a caller-implemented backend object, which C cannot write, so
     * CNA supplies the backend and publishes only the switch. Without it there is no compass
     * on any desktop and no way to reach a single line past the unsupported refusal.
     *
     * <p><strong>Compass.getIsSupported() still answers false afterwards</strong>, and that is not
     * a defect: the backend is installed on this sensor and the static query asks the platform.
     * Measured in {@code tools/native-abi/probes/sensor_injection.c}.
     *
     * @param installed whether to install it
     * @param supported what the installed backend reports for support
     * @throws IllegalStateException when acquisition is currently started, which CNA refuses
     */
    public void setTestBackend(boolean installed, boolean supported) {
        check("setTestBackend", NativeSensorExtensionRoutes
                .compassSetTestBackendExt(open(), installed, supported));
    }

    /**
     * Feeds the sensor a synthetic reading.
     *
     * @param reading the reading the sensor then reports
     */
    public void injectSyntheticUpdate(CompassReading reading) {
        java.util.Objects.requireNonNull(reading, "reading");
        check("injectSyntheticUpdate", NativeSensorExtensionRoutes
                .compassInjectSyntheticUpdateExt(open(), reading.toIntegralLeaves(), reading.toFloatingLeaves(), reading.toDoubleLeaves()));
    }

    /**
     * Raises the sensor's calibration-requested event.
     *
     * @throws IllegalStateException when the sensor is closed
     */
    public void injectCalibrationRequest() {
        check("injectCalibrationRequest", NativeSensorExtensionRoutes
                .compassInjectCalibrationRequestExt(open()));
    }

    /**
     * Calls a handler whenever the host asks for the sensor to be calibrated.
     *
     * <p>The event carries no reading: it is a request to the player to wave the device, not a
     * measurement.
     *
     * @param handler what to do when calibration is requested
     * @return the subscription, which the caller closes
     */
    public SensorSubscription addCalibrateListener(Runnable handler) {
        java.util.Objects.requireNonNull(handler, "handler");
        return subscribe(org.openeggbert.cna.internal.NativeBindings.SENSOR_COMPASS_CALIBRATE,
                leaves -> handler.run());
    }
}
