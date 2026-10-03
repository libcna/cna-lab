package org.openeggbert.cna.extensions.sensors;

import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.generated.NativeSensorExtensionRoutes;

/**
 * One handler attached to one sensor event.
 *
 * <p>The handle is <strong>owned</strong>: {@link #close()} detaches the handler from the sensor
 * and releases the reference that kept it alive. One route releases every sensor event, because a
 * registration already knows which event and which sensor it came from.
 *
 * <p><strong>Close it before the sensor.</strong> CNA's registration detaches from the sensor it
 * was created for, which must still exist -- so a sensor closed first leaves nothing to detach
 * from. Closing twice is a no-op here even though CNA refuses a second release, because
 * {@link AutoCloseable} requires it.
 */
public final class SensorSubscription implements AutoCloseable {

    private final String owner;
    private final long registration;
    private final long token;
    private boolean closed;

    SensorSubscription(String owner, long registration, long token) {
        this.owner = owner;
        this.registration = registration;
        this.token = token;
    }

    /** Detaches the handler and releases it. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        try {
            SensorExtension.check(owner + ".unsubscribe",
                    NativeSensorExtensionRoutes.sensorUnsubscribeExt(registration));
        } finally {
            // After the registration is gone nothing can deliver a reading, so the handler's
            // reference goes with it -- including when CNA refused the release, because a
            // registration that would not detach is not a reason to keep alive a handler that
            // can no longer be reached.
            NativeBindings.releaseCallbackToken(token);
        }
    }
}
