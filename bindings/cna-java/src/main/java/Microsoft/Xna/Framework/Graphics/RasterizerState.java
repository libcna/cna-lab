package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Complete XNA rasterizer-state descriptor, copied to CNA when it is bound. */
public class RasterizerState extends GraphicsResource {

    public static final RasterizerState CullClockwise = preset(
            "RasterizerState.CullClockwise", CullMode.CullClockwiseFace);
    public static final RasterizerState CullCounterClockwise = preset(
            "RasterizerState.CullCounterClockwise", CullMode.CullCounterClockwiseFace);
    public static final RasterizerState CullNone = preset(
            "RasterizerState.CullNone", CullMode.None);

    private CullMode cullMode = CullMode.CullCounterClockwiseFace;
    private float depthBias;
    private FillMode fillMode = FillMode.Solid;
    private boolean multiSampleAntiAlias = true;
    private boolean scissorTestEnable;
    private float slopeScaleDepthBias;
    private boolean bound;

    public RasterizerState() {
        super();
    }

    public final CullMode getCullMode() {
        return cullMode;
    }

    public final void setCullMode(CullMode value) {
        ensureMutable();
        cullMode = Objects.requireNonNull(value, "value");
    }

    public final float getDepthBias() {
        return depthBias;
    }

    public final void setDepthBias(float value) {
        ensureMutable();
        depthBias = value;
    }

    public final FillMode getFillMode() {
        return fillMode;
    }

    public final void setFillMode(FillMode value) {
        ensureMutable();
        fillMode = Objects.requireNonNull(value, "value");
    }

    public final boolean getMultiSampleAntiAlias() {
        return multiSampleAntiAlias;
    }

    public final void setMultiSampleAntiAlias(boolean value) {
        ensureMutable();
        multiSampleAntiAlias = value;
    }

    public final boolean getScissorTestEnable() {
        return scissorTestEnable;
    }

    public final void setScissorTestEnable(boolean value) {
        ensureMutable();
        scissorTestEnable = value;
    }

    public final float getSlopeScaleDepthBias() {
        return slopeScaleDepthBias;
    }

    public final void setSlopeScaleDepthBias(float value) {
        ensureMutable();
        slopeScaleDepthBias = value;
    }

    @Override
    protected void Dispose(boolean arg0) {
        super.Dispose(arg0);
    }

    final int[] snapshotIntegersForBinding() {
        ensureCanBind();
        return nativeIntegers();
    }

    final float[] snapshotFloatsForBinding() {
        ensureCanBind();
        return nativeFloats();
    }

    final void bind(GraphicsDevice device) {
        ensureCanBind();
        attachGraphicsDevice(device);
        bound = true;
    }

    final boolean nativeEquals(int[] integers, float[] floats) {
        return integers != null && integers.length == 4
                && floats != null && floats.length == 2
                && cullMode.ordinal() == integers[0]
                && fillMode.ordinal() == integers[1]
                && (multiSampleAntiAlias ? 1 : 0) == integers[2]
                && (scissorTestEnable ? 1 : 0) == integers[3]
                && Float.floatToIntBits(depthBias) == Float.floatToIntBits(floats[0])
                && Float.floatToIntBits(slopeScaleDepthBias) == Float.floatToIntBits(floats[1]);
    }

    static RasterizerState fromNative(
            int[] integers,
            float[] floats,
            GraphicsDevice device,
            RasterizerState current) {
        requireDescriptor(integers, floats);
        if (current != null && current.nativeEquals(integers, floats)) {
            return current;
        }
        for (RasterizerState preset :
                new RasterizerState[]{CullCounterClockwise, CullClockwise, CullNone}) {
            if (!preset.getIsDisposed() && preset.nativeEquals(integers, floats)) {
                preset.bind(device);
                return preset;
            }
        }
        RasterizerState result = new RasterizerState();
        result.cullMode = enumAt(CullMode.values(), integers[0], "CullMode");
        result.fillMode = enumAt(FillMode.values(), integers[1], "FillMode");
        result.multiSampleAntiAlias = booleanAt(integers[2], "MultiSampleAntiAlias");
        result.scissorTestEnable = booleanAt(integers[3], "ScissorTestEnable");
        result.depthBias = floats[0];
        result.slopeScaleDepthBias = floats[1];
        result.bind(device);
        return result;
    }

    private static RasterizerState preset(String name, CullMode cullMode) {
        RasterizerState result = new RasterizerState();
        result.cullMode = cullMode;
        result.setName(name);
        result.bound = true;
        return result;
    }

    private int[] nativeIntegers() {
        return new int[]{
                cullMode.ordinal(),
                fillMode.ordinal(),
                multiSampleAntiAlias ? 1 : 0,
                scissorTestEnable ? 1 : 0
        };
    }

    private float[] nativeFloats() {
        return new float[]{depthBias, slopeScaleDepthBias};
    }

    private void ensureMutable() {
        if (bound) {
            throw new IllegalStateException(
                    "RasterizerState cannot be modified after it has been bound to a GraphicsDevice");
        }
    }

    private void ensureCanBind() {
        if (getIsDisposed()) {
            throw new IllegalStateException("RasterizerState is already disposed");
        }
    }

    private static boolean booleanAt(int value, String name) {
        if (value != 0 && value != 1) {
            throw new IllegalArgumentException("CNA returned invalid " + name + " value " + value);
        }
        return value != 0;
    }

    private static <T> T enumAt(T[] values, int index, String name) {
        if (index < 0 || index >= values.length) {
            throw new IllegalArgumentException("CNA returned invalid " + name + " value " + index);
        }
        return values[index];
    }

    private static void requireDescriptor(int[] integers, float[] floats) {
        if (integers == null || integers.length != 4 || floats == null || floats.length != 2) {
            throw new IllegalArgumentException("Invalid CNA RasterizerState descriptor");
        }
    }
}
