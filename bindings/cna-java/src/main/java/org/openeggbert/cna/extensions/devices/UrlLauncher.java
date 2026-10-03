package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.net.URI;
import java.util.Objects;

/**
 * Opens a URL in the host's browser.
 *
 * <p>A CNA extension: XNA 4.0 has {@code Guide.ShowMarketplace} and nothing else that leaves the
 * game.
 */
public final class UrlLauncher {

    private UrlLauncher() {
    }

    /**
     * Asks the host to open a URL.
     *
     * @return whether the host accepted the request. It reports that the request was made, not
     *     that a browser appeared; only the host knows the second.
     */
    public static boolean Open(URI url) {
        Objects.requireNonNull(url, "url");
        boolean[] opened = new boolean[1];
        DeviceExtension.check("UrlLauncher.Open",
                NativeDeviceExtensionRoutes.urlLauncherOpenExt(
                        DeviceExtension.game("UrlLauncher"),
                        NativeGamerServices.utf8(url.toString()), opened));
        return opened[0];
    }
}
