package org.openeggbert.cna.extensions.content;

/**
 * The shape of a compiled sound a {@code .cnb} file carries.
 *
 * <p>Counts are in <em>frames</em>, that is samples per channel, which is also how XNA states a
 * {@code SoundEffect}'s loop region. The byte count is a consequence of the format and the
 * channel count rather than a field, so the two can never disagree.
 *
 * @param Format how the samples are encoded
 * @param SampleRate the rate in Hz
 * @param Channels one for mono, two for stereo
 * @param FrameCount how many sample frames there are
 * @param LoopStart the first frame of the loop region
 * @param LoopLength how many frames the loop region spans; zero means no loop
 */
public record CnbSoundEffectInfo(
        CnbAudioFormat Format,
        int SampleRate,
        int Channels,
        int FrameCount,
        int LoopStart,
        int LoopLength) {

    /** Returns how many sample bytes a sound of this shape occupies. */
    public long getByteCount() {
        return (long) Format.getFrameByteSize(Channels) * FrameCount;
    }
}
