package Microsoft.Xna.Framework.Audio;

import System.IO.Stream;
import org.openeggbert.cna.internal.NativeAudio;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/** XNA-compatible 16-bit PCM or encoded sound resource backed by CNA. */
public final class SoundEffect implements AutoCloseable {
    private final Object lock = new Object();
    private final List<SoundEffectInstance> children = new ArrayList<>();
    private final Duration duration;
    private long handle;

    public SoundEffect(int[] buffer, int sampleRate, AudioChannels channels) {
        if (buffer == null || buffer.length == 0) {
            throw new IllegalArgumentException("buffer must contain audio data");
        }
        AudioMath.validateFormat(sampleRate, channels);
        int alignment = AudioMath.blockAlignment(channels);
        if (buffer.length % alignment != 0) {
            throw new IllegalArgumentException("buffer length is not sample aligned");
        }
        handle = NativeAudio.createSoundEffect(
                unsignedBytes(buffer), 0, buffer.length, sampleRate,
                channels.getValue(), 0, 0);
        duration = AudioMath.durationFromSize(buffer.length, sampleRate, channels);
        NativeAudio.registerOwner(this);
    }

    public SoundEffect(int[] buffer, int offset, int count, int sampleRate,
            AudioChannels channels, int loopStart, int loopLength) {
        AudioMath.validateFormat(sampleRate, channels);
        Objects.requireNonNull(buffer, "buffer");
        int alignment = AudioMath.blockAlignment(channels);
        if (buffer.length == 0 || buffer.length % alignment != 0) {
            throw new IllegalArgumentException("buffer must contain aligned audio data");
        }
        if (offset < 0 || offset >= buffer.length || offset % alignment != 0) {
            throw new IllegalArgumentException("offset is outside or misaligned");
        }
        int end;
        try {
            end = Math.addExact(offset, count);
        } catch (ArithmeticException overflow) {
            throw new IllegalArgumentException("offset and count overflow", overflow);
        }
        if (count <= 0 || end > buffer.length || count % alignment != 0) {
            throw new IllegalArgumentException("count is outside or misaligned");
        }
        int sampleCount = count / alignment;
        int loopEnd;
        try {
            loopEnd = Math.addExact(loopStart, loopLength);
        } catch (ArithmeticException overflow) {
            throw new IllegalArgumentException("loop range overflow", overflow);
        }
        if (loopStart < 0 || loopStart > sampleCount
                || loopLength < 0 || loopEnd > sampleCount) {
            throw new IllegalArgumentException("loop range is outside the selected audio");
        }
        if (loopLength == 0) {
            loopStart = 0;
            loopLength = sampleCount;
        }
        handle = NativeAudio.createSoundEffect(unsignedBytes(buffer), offset, count,
                sampleRate, channels.getValue(), loopStart, loopLength);
        duration = AudioMath.durationFromSize(count, sampleRate, channels);
        NativeAudio.registerOwner(this);
    }

    private SoundEffect(long handle) {
        if (handle == 0L) throw new NoAudioHardwareException("SoundEffect creation failed");
        this.handle = handle;
        duration = NativeAudio.duration(handle);
        NativeAudio.registerOwner(this);
    }

    public static SoundEffect FromStream(Stream stream) {
        Objects.requireNonNull(stream, "stream");
        try {
            return new SoundEffect(NativeAudio.createSoundEffectEncoded(stream.readAllBytes()));
        } catch (IOException exception) {
            throw new UncheckedIOException(exception);
        }
    }

    public static Duration GetSampleDuration(
            int sizeInBytes, int sampleRate, AudioChannels channels) {
        return AudioMath.durationFromSize(sizeInBytes, sampleRate, channels);
    }

    public static int GetSampleSizeInBytes(
            Duration duration, int sampleRate, AudioChannels channels) {
        return AudioMath.sizeFromDuration(duration, sampleRate, channels);
    }

