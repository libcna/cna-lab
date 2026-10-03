package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.Color;

import java.util.Arrays;
import java.util.Objects;

/** Complete XNA blend-state descriptor, copied to CNA when it is bound. */
public class BlendState extends GraphicsResource {

    public static final BlendState Additive = preset(
            "BlendState.Additive",
            Blend.SourceAlpha, Blend.One,
            Blend.SourceAlpha, Blend.One);
    public static final BlendState AlphaBlend = preset(
            "BlendState.AlphaBlend",
            Blend.One, Blend.InverseSourceAlpha,
            Blend.One, Blend.InverseSourceAlpha);
    public static final BlendState NonPremultiplied = preset(
            "BlendState.NonPremultiplied",
            Blend.SourceAlpha, Blend.InverseSourceAlpha,
            Blend.SourceAlpha, Blend.InverseSourceAlpha);
    public static final BlendState Opaque = preset(
            "BlendState.Opaque",
            Blend.One, Blend.Zero,
            Blend.One, Blend.Zero);

    private BlendFunction alphaBlendFunction = BlendFunction.Add;
    private Blend alphaDestinationBlend = Blend.Zero;
    private Blend alphaSourceBlend = Blend.One;
    private Color blendFactor = new Color(Color.White);
    private BlendFunction colorBlendFunction = BlendFunction.Add;
    private Blend colorDestinationBlend = Blend.Zero;
    private Blend colorSourceBlend = Blend.One;
    private ColorWriteChannels colorWriteChannels = ColorWriteChannels.All;
    private ColorWriteChannels colorWriteChannels1 = ColorWriteChannels.All;
    private ColorWriteChannels colorWriteChannels2 = ColorWriteChannels.All;
    private ColorWriteChannels colorWriteChannels3 = ColorWriteChannels.All;
    private int multiSampleMask = -1;
    private boolean bound;

    public BlendState() {
        super();
    }

    public final BlendFunction getAlphaBlendFunction() {
        return alphaBlendFunction;
    }

    public final void setAlphaBlendFunction(BlendFunction value) {
        ensureMutable();
        alphaBlendFunction = Objects.requireNonNull(value, "value");
    }

    public final Blend getAlphaDestinationBlend() {
        return alphaDestinationBlend;
    }

    public final void setAlphaDestinationBlend(Blend value) {
        ensureMutable();
        alphaDestinationBlend = Objects.requireNonNull(value, "value");
    }

    public final Blend getAlphaSourceBlend() {
        return alphaSourceBlend;
    }

    public final void setAlphaSourceBlend(Blend value) {
        ensureMutable();
        alphaSourceBlend = Objects.requireNonNull(value, "value");
    }

    public final Color getBlendFactor() {
        return new Color(blendFactor);
    }

    public final void setBlendFactor(Color value) {
        ensureMutable();
        blendFactor = new Color(Objects.requireNonNull(value, "value"));
    }

    public final BlendFunction getColorBlendFunction() {
        return colorBlendFunction;
    }

    public final void setColorBlendFunction(BlendFunction value) {
        ensureMutable();
        colorBlendFunction = Objects.requireNonNull(value, "value");
    }

    public final Blend getColorDestinationBlend() {
        return colorDestinationBlend;
    }

    public final void setColorDestinationBlend(Blend value) {
        ensureMutable();
        colorDestinationBlend = Objects.requireNonNull(value, "value");
    }

    public final Blend getColorSourceBlend() {
        return colorSourceBlend;
    }

    public final void setColorSourceBlend(Blend value) {
        ensureMutable();
        colorSourceBlend = Objects.requireNonNull(value, "value");
    }

    public final ColorWriteChannels getColorWriteChannels() {
        return colorWriteChannels;
    }

    public final void setColorWriteChannels(ColorWriteChannels value) {
        ensureMutable();
        colorWriteChannels = Objects.requireNonNull(value, "value");
    }

    public final ColorWriteChannels getColorWriteChannels1() {
        return colorWriteChannels1;
    }

    public final void setColorWriteChannels1(ColorWriteChannels value) {
        ensureMutable();
        colorWriteChannels1 = Objects.requireNonNull(value, "value");
    }

