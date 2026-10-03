package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Audio.Microphone;
import Microsoft.Xna.Framework.EventHandler;
import Microsoft.Xna.Framework.PlayerIndex;
import org.openeggbert.cna.internal.CompletedAsyncResult;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;
import System.AsyncCallback;
import System.IAsyncResult;

import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** A gamer signed in on this machine, in one of the four player slots. */
public final class SignedInGamer extends Gamer {

    private static final List<EventHandler<SignedInEventArgs>> SIGNED_IN =
            new CopyOnWriteArrayList<>();
    private static final List<EventHandler<SignedOutEventArgs>> SIGNED_OUT =
            new CopyOnWriteArrayList<>();

    SignedInGamer(long handle) {
        super(handle);
    }

    /** Returns the projected gamer behind one native handle, or null when there is none. */
    static SignedInGamer of(long handle) {
        return handle == 0L ? null : new SignedInGamer(handle);
    }

    public static void addSignedInListener(EventHandler<SignedInEventArgs> listener) {
        SIGNED_IN.add(Objects.requireNonNull(listener, "listener"));
    }

    public static void removeSignedInListener(EventHandler<SignedInEventArgs> listener) {
        SIGNED_IN.remove(Objects.requireNonNull(listener, "listener"));
    }

    public static void addSignedOutListener(EventHandler<SignedOutEventArgs> listener) {
        SIGNED_OUT.add(Objects.requireNonNull(listener, "listener"));
    }

    public static void removeSignedOutListener(EventHandler<SignedOutEventArgs> listener) {
        SIGNED_OUT.remove(Objects.requireNonNull(listener, "listener"));
    }

    public void AwardAchievement(String achievementKey) {
        NativeGamerServices.check("SignedInGamer.AwardAchievement",
                NativeGamerServicesRoutes.signedInGamerAwardAchievement(handle(),
                        NativeGamerServices.utf8(
                                Objects.requireNonNull(achievementKey, "achievementKey"))));
    }

    public IAsyncResult BeginAwardAchievement(
            String achievementKey, AsyncCallback callback, Object state) {
        return CompletedAsyncResult.begin(callback, state,
                () -> AwardAchievement(achievementKey));
    }

    public IAsyncResult BeginGetAchievements(AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, this::GetAchievements);
    }

    public void EndAwardAchievement(IAsyncResult result) {
        CompletedAsyncResult.endVoid(result);
    }

    public AchievementCollection EndGetAchievements(IAsyncResult result) {
        return CompletedAsyncResult.end(result, AchievementCollection.class);
    }

    public AchievementCollection GetAchievements() {
        long[] achievements = new long[1];
        NativeGamerServices.check("SignedInGamer.GetAchievements",
                NativeGamerServicesRoutes.signedInGamerGetAchievements(handle(), achievements));
        return new AchievementCollection(achievements[0]);
    }

    public FriendCollection GetFriends() {
        long[] friends = new long[1];
        NativeGamerServices.check("SignedInGamer.GetFriends",
                NativeGamerServicesRoutes.signedInGamerGetFriends(handle(), friends));
        return new FriendCollection(friends[0]);
    }

    public boolean IsFriend(Gamer gamer) {
        boolean[] friend = new boolean[1];
        NativeGamerServices.check("SignedInGamer.IsFriend",
                NativeGamerServicesRoutes.signedInGamerIsFriend(handle(),
                        Objects.requireNonNull(gamer, "gamer").handle(), friend));
        return friend[0];
    }

    /**
     * Reports whether this microphone is the gamer's headset.
     *
     * <p>XNA identifies the microphone object; CNA identifies it by its index in
     * {@code Microphone.All}, which is the same identity the Java projection uses.
     */
    public boolean IsHeadset(Microphone microphone) {
        int index = Microphone.getAll().indexOf(Objects.requireNonNull(microphone, "microphone"));
        if (index < 0) {
            return false;
        }
        boolean[] headset = new boolean[1];
        NativeGamerServices.check("SignedInGamer.IsHeadset",
                NativeGamerServicesRoutes.signedInGamerIsHeadset(handle(), index, headset));
        return headset[0];
    }

    public GameDefaults getGameDefaults() {
        long[] values = new long[20];
        NativeGamerServices.check("SignedInGamer.GameDefaults",
                NativeGamerServicesRoutes.signedInGamerGetGameDefaults(
                        handle(), new byte[3], values));
        return new GameDefaults(values);
    }

    public boolean getIsGuest() {
        boolean[] guest = new boolean[1];
        NativeGamerServices.check("SignedInGamer.IsGuest",
                NativeGamerServicesRoutes.signedInGamerGetIsGuest(handle(), guest));
        return guest[0];
    }

    public boolean getIsSignedInToLive() {
        boolean[] live = new boolean[1];
        NativeGamerServices.check("SignedInGamer.IsSignedInToLive",
                NativeGamerServicesRoutes.signedInGamerGetIsSignedInToLive(handle(), live));
        return live[0];
    }

    public int getPartySize() {
        int[] size = new int[1];
        NativeGamerServices.check("SignedInGamer.PartySize",
                NativeGamerServicesRoutes.signedInGamerGetPartySize(handle(), size));
        return size[0];
    }

    protected void setPartySize(int value) {
        NativeGamerServices.check("SignedInGamer.PartySize",
                NativeGamerServicesRoutes.signedInGamerSetPartySize(handle(), value));
    }

    public PlayerIndex getPlayerIndex() {
        int[] index = new int[1];
        NativeGamerServices.check("SignedInGamer.PlayerIndex",
                NativeGamerServicesRoutes.signedInGamerGetPlayerIndex(handle(), index));
        return PlayerIndex.values()[index[0]];
    }

    /**
     * Returns the gamer's presence.
     *
     * <p>The returned object writes through: setting its mode or value publishes the change,
     * which is what XNA's {@code SignedInGamer.Presence} does.
     */
    public GamerPresence getPresence() {
        long[] values = new long[2];
        NativeGamerServices.check("SignedInGamer.Presence",
                NativeGamerServicesRoutes.signedInGamerGetPresence(handle(), values));
        return GamerPresence.attached(this, values);
    }

    public GamerPrivileges getPrivileges() {
        long[] values = new long[7];
        NativeGamerServices.check("SignedInGamer.Privileges",
                NativeGamerServicesRoutes.signedInGamerGetPrivileges(
                        handle(), new byte[4], values));
        return new GamerPrivileges(values);
    }

    void publishPresence(GamerPresenceMode mode, int value) {
        NativeGamerServices.check("SignedInGamer.Presence",
                NativeGamerServicesRoutes.signedInGamerSetPresence(handle(),
                        new long[] {mode.ordinal(), value}));
    }

    static void raiseSignedIn(SignedInGamer gamer) {
        SignedInEventArgs args = new SignedInEventArgs(gamer);
        for (EventHandler<SignedInEventArgs> listener : SIGNED_IN) {
            listener.invoke(null, args);
        }
    }

    static void raiseSignedOut(SignedInGamer gamer) {
        SignedOutEventArgs args = new SignedOutEventArgs(gamer);
        for (EventHandler<SignedOutEventArgs> listener : SIGNED_OUT) {
            listener.invoke(null, args);
        }
    }
}
