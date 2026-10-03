package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Color;

/**
 * The system-wide game preferences a signed-in gamer has chosen.
 *
 * <p>A title reads these to match the player's stated preference -- difficulty, controller
 * sensitivity, inverted look, transmission -- instead of asking again in its own options screen.
 */
public final class GameDefaults {

    private final GameDifficulty gameDifficulty;
    private final ControllerSensitivity controllerSensitivity;
    private final RacingCameraAngle racingCameraAngle;
    private final boolean autoAim;
    private final boolean autoCenter;
    private final boolean moveWithRightThumbStick;
    private final boolean invertYAxis;
    private final boolean manualTransmission;
    private final boolean accelerateWithButtons;
    private final boolean brakeWithButtons;
    private final Color primaryColor;
    private final Color secondaryColor;

    GameDefaults(long[] values) {
        gameDifficulty = GameDifficulty.values()[(int) values[0]];
        controllerSensitivity = ControllerSensitivity.values()[(int) values[1]];
        racingCameraAngle = RacingCameraAngle.values()[(int) values[2]];
        autoAim = values[5] != 0L;
        autoCenter = values[6] != 0L;
        moveWithRightThumbStick = values[7] != 0L;
        invertYAxis = values[8] != 0L;
        manualTransmission = values[9] != 0L;
        accelerateWithButtons = values[10] != 0L;
        brakeWithButtons = values[11] != 0L;
        primaryColor = new Color((int) values[12], (int) values[13],
                (int) values[14], (int) values[15]);
        secondaryColor = new Color((int) values[16], (int) values[17],
                (int) values[18], (int) values[19]);
    }

    public boolean getAccelerateWithButtons() {
        return accelerateWithButtons;
    }

    public boolean getAutoAim() {
        return autoAim;
    }

    public boolean getAutoCenter() {
        return autoCenter;
    }

    public boolean getBrakeWithButtons() {
        return brakeWithButtons;
    }

    public ControllerSensitivity getControllerSensitivity() {
        return controllerSensitivity;
    }

    public GameDifficulty getGameDifficulty() {
        return gameDifficulty;
    }

    public boolean getInvertYAxis() {
        return invertYAxis;
    }

    public boolean getManualTransmission() {
        return manualTransmission;
    }

    public boolean getMoveWithRightThumbStick() {
        return moveWithRightThumbStick;
    }

    /** Returns a copy, because XNA's Color is a struct and this one is shared state. */
    public Color getPrimaryColor() {
        return new Color(primaryColor);
    }

    public RacingCameraAngle getRacingCameraAngle() {
        return racingCameraAngle;
    }

    /** Returns a copy, because XNA's Color is a struct and this one is shared state. */
    public Color getSecondaryColor() {
        return new Color(secondaryColor);
    }
}
