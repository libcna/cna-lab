package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.DisplayOrientation;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.WindowHandle;

import java.util.Objects;

/** Mutable XNA presentation settings; native calls receive explicit snapshots. */
public class PresentationParameters {

    private SurfaceFormat backBufferFormat = SurfaceFormat.Color;
    private int backBufferHeight;
    private int backBufferWidth;
    private DepthFormat depthStencilFormat = DepthFormat.None;
    private WindowHandle deviceWindowHandle = WindowHandle.Zero;
    private DisplayOrientation displayOrientation = DisplayOrientation.Default;
    private boolean fullScreen = true;
    private int multiSampleCount;
    private PresentInterval presentationInterval = PresentInterval.Default;
    private RenderTargetUsage renderTargetUsage = RenderTargetUsage.DiscardContents;

    public PresentationParameters() {
    }

    private PresentationParameters(PresentationParameters source) {
        backBufferFormat = source.backBufferFormat;
        backBufferHeight = source.backBufferHeight;
        backBufferWidth = source.backBufferWidth;
        depthStencilFormat = source.depthStencilFormat;
        deviceWindowHandle = source.deviceWindowHandle;
        displayOrientation = source.displayOrientation;
        fullScreen = source.fullScreen;
        multiSampleCount = source.multiSampleCount;
        presentationInterval = source.presentationInterval;
        renderTargetUsage = source.renderTargetUsage;
    }

    public final SurfaceFormat getBackBufferFormat() {
        return backBufferFormat;
    }

    public final void setBackBufferFormat(SurfaceFormat value) {
        backBufferFormat = Objects.requireNonNull(value, "value");
    }

    public final int getBackBufferHeight() {
        return backBufferHeight;
    }

    public final void setBackBufferHeight(int value) {
        backBufferHeight = value;
    }

    public final int getBackBufferWidth() {
        return backBufferWidth;
    }

    public final void setBackBufferWidth(int value) {
        backBufferWidth = value;
    }

    public final Rectangle getBounds() {
        return new Rectangle(0, 0, backBufferWidth, backBufferHeight);
    }

    public final DepthFormat getDepthStencilFormat() {
        return depthStencilFormat;
    }

    public final void setDepthStencilFormat(DepthFormat value) {
        depthStencilFormat = Objects.requireNonNull(value, "value");
    }

    public final WindowHandle getDeviceWindowHandle() {
        return deviceWindowHandle;
    }

    public final void setDeviceWindowHandle(WindowHandle value) {
        deviceWindowHandle = Objects.requireNonNull(value, "value");
    }

    public final DisplayOrientation getDisplayOrientation() {
        return displayOrientation;
    }

    public final void setDisplayOrientation(DisplayOrientation value) {
        displayOrientation = Objects.requireNonNull(value, "value");
    }

    public final boolean getIsFullScreen() {
        return fullScreen;
    }

    public final void setIsFullScreen(boolean value) {
        fullScreen = value;
    }

    public final int getMultiSampleCount() {
        return multiSampleCount;
    }

    public final void setMultiSampleCount(int value) {
        multiSampleCount = value;
    }

    public final PresentInterval getPresentationInterval() {
        return presentationInterval;
    }

    public final void setPresentationInterval(PresentInterval value) {
        presentationInterval = Objects.requireNonNull(value, "value");
    }

    public final RenderTargetUsage getRenderTargetUsage() {
        return renderTargetUsage;
    }

    public final void setRenderTargetUsage(RenderTargetUsage value) {
        renderTargetUsage = Objects.requireNonNull(value, "value");
    }

    public final PresentationParameters Clone() {
        return new PresentationParameters(this);
    }
}
