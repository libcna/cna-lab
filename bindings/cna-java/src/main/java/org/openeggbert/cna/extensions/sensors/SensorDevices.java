package org.openeggbert.cna.extensions.sensors;

import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.generated.NativeSensorExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * Enumerates the motion sensors the host machine reports, and reads the first of each kind.
 *
 * <p>A CNA extension. This is the machine's own hardware, not a controller's: a gamepad's
 * accelerometer and gyroscope are read through the gamepad, not through here.
 *
 * <p><strong>An enumeration is a snapshot.</strong> Each call takes a fresh one, so an index is
 * only meaningful until the sensor set changes; {@link SensorDeviceInfo#Id()} is what survives.
 *
 * <p>The two quick reads are a different thing from {@link Accelerometer} and {@link Gyroscope}:
 * they take whatever the host already has, with no sensor to create, start or close, and they
 * report in metres per second squared and radians per second respectively. The sensor classes
 * report acceleration in g, which is the unit their own contract states.
 */
public final class SensorDevices {

    private SensorDevices() {
    }

    /** Returns a fresh snapshot of the motion sensors the host reports. */
    public static List<SensorDeviceInfo> enumerate() {
        long game = SensorExtension.game("SensorDevices");
        int[] count = new int[1];
        SensorExtension.check("SensorDevices.count",
                NativeSensorExtensionRoutes.sensorsGetCount(game, count));
        List<SensorDeviceInfo> sensors = new ArrayList<>(count[0]);
        for (int index = 0; index < count[0]; index++) {
            long[] info = new long[2];
            SensorExtension.check("SensorDevices.info",
                    NativeSensorExtensionRoutes.sensorsGetInfoAt(game, index, info));
            sensors.add(new SensorDeviceInfo(
                    info[0], SensorType.values()[(int) info[1]], name(game, index)));
        }
        return List.copyOf(sensors);
    }

    /**
     * Returns the first accelerometer reading the host has, in metres per second squared.
     *
     * <p>{@code null} means the host produced no reading, which is an ordinary answer on a
     * machine with no accelerometer. It is not a zero vector: a zero vector would be free fall.
     *
     * @return the acceleration, or {@code null} when no reading was produced
     */
    public static Vector3 getAcceleration() {
        float[] acceleration = new float[3];
        boolean[] available = new boolean[1];
        SensorExtension.check("SensorDevices.getAcceleration",
                NativeSensorExtensionRoutes.sensorsGetAccelerometer(
                        SensorExtension.game("SensorDevices"), acceleration, available));
        return available[0]
                ? new Vector3(acceleration[0], acceleration[1], acceleration[2])
                : null;
    }

    /**
     * Returns the first gyroscope reading the host has, in radians per second.
     *
     * @return the angular velocity, or {@code null} when no reading was produced
     */
    public static Vector3 getAngularVelocity() {
        float[] velocity = new float[3];
        boolean[] available = new boolean[1];
        SensorExtension.check("SensorDevices.getAngularVelocity",
                NativeSensorExtensionRoutes.sensorsGetGyroscope(
                        SensorExtension.game("SensorDevices"), velocity, available));
        return available[0] ? new Vector3(velocity[0], velocity[1], velocity[2]) : null;
    }

    private static String name(long game, int index) {
        long[] bytes = new long[1];
        SensorExtension.check("SensorDevices.nameSize",
                NativeSensorExtensionRoutes.sensorsGetNameSizeAt(game, index, bytes));
        byte[] destination = new byte[(int) bytes[0]];
        SensorExtension.check("SensorDevices.name",
                NativeSensorExtensionRoutes.sensorsCopyNameAt(game, index, destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }
}
