package org.openeggbert.cna.extensions.input;

/**
 * What kind of device a raw joystick is, in CNA's own identity order.
 *
 * <p>{@link #GamePad} means the host also maps this device as an XNA game pad, so the same
 * hardware is readable both as {@code Microsoft.Xna.Framework.Input.GamePad} state and as raw
 * axes and buttons here.
 */
public enum JoystickType {

    /** The host does not recognise the device's shape. */
    Unknown,

    /** A device the host also maps as a game pad. */
    GamePad,

    /** A steering wheel. */
    Wheel,

    /** An arcade-style stick. */
    ArcadeStick,

    /** A flight stick. */
    FlightStick,

    /** A dance pad. */
    DancePad,

    /** A guitar-shaped controller. */
    Guitar,

    /** A drum-kit controller. */
    DrumKit,

    /** An arcade-cabinet pad. */
    ArcadePad,

    /** A throttle quadrant. */
    Throttle
}
