package Microsoft.Xna.Framework.Input;

import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Input.Touch.TouchCollection;
import Microsoft.Xna.Framework.Input.Touch.TouchLocation;
import Microsoft.Xna.Framework.Input.Touch.TouchLocationState;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/** Engine-neutral value inputs mirrored from the shared XNA input behavior corpus. */
final class XnaInputBehaviorCorpus {

    private XnaInputBehaviorCorpus() {
    }

    static List<String> capture() {
        List<String> observations = new ArrayList<>();

        KeyboardState nullKeyboard = KeyboardState.fromValues((int[]) null);
        observations.add("keyboard.null.count=" + nullKeyboard.GetPressedKeys().length);

        KeyboardState keyboard = KeyboardState.fromValues(90, 7, 65, 65, 300);
        observations.add("keyboard.pressed=" + Arrays.stream(keyboard.GetPressedKeys())
                .map(key -> Integer.toString(key.getValue()))
                .collect(Collectors.joining(",")));
        observations.add("keyboard.invalid=" + flag(keyboard.isKeyDownValue(7))
                + ',' + flag(keyboard.isKeyDownValue(300)));
        observations.add("keyboard.hash=" + keyboard.hashCode());

        MouseState mouse = new MouseState(
                12, -3, 120,
                ButtonState.Pressed, ButtonState.Released, ButtonState.Pressed,
                ButtonState.Pressed, ButtonState.Released);
        observations.add("mouse.string=" + mouse);
        observations.add("mouse.hash=" + mouse.hashCode());

        GamePadThumbSticks thumbSticks = new GamePadThumbSticks(
                new Vector2(2.0f, -2.0f), new Vector2(0.25f, -0.5f));
        Vector2 left = thumbSticks.getLeft();
        Vector2 right = thumbSticks.getRight();
        observations.add("thumbs.clamp=" + bits(left.X) + ',' + bits(left.Y)
                + ',' + bits(right.X) + ',' + bits(right.Y));
        GamePadTriggers triggers = new GamePadTriggers(-0.5f, 1.5f);
        observations.add("triggers.clamp=" + bits(triggers.getLeft())
                + ',' + bits(triggers.getRight()));

        observations.add("gamepad.null=" + exceptionName(() -> new GamePadState(
                new Vector2(0.1f, -0.3f),
                new Vector2(0.3f, -0.3f),
                0.1f,
                0.2f,
                (Buttons[]) null)));
        GamePadState state = new GamePadState(
                new Vector2(0.1f, -0.3f),
                new Vector2(0.3f, -0.3f),
                0.1f,
                0.2f,
                new Buttons[0]);
        observations.add("gamepad.virtual="
                + flag(state.IsButtonDown(Buttons.LeftThumbstickRight)) + ','
                + flag(state.IsButtonDown(Buttons.LeftThumbstickDown)) + ','
                + flag(state.IsButtonDown(Buttons.RightThumbstickRight)) + ','
                + flag(state.IsButtonDown(Buttons.RightThumbstickDown)) + ','
                + flag(state.IsButtonDown(Buttons.LeftTrigger)) + ','
                + flag(state.IsButtonDown(Buttons.RightTrigger)));
        GamePadState filteredButtons = new GamePadState(
                Vector2.getZero(),
                Vector2.getZero(),
                0.0f,
                0.0f,
                Buttons.A,
                Buttons.LeftTrigger,
                Buttons.FromValue(0x40000000),
                Buttons.FromValue(0x80000000));
        observations.add("gamepad.filtered="
                + flag(filteredButtons.IsButtonDown(Buttons.A)) + ','
                + flag(filteredButtons.IsButtonDown(Buttons.LeftTrigger)) + ','
                + flag(filteredButtons.IsButtonDown(Buttons.FromValue(0x40000000))) + ','
                + flag(filteredButtons.IsButtonDown(Buttons.FromValue(0x80000000))));
        observations.add("gamepad.string=" + state);

        GamePadButtons buttons = new GamePadButtons(
                Buttons.A.Or(Buttons.Y).Or(Buttons.Back));
        observations.add("buttons.string=" + buttons);
        observations.add("buttons.hash=" + buttons.hashCode());
        GamePadDPad dPad = new GamePadDPad(
                ButtonState.Pressed, ButtonState.Released,
                ButtonState.Released, ButtonState.Pressed);
        observations.add("dpad.string=" + dPad);
        observations.add("dpad.hash=" + dPad.hashCode());

        TouchLocation withoutPrevious = new TouchLocation(
                7, TouchLocationState.Pressed, new Vector2(1.0f, 2.0f));
        TouchLocation.PreviousLocationResult previous = withoutPrevious.TryGetPreviousLocation();
        observations.add("touch.previous.none=" + flag(previous.getSucceeded())
                + ',' + previous.getPreviousLocation().getId()
                + ',' + previous.getPreviousLocation().getState().ordinal());

        TouchLocation first = new TouchLocation(
                5, TouchLocationState.Pressed, new Vector2(1.0f, 2.0f),
                TouchLocationState.Moved, new Vector2(0.5f, 1.5f));
        TouchLocation sameCoordinates = new TouchLocation(
                5, TouchLocationState.Released, new Vector2(1.0f, 2.0f),
                TouchLocationState.Released, new Vector2(0.5f, 1.5f));
        TouchCollection equalityProbe = new TouchCollection(new TouchLocation[] {first});
        observations.add("touch.equals=" + flag(first.equals(sameCoordinates))
                + ',' + flag(equalityProbe.Contains(sameCoordinates)));
        observations.add("touch.hash=" + first.hashCode());
        observations.add("touch.string=" + first);

        TouchLocation[] source = {first};
        TouchCollection collection = new TouchCollection(source);
        source[0] = new TouchLocation(
                99, TouchLocationState.Released, Vector2.getZero());
        observations.add("touch.collection.clone=" + collection.get(0).getId());
        observations.add("touch.collection.contains="
                + flag(collection.Contains(sameCoordinates)));
        observations.add("touch.collection.oob="
                + exceptionName(() -> collection.get(1)));

        return observations;
    }

    private static int flag(boolean value) {
        return value ? 1 : 0;
    }

    private static String bits(float value) {
        return String.format("%08X", Float.floatToRawIntBits(value));
    }

    private static String exceptionName(Runnable action) {
        try {
            action.run();
            return "none";
        } catch (RuntimeException exception) {
            return exception.getClass().getSimpleName();
        }
    }
}
