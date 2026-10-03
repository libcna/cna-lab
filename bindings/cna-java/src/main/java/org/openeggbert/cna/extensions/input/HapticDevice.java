package org.openeggbert.cna.extensions.input;

import org.openeggbert.cna.internal.generated.NativeInputExtensionRoutes;

import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * An opened force-feedback device.
 *
 * <p>A CNA extension. XNA 4.0 has {@code GamePad.SetVibration} and nothing else: two normalised
 * motor speeds on an Xbox controller. A wheel that can push back, a stick with a spring, or a
 * device that stores and replays a waveform has no XNA shape at all.
 *
 * <p><strong>A device that failed to open is still a real device.</strong> That is CNA's
 * contract, and it is reported rather than smoothed over: {@link #getIsOpen()} says so and every
 * other operation is a safe no-op that reports nothing was applied. Opening never throws for
 * absent hardware.
 *
 * <p>The handle is owned; {@link #close()} releases it. Every effect uploaded to the device is
 * owned by the device, so closing the device frees them all and a later
 * {@link HapticEffectPlayback#close()} is a no-op rather than a second free.
 */
public final class HapticDevice implements AutoCloseable {

    private final long handle;
    private final List<HapticEffectPlayback> effects = new ArrayList<>();
    private boolean closed;

    HapticDevice(long handle) {
        this.handle = handle;
    }

    /**
     * Closes the device without releasing the handle.
     *
     * <p>The canonical disposal, and it is not {@link #close()}: afterwards this object is still
     * usable and {@link #getIsOpen()} reports {@code false}, with every other operation a safe
     * no-op that reports nothing was applied. That is exactly what a device that failed to open
     * already behaves like, so a game has one code path for both. Idempotent.
     *
     * <p>{@link #close()} is the other end: it releases the handle, and a second release is
     * refused.
     */
    public void dispose() {
        check("dispose", NativeInputExtensionRoutes.hapticDeviceDispose(open()));
    }

    /** Reports whether the handle is actually attached to hardware. */
    public boolean getIsOpen() {
        boolean[] open = new boolean[1];
        check("getIsOpen", NativeInputExtensionRoutes.hapticDeviceGetIsOpen(open(), open));
        return open[0];
    }

    /** Returns the host's display name for the device, empty when the host has none. */
    public String getName() {
        long[] bytes = new long[1];
        check("getNameSize",
                NativeInputExtensionRoutes.hapticDeviceGetNameSize(open(), bytes));
        byte[] destination = new byte[(int) bytes[0]];
        check("getName",
                NativeInputExtensionRoutes.hapticDeviceCopyName(open(), destination, bytes));
        return new String(destination, 0, (int) bytes[0], StandardCharsets.UTF_8);
    }

    /** Returns what the device can do. A closed device reports no features and no maxima. */
    public HapticCapabilities getCapabilities() {
        long[] values = new long[6];
        check("getCapabilities", NativeInputExtensionRoutes
                .hapticDeviceGetCapabilities(open(), new byte[2], values));
        return new HapticCapabilities(getName(), HapticFeature.decode(values[0]),
                (int) values[1], unknownAsAbsent(values[2]), unknownAsAbsent(values[3]),
                values[4] != 0L, values[5] != 0L);
    }

    /**
     * Reports whether this device can play one particular effect.
     *
     * <p>This asks the device rather than reading the capability bits, because a device may
     * refuse an effect its family bit allows -- too many axes, or a waveform it cannot store.
     *
     * @param effect the effect to ask about
     * @return whether the device says it can play it
     */
    public boolean isEffectSupported(HapticEffect effect) {
        Objects.requireNonNull(effect, "effect");
        boolean[] supported = new boolean[1];
        check("isEffectSupported", NativeInputExtensionRoutes.hapticDeviceGetIsEffectSupported(
                open(), effect.encode(), samples(effect), supported));
        return supported[0];
    }

    /**
     * Prepares the simple rumble shortcut.
     *
     * @return whether the device prepared it; a device with no rumble reports false
     */
    public boolean InitializeRumble() {
        boolean[] applied = new boolean[1];
        check("InitializeRumble",
                NativeInputExtensionRoutes.hapticDeviceInitRumble(open(), applied));
        return applied[0];
    }

    /**
     * Plays the simple rumble.
     *
     * <p>CNA passes the strength through to the platform without validating it, exactly as its
     * own operation does, so this does not clamp it either: a platform that treats an
     * out-of-range value specially keeps doing so.
     *
     * @param strength how hard to rumble, normally from zero through one
     * @param length how long to rumble for
     * @return whether the rumble started
     */
    public boolean PlayRumble(float strength, Duration length) {
        Objects.requireNonNull(length, "length");
        boolean[] applied = new boolean[1];
        check("PlayRumble", NativeInputExtensionRoutes.hapticDevicePlayRumble(open(), strength,
                (int) HapticEffectLayout.milliseconds(length, "length"), applied));
        return applied[0];
    }

    /** Stops the simple rumble. Returns whether it was stopped. */
    public boolean StopRumble() {
        boolean[] applied = new boolean[1];
        check("StopRumble",
                NativeInputExtensionRoutes.hapticDeviceStopRumble(open(), applied));
        return applied[0];
    }

    /**
     * Uploads an effect to the device.
     *
     * <p>A device that cannot store it does not fail: the returned playback reports
     * {@link HapticEffectPlayback#getIsStored()} false, which is CNA's own behaviour.
     *
     * @param effect the effect to upload
     * @return the device-owned effect, which this device frees when it closes
     */
    public HapticEffectPlayback createEffect(HapticEffect effect) {
        Objects.requireNonNull(effect, "effect");
        int[] effectId = new int[1];
        check("createEffect", NativeInputExtensionRoutes.hapticDeviceCreateEffect(
                open(), effect.encode(), samples(effect), effectId));
        HapticEffectPlayback playback = new HapticEffectPlayback(this, effectId[0]);
        synchronized (effects) {
            effects.add(playback);
        }
        return playback;
    }

    /** Stops every effect the device is playing. Returns whether anything was stopped. */
    public boolean StopAllEffects() {
        boolean[] applied = new boolean[1];
        check("StopAllEffects",
                NativeInputExtensionRoutes.hapticDeviceStopAllEffects(open(), applied));
        return applied[0];
    }

    /**
     * Sets the overall effect gain.
     *
     * @param gain the gain, from 0 to 100
     * @return whether the device applied it; one without {@link HapticFeature#Gain} reports false
     */
    public boolean setGain(int gain) {
        boolean[] applied = new boolean[1];
        check("setGain",
                NativeInputExtensionRoutes.hapticDeviceSetGain(open(), gain, applied));
        return applied[0];
    }

    /**
     * Sets how hard the device pulls back to centre.
     *
     * @param autocenter the strength, from 0 to 100
     * @return whether the device applied it
     */
    public boolean setAutocenter(int autocenter) {
        boolean[] applied = new boolean[1];
        check("setAutocenter", NativeInputExtensionRoutes
                .hapticDeviceSetAutocenter(open(), autocenter, applied));
        return applied[0];
    }

    /** Pauses every playing effect. Returns whether the device paused. */
    public boolean Pause() {
        boolean[] applied = new boolean[1];
        check("Pause", NativeInputExtensionRoutes.hapticDevicePause(open(), applied));
        return applied[0];
    }

    /** Resumes every paused effect. Returns whether the device resumed. */
    public boolean Resume() {
        boolean[] applied = new boolean[1];
        check("Resume", NativeInputExtensionRoutes.hapticDeviceResume(open(), applied));
        return applied[0];
    }

    /**
     * Releases the device and every effect it owns.
     *
     * <p>CNA frees the device's effects with the device, so this marks each outstanding playback
     * gone rather than freeing it twice. Closing twice is a no-op.
     */
    @Override
    public void close() {
        List<HapticEffectPlayback> outstanding;
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        synchronized (effects) {
            outstanding = List.copyOf(effects);
            effects.clear();
        }
        for (HapticEffectPlayback playback : outstanding) {
            playback.ownerClosed();
        }
        check("close", NativeInputExtensionRoutes.hapticDeviceDestroy(handle));
    }

    boolean runEffect(int effectId, int iterations) {
        boolean[] applied = new boolean[1];
        check("HapticEffectPlayback.Run", NativeInputExtensionRoutes
                .hapticDeviceRunEffect(open(), effectId, iterations, applied));
        return applied[0];
    }

    boolean stopEffect(int effectId) {
        boolean[] applied = new boolean[1];
        check("HapticEffectPlayback.Stop", NativeInputExtensionRoutes
                .hapticDeviceStopEffect(open(), effectId, applied));
        return applied[0];
    }

    boolean effectStatus(int effectId) {
        boolean[] playing = new boolean[1];
        check("HapticEffectPlayback.getIsPlaying", NativeInputExtensionRoutes
                .hapticDeviceGetEffectStatus(open(), effectId, playing));
        return playing[0];
    }

    boolean updateEffect(int effectId, HapticEffect effect) {
        boolean[] applied = new boolean[1];
        check("HapticEffectPlayback.Update", NativeInputExtensionRoutes.hapticDeviceUpdateEffect(
                open(), effectId, effect.encode(), samples(effect), applied));
        return applied[0];
    }

    void destroyEffect(int effectId) {
        synchronized (this) {
            if (closed) {
                // The device already freed it. Freeing it again would be a second release of
                // something this object does not own.
                return;
            }
        }
        check("HapticEffectPlayback.close",
                NativeInputExtensionRoutes.hapticDeviceDestroyEffect(handle, effectId));
    }

    private static int[] samples(HapticEffect effect) {
        List<Integer> values = effect.samples();
        int[] samples = new int[values.size()];
        for (int index = 0; index < samples.length; index++) {
            samples[index] = values.get(index);
        }
        return samples;
    }

    /** CNA reports an unknown or closed-device maximum as -1, which is absent, not zero. */
    private static Integer unknownAsAbsent(long value) {
        return value == -1L ? null : (int) value;
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This HapticDevice is closed");
            }
        }
        return handle;
    }

    private static void check(String operation, int result) {
        InputExtension.check("HapticDevice." + operation, result);
    }
}
