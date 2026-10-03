package org.openeggbert.cna.extensions.sensors;

import org.openeggbert.cna.internal.NativeBindings;

import java.time.Instant;
import java.time.OffsetDateTime;
import java.time.ZoneOffset;

/** Shared plumbing for the sensor extensions. This class is not application API. */
final class SensorExtension {

    private static final int RESULT_SUCCESS = 0;
    private static final int RESULT_INVALID_STATE = 3;
    private static final int RESULT_NOT_SUPPORTED = 6;

    /** One tick is 100 nanoseconds, the unit every duration in the CNA ABI uses. */
    static final long NANOSECONDS_PER_TICK = 100L;

    /** Ticks from 0001-01-01, CNA's epoch, to 1970-01-01, the Java one. */
    private static final long TICKS_AT_UNIX_EPOCH = 621_355_968_000_000_000L;

    private static final long TICKS_PER_SECOND = 10_000_000L;

    private SensorExtension() {
    }

    static long game(String owner) {
        NativeBindings.requireAvailable();
        return NativeBindings.currentGameHandleValue(owner);
    }

    /**
     * Maps one CNA result, keeping the identities the sensor contract depends on.
     *
     * <p>{@code NOT_SUPPORTED} means the build has no sensor layer. {@code INVALID_STATE} is what
     * CNA answers for a sensor that has been disposed, for a second disposal, and for a reading
     * asked of an unsupported sensor; each is a state error the caller could have avoided by
     * asking first, which is what {@link IllegalStateException} means in Java.
     */
    static void check(String operation, int result) {
        if (result == RESULT_SUCCESS) {
            return;
        }
        if (result == RESULT_NOT_SUPPORTED) {
            throw new SensorNotSupportedException(operation
                    + " is not supported by this CNA build");
        }
        if (result == RESULT_INVALID_STATE) {
            throw new IllegalStateException(operation
                    + " was refused: the sensor is disposed, unsupported, or already in that state"
                    + lastErrorSuffix());
        }
        throw NativeBindings.failure(operation, result);
    }

    /**
     * Converts CNA's date-and-offset to the Java equivalent.
     *
     * <p>Both members are 100-nanosecond ticks. {@code ticks} is local time counted from CNA's own
     * epoch of 0001-01-01, so UTC is {@code ticks - offsetTicks}; that is the value the canonical
     * equality compares, and the one an {@link Instant} has to be built from.
     */
    /**
     * Turns one timestamp back into the two leaves CNA's structure carries.
     *
     * <p>The inverse of {@link #timestamp(long, long)}, and the one place that knows the epoch
     * and the tick size in that direction -- so a reading injected and read back is the same
     * instant rather than two conventions that happen to agree.
     */
    static long[] timestampLeaves(OffsetDateTime timestamp) {
        long offsetTicks = (long) timestamp.getOffset().getTotalSeconds() * TICKS_PER_SECOND;
        long utcTicks = TICKS_AT_UNIX_EPOCH
                + Math.multiplyExact(timestamp.toEpochSecond(), TICKS_PER_SECOND)
                + timestamp.getNano() / NANOSECONDS_PER_TICK;
        return new long[] {utcTicks + offsetTicks, offsetTicks};
    }

    static OffsetDateTime timestamp(long ticks, long offsetTicks) {
        long utcTicks = Math.subtractExact(ticks, offsetTicks) - TICKS_AT_UNIX_EPOCH;
        Instant instant = Instant.ofEpochSecond(
                Math.floorDiv(utcTicks, TICKS_PER_SECOND),
                Math.floorMod(utcTicks, TICKS_PER_SECOND) * NANOSECONDS_PER_TICK);
        return instant.atOffset(zoneOffset(offsetTicks));
    }

    private static ZoneOffset zoneOffset(long offsetTicks) {
        long seconds = offsetTicks / TICKS_PER_SECOND;
        // A host that reports an offset outside the range a ZoneOffset can hold is reporting
        // something this projection cannot represent; UTC is the honest fallback, and the
        // instant itself is unaffected because it was computed before the offset was applied.
        if (seconds < -18 * 3600L || seconds > 18 * 3600L) {
            return ZoneOffset.UTC;
        }
        return ZoneOffset.ofTotalSeconds((int) seconds);
    }

    /**
     * Returns CNA's own last sensor error id, when it recorded one.
     *
     * <p>The canonical sensor failure carries an error id on the exception; the ABI records it
     * per thread instead, so it is read here and appended to the diagnostic rather than lost.
     */
    private static String lastErrorSuffix() {
        int[] errorId = new int[1];
        boolean[] present = new boolean[1];
        int result = org.openeggbert.cna.internal.generated.NativeSensorExtensionRoutes
                .sensorsGetLastErrorIdExt(errorId, present);
        if (result != RESULT_SUCCESS || !present[0]) {
            return "";
        }
        return " (CNA sensor error id " + errorId[0] + ")";
    }
}
