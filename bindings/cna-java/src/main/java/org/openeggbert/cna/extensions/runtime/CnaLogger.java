package org.openeggbert.cna.extensions.runtime;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeRuntimeExtensionRoutes;

import java.util.Objects;

/**
 * CNA's own logger, so a game's diagnostics land in the same stream as the runtime's.
 *
 * <p>A CNA extension: XNA 4.0 has no logging API. The minimum level is process-wide state, so a
 * library that raises it should restore it.
 */
public final class CnaLogger {

    private CnaLogger() {
    }

    public static void Fatal(String message, LogCategory category) {
        Log(LogLevel.Fatal, message, category);
    }

    public static void Error(String message, LogCategory category) {
        Log(LogLevel.Error, message, category);
    }

    public static void Warn(String message, LogCategory category) {
        Log(LogLevel.Warn, message, category);
    }

    public static void Info(String message, LogCategory category) {
        Log(LogLevel.Info, message, category);
    }

    public static void Debug(String message, LogCategory category) {
        Log(LogLevel.Debug, message, category);
    }

    public static void Trace(String message, LogCategory category) {
        Log(LogLevel.Trace, message, category);
    }

    /** Writes one message at an explicit level. */
    public static void Log(LogLevel level, String message, LogCategory category) {
        Log(level, message, category, true);
    }

    /** Writes one message only when {@code condition} holds, as CNA's own conditional log does. */
    public static void Log(LogLevel level, String message, LogCategory category,
            boolean condition) {
        NativeBindings.requireAvailable();
        int result = NativeRuntimeExtensionRoutes.loggerLog(
                Objects.requireNonNull(level, "level").getValue(),
                NativeGamerServices.utf8(Objects.requireNonNull(message, "message")),
                Objects.requireNonNull(category, "category").ordinal(), condition);
        if (result != 0) {
            throw NativeBindings.failure("CnaLogger.Log", result);
        }
    }

    /**
     * Sends every line CNA writes to a Java sink instead of its own stderr one.
     *
     * <p>Process-wide, like the minimum level: a library that installs a sink should restore the
     * previous state with {@link #resetSink()}. Installing a second sink replaces the first, and
     * the adapter releases its reference to the one it replaced only after CNA has stopped
     * calling through it -- so a line already inside CNA's logger finds either the sink it
     * started with or nothing, never a released reference.
     *
     * <p>CNA hands the line over as bytes borrowed for the call; they are copied before the sink
     * sees them, so nothing here outlives the callback.
     *
     * @param sink where lines go
     * @see LogSink for what a sink must not do
     */
    public static void setSink(LogSink sink) {
        Objects.requireNonNull(sink, "sink");
        NativeBindings.requireAvailable();
        int result = NativeBindings.loggerSetSink(new NativeLogSink(sink));
        if (result != 0) {
            throw NativeBindings.failure("CnaLogger.setSink", result);
        }
    }

    /**
     * Restores CNA's own stderr sink and releases the Java one.
     *
     * <p>Calling it with no sink installed is not an error: it is how a caller that does not know
     * whether one was installed gets back to the default.
     */
    public static void resetSink() {
        NativeBindings.requireAvailable();
        int result = NativeBindings.loggerSetSink(null);
        if (result != 0) {
            throw NativeBindings.failure("CnaLogger.resetSink", result);
        }
    }

    /**
     * What the adapter calls, which is deliberately not {@link LogSink} itself.
     *
     * <p>The native side needs a method taking CNA's own two integers and the raw bytes; a game's
     * sink should see a {@link LogLevel}, a {@link LogCategory} and a {@code String}. Keeping the
     * translation here rather than in the JNI adapter means the adapter looks up one fixed
     * signature on one class it owns, instead of reflecting over whatever a caller passed.
     */
    private record NativeLogSink(LogSink sink) {

        /**
         * Called from whichever CNA thread wrote the line.
         *
         * @param level CNA's own numeric level
         * @param category CNA's own numeric category
         * @param message the formatted line as UTF-8 bytes
         */
        void acceptNativeLine(int level, int category, byte[] message) {
            LogCategory[] categories = LogCategory.values();
            // An unknown identity must not become an exception thrown back through CNA's
            // logger; a line from a category this build has no name for is still a line.
            LogCategory named = category >= 0 && category < categories.length
                    ? categories[category] : LogCategory.System;
            sink.accept(LogLevel.fromValue(level), named,
                    new String(message, java.nio.charset.StandardCharsets.UTF_8));
        }
    }

    /** Returns the process-wide minimum level CNA is currently writing. */
    public static LogLevel getMinimumLevel() {
        NativeBindings.requireAvailable();
        int[] level = new int[1];
        int result = NativeRuntimeExtensionRoutes.loggerGetMinimumLevel(level);
        if (result != 0) {
            throw NativeBindings.failure("CnaLogger.getMinimumLevel", result);
        }
        return LogLevel.fromValue(level[0]);
    }

    /** Sets the process-wide minimum level. A library that raises it should restore it. */
    public static void setMinimumLevel(LogLevel value) {
        NativeBindings.requireAvailable();
        int result = NativeRuntimeExtensionRoutes.loggerSetMinimumLevel(
                Objects.requireNonNull(value, "value").getValue());
        if (result != 0) {
            throw NativeBindings.failure("CnaLogger.setMinimumLevel", result);
        }
    }
}
