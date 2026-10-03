package Microsoft.Xna.Framework.Net;

import java.time.Duration;

/**
 * The measured connection quality to one discovered session.
 *
 * <p>{@code IsAvailable} is what a title checks first: a search that has not finished probing a
 * session reports no measurement rather than a zeroed one.
 */
public final class QualityOfService {

    private final boolean available;
    private final Duration averageRoundtripTime;
    private final Duration minimumRoundtripTime;
    private final int bytesPerSecondDownstream;
    private final int bytesPerSecondUpstream;

    QualityOfService(long[] values) {
        available = values[0] != 0L;
        averageRoundtripTime = ticks(values[1]);
        minimumRoundtripTime = ticks(values[2]);
        bytesPerSecondDownstream = (int) values[3];
        bytesPerSecondUpstream = (int) values[4];
    }

    private static Duration ticks(long value) {
        return Duration.ofSeconds(value / 10_000_000L, (value % 10_000_000L) * 100L);
    }

    public Duration getAverageRoundtripTime() {
        return averageRoundtripTime;
    }

    public int getBytesPerSecondDownstream() {
        return bytesPerSecondDownstream;
    }

    public int getBytesPerSecondUpstream() {
        return bytesPerSecondUpstream;
    }

    public boolean getIsAvailable() {
        return available;
    }

    public Duration getMinimumRoundtripTime() {
        return minimumRoundtripTime;
    }
}
