package Microsoft.Xna.Framework.Storage;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.PlayerIndex;
import System.IAsyncResult;
import System.IO.FileAccess;
import System.IO.FileMode;
import System.IO.FileShare;
import System.IO.SeekOrigin;
import System.IO.Stream;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.openeggbert.cna.internal.CnaNativeException;
import org.openeggbert.cna.internal.NativeStorage;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.*;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class StorageNativeIntegrationTests {
    private static final String CONTAINER_PREFIX = "cna-java-storage-integration-";

    @Test
    void FakeAsyncSelectorsAndContainerOpenMatchXnaEndSemantics() {
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            Object selectorState = new Object();
            AtomicReference<IAsyncResult> selectorCallback = new AtomicReference<>();
            IAsyncResult selector = StorageDevice.BeginShowSelector(
                    result -> selectorCallback.set(result), selectorState);
            assertSame(selector, selectorCallback.get());
            assertSame(selectorState, selector.getAsyncState());
            assertTrue(selector.getCompletedSynchronously());
            assertTrue(selector.getIsCompleted());

            StorageDevice device = StorageDevice.EndShowSelector(selector);
            assertThrows(IllegalStateException.class,
                    () -> StorageDevice.EndShowSelector(selector));
            assertTrue(device.getIsConnected());
            assertTrue(device.getFreeSpace() >= 0L);
            assertTrue(device.getTotalSpace() >= device.getFreeSpace());

            IAsyncResult negativeDirectoryCount = StorageDevice.BeginShowSelector(
                    0, -1, null, null);
            assertNotNull(StorageDevice.EndShowSelector(negativeDirectoryCount),
                    "XNA does not reject directoryCount at Begin");
            IAsyncResult playerSelector = StorageDevice.BeginShowSelector(
                    PlayerIndex.Four, 0, 1, null, null);
            assertNotNull(StorageDevice.EndShowSelector(playerSelector));
            IAsyncResult playerOnlySelector = StorageDevice.BeginShowSelector(
                    PlayerIndex.One, null, null);
            assertNotNull(StorageDevice.EndShowSelector(playerOnlySelector));

            IAsyncResult foreign = foreignResult();
            assertThrows(NullPointerException.class,
                    () -> StorageDevice.EndShowSelector(null));
            assertThrows(NullPointerException.class,
                    () -> StorageDevice.EndShowSelector(foreign));

            String name = CONTAINER_PREFIX + "async";
            deleteIfPresent(device, name);
            Object openState = new Object();
            AtomicReference<IAsyncResult> openCallback = new AtomicReference<>();
            IAsyncResult opening = device.BeginOpenContainer(
                    name, result -> openCallback.set(result), openState);
            assertSame(opening, openCallback.get());
            assertSame(openState, opening.getAsyncState());
            assertTrue(opening.getCompletedSynchronously());
            assertTrue(opening.getIsCompleted());
            try (StorageContainer container = device.EndOpenContainer(opening)) {
                assertEquals(name, container.getDisplayName());
                assertSame(device, container.getStorageDevice());
            }
            assertThrows(IllegalStateException.class,
                    () -> device.EndOpenContainer(opening));
            assertThrows(NullPointerException.class,
                    () -> device.EndOpenContainer(foreign));

            IAsyncResult nullName = device.BeginOpenContainer(null, null, null);
            assertThrows(NullPointerException.class,
                    () -> device.EndOpenContainer(nullName));
            IAsyncResult emptyName = device.BeginOpenContainer("", null, null);
            assertThrows(NullPointerException.class,
                    () -> device.EndOpenContainer(emptyName));
            deleteIfPresent(device, name);
        }
    }

    @Test
    void ContainerCrudStreamsPatternsAndContainmentAreNativeBacked() throws IOException {
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            StorageDevice device = selectDevice();
            String name = CONTAINER_PREFIX + "crud";
            deleteIfPresent(device, name);
            try (StorageContainer container = open(device, name)) {
                assertEquals(name, container.getDisplayName());
                assertSame(device, container.getStorageDevice());
                assertFalse(container.getIsDisposed());

                container.CreateDirectory("b");
                container.CreateDirectory("a");
                container.CreateDirectory("nested/./child");
                assertTrue(container.DirectoryExists("a"));
                assertFalse(container.DirectoryExists("missing"));
                assertTrue(container.DirectoryExists("nested/child"));
                assertNames(container.GetDirectoryNames(), "a", "b", "nested");
                assertArrayEquals(sorted(container.GetDirectoryNames()),
                        sorted(container.GetDirectoryNames(null)));
                assertNames(container.GetDirectoryNames("a*"), "a");
                assertThrows(IllegalArgumentException.class,
                        () -> container.CreateDirectory("../escape"));
                assertThrows(IllegalArgumentException.class,
                        () -> container.CreateDirectory("C:\\escape"));
                containmentIsTheRuntimesGuaranteeToo(container);

                byte[] payload = "CNA storage".getBytes(StandardCharsets.UTF_8);
                try (Stream stream = container.CreateFile("save.bin")) {
                    assertTrue(stream.getCanWrite());
                    assertTrue(stream.getCanSeek());
                    stream.write(payload);
                    stream.flush();
                    assertEquals(payload.length, stream.getLength());
                    assertEquals(payload.length, stream.getPosition());
                    assertEquals(0L, stream.seek(0L, SeekOrigin.Begin));
                }

                assertTrue(container.FileExists("save.bin"));
                assertFalse(container.FileExists("missing.bin"));
                assertNames(container.GetFileNames(), "save.bin");
                assertNames(container.GetFileNames("*.bin"), "save.bin");
                assertArrayEquals(sorted(container.GetFileNames()),
                        sorted(container.GetFileNames(null)));

                try (Stream stream = container.OpenFile(
                        "save.bin", FileMode.Open, FileAccess.Read, FileShare.Read)) {
                    assertTrue(stream.getCanRead());
                    assertFalse(stream.getCanWrite());
                    byte[] actual = new byte[payload.length];
                    assertEquals(payload.length, stream.read(actual, 0, actual.length));
                    assertArrayEquals(payload, actual);
                    assertEquals(-1, stream.read());
                }

                try (Stream stream = container.OpenFile(
                        "save.bin", FileMode.Open, FileAccess.ReadWrite)) {
                    stream.setLength(3L);
                    assertEquals(3L, stream.getLength());
                    stream.setPosition(0L);
                    assertEquals('C', stream.read());
                }

                container.DeleteFile("save.bin");
                container.DeleteDirectory("nested/child");
                container.DeleteDirectory("nested");
                container.DeleteDirectory("a");
                container.DeleteDirectory("b");
                assertFalse(container.FileExists("save.bin"));
                assertFalse(container.DirectoryExists("a"));
            }
            deleteIfPresent(device, name);
        }
    }

    @Test
    void DisposingIsNativeOneShotReentrantMutableAndExceptionContained() {
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            StorageDevice device = selectDevice();
            String name = CONTAINER_PREFIX + "events";
            deleteIfPresent(device, name);
            StorageContainer container = open(device, name);
            AtomicInteger duplicateCalls = new AtomicInteger();
            AtomicInteger selfCalls = new AtomicInteger();
            AtomicInteger closingCalls = new AtomicInteger();
            EventHandler<EventArgs> duplicate = (sender, args) -> {
                assertSame(container, sender);
                assertSame(EventArgs.Empty, args);
                duplicateCalls.incrementAndGet();
            };
            AtomicReference<EventHandler<EventArgs>> selfRemoving = new AtomicReference<>();
            selfRemoving.set((sender, args) -> {
                selfCalls.incrementAndGet();
                container.removeDisposingListener(selfRemoving.get());
            });
            container.addDisposingListener(duplicate);
            container.addDisposingListener(duplicate);
            container.addDisposingListener(selfRemoving.get());
            container.addDisposingListener((sender, args) -> {
                closingCalls.incrementAndGet();
                container.close();
            });
            container.close();
            container.close();
            assertTrue(container.getIsDisposed());
            assertTrue(container.nativeDisposingWasObserved(),
                    "the qualified ABI 0.7 route must emit its native Disposing callback");
            assertEquals(2, duplicateCalls.get());
            assertEquals(1, selfCalls.get());
            assertEquals(1, closingCalls.get());

            String throwingName = CONTAINER_PREFIX + "events-throw";
            deleteIfPresent(device, throwingName);
            StorageContainer throwing = open(device, throwingName);
            AtomicInteger laterCalls = new AtomicInteger();
            throwing.addDisposingListener((sender, args) -> {
                throw new IllegalStateException("storage-disposing-handler");
            });
            throwing.addDisposingListener((sender, args) -> laterCalls.incrementAndGet());
            IllegalStateException failure = assertThrows(
                    IllegalStateException.class, throwing::close);
            assertEquals("storage-disposing-handler", failure.getMessage());
            assertEquals(1, laterCalls.get());
            assertTrue(throwing.getIsDisposed());
            assertTrue(throwing.nativeDisposingWasObserved());
            throwing.close();

            deleteIfPresent(device, name);
            deleteIfPresent(device, throwingName);
        }
    }

    @Test
    void DeviceChangedUsesTheFrameworkDispatcherOwnerThreadPump() {
        AtomicInteger calls = new AtomicInteger();
        EventHandler<EventArgs> listener = (sender, args) -> {
            assertNull(sender);
            assertSame(EventArgs.Empty, args);
            calls.incrementAndGet();
        };
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            StorageDevice.addDeviceChangedListener(listener);
            StorageDevice.addDeviceChangedListener(listener);
            runOnThread(NativeStorage::enqueueDeviceChangedForQualification,
                    "cna-java-storage-device-event");
            assertEquals(0, calls.get(), "callbacks must enqueue rather than run user code");
            game.RunOneFrame();
            assertEquals(2, calls.get());
            StorageDevice.removeDeviceChangedListener(listener);
            NativeStorage.enqueueDeviceChangedForQualification();
            game.RunOneFrame();
            assertEquals(3, calls.get(), "one duplicate registration remains");
            StorageDevice.removeDeviceChangedListener(listener);

            AtomicInteger afterThrow = new AtomicInteger();
            EventHandler<EventArgs> throwing = (sender, args) -> {
                throw new IllegalStateException("storage-device-event-failure");
            };
            EventHandler<EventArgs> later = (sender, args) -> afterThrow.incrementAndGet();
            StorageDevice.addDeviceChangedListener(throwing);
            StorageDevice.addDeviceChangedListener(later);
            NativeStorage.enqueueDeviceChangedForQualification();
            CnaNativeException failure = assertThrows(
                    CnaNativeException.class, game::RunOneFrame);
            assertEquals(9, failure.getResult());
            assertTrue(failure.getMessage().contains("storage-device-event-failure"));
            assertEquals(1, afterThrow.get(),
                    "one throwing handler must not suppress later handlers");
            StorageDevice.removeDeviceChangedListener(throwing);
            StorageDevice.removeDeviceChangedListener(later);
        }

        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            StorageDevice.addDeviceChangedListener(listener);
            NativeStorage.enqueueDeviceChangedForQualification();
        }
        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            NativeStorage.dispatchPendingEvents();
            assertEquals(3, calls.get(), "Game shutdown must discard queued device work");
        } finally {
            StorageDevice.removeDeviceChangedListener(listener);
        }
    }

    @Test
    void ParentChildShutdownAndWrongThreadRefusalRemainRetryable() throws IOException {
        Game game = preparedGame();
        StorageDevice device = selectDevice();
        String name = CONTAINER_PREFIX + "ownership";
        deleteIfPresent(device, name);
        StorageContainer container = open(device, name);
        Stream child = container.CreateFile("owned.bin");
        child.write(7);

        AtomicReference<Throwable> refused = new AtomicReference<>();
        runOnThread(() -> {
            try { container.close(); }
            catch (Throwable failure) { refused.set(failure); }
        }, "cna-java-storage-wrong-thread-release");
        CnaNativeException wrongThread = assertInstanceOf(
                CnaNativeException.class, refused.get());
        assertEquals(8, wrongThread.getResult());
        assertFalse(container.getIsDisposed());
        assertTrue(child.getCanWrite());

        game.close();
        assertTrue(container.getIsDisposed());
        assertThrows(IllegalStateException.class, child::getCanWrite);
        assertThrows(StorageDeviceNotConnectedException.class, device::getIsConnected);

        try (Game cleanupGame = preparedGame()) {
            assertNotNull(cleanupGame.getWindow());
            deleteIfPresent(selectDevice(), name);
        }
    }

    @Test
    void StorageOwnershipStressSurvivesFortyCompleteGameLifetimes() throws IOException {
        final int cycles = 40;
        for (int iteration = 0; iteration < cycles; iteration++) {
            try (Game game = preparedGame()) {
                assertNotNull(game.getWindow());
                StorageDevice device = selectDevice();
                String name = CONTAINER_PREFIX + "stress-" + iteration;
                deleteIfPresent(device, name);
                try (StorageContainer container = open(device, name)) {
                    try (Stream stream = container.CreateFile("cycle.bin")) {
                        stream.write(iteration);
                    }
                    assertTrue(container.FileExists("cycle.bin"));
                    container.DeleteFile("cycle.bin");
                }
                deleteIfPresent(device, name);
            }
        }

        try (Game game = preparedGame()) {
            assertNotNull(game.getWindow());
            StorageDevice device = selectDevice();
            assertTrue(device.getIsConnected());
            assertTrue(device.getTotalSpace() >= device.getFreeSpace());
        }
    }

    private static Game preparedGame() {
        Game game = new Game();
        new GraphicsDeviceManager(game);
        game.RunOneFrame();
        return game;
    }

    private static StorageDevice selectDevice() {
        return StorageDevice.EndShowSelector(
                StorageDevice.BeginShowSelector(null, null));
    }

    private static StorageContainer open(StorageDevice device, String name) {
        return device.EndOpenContainer(device.BeginOpenContainer(name, null, null));
    }

    private static void deleteIfPresent(StorageDevice device, String name) {
        device.DeleteContainer(name);
    }

    /**
     * Proves the containment guarantee belongs to CNA, not only to this binding.
     *
     * <p>The projection refuses an escaping path before the JNI call, which is why the
     * assertions above pass whatever the runtime does -- so they cannot tell whether CNA would
     * have refused it too. That mattered: CNA once accepted parent traversal, which
     * JAVA-UPSTREAM-001 recorded. This asks the runtime directly, past the Java guard, and
     * requires the refusal to be its own.
     */
    private static void containmentIsTheRuntimesGuaranteeToo(StorageContainer container) {
        long handle = NativeStorage.containerHandleForQualification(container);
        for (String escaping : new String[] {
            "../escaped.sav", "a/../../escaped.sav", "/absolute.sav"}) {
            assertThrows(RuntimeException.class,
                    () -> NativeStorage.pathQuery(handle, false, escaping),
                    "CNA itself must refuse '" + escaping + "'");
        }
        // And an ordinary name is still accepted, so the refusal is containment rather than a
        // route that stopped working.
        assertFalse(NativeStorage.pathQuery(handle, false, "not-created.sav"));
    }

    private static void assertNames(String[] actual, String... expected) {
        assertArrayEquals(sorted(expected), sorted(actual));
    }

    private static String[] sorted(String[] values) {
        String[] copy = values.clone();
        Arrays.sort(copy);
        return copy;
    }

    private static IAsyncResult foreignResult() {
        return new IAsyncResult() {
            @Override public Object getAsyncState() { return null; }
            @Override public boolean getCompletedSynchronously() { return true; }
            @Override public boolean getIsCompleted() { return true; }
        };
    }

    private static void runOnThread(Runnable operation, String name) {
        Thread thread = new Thread(operation, name);
        thread.start();
        try {
            thread.join();
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            throw new IllegalStateException("Interrupted while waiting for Storage probe", exception);
        }
    }
}
