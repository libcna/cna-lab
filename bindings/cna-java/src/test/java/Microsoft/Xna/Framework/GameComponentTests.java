package Microsoft.Xna.Framework;

import org.junit.jupiter.api.Test;

import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

final class GameComponentTests {

    @Test
    void GameComponentCollection_ValidatesMembershipAndRaisesOrderedEvents() {
        GameComponentCollection components = new GameComponentCollection();
        TestGame game = new TestGame();
        GameComponent first = new GameComponent(game);
        GameComponent second = new GameComponent(game);
        List<String> events = new ArrayList<>();
        components.addComponentAddedListener((sender, args) ->
                events.add("add:" + (args.getGameComponent() == first ? "first" : "second")));
        components.addComponentRemovedListener((sender, args) ->
                events.add("remove:" + (args.getGameComponent() == first ? "first" : "second")));

        components.add(first);
        components.set(0, second);
        assertSame(second, components.get(0));
        components.add(second);
        assertEquals(2, components.size());
        assertThrows(NullPointerException.class, () -> components.add(null));
        components.clear();

        assertEquals(List.of("add:first", "remove:first", "add:second", "add:second",
                "remove:second", "remove:second"), events);
        game.close();
    }

    @Test
    void GameComponent_EventsPermitDuplicatesAndCloseExactlyOnce() {
        TestGame game = new TestGame();
        ClosingComponent component = new ClosingComponent(game);
        AtomicInteger enabledEvents = new AtomicInteger();
        AtomicInteger disposedEvents = new AtomicInteger();
        EventHandler<EventArgs> enabled = (sender, args) -> enabledEvents.incrementAndGet();
        component.addEnabledChangedListener(enabled);
        component.addEnabledChangedListener(enabled);
        component.addDisposedListener((sender, args) -> disposedEvents.incrementAndGet());

        component.setEnabled(false);
        component.removeEnabledChangedListener(enabled);
        component.setEnabled(true);
        component.close();
        component.close();

        assertEquals(3, enabledEvents.get());
        assertEquals(1, disposedEvents.get());
        assertEquals(1, component.disposeCalls);
        assertThrows(IllegalStateException.class, () -> component.setUpdateOrder(4));
        game.close();
    }

    @Test
    void CollectionListenerMutation_UsesAStableDispatchSnapshot() {
        TestGame game = new TestGame();
        GameComponentCollection components = new GameComponentCollection();
        List<String> calls = new ArrayList<>();
        EventHandler<GameComponentCollectionEventArgs> second =
                (sender, args) -> calls.add("second");
        EventHandler<GameComponentCollectionEventArgs> first = (sender, args) -> {
            calls.add("first");
            components.removeComponentAddedListener(second);
        };
        components.addComponentAddedListener(first);
        components.addComponentAddedListener(second);

        components.add(new GameComponent(game));
        components.add(new GameComponent(game));

        assertEquals(List.of("first", "second", "first"), calls);
        game.close();
    }

    @Test
    void Game_BaseLifecycleUsesStableUpdateAndDrawOrder() {
        TestGame game = new TestGame();
        List<String> log = new ArrayList<>();
        LoggingDrawable later = new LoggingDrawable(game, "later", log);
        later.setUpdateOrder(10);
        later.setDrawOrder(10);
        LoggingDrawable first = new LoggingDrawable(game, "first", log);
        first.setUpdateOrder(-1);
        first.setDrawOrder(-1);
        LoggingDrawable disabled = new LoggingDrawable(game, "disabled", log);
        disabled.setEnabled(false);
        disabled.setVisible(false);
        game.getComponents().add(later);
        game.getComponents().add(first);
        game.getComponents().add(disabled);

        game.callInitialize();
        assertEquals(List.of("initialize:later", "load:later", "initialize:first", "load:first",
                "initialize:disabled", "load:disabled"), log);
        log.clear();

        GameTime time = new GameTime(Duration.ofSeconds(1), Duration.ofMillis(16));
        game.callUpdate(time);
        game.callDraw(time);
        assertEquals(List.of("update:first", "update:later", "draw:first", "draw:later"), log);

        game.close();
        assertEquals(List.of("update:first", "update:later", "draw:first", "draw:later",
                "unload:later", "unload:first", "unload:disabled"), log);
    }

