package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.CompletedAsyncResult;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;
import System.AsyncCallback;
import System.IAsyncResult;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/** One page of a leaderboard, with the cursor that walks to the next page or the previous one. */
public final class LeaderboardReader implements AutoCloseable {

    private final long handle;
    private boolean disposed;

    private LeaderboardReader(long handle) {
        this.handle = handle;
    }

    public IAsyncResult BeginPageDown(AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, this::PageDown);
    }

    public IAsyncResult BeginPageUp(AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, this::PageUp);
    }

    public static IAsyncResult BeginRead(
            LeaderboardIdentity leaderboardId, Iterable<Gamer> gamers, Gamer pivotGamer,
            int pageSize, AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState,
                () -> Read(leaderboardId, gamers, pivotGamer, pageSize));
    }

    public static IAsyncResult BeginRead(
            LeaderboardIdentity leaderboardId, int pageStart, int pageSize,
            AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState,
                () -> Read(leaderboardId, pageStart, pageSize));
    }

    public static IAsyncResult BeginRead(
            LeaderboardIdentity leaderboardId, Gamer pivotGamer, int pageSize,
            AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState,
                () -> Read(leaderboardId, pivotGamer, pageSize));
    }

    public void Dispose() {
        synchronized (this) {
            if (disposed) {
                return;
            }
            disposed = true;
        }
        NativeGamerServices.check("LeaderboardReader.Dispose",
                NativeGamerServicesRoutes.leaderboardReaderDestroy(handle));
    }

    @Override
    public void close() {
        Dispose();
    }

    public void EndPageDown(IAsyncResult result) {
        CompletedAsyncResult.endVoid(result);
    }

    public void EndPageUp(IAsyncResult result) {
        CompletedAsyncResult.endVoid(result);
    }

    public static LeaderboardReader EndRead(IAsyncResult result) {
        return CompletedAsyncResult.end(result, LeaderboardReader.class);
    }

    public void PageDown() {
        NativeGamerServices.check("LeaderboardReader.PageDown",
                NativeGamerServicesRoutes.leaderboardReaderPageDown(handle));
    }

    public void PageUp() {
        NativeGamerServices.check("LeaderboardReader.PageUp",
                NativeGamerServicesRoutes.leaderboardReaderPageUp(handle));
    }

    public static LeaderboardReader Read(
            LeaderboardIdentity leaderboardId, Iterable<Gamer> gamers, Gamer pivotGamer,
            int pageSize) {
        NativeGamerServices.requireAvailable("LeaderboardReader.Read");
        List<Long> handles = new ArrayList<>();
        for (Gamer gamer : Objects.requireNonNull(gamers, "gamers")) {
            handles.add(Objects.requireNonNull(gamer, "gamers").handle());
        }
        long[] values = new long[handles.size()];
        for (int index = 0; index < values.length; index++) {
            values[index] = handles.get(index);
        }
        long[] reader = new long[1];
        NativeGamerServices.check("LeaderboardReader.Read",
                NativeGamerServicesRoutes.leaderboardReaderReadFromGamers(
                        identityBytes(leaderboardId), identityIntegers(leaderboardId), values,
                        pivotGamer == null ? 0L : pivotGamer.handle(), pageSize, reader));
        return new LeaderboardReader(reader[0]);
    }

    public static LeaderboardReader Read(
            LeaderboardIdentity leaderboardId, int pageStart, int pageSize) {
        NativeGamerServices.requireAvailable("LeaderboardReader.Read");
        long[] reader = new long[1];
        NativeGamerServices.check("LeaderboardReader.Read",
                NativeGamerServicesRoutes.leaderboardReaderRead(
                        identityBytes(leaderboardId), identityIntegers(leaderboardId),
                        pageStart, pageSize, reader));
        return new LeaderboardReader(reader[0]);
    }

    public static LeaderboardReader Read(
            LeaderboardIdentity leaderboardId, Gamer pivotGamer, int pageSize) {
        NativeGamerServices.requireAvailable("LeaderboardReader.Read");
        long[] reader = new long[1];
        NativeGamerServices.check("LeaderboardReader.Read",
                NativeGamerServicesRoutes.leaderboardReaderReadFromPivot(
                        identityBytes(leaderboardId), identityIntegers(leaderboardId),
                        Objects.requireNonNull(pivotGamer, "pivotGamer").handle(),
                        pageSize, reader));
        return new LeaderboardReader(reader[0]);
    }

    public boolean getCanPageDown() {
        return info()[4] != 0L;
    }

    public boolean getCanPageUp() {
        return info()[5] != 0L;
    }

    /** Returns an unmodifiable snapshot of this page's rows. */
    public List<LeaderboardEntry> getEntries() {
        int count = (int) info()[2];
        List<LeaderboardEntry> entries = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            long[] entry = new long[1];
            NativeGamerServices.check("LeaderboardReader.Entries",
                    NativeGamerServicesRoutes.leaderboardReaderGetEntryAt(handle, index, entry));
            entries.add(new LeaderboardEntry(entry[0]));
        }
        return Collections.unmodifiableList(entries);
    }

    public boolean getIsDisposed() {
        return disposed || info()[3] != 0L;
    }

    public LeaderboardIdentity getLeaderboardIdentity() {
        byte[] key = new byte[64];
        long[] gameMode = new long[1];
        NativeGamerServices.check("LeaderboardReader.LeaderboardIdentity",
                NativeGamerServicesRoutes.leaderboardReaderGetIdentity(handle, key, gameMode));
        LeaderboardIdentity identity = new LeaderboardIdentity();
        identity.setKey(NativeGamerServices.string(key, terminator(key)));
        identity.setGameMode((int) gameMode[0]);
        return identity;
    }

    public int getPageStart() {
        return (int) info()[0];
    }

    public int getTotalLeaderboardSize() {
        return (int) info()[1];
    }

    /** Packs an identity's fixed 64-byte key buffer exactly as CNA lays it out. */
    private static byte[] identityBytes(LeaderboardIdentity identity) {
        Objects.requireNonNull(identity, "leaderboardId");
        byte[] buffer = new byte[64];
        byte[] key = NativeGamerServices.utf8(identity.getKey());
        if (key.length >= buffer.length) {
            throw new IllegalArgumentException(
                    "A leaderboard key must be shorter than 64 UTF-8 bytes");
        }
        System.arraycopy(key, 0, buffer, 0, key.length);
        return buffer;
    }

    private static long[] identityIntegers(LeaderboardIdentity identity) {
        return new long[] {identity.getGameMode()};
    }

    private static int terminator(byte[] buffer) {
        for (int index = 0; index < buffer.length; index++) {
            if (buffer[index] == 0) {
                return index;
            }
        }
        return buffer.length;
    }

    private long[] info() {
        long[] values = new long[7];
        NativeGamerServices.check("LeaderboardReader",
                NativeGamerServicesRoutes.leaderboardReaderGetInfo(handle, values));
        return values;
    }
}
