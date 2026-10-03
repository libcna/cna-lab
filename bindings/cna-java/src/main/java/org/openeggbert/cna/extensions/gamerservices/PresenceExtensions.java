package org.openeggbert.cna.extensions.gamerservices;

import Microsoft.Xna.Framework.GamerServices.SignedInGamer;
import org.openeggbert.cna.internal.FacadeFactory;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.util.Objects;

/**
 * A presence line in the title's own words.
 *
 * <p>A CNA extension. XNA's {@code GamerPresenceMode} is a closed enumeration of about thirty
 * phrases -- {@code AtMenu}, {@code InCombat}, {@code Level} and so on -- chosen because the Xbox
 * dashboard had to render them in every language it shipped. CNA has no such dashboard and no such
 * constraint, so a title can say what is actually happening.
 *
 * <p>It sits here rather than on {@link SignedInGamer} for the reason every extension does: adding
 * a member to the strict projection that the reference API has no counterpart for is what the
 * full-profile gate refuses. {@code SignedInGamer.getPresence()} still reads and writes the
 * canonical enumeration, and this writes beside it.
 */
public final class PresenceExtensions {

    private PresenceExtensions() {
    }

    /**
     * Sets the gamer's presence to a line of the title's own words.
     *
     * @param gamer the gamer whose presence to set
     * @param mode the text to show; empty clears it
     */
    public static void SetPresenceMode(SignedInGamer gamer, String mode) {
        NativeGamerServices.requireAvailable("PresenceExtensions.SetPresenceMode");
        Objects.requireNonNull(gamer, "gamer");
        Objects.requireNonNull(mode, "mode");
        NativeGamerServices.check("PresenceExtensions.SetPresenceMode",
                NativeGamerServicesRoutes.signedInGamerSetPresenceModeStringExt(
                        FacadeFactory.signedInGamerHandle(gamer),
                        NativeGamerServices.utf8(mode)));
    }
}
