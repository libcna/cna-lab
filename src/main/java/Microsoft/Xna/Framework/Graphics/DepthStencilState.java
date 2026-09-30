package Microsoft.Xna.Framework.Graphics;

import java.util.Arrays;
import java.util.Objects;

/** Complete XNA depth/stencil-state descriptor, copied to CNA when it is bound. */
public class DepthStencilState extends GraphicsResource {

    public static final DepthStencilState Default = preset(
            "DepthStencilState.Default", true, true);
    public static final DepthStencilState DepthRead = preset(
            "DepthStencilState.DepthRead", true, false);
    public static final DepthStencilState None = preset(
            "DepthStencilState.None", false, false);

    private StencilOperation counterClockwiseStencilDepthBufferFail = StencilOperation.Keep;
    private StencilOperation counterClockwiseStencilFail = StencilOperation.Keep;
    private CompareFunction counterClockwiseStencilFunction = CompareFunction.Always;
    private StencilOperation counterClockwiseStencilPass = StencilOperation.Keep;
    private boolean depthBufferEnable = true;
    private CompareFunction depthBufferFunction = CompareFunction.LessEqual;
    private boolean depthBufferWriteEnable = true;
    private int referenceStencil;
    private StencilOperation stencilDepthBufferFail = StencilOperation.Keep;
    private boolean stencilEnable;
    private StencilOperation stencilFail = StencilOperation.Keep;
    private CompareFunction stencilFunction = CompareFunction.Always;
    private int stencilMask = -1;
    private StencilOperation stencilPass = StencilOperation.Keep;
    private int stencilWriteMask = -1;
    private boolean twoSidedStencilMode;
    private boolean bound;

    public DepthStencilState() {
        super();
    }

    public final StencilOperation getCounterClockwiseStencilDepthBufferFail() {
        return counterClockwiseStencilDepthBufferFail;
    }

    public final void setCounterClockwiseStencilDepthBufferFail(StencilOperation value) {
        ensureMutable();
        counterClockwiseStencilDepthBufferFail = Objects.requireNonNull(value, "value");
    }

    public final StencilOperation getCounterClockwiseStencilFail() {
        return counterClockwiseStencilFail;
    }

    public final void setCounterClockwiseStencilFail(StencilOperation value) {
        ensureMutable();
        counterClockwiseStencilFail = Objects.requireNonNull(value, "value");
    }

    public final CompareFunction getCounterClockwiseStencilFunction() {
        return counterClockwiseStencilFunction;
    }

    public final void setCounterClockwiseStencilFunction(CompareFunction value) {
        ensureMutable();
        counterClockwiseStencilFunction = Objects.requireNonNull(value, "value");
    }

    public final StencilOperation getCounterClockwiseStencilPass() {
        return counterClockwiseStencilPass;
    }

    public final void setCounterClockwiseStencilPass(StencilOperation value) {
        ensureMutable();
        counterClockwiseStencilPass = Objects.requireNonNull(value, "value");
    }

    public final boolean getDepthBufferEnable() {
        return depthBufferEnable;
    }

    public final void setDepthBufferEnable(boolean value) {
        ensureMutable();
        depthBufferEnable = value;
    }

    public final CompareFunction getDepthBufferFunction() {
        return depthBufferFunction;
    }

    public final void setDepthBufferFunction(CompareFunction value) {
        ensureMutable();
        depthBufferFunction = Objects.requireNonNull(value, "value");
    }

    public final boolean getDepthBufferWriteEnable() {
        return depthBufferWriteEnable;
    }

    public final void setDepthBufferWriteEnable(boolean value) {
        ensureMutable();
        depthBufferWriteEnable = value;
    }

    public final int getReferenceStencil() {
        return referenceStencil;
    }

    public final void setReferenceStencil(int value) {
        ensureMutable();
        referenceStencil = value;
    }

    public final StencilOperation getStencilDepthBufferFail() {
        return stencilDepthBufferFail;
    }

    public final void setStencilDepthBufferFail(StencilOperation value) {
        ensureMutable();
        stencilDepthBufferFail = Objects.requireNonNull(value, "value");
    }

    public final boolean getStencilEnable() {
        return stencilEnable;
    }

    public final void setStencilEnable(boolean value) {
        ensureMutable();
        stencilEnable = value;
    }

    public final StencilOperation getStencilFail() {
        return stencilFail;
    }

    public final void setStencilFail(StencilOperation value) {
        ensureMutable();
        stencilFail = Objects.requireNonNull(value, "value");
    }

    public final CompareFunction getStencilFunction() {
        return stencilFunction;
    }

    public final void setStencilFunction(CompareFunction value) {
        ensureMutable();
        stencilFunction = Objects.requireNonNull(value, "value");
    }

    public final int getStencilMask() {
        return stencilMask;
    }

