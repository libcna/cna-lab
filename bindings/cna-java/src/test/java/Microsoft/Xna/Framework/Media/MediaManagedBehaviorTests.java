package Microsoft.Xna.Framework.Media;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

final class MediaManagedBehaviorTests {

    @Test
    void VisualizationDataExposesStableReadOnlyZeroedBuffers() {
        VisualizationData data = new VisualizationData();

        assertSame(data.getFrequencies(), data.getFrequencies());
        assertSame(data.getSamples(), data.getSamples());
        assertEquals(256, data.getFrequencies().size());
        assertEquals(256, data.getSamples().size());
        assertTrue(data.getFrequencies().stream().allMatch(value -> value == 0f));
        assertTrue(data.getSamples().stream().allMatch(value -> value == 0f));
        assertThrows(UnsupportedOperationException.class,
                () -> data.getFrequencies().set(0, 1f));
        assertThrows(IndexOutOfBoundsException.class,
                () -> data.getSamples().get(256));
    }

    @Test
    void VisualizationNativeUpdatesPreserveBothListIdentities() {
        VisualizationData data = new VisualizationData();
        List<Float> frequencies = data.getFrequencies();
        List<Float> samples = data.getSamples();
        float[] newFrequencies = new float[256];
        float[] newSamples = new float[256];
        newFrequencies[3] = 0.25f;
        newSamples[7] = -0.5f;

        data.setNativeValues(newFrequencies, newSamples);

        assertSame(frequencies, data.getFrequencies());
        assertSame(samples, data.getSamples());
        assertEquals(0.25f, frequencies.get(3));
        assertEquals(-0.5f, samples.get(7));
        assertThrows(IllegalArgumentException.class,
                () -> data.setNativeValues(new float[1], new float[256]));
    }

    @Test
    void MediaEnumsKeepAuthoritativeXnaIdentities() {
        assertArrayEquals(new MediaState[] {
                MediaState.Stopped, MediaState.Playing, MediaState.Paused
        }, MediaState.values());
        assertEquals(0, MediaSourceType.LocalDevice.getValue());
        assertEquals(4, MediaSourceType.WindowsMediaConnect.getValue());
        assertArrayEquals(new VideoSoundtrackType[] {
                VideoSoundtrackType.Music,
                VideoSoundtrackType.Dialog,
                VideoSoundtrackType.MusicAndDialog
        }, VideoSoundtrackType.values());
    }

    @Test
    void NullValidationHappensBeforeAnyNativeGameLookup() {
        assertThrows(NullPointerException.class, () -> MediaPlayer.Play((Song)null));
        assertThrows(NullPointerException.class,
                () -> MediaPlayer.Play((SongCollection)null));
        assertThrows(NullPointerException.class,
                () -> MediaPlayer.Play((SongCollection)null, 0));
        assertThrows(NullPointerException.class,
                () -> MediaPlayer.GetVisualizationData(null));
        assertThrows(NullPointerException.class, () -> new MediaLibrary(null));
    }
}
