package org.openeggbert.cna.extensions.input;

import Microsoft.Xna.Framework.Point;

import java.util.List;

/**
 * One instant of a raw joystick's axes, buttons, hats and trackballs.
 *
 * <p>This is <strong>not</strong> XNA game-pad state and does not pretend to be. Axes are the
 * host's raw signed 16-bit values, not the normalised floats {@code GamePadState} reports, and
 * buttons are indexed by the device's own numbering rather than named. A device the host also
 * maps as a game pad can be read either way; neither reading is derived from the other.
 *
 * <p>A snapshot is a copy. CNA hands out an owned native snapshot so that all four arrays come
 * from one instant; this record copies it and releases the native one before returning, so it
 * carries no handle, no lifetime and no thread affinity, and two snapshots compare by value.
 *
 * <p><strong>Reading trackballs consumes them.</strong> Ball values are relative motion since the
 * previous capture, so capturing twice in one frame gives the second capture nothing to report.
 *
 * @param Axes one raw value per axis, from -32768 to 32767
 * @param Buttons one flag per button, in the device's own button order
 * @param Hats one position per POV hat
 * @param Balls relative motion per trackball since the previous capture
 */
public record JoystickState(
        List<Short> Axes,
        List<Boolean> Buttons,
        List<JoystickHatPosition> Hats,
        List<Point> Balls) {

    /** Copies each list, so a snapshot cannot be changed after it is taken. */
    public JoystickState {
        Axes = List.copyOf(Axes);
        Buttons = List.copyOf(Buttons);
        Hats = List.copyOf(Hats);
        Balls = List.copyOf(Balls);
    }

    /**
     * Reports whether the capture found nothing at all.
     *
     * <p>An identifier that is not connected is not an error in CNA: the capture succeeds and
     * every list is empty, which is what this reports.
     *
     * @return {@code true} when no axis, button, hat or ball was reported
     */
    public boolean isEmpty() {
        return Axes.isEmpty() && Buttons.isEmpty() && Hats.isEmpty() && Balls.isEmpty();
    }
}
