package org.openeggbert.cna.extensions.input;

import java.time.Duration;

/**
 * A device button that replays an effect, and how often it may do so.
 *
 * @param Button the device's own one-based button index, or zero for no trigger
 * @param Interval the shortest time between two button-driven replays
 */
public record HapticTrigger(int Button, Duration Interval) {

    /** No button replays the effect; it plays only when the game runs it. */
    public static final HapticTrigger NONE = new HapticTrigger(0, Duration.ZERO);
}
