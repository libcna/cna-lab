package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.WindowHandle;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/** XNA graphics-adapter metadata and format negotiation backed by CNA's stable C ABI. */
public final class GraphicsAdapter {

    private static final GraphicsAdapter DEFAULT = new GraphicsAdapter(0);
    private static volatile List<GraphicsAdapter> adapters = List.of(DEFAULT);
    private static boolean useNullDevice;
    private static boolean useReferenceDevice;

    private final int nativeIndex;
    private final GraphicsDevice owningDevice;
    private AdapterInfo info;
    private DisplayMode currentDisplayMode;
    private DisplayModeCollection supportedDisplayModes;

    private GraphicsAdapter(int nativeIndex) {
        this(nativeIndex, null);
    }

    private GraphicsAdapter(int nativeIndex, GraphicsDevice owningDevice) {
        this.nativeIndex = nativeIndex;
        this.owningDevice = owningDevice;
    }

    static GraphicsAdapter forDevice(GraphicsDevice device, int nativeIndex) {
        return new GraphicsAdapter(nativeIndex, Objects.requireNonNull(device, "device"));
    }

    final GraphicsDevice owningDevice() {
        return owningDevice;
    }

    final int nativeIndex() {
        return nativeIndex;
    }

    public static synchronized List<GraphicsAdapter> getAdapters() {
        int count = NativeBindings.getGraphicsAdapterCount();
        if (count <= 0) {
            return List.of();
        }
        if (adapters.size() != count) {
            ArrayList<GraphicsAdapter> next = new ArrayList<>(count);
            for (int index = 0; index < count; index++) {
                next.add(index < adapters.size() ? adapters.get(index) : new GraphicsAdapter(index));
            }
            adapters = List.copyOf(next);
        }
        return adapters;
    }

    public static GraphicsAdapter getDefaultAdapter() {
        return DEFAULT;
    }

    public static synchronized boolean getUseNullDevice() {
        return useNullDevice;
    }

    public static synchronized void setUseNullDevice(boolean value) {
        useNullDevice = value;
    }

    public static synchronized boolean getUseReferenceDevice() {
        return useReferenceDevice;
    }

    public static synchronized void setUseReferenceDevice(boolean value) {
        useReferenceDevice = value;
    }

    public final boolean IsProfileSupported(GraphicsProfile graphicsProfile) {
        prepareNativeQuery();
        return NativeBindings.isGraphicsAdapterProfileSupported(
                nativeIndex, Objects.requireNonNull(graphicsProfile, "graphicsProfile").ordinal());
    }

    public final FormatSelectionResult QueryBackBufferFormat(
            GraphicsProfile graphicsProfile,
            SurfaceFormat format,
            DepthFormat depthFormat,
            int multiSampleCount) {
        return queryFormat(true, graphicsProfile, format, depthFormat, multiSampleCount);
    }

    public final FormatSelectionResult QueryRenderTargetFormat(
            GraphicsProfile graphicsProfile,
            SurfaceFormat format,
            DepthFormat depthFormat,
            int multiSampleCount) {
        return queryFormat(false, graphicsProfile, format, depthFormat, multiSampleCount);
    }

    public final synchronized DisplayMode getCurrentDisplayMode() {
        if (currentDisplayMode == null) {
            prepareNativeQuery();
            currentDisplayMode = DisplayMode.fromNative(
                    NativeBindings.getGraphicsAdapterCurrentDisplayMode(nativeIndex));
        }
        return currentDisplayMode;
    }

    public final String getDescription() {
        return getInfo().description();
    }

    public final int getDeviceId() {
        return getInfo().deviceId();
    }

    public final String getDeviceName() {
        return getInfo().deviceName();
    }

    public final boolean getIsDefaultAdapter() {
        return getInfo().defaultAdapter();
    }

    public final boolean getIsWideScreen() {
        return getInfo().wideScreen();
    }

    public final WindowHandle getMonitorHandle() {
        prepareNativeQuery();
        return NativeBindings.getGraphicsAdapterMonitorHandle(nativeIndex);
    }

    public final int getRevision() {
        return getInfo().revision();
    }

