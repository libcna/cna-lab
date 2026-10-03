package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Fixed-size device sampler collection for one shader stage. */
public final class SamplerStateCollection {

    private final GraphicsDevice graphicsDevice;
    private final int shaderStage;
    private final SamplerState[] states;

    SamplerStateCollection(GraphicsDevice graphicsDevice, int shaderStage, int slotCount) {
        this.graphicsDevice = Objects.requireNonNull(graphicsDevice, "graphicsDevice");
        this.shaderStage = shaderStage;
        this.states = new SamplerState[slotCount];
    }

    public SamplerState get(int index) {
        validateIndex(index);
        graphicsDevice.ensureOpen();
        int[] integers = new int[6];
        float[] bias = new float[1];
        NativeBindings.getGraphicsDeviceSamplerState(
                graphicsDevice, shaderStage, index, integers, bias);
        SamplerState result = SamplerState.fromNative(
                integers, bias[0], graphicsDevice, states[index]);
        states[index] = result;
        return result;
    }

    public void set(int index, SamplerState value) {
        validateIndex(index);
        graphicsDevice.ensureOpen();
        SamplerState selected = Objects.requireNonNull(value, "value");
        if (states[index] == selected) {
            return;
        }
        int[] integers = selected.snapshotIntegersForBinding();
        float bias = selected.snapshotBiasForBinding();
        NativeBindings.setGraphicsDeviceSamplerState(
                graphicsDevice, shaderStage, index, integers, bias);
        selected.bind(graphicsDevice);
        states[index] = selected;
    }

    final void invalidate() {
        java.util.Arrays.fill(states, null);
    }

    private void validateIndex(int index) {
        if (index < 0 || index >= states.length) {
            throw new IndexOutOfBoundsException(
                    "Sampler-state index " + index + " is outside 0.." + (states.length - 1));
        }
    }
}
