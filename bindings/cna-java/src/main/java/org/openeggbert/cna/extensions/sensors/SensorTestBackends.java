package org.openeggbert.cna.extensions.sensors;

import org.openeggbert.cna.internal.generated.NativeSensorExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Objects;

/**
 * CNA's own stand-in for sensor hardware, which is what makes the family reachable on a desktop.
 *
 * <p>An accelerometer and a gyroscope have no test backend the way a compass and a motion sensor
 * do. What they have instead is a set of seams, and the difference is worth stating because it
 * decides what a test can do: <strong>{@link Accelerometer#Start()} is refused on a machine with
 * no accelerometer</strong> -- CNA answers {@code INVALID_STATE} because there is no platform
 * subsystem to hold -- so the only way to reach a reading is to force the started flag, which is
 * what {@link #forceStarted} does.
 *
 * <p>With that done, a game's own event wiring is fully exercisable:
 * {@link Accelerometer#injectSyntheticUpdate} raises the subscription synchronously on the
 * injecting thread, and the reading it produces is the injected value converted into the
 * canonical unit -- 9.80665 metres per second squared reads back as one g. All of that is
 * measured in {@code tools/native-abi/probes/sensor_injection.c} rather than assumed.
 *
 * <p>These are CNA's published seams and they are here rather than hidden for the reason the
 * device dialogs' are: a consumer's own tests need them for exactly the reason this projection's
 * do, and a sensor that cannot be started is not something a game can work around.
 */
public final class SensorTestBackends {

    private SensorTestBackends() {
    }

    /**
     * Forces a sensor's started flag, without holding the platform subsystem.
     *
     * <p>The one call that makes an accelerometer or a gyroscope deliver anything on a machine
     * with no such hardware.
     *
     * @param sensor the sensor to force
     * @param started whether it should report itself started
     */
    public static void forceStarted(Sensor<?> sensor, boolean started) {
        Objects.requireNonNull(sensor, "sensor");
        SensorExtension.check("SensorTestBackends.forceStarted", isGyroscope(sensor)
                ? NativeSensorExtensionRoutes.gyroscopeSetStartedForTestsExt(
                        sensor.open(), started)
                : NativeSensorExtensionRoutes.accelerometerSetStartedForTestsExt(
                        sensor.open(), started));
    }

    /**
     * Forces one sensor's supported flag.
     *
     * <p>Per instance, which is why the static {@code getIsSupported} still answers about the
     * platform afterwards: the override is on this sensor and the query asks the host.
     *
     * @param sensor the sensor to force
     * @param supported whether it should report itself supported
     */
    public static void forceSupported(Sensor<?> sensor, boolean supported) {
        Objects.requireNonNull(sensor, "sensor");
        SensorExtension.check("SensorTestBackends.forceSupported", isGyroscope(sensor)
                ? NativeSensorExtensionRoutes.gyroscopeSetSupportedForTestsExt(
                        sensor.open(), supported)
                : NativeSensorExtensionRoutes.accelerometerSetSupportedForTestsExt(
                        sensor.open(), supported));
    }

    /**
     * Reports whether a sensor currently holds the platform subsystem.
     *
     * <p>The question {@link #forceStarted} does not change: a forced sensor reports itself
     * started and still holds nothing, which is the difference between a stand-in and hardware.
     *
     * @param sensor the sensor to ask
     * @return whether the hold is taken
     */
    public static boolean getSubsystemHeld(Sensor<?> sensor) {
        Objects.requireNonNull(sensor, "sensor");
        boolean[] held = new boolean[1];
        SensorExtension.check("SensorTestBackends.getSubsystemHeld", isGyroscope(sensor)
                ? NativeSensorExtensionRoutes.gyroscopeGetSubsystemHeldForTestsExt(
                        sensor.open(), held)
                : NativeSensorExtensionRoutes.accelerometerGetSubsystemHeldForTestsExt(
                        sensor.open(), held));
        return held[0];
    }

    /**
     * Adds a sensor to the registry an untargeted dispatch reaches.
     *
     * @param sensor the sensor to register
     */
    public static void registerStarted(Sensor<?> sensor) {
        Objects.requireNonNull(sensor, "sensor");
        SensorExtension.check("SensorTestBackends.registerStarted", isGyroscope(sensor)
                ? NativeSensorExtensionRoutes.gyroscopeRegisterStartedInstanceForTestsExt(
                        sensor.open())
                : NativeSensorExtensionRoutes.accelerometerRegisterStartedInstanceForTestsExt(
                        sensor.open()));
    }

    /**
     * Removes a sensor from that registry.
     *
     * @param sensor the sensor to unregister
     */
    public static void unregisterStarted(Sensor<?> sensor) {
        Objects.requireNonNull(sensor, "sensor");
        SensorExtension.check("SensorTestBackends.unregisterStarted", isGyroscope(sensor)
                ? NativeSensorExtensionRoutes.gyroscopeUnregisterStartedInstanceForTestsExt(
                        sensor.open())
                : NativeSensorExtensionRoutes.accelerometerUnregisterStartedInstanceForTestsExt(
                        sensor.open()));
    }

