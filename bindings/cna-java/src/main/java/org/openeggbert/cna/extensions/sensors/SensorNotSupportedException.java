package org.openeggbert.cna.extensions.sensors;

/**
 * Thrown when this CNA build carries no sensor layer at all.
 *
 * <p>This is not the same thing as a host with no accelerometer. A machine without the hardware
 * answers {@link SensorState#NotSupported} and its readings are not valid; this exception means
 * the route itself is absent, which {@code CNA_RESULT_NOT_SUPPORTED} reports and which keeps its
 * own identity rather than being flattened into an ordinary failure.
 */
public final class SensorNotSupportedException extends UnsupportedOperationException {

    private static final long serialVersionUID = 1L;

    SensorNotSupportedException(String message) {
        super(message);
    }
}
