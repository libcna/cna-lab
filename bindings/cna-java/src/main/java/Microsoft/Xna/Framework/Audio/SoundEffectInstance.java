package Microsoft.Xna.Framework.Audio;

import org.openeggbert.cna.internal.NativeAudio;

import java.util.Objects;

/** Independently owned playback voice created by a {@link SoundEffect}. */
public class SoundEffectInstance implements AutoCloseable {
    private final Object lock = new Object();
    private final SoundEffect parent;
    private long handle;
    private float volume = 1.0f;
    private float pitch;
    private float pan;
    private boolean looped;

    SoundEffectInstance(SoundEffect parent, long handle) {
        this.parent = parent;
        if (handle == 0L) throw new NoAudioHardwareException("Audio instance creation failed");
        this.handle = handle;
    }

    SoundEffectInstance(long handle) {
        this(null, handle);
    }

    public final void Apply3D(AudioListener listener, AudioEmitter emitter) {
        synchronized (lock) {
            NativeAudio.apply3D(requireHandle(),
                    Objects.requireNonNull(listener, "listener").nativeValues(),
                    Objects.requireNonNull(emitter, "emitter").nativeValues());
        }
    }

    public final void Apply3D(AudioListener[] listeners, AudioEmitter emitter) {
        Objects.requireNonNull(listeners, "listeners");
        float[] values = new float[listeners.length * 12];
        for (int i = 0; i < listeners.length; i++) {
            float[] next = Objects.requireNonNull(listeners[i], "listeners element").nativeValues();
            System.arraycopy(next, 0, values, i * 12, 12);
        }
        synchronized (lock) {
            NativeAudio.apply3D(requireHandle(), values,
                    Objects.requireNonNull(emitter, "emitter").nativeValues());
        }
    }

    public final void Pause() {
        synchronized (lock) { NativeAudio.instanceTransport(requireHandle(), 1, false); }
    }

    public void Play() {
        synchronized (lock) { NativeAudio.instanceTransport(requireHandle(), 0, false); }
    }

    public final void Resume() {
        synchronized (lock) { NativeAudio.instanceTransport(requireHandle(), 2, false); }
    }

    public final void Stop() { Stop(true); }

    public final void Stop(boolean immediate) {
        synchronized (lock) { NativeAudio.instanceTransport(requireHandle(), 3, immediate); }
    }

    public final boolean getIsDisposed() {
        synchronized (lock) { return handle == 0L; }
    }

    public boolean getIsLooped() {
        synchronized (lock) { requireHandle(); return looped; }
    }

    public void setIsLooped(boolean value) {
        synchronized (lock) {
            NativeAudio.setInstanceBoolean(requireHandle(), value);
            looped = value;
        }
    }

    public final float getPan() { synchronized (lock) { requireHandle(); return pan; } }
    public final void setPan(float value) {
        SoundEffect.requireRange(value, -1.0f, 1.0f, "Pan");
        synchronized (lock) { NativeAudio.setInstanceFloat(requireHandle(), 2, value); pan = value; }
    }
    public final float getPitch() { synchronized (lock) { requireHandle(); return pitch; } }
    public final void setPitch(float value) {
        SoundEffect.requireRange(value, -1.0f, 1.0f, "Pitch");
        synchronized (lock) { NativeAudio.setInstanceFloat(requireHandle(), 1, value); pitch = value; }
    }
    public final SoundState getState() {
        synchronized (lock) { return SoundState.values()[NativeAudio.getInstanceState(requireHandle())]; }
    }
    public final float getVolume() { synchronized (lock) { requireHandle(); return volume; } }
    public final void setVolume(float value) {
        SoundEffect.requireRange(value, 0.0f, 1.0f, "Volume");
        synchronized (lock) { NativeAudio.setInstanceFloat(requireHandle(), 0, value); volume = value; }
    }

    protected void Dispose(boolean disposing) {
        closeInternal();
    }

    public final void Dispose() {
        Dispose(true);
    }

    @Override
    public final void close() {
        Dispose();
    }

    final long requireHandle() {
        if (handle == 0L) throw new IllegalStateException("SoundEffectInstance is disposed");
        return handle;
    }

    private void closeInternal() {
        long closing;
        synchronized (lock) {
            if (handle == 0L) return;
            closing = handle;
            NativeAudio.destroySoundEffectInstance(closing);
            handle = 0L;
        }
        if (parent != null) parent.childClosed(this);
        else NativeAudio.unregisterOwner(this);
    }
}
