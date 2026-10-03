package Microsoft.Xna.Framework.Media;

import java.util.AbstractList;
import java.util.List;

/** Stable read-only 256-bin visualization buffers filled by {@link MediaPlayer}. */
public class VisualizationData {
    private static final int SIZE = 256;
    private final float[] frequencies = new float[SIZE];
    private final float[] samples = new float[SIZE];
    private final List<Float> frequencyView = new FloatArrayView(frequencies);
    private final List<Float> sampleView = new FloatArrayView(samples);

    public VisualizationData() { }

    public final List<Float> getFrequencies() { return frequencyView; }

    public final List<Float> getSamples() { return sampleView; }

    void setNativeValues(float[] newFrequencies, float[] newSamples) {
        if (newFrequencies.length != SIZE || newSamples.length != SIZE) {
            throw new IllegalArgumentException("Visualization buffers must contain 256 values");
        }
        System.arraycopy(newFrequencies, 0, frequencies, 0, SIZE);
        System.arraycopy(newSamples, 0, samples, 0, SIZE);
    }

    private static final class FloatArrayView extends AbstractList<Float> {
        private final float[] values;
        private FloatArrayView(float[] values) { this.values = values; }
        @Override public Float get(int index) { return values[index]; }
        @Override public int size() { return values.length; }
    }
}
