package org.openeggbert.cna.extensions.input;

/**
 * One enumerated standalone haptic device.
 *
 * @param Id the host's own device identifier, which {@link HapticDevices#open(int)} takes
 * @param Name the host's display name for the device, empty when the host has none
 */
public record HapticDeviceInfo(int Id, String Name) {
}
