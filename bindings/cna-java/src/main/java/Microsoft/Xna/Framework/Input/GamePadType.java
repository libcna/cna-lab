package Microsoft.Xna.Framework.Input;

/** Identifies an XNA game-controller category. */
public enum GamePadType {
    Unknown(0),
    GamePad(1),
    Wheel(2),
    ArcadeStick(3),
    FlightStick(4),
    DancePad(5),
    Guitar(6),
    AlternateGuitar(7),
    DrumKit(8),
    BigButtonPad(768);

    private final int value;

    GamePadType(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    static GamePadType fromValue(int value) {
        for (GamePadType candidate : values()) {
            if (candidate.value == value) {
                return candidate;
            }
        }
        return Unknown;
    }
}
