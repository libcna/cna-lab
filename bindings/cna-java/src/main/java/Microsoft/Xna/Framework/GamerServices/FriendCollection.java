package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

/** A signed-in gamer's friends list. */
public final class FriendCollection extends GamerCollection<FriendGamer> implements AutoCloseable {

    private boolean disposed;

    FriendCollection(long handle) {
        super(handle, FriendGamer::new);
    }

    public void Dispose() {
        synchronized (this) {
            if (disposed) {
                return;
            }
            disposed = true;
        }
        NativeGamerServices.check("FriendCollection.Dispose",
                NativeGamerServicesRoutes.gamerCollectionDestroy(handle()));
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
        NativeGamerServices.check("FriendCollection.IsDisposed",
                NativeGamerServicesRoutes.friendCollectionGetIsDisposed(handle(), value));
        return value[0];
    }
}