    /**
     * Dispatches one synthetic reading to an explicit set of sensors.
     *
     * <p>Different from injecting into each in turn: this is one dispatch that names its
     * recipients, which is how CNA's own tests exercise the fan-out rather than the delivery.
     *
     * @param sensors the sensors to deliver to; every one must be the same family
     * @param x the first axis, in platform units
     * @param y the second axis
     * @param z the third axis
     */
    public static void dispatchToInstances(List<? extends Sensor<?>> sensors,
            float x, float y, float z) {
        Objects.requireNonNull(sensors, "sensors");
        long[] handles = new long[sensors.size()];
        boolean gyroscope = false;
        for (int index = 0; index < handles.length; index++) {
            Sensor<?> sensor = Objects.requireNonNull(sensors.get(index), "sensor");
            handles[index] = sensor.open();
            gyroscope = isGyroscope(sensor);
        }
        SensorExtension.check("SensorTestBackends.dispatchToInstances", gyroscope
                ? NativeSensorExtensionRoutes.gyroscopeDispatchToInstancesForTestsExt(
                        SensorExtension.game("SensorTestBackends"), handles, x, y, z)
                : NativeSensorExtensionRoutes.accelerometerDispatchToInstancesForTestsExt(
                        SensorExtension.game("SensorTestBackends"), handles, x, y, z));
    }

    /**
     * Reports whether the host says a sensor with this identifier is connected.
     *
     * @param family which sensor family to ask about
     * @param sensorId the host's own sensor identifier
     * @return whether the host reports it connected
     */
    public static boolean isSensorConnected(SensorType family, long sensorId) {
        Objects.requireNonNull(family, "family");
        boolean[] connected = new boolean[1];
        SensorExtension.check("SensorTestBackends.isSensorConnected",
                family == SensorType.Gyroscope
                        ? NativeSensorExtensionRoutes.gyroscopeIsSensorConnectedForTestsExt(
                                SensorExtension.game("SensorTestBackends"), sensorId, connected)
                        : NativeSensorExtensionRoutes.accelerometerIsSensorConnectedForTestsExt(
                                SensorExtension.game("SensorTestBackends"), sensorId, connected));
        return connected[0];
    }

    /**
     * Makes the next event-watch registration fail, so a caller can exercise that path.
     *
     * @param family which sensor family to affect
     * @param shouldFail whether the registration should fail
     */
    public static void failEventWatchRegistration(SensorType family, boolean shouldFail) {
        Objects.requireNonNull(family, "family");
        SensorExtension.check("SensorTestBackends.failEventWatchRegistration",
                family == SensorType.Gyroscope
                        ? NativeSensorExtensionRoutes
                                .gyroscopeSetEventWatchRegistrationFailureForTestsExt(
                                        SensorExtension.game("SensorTestBackends"), shouldFail)
                        : NativeSensorExtensionRoutes
                                .accelerometerSetEventWatchRegistrationFailureForTestsExt(
                                        SensorExtension.game("SensorTestBackends"), shouldFail));
    }

    /**
     * Returns how many handler exceptions CNA's own dispatch has swallowed.
     *
     * <p>A sensor event returns nothing, so a handler that throws cannot fail the dispatch. CNA
     * counts them instead, and this is how a test proves its handler ran and threw rather than
     * never running at all.
     *
     * @param family which sensor family to ask about
     * @return the count
     */
    public static int getDispatchExceptionCount(SensorType family) {
        Objects.requireNonNull(family, "family");
        int[] count = new int[1];
        SensorExtension.check("SensorTestBackends.getDispatchExceptionCount",
                family == SensorType.Gyroscope
                        ? NativeSensorExtensionRoutes
                                .gyroscopeGetDispatchExceptionCountForTestsExt(
                                        SensorExtension.game("SensorTestBackends"), count)
                        : NativeSensorExtensionRoutes
                                .accelerometerGetDispatchExceptionCountForTestsExt(
                                        SensorExtension.game("SensorTestBackends"), count));
        return count[0];
    }

    /**
     * Returns the message of the most recent swallowed handler exception.
     *
     * @param family which sensor family to ask about
     * @return the message, empty when nothing has thrown
     */
    public static String getLastDispatchExceptionMessage(SensorType family) {
        Objects.requireNonNull(family, "family");
        long game = SensorExtension.game("SensorTestBackends");
        long[] bytes = new long[1];
        boolean gyroscope = family == SensorType.Gyroscope;
        SensorExtension.check("SensorTestBackends.getLastDispatchExceptionMessage", gyroscope
                ? NativeSensorExtensionRoutes
                        .gyroscopeGetLastDispatchExceptionMessageSizeForTestsExt(game, bytes)
                : NativeSensorExtensionRoutes
                        .accelerometerGetLastDispatchExceptionMessageSizeForTestsExt(game, bytes));
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        SensorExtension.check("SensorTestBackends.getLastDispatchExceptionMessage", gyroscope
                ? NativeSensorExtensionRoutes
                        .gyroscopeCopyLastDispatchExceptionMessageForTestsExt(
                                game, destination, bytes)
                : NativeSensorExtensionRoutes
                        .accelerometerCopyLastDispatchExceptionMessageForTestsExt(
                                game, destination, bytes));
        return new String(destination, 0, Math.toIntExact(bytes[0]), StandardCharsets.UTF_8);
    }

    private static boolean isGyroscope(Sensor<?> sensor) {
        return sensor instanceof Gyroscope;
    }
}
