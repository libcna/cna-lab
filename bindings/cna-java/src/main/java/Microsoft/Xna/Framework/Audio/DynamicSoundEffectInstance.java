package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeAudio;

import java.time.Duration;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Streaming PCM playback instance with dispatcher-driven buffer callbacks. */
public final class DynamicSoundEffectInstance extends SoundEffectInstance {
    private final int sampleRate;
    private final AudioChannels channels;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> bufferNeededListeners =
            new CopyOnWriteArrayList<>();
    private long bufferNeededRegistration;
    private Throwable callbackFailure;

    public DynamicSoundEffectInstance(int sampleRate, AudioChannels channels) {
        super(create(sampleRate, channels));
        this.sampleRate = sampleRate;
        this.channels = channels;
        NativeAudio.registerOwner(this);
    }

    private static long create(int sampleRate, AudioChannels channels) {
        AudioMath.validateFormat(sampleRate, channels);
        return NativeAudio.createDynamicSoundEffect(sampleRate, channels.getValue());
    }

    public void addBufferNeededListener(EventHandler<EventArgs> listener) {
        rethrowCallbackFailure();
        EventHandler<EventArgs> selected = Objects.requireNonNull(listener, "listener");
        requireHandle();
        if (bufferNeededRegistration == 0L) {
            bufferNeededRegistration = NativeAudio.subscribeBufferNeeded(requireHandle(), this);
        }
        bufferNeededListeners.add(selected);
    }

    public void removeBufferNeededListener(EventHandler<EventArgs> listener) {
        bufferNeededListeners.remove(listener);
        if (bufferNeededListeners.isEmpty() && bufferNeededRegistration != 0L) {
            NativeAudio.unsubscribe(bufferNeededRegistration);
            bufferNeededRegistration = 0L;
        }
    }

    public Duration GetSampleDuration(int sizeInBytes) {
        rethrowCallbackFailure();
        requireHandle();
        return AudioMath.durationFromSize(sizeInBytes, sampleRate, channels);
    }

    public int GetSampleSizeInBytes(Duration duration) {
        rethrowCallbackFailure();
        requireHandle();
        return AudioMath.sizeFromDuration(duration, sampleRate, channels);
    }

    @Override
    public void Play() {
        rethrowCallbackFailure();
        super.Play();
    }

    @Override
    public boolean getIsLooped() {
        rethrowCallbackFailure();
        requireHandle();
        return false;
    }

    @Override
    public void setIsLooped(boolean value) {
        rethrowCallbackFailure();
        requireHandle();
        if (value) throw new IllegalArgumentException("Dynamic sound cannot be looped");
    }

    public int getPendingBufferCount() {
        rethrowCallbackFailure();
        return NativeAudio.getPendingBufferCount(requireHandle());
    }

    public void SubmitBuffer(int[] buffer) {
        SubmitBuffer(buffer, 0, buffer.length);
    }

    public void SubmitBuffer(int[] buffer, int offset, int count) {
        rethrowCallbackFailure();
        Objects.requireNonNull(buffer, "buffer");
        int alignment = AudioMath.blockAlignment(channels);
        if (buffer.length == 0 || buffer.length % alignment != 0) {
            throw new IllegalArgumentException("buffer must contain aligned audio data");
        }
        if (offset < 0 || offset >= buffer.length || offset % alignment != 0) {
            throw new IllegalArgumentException("offset is outside or misaligned");
        }
        int end;
        try { end = Math.addExact(offset, count); }
        catch (ArithmeticException overflow) {
            throw new IllegalArgumentException("offset and count overflow", overflow);
        }
        if (count <= 0 || end > buffer.length || count % alignment != 0) {
            throw new IllegalArgumentException("count is outside or misaligned");
        }
        NativeAudio.submitDynamicBuffer(requireHandle(),
                SoundEffect.unsignedBytes(buffer), offset, count);
    }

    @Override
    protected void Dispose(boolean disposing) {
        long registration = bufferNeededRegistration;
        Throwable failure;
        synchronized (this) {
            failure = callbackFailure;
            callbackFailure = null;
        }
        try {
            if (registration != 0L) {
                NativeAudio.unsubscribe(registration);
                bufferNeededRegistration = 0L;
            }
            super.Dispose(disposing);
        } catch (Throwable exception) {
            failure = AudioEngine.append(failure, exception);
        }
        if (getIsDisposed()) bufferNeededListeners.clear();
        AudioEngine.rethrow(failure);
    }

    @SuppressWarnings("unused")
    private void nativeBufferNeeded() {
        if (getIsDisposed()) return;
        Throwable failure = null;
        for (EventHandler<EventArgs> listener : bufferNeededListeners) {
            try { listener.invoke(this, EventArgs.Empty); }
            catch (Throwable exception) {
                if (failure == null) failure = exception; else failure.addSuppressed(exception);
            }
        }
        if (failure != null) {
            synchronized (this) {
                if (callbackFailure == null) callbackFailure = failure;
                else callbackFailure.addSuppressed(failure);
            }
        }
    }

    private void rethrowCallbackFailure() {
        Throwable failure;
        synchronized (this) { failure = callbackFailure; callbackFailure = null; }
        if (failure instanceof RuntimeException runtime) throw runtime;
        if (failure instanceof Error error) throw error;
        if (failure != null) throw new RuntimeException("BufferNeeded listener failed", failure);
    }
}
