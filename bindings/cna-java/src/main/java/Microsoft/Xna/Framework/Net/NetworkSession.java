package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.EventHandler;
import Microsoft.Xna.Framework.GamerServices.GamerCollection;
import Microsoft.Xna.Framework.GamerServices.InviteAcceptedEventArgs;
import Microsoft.Xna.Framework.GamerServices.SignedInGamer;
import org.openeggbert.cna.internal.CompletedAsyncResult;
import org.openeggbert.cna.internal.GamerEventPump;
import org.openeggbert.cna.internal.GamerHandles;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;
import System.AsyncCallback;
import System.IAsyncResult;

import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * One multiplayer session: its gamers, its state, and the events that report both changing.
 *
 * <p>Every asynchronous member follows XNA's {@code Begin}/{@code End} pair. CNA completes each
 * one synchronously -- its own asynchronous routes are one synchronous call that still invokes
 * the completion callback -- so the result reports {@code CompletedSynchronously} truthfully
 * and {@code End} returns immediately.
 *
 * <p>{@code StartGame} and {@code EndGame} only queue a state change; it takes effect when
 * {@link #Update()} runs, which is what XNA's session does too.
 */
public final class NetworkSession implements AutoCloseable {

    /** The most previous gamers a session remembers. */
    public static final int MaxPreviousGamers = 100;

    /** The most gamers one session can hold. */
    public static final int MaxSupportedGamers = 31;

    private static final int ROSTER_ALL = 0;
    private static final int ROSTER_LOCAL = 1;
    private static final int ROSTER_REMOTE = 2;
    private static final int ROSTER_PREVIOUS = 3;

    /** The event kinds CNA raises for a session, matching the JNI adapter's numbering. */
    private static final int KIND_GAME_ENDED = 10;
    private static final int KIND_GAME_STARTED = 11;
    private static final int KIND_GAMER_JOINED = 12;
    private static final int KIND_GAMER_LEFT = 13;
    private static final int KIND_HOST_CHANGED = 14;
    private static final int KIND_SESSION_ENDED = 15;
    private static final int KIND_WRITE_ARBITRATED = 16;
    private static final int KIND_WRITE_TRUE_SKILL = 17;
    private static final int KIND_WRITE_UNARBITRATED = 18;
    private static final int KIND_INVITE_ACCEPTED = 19;

    private static final List<EventHandler<InviteAcceptedEventArgs>> INVITE_ACCEPTED =
            new CopyOnWriteArrayList<>();

    /** Every live session, so a queued event reaches the one it belongs to. */
    private static final Map<Long, NetworkSession> LIVE = new ConcurrentHashMap<>();

    static {
        // The listeners live in this package, so the pump's session handler does too.
        GamerEventPump.setSessionHandler(NetworkSession::dispatch);
    }

    private final List<EventHandler<GameEndedEventArgs>> gameEnded = new CopyOnWriteArrayList<>();
    private final List<EventHandler<GameStartedEventArgs>> gameStarted =
            new CopyOnWriteArrayList<>();
    private final List<EventHandler<GamerJoinedEventArgs>> gamerJoined =
            new CopyOnWriteArrayList<>();
    private final List<EventHandler<GamerLeftEventArgs>> gamerLeft = new CopyOnWriteArrayList<>();
    private final List<EventHandler<HostChangedEventArgs>> hostChanged =
            new CopyOnWriteArrayList<>();
    private final List<EventHandler<NetworkSessionEndedEventArgs>> sessionEnded =
            new CopyOnWriteArrayList<>();
    private final List<EventHandler<WriteLeaderboardsEventArgs>> writeArbitrated =
            new CopyOnWriteArrayList<>();
    private final List<EventHandler<WriteLeaderboardsEventArgs>> writeTrueSkill =
            new CopyOnWriteArrayList<>();
    private final List<EventHandler<WriteLeaderboardsEventArgs>> writeUnarbitrated =
            new CopyOnWriteArrayList<>();

    private final long handle;
    private final boolean owned;
    private long[] registrations;
    private boolean disposed;

    private NetworkSession(long handle, boolean owned) {
        this.handle = handle;
        this.owned = owned;
        if (owned) {
            // A session subscribes on creation and unsubscribes on disposal, so its events have
            // a producer for exactly as long as the session exists.
            registrations = GamerEventPump.subscribeSession(handle);
            LIVE.put(handle, this);
        }
    }

    static NetworkSession borrowed(long handle) {
        NetworkSession live = LIVE.get(handle);
        return live != null ? live : new NetworkSession(handle, false);
    }

    public void addGameEndedListener(EventHandler<GameEndedEventArgs> listener) {
        gameEnded.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeGameEndedListener(EventHandler<GameEndedEventArgs> listener) {
        gameEnded.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void addGameStartedListener(EventHandler<GameStartedEventArgs> listener) {
        gameStarted.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeGameStartedListener(EventHandler<GameStartedEventArgs> listener) {
        gameStarted.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void addGamerJoinedListener(EventHandler<GamerJoinedEventArgs> listener) {
        gamerJoined.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeGamerJoinedListener(EventHandler<GamerJoinedEventArgs> listener) {
        gamerJoined.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void addGamerLeftListener(EventHandler<GamerLeftEventArgs> listener) {
        gamerLeft.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeGamerLeftListener(EventHandler<GamerLeftEventArgs> listener) {
        gamerLeft.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void addHostChangedListener(EventHandler<HostChangedEventArgs> listener) {
        hostChanged.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeHostChangedListener(EventHandler<HostChangedEventArgs> listener) {
        hostChanged.remove(Objects.requireNonNull(listener, "listener"));
    }

    public static void addInviteAcceptedListener(
            EventHandler<InviteAcceptedEventArgs> listener) {
        INVITE_ACCEPTED.add(Objects.requireNonNull(listener, "listener"));
    }

    public static void removeInviteAcceptedListener(
            EventHandler<InviteAcceptedEventArgs> listener) {
        INVITE_ACCEPTED.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void addSessionEndedListener(EventHandler<NetworkSessionEndedEventArgs> listener) {
        sessionEnded.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeSessionEndedListener(EventHandler<NetworkSessionEndedEventArgs> listener) {
        sessionEnded.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void addWriteArbitratedLeaderboardListener(
            EventHandler<WriteLeaderboardsEventArgs> listener) {
        writeArbitrated.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeWriteArbitratedLeaderboardListener(
            EventHandler<WriteLeaderboardsEventArgs> listener) {
        writeArbitrated.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void addWriteTrueSkillListener(EventHandler<WriteLeaderboardsEventArgs> listener) {
        writeTrueSkill.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeWriteTrueSkillListener(EventHandler<WriteLeaderboardsEventArgs> listener) {
        writeTrueSkill.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void addWriteUnarbitratedLeaderboardListener(
            EventHandler<WriteLeaderboardsEventArgs> listener) {
        writeUnarbitrated.add(Objects.requireNonNull(listener, "listener"));
    }

    public void removeWriteUnarbitratedLeaderboardListener(
            EventHandler<WriteLeaderboardsEventArgs> listener) {
        writeUnarbitrated.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void AddLocalGamer(SignedInGamer gamer) {
        NativeGamerServices.check("NetworkSession.AddLocalGamer",
                NativeNetworkRoutes.networkSessionAddLocalGamer(handle,
                        GamerHandles.of(Objects.requireNonNull(gamer, "gamer"))));
    }

    public static IAsyncResult BeginCreate(NetworkSessionType sessionType,
            Iterable<SignedInGamer> localGamers, int maxGamers, int privateGamerSlots,
            NetworkSessionProperties sessionProperties, AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, () -> Create(
                sessionType, localGamers, maxGamers, privateGamerSlots, sessionProperties));
    }

    public static IAsyncResult BeginCreate(NetworkSessionType sessionType, int maxLocalGamers,
            int maxGamers, AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState,
                () -> Create(sessionType, maxLocalGamers, maxGamers));
    }

    public static IAsyncResult BeginCreate(NetworkSessionType sessionType, int maxLocalGamers,
            int maxGamers, int privateGamerSlots, NetworkSessionProperties sessionProperties,
            AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, () -> Create(
                sessionType, maxLocalGamers, maxGamers, privateGamerSlots, sessionProperties));
    }

    public static IAsyncResult BeginFind(NetworkSessionType sessionType,
            Iterable<SignedInGamer> localGamers, NetworkSessionProperties searchProperties,
            AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState,
                () -> Find(sessionType, localGamers, searchProperties));
    }

    public static IAsyncResult BeginFind(NetworkSessionType sessionType, int maxLocalGamers,
            NetworkSessionProperties searchProperties, AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState,
                () -> Find(sessionType, maxLocalGamers, searchProperties));
    }

    public static IAsyncResult BeginJoin(AvailableNetworkSession availableSession,
            AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, () -> Join(availableSession));
    }

    public static IAsyncResult BeginJoinInvited(Iterable<SignedInGamer> localGamers,
            AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, () -> JoinInvited(localGamers));
    }

    public static IAsyncResult BeginJoinInvited(int maxLocalGamers, AsyncCallback callback,
            Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, () -> JoinInvited(maxLocalGamers));
    }

    public static NetworkSession Create(NetworkSessionType sessionType,
            Iterable<SignedInGamer> localGamers, int maxGamers, int privateGamerSlots,
            NetworkSessionProperties sessionProperties) {
        long[] session = new long[1];
        NativeGamerServices.check("NetworkSession.Create",
                NativeNetworkRoutes.networkSessionCreateWithLocalGamers(
                        kind(sessionType), handles(localGamers), maxGamers, privateGamerSlots,
                        properties(sessionProperties), session));
        return new NetworkSession(session[0], true);
    }

    public static NetworkSession Create(NetworkSessionType sessionType, int maxLocalGamers,
            int maxGamers, int privateGamerSlots, NetworkSessionProperties sessionProperties) {
        long[] session = new long[1];
        NativeGamerServices.check("NetworkSession.Create",
                NativeNetworkRoutes.networkSessionCreateWithProperties(
                        kind(sessionType), maxLocalGamers, maxGamers, privateGamerSlots,
                        properties(sessionProperties), session));
        return new NetworkSession(session[0], true);
    }

    public static NetworkSession Create(NetworkSessionType sessionType, int maxLocalGamers,
            int maxGamers) {
        long[] session = new long[1];
        NativeGamerServices.check("NetworkSession.Create",
                NativeNetworkRoutes.networkSessionCreate(
                        kind(sessionType), maxLocalGamers, maxGamers, session));
        return new NetworkSession(session[0], true);
    }

    public void Dispose() {
        synchronized (this) {
            if (disposed) {
                return;
            }
            disposed = true;
        }
        if (registrations != null) {
            // Release the subscriptions before the session they belong to, so no callback can
            // arrive for a session that is being torn down.
            LIVE.remove(handle, this);
            GamerEventPump.unsubscribeSession(registrations);
            registrations = null;
        }
        NativeGamerServices.check("NetworkSession.Dispose",
                NativeNetworkRoutes.networkSessionDispose(handle));
        if (owned) {
            NativeGamerServices.check("NetworkSession.Dispose",
                    NativeNetworkRoutes.networkSessionDestroy(handle));
        }
    }

    @Override
    public void close() {
        Dispose();
    }

    public static NetworkSession EndCreate(IAsyncResult result) {
        return CompletedAsyncResult.end(result, NetworkSession.class);
    }

    public static AvailableNetworkSessionCollection EndFind(IAsyncResult result) {
        return CompletedAsyncResult.end(result, AvailableNetworkSessionCollection.class);
    }

    public void EndGame() {
        NativeGamerServices.check("NetworkSession.EndGame",
                NativeNetworkRoutes.networkSessionEndGame(handle));
    }

    public static NetworkSession EndJoin(IAsyncResult result) {
        return CompletedAsyncResult.end(result, NetworkSession.class);
    }

    public static NetworkSession EndJoinInvited(IAsyncResult result) {
        return CompletedAsyncResult.end(result, NetworkSession.class);
    }

    public static AvailableNetworkSessionCollection Find(NetworkSessionType sessionType,
            Iterable<SignedInGamer> localGamers, NetworkSessionProperties searchProperties) {
        long[] collection = new long[1];
        NativeGamerServices.check("NetworkSession.Find",
                NativeNetworkRoutes.networkSessionFindWithLocalGamers(kind(sessionType),
                        handles(localGamers), properties(searchProperties), collection));
        return new AvailableNetworkSessionCollection(collection[0]);
    }

    public static AvailableNetworkSessionCollection Find(NetworkSessionType sessionType,
            int maxLocalGamers, NetworkSessionProperties searchProperties) {
        long[] collection = new long[1];
        NativeGamerServices.check("NetworkSession.Find",
                NativeNetworkRoutes.networkSessionFind(kind(sessionType), maxLocalGamers,
                        properties(searchProperties), collection));
        return new AvailableNetworkSessionCollection(collection[0]);
    }

    /** Returns the gamer with this session-unique id, or {@code null} when there is none. */
    public NetworkGamer FindGamerById(int gamerId) {
        if (gamerId < 0 || gamerId > 255) {
            throw new IllegalArgumentException(
                    "gamerId is " + gamerId + ", outside the byte range 0..255");
        }
        long[] gamer = new long[1];
        NativeGamerServices.check("NetworkSession.FindGamerById",
                NativeNetworkRoutes.networkSessionFindGamerById(
                        handle, (byte) gamerId, gamer));
        return gamer[0] == 0L ? null : new NetworkGamer(gamer[0]);
    }

    public static NetworkSession Join(AvailableNetworkSession availableSession) {
        long[] session = new long[1];
        checkJoin("NetworkSession.Join",
                NativeNetworkRoutes.networkSessionJoin(
                        Objects.requireNonNull(availableSession, "availableSession").handle(),
                        session));
        return new NetworkSession(session[0], true);
    }

    public static NetworkSession JoinInvited(Iterable<SignedInGamer> localGamers) {
        long[] session = new long[1];
        checkJoin("NetworkSession.JoinInvited",
                NativeNetworkRoutes.networkSessionJoinInvitedWithLocalGamers(
                        handles(localGamers), session));
        return new NetworkSession(session[0], true);
    }

    public static NetworkSession JoinInvited(int maxLocalGamers) {
        long[] session = new long[1];
        checkJoin("NetworkSession.JoinInvited",
                NativeNetworkRoutes.networkSessionJoinInvited(maxLocalGamers, session));
        return new NetworkSession(session[0], true);
    }

    /**
     * Turns a failed join into the exception XNA raises, carrying the reason CNA recorded.
     *
     * <p>XNA throws {@link NetworkSessionJoinException} from a join and a game reads
     * {@link NetworkSessionJoinException#getJoinError()} to tell a full session from one that is
     * gone. CNA's join error never crosses as a return value -- the canonical exception carries
     * it on the object -- so the C firewall records it per thread and this reads it back on the
     * thread that just failed. When CNA recorded no join error the failure was not a join
     * failure, and it is reported as whatever it was rather than dressed up as one.
     */
    private static void checkJoin(String operation, int result) {
        if (result == 0) {
            return;
        }
        int[] joinError = new int[1];
        boolean[] present = new boolean[1];
        if (NativeNetworkRoutes.netGetLastJoinError(joinError, present) == 0 && present[0]) {
            NetworkSessionJoinError[] errors = NetworkSessionJoinError.values();
            NetworkSessionJoinError error = joinError[0] >= 0 && joinError[0] < errors.length
                    ? errors[joinError[0]]
                    : NetworkSessionJoinError.SessionNotFound;
            throw new NetworkSessionJoinException(
                    NativeGamerServices.failureMessage(operation, result), error);
        }
        NativeGamerServices.check(operation, result);
    }

    public void ResetReady() {
        NativeGamerServices.check("NetworkSession.ResetReady",
                NativeNetworkRoutes.networkSessionResetReady(handle));
    }

    public void StartGame() {
        NativeGamerServices.check("NetworkSession.StartGame",
                NativeNetworkRoutes.networkSessionStartGame(handle));
    }

    /**
     * Pumps the session: queued state changes take effect and queued events are raised.
     *
     * <p>Draining here is what puts a joined gamer, a host change or a game start on the game
     * thread during {@code Update}, which is where XNA raises them.
     */
    public void Update() {
        NativeGamerServices.check("NetworkSession.Update",
                NativeNetworkRoutes.networkSessionUpdate(handle));
        GamerEventPump.drain();
    }

    public GamerCollection<NetworkGamer> getAllGamers() {
        return roster(ROSTER_ALL);
    }

    public boolean getAllowHostMigration() {
        return flag("AllowHostMigration",
                value -> NativeNetworkRoutes.networkSessionGetAllowHostMigration(handle, value));
    }

    public void setAllowHostMigration(boolean value) {
        NativeGamerServices.check("NetworkSession.AllowHostMigration",
                NativeNetworkRoutes.networkSessionSetAllowHostMigration(handle, value));
    }

    public boolean getAllowJoinInProgress() {
        return flag("AllowJoinInProgress",
                value -> NativeNetworkRoutes.networkSessionGetAllowJoinInProgress(handle, value));
    }

    public void setAllowJoinInProgress(boolean value) {
        NativeGamerServices.check("NetworkSession.AllowJoinInProgress",
                NativeNetworkRoutes.networkSessionSetAllowJoinInProgress(handle, value));
    }

    public int getBytesPerSecondReceived() {
        return number("BytesPerSecondReceived",
                value -> NativeNetworkRoutes.networkSessionGetBytesPerSecondReceived(
                        handle, value));
    }

    public int getBytesPerSecondSent() {
        return number("BytesPerSecondSent",
                value -> NativeNetworkRoutes.networkSessionGetBytesPerSecondSent(handle, value));
    }

    public NetworkGamer getHost() {
        long[] gamer = new long[1];
        NativeGamerServices.check("NetworkSession.Host",
                NativeNetworkRoutes.networkSessionGetHost(handle, gamer));
        return gamer[0] == 0L ? null : new NetworkGamer(gamer[0]);
    }

    public boolean getIsDisposed() {
        if (disposed) {
            return true;
        }
        return flag("IsDisposed",
                value -> NativeNetworkRoutes.networkSessionGetIsDisposed(handle, value));
    }

    public boolean getIsEveryoneReady() {
        return flag("IsEveryoneReady",
                value -> NativeNetworkRoutes.networkSessionGetIsEveryoneReady(handle, value));
    }

    public boolean getIsHost() {
        return flag("IsHost",
                value -> NativeNetworkRoutes.networkSessionGetIsHost(handle, value));
    }

    public GamerCollection<LocalNetworkGamer> getLocalGamers() {
        return new LocalRoster(handle);
    }

    public int getMaxGamers() {
        return number("MaxGamers",
                value -> NativeNetworkRoutes.networkSessionGetMaxGamers(handle, value));
    }

    public void setMaxGamers(int value) {
        NativeGamerServices.check("NetworkSession.MaxGamers",
                NativeNetworkRoutes.networkSessionSetMaxGamers(handle, value));
    }

    public GamerCollection<NetworkGamer> getPreviousGamers() {
        return roster(ROSTER_PREVIOUS);
    }

    public int getPrivateGamerSlots() {
        return number("PrivateGamerSlots",
                value -> NativeNetworkRoutes.networkSessionGetPrivateGamerSlots(handle, value));
    }

    public void setPrivateGamerSlots(int value) {
        NativeGamerServices.check("NetworkSession.PrivateGamerSlots",
                NativeNetworkRoutes.networkSessionSetPrivateGamerSlots(handle, value));
    }

    public GamerCollection<NetworkGamer> getRemoteGamers() {
        return roster(ROSTER_REMOTE);
    }

    public NetworkSessionProperties getSessionProperties() {
        long[] properties = new long[1];
        NativeGamerServices.check("NetworkSession.SessionProperties",
                NativeNetworkRoutes.networkSessionCopySessionProperties(handle, properties));
        return new NetworkSessionProperties(properties[0]);
    }

    public NetworkSessionState getSessionState() {
        return NetworkSessionState.values()[number("SessionState",
                value -> NativeNetworkRoutes.networkSessionGetSessionState(handle, value))];
    }

    public NetworkSessionType getSessionType() {
        return NetworkSessionType.values()[number("SessionType",
                value -> NativeNetworkRoutes.networkSessionGetSessionType(handle, value))];
    }

    public Duration getSimulatedLatency() {
        long[] ticks = new long[1];
        NativeGamerServices.check("NetworkSession.SimulatedLatency",
                NativeNetworkRoutes.networkSessionGetSimulatedLatencyTicks(handle, ticks));
        return NativeGamerServices.duration(ticks[0]);
    }

    public void setSimulatedLatency(Duration value) {
        NativeGamerServices.check("NetworkSession.SimulatedLatency",
                NativeNetworkRoutes.networkSessionSetSimulatedLatencyTicks(handle,
                        NativeGamerServices.ticks(Objects.requireNonNull(value, "value"))));
    }

    public float getSimulatedPacketLoss() {
        float[] value = new float[1];
        NativeGamerServices.check("NetworkSession.SimulatedPacketLoss",
                NativeNetworkRoutes.networkSessionGetSimulatedPacketLoss(handle, value));
        return value[0];
    }

    public void setSimulatedPacketLoss(float value) {
        NativeGamerServices.check("NetworkSession.SimulatedPacketLoss",
                NativeNetworkRoutes.networkSessionSetSimulatedPacketLoss(handle, value));
    }

    private static void dispatch(long kind, long session, long first, long second, long flag) {
        if ((int) kind == KIND_INVITE_ACCEPTED) {
            InviteAcceptedEventArgs args = new InviteAcceptedEventArgs(
                    first == 0L ? null : (SignedInGamer) org.openeggbert.cna.internal
                            .GamerFactories.createSignedInGamer(first),
                    flag != 0L);
            for (EventHandler<InviteAcceptedEventArgs> listener : INVITE_ACCEPTED) {
                listener.invoke(null, args);
            }
            return;
        }
        NetworkSession owner = LIVE.get(session);
        if (owner == null) {
            // The session was disposed between CNA raising the event and this drain. XNA would
            // have no object to raise it on either.
            return;
        }
        owner.raise((int) kind, first, second, flag);
    }

    private void raise(int kind, long first, long second, long flag) {
        switch (kind) {
            case KIND_GAME_ENDED -> {
                GameEndedEventArgs args = new GameEndedEventArgs();
                for (EventHandler<GameEndedEventArgs> listener : gameEnded) {
                    listener.invoke(this, args);
                }
            }
            case KIND_GAME_STARTED -> {
                GameStartedEventArgs args = new GameStartedEventArgs();
                for (EventHandler<GameStartedEventArgs> listener : gameStarted) {
                    listener.invoke(this, args);
                }
            }
            case KIND_GAMER_JOINED -> {
                GamerJoinedEventArgs args = new GamerJoinedEventArgs(gamer(first));
                for (EventHandler<GamerJoinedEventArgs> listener : gamerJoined) {
                    listener.invoke(this, args);
                }
            }
            case KIND_GAMER_LEFT -> {
                GamerLeftEventArgs args = new GamerLeftEventArgs(gamer(first));
                for (EventHandler<GamerLeftEventArgs> listener : gamerLeft) {
                    listener.invoke(this, args);
                }
            }
            case KIND_HOST_CHANGED -> {
                HostChangedEventArgs args =
                        new HostChangedEventArgs(gamer(first), gamer(second));
                for (EventHandler<HostChangedEventArgs> listener : hostChanged) {
                    listener.invoke(this, args);
                }
            }
            case KIND_SESSION_ENDED -> {
                NetworkSessionEndedEventArgs args = new NetworkSessionEndedEventArgs(
                        NetworkSessionEndReason.values()[(int) flag]);
                for (EventHandler<NetworkSessionEndedEventArgs> listener : sessionEnded) {
                    listener.invoke(this, args);
                }
            }
            case KIND_WRITE_ARBITRATED -> writeLeaderboards(writeArbitrated, first, flag);
            case KIND_WRITE_TRUE_SKILL -> writeLeaderboards(writeTrueSkill, first, flag);
            case KIND_WRITE_UNARBITRATED -> writeLeaderboards(writeUnarbitrated, first, flag);
            default -> {
                // A kind this projection does not know is ignored rather than guessed at.
            }
        }
    }

    private void writeLeaderboards(
            List<EventHandler<WriteLeaderboardsEventArgs>> listeners, long first, long flag) {
        WriteLeaderboardsEventArgs args =
                new WriteLeaderboardsEventArgs(gamer(first), flag != 0L);
        for (EventHandler<WriteLeaderboardsEventArgs> listener : listeners) {
            listener.invoke(this, args);
        }
    }

    private static NetworkGamer gamer(long handle) {
        return handle == 0L ? null : new NetworkGamer(handle);
    }

    private GamerCollection<NetworkGamer> roster(int roster) {
        return new Roster(handle, roster);
    }

    private static final class Roster extends GamerCollection<NetworkGamer> {

        Roster(long session, int roster) {
            super(index -> new NetworkGamer(gamerAt(session, roster, index)),
                    () -> gamerCount(session, roster));
        }
    }

    private static final class LocalRoster extends GamerCollection<LocalNetworkGamer> {

        LocalRoster(long session) {
            super(index -> new LocalNetworkGamer(gamerAt(session, ROSTER_LOCAL, index)),
                    () -> gamerCount(session, ROSTER_LOCAL));
        }
    }

    private static long gamerAt(long session, int roster, int index) {
        long[] gamer = new long[1];
        NativeGamerServices.check("NetworkSession gamer roster",
                NativeNetworkRoutes.networkSessionGetGamer(session, roster, index, gamer));
        return gamer[0];
    }

    private static int gamerCount(long session, int roster) {
        int[] count = new int[1];
        NativeGamerServices.check("NetworkSession gamer roster",
                NativeNetworkRoutes.networkSessionGetGamerCount(session, roster, count));
        return count[0];
    }

    private static int kind(NetworkSessionType sessionType) {
        return Objects.requireNonNull(sessionType, "sessionType").ordinal();
    }

    private static long properties(NetworkSessionProperties properties) {
        return properties == null ? 0L : properties.handle();
    }

    private static long[] handles(Iterable<SignedInGamer> gamers) {
        Objects.requireNonNull(gamers, "localGamers");
        List<Long> values = new ArrayList<>();
        for (SignedInGamer gamer : gamers) {
            values.add(GamerHandles.of(Objects.requireNonNull(gamer, "localGamers")));
        }
        long[] handles = new long[values.size()];
        for (int index = 0; index < handles.length; index++) {
            handles[index] = values.get(index);
        }
        return handles;
    }

    private interface Flag {
        int read(boolean[] value);
    }

    private interface Number {
        int read(int[] value);
    }

    private static boolean flag(String property, Flag reader) {
        boolean[] value = new boolean[1];
        NativeGamerServices.check("NetworkSession." + property, reader.read(value));
        return value[0];
    }

    private static int number(String property, Number reader) {
        int[] value = new int[1];
        NativeGamerServices.check("NetworkSession." + property, reader.read(value));
        return value[0];
    }
}
