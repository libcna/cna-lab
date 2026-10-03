package Microsoft.Xna.Framework.Input.Touch;

import Microsoft.Xna.Framework.Vector2;

import java.util.Objects;

/** Immutable XNA touch-location snapshot, including an optional previous location. */
public final class TouchLocation {

    private final int id;
    private final TouchLocationState state;
    private final float x;
    private final float y;
    private final TouchLocationState previousState;
    private final float previousX;
    private final float previousY;

    public TouchLocation() {
        this(0, TouchLocationState.Invalid, 0.0f, 0.0f,
                TouchLocationState.Invalid, 0.0f, 0.0f);
    }

    public TouchLocation(TouchLocation value) {
        this(Objects.requireNonNull(value, "value").id,
                value.state, value.x, value.y,
                value.previousState, value.previousX, value.previousY);
    }

    public TouchLocation(int id, TouchLocationState state, Vector2 position) {
        this(id,
                Objects.requireNonNull(state, "state"),
                Objects.requireNonNull(position, "position").X,
                position.Y,
                TouchLocationState.Invalid,
                0.0f,
                0.0f);
    }

    public TouchLocation(
            int id,
            TouchLocationState state,
            Vector2 position,
            TouchLocationState previousState,
            Vector2 previousPosition) {
        this(id,
                Objects.requireNonNull(state, "state"),
                Objects.requireNonNull(position, "position").X,
                position.Y,
                Objects.requireNonNull(previousState, "previousState"),
                Objects.requireNonNull(previousPosition, "previousPosition").X,
                previousPosition.Y);
    }

    private TouchLocation(
            int id,
            TouchLocationState state,
            float x,
            float y,
            TouchLocationState previousState,
            float previousX,
            float previousY) {
        this.id = id;
        this.state = state;
        this.x = x;
        this.y = y;
        this.previousState = previousState;
        this.previousX = previousX;
        this.previousY = previousY;
    }

    public int getId() {
        return id;
    }

    public Vector2 getPosition() {
        return new Vector2(x, y);
    }

    public TouchLocationState getState() {
        return state;
    }

    public PreviousLocationResult TryGetPreviousLocation() {
        if (previousState == TouchLocationState.Invalid) {
            return new PreviousLocationResult(false, new TouchLocation(
                    -1, TouchLocationState.Invalid, 0.0f, 0.0f,
                    TouchLocationState.Invalid, 0.0f, 0.0f));
        }
        return new PreviousLocationResult(true, new TouchLocation(
                id, previousState, previousX, previousY,
                TouchLocationState.Invalid, 0.0f, 0.0f));
    }

    public boolean equals(TouchLocation other) {
        return other != null && id == other.id
                && x == other.x && y == other.y
                && previousX == other.previousX && previousY == other.previousY;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof TouchLocation other && equals(other);
    }

    @Override
    public int hashCode() {
        return id + floatHash(x) + floatHash(y);
    }

    @Override
    public String toString() {
        return "{Position:{X:" + formatFloat(x) + " Y:" + formatFloat(y) + "}}";
    }

    boolean operatorEquals(TouchLocation other) {
        return other != null && id == other.id && state == other.state
                && x == other.x && y == other.y && previousState == other.previousState
                && previousX == other.previousX && previousY == other.previousY;
    }

    private static int floatHash(float value) {
        return value == 0.0f ? 0 : Float.floatToIntBits(value);
    }

    private static String formatFloat(float value) {
        if (value == 0.0f) {
            return "0";
        }
        if (Float.isNaN(value)) {
            return "NaN";
        }
        if (value == Float.POSITIVE_INFINITY) {
            return "Infinity";
        }
        if (value == Float.NEGATIVE_INFINITY) {
            return "-Infinity";
        }
        int integer = (int) value;
        return value == integer ? Integer.toString(integer) : Float.toString(value);
    }

    /** Immutable Java carrier for XNA's Boolean-plus-out-value method. */
    public static final class PreviousLocationResult {

        private final boolean succeeded;
        private final TouchLocation previousLocation;

        private PreviousLocationResult(boolean succeeded, TouchLocation previousLocation) {
            this.succeeded = succeeded;
            this.previousLocation = previousLocation;
        }

        public boolean getSucceeded() {
            return succeeded;
        }

        public TouchLocation getPreviousLocation() {
            return new TouchLocation(previousLocation);
        }
    }
}
