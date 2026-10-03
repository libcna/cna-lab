package Microsoft.Xna.Framework.Input;

import Microsoft.Xna.Framework.WindowHandle;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Static XNA mouse facade backed by the current CNA game. */
public final class Mouse {

    private Mouse() {
    }

    public static MouseState GetState() {
        return new MouseState(NativeBindings.getMouseState());
    }

    public static void SetPosition(int x, int y) {
        NativeBindings.setMousePosition(x, y);
    }

    public static WindowHandle getWindowHandle() {
        return NativeBindings.getMouseWindowHandle();
    }

    public static void setWindowHandle(WindowHandle value) {
        NativeBindings.setMouseWindowHandle(Objects.requireNonNull(value, "value"));
    }
}
