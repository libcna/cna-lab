package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.GamerServices.GamerServicesNotAvailableException;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.time.Instant;

/**
 * Internal ergonomics over the generated CNA gamer-services routes.
 *
 * <p>This class is not application API. It owns three things the generated boundary
 * deliberately does not: the result-to-exception mapping XNA callers expect, the
 * count-then-copy string protocol, and the tick conversions between CNA's 100-nanosecond
 * integers and {@code java.time}.
 */
public final class NativeGamerServices {

    /** CNA's ticks are the CLR's: ten million per second. */
    public static final long TICKS_PER_SECOND = 10_000_000L;

    private static final int RESULT_SUCCESS = 0;
    private static final int RESULT_INVALID_STATE = 3;
    private static final int RESULT_NOT_SUPPORTED = 6;

    private NativeGamerServices() {
    }

    /**
     * Maps one CNA result to the exception XNA raises for the same condition.
     *
     * <p>XNA raises {@code GamerServicesNotAvailableException} both when the dispatcher was
     * never initialized and when the platform has no gamer services at all. CNA reports the
     * first as {@code CNA_RESULT_INVALID_STATE} and the second as
     * {@code CNA_RESULT_NOT_SUPPORTED}, so both map to that one XNA identity rather than to a
     * generic native failure. Every other result keeps the exact native diagnostic, because
     * guessing a more specific XNA exception from a result CNA cannot distinguish would be a
     * fabrication.
     */
    /**
     * Returns the diagnostic CNA recorded for a failed call, without throwing.
     *
     * <p>For the one caller that has to raise a different exception type than {@link #check} would
     * -- a failed join, which XNA reports as {@code NetworkSessionJoinException} -- and still
     * wants CNA's own message rather than a restatement of it.
     */
    public static String failureMessage(String operation, int result) {
        return NativeBindings.failure(operation, result).getMessage();
    }

    public static void check(String operation, int result) {
        if (result == RESULT_SUCCESS) {
            return;
        }
        if (result == RESULT_INVALID_STATE || result == RESULT_NOT_SUPPORTED) {
            throw new GamerServicesNotAvailableException(
                    NativeBindings.failure(operation, result).getMessage());
        }
        throw NativeBindings.failure(operation, result);
    }

    /** Requires the native backend and the gamer-services dispatcher to be usable. */
    public static void requireAvailable(String operation) {
        NativeBindings.requireAvailable();
        boolean[] initialized = new boolean[1];
        check(operation, NativeGamerServicesRoutes.gamerServicesDispatcherGetIsInitialized(initialized));
        if (!initialized[0]) {
            throw new GamerServicesNotAvailableException(
                    operation + " requires GamerServicesDispatcher.Initialize");
        }
    }

    public static byte[] utf8(String value) {
        return value == null ? new byte[0] : value.getBytes(StandardCharsets.UTF_8);
    }

    public static String string(byte[] bytes, int length) {
        return new String(bytes, 0, length, StandardCharsets.UTF_8);
    }

    /** One step of CNA's count-then-copy string protocol. */
    public interface SizeQuery {
        int query(long[] outBytes);
    }

    /** The copy step of CNA's count-then-copy string protocol. */
    public interface CopyQuery {
        int copy(byte[] destination, long[] outBytes);
    }

    /**
     * Reads a UTF-8 string through CNA's count-then-copy pair.
     *
     * <p>The size is read first because CNA never allocates for a caller; the buffer is then
     * exactly the size CNA reported, and the copy reports how much it actually wrote.
     */
    public static String text(String operation, SizeQuery size, CopyQuery copy) {
        long[] bytes = new long[1];
        check(operation, size.query(bytes));
        if (bytes[0] == 0L) {
            return "";
        }
        byte[] destination = new byte[Math.toIntExact(bytes[0])];
        check(operation, copy.copy(destination, bytes));
        return string(destination, Math.toIntExact(bytes[0]));
    }

    public static Duration duration(long ticks) {
        return Duration.ofSeconds(ticks / TICKS_PER_SECOND, (ticks % TICKS_PER_SECOND) * 100L);
    }

    public static long ticks(Duration duration) {
        return Math.addExact(Math.multiplyExact(duration.getSeconds(), TICKS_PER_SECOND),
                duration.getNano() / 100L);
    }

