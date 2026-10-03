package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Audio.AudioChannels;
import Microsoft.Xna.Framework.Audio.SoundEffect;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.Objects;

/**
 * A sound as {@code .cnb} carries it, before it is an audio resource.
 *
 * <p><strong>Compiled sound, not a playable one.</strong> This is the shape and the sample bytes
 * a content file holds; nothing here touches the audio device, so a tool with no sound card can
 * read, inspect and write one. {@link #toSoundEffect()} is the single step that crosses into
 * XNA's {@link SoundEffect} and therefore into the host's audio backend, and it is the only
 * method that can fail for want of hardware.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op. CNA copies
 * the samples on the way in, so the array a caller passed stays theirs, and samples read back out
 * are a fresh copy with no lifetime.
 */
public final class CnbSoundEffectData implements AutoCloseable {

    private final long handle;
    private boolean closed;

    CnbSoundEffectData(long handle) {
        this.handle = handle;
    }

    /**
     * Creates a compiled sound from its shape and its samples.
     *
     * <p>The description is stored as given. CNA's encoder is what checks the samples against the
     * declared shape, so a disagreement surfaces when the file is written rather than here.
     *
     * @param info the encoding, rate, shape and loop region
     * @param samples headerless little-endian sample bytes in the declared format; CNA copies them
     * @return the sound data, which the caller closes
     */
    public static CnbSoundEffectData create(CnbSoundEffectInfo info, byte[] samples) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(info, "info");
        Objects.requireNonNull(samples, "samples");
        Objects.requireNonNull(info.Format(), "info.Format");
        long[] fields = {
            info.Format().ordinal(), info.SampleRate(), info.Channels(),
            info.FrameCount(), info.LoopStart(), info.LoopLength(),
        };
        long[] sound = new long[1];
        CnbExtension.check("CnbSoundEffectData.create",
                NativeCnbRoutes.cnbSoundEffectDataCreate(fields, samples, sound));
        return new CnbSoundEffectData(sound[0]);
    }

    /**
     * Creates a signed 16-bit PCM sound with no loop region.
     *
     * <p>The convenience for the one format CNA's v1 container actually implements. The frame
     * count is derived from the bytes, so the two cannot disagree.
     *
     * @param sampleRate the rate in Hz
     * @param channels one for mono, two for stereo
     * @param samples signed 16-bit little-endian frames; CNA copies them
     * @return the sound data, which the caller closes
     */
    public static CnbSoundEffectData ofPcm16(int sampleRate, int channels, byte[] samples) {
        Objects.requireNonNull(samples, "samples");
        int frameSize = CnbAudioFormat.Pcm16.getFrameByteSize(channels);
        if (samples.length % frameSize != 0) {
            throw new IllegalArgumentException(
                    "samples do not divide into whole " + channels + "-channel frames");
        }
        return create(new CnbSoundEffectInfo(CnbAudioFormat.Pcm16, sampleRate, channels,
                samples.length / frameSize, 0, 0), samples);
    }

    /** Returns the sound's encoding, rate, shape and loop region. */
    public CnbSoundEffectInfo getInfo() {
        long[] values = new long[6];
        CnbExtension.check("CnbSoundEffectData.getInfo",
                NativeCnbRoutes.cnbSoundEffectDataGetInfo(open(), values));
        return new CnbSoundEffectInfo(CnbAudioFormat.fromValue(values[0]), (int) values[1],
                (int) values[2], (int) values[3], (int) values[4], (int) values[5]);
    }

    /**
     * Returns the sound's headerless sample bytes.
     *
     * @return a fresh copy of the samples, in the format {@link #getInfo()} reports
     */
    public byte[] readSamples() {
        long sound = open();
        long[] bytes = new long[1];
        // The size-then-copy protocol, asked of the sound itself rather than computed from the
        // shape: a reserved format has no frame size this build can name, and CNA still knows how
        // many bytes it is holding. A zero-capacity probe reports the size and refuses to write,
        // so BUFFER_TOO_SMALL is the expected answer here rather than a failure.
        int probe = NativeCnbRoutes.cnbSoundEffectDataCopySamples(sound, new byte[0], bytes);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("CnbSoundEffectData.readSamples", probe);
        }
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        CnbExtension.check("CnbSoundEffectData.readSamples", NativeCnbRoutes
                .cnbSoundEffectDataCopySamples(sound, destination, bytes));
        return destination;
    }

    /**
     * Creates an ordinary XNA {@link SoundEffect} from this compiled sound.
     *
     * <p>This is the step that needs the host's audio backend, and the only one that does. The
     * loop region crosses unchanged because both sides state it in frames; a sound with no loop
     * becomes a {@code SoundEffect} whose loop is its whole length, which is XNA's own rule for a
     * zero-length loop rather than an interpretation added here.
     *
     * @return the sound effect, which the caller owns and disposes as any other
     * @throws ContentNotSupportedException when the sound is not in a format XNA can play, or
     *         declares a channel count XNA has no {@link AudioChannels} for
     */
    public SoundEffect toSoundEffect() {
        CnbSoundEffectInfo info = getInfo();
        if (info.Format() != CnbAudioFormat.Pcm16) {
            throw new ContentNotSupportedException("this .cnb sound is " + info.Format()
                    + "; XNA's SoundEffect takes signed 16-bit PCM");
        }
        AudioChannels channels = switch (info.Channels()) {
            case 1 -> AudioChannels.Mono;
            case 2 -> AudioChannels.Stereo;
            default -> throw new ContentNotSupportedException("this .cnb sound declares "
                    + info.Channels() + " channels; XNA names mono and stereo only");
        };
        byte[] samples = readSamples();
        // XNA's byte[] is Java's int[] here, because a C# byte is unsigned and a Java one is not.
        int[] buffer = new int[samples.length];
        for (int index = 0; index < samples.length; index++) {
            buffer[index] = samples[index] & 0xFF;
        }
        return new SoundEffect(buffer, 0, buffer.length, info.SampleRate(), channels,
                info.LoopStart(), info.LoopLength());
    }

    long handle() {
        return open();
    }

    /** Releases the sound data. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbSoundEffectData.close",
                NativeCnbRoutes.cnbSoundEffectDataDestroy(handle));
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnbSoundEffectData is closed");
            }
        }
        return handle;
    }
}
