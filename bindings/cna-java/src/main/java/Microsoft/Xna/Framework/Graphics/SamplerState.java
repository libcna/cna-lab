package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Complete XNA sampler-state descriptor, copied to CNA when it is bound. */
public class SamplerState extends GraphicsResource {

    public static final SamplerState AnisotropicClamp = preset(
            "SamplerState.AnisotropicClamp", TextureFilter.Anisotropic,
            TextureAddressMode.Clamp);
    public static final SamplerState AnisotropicWrap = preset(
            "SamplerState.AnisotropicWrap", TextureFilter.Anisotropic,
            TextureAddressMode.Wrap);
    public static final SamplerState LinearClamp = preset(
            "SamplerState.LinearClamp", TextureFilter.Linear,
            TextureAddressMode.Clamp);
    public static final SamplerState LinearWrap = preset(
            "SamplerState.LinearWrap", TextureFilter.Linear,
            TextureAddressMode.Wrap);
    public static final SamplerState PointClamp = preset(
            "SamplerState.PointClamp", TextureFilter.Point,
            TextureAddressMode.Clamp);
    public static final SamplerState PointWrap = preset(
            "SamplerState.PointWrap", TextureFilter.Point,
            TextureAddressMode.Wrap);

    private TextureAddressMode addressU = TextureAddressMode.Wrap;
    private TextureAddressMode addressV = TextureAddressMode.Wrap;
    private TextureAddressMode addressW = TextureAddressMode.Wrap;
    private TextureFilter filter = TextureFilter.Linear;
    private int maxAnisotropy = 4;
    private int maxMipLevel;
    private float mipMapLevelOfDetailBias;
    private boolean bound;

    public SamplerState() {
        super();
    }

    public final TextureAddressMode getAddressU() {
        return addressU;
    }

    public final void setAddressU(TextureAddressMode value) {
        ensureMutable();
        addressU = Objects.requireNonNull(value, "value");
    }

    public final TextureAddressMode getAddressV() {
        return addressV;
    }

    public final void setAddressV(TextureAddressMode value) {
        ensureMutable();
        addressV = Objects.requireNonNull(value, "value");
    }

    public final TextureAddressMode getAddressW() {
        return addressW;
    }

    public final void setAddressW(TextureAddressMode value) {
        ensureMutable();
        addressW = Objects.requireNonNull(value, "value");
    }

    public final TextureFilter getFilter() {
        return filter;
    }

    public final void setFilter(TextureFilter value) {
        ensureMutable();
        filter = Objects.requireNonNull(value, "value");
    }

    public final int getMaxAnisotropy() {
        return maxAnisotropy;
    }

    public final void setMaxAnisotropy(int value) {
        ensureMutable();
        maxAnisotropy = value;
    }

    public final int getMaxMipLevel() {
        return maxMipLevel;
    }

    public final void setMaxMipLevel(int value) {
        ensureMutable();
        maxMipLevel = value;
    }

    public final float getMipMapLevelOfDetailBias() {
        return mipMapLevelOfDetailBias;
    }

    public final void setMipMapLevelOfDetailBias(float value) {
        ensureMutable();
        mipMapLevelOfDetailBias = value;
    }

    @Override
    protected void Dispose(boolean arg0) {
        super.Dispose(arg0);
    }

    final int[] snapshotIntegersForBinding() {
        ensureCanBind();
        return nativeIntegers();
    }

    final float snapshotBiasForBinding() {
        ensureCanBind();
        return mipMapLevelOfDetailBias;
    }

    final void bind(GraphicsDevice device) {
        ensureCanBind();
        attachGraphicsDevice(device);
        bound = true;
    }

    final boolean nativeEquals(int[] integers, float bias) {
        return integers != null && integers.length == 6
                && addressU.ordinal() == integers[0]
                && addressV.ordinal() == integers[1]
                && addressW.ordinal() == integers[2]
                && filter.ordinal() == integers[3]
                && maxAnisotropy == integers[4]
                && maxMipLevel == integers[5]
                && Float.floatToIntBits(mipMapLevelOfDetailBias) == Float.floatToIntBits(bias);
    }

    static SamplerState fromNative(
            int[] integers,
            float bias,
            GraphicsDevice device,
            SamplerState current) {
        requireDescriptor(integers);
        if (current != null && current.nativeEquals(integers, bias)) {
            return current;
        }
        for (SamplerState preset : new SamplerState[]{
                LinearWrap, LinearClamp, PointWrap, PointClamp,
                AnisotropicWrap, AnisotropicClamp}) {
            if (!preset.getIsDisposed() && preset.nativeEquals(integers, bias)) {
                preset.bind(device);
                return preset;
            }
        }
        SamplerState result = new SamplerState();
        result.addressU = enumAt(TextureAddressMode.values(), integers[0], "AddressU");
        result.addressV = enumAt(TextureAddressMode.values(), integers[1], "AddressV");
        result.addressW = enumAt(TextureAddressMode.values(), integers[2], "AddressW");
        result.filter = enumAt(TextureFilter.values(), integers[3], "Filter");
        result.maxAnisotropy = integers[4];
        result.maxMipLevel = integers[5];
        result.mipMapLevelOfDetailBias = bias;
        result.bind(device);
        return result;
    }

    private static SamplerState preset(
            String name, TextureFilter filter, TextureAddressMode address) {
        SamplerState result = new SamplerState();
        result.filter = filter;
        result.addressU = address;
        result.addressV = address;
        result.addressW = address;
        result.setName(name);
        result.bound = true;
        return result;
    }

    private int[] nativeIntegers() {
        return new int[]{
                addressU.ordinal(),
                addressV.ordinal(),
                addressW.ordinal(),
                filter.ordinal(),
                maxAnisotropy,
                maxMipLevel
        };
    }

    private void ensureMutable() {
        if (bound) {
            throw new IllegalStateException(
                    "SamplerState cannot be modified after it has been bound to a GraphicsDevice");
        }
    }

    private void ensureCanBind() {
        if (getIsDisposed()) {
            throw new IllegalStateException("SamplerState is already disposed");
        }
    }

    private static <T> T enumAt(T[] values, int index, String name) {
        if (index < 0 || index >= values.length) {
            throw new IllegalArgumentException("CNA returned invalid " + name + " value " + index);
        }
        return values[index];
    }

    private static void requireDescriptor(int[] integers) {
        if (integers == null || integers.length != 6) {
            throw new IllegalArgumentException("Invalid CNA SamplerState descriptor");
        }
    }
}
