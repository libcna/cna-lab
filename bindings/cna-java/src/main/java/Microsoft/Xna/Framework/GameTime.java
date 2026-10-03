package Microsoft.Xna.Framework;

import java.time.Duration;
import java.util.Objects;

/** XNA 4.0 game timing snapshot with {@code TimeSpan} mapped to {@link Duration}. */
public class GameTime {

    private Duration totalGameTime;
    private Duration elapsedGameTime;
    private boolean runningSlowly;

    public GameTime() {
        this(Duration.ZERO, Duration.ZERO, false);
    }

    public GameTime(Duration totalGameTime, Duration elapsedGameTime) {
        this(totalGameTime, elapsedGameTime, false);
    }

    public GameTime(Duration totalGameTime, Duration elapsedGameTime, boolean isRunningSlowly) {
        this.totalGameTime = Objects.requireNonNull(totalGameTime, "totalGameTime");
        this.elapsedGameTime = Objects.requireNonNull(elapsedGameTime, "elapsedGameTime");
        runningSlowly = isRunningSlowly;
    }

    public final Duration getTotalGameTime() {
        return totalGameTime;
    }

    protected final void setTotalGameTime(Duration value) {
        totalGameTime = Objects.requireNonNull(value, "value");
    }

    public final Duration getElapsedGameTime() {
        return elapsedGameTime;
    }

    protected final void setElapsedGameTime(Duration value) {
        elapsedGameTime = Objects.requireNonNull(value, "value");
    }

    public final boolean getIsRunningSlowly() {
        return runningSlowly;
    }

    protected final void setIsRunningSlowly(boolean value) {
        runningSlowly = value;
    }

    void setNativeValues(Duration total, Duration elapsed, boolean slowly) {
        totalGameTime = Objects.requireNonNull(total, "total");
        elapsedGameTime = Objects.requireNonNull(elapsed, "elapsed");
        runningSlowly = slowly;
    }
}
