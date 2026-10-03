package org.openeggbert.cna.extensions.devices;

import Microsoft.Xna.Framework.Rectangle;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

/**
 * Display metrics XNA never exposed: the content scale, and the area a game may safely draw in.
 *
 * <p>A CNA extension. The safe area matters on a television and on a phone with a cutout, where
 * the whole back buffer is not all visible.
 */
public final class DisplayInformation {

    private DisplayInformation() {
    }

    /** Returns the display's content scale, where 1.0 is one back-buffer pixel per device pixel. */
    public static float getContentScale() {
        float[] scale = new float[1];
        DeviceExtension.check("DisplayInformation.getContentScale",
                NativeDeviceExtensionRoutes.displayInfoGetContentScaleExt(
                        DeviceExtension.game("DisplayInformation"), scale));
        return scale[0];
    }

    /** Returns the region of the back buffer the host guarantees is visible. */
    public static Rectangle getSafeArea() {
        long[] area = new long[4];
        DeviceExtension.check("DisplayInformation.getSafeArea",
                NativeDeviceExtensionRoutes.displayInfoGetSafeAreaExt(
                        DeviceExtension.game("DisplayInformation"), area));
        return new Rectangle((int) area[0], (int) area[1], (int) area[2], (int) area[3]);
    }
}
