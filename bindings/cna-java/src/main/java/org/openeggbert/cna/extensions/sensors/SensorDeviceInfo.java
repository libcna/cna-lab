package org.openeggbert.cna.extensions.sensors;

/**
 * One enumerated host motion sensor.
 *
 * @param Id the host's own sensor instance identifier
 * @param Type the kind of sensor the host reported
 * @param Name the host's display name for it, empty when the host has none
 */
public record SensorDeviceInfo(long Id, SensorType Type, String Name) {
}
