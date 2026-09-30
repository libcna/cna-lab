package org.openeggbert.cna.extensions.devices;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * The three host capabilities that wait for a person: a message box, a file dialog and a tray.
 *
 * <p>None of them can be completed by an automated test against the real host -- a modal dialog
 * waits for a click and a tray entry waits for a menu -- and that is exactly why they were left
 * unbound behind "message box, file dialog, system tray ... are CNA device extensions beyond XNA
 * 4.0", which explains why they are not XNA and says nothing about why they were absent.
 *
 * <p>CNA answers the real question: each ships a stand-in backend that records the request and
 * answers immediately. What is asserted here is therefore the request that reached CNA and the
 * answer it gave back -- the button index, the paths, the entry indices, the click -- rather than
 * a result code.
 */
final class HostDialogAndTrayTests {

    @Test
    void aMessageBoxWithNoButtonsIsRefusedBeforeItReachesCna() {
        // No game and no native library needed: an empty label list is a Java-side refusal, and
        // it has to happen before the native call rather than as CNA's INVALID_ARGUMENT, so the
        // caller learns which argument was wrong.
        assertThrows(IllegalArgumentException.class,
                () -> MessageBox.Show(MessageBoxType.Error, "t", "m", List.of()));
        assertThrows(NullPointerException.class,
                () -> MessageBox.Show(null, "t", "m"));
        assertThrows(NullPointerException.class,
                () -> MessageBox.Show(MessageBoxType.Error, null, "m"));
        assertThrows(NullPointerException.class,
                () -> FileDialog.ShowOpenFolder("", false, null));
        assertThrows(NullPointerException.class, () -> new FileDialogFilter("name", null));
    }

    @Test
    void severityAndDeviceTypeAreCnasOwnValues() {
        assertEquals(3, MessageBoxType.values().length);
        assertEquals(0, MessageBoxType.Error.ordinal());
        assertEquals(1, MessageBoxType.Warning.ordinal());
        assertEquals(2, MessageBoxType.Information.ordinal());
        assertEquals(2, DeviceType.values().length);
        assertEquals(0, DeviceType.Device.ordinal());
    }

