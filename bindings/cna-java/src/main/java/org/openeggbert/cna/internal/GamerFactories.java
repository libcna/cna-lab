package org.openeggbert.cna.internal;

import java.util.Objects;
import java.util.function.LongFunction;

/**
 * Lets {@code Microsoft.Xna.Framework.Net} build the gamer types that
 * {@code Microsoft.Xna.Framework.GamerServices} declares.
 *
 * <p>CLR allows that directly through assembly-internal access. Java does not allow it across
 * packages, and the alternative -- widening a constructor XNA does not declare -- would put a
 * member in the public surface that the reference API has no counterpart for. The GamerServices
 * package installs its factory when {@code Gamer} initializes, which is guaranteed to happen
 * before any network gamer exists, because every network gamer derives from it.
 *
 * <p>This class is not application API.
 */
public final class GamerFactories {

    private static volatile LongFunction<?> signedInGamer;

    private GamerFactories() {
    }

    /** Installs the factory. Called from the GamerServices package during class initialization. */
    public static void setSignedInGamer(LongFunction<?> factory) {
        signedInGamer = Objects.requireNonNull(factory, "factory");
    }

    /** Builds the projected signed-in gamer behind one native handle. */
    public static Object createSignedInGamer(long handle) {
        LongFunction<?> factory = signedInGamer;
        if (factory == null) {
            throw new IllegalStateException(
                    "The GamerServices package has not installed its gamer factories yet");
        }
        return factory.apply(handle);
    }
}
