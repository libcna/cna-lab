package org.openeggbert.cna.extensions.devices;

/**
 * One enumerated input device: a mouse, a keyboard or a touch device.
 *
 * <p>This is metadata only. XNA's input state is merged across every device of a kind, and CNA
 * keeps it that way, so an identifier here does not select a device to read from; it is what
 * lets a game name the hardware and recognise the same device across an enumeration.
 *
 * @param Id the host's own device instance identifier, stable while the device stays connected
 * @param Name the host's display name for the device, empty when the host has none
 */
public record InputDeviceInfo(long Id, String Name) {
}
