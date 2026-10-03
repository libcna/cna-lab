package org.openeggbert.cna.extensions.runtime;

/**
 * The severity of a log message.
 *
 * <p>A CNA extension: XNA 4.0 has no logging API. The numbers are CNA's own and are deliberately
 * not contiguous -- {@code Experiment} is 100, not 6 -- so the value is carried explicitly rather
 * than taken from the Java ordinal.
 */
public enum LogLevel {
    Fatal(0),
    Error(1),
    Warn(2),
    Info(3),
    Debug(4),
    Trace(5),
    Experiment(100);

    private final int value;

    LogLevel(int value) {
        this.value = value;
    }

    /** Returns the exact numeric value CNA gives this level. */
    public int getValue() {
        return value;
    }

    static LogLevel fromValue(int value) {
        for (LogLevel level : values()) {
            if (level.value == value) {
                return level;
            }
        }
        throw new IllegalStateException("CNA reported an unknown log level: " + value);
    }
}
