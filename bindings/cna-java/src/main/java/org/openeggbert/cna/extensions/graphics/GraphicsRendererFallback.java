package org.openeggbert.cna.extensions.graphics;

import java.util.Objects;

/**
 * One renderer CNA tried, passed over, and its reason for doing so.
 *
 * <p>A CNA extension. When {@link GraphicsRenderer#setAutomaticFallback} is on and the preferred
 * renderer cannot be created, CNA works down {@link GraphicsRenderer#setFallbackChain the chain}
 * and records what it rejected on the way. A game that starts on a renderer it did not ask for
 * gets the whole story here rather than a line in a log it does not read.
 *
 * <p>Immutable, and cheap: reading the history costs one call per record plus one for its message.
 */
public final class GraphicsRendererFallback {

    /** Why one renderer was passed over. */
    public enum Reason {

        /** The identity is not compiled into this build at all. */
        NotCompiledIn(0),
        /** The renderer's own availability probe reported it cannot run here. */
        ProbeUnavailable(1),
        /** The renderer was attempted and its construction failed. */
        InitializationFailed(2),
        /** The renderer needs a different window kind and the window could not be recreated. */
        WindowKindConflict(3);

        private final int value;

        Reason(int value) {
            this.value = value;
        }

        /**
         * Returns CNA's own number for this reason.
         *
         * @return the number
         */
        public int toValue() {
            return value;
        }

        /**
         * Returns the reason CNA gives one number.
         *
         * @param value CNA's number
         * @return the reason
         * @throws IllegalArgumentException when no reason has that number
         */
        public static Reason fromValue(long value) {
            for (Reason reason : values()) {
                if (reason.value == value) {
                    return reason;
                }
            }
            throw new IllegalArgumentException("no CNA fallback reason has the identity " + value);
        }
    }

    private final GraphicsRendererType type;
    private final Reason reason;
    private final String message;

    GraphicsRendererFallback(GraphicsRendererType type, Reason reason, String message) {
        this.type = Objects.requireNonNull(type, "type");
        this.reason = Objects.requireNonNull(reason, "reason");
        this.message = Objects.requireNonNull(message, "message");
    }

    /**
     * Returns the renderer that was tried and passed over.
     *
     * @return the identity
     */
    public GraphicsRendererType getType() {
        return type;
    }

    /**
     * Returns why it was passed over.
     *
     * @return the reason
     */
    public Reason getReason() {
        return reason;
    }

    /**
     * Returns CNA's diagnostic message for this record.
     *
     * <p>For {@link Reason#InitializationFailed} it is the message the renderer's own construction
     * produced, verbatim; for the others a short explanatory sentence. CNA documents that it is
     * never empty.
     *
     * @return the message
     */
    public String getMessage() {
        return message;
    }

    @Override
    public String toString() {
        return type + " passed over (" + reason + "): " + message;
    }
}
