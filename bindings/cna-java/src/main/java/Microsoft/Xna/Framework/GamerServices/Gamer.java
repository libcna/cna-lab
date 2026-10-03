package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.CompletedAsyncResult;
import org.openeggbert.cna.internal.GamerFactories;
import org.openeggbert.cna.internal.GamerHandles;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;
import System.AsyncCallback;
import System.IAsyncResult;

import java.util.Objects;

/**
 * One gamer known to the platform's gamer services.
 *
 * <p>{@code Gamer} is abstract in XNA and abstract here: a title receives a
 * {@link SignedInGamer}, a {@code FriendGamer} or a {@code NetworkGamer}, never a bare
 * {@code Gamer}. Every asynchronous member follows XNA's {@code Begin}/{@code End} pair; CNA
 * completes each one synchronously, so the returned result reports
 * {@code getCompletedSynchronously() == true} and {@code End} returns immediately.
 */
public abstract class Gamer {

    static {
        // Microsoft.Xna.Framework.Net builds signed-in gamers, which CLR reaches through
        // assembly-internal access. Installing the factory here is safe because every network
        // gamer derives from this type, so this initializer always runs first.
        GamerFactories.setSignedInGamer(SignedInGamer::new);
    }

    private final LeaderboardWriter leaderboardWriter = new LeaderboardWriter();

    Gamer(long handle) {
        GamerHandles.register(this, handle);
    }

    /**
     * Creates a gamer whose native handle its own constructor records.
     *
     * <p>CLR derives {@code Microsoft.Xna.Framework.Net}'s gamers from this type through
     * assembly-internal access, which Java has no equivalent for across packages. The
     * replacement is this protected constructor: the handle is recorded through the internal
     * handle table instead of crossing a protected signature, so no raw native handle appears
     * in the public or protected surface.
     */
    protected Gamer() {
    }

    long handle() {
        return GamerHandles.of(this);
    }

    public static IAsyncResult BeginGetFromGamertag(
            String gamertag, AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, () -> GetFromGamertag(gamertag));
    }

    public static IAsyncResult BeginGetPartnerToken(
            String audienceUri, AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, () -> GetPartnerToken(audienceUri));
    }

    public final IAsyncResult BeginGetProfile(AsyncCallback callback, Object asyncState) {
        return CompletedAsyncResult.begin(callback, asyncState, this::GetProfile);
    }

    public static Gamer EndGetFromGamertag(IAsyncResult result) {
        return CompletedAsyncResult.end(result, Gamer.class);
    }

    public static String EndGetPartnerToken(IAsyncResult result) {
        return CompletedAsyncResult.end(result, String.class);
    }

    public final GamerProfile EndGetProfile(IAsyncResult result) {
        return CompletedAsyncResult.end(result, GamerProfile.class);
    }

    /**
     * Returns the gamer with this gamertag, or {@code null} when no such gamer is known.
     *
     * <p>XNA answers {@code null} rather than failing for an unknown gamertag, and CNA reports
     * the same absence, so this never fabricates a gamer.
     */
    public static Gamer GetFromGamertag(String gamertag) {
        NativeGamerServices.requireAvailable("Gamer.GetFromGamertag");
        long[] found = new long[1];
        NativeGamerServices.check("Gamer.GetFromGamertag",
                NativeGamerServicesRoutes.gamerGetFromGamertag(
                        NativeGamerServices.utf8(Objects.requireNonNull(gamertag, "gamertag")),
                        found));
        return found[0] == 0L ? null : new RemoteGamer(found[0]);
    }

    public static String GetPartnerToken(String audienceUri) {
        NativeGamerServices.requireAvailable("Gamer.GetPartnerToken");
        byte[] uri = NativeGamerServices.utf8(Objects.requireNonNull(audienceUri, "audienceUri"));
        return NativeGamerServices.text("Gamer.GetPartnerToken",
                out -> NativeGamerServicesRoutes.gamerGetPartnerTokenSize(uri, out),
                (buffer, out) -> NativeGamerServicesRoutes.gamerCopyPartnerToken(uri, buffer, out));
    }

    public final GamerProfile GetProfile() {
        long[] profile = new long[1];
        NativeGamerServices.check("Gamer.GetProfile",
                NativeGamerServicesRoutes.gamerGetProfile(handle(), profile));
        return new GamerProfile(profile[0]);
    }

    /** Returns the gamertag, which is what XNA's {@code Gamer.ToString()} returns. */
    @Override
    public String toString() {
        return NativeGamerServices.text("Gamer.ToString",
                out -> NativeGamerServicesRoutes.gamerGetTextSize(handle(), out),
                (buffer, out) -> NativeGamerServicesRoutes.gamerCopyText(handle(), buffer, out));
    }

    public final String getDisplayName() {
        return NativeGamerServices.text("Gamer.DisplayName",
                out -> NativeGamerServicesRoutes.gamerGetDisplayNameSize(handle(), out),
                (buffer, out) -> NativeGamerServicesRoutes.gamerCopyDisplayName(
                        handle(), buffer, out));
    }

    protected final void setDisplayName(String value) {
        NativeGamerServices.check("Gamer.DisplayName",
                NativeGamerServicesRoutes.gamerSetDisplayName(
                        handle(), NativeGamerServices.utf8(value)));
    }

    public final String getGamertag() {
        return NativeGamerServices.text("Gamer.Gamertag",
                out -> NativeGamerServicesRoutes.gamerGetGamertagSize(handle(), out),
                (buffer, out) -> NativeGamerServicesRoutes.gamerCopyGamertag(handle(), buffer, out));
    }

    public final boolean getIsDisposed() {
        boolean[] disposed = new boolean[1];
        NativeGamerServices.check("Gamer.IsDisposed",
                NativeGamerServicesRoutes.gamerGetIsDisposed(handle(), disposed));
        return disposed[0];
    }

    public final LeaderboardWriter getLeaderboardWriter() {
        return leaderboardWriter;
    }

    public static SignedInGamerCollection getSignedInGamers() {
        return SignedInGamerCollection.current();
    }

    /**
     * Returns the title's own object for this gamer.
     *
     * <p>XNA stores an arbitrary reference here. CNA's tag is a 64-bit value it never
     * dereferences, so Java keeps the object itself and gives CNA a stable identifier for it;
     * a tag is therefore visible to this process only, exactly as XNA's is.
     */
    public final Object getTag() {
        long[] tag = new long[1];
        NativeGamerServices.check("Gamer.Tag",
                NativeGamerServicesRoutes.gamerGetTag(handle(), tag));
        return GamerTags.get(tag[0]);
    }

    public final void setTag(Object value) {
        long token = GamerTags.put(value);
        NativeGamerServices.check("Gamer.Tag",
                NativeGamerServicesRoutes.gamerSetTag(handle(), token));
    }

    /** A gamer reached by gamertag, which XNA models as an ordinary abstract {@code Gamer}. */
    static final class RemoteGamer extends Gamer {
        RemoteGamer(long handle) {
            super(handle);
        }
    }
}
