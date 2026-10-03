package Microsoft.Xna.Framework.Net;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;

import java.util.AbstractList;

/** The sessions one search found. Disposing it releases every session it holds. */
public final class AvailableNetworkSessionCollection
        extends AbstractList<AvailableNetworkSession> implements AutoCloseable {

    private final long handle;
    private boolean disposed;

    AvailableNetworkSessionCollection(long handle) {
        this.handle = handle;
    }

    public void Dispose() {
        synchronized (this) {
            if (disposed) {
                return;
            }
            disposed = true;
        }
        NativeGamerServices.check("AvailableNetworkSessionCollection.Dispose",
                NativeNetworkRoutes.availableNetworkSessionCollectionDispose(handle));
    }

    @Override
    public void close() {
        Dispose();
    }

    public boolean getIsDisposed() {
        if (disposed) {
            return true;
        }
        boolean[] value = new boolean[1];
        NativeGamerServices.check("AvailableNetworkSessionCollection.IsDisposed",
                NativeNetworkRoutes.availableNetworkSessionCollectionGetIsDisposed(handle, value));
        return value[0];
    }

    @Override
    public AvailableNetworkSession get(int index) {
        long[] session = new long[1];
        NativeGamerServices.check("AvailableNetworkSessionCollection.get",
                NativeNetworkRoutes.availableNetworkSessionCollectionCopySession(
                        handle, index, session));
        return new AvailableNetworkSession(session[0]);
    }

    @Override
    public int size() {
        int[] count = new int[1];
        NativeGamerServices.check("AvailableNetworkSessionCollection.size",
                NativeNetworkRoutes.availableNetworkSessionCollectionGetCount(handle, count));
        return count[0];
    }
}
