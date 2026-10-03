package org.openeggbert.cna.extensions.devices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

/**
 * The locales the player has chosen, in their own order of preference.
 *
 * <p>A CNA extension: XNA 4.0 exposes the CLR's current culture and nothing about the player's
 * ordered preferences, which is what a game needs to pick the best of the languages it ships.
 */
public final class HostLocales {

    private HostLocales() {
    }

    /**
     * Returns the preferred locales, most preferred first.
     *
     * <p>A locale CNA reports with a language but no country becomes a language-only
     * {@link Locale}; nothing is invented to fill the country in.
     */
    public static List<Locale> getPreferred() {
        long game = DeviceExtension.game("HostLocales");
        long[] count = new long[1];
        DeviceExtension.check("HostLocales.getPreferred",
                NativeDeviceExtensionRoutes.localeGetPreferredCountExt(game, count));
        List<Locale> locales = new ArrayList<>(Math.toIntExact(count[0]));
        for (long index = 0; index < count[0]; index++) {
            long position = index;
            String language = NativeGamerServices.text("HostLocales.getPreferred",
                    out -> NativeDeviceExtensionRoutes.localeGetLanguageSizeAtExt(
                            game, position, out),
                    (buffer, out) -> NativeDeviceExtensionRoutes.localeCopyLanguageAtExt(
                            game, position, buffer, out));
            String country = NativeGamerServices.text("HostLocales.getPreferred",
                    out -> NativeDeviceExtensionRoutes.localeGetCountrySizeAtExt(
                            game, position, out),
                    (buffer, out) -> NativeDeviceExtensionRoutes.localeCopyCountryAtExt(
                            game, position, buffer, out));
            locales.add(country.isEmpty() ? new Locale(language) : new Locale(language, country));
        }
        return Collections.unmodifiableList(locales);
    }
}
