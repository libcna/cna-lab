package Microsoft.Xna.Framework.Input;

import java.util.Objects;

/** Immutable XNA controller-capabilities snapshot. */
public final class GamePadCapabilities {

    private final boolean connected;
    private final GamePadType gamePadType;
    private final boolean[] features;

    public GamePadCapabilities() {
        connected = false;
        gamePadType = GamePadType.Unknown;
        features = new boolean[24];
    }

    public GamePadCapabilities(GamePadCapabilities value) {
        GamePadCapabilities source = Objects.requireNonNull(value, "value");
        connected = source.connected;
        gamePadType = source.gamePadType;
        features = source.features.clone();
    }

    private GamePadCapabilities(boolean connected, GamePadType gamePadType, boolean[] features) {
        if (features.length != 24) {
            throw new IllegalArgumentException("game-pad capabilities require 24 feature flags");
        }
        this.connected = connected;
        this.gamePadType = Objects.requireNonNull(gamePadType, "gamePadType");
        this.features = features.clone();
    }

    public GamePadType getGamePadType() { return gamePadType; }
    public boolean getHasAButton() { return features[0]; }
    public boolean getHasBButton() { return features[1]; }
    public boolean getHasBackButton() { return features[2]; }
    public boolean getHasBigButton() { return features[3]; }
    public boolean getHasDPadDownButton() { return features[4]; }
    public boolean getHasDPadLeftButton() { return features[5]; }
    public boolean getHasDPadRightButton() { return features[6]; }
    public boolean getHasDPadUpButton() { return features[7]; }
    public boolean getHasLeftShoulderButton() { return features[8]; }
    public boolean getHasLeftStickButton() { return features[9]; }
    public boolean getHasLeftTrigger() { return features[10]; }
    public boolean getHasLeftVibrationMotor() { return features[11]; }
    public boolean getHasLeftXThumbStick() { return features[12]; }
    public boolean getHasLeftYThumbStick() { return features[13]; }
    public boolean getHasRightShoulderButton() { return features[14]; }
    public boolean getHasRightStickButton() { return features[15]; }
    public boolean getHasRightTrigger() { return features[16]; }
    public boolean getHasRightVibrationMotor() { return features[17]; }
    public boolean getHasRightXThumbStick() { return features[18]; }
    public boolean getHasRightYThumbStick() { return features[19]; }
    public boolean getHasStartButton() { return features[20]; }
    public boolean getHasVoiceSupport() { return features[21]; }
    public boolean getHasXButton() { return features[22]; }
    public boolean getHasYButton() { return features[23]; }
    public boolean getIsConnected() { return connected; }

    static GamePadCapabilities fromNative(boolean connected, int type, boolean[] features) {
        int xnaType = type == 9 ? GamePadType.BigButtonPad.getValue() : type;
        return new GamePadCapabilities(connected, GamePadType.fromValue(xnaType), features);
    }
}
