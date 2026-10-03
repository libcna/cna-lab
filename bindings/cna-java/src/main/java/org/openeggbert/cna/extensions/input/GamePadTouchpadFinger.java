package org.openeggbert.cna.extensions.input;

/**
 * One finger on a game pad's touchpad.
 *
 * @param IsDown whether the finger is touching the pad
 * @param X where it is across the pad, from 0 at the left to 1 at the right
 * @param Y where it is down the pad, from 0 at the top to 1 at the bottom
 * @param Pressure how hard it is pressing, from 0 to 1
 */
public record GamePadTouchpadFinger(boolean IsDown, float X, float Y, float Pressure) {
}
