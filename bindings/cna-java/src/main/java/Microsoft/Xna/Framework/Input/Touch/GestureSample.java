package Microsoft.Xna.Framework.Input.Touch;

import Microsoft.Xna.Framework.Vector2;

import java.time.Duration;
import java.util.Objects;

/** Immutable snapshot of one gesture recognized by CNA's XNA touch runtime. */
public final class GestureSample {

    private static final long TICKS_PER_SECOND = 10_000_000L;

    private final GestureType gestureType;
    private final long timestampTicks;
    private final Vector2 position;
    private final Vector2 position2;
    private final Vector2 delta;
    private final Vector2 delta2;

    public GestureSample() {
        this(GestureType.None, Duration.ZERO,
                new Vector2(), new Vector2(), new Vector2(), new Vector2());
    }

    public GestureSample(GestureSample value) {
        GestureSample source = Objects.requireNonNull(value, "value");
        gestureType = source.gestureType;
        timestampTicks = source.timestampTicks;
        position = new Vector2(source.position);
        position2 = new Vector2(source.position2);
        delta = new Vector2(source.delta);
        delta2 = new Vector2(source.delta2);
    }

    public GestureSample(
            GestureType gestureType,
            Duration timestamp,
            Vector2 position,
            Vector2 position2,
            Vector2 delta,
            Vector2 delta2) {
        this.gestureType = Objects.requireNonNull(gestureType, "gestureType");
        timestampTicks = durationTicks(Objects.requireNonNull(timestamp, "timestamp"));
        this.position = new Vector2(Objects.requireNonNull(position, "position"));
        this.position2 = new Vector2(Objects.requireNonNull(position2, "position2"));
        this.delta = new Vector2(Objects.requireNonNull(delta, "delta"));
        this.delta2 = new Vector2(Objects.requireNonNull(delta2, "delta2"));
    }

    public Vector2 getDelta() {
        return new Vector2(delta);
    }

    public Vector2 getDelta2() {
        return new Vector2(delta2);
    }

    public GestureType getGestureType() {
        return gestureType;
    }

    public Vector2 getPosition() {
        return new Vector2(position);
    }

    public Vector2 getPosition2() {
        return new Vector2(position2);
    }

    public Duration getTimestamp() {
        return durationFromTicks(timestampTicks);
    }

    private static long durationTicks(Duration value) {
        try {
            return Math.addExact(
                    Math.multiplyExact(value.getSeconds(), TICKS_PER_SECOND),
                    value.getNano() / 100L);
        } catch (ArithmeticException exception) {
            throw new IllegalArgumentException("timestamp is outside the XNA TimeSpan range", exception);
        }
    }

    private static Duration durationFromTicks(long ticks) {
        long seconds = Math.floorDiv(ticks, TICKS_PER_SECOND);
        long remainingTicks = Math.floorMod(ticks, TICKS_PER_SECOND);
        return Duration.ofSeconds(seconds, remainingTicks * 100L);
    }
}
