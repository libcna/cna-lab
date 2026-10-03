package Microsoft.Xna.Framework.Input.Touch;

import Microsoft.Xna.Framework.DisplayOrientation;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.WindowHandle;
import org.openeggbert.cna.internal.NativeBindings;

import java.util.Objects;

/** Static XNA touch facade backed by CNA's current game-scoped touch runtime. */
public final class TouchPanel {

    private TouchPanel() {
    }

    public static TouchPanelCapabilities GetCapabilities() {
        int[] values = NativeBindings.getTouchCapabilities();
        return new TouchPanelCapabilities(values[0] != 0, values[1]);
    }

    public static TouchCollection GetState() {
        int[] discrete = new int[26];
        float[] positions = new float[32];
        NativeBindings.getTouchState(discrete, positions);
        int count = discrete[1];
        TouchLocation[] locations = new TouchLocation[count];
        for (int index = 0; index < count; index++) {
            int integer = 2 + index * 3;
            int vector = index * 4;
            locations[index] = new TouchLocation(
                    discrete[integer],
                    TouchLocationState.values()[discrete[integer + 1]],
                    new Vector2(positions[vector], positions[vector + 1]),
                    TouchLocationState.values()[discrete[integer + 2]],
                    new Vector2(positions[vector + 2], positions[vector + 3]));
        }
        return TouchCollection.fromNative(discrete[0] != 0, locations);
    }

    public static GestureSample ReadGesture() {
        int[] type = new int[1];
        long[] timestamp = new long[1];
        float[] vectors = new float[8];
        NativeBindings.readTouchGesture(type, timestamp, vectors);
        return new GestureSample(
                GestureType.FromValue(type[0]), durationFromTicks(timestamp[0]),
                new Vector2(vectors[0], vectors[1]),
                new Vector2(vectors[2], vectors[3]),
                new Vector2(vectors[4], vectors[5]),
                new Vector2(vectors[6], vectors[7]));
    }

    public static int getDisplayHeight() {
        return NativeBindings.getTouchPanelDisplayHeight();
    }

    public static void setDisplayHeight(int value) {
        NativeBindings.setTouchPanelDisplayHeight(value);
    }

    public static DisplayOrientation getDisplayOrientation() {
        return DisplayOrientation.FromValue(NativeBindings.getTouchPanelDisplayOrientation());
    }

    public static void setDisplayOrientation(DisplayOrientation value) {
        NativeBindings.setTouchPanelDisplayOrientation(
                Objects.requireNonNull(value, "value").getValue());
    }

    public static int getDisplayWidth() {
        return NativeBindings.getTouchPanelDisplayWidth();
    }

    public static void setDisplayWidth(int value) {
        NativeBindings.setTouchPanelDisplayWidth(value);
    }

    public static GestureType getEnabledGestures() {
        return GestureType.FromValue(NativeBindings.getTouchPanelEnabledGestures());
    }

    public static void setEnabledGestures(GestureType value) {
        NativeBindings.setTouchPanelEnabledGestures(
                Objects.requireNonNull(value, "value").getValue());
    }

    public static boolean getIsGestureAvailable() {
        return NativeBindings.getTouchPanelIsGestureAvailable();
    }

    public static WindowHandle getWindowHandle() {
        return NativeBindings.getTouchPanelWindowHandle();
    }

    public static void setWindowHandle(WindowHandle value) {
        NativeBindings.setTouchPanelWindowHandle(Objects.requireNonNull(value, "value"));
    }

    private static java.time.Duration durationFromTicks(long ticks) {
        long seconds = Math.floorDiv(ticks, 10_000_000L);
        long remainingTicks = Math.floorMod(ticks, 10_000_000L);
        return java.time.Duration.ofSeconds(seconds, remainingTicks * 100L);
    }
}