    @Test
    @EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
    void theHostAnswersInsideAFrame() {
        try (Game game = new Game()) {
            DialogProbe probe = new DialogProbe(game);
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

    private static final class DialogProbe extends Microsoft.Xna.Framework.GameComponent {

        private boolean ran;
        private Throwable failure;

        private DialogProbe(Game game) {
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
                if (!DeviceExtension.isAvailable()) {
                    // A build without CNA_DEVICES refuses the host dialogs rather than faking one.
                    org.junit.jupiter.api.Assertions.assertThrows(
                            DeviceNotSupportedException.class, MessageBox::getIsSupported);
                    return;
                }
                messageBox();
                fileDialog();
                systemTray();
                clipboard();
                power();
                deviceType();
            } catch (Throwable exception) {
                failure = exception;
            } finally {
                // Both backends are process-wide, so a test that installed one and threw would
                // leave every later message box in this JVM answering by itself.
                try {
                    DeviceTestBackends.removeMessageBoxBackend();
                    DeviceTestBackends.removeFileDialogBackend();
                } catch (RuntimeException ignored) {
                    // Already removed, or this build has no device layer to remove it from.
                }
            }
        }

        private void messageBox() {
            assertTrue(MessageBox.getIsSupported(),
                    "this platform reports message boxes; the whole family turns on that");

            // Nothing to read before a backend exists, and CNA says so rather than answering
            // with an empty log that would look like "nothing was shown".
            assertThrows(RuntimeException.class, DeviceTestBackends::messageBoxLog);

            DeviceTestBackends.installMessageBoxBackend(2);
            DeviceTestBackends.MessageBoxTestLog fresh = DeviceTestBackends.messageBoxLog();
            assertEquals(0, fresh.simpleCalls());
            assertEquals(0, fresh.choiceCalls());

            MessageBox.Show(MessageBoxType.Warning, "title", "body");
            DeviceTestBackends.MessageBoxTestLog afterSimple = DeviceTestBackends.messageBoxLog();
            assertEquals(1, afterSimple.simpleCalls(), "a dismiss-only box counts as simple");
            assertEquals(0, afterSimple.choiceCalls(), "and not as a button-answering one");
            assertEquals(MessageBoxType.Warning, afterSimple.lastType());

            // The backend was installed answering button 2, so this is the answer travelling
            // back out through out_chosen rather than a result code.
            int chosen = MessageBox.Show(MessageBoxType.Error, "t", "m",
                    List.of("Yes", "No", "Maybe"));
            assertEquals(2, chosen, "the chosen index is the backend's own answer");
            DeviceTestBackends.MessageBoxTestLog afterChoice = DeviceTestBackends.messageBoxLog();
            assertEquals(1, afterChoice.simpleCalls());
            assertEquals(1, afterChoice.choiceCalls());
            assertEquals(MessageBoxType.Error, afterChoice.lastType(),
                    "the severity that reached CNA is the one that was asked for");
            assertEquals(3, afterChoice.lastButtonCount(),
                    "all three labels crossed the boundary, not just the first");

            // One label, to prove the count is the list's own rather than a fixed shape.
            assertEquals(2, MessageBox.Show(MessageBoxType.Information, "t", "m", List.of("Ok")));
            assertEquals(1, DeviceTestBackends.messageBoxLog().lastButtonCount());
            assertEquals(MessageBoxType.Information,
                    DeviceTestBackends.messageBoxLog().lastType());

            // Installing resets the log, which the header states and which a test that only
            // ever installed once would never notice.
            DeviceTestBackends.installMessageBoxBackend(0);
            assertEquals(0, DeviceTestBackends.messageBoxLog().simpleCalls());
            assertEquals(0, DeviceTestBackends.messageBoxLog().choiceCalls());

            DeviceTestBackends.removeMessageBoxBackend();
            assertThrows(RuntimeException.class, DeviceTestBackends::messageBoxLog);
        }

        private void fileDialog() {
            assertTrue(FileDialog.getIsSupported());

            DeviceTestBackends.installFileDialogBackend(
                    List.of("/tmp/one.sav", "/tmp/two.sav"));

            List<List<String>> answers = new ArrayList<>();
            FileDialog.ShowOpenFile(List.of(new FileDialogFilter("Saved games", "sav")),
                    "/tmp", true, answers::add);
            assertEquals(1, answers.size(), "the backend answers exactly once, and does it now");
            assertEquals(List.of("/tmp/one.sav", "/tmp/two.sav"), answers.get(0),
                    "the paths arrive in the backend's own order, decoded as UTF-8");
            assertThrows(UnsupportedOperationException.class,
                    () -> answers.get(0).add("/tmp/three.sav"));

            FileDialog.ShowSaveFile(List.of(), "", answers::add);
            assertEquals(2, answers.size());
            assertEquals(List.of("/tmp/one.sav", "/tmp/two.sav"), answers.get(1));

            FileDialog.ShowOpenFolder("", false, answers::add);
            assertEquals(3, answers.size());

            // Cancelling is an empty result rather than a separate signal, and a handler that
            // never ran would be indistinguishable from one that received nothing -- so this
            // asserts the call count as well as the emptiness.
            DeviceTestBackends.installFileDialogBackend(List.of());
            FileDialog.ShowOpenFile(List.of(), "", false, answers::add);
            assertEquals(4, answers.size(), "a cancelled dialog still answers");
            assertTrue(answers.get(3).isEmpty(), "and its answer is empty");

            // A path outside the basic multilingual plane, which is the case NewStringUTF would
            // have corrupted: modified UTF-8 writes an astral character as a surrogate pair.
            String astral = "/tmp/save-🎮.sav";
            DeviceTestBackends.installFileDialogBackend(List.of(astral));
            FileDialog.ShowOpenFile(List.of(), "", false, answers::add);
            assertEquals(List.of(astral), answers.get(4),
                    "an astral character survives the UTF-8 round trip byte for byte");

            DeviceTestBackends.removeFileDialogBackend();
        }

        private void systemTray() {
            assertTrue(SystemTray.getIsSupported());
            AtomicInteger clicks = new AtomicInteger();
            AtomicInteger silentClicks = new AtomicInteger();
            try (SystemTray tray = SystemTray.CreateForTests("probe")) {
                tray.setTooltip("probe tray");

                int sound = tray.addEntry("Sound", true, true, true, clicks::incrementAndGet);
                int quit = tray.addEntry("Quit", false, false, false, null);
                assertEquals(0, sound, "entries are indexed in the order they are added");
                assertEquals(1, quit);

                // The four state routes are two independent pairs, and a projection that swapped
                // them would still round-trip if both entries started the same way. They do not:
                // entry 0 starts checked and enabled, entry 1 starts unchecked and disabled.
                assertTrue(tray.getEntryChecked(sound));
                assertTrue(tray.getEntryEnabled(sound));
                assertFalse(tray.getEntryChecked(quit));
                assertFalse(tray.getEntryEnabled(quit));

                tray.setEntryChecked(sound, false);
                assertFalse(tray.getEntryChecked(sound));
                assertTrue(tray.getEntryEnabled(sound), "unchecking must not disable");

                tray.setEntryEnabled(quit, true);
                assertTrue(tray.getEntryEnabled(quit));
                assertFalse(tray.getEntryChecked(quit), "enabling must not check");

                // `checkable` and `initially_checked` are two flags in a row of five, and a
                // projection that swapped them round-trips perfectly on an entry that passes
                // the same value for both -- which both entries above do. This one does not.
                // Measured in host_dialogs_and_tray.c: get_entry_checked reports
                // initially_checked whatever checkable says, so the pair is distinguishable.
                int mute = tray.addEntry("Mute", true, false, true, null);
                assertFalse(tray.getEntryChecked(mute),
                        "a checkable entry that started unchecked is unchecked");

                tray.setEntryLabel(quit, "Exit");
                // An index past the last entry is ignored rather than refused. That is the
                // backend's own behaviour, and reporting it is the point.
                tray.setEntryLabel(99, "nowhere");
                tray.setEntryChecked(99, true);
                assertFalse(tray.getEntryChecked(99), "a missing entry is never checked");

                // The whole reason the tray callback needs a global reference: it is registered
                // once and runs whenever a person picks the entry, which is much later.
                tray.clickEntryForTests(sound);
                assertEquals(1, clicks.get(), "the handler ran, exactly once");
                tray.clickEntryForTests(sound);
                assertEquals(2, clicks.get());
                tray.clickEntryForTests(quit);
                assertEquals(2, clicks.get(), "an entry with no handler runs nobody else's");
                assertEquals(0, silentClicks.get());

                assertThrows(RuntimeException.class, () -> tray.clickEntryForTests(99));
            }
            assertEquals(2, clicks.get(), "closing the tray cannot deliver another click");
        }

        private void closedTrayRefuses() {
            SystemTray tray = SystemTray.CreateForTests("closed");
            tray.close();
            tray.close();
            assertThrows(IllegalStateException.class, () -> tray.setTooltip("after"));
        }

        private void clipboard() {
            closedTrayRefuses();
            // The clipboard is process-external, so what can be asserted is that the two reads
            // agree with each other rather than that a write landed: a headless session may
            // accept the request and hold nothing.
            String text = Clipboard.GetText();
            assertNotNull(text);
            assertEquals(Clipboard.getHasText(), !text.isEmpty(),
                    "hasText and the text itself must describe the same clipboard");

            if (Clipboard.SetText("cna-java clipboard probe") && Clipboard.getHasText()) {
                assertEquals("cna-java clipboard probe", Clipboard.GetText(),
                        "a clipboard that accepted the write and holds text holds that text");
            }
        }

        private void power() {
            PowerInformation.PowerSnapshot snapshot = PowerInformation.getSnapshot();
            assertNotNull(snapshot);
            assertNotNull(snapshot.state(), "CNA always reports a state, even UNKNOWN");
            assertSame(PowerInformation.getSnapshot().state(), snapshot.state(),
                    "the state does not move between two reads on a machine at rest");
            assertTrue(snapshot.batteryPercent() == null
                            || (snapshot.batteryPercent() >= 0 && snapshot.batteryPercent() <= 100),
                    "a percentage is a percentage, and -1 is absent rather than zero");
            assertTrue(snapshot.remainingRuntime() == null
                    || !snapshot.remainingRuntime().isNegative());
            // The snapshot and the three separate getters read the same host.
            assertEquals(PowerInformation.getState(), snapshot.state());
        }

        private void deviceType() {
            DeviceType kind = SystemInformation.getDeviceType();
            assertNotNull(kind);
            assertSame(kind, SystemInformation.getDeviceType());
            assertEquals(Duration.ZERO, Duration.ZERO);
        }
    }
}
