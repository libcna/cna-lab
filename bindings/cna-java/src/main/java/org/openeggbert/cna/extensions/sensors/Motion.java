package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.generated.NativeSensorExtensionRoutes;

/**
 * The host's fused motion sensor: orientation, motion and gravity separated from one another.
 *
 * <p>A CNA extension. This is the sensor a game wants for tilt control, because the fusion has
 * already removed gravity from the acceleration; the raw {@link Accelerometer} has not.
 */
public final class Motion extends Sensor<MotionReading> {

    private static final Routes ROUTES = new Routes() {

        @Override
        public int state(long sensor, int[] outState) {
            return NativeSensorExtensionRoutes.motionGetState(sensor, outState);
        }

        @Override
        public int start(long sensor) {
            return NativeSensorExtensionRoutes.motionStart(sensor);
        }

        @Override
        public int stop(long sensor) {
            return NativeSensorExtensionRoutes.motionStop(sensor);
        }

        @Override
        public int dataValid(long sensor, boolean[] outValid) {
            return NativeSensorExtensionRoutes.motionGetIsDataValid(sensor, outValid);
        }

        @Override
        public int updateTicks(long sensor, long[] outTicks) {
            return NativeSensorExtensionRoutes.motionGetTimeBetweenUpdatesTicks(sensor, outTicks);
        }

        @Override
        public int setUpdateTicks(long sensor, long ticks) {
            return NativeSensorExtensionRoutes.motionSetTimeBetweenUpdatesTicks(sensor, ticks);
        }

        @Override
        public int dispose(long sensor) {
            return NativeSensorExtensionRoutes.motionDispose(sensor);
        }

        @Override
        public int destroy(long sensor) {
            return NativeSensorExtensionRoutes.motionDestroy(sensor);
        }

        @Override
        public int currentValueKind() {
            return org.openeggbert.cna.internal.NativeBindings.SENSOR_MOTION_CURRENT;
        }
    };

    private Motion(long handle) {
        super("Motion", ROUTES, handle);
    }

    /** Reports whether this host fuses motion. */
    public static boolean getIsSupported() {
        boolean[] supported = new boolean[1];
        SensorExtension.check("Motion.getIsSupported",
                NativeSensorExtensionRoutes.motionGetIsSupported(
                        SensorExtension.game("Motion"), supported));
        return supported[0];
    }

    /** Creates a motion sensor. Creation succeeds even where the host does not fuse motion. */
    public static Motion Create() {
        long[] sensor = new long[1];
        SensorExtension.check("Motion.Create",
                NativeSensorExtensionRoutes.motionCreate(
                        SensorExtension.game("Motion"), sensor));
        return new Motion(sensor[0]);
    }

    /**
     * Reports whether the attitude the sensor produces has an absolute yaw reference.
     *
     * <p>A CNA extension to the sensor itself, and one whose default is deliberately vacuous:
     * before a backend starts, the answer is {@code true} meaning "nothing is drifting yet", not
     * "north is known". It is informative only once a started backend answers for itself, which
     * {@link #getState()} reports.
     */
    public boolean getIsAttitudeNorthReferenced() {
        boolean[] referenced = new boolean[1];
        check("getIsAttitudeNorthReferenced", NativeSensorExtensionRoutes
                .motionGetIsAttitudeNorthReferencedExt(open(), referenced));
        return referenced[0];
    }

    /**
     * Returns the most recent sample.
     *
     * @throws IllegalStateException when the host does not fuse motion, which CNA refuses rather
     *     than answering with a default, or when this sensor is closed
     */
    @Override
    public MotionReading getCurrentValue() {
        long[] integral = new long[4];
        float[] floating = new float[32];
        check("getCurrentValue", NativeSensorExtensionRoutes
                .motionGetCurrentValue(open(), integral, floating));
        AttitudeReading attitude = new AttitudeReading(
                SensorExtension.timestamp(integral[2], integral[3]),
                floating[0], floating[1], floating[2],
                new Quaternion(floating[3], floating[4], floating[5], floating[6]),
                new Matrix(
                        floating[7], floating[8], floating[9], floating[10],
                        floating[11], floating[12], floating[13], floating[14],
                        floating[15], floating[16], floating[17], floating[18],
                        floating[19], floating[20], floating[21], floating[22]));
        return new MotionReading(
                SensorExtension.timestamp(integral[0], integral[1]),
                attitude,
                new Vector3(floating[23], floating[24], floating[25]),
                new Vector3(floating[26], floating[27], floating[28]),
                new Vector3(floating[29], floating[30], floating[31]));
    }

    @Override
    MotionReading readingOf(double[] leaves) {
        AttitudeReading attitude = new AttitudeReading(
                SensorExtension.timestamp((long) leaves[2], (long) leaves[3]),
                (float) leaves[4], (float) leaves[5], (float) leaves[6],
                new Quaternion((float) leaves[7], (float) leaves[8], (float) leaves[9],
                        (float) leaves[10]),
                new Matrix(
                        (float) leaves[11], (float) leaves[12], (float) leaves[13],
                        (float) leaves[14], (float) leaves[15], (float) leaves[16],
                        (float) leaves[17], (float) leaves[18], (float) leaves[19],
                        (float) leaves[20], (float) leaves[21], (float) leaves[22],
                        (float) leaves[23], (float) leaves[24], (float) leaves[25],
                        (float) leaves[26]));
        return new MotionReading(
                SensorExtension.timestamp((long) leaves[0], (long) leaves[1]),
                attitude,
                new Vector3((float) leaves[27], (float) leaves[28], (float) leaves[29]),
                new Vector3((float) leaves[30], (float) leaves[31], (float) leaves[32]),
                new Vector3((float) leaves[33], (float) leaves[34], (float) leaves[35]));
    }

    /**
     * Installs or removes CNA's own stand-in backend for this sensor.
     *
     * <p>The canonical hook takes a caller-implemented backend object, which C cannot write, so
     * CNA supplies the backend and publishes only the switch. Without it there is no motion
     * on any desktop and no way to reach a single line past the unsupported refusal.
     *
     * <p><strong>Motion.getIsSupported() still answers false afterwards</strong>, and that is not
     * a defect: the backend is installed on this sensor and the static query asks the platform.
     * Measured in {@code tools/native-abi/probes/sensor_injection.c}.
     *
     * @param installed whether to install it
     * @param supported what the installed backend reports for support
     * @param northReferenced whether the installed backend reports a north-referenced attitude
     * @throws IllegalStateException when acquisition is currently started, which CNA refuses
     */
    public void setTestBackend(boolean installed, boolean supported, boolean northReferenced) {
        check("setTestBackend", NativeSensorExtensionRoutes
                .motionSetTestBackendExt(open(), installed, supported, northReferenced));
    }

    /**
     * Feeds the sensor a synthetic reading.
     *
     * @param reading the reading the sensor then reports
     */
    public void injectSyntheticUpdate(MotionReading reading) {
        java.util.Objects.requireNonNull(reading, "reading");
        check("injectSyntheticUpdate", NativeSensorExtensionRoutes
                .motionInjectSyntheticUpdateExt(open(), reading.toIntegralLeaves(), reading.toFloatingLeaves()));
    }

    /**
     * Raises the sensor's calibration-requested event.
     *
     * @throws IllegalStateException when the sensor is closed
     */
    public void injectCalibrationRequest() {
        check("injectCalibrationRequest", NativeSensorExtensionRoutes
                .motionInjectCalibrationRequestExt(open()));
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
        return subscribe(org.openeggbert.cna.internal.NativeBindings.SENSOR_MOTION_CALIBRATE,
                leaves -> handler.run());
    }
}
