package org.openeggbert.cna.internal;

import java.util.Map;
import java.util.Objects;
import java.util.WeakHashMap;

/**
 * Keeps the native handle behind each projected gamer.
 *
 * <p>{@code Microsoft.Xna.Framework.Net} derives its gamer types from
 * {@code Microsoft.Xna.Framework.GamerServices.Gamer}, which CLR allows through
 * assembly-internal access and Java does not allow across packages. Storing the handle here
 * rather than in a protected constructor parameter is what keeps a raw native handle out of
 * every public and protected signature while still letting both namespaces reach it.
 *
 * <p>The map holds its keys weakly, so a gamer the game drops is collectable; the handle is
 * owned by CNA's gamer services, not by this table.
 *
 * <p>This class is not application API.
 */
public final class GamerHandles {

    private static final Map<Object, Long> HANDLES = new WeakHashMap<>();

    private GamerHandles() {
    }

    /** Records the native handle behind one projected gamer. */
    public static void register(Object gamer, long handle) {
        synchronized (HANDLES) {
            HANDLES.put(Objects.requireNonNull(gamer, "gamer"), handle);
        }
    }

    /** Returns the native handle behind one projected gamer, or zero when it has none. */
    public static long of(Object gamer) {
        synchronized (HANDLES) {
            Long handle = HANDLES.get(Objects.requireNonNull(gamer, "gamer"));
            return handle == null ? 0L : handle;
        }
    }
}
