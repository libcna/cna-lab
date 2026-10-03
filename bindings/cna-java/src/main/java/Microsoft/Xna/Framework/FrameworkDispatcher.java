package Microsoft.Xna.Framework;

import org.openeggbert.cna.internal.NativeBindings;

/** XNA's framework-wide asynchronous subsystem pump. */
public final class FrameworkDispatcher {

    private FrameworkDispatcher() {
    }

    /** Pumps CNA audio, media, microphone, touch, and future framework services. */
    public static void Update() {
        NativeBindings.updateFrameworkDispatcher();
    }
}
