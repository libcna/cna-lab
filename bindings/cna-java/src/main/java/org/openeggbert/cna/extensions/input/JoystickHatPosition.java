package org.openeggbert.cna.extensions.input;

/**
 * Where a joystick's POV hat is pushed, in CNA's own identity order.
 *
 * <p>This is <strong>not</strong> a bit set. The host encodes a hat as an up/down bit combined
 * with a left/right bit, but CNA enumerates the nine reachable combinations instead, so
 * {@link #RightUp} is its own constant and not {@code Right} combined with {@code Up}.
 */
public enum JoystickHatPosition {

    /** The hat is not pushed in any direction. */
    Centered,

    /** Pushed up. */
    Up,

    /** Pushed right. */
    Right,

    /** Pushed down. */
    Down,

    /** Pushed left. */
    Left,

    /** Pushed up and to the right. */
    RightUp,

    /** Pushed down and to the right. */
    RightDown,

    /** Pushed up and to the left. */
    LeftUp,

    /** Pushed down and to the left. */
    LeftDown
}