    /**
     * Converts CLR ticks since 0001-01-01 to an instant.
     *
     * <p>XNA reports achievement and leaderboard times as {@code DateTime}, whose epoch is
     * 6,213,559,680,000,000,000 ticks earlier than the Unix epoch. A zero tick count is the
     * CLR's {@code DateTime.MinValue}, which this projects as {@link Instant#EPOCH}'s CLR
     * counterpart rather than inventing a value.
     */
    public static Instant instant(long clrTicks) {
        long unixTicks = clrTicks - 621_355_968_000_000_000L;
        return Instant.ofEpochSecond(Math.floorDiv(unixTicks, TICKS_PER_SECOND),
                Math.floorMod(unixTicks, TICKS_PER_SECOND) * 100L);
    }

    /**
     * Shows a Guide message box with one or two buttons.
     *
     * <p>Hand-written rather than generated: CNA takes an array of {@code CNA_StringView},
     * which has no scalar projection. XNA's own contract allows one or two buttons, so the two
     * captions cross as separate UTF-8 arrays with the count actually used.
     */
    public static native int nativeGuideShowMessageBox(
            int player, byte[] title, byte[] text, byte[] firstButton, byte[] secondButton,
            int buttonCount, int focusButton, int icon);

    /** Subscribes the three process-wide gamer-service events. */
    static native int nativeSubscribeGamerEvents(long[] outRegistrations);

    /** Releases the three process-wide gamer-service registrations. */
    static native int nativeUnsubscribeGamerEvents(long[] registrations);

    /** Subscribes one session's nine events plus the static accepted-invite event. */
    static native int nativeSubscribeSessionEvents(long session, long[] outRegistrations);

    /** Releases one session's ten registrations, in reverse subscription order. */
    static native int nativeUnsubscribeSessionEvents(long[] registrations);

    /** Drains one queued event into {@code record}; false when the queue is empty. */
    static native boolean nativePollEvent(long[] record);

    /** Returns how many events were dropped because the queue was full. */
    static native long nativeDroppedEventCount();

    /** Discards every queued event. */
    static native void nativeResetEvents();

    /** Subscribes CNA's typed-character event. */
    static native int nativeSubscribeTextInput(long[] outRegistration);

    /** Releases the typed-character subscription. */
    static native int nativeUnsubscribeTextInput(long registration);

    /** Subscribes the four process-wide mouse and keyboard hot-plug events. */
    static native int nativeSubscribeInputDeviceEvents(long[] outRegistrations);

    /** Releases the four mouse and keyboard hot-plug registrations. */
    static native int nativeUnsubscribeInputDeviceEvents(long[] registrations);

    /** Subscribes both process-wide raw-joystick hot-plug events. */
    static native int nativeSubscribeJoystickEvents(long[] outRegistrations);

    /** Releases both raw-joystick hot-plug registrations. */
    static native int nativeUnsubscribeJoystickEvents(long[] registrations);

    /** Subscribes CNA's process-wide mouse-click event. */
    static native int nativeSubscribeMouseClicked(long[] outRegistration);

    /** Releases the mouse-click subscription. */
    static native int nativeUnsubscribeMouseClicked(long registration);

    /** Subscribes CNA's two string-carrying text events. */
    static native int nativeSubscribeTextComposition(long[] outRegistrations);

    /** Releases both composition registrations. */
    static native int nativeUnsubscribeTextComposition(long[] registrations);

    /**
     * Drains one string-carrying event.
     *
     * <p>Returns null when the queue is empty, otherwise the UTF-8 payloads with the numeric
     * header written into {@code outHeader}. The native record is freed before this returns, so
     * nothing Java holds afterwards refers to native memory.
     */
    static native byte[][] nativePollTextEvent(long[] outHeader);

    /** Returns how many string events were dropped because the queue was full. */
    static native long nativeDroppedTextEventCount();

    /** Discards every queued string event. */
    static native void nativeResetTextEvents();

    /**
     * Raises CNA's candidate-list event.
     *
     * <p>Hand-written rather than generated: the route takes an array of string views, which the
     * generator refuses because it cannot prove where the bytes behind them live.
     */
    public static native int nativeRaiseTextCandidates(
            long game, byte[][] candidates, int selected, boolean horizontal);

    /** Converts this adapter's game token into the CNA game handle a generated route needs. */
    static native long nativeCnaGameHandle(long game);

    /** Resolves the CNA graphics-device handle behind one game. */
    static native int nativeGraphicsDeviceHandle(long game, long[] outDevice);

    public static long clrTicks(Instant instant) {
        return Math.addExact(
                Math.addExact(Math.multiplyExact(instant.getEpochSecond(), TICKS_PER_SECOND),
                        instant.getNano() / 100L),
                621_355_968_000_000_000L);
    }
}