    public static float getDistanceScale() { return NativeAudio.getSoundSetting(1); }
    public static void setDistanceScale(float value) {
        if (value < 0.0f || Float.isNaN(value)) {
            throw new IllegalArgumentException("DistanceScale must not be negative or NaN");
        }
        NativeAudio.setSoundSetting(1, value <= Float.MIN_VALUE ? Float.MIN_VALUE : value);
    }
    public static float getDopplerScale() { return NativeAudio.getSoundSetting(2); }
    public static void setDopplerScale(float value) {
        if (value < 0.0f || Float.isNaN(value)) {
            throw new IllegalArgumentException("DopplerScale must not be negative or NaN");
        }
        NativeAudio.setSoundSetting(2, value);
    }
    public static float getMasterVolume() { return NativeAudio.getSoundSetting(0); }
    public static void setMasterVolume(float value) {
        requireRange(value, 0.0f, 1.0f, "MasterVolume");
        NativeAudio.setSoundSetting(0, value);
    }
    public static float getSpeedOfSound() { return NativeAudio.getSoundSetting(3); }
    public static void setSpeedOfSound(float value) {
        if (value <= 0.0f || Float.isNaN(value)) {
            throw new IllegalArgumentException("SpeedOfSound must be positive and not NaN");
        }
        NativeAudio.setSoundSetting(3, value);
    }

    public SoundEffectInstance CreateInstance() {
        synchronized (lock) {
            long selected = requireHandle();
            SoundEffectInstance instance = new SoundEffectInstance(
                    this, NativeAudio.createSoundEffectInstance(selected));
            children.add(instance);
            return instance;
        }
    }

    public boolean Play() {
        synchronized (lock) {
            recycleFireAndForget();
            return NativeAudio.playSoundEffect(requireHandle(), 1.0f, 0.0f, 0.0f, false);
        }
    }

    public boolean Play(float volume, float pitch, float pan) {
        requireRange(volume, 0.0f, 1.0f, "volume");
        requireRange(pitch, -1.0f, 1.0f, "pitch");
        requireRange(pan, -1.0f, 1.0f, "pan");
        synchronized (lock) {
            recycleFireAndForget();
            return NativeAudio.playSoundEffect(requireHandle(), volume, pitch, pan, true);
        }
    }

    public Duration getDuration() {
        synchronized (lock) {
            requireHandle();
            return duration;
        }
    }

    public boolean getIsDisposed() {
        synchronized (lock) { return handle == 0L; }
    }

    public String getName() {
        synchronized (lock) { return NativeAudio.getSoundEffectName(requireHandle()); }
    }

    public void setName(String value) {
        Objects.requireNonNull(value, "value");
        synchronized (lock) { NativeAudio.setSoundEffectName(requireHandle(), value); }
    }

    public void Dispose() {
        List<SoundEffectInstance> closing;
        long closingHandle;
        synchronized (lock) {
            if (handle == 0L) return;
            closing = new ArrayList<>(children);
            closingHandle = handle;
        }
        Throwable failure = null;
        for (int i = closing.size() - 1; i >= 0; i--) {
            try { closing.get(i).close(); }
            catch (Throwable exception) { failure = AudioEngine.append(failure, exception); }
        }
        if (failure == null) {
            try {
                NativeAudio.destroySoundEffect(closingHandle);
                synchronized (lock) {
                    if (handle == closingHandle) {
                        handle = 0L;
                        children.clear();
                    }
                }
                NativeAudio.unregisterOwner(this);
            } catch (Throwable exception) {
                failure = AudioEngine.append(failure, exception);
            }
        }
        AudioEngine.rethrow(failure);
    }

    @Override
    public void close() {
        Dispose();
    }

    final long requireHandle() {
        if (handle == 0L) throw new IllegalStateException("SoundEffect is disposed");
        return handle;
    }

    final void childClosed(SoundEffectInstance child) {
        synchronized (lock) { children.remove(child); }
    }

    private void recycleFireAndForget() {
        children.removeIf(child -> child.getIsDisposed());
    }

    static void requireRange(float value, float minimum, float maximum, String name) {
        if (value < minimum || value > maximum || Float.isNaN(value)) {
            throw new IllegalArgumentException(name + " is outside [" + minimum + ", " + maximum + "]");
        }
    }

    static byte[] unsignedBytes(int[] values) {
        byte[] result = new byte[values.length];
        for (int i = 0; i < values.length; i++) {
            if ((values[i] & ~0xff) != 0) {
                throw new IllegalArgumentException("buffer element is outside the Byte range");
            }
            result[i] = (byte) values[i];
        }
        return result;
    }
}
