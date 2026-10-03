package Microsoft.Xna.Framework.Input;

import Microsoft.Xna.Framework.PlayerIndex;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Static XNA keyboard facade backed by the current CNA game. */
public final class Keyboard {

    private Keyboard() {
    }

    public static KeyboardState GetState() {
        return new KeyboardState(NativeBindings.getKeyboardState(-1));
    }

    public static KeyboardState GetState(PlayerIndex playerIndex) {
        return new KeyboardState(NativeBindings.getKeyboardState(
                Objects.requireNonNull(playerIndex, "playerIndex").ordinal()));
    }
}
