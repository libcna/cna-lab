package Microsoft.Xna.Framework;

import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Package-private CNA implementation of the strict abstract window facade. */
final class CnaGameWindow extends GameWindow {

    CnaGameWindow(Game game, String title) {
        super(game, title);
    }

    @Override
    public boolean getAllowUserResizing() {
        game().prepareNativeWindow();
        return NativeBindings.getWindowAllowUserResizing(game());
    }

    @Override
    public void setAllowUserResizing(boolean value) {
        game().prepareNativeWindow();
        NativeBindings.setWindowAllowUserResizing(game(), value);
        rethrowPendingListenerFailure();
    }

    @Override
    public Rectangle getClientBounds() {
        game().prepareNativeWindow();
        int[] value = NativeBindings.getWindowClientBounds(game());
        return new Rectangle(value[0], value[1], value[2], value[3]);
    }

    @Override
    public DisplayOrientation getCurrentOrientation() {
        game().prepareNativeWindow();
        return DisplayOrientation.FromValue(NativeBindings.getWindowCurrentOrientation(game()));
    }

    @Override
    public WindowHandle getHandle() {
        game().prepareNativeWindow();
        long value = NativeBindings.getWindowHandle(game());
        WindowHandle handle = value == 0L ? WindowHandle.Zero : new WindowHandle(value);
        NativeBindings.registerWindowHandle(handle, value);
        return handle;
    }

    @Override
    public String getScreenDeviceName() {
        game().prepareNativeWindow();
        return NativeBindings.getWindowScreenDeviceName(game());
    }

    @Override
    public void BeginScreenDeviceChange(boolean willBeFullScreen) {
        game().prepareNativeWindow();
        NativeBindings.beginWindowScreenDeviceChange(game(), willBeFullScreen);
        rethrowPendingListenerFailure();
    }

    @Override
    public void EndScreenDeviceChange(
            String screenDeviceName,
            int clientWidth,
            int clientHeight) {
        game().prepareNativeWindow();
        NativeBindings.endWindowScreenDeviceChange(
                game(), Objects.requireNonNull(screenDeviceName, "screenDeviceName"),
                clientWidth, clientHeight);
        rethrowPendingListenerFailure();
    }

    @Override
    protected void SetSupportedOrientations(DisplayOrientation orientations) {
        game().setSupportedOrientationsFromWindow(
                Objects.requireNonNull(orientations, "orientations"));
    }

    @Override
    protected void SetTitle(String title) {
        game().setNativeWindowTitleIfCreated(title);
    }
}
