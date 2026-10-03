package Microsoft.Xna.Framework.Audio;

import java.time.Duration;
import java.util.Objects;

/** XNA's intentionally float-based 16-bit PCM sizing arithmetic. */
final class AudioMath {
    private AudioMath() {
    }

    static void validateFormat(int sampleRate, AudioChannels channels) {
        if (sampleRate < 8000 || sampleRate > 48000) {
            throw new IllegalArgumentException("sampleRate must be between 8000 and 48000");
        }
        Objects.requireNonNull(channels, "channels");
    }

    static int blockAlignment(AudioChannels channels) {
        return Objects.requireNonNull(channels, "channels").getValue() * 2;
    }

    static Duration durationFromSize(int sizeInBytes, int sampleRate,
            AudioChannels channels) {
        if (sizeInBytes < 0) {
            throw new IllegalArgumentException("sizeInBytes must not be negative");
        }
        validateFormat(sampleRate, channels);
        if (sizeInBytes == 0) {
            return Duration.ZERO;
        }
        int samples = sizeInBytes / blockAlignment(channels);
        float milliseconds = (float) samples * 1000.0f / (float) sampleRate;
        return Duration.ofMillis((long) (milliseconds + 0.5f));
    }

    static int sizeFromDuration(Duration duration, int sampleRate,
            AudioChannels channels) {
        Duration selected = Objects.requireNonNull(duration, "duration");
        double milliseconds = selected.getSeconds() * 1000.0
                + selected.getNano() / 1_000_000.0;
        if (selected.isNegative() || milliseconds > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("duration is outside the supported range");
        }
        validateFormat(sampleRate, channels);
        if (selected.isZero()) {
            return 0;
        }
        double samplesValue = milliseconds * ((float) sampleRate / 1000.0f);
        if (samplesValue > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("duration is outside the supported range");
        }
        int samples = (int) samplesValue;
        try {
            int alignedSamples = Math.addExact(samples, samples % channels.getValue());
            return Math.multiplyExact(alignedSamples, blockAlignment(channels));
        } catch (ArithmeticException overflow) {
            throw new IllegalArgumentException("duration is outside the supported range", overflow);
        }
    }
}