    @Test
    void Game_ServicesTimingAndDisposedEventAreDeterministicWithoutNativeCreation() {
        TestGame game = new TestGame();
        Runnable service = () -> { };
        game.getServices().AddService(Runnable.class, service);
        assertSame(service, game.getServices().GetService(Runnable.class));
        assertThrows(IllegalArgumentException.class,
                () -> game.getServices().AddService(Runnable.class, service));
        assertThrows(IllegalArgumentException.class,
                () -> game.getServices().AddService(CharSequence.class, service));
        game.getServices().RemoveService(Runnable.class);
        assertEquals(null, game.getServices().GetService(Runnable.class));

        game.setTargetElapsedTime(Duration.ofNanos(16_666_799));
        game.setInactiveSleepTime(Duration.ofMillis(20));
        game.setIsFixedTimeStep(false);
        assertEquals(Duration.ofNanos(16_666_700), game.getTargetElapsedTime());
        assertEquals(Duration.ofMillis(20), game.getInactiveSleepTime());
        assertFalse(game.getIsFixedTimeStep());
        assertFalse(game.getIsActive());
        assertFalse(game.callShowMissingRequirementMessage(new IllegalStateException("missing")));
        assertThrows(IllegalArgumentException.class, () -> game.setTargetElapsedTime(Duration.ZERO));
        assertThrows(IllegalArgumentException.class,
                () -> game.setTargetElapsedTime(Duration.ofSeconds(Long.MAX_VALUE)));
        assertThrows(IllegalArgumentException.class,
                () -> game.setInactiveSleepTime(Duration.ofNanos(-100)));

        AtomicInteger disposed = new AtomicInteger();
        game.addDisposedListener((sender, args) -> disposed.incrementAndGet());
        game.close();
        game.close();
        assertEquals(1, disposed.get());
        assertThrows(IllegalStateException.class, () -> game.setIsFixedTimeStep(true));
    }

    @Test
    void LaunchParameters_PreserveInsertionOrder() {
        LaunchParameters parameters = new LaunchParameters();
        parameters.put("second", "2");
        parameters.put("first", "1");
        assertEquals(List.of("second", "first"), new ArrayList<>(parameters.keySet()));
    }

    @Test
    void DisplayOrientation_ComposesFlagsWithoutPretendingJavaEnumSemantics() {
        DisplayOrientation landscape =
                DisplayOrientation.LandscapeLeft.Or(DisplayOrientation.LandscapeRight);
        assertEquals(3, landscape.getValue());
        assertTrue(landscape.Contains(DisplayOrientation.LandscapeLeft));
        assertTrue(landscape.Contains(DisplayOrientation.Default));
        assertEquals(landscape, DisplayOrientation.FromValue(3));
        assertTrue(WindowHandle.Zero.getIsZero());
        assertEquals(WindowHandle.Zero, WindowHandle.Zero);
    }

    @Test
    void GameWindow_MapsPropertiesOverloadsAndStableEvents() {
        TestGame game = new TestGame();
        ProbeWindow window = new ProbeWindow(game);
        List<String> events = new ArrayList<>();
        EventHandler<EventArgs> client = (sender, args) -> events.add("client");
        window.addClientSizeChangedListener(client);
        window.addOrientationChangedListener((sender, args) -> events.add("orientation"));
        window.addScreenDeviceNameChangedListener((sender, args) -> events.add("screen"));

        assertEquals("Initial", window.getTitle());
        window.setTitle("Changed");
        assertEquals("Changed", window.getTitle());
        assertEquals("Changed", window.appliedTitle);
        window.BeginScreenDeviceChange(true);
        window.EndScreenDeviceChange("display");
        assertTrue(window.fullScreen);
        assertEquals(List.of("display", "0", "0"), window.endedChange);

        window.fireClientSizeChanged();
        window.fireOrientationChanged();
        window.fireScreenDeviceNameChanged();
        assertEquals(List.of("client", "orientation", "screen"), events);
        window.removeClientSizeChangedListener(client);
        window.fireClientSizeChanged();
        assertEquals(3, events.size());
        game.close();
    }

    @Test
    void GameWindow_TitleCanBeConfiguredBeforeNativeCreationAndRejectsUseAfterClose() {
        TestGame game = new TestGame();
        game.getWindow().setTitle("Configured before Run");
        assertEquals("Configured before Run", game.getWindow().getTitle());

        game.close();
        assertThrows(IllegalStateException.class,
                () -> game.getWindow().setTitle("Too late"));
    }

