package Microsoft.Xna.Framework.Audio;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.NativeAudio;

import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Runtime-owned microphone device. A NULL/headless CNA backend reports no instances. */
public final class Microphone {
    private static final Object cacheLock = new Object();
    private static long cacheGeneration = -1L;
    private static List<Microphone> cachedAll = List.of();

    private final int index;
    private final int sampleRate;
    private final long generation;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> bufferReadyListeners =
            new CopyOnWriteArrayList<>();
    private long bufferReadyRegistration;
    private Throwable callbackFailure;

    public final String Name;

    private Microphone(int index, long generation) {
        this.index = index;
        this.generation = generation;
        sampleRate = NativeAudio.getMicrophoneInt(index, 2);
        Name = NativeAudio.getMicrophoneName(index);
    }

    public static List<Microphone> getAll() {
        long generation = NativeAudio.audioGeneration();
        synchronized (cacheLock) {
            int count = NativeAudio.getMicrophoneCount();
            if (cacheGeneration != generation || cachedAll.size() != count) {
                List<Microphone> values = new ArrayList<>(count);
                for (int i = 0; i < count; i++) values.add(new Microphone(i, generation));
                cachedAll = List.copyOf(values);
                cacheGeneration = generation;
            }
            return cachedAll;
        }
    }

    public static Microphone getDefault() {
        int index = NativeAudio.getDefaultMicrophoneIndex();
        if (index < 0) return null;
        List<Microphone> all = getAll();
        if (index >= all.size()) {
            throw new IllegalStateException("Default microphone index is outside the device list");
        }
        return all.get(index);
    }

    public void addBufferReadyListener(EventHandler<EventArgs> listener) {
        rethrowCallbackFailure();
        requireCurrent();
        EventHandler<EventArgs> selected = Objects.requireNonNull(listener, "listener");
        if (bufferReadyRegistration == 0L) {
            bufferReadyRegistration = NativeAudio.subscribeMicrophone(index, this);
        }
        bufferReadyListeners.add(selected);
    }

    public void removeBufferReadyListener(EventHandler<EventArgs> listener) {
        bufferReadyListeners.remove(listener);
        if (bufferReadyListeners.isEmpty() && bufferReadyRegistration != 0L) {
            NativeAudio.unsubscribe(bufferReadyRegistration);
            bufferReadyRegistration = 0L;
        }
    }

    public Duration getBufferDuration() {
        rethrowCallbackFailure();
        requireCurrent();
        return NativeAudio.getMicrophoneDuration(index);
    }

    public void setBufferDuration(Duration value) {
        requireCurrent();
        Duration selected = Objects.requireNonNull(value, "value");
        long milliseconds;
        try { milliseconds = selected.toMillis(); }
        catch (ArithmeticException overflow) {
            throw new IllegalArgumentException("BufferDuration is outside 100-1000 ms", overflow);
        }
        if (selected.isNegative() || selected.minusMillis(milliseconds).isZero() == false
                || milliseconds < 100 || milliseconds > 1000 || milliseconds % 10 != 0) {
            throw new IllegalArgumentException("BufferDuration must be 100-1000 ms in 10 ms steps");
        }
        NativeAudio.setMicrophoneDuration(index, selected);
    }

    public boolean getIsHeadset() {
        rethrowCallbackFailure();
        requireCurrent();
        return NativeAudio.getMicrophoneInt(index, 1) != 0;
    }

    public int getSampleRate() { rethrowCallbackFailure(); requireCurrent(); return sampleRate; }

    public MicrophoneState getState() {
        rethrowCallbackFailure();
        requireCurrent();
        return NativeAudio.getMicrophoneInt(index, 3) == 0
                ? MicrophoneState.Started : MicrophoneState.Stopped;
    }

    public int GetData(int[] buffer) {
        if (buffer == null || buffer.length == 0 || (buffer.length & 1) != 0) {
            throw new IllegalArgumentException("buffer must contain aligned audio storage");
        }
        return GetData(buffer, 0, buffer.length);
    }

    public int GetData(int[] buffer, int offset, int count) {
        rethrowCallbackFailure();
        Objects.requireNonNull(buffer, "buffer");
        if (buffer.length == 0 || (buffer.length & 1) != 0) {
            throw new IllegalArgumentException("buffer must contain aligned audio storage");
        }
        int end;
        try { end = Math.addExact(offset, count); }
        catch (ArithmeticException overflow) {
            throw new IllegalArgumentException("offset and count overflow", overflow);
        }
        if (offset < 0 || offset >= buffer.length || (offset & 1) != 0
                || count <= 0 || end <= 0 || end > buffer.length || (count & 1) != 0
                || AudioMath.durationFromSize(count, sampleRate, AudioChannels.Mono).isZero()) {
            throw new IllegalArgumentException("offset and count must select aligned audio");
        }
        if (getState() != MicrophoneState.Started) return 0;
        byte[] bytes = new byte[buffer.length];
        int read = NativeAudio.getMicrophoneData(index, bytes, offset, count);
        for (int i = offset; i < offset + read; i++) buffer[i] = Byte.toUnsignedInt(bytes[i]);
        return read;
    }

    public Duration GetSampleDuration(int sizeInBytes) {
        rethrowCallbackFailure();
        requireCurrent();
        return AudioMath.durationFromSize(sizeInBytes, sampleRate, AudioChannels.Mono);
    }

    public int GetSampleSizeInBytes(Duration duration) {
        rethrowCallbackFailure();
        requireCurrent();
        return AudioMath.sizeFromDuration(duration, sampleRate, AudioChannels.Mono);
    }

    public void Start() {
        rethrowCallbackFailure(); requireCurrent(); NativeAudio.microphoneTransport(index, true);
    }
    public void Stop() {
        rethrowCallbackFailure(); requireCurrent(); NativeAudio.microphoneTransport(index, false);
    }

    @SuppressWarnings("unused")
    private void nativeBufferReady() {
        Throwable failure = null;
        for (EventHandler<EventArgs> listener : bufferReadyListeners) {
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
        if (failure != null) throw new RuntimeException("BufferReady listener failed", failure);
    }

    private void requireCurrent() {
        if (generation != NativeAudio.audioGeneration()) {
            throw new IllegalStateException("Microphone belongs to a disposed Game");
        }
    }
}
