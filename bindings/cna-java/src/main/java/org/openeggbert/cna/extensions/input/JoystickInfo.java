package org.openeggbert.cna.extensions.input;

/**
 * One enumerated raw joystick.
 *
 * @param Id the host's own joystick instance identifier, which is what every other raw-joystick
 *     call takes and what a hot-plug event carries
 * @param Type the device shape the host recognised
 * @param Name the host's display name for the device, empty when the host has none
 */
public record JoystickInfo(int Id, JoystickType Type, String Name) {
}