    public final void setStencilMask(int value) {
        ensureMutable();
        stencilMask = value;
    }

    public final StencilOperation getStencilPass() {
        return stencilPass;
    }

    public final void setStencilPass(StencilOperation value) {
        ensureMutable();
        stencilPass = Objects.requireNonNull(value, "value");
    }

    public final int getStencilWriteMask() {
        return stencilWriteMask;
    }

    public final void setStencilWriteMask(int value) {
        ensureMutable();
        stencilWriteMask = value;
    }

    public final boolean getTwoSidedStencilMode() {
        return twoSidedStencilMode;
    }

    public final void setTwoSidedStencilMode(boolean value) {
        ensureMutable();
        twoSidedStencilMode = value;
    }

    @Override
    protected void Dispose(boolean arg0) {
        super.Dispose(arg0);
    }

    final int[] snapshotForBinding() {
        ensureCanBind();
        return nativeValues();
    }

    final void bind(GraphicsDevice device) {
        ensureCanBind();
        attachGraphicsDevice(device);
        bound = true;
    }

    final boolean nativeEquals(int[] values) {
        return Arrays.equals(nativeValues(), values);
    }

    static DepthStencilState fromNative(
            int[] values, GraphicsDevice device, DepthStencilState current) {
        requireLength(values, 16);
        if (current != null && current.nativeEquals(values)) {
            return current;
        }
        for (DepthStencilState preset : new DepthStencilState[]{Default, DepthRead, None}) {
            if (!preset.getIsDisposed() && preset.nativeEquals(values)) {
                preset.bind(device);
                return preset;
            }
        }
        DepthStencilState result = new DepthStencilState();
        result.depthBufferEnable = booleanAt(values[0], "DepthBufferEnable");
        result.depthBufferWriteEnable = booleanAt(values[1], "DepthBufferWriteEnable");
        result.stencilEnable = booleanAt(values[2], "StencilEnable");
        result.twoSidedStencilMode = booleanAt(values[3], "TwoSidedStencilMode");
        result.depthBufferFunction = enumAt(
                CompareFunction.values(), values[4], "DepthBufferFunction");
        result.stencilFunction = enumAt(
                CompareFunction.values(), values[5], "StencilFunction");
        result.stencilMask = values[6];
        result.stencilWriteMask = values[7];
        result.referenceStencil = values[8];
        result.stencilFail = enumAt(StencilOperation.values(), values[9], "StencilFail");
        result.stencilDepthBufferFail = enumAt(
                StencilOperation.values(), values[10], "StencilDepthBufferFail");
        result.stencilPass = enumAt(StencilOperation.values(), values[11], "StencilPass");
        result.counterClockwiseStencilFunction = enumAt(
                CompareFunction.values(), values[12], "CounterClockwiseStencilFunction");
        result.counterClockwiseStencilFail = enumAt(
                StencilOperation.values(), values[13], "CounterClockwiseStencilFail");
        result.counterClockwiseStencilDepthBufferFail = enumAt(
                StencilOperation.values(), values[14],
                "CounterClockwiseStencilDepthBufferFail");
        result.counterClockwiseStencilPass = enumAt(
                StencilOperation.values(), values[15], "CounterClockwiseStencilPass");
        result.bind(device);
        return result;
    }

    private static DepthStencilState preset(
            String name, boolean depthEnable, boolean depthWriteEnable) {
        DepthStencilState result = new DepthStencilState();
        result.depthBufferEnable = depthEnable;
        result.depthBufferWriteEnable = depthWriteEnable;
        result.setName(name);
        result.bound = true;
        return result;
    }

    private int[] nativeValues() {
        return new int[]{
                depthBufferEnable ? 1 : 0,
                depthBufferWriteEnable ? 1 : 0,
                stencilEnable ? 1 : 0,
                twoSidedStencilMode ? 1 : 0,
                depthBufferFunction.ordinal(),
                stencilFunction.ordinal(),
                stencilMask,
                stencilWriteMask,
                referenceStencil,
                stencilFail.ordinal(),
                stencilDepthBufferFail.ordinal(),
                stencilPass.ordinal(),
                counterClockwiseStencilFunction.ordinal(),
                counterClockwiseStencilFail.ordinal(),
                counterClockwiseStencilDepthBufferFail.ordinal(),
                counterClockwiseStencilPass.ordinal()
        };
    }

    private void ensureMutable() {
        if (bound) {
            throw new IllegalStateException(
                    "DepthStencilState cannot be modified after it has been bound to a GraphicsDevice");
        }
    }

    private void ensureCanBind() {
        if (getIsDisposed()) {
            throw new IllegalStateException("DepthStencilState is already disposed");
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

    private static void requireLength(int[] values, int expected) {
        if (values == null || values.length != expected) {
            throw new IllegalArgumentException("Invalid CNA DepthStencilState descriptor");
        }
    }
}
