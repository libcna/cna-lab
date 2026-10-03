package Microsoft.Xna.Framework.Input;

import Microsoft.Xna.Framework.Vector2;

import java.util.Objects;

/** Immutable XNA controller-state snapshot with XNA-compatible virtual-button filtering. */
public final class GamePadState {

    private static final int ALL_RECOGNIZED_BUTTONS = GamePadButtons.NORMAL_BUTTON_MASK
            | Buttons.LeftThumbstickLeft.getValue()
            | Buttons.LeftThumbstickRight.getValue()
            | Buttons.LeftThumbstickDown.getValue()
            | Buttons.LeftThumbstickUp.getValue()
            | Buttons.RightThumbstickLeft.getValue()
            | Buttons.RightThumbstickRight.getValue()
            | Buttons.RightThumbstickDown.getValue()
            | Buttons.RightThumbstickUp.getValue()
            | Buttons.LeftTrigger.getValue()
            | Buttons.RightTrigger.getValue();

    private final boolean connected;
    private final int packet;
    private final GamePadThumbSticks thumbSticks;
    private final GamePadTriggers triggers;
    private final GamePadButtons buttons;
    private final GamePadDPad dPad;
    private final int pressedButtons;

    public GamePadState() {
        connected = false;
        packet = 0;
        thumbSticks = new GamePadThumbSticks();
        triggers = new GamePadTriggers();
        buttons = new GamePadButtons();
        dPad = new GamePadDPad();
        pressedButtons = 0;
    }

    public GamePadState(GamePadState value) {
        GamePadState source = Objects.requireNonNull(value, "value");
        connected = source.connected;
        packet = source.packet;
        thumbSticks = new GamePadThumbSticks(source.thumbSticks);
        triggers = new GamePadTriggers(source.triggers);
        buttons = new GamePadButtons(source.buttons);
        dPad = new GamePadDPad(source.dPad);
        pressedButtons = source.pressedButtons;
    }

    public GamePadState(
            Vector2 leftThumbStick,
            Vector2 rightThumbStick,
            float leftTrigger,
            float rightTrigger,
            Buttons... buttons) {
        thumbSticks = new GamePadThumbSticks(leftThumbStick, rightThumbStick);
        triggers = new GamePadTriggers(leftTrigger, rightTrigger);
        int buttonMask = 0;
        if (buttons != null) {
            for (Buttons button : buttons) {
                buttonMask |= Objects.requireNonNull(button, "buttons element").getValue();
            }
        }
        this.buttons = new GamePadButtons(Buttons.FromValue(buttonMask));
        dPad = dPadFromMask(buttonMask);
        connected = true;
        packet = 0;
        pressedButtons = recognizedButtons(
                this.buttons.mask() | dPad.mask(), thumbSticks, triggers);
    }

    public GamePadState(
            GamePadThumbSticks thumbSticks,
            GamePadTriggers triggers,
            GamePadButtons buttons,
            GamePadDPad dPad) {
        this.thumbSticks = new GamePadThumbSticks(
                Objects.requireNonNull(thumbSticks, "thumbSticks"));
        this.triggers = new GamePadTriggers(Objects.requireNonNull(triggers, "triggers"));
        this.buttons = new GamePadButtons(Objects.requireNonNull(buttons, "buttons"));
        this.dPad = new GamePadDPad(Objects.requireNonNull(dPad, "dPad"));
        connected = true;
        packet = 0;
        pressedButtons = recognizedButtons(
                this.buttons.mask() | this.dPad.mask(), this.thumbSticks, this.triggers);
    }

    private GamePadState(
            boolean connected,
            int packet,
            int pressedButtons,
            GamePadThumbSticks thumbSticks,
            GamePadTriggers triggers) {
        this.connected = connected;
        this.packet = packet;
        this.pressedButtons = pressedButtons & ALL_RECOGNIZED_BUTTONS;
        this.thumbSticks = new GamePadThumbSticks(thumbSticks);
        this.triggers = new GamePadTriggers(triggers);
        buttons = new GamePadButtons(Buttons.FromValue(this.pressedButtons));
        dPad = dPadFromMask(this.pressedButtons);
    }

    public GamePadButtons getButtons() { return new GamePadButtons(buttons); }
    public GamePadDPad getDPad() { return new GamePadDPad(dPad); }
    public boolean getIsConnected() { return connected; }
    public int getPacketNumber() { return packet; }
    public GamePadThumbSticks getThumbSticks() { return new GamePadThumbSticks(thumbSticks); }
    public GamePadTriggers getTriggers() { return new GamePadTriggers(triggers); }

