package org.openeggbert.cna.extensions.devices;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Rectangle;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.net.URI;
import java.time.Duration;
import java.util.List;
import java.util.Locale;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The host device extensions, against the live runtime.
 *
 * <p>Every capability here reaches the host through the platform the game created, so the probe
 * runs inside a frame. What is asserted is what CNA actually answers on a headless platform, not
 * what a desktop with a window would.
 */
final class DeviceExtensionTests {

    @Test
    void availabilityAnswersWithNoNativeBackend() {
        assertFalse(DeviceExtension.isAvailable() && System.getenv("CNA_NATIVE_LIBRARY") == null,
                "availability must be false when no native backend is loaded");
    }

    @Test
    void powerStateNamesAreCnasOwn() {
        assertEquals(6, PowerState.values().length);
        assertEquals(0, PowerState.Error.ordinal());
        assertEquals(1, PowerState.Unknown.ordinal());
        assertEquals(5, PowerState.Charged.ordinal());
    }

    @Test
    void everyDeviceCapabilityNeedsARunningGame() {
        // CNA reaches the host through the platform the game created, so there is nothing to ask
        // before a game exists. That is CNA's shape, and the projection reports it rather than
        // answering with a guess.
        assertThrows(RuntimeException.class, SystemInformation::getLogicalCpuCoreCount);
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void theHostAnswersForItselfInsideAFrame() {
        try (Game game = new Game()) {
            DeviceProbe probe = new DeviceProbe(game);
            game.getComponents().add(probe);
            game.RunOneFrame();
            if (probe.failure != null) {
                if (probe.failure instanceof RuntimeException runtime) {
                    throw runtime;
                }
                throw new IllegalStateException(probe.failure);
            }
            assertTrue(probe.ran, "the probe must have run");
        }
    }

    private static final class DeviceProbe extends Microsoft.Xna.Framework.GameComponent {

        private boolean ran;
        private Throwable failure;

        private DeviceProbe(Game game) {
            super(game);
        }

        @Override
        public void Update(GameTime gameTime) {
            super.Update(gameTime);
            if (ran) {
                return;
            }
            ran = true;
            try {
                probe();
            } catch (Throwable exception) {
                failure = exception;
            }
        }

        private void probe() {
            if (!DeviceExtension.isAvailable()) {
                // A build without CNA_DEVICES: every host query is refused rather than guessed.
                assertThrows(DeviceNotSupportedException.class,
                        SystemInformation::getLogicalCpuCoreCount);
                return;
            }

            // The headless platform does not report every host fact. What the projection can
            // guarantee is that each query answers, answers the same way twice, and never
            // reports a negative quantity; a zero is the host saying it does not know.
            int cores = SystemInformation.getLogicalCpuCoreCount();
            assertTrue(cores >= 0, "logical core count must not be negative");
            assertEquals(cores, SystemInformation.getLogicalCpuCoreCount());
            int megabytes = SystemInformation.getSystemRamMegabytes();
            assertTrue(megabytes >= 0, "system memory must not be negative");
            assertEquals(megabytes, SystemInformation.getSystemRamMegabytes());

            assertNotNull(PowerInformation.getState());
            // A percentage or a remaining time the host will not report is absent, not zero.
            Integer percent = PowerInformation.getBatteryPercent();
            assertTrue(percent == null || (percent >= 0 && percent <= 100));
            Duration remaining = PowerInformation.getRemainingRuntime();
            assertTrue(remaining == null || !remaining.isNegative());

            assertTrue(DisplayInformation.getContentScale() >= 0.0f,
                    "content scale must not be negative");
            Rectangle safeArea = DisplayInformation.getSafeArea();
            assertNotNull(safeArea);
            assertTrue(safeArea.Width >= 0 && safeArea.Height >= 0);

            List<Locale> locales = HostLocales.getPreferred();
            assertNotNull(locales);
            for (Locale locale : locales) {
                assertFalse(locale.getLanguage().isEmpty());
            }
            assertThrows(UnsupportedOperationException.class, () -> locales.add(Locale.ROOT));

            // A request the host does not honour reports that, rather than throwing or lying.
            assertNotNull(VibrateController.getDeviceName());
            boolean vibration = VibrateController.getIsSupported();
            if (vibration) {
                VibrateController.Start(Duration.ofMillis(1));
                VibrateController.Stop();
            }
            assertEquals(vibration, VibrateController.getIsSupported());

            Clipboard.SetText("cna-java device probe");

            // UrlLauncher is exercised only through its refusals, never with a real URL, and
            // devices.h is explicit about why: the route "hands control to another application,
            // so nothing in this ABI's own test suite calls it with a real URL". An earlier
            // version of this test called it with an https URL under a reserved .invalid host,
            // on the reasoning that a name RFC 6761 guarantees will never resolve is harmless.
            // The name is harmless; the call is not. CNA passes it to the host, which on a
            // desktop session is xdg-open, so running the suite opened a browser window on
            // whoever's machine had DISPLAY set. A test must not reach outside the process.
            assertThrows(org.openeggbert.cna.internal.CnaNativeException.class,
                    () -> UrlLauncher.Open(URI.create("")),
                    "an empty URL is refused by CNA before anything leaves the process");

            assertThrows(NullPointerException.class, () -> Clipboard.SetText(null));
            assertThrows(NullPointerException.class, () -> UrlLauncher.Open(null));
            assertThrows(NullPointerException.class, () -> VibrateController.Start(null));
        }
    }
}