    public final int getSubSystemId() {
        return getInfo().subSystemId();
    }

    public final synchronized DisplayModeCollection getSupportedDisplayModes() {
        if (supportedDisplayModes == null) {
            prepareNativeQuery();
            int[][] values = NativeBindings.getGraphicsAdapterDisplayModes(nativeIndex);
            ArrayList<DisplayMode> modes = new ArrayList<>(values.length);
            for (int[] value : values) {
                modes.add(DisplayMode.fromNative(value));
            }
            supportedDisplayModes = new DisplayModeCollection(modes);
        }
        return supportedDisplayModes;
    }

    public final int getVendorId() {
        return getInfo().vendorId();
    }

    private FormatSelectionResult queryFormat(
            boolean backBuffer,
            GraphicsProfile graphicsProfile,
            SurfaceFormat format,
            DepthFormat depthFormat,
            int multiSampleCount) {
        prepareNativeQuery();
        int[] result = NativeBindings.queryGraphicsAdapterFormat(
                nativeIndex,
                backBuffer,
                Objects.requireNonNull(graphicsProfile, "graphicsProfile").ordinal(),
                Objects.requireNonNull(format, "format").ordinal(),
                Objects.requireNonNull(depthFormat, "depthFormat").ordinal(),
                multiSampleCount);
        return new FormatSelectionResult(
                result[0] != 0,
                enumValue(SurfaceFormat.values(), result[1], "SurfaceFormat"),
                enumValue(DepthFormat.values(), result[2], "DepthFormat"),
                result[3]);
    }

    private synchronized AdapterInfo getInfo() {
        if (info == null) {
            prepareNativeQuery();
            long[] value = NativeBindings.getGraphicsAdapterInfo(nativeIndex);
            info = new AdapterInfo(
                    value[0] != 0L,
                    value[1] != 0L,
                    Math.toIntExact(value[4]),
                    Math.toIntExact(value[5]),
                    Math.toIntExact(value[6]),
                    Math.toIntExact(value[7]),
                    NativeBindings.getGraphicsAdapterDescription(nativeIndex, value[8]),
                    NativeBindings.getGraphicsAdapterDeviceName(nativeIndex, value[9]));
        }
        return info;
    }

    private void prepareNativeQuery() {
        boolean nullDevice;
        boolean referenceDevice;
        synchronized (GraphicsAdapter.class) {
            nullDevice = useNullDevice;
            referenceDevice = useReferenceDevice;
        }
        NativeBindings.setGraphicsAdapterDevicePreferences(
                nativeIndex, nullDevice, referenceDevice);
    }

    private static <T> T enumValue(T[] values, int index, String name) {
        if (index < 0 || index >= values.length) {
            throw new IllegalArgumentException("CNA returned an invalid " + name + " value " + index);
        }
        return values[index];
    }

    private record AdapterInfo(
            boolean defaultAdapter,
            boolean wideScreen,
            int vendorId,
            int deviceId,
            int revision,
            int subSystemId,
            String description,
            String deviceName) {
    }

    /** Immutable Java carrier for XNA's Boolean return plus three format-selection outputs. */
    public static final class FormatSelectionResult {

        private final boolean exactMatch;
        private final SurfaceFormat selectedFormat;
        private final DepthFormat selectedDepthFormat;
        private final int selectedMultiSampleCount;

        private FormatSelectionResult(
                boolean exactMatch,
                SurfaceFormat selectedFormat,
                DepthFormat selectedDepthFormat,
                int selectedMultiSampleCount) {
            this.exactMatch = exactMatch;
            this.selectedFormat = selectedFormat;
            this.selectedDepthFormat = selectedDepthFormat;
            this.selectedMultiSampleCount = selectedMultiSampleCount;
        }

        public boolean getExactMatch() {
            return exactMatch;
        }

        public SurfaceFormat getSelectedFormat() {
            return selectedFormat;
        }

        public DepthFormat getSelectedDepthFormat() {
            return selectedDepthFormat;
        }

        public int getSelectedMultiSampleCount() {
            return selectedMultiSampleCount;
        }
    }
}