    public boolean IsButtonDown(Buttons button) {
        int requested = Objects.requireNonNull(button, "button").getValue();
        return (requested & pressedButtons) == requested;
    }

    public boolean IsButtonUp(Buttons button) {
        return !IsButtonDown(button);
    }

    @Override
    public boolean equals(Object obj) {
        return this == obj || obj instanceof GamePadState other
                && connected == other.connected && packet == other.packet
                && thumbSticks.equals(other.thumbSticks) && triggers.equals(other.triggers)
                && buttons.equals(other.buttons) && dPad.equals(other.dPad);
    }

    @Override
    public int hashCode() {
        return thumbSticks.hashCode() ^ triggers.hashCode()
                ^ buttons.hashCode() ^ (connected ? 1 : 0)
                ^ dPad.hashCode() ^ packet;
    }

    @Override
    public String toString() {
        return "{IsConnected:" + (connected ? "True" : "False") + '}';
    }

    static GamePadState fromNative(
            boolean connected,
            int packet,
            int pressedButtons,
            float leftX,
            float leftY,
            float rightX,
            float rightY,
            float leftTrigger,
            float rightTrigger) {
        return new GamePadState(
                connected, packet, pressedButtons,
                new GamePadThumbSticks(new Vector2(leftX, leftY), new Vector2(rightX, rightY)),
                new GamePadTriggers(leftTrigger, rightTrigger));
    }

    private static GamePadDPad dPadFromMask(int buttons) {
        return new GamePadDPad(
                state(buttons, Buttons.DPadUp),
                state(buttons, Buttons.DPadDown),
                state(buttons, Buttons.DPadLeft),
                state(buttons, Buttons.DPadRight));
    }

    private static ButtonState state(int buttons, Buttons button) {
        return (buttons & button.getValue()) != 0 ? ButtonState.Pressed : ButtonState.Released;
    }

    private static int recognizedButtons(
            int physicalButtons,
            GamePadThumbSticks thumbSticks,
            GamePadTriggers triggers) {
        Vector2 left = thumbSticks.getLeft();
        Vector2 right = thumbSticks.getRight();
        int leftX = (int) (left.X * 32767.0f);
        int leftY = (int) (left.Y * 32767.0f);
        int rightX = (int) (right.X * 32767.0f);
        int rightY = (int) (right.Y * 32767.0f);
        int leftTrigger = (int) (triggers.getLeft() * 255.0f);
        int rightTrigger = (int) (triggers.getRight() * 255.0f);

        Vector2 leftDeadZone = GamePadDeadZoneUtils.applyLeftStickDeadZone(
                leftX, leftY, GamePadDeadZone.IndependentAxes);
        Vector2 rightDeadZone = GamePadDeadZoneUtils.applyRightStickDeadZone(
                rightX, rightY, GamePadDeadZone.IndependentAxes);
        int result = physicalButtons & GamePadButtons.NORMAL_BUTTON_MASK;
        if (leftDeadZone.X < 0.0f) { result |= Buttons.LeftThumbstickLeft.getValue(); }
        if (leftDeadZone.X > 0.0f) { result |= Buttons.LeftThumbstickRight.getValue(); }
        if (leftDeadZone.Y < 0.0f) { result |= Buttons.LeftThumbstickDown.getValue(); }
        if (leftDeadZone.Y > 0.0f) { result |= Buttons.LeftThumbstickUp.getValue(); }
        if (rightDeadZone.X < 0.0f) { result |= Buttons.RightThumbstickLeft.getValue(); }
        if (rightDeadZone.X > 0.0f) { result |= Buttons.RightThumbstickRight.getValue(); }
        if (rightDeadZone.Y < 0.0f) { result |= Buttons.RightThumbstickDown.getValue(); }
        if (rightDeadZone.Y > 0.0f) { result |= Buttons.RightThumbstickUp.getValue(); }
        if (GamePadDeadZoneUtils.applyTriggerDeadZone(
                leftTrigger, GamePadDeadZone.IndependentAxes) > 0.0f) {
            result |= Buttons.LeftTrigger.getValue();
        }
        if (GamePadDeadZoneUtils.applyTriggerDeadZone(
                rightTrigger, GamePadDeadZone.IndependentAxes) > 0.0f) {
            result |= Buttons.RightTrigger.getValue();
        }
        return result;
    }
}
