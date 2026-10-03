package Microsoft.Xna.Framework.Net;

import org.openeggbert.cna.internal.NativeDeferredRelease;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;

/** One session a search found, and what joining it would cost. */
public final class AvailableNetworkSession {

    private final long handle;

    /**
     * Takes ownership of a discovered-session handle.
     *
     * <p>Every way of obtaining one produces an <strong>owned</strong> handle:
     * {@code cna_available_network_session_collection_copy_session} is documented to return "an
     * independent copy" that "stays valid after the collection is disposed", and the direct
     * constructor returns a new one. XNA's {@code AvailableNetworkSession} is not disposable, so
     * there is nowhere for a game to release it, and until this was written nothing did -- every
     * session a search returned leaked its handle.
     *
     * <p>The release is deferred to the thread that created this object, because CNA answers
     * {@code CNA_RESULT_THREAD} to a release from anywhere else. It happens once this object is
     * unreachable, the next time that thread pumps.
     *
     * <p>{@code this-escape} is suppressed because {@code Cleaner.register} keeps only a phantom
     * reference: nothing reads this object before the constructor returns.
     */
    @SuppressWarnings("this-escape")
    AvailableNetworkSession(long handle) {
        this.handle = handle;
        NativeDeferredRelease.onOwningThread(this, handle,
                NativeNetworkRoutes::availableNetworkSessionDestroy,
                "cna_available_network_session_destroy");
    }

    long handle() {
        return handle;
    }

    public int getCurrentGamerCount() {
        int[] value = new int[1];
        NativeGamerServices.check("AvailableNetworkSession.CurrentGamerCount",
                NativeNetworkRoutes.availableNetworkSessionGetCurrentGamerCount(handle, value));
        return value[0];
    }

    public String getHostGamertag() {
        return NativeGamerServices.text("AvailableNetworkSession.HostGamertag",
                out -> NativeNetworkRoutes.availableNetworkSessionGetHostGamertagSize(handle, out),
                (buffer, out) -> NativeNetworkRoutes.availableNetworkSessionCopyHostGamertag(
                        handle, buffer, out));
    }

    public int getOpenPrivateGamerSlots() {
        int[] value = new int[1];
        NativeGamerServices.check("AvailableNetworkSession.OpenPrivateGamerSlots",
                NativeNetworkRoutes.availableNetworkSessionGetOpenPrivateGamerSlots(handle, value));
        return value[0];
    }

    public int getOpenPublicGamerSlots() {
        int[] value = new int[1];
        NativeGamerServices.check("AvailableNetworkSession.OpenPublicGamerSlots",
                NativeNetworkRoutes.availableNetworkSessionGetOpenPublicGamerSlots(handle, value));
        return value[0];
    }

    public QualityOfService getQualityOfService() {
        long[] values = new long[5];
        NativeGamerServices.check("AvailableNetworkSession.QualityOfService",
                NativeNetworkRoutes.availableNetworkSessionGetQualityOfService(
                        handle, new byte[7], values));
        return new QualityOfService(values);
    }

    public NetworkSessionProperties getSessionProperties() {
        long[] properties = new long[1];
        NativeGamerServices.check("AvailableNetworkSession.SessionProperties",
                NativeNetworkRoutes.availableNetworkSessionCopySessionProperties(
                        handle, properties));
        return new NetworkSessionProperties(properties[0]);
    }
}
