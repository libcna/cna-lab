package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import Microsoft.Xna.Framework.ServiceProvider;
import Microsoft.Xna.Framework.WindowHandle;
import org.openeggbert.cna.internal.GamerEventPump;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * The process-wide pump behind every gamer-services facility.
 *
 * <p>A title that does not use {@link GamerServicesComponent} calls {@code Initialize} once and
 * {@code Update} every frame; the component does exactly that on the title's behalf. Nothing
 * else in this namespace works before {@code Initialize} has run -- XNA raises
 * {@link GamerServicesNotAvailableException} for that, and so does this projection.
 *
 * <p>The type is {@code abstract sealed} in XNA, the CLR spelling of a static class, so it is
 * a final class with a private constructor here and cannot be instantiated or extended.
 */
public final class GamerServicesDispatcher {

    /** The event kinds CNA raises outside a session, matching the JNI adapter's numbering. */
    private static final int KIND_SIGNED_IN = 0;
    private static final int KIND_SIGNED_OUT = 1;
    private static final int KIND_INSTALLING_TITLE_UPDATE = 2;

    private static final List<EventHandler<EventArgs>> INSTALLING_TITLE_UPDATE =
            new CopyOnWriteArrayList<>();

    static {
        // The listeners live in this package, so the pump's handler does too.
        GamerEventPump.setGamerHandler(GamerServicesDispatcher::dispatch);
    }

    private GamerServicesDispatcher() {
    }

    public static void addInstallingTitleUpdateListener(EventHandler<EventArgs> listener) {
        INSTALLING_TITLE_UPDATE.add(Objects.requireNonNull(listener, "listener"));
    }

    public static void removeInstallingTitleUpdateListener(EventHandler<EventArgs> listener) {
        INSTALLING_TITLE_UPDATE.remove(Objects.requireNonNull(listener, "listener"));
    }

    /**
     * Starts gamer services for this process.
     *
     * <p>XNA takes the game's service provider so the dispatcher can reach the graphics device
     * it draws the Guide over. CNA's dispatcher takes the live game directly, so the provider
     * is required to be present but its services are resolved natively.
     */
    public static void Initialize(ServiceProvider serviceProvider) {
        Objects.requireNonNull(serviceProvider, "serviceProvider");
        NativeBindings.initializeGamerServicesDispatcher();
        GamerEventPump.ensureSubscribed();
    }

    /**
     * Pumps gamer services and raises whatever CNA queued.
     *
     * <p>Draining here is what puts a sign-in or a title update on the game thread during
     * {@code Update}, which is where XNA raises them.
     */
    public static void Update() {
        NativeBindings.updateGamerServicesDispatcher();
        GamerEventPump.drain();
    }

    public static boolean getIsInitialized() {
        if (!NativeBindings.isAvailable()) {
            return false;
        }
        boolean[] initialized = new boolean[1];
        NativeGamerServices.check("GamerServicesDispatcher.IsInitialized",
                NativeGamerServicesRoutes.gamerServicesDispatcherGetIsInitialized(initialized));
        return initialized[0];
    }

    public static WindowHandle getWindowHandle() {
        long[] handle = new long[1];
        NativeGamerServices.check("GamerServicesDispatcher.WindowHandle",
                NativeGamerServicesRoutes.gamerServicesDispatcherGetWindowHandle(handle));
        return NativeBindings.windowHandle(handle[0]);
    }

    public static void setWindowHandle(WindowHandle value) {
        NativeBindings.setGamerServicesWindowHandle(Objects.requireNonNull(value, "value"));
    }

    static void raiseInstallingTitleUpdate() {
        for (EventHandler<EventArgs> listener : INSTALLING_TITLE_UPDATE) {
            listener.invoke(null, EventArgs.Empty);
        }
    }

    private static void dispatch(long kind, long session, long first, long second, long flag) {
        switch ((int) kind) {
            case KIND_SIGNED_IN -> SignedInGamer.raiseSignedIn(SignedInGamer.of(first));
            case KIND_SIGNED_OUT -> SignedInGamer.raiseSignedOut(SignedInGamer.of(first));
            case KIND_INSTALLING_TITLE_UPDATE -> raiseInstallingTitleUpdate();
            default -> {
                // A kind this package does not own belongs to the session handler.
            }
        }
    }
}
