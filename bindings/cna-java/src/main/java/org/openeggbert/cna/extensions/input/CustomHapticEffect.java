package org.openeggbert.cna.extensions.input;

import java.time.Duration;
import java.util.List;

/**
 * A waveform the game supplies sample by sample.
 *
 * <p>The samples are unsigned 16-bit values, so they are carried as {@code int} rather than
 * {@code short}: a Java {@code short} cannot hold 40000 without becoming negative, and CNA's
 * sample is not signed.
 *
 * <p>CNA keeps the sample buffer beside the descriptor rather than inside it, so an effect value
 * owns no heap memory. The samples here are copied into an immutable list for the same reason:
 * a game can keep the effect and reuse it without the device holding a reference to a buffer the
 * game may still be writing.
 *
 * @param Direction where the force comes from, used when the waveform drives one channel
 * @param Length how long it plays, or {@code null} to play until stopped
 * @param Delay how long to wait before it starts
 * @param Channels how many axes the waveform drives
 * @param SamplePeriod how long each sample lasts
 * @param Samples the waveform, each value from 0 to 65535
 * @param Trigger the device button that replays it, or {@link HapticTrigger#NONE}
 * @param Envelope how it ramps in and out, or {@link HapticEnvelope#NONE}
 */
public record CustomHapticEffect(
        HapticDirection Direction,
        Duration Length,
        Duration Delay,
        int Channels,
        Duration SamplePeriod,
        List<Integer> Samples,
        HapticTrigger Trigger,
        HapticEnvelope Envelope) implements HapticEffect {

    /** Copies the samples, and refuses one that is not an unsigned 16-bit value. */
    public CustomHapticEffect {
        Samples = List.copyOf(Samples);
        for (int sample : Samples) {
            if (sample < 0 || sample > 0xFFFF) {
                throw new IllegalArgumentException(
                        "a custom sample is an unsigned 16-bit value; got " + sample);
            }
        }
        if (Channels <= 0 || Channels > 0xFF) {
            throw new IllegalArgumentException(
                    "channels must be between 1 and 255; got " + Channels);
        }
    }

    @Override
    public long[] encode() {
        long[] effect = HapticEffectLayout.of(12, Length());
        HapticEffectLayout.direction(effect, Direction);
        HapticEffectLayout.schedule(effect, Delay, Trigger);
        HapticEffectLayout.envelope(effect, Envelope);
        effect[HapticEffectLayout.CUSTOM_PERIOD] =
                HapticEffectLayout.milliseconds(SamplePeriod, "samplePeriod");
        effect[HapticEffectLayout.CUSTOM_CHANNELS] = Channels;
        return effect;
    }

    @Override
    public List<Integer> samples() {
        return Samples;
    }
}
