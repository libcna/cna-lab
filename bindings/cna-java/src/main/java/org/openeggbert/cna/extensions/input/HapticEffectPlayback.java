package org.openeggbert.cna.extensions.input;

import java.util.Objects;

/**
 * One effect uploaded to a device, and the handle that plays it.
 *
 * <p>The effect is <strong>owned by the device</strong>, not by this object: the identifier is
 * only meaningful on the device that produced it, and closing the device frees every effect it
 * holds. Closing this releases just this one, and closing it after its device is closed is a
 * no-op rather than a second free.
 *
 * <p>A device that could not store the effect does not fail: {@link #getIsStored()} is false and
 * every operation reports that nothing was applied, which is what CNA's own device does.
 */
public final class HapticEffectPlayback implements AutoCloseable {

    /** CNA reports a device that could not store the effect as identifier -1. */
    static final int NOT_STORED = -1;

    private final HapticDevice device;
    private final int effectId;
    private boolean closed;

    HapticEffectPlayback(HapticDevice device, int effectId) {
        this.device = device;
        this.effectId = effectId;
    }

    /** Reports whether the device actually stored the effect. */
    public boolean getIsStored() {
        return effectId != NOT_STORED;
    }

    /**
     * Plays the effect a number of times.
     *
     * @param iterations how many times to repeat it; one is CNA's own default
     * @return whether playback started
     */
    public boolean Run(int iterations) {
        if (iterations <= 0) {
            throw new IllegalArgumentException("iterations must be positive: " + iterations);
        }
        return device.runEffect(open(), iterations);
    }

    /** Plays the effect once. */
    public boolean Run() {
        return Run(1);
    }

    /** Stops the effect. Returns whether it was stopped. */
    public boolean Stop() {
        return device.stopEffect(open());
    }

    /**
     * Reports whether the effect is playing right now.
     *
     * <p>Only a device whose capabilities include {@link HapticFeature#Status} can answer this;
     * one that cannot reports false.
     */
    public boolean getIsPlaying() {
        return device.effectStatus(open());
    }

    /**
     * Replaces the effect's settings without re-uploading it.
     *
     * @param effect the new settings; changing the family is a device decision, not a Java one
     * @return whether the device applied them
     */
    public boolean Update(HapticEffect effect) {
        Objects.requireNonNull(effect, "effect");
        return device.updateEffect(open(), effect);
    }

    /** Frees the effect on its device. A no-op once the device itself is closed. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        device.destroyEffect(effectId);
    }

    private int open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This HapticEffectPlayback is closed");
            }
        }
        return effectId;
    }

    /** Marks the effect gone because its owning device was closed, without freeing it again. */
    synchronized void ownerClosed() {
        closed = true;
    }
}