    public final ColorWriteChannels getColorWriteChannels2() {
        return colorWriteChannels2;
    }

    public final void setColorWriteChannels2(ColorWriteChannels value) {
        ensureMutable();
        colorWriteChannels2 = Objects.requireNonNull(value, "value");
    }

    public final ColorWriteChannels getColorWriteChannels3() {
        return colorWriteChannels3;
    }

    public final void setColorWriteChannels3(ColorWriteChannels value) {
        ensureMutable();
        colorWriteChannels3 = Objects.requireNonNull(value, "value");
    }

    public final int getMultiSampleMask() {
        return multiSampleMask;
    }

    public final void setMultiSampleMask(int value) {
        ensureMutable();
        multiSampleMask = value;
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

    static BlendState fromNative(int[] values, GraphicsDevice device, BlendState current) {
        requireLength(values, 12);
        if (current != null && current.nativeEquals(values)) {
            return current;
        }
        for (BlendState preset : new BlendState[]{Opaque, AlphaBlend, Additive, NonPremultiplied}) {
            if (!preset.getIsDisposed() && preset.nativeEquals(values)) {
                preset.bind(device);
                return preset;
            }
        }
        BlendState result = new BlendState();
        result.alphaBlendFunction = enumAt(BlendFunction.values(), values[0], "AlphaBlendFunction");
        result.alphaDestinationBlend = enumAt(Blend.values(), values[1], "AlphaDestinationBlend");
        result.alphaSourceBlend = enumAt(Blend.values(), values[2], "AlphaSourceBlend");
        result.colorBlendFunction = enumAt(BlendFunction.values(), values[3], "ColorBlendFunction");
        result.colorDestinationBlend = enumAt(Blend.values(), values[4], "ColorDestinationBlend");
        result.colorSourceBlend = enumAt(Blend.values(), values[5], "ColorSourceBlend");
        result.colorWriteChannels = ColorWriteChannels.FromValue(values[6]);
        result.colorWriteChannels1 = ColorWriteChannels.FromValue(values[7]);
        result.colorWriteChannels2 = ColorWriteChannels.FromValue(values[8]);
        result.colorWriteChannels3 = ColorWriteChannels.FromValue(values[9]);
        result.blendFactor = color(values[10]);
        result.multiSampleMask = values[11];
        result.bind(device);
        return result;
    }

    private static BlendState preset(
            String name,
            Blend colorSource,
            Blend colorDestination,
            Blend alphaSource,
            Blend alphaDestination) {
        BlendState result = new BlendState();
        result.colorSourceBlend = colorSource;
        result.colorDestinationBlend = colorDestination;
        result.alphaSourceBlend = alphaSource;
        result.alphaDestinationBlend = alphaDestination;
        result.setName(name);
        result.bound = true;
        return result;
    }

    private int[] nativeValues() {
        return new int[]{
                alphaBlendFunction.ordinal(),
                alphaDestinationBlend.ordinal(),
                alphaSourceBlend.ordinal(),
                colorBlendFunction.ordinal(),
                colorDestinationBlend.ordinal(),
                colorSourceBlend.ordinal(),
                colorWriteChannels.getValue(),
                colorWriteChannels1.getValue(),
                colorWriteChannels2.getValue(),
                colorWriteChannels3.getValue(),
                blendFactor.getPackedValue().intValue(),
                multiSampleMask
        };
    }

    private void ensureMutable() {
        if (bound) {
            throw new IllegalStateException(
                    "BlendState cannot be modified after it has been bound to a GraphicsDevice");
        }
    }

    private void ensureCanBind() {
        if (getIsDisposed()) {
            throw new IllegalStateException("BlendState is already disposed");
        }
    }

    private static Color color(int packed) {
        Color result = new Color(0, 0, 0, 0);
        result.setPackedValue(Integer.toUnsignedLong(packed));
        return result;
    }

    private static <T> T enumAt(T[] values, int index, String name) {
        if (index < 0 || index >= values.length) {
            throw new IllegalArgumentException("CNA returned invalid " + name + " value " + index);
        }
        return values[index];
    }

    private static void requireLength(int[] values, int expected) {
        if (values == null || values.length != expected) {
            throw new IllegalArgumentException("Invalid CNA BlendState descriptor");
        }
    }
}
