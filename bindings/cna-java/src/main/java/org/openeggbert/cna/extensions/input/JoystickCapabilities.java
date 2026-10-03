package org.openeggbert.cna.extensions.input;

import org.openeggbert.cna.extensions.devices.PowerState;

/**
 * The fixed hardware shape of one raw joystick.
 *
 * <p>These counts are what {@link JoystickState} will have: a device with six axes reports six
 * axis values, whatever the host's own maximum happens to be.
 *
 * @param Name the host's display name for the device
 * @param Guid the host's stable identifier for the device model, empty when the host has none
 * @param AxisCount how many analogue axes the device reports
 * @param ButtonCount how many buttons the device reports
 * @param HatCount how many POV hats the device reports
 * @param BallCount how many trackballs the device reports
 * @param Type the device shape the host recognised
 * @param Power the device's power state
 * @param PowerPercent the remaining charge as a percentage, or {@code null} when the host will
 *     not say; absent is not zero, because zero means empty
 * @param IsConnected whether the device is connected right now
 */
public record JoystickCapabilities(
        String Name,
        String Guid,
        int AxisCount,
        int ButtonCount,
        int HatCount,
        int BallCount,
        JoystickType Type,
        PowerState Power,
        Integer PowerPercent,
        boolean IsConnected) {
}