    @Test
    void GameWindow_NativeDispatchContainsExceptionsAndToleratesRemovalDuringDispatch() {
        TestGame game = new TestGame();
        ProbeWindow window = new ProbeWindow(game);
        List<String> events = new ArrayList<>();
        AtomicReference<EventHandler<EventArgs>> removing = new AtomicReference<>();
        removing.set((sender, args) -> {
            events.add("removing");
            window.removeClientSizeChangedListener(removing.get());
        });
        window.addClientSizeChangedListener(removing.get());
        window.addClientSizeChangedListener((sender, args) -> events.add("stable"));

        window.dispatchNativeEvent(0);
        window.rethrowPendingListenerFailure();
        window.dispatchNativeEvent(0);
        window.rethrowPendingListenerFailure();
        assertEquals(List.of("removing", "stable", "stable"), events);

        window.addOrientationChangedListener((sender, args) -> {
            throw new IllegalStateException("window listener failure");
        });
        window.dispatchNativeEvent(1);
        IllegalStateException failure = assertThrows(
                IllegalStateException.class, window::rethrowPendingListenerFailure);
        assertEquals("window listener failure", failure.getMessage());
        window.rethrowPendingListenerFailure();
        window.clearEventListeners();
        game.close();
    }

    @Test
    void Teardown_RemainsClosedWhenDisposedListenersThrow() {
        TestGame game = new TestGame();
        ClosingComponent component = new ClosingComponent(game);
        game.getComponents().add(component);
        game.getComponents().add(component);
        game.addDisposedListener((sender, args) -> {
            throw new IllegalStateException("listener failure");
        });

        IllegalStateException failure = assertThrows(IllegalStateException.class, game::close);
        assertEquals("listener failure", failure.getMessage());
        assertEquals(1, component.disposeCalls);
        assertThrows(IllegalStateException.class, () -> component.setEnabled(false));
        game.close();
    }

    private static final class TestGame extends Game {
        void callInitialize() {
            super.Initialize();
        }

        void callUpdate(GameTime gameTime) {
            super.Update(gameTime);
        }

        void callDraw(GameTime gameTime) {
            super.Draw(gameTime);
        }

        boolean callShowMissingRequirementMessage(RuntimeException exception) {
            return super.ShowMissingRequirementMessage(exception);
        }
    }

    private static final class ClosingComponent extends GameComponent {
        private int disposeCalls;

        ClosingComponent(Game game) {
            super(game);
        }

        @Override
        protected void Dispose(boolean disposing) {
            disposeCalls++;
            super.Dispose(disposing);
        }
    }

    private static final class LoggingDrawable extends DrawableGameComponent {
        private final String name;
        private final List<String> log;

        LoggingDrawable(Game game, String name, List<String> log) {
            super(game);
            this.name = name;
            this.log = log;
        }

        @Override
        public void Initialize() {
            log.add("initialize:" + name);
            super.Initialize();
        }

        @Override
        protected void LoadContent() {
            log.add("load:" + name);
        }

        @Override
        public void Update(GameTime gameTime) {
            log.add("update:" + name);
        }

        @Override
        public void Draw(GameTime gameTime) {
            log.add("draw:" + name);
        }

        @Override
        protected void UnloadContent() {
            log.add("unload:" + name);
        }
    }

    private static final class ProbeWindow extends GameWindow {
        private boolean allowUserResizing;
        private boolean fullScreen;
        private String appliedTitle;
        private List<String> endedChange = List.of();

        ProbeWindow(Game game) {
            super(game, "Initial");
        }

        @Override public boolean getAllowUserResizing() { return allowUserResizing; }
        @Override public void setAllowUserResizing(boolean value) { allowUserResizing = value; }
        @Override public Rectangle getClientBounds() { return new Rectangle(0, 0, 640, 480); }
        @Override public DisplayOrientation getCurrentOrientation() { return DisplayOrientation.Default; }
        @Override public WindowHandle getHandle() { return WindowHandle.Zero; }
        @Override public String getScreenDeviceName() { return "display"; }
        @Override public void BeginScreenDeviceChange(boolean willBeFullScreen) {
            fullScreen = willBeFullScreen;
        }
        @Override public void EndScreenDeviceChange(
                String screenDeviceName, int clientWidth, int clientHeight) {
            endedChange = List.of(screenDeviceName, Integer.toString(clientWidth),
                    Integer.toString(clientHeight));
        }
        @Override protected void SetSupportedOrientations(DisplayOrientation orientations) { }
        @Override protected void SetTitle(String title) { appliedTitle = title; }

        void fireClientSizeChanged() { OnClientSizeChanged(); }
        void fireOrientationChanged() { OnOrientationChanged(); }
        void fireScreenDeviceNameChanged() { OnScreenDeviceNameChanged(); }
    }
}
