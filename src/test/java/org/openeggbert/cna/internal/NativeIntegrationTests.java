package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.DisplayOrientation;
import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.IGraphicsDeviceManager;
import Microsoft.Xna.Framework.PreparingDeviceSettingsEventArgs;
import Microsoft.Xna.Framework.PlayerIndex;
import Microsoft.Xna.Framework.Rectangle;
import Microsoft.Xna.Framework.Input.GamePad;
import Microsoft.Xna.Framework.Input.GamePadCapabilities;
import Microsoft.Xna.Framework.Input.GamePadDeadZone;
import Microsoft.Xna.Framework.Input.GamePadState;
import Microsoft.Xna.Framework.Input.Keyboard;
import Microsoft.Xna.Framework.Input.KeyboardState;
import Microsoft.Xna.Framework.Input.Keys;
import Microsoft.Xna.Framework.Input.ButtonState;
import Microsoft.Xna.Framework.Input.Mouse;
import Microsoft.Xna.Framework.Input.MouseState;
import Microsoft.Xna.Framework.Audio.AudioChannels;
import Microsoft.Xna.Framework.Audio.DynamicSoundEffectInstance;
import Microsoft.Xna.Framework.Audio.SoundEffect;
import Microsoft.Xna.Framework.Audio.SoundEffectInstance;
import Microsoft.Xna.Framework.Graphics.SpriteBatch;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Graphics.SpriteSortMode;
import Microsoft.Xna.Framework.Graphics.Blend;
import Microsoft.Xna.Framework.Graphics.BlendFunction;
import Microsoft.Xna.Framework.Graphics.BlendState;
import Microsoft.Xna.Framework.Graphics.ColorWriteChannels;
import Microsoft.Xna.Framework.Graphics.CompareFunction;
import Microsoft.Xna.Framework.Graphics.CullMode;
import Microsoft.Xna.Framework.Graphics.DepthStencilState;
import Microsoft.Xna.Framework.Graphics.DepthFormat;
import Microsoft.Xna.Framework.Graphics.DisplayMode;
import Microsoft.Xna.Framework.Graphics.DisplayModeCollection;
import Microsoft.Xna.Framework.Graphics.GraphicsAdapter;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.GraphicsDeviceStatus;
import Microsoft.Xna.Framework.Graphics.GraphicsProfile;
import Microsoft.Xna.Framework.Graphics.ClearOptions;
import Microsoft.Xna.Framework.Graphics.RasterizerState;
import Microsoft.Xna.Framework.Graphics.RenderTarget2D;
import Microsoft.Xna.Framework.Graphics.RenderTargetBinding;
import Microsoft.Xna.Framework.Graphics.RenderTargetCube;
import Microsoft.Xna.Framework.Graphics.SamplerState;
import Microsoft.Xna.Framework.Graphics.SamplerStateCollection;
import Microsoft.Xna.Framework.Graphics.PresentationParameters;
import Microsoft.Xna.Framework.Graphics.ResourceCreatedEventArgs;
import Microsoft.Xna.Framework.Graphics.ResourceDestroyedEventArgs;
import Microsoft.Xna.Framework.Graphics.SurfaceFormat;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.TextureAddressMode;
import Microsoft.Xna.Framework.Graphics.TextureCollection;
import Microsoft.Xna.Framework.Graphics.TextureCube;
import Microsoft.Xna.Framework.Graphics.TextureFilter;
import Microsoft.Xna.Framework.Graphics.Viewport;
import Microsoft.Xna.Framework.Graphics.CubeMapFace;
import Microsoft.Xna.Framework.Graphics.BufferUsage;
import Microsoft.Xna.Framework.Graphics.IndexBuffer;
import Microsoft.Xna.Framework.Graphics.IndexElementSize;
import Microsoft.Xna.Framework.Graphics.PrimitiveType;
import Microsoft.Xna.Framework.Graphics.VertexBuffer;
import Microsoft.Xna.Framework.Graphics.VertexBufferBinding;
import Microsoft.Xna.Framework.Graphics.VertexPositionColor;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector2;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.junit.jupiter.api.io.TempDir;
import org.junit.jupiter.api.function.Executable;

import java.time.Duration;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.*;

@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class NativeIntegrationTests {

    @Test
    void LoadsCompatibleCnaAbi() {
        int version = NativeBindings.requireAvailable();
        assertEquals(0, version >>> 16);
        assertTrue(version >= NativeBindings.COMPILED_ABI_VERSION);
    }

    @Test
    void NativeGameRunsThreeFramesAndShutsDownInOrder() {
        ProbeGame game = new ProbeGame(3);
        game.Run();
        game.close();
        assertEquals(3, game.frames);
        assertEquals(List.of("Initialize", "LoadContent", "BeginRun"), game.events.subList(0, 3));
        assertTrue(game.events.indexOf("EndRun") > game.events.lastIndexOf("Draw"));
        assertTrue(game.events.indexOf("UnloadContent") > game.events.indexOf("EndRun"));
    }

    @Test
    void RepeatedNativeCreateRunDestroyDoesNotRetainParentHandles() {
        for (int iteration = 0; iteration < 25; iteration++) {
            try (ProbeGame game = new ProbeGame(1)) {
                game.Run();
                assertEquals(1, game.frames);
            }
        }
    }

    @Test
    void NativeGameSupportsOneFrameTimingAndSuppressedDrawOperations() {
        StepGame game = new StepGame();
        game.setIsFixedTimeStep(false);
        game.setTargetElapsedTime(Duration.ofMillis(20));
        game.setInactiveSleepTime(Duration.ofMillis(7));
        game.RunOneFrame();

        assertFalse(game.getIsFixedTimeStep());
        assertEquals(Duration.ofMillis(20), game.getTargetElapsedTime());
        assertEquals(Duration.ofMillis(7), game.getInactiveSleepTime());
        assertEquals(1, game.updates);
        assertEquals(1, game.draws);
        assertTrue(game.lateComponent.initialized);

        game.SuppressDraw();
        game.Tick();
        game.ResetElapsedTime();
        assertEquals(2, game.updates);
        assertEquals(1, game.draws);
        game.close();
    }

    @Test
    void NativeGameWindowReportsAWindowOrTheAbsenceOfOneConsistently() {
        try (Game game = new Game()) {
            var window = game.getWindow();
            window.addClientSizeChangedListener((sender, args) -> { });
            window.addOrientationChangedListener((sender, args) -> { });
            AtomicInteger screenChanges = new AtomicInteger();
            EventHandler<EventArgs> screenListener = (sender, args) -> {
                assertSame(window, sender);
                screenChanges.incrementAndGet();
            };
            window.addScreenDeviceNameChangedListener(screenListener);
            assertSame(window, game.getWindow());
            window.setTitle("CNA Java window probe");
            assertEquals("CNA Java window probe", window.getTitle());
            // A platform with no display reports Default -- the orientation of nothing -- and
            // one with a window reports the orientation that window is in. Both are answers, and
            // asserting only the first described the headless platform.
            assertNotNull(window.getCurrentOrientation());
            // The two facts are tied to each other rather than to a platform. CNA's headless
            // platform has no window at all, so the handle is zero and the client bounds are an
            // empty rectangle; a platform that opens one reports both. Asserting the empty
            // rectangle on its own described the headless platform rather than the contract, and
            // said nothing about a build that has a window.
            Rectangle bounds = window.getClientBounds();
            assertEquals(0, bounds.X, "a client rectangle starts at its own origin");
            assertEquals(0, bounds.Y);
            if (window.getHandle().getIsZero()) {
                assertEquals(new Rectangle(), bounds,
                        "a platform with no window has nothing to measure");
                assertEquals(DisplayOrientation.Default, window.getCurrentOrientation(),
                        "and nothing to be oriented");
            } else {
                assertTrue(bounds.Width > 0 && bounds.Height > 0,
                        "a platform with a window measures it: " + bounds);
            }
            assertNotNull(window.getScreenDeviceName());
            window.getAllowUserResizing();
            window.BeginScreenDeviceChange(false);
            String before = window.getScreenDeviceName();
            window.EndScreenDeviceChange("java-headless-display");
            // The event says the name *changed*, not that a change was asked for. A platform
            // with no display takes the requested name, so it changes and the listener fires;
            // one driving a real display keeps the display's own name, so nothing changed and
            // nothing fires. Both are correct, and tying the count to the name is what says so
            // -- asserting one unconditionally described the headless platform.
            int afterFirst = window.getScreenDeviceName().equals(before) ? 0 : 1;
            assertEquals(afterFirst, screenChanges.get(),
                    "the listener fires exactly when the name changed, and it is now "
                    + window.getScreenDeviceName());
            window.removeScreenDeviceNameChangedListener(screenListener);
            window.EndScreenDeviceChange("java-headless-display-removed");
            assertEquals(afterFirst, screenChanges.get(),
                    "a removed listener receives nothing");

            EventHandler<EventArgs> throwing = (sender, args) -> {
                throw new IllegalStateException("native window listener failure");
            };
            window.addScreenDeviceNameChangedListener(throwing);
            if (afterFirst == 1) {
                // Only reachable where the name really changes, for the reason above: a listener
                // that is never entered cannot throw.
                IllegalStateException callbackFailure = assertThrows(
                        IllegalStateException.class,
                        () -> window.EndScreenDeviceChange("java-headless-display-throwing"));
                assertEquals("native window listener failure", callbackFailure.getMessage());
            } else {
                window.EndScreenDeviceChange("java-headless-display-throwing");
            }
            window.removeScreenDeviceNameChangedListener(throwing);
            game.RunOneFrame();
        }
    }

    @Test
    void NativeGraphicsDeviceManagerRoutesPreferencesMutableEventsAndTeardown() {
        Game game = new Game();
        GraphicsDeviceManager manager = new GraphicsDeviceManager(game);
        manager.setPreferredBackBufferWidth(640);
        manager.setPreferredBackBufferHeight(360);
        manager.setSupportedOrientations(
                DisplayOrientation.LandscapeLeft.Or(DisplayOrientation.LandscapeRight));

        AtomicInteger preparing = new AtomicInteger();
        AtomicInteger created = new AtomicInteger();
        AtomicInteger disposed = new AtomicInteger();
        EventHandler<PreparingDeviceSettingsEventArgs> preparingListener = (sender, args) -> {
            assertSame(manager, sender);
            preparing.incrementAndGet();
            args.getGraphicsDeviceInformation().getPresentationParameters()
                    .setBackBufferWidth(641);
        };
        manager.addPreparingDeviceSettingsListener(preparingListener);
        manager.addDeviceCreatedListener((sender, args) -> {
            assertSame(manager, sender);
            created.incrementAndGet();
        });
        manager.addDisposedListener((sender, args) -> disposed.incrementAndGet());

        assertSame(manager, game.getServices().GetService(IGraphicsDeviceManager.class));
        game.RunOneFrame();
        assertTrue(preparing.get() >= 1);
        assertTrue(created.get() >= 1);
        assertEquals(640, manager.getPreferredBackBufferWidth());
        assertEquals(360, manager.getPreferredBackBufferHeight());

        AtomicInteger removedDuringDispatch = new AtomicInteger();
        AtomicReference<EventHandler<PreparingDeviceSettingsEventArgs>> removable =
                new AtomicReference<>();
        EventHandler<PreparingDeviceSettingsEventArgs> removing = (sender, args) ->
                manager.removePreparingDeviceSettingsListener(removable.get());
        removable.set((sender, args) -> removedDuringDispatch.incrementAndGet());
        manager.addPreparingDeviceSettingsListener(removing);
        manager.addPreparingDeviceSettingsListener(removable.get());
        manager.setPreferredBackBufferWidth(642);
        manager.ApplyChanges();
        assertEquals(1, removedDuringDispatch.get());

        EventHandler<PreparingDeviceSettingsEventArgs> throwing = (sender, args) -> {
            throw new IllegalStateException("native manager listener failure");
        };
        manager.addPreparingDeviceSettingsListener(throwing);
        manager.setPreferredBackBufferHeight(361);
        IllegalStateException callbackFailure = assertThrows(
                IllegalStateException.class, manager::ApplyChanges);
        assertEquals("native manager listener failure", callbackFailure.getMessage());
        manager.removePreparingDeviceSettingsListener(throwing);
        manager.removePreparingDeviceSettingsListener(removing);
        manager.removePreparingDeviceSettingsListener(preparingListener);

        game.close();
        assertEquals(1, disposed.get());
        assertNull(game.getServices().GetService(IGraphicsDeviceManager.class));
        manager.close();
        game.close();
    }

    @Test
    void NativeGraphicsAdapterEnumeratesModesAndNegotiatesFormatsInsideFrame() {
        try (AdapterGame game = new AdapterGame()) {
            assertThrows(IllegalStateException.class, GraphicsAdapter::getAdapters);
            game.RunOneFrame();
            assertFalse(game.adapters.isEmpty());
            assertSame(GraphicsAdapter.getDefaultAdapter(), game.adapters.get(0));
            assertTrue(game.defaultAdapter);
            assertNotNull(game.description);
            assertNotNull(game.deviceName);
            assertTrue(game.currentMode.getWidth() > 0);
            assertTrue(game.currentMode.getHeight() > 0);
            assertEquals(new Rectangle(
                    0, 0, game.currentMode.getWidth(), game.currentMode.getHeight()),
                    game.currentMode.getTitleSafeArea());
            assertNotNull(game.supportedModes);
            for (DisplayMode mode : game.supportedModes) {
                assertTrue(mode.getWidth() > 0);
                assertTrue(mode.getHeight() > 0);
            }
            assertNotNull(game.backBufferSelection.getSelectedFormat());
            assertNotNull(game.backBufferSelection.getSelectedDepthFormat());
            assertTrue(game.monitorNotSupported);
        }
    }

    @Test
    void NativeGraphicsDeviceRoutesStateResetResourceEventsAndTeardown() {
        DeviceFoundationGame game = new DeviceFoundationGame();
        game.RunOneFrame();

        assertEquals(GraphicsDeviceStatus.Normal, game.status);
        assertEquals(GraphicsProfile.Reach, game.profile);
        assertNotNull(game.adapter);
        assertTrue(game.displayMode.getWidth() > 0);
        assertTrue(game.displayMode.getHeight() > 0);
        assertTrue(game.presentationParameters.getBackBufferWidth() >= 0);
        assertEquals(new Rectangle(3, 4, 11, 12), game.scissor);
        assertEquals(new Color(9, 17, 33, 65), game.blendFactor);
        assertEquals(0x13579bdf, game.multiSampleMask);
        assertEquals(23, game.referenceStencil);
        assertEquals(7, game.viewport.getX());
        assertEquals(8, game.viewport.getY());
        assertEquals(40, game.viewport.getWidth());
        assertEquals(30, game.viewport.getHeight());
        assertEquals(0.25f, game.viewport.getMinDepth());
        assertEquals(0.75f, game.viewport.getMaxDepth());
        assertEquals(List.of("resetting", "reset"), game.resetEvents);
        assertEquals(2, game.createdCount.get());
        assertEquals(1, game.removedDuringDispatch.get());
        assertSame(game.firstTexture, game.firstCreatedResource);
        assertSame(game.firstTexture, game.firstDestroyedResource);
        assertEquals("event-texture", game.destroyedName);
        assertEquals("event-tag", game.destroyedTag);
        assertTrue(game.presentRouteReached);
        assertEquals("device reset listener failure", game.listenerFailureMessage);

        game.close();
        assertEquals(1, game.disposingCount.get());
        game.close();
    }

    @Test
    void NativeGraphicsStatesAndCollectionsPreserveDescriptorsIdentityAndLifetimeRules() {
        try (GraphicsStateGame game = new GraphicsStateGame()) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    @Test
    void NativeSpriteBatchStateBeginsEnforcePairingDefaultsDisposalAndRecovery() {
        try (SpriteBatchStateGame game = new SpriteBatchStateGame()) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    @Test
    void NativeContentLoadsRealLooseTextureAndCnjSpriteFontAndDrawsAllTextShapes(
            @TempDir Path contentRoot) {
        try (ContentSpriteFontGame game = new ContentSpriteFontGame(contentRoot)) {
            game.RunOneFrame();
            assertTrue(game.completed);
            assertEquals(6, game.drawStringCalls);
            assertTrue(game.firstTextureDisposedAfterUnload);
            assertTrue(game.firstFontDisposedAfterUnload);
            assertTrue(game.missingAssetReportedAsContentFailure);
        }
    }

    @Test
    void NativeKeyboardCapturesIndependentSnapshotsThatAgreeWithThemselves() {
        try (KeyboardGame game = new KeyboardGame()) {
            assertThrows(IllegalStateException.class, Keyboard::GetState);
            game.RunOneFrame();
            assertNotNull(game.first);
            assertNotSame(game.first, game.second);
            assertEquals(game.first, game.second);
            // What is true whatever the host's keyboard is doing: two snapshots of one frame
            // agree, and each one's two ways of asking agree with each other. Asserting that no
            // key is pressed described a platform with no keyboard -- on one that reads a real
            // one, whatever the person at the machine is holding down turns up here.
            Keys[] pressed = game.first.GetPressedKeys();
            assertNotNull(pressed);
            assertArrayEquals(pressed, game.second.GetPressedKeys(),
                    "two snapshots of one frame report the same keys");
            for (Keys key : pressed) {
                assertTrue(game.first.IsKeyDown(key),
                        key + " is in the pressed list, so it must be down");
                assertFalse(game.first.IsKeyUp(key));
            }
            assertEquals(java.util.Arrays.asList(pressed).contains(Keys.Escape),
                    game.first.IsKeyDown(Keys.Escape),
                    "the list and the question agree about a key nobody pressed either");
        }
        assertThrows(IllegalStateException.class, Keyboard::GetState);
    }

    @Test
    void NativeMouseCapturesASnapshotAndBindsToTheWindowTheGameHas() {
        try (MouseGame game = new MouseGame()) {
            assertThrows(IllegalStateException.class, Mouse::GetState);
            game.RunOneFrame();
            assertEquals(new MouseState(), game.state);
            assertEquals(ButtonState.Released, game.state.getLeftButton());
            // Bound to whatever window the game has, which on a platform with none is the zero
            // handle and on one with a window is that window. Tied to the game's own window
            // rather than asserted as zero, which described the headless platform rather than
            // the binding.
            assertEquals(game.getWindow().getHandle().getIsZero(),
                    Mouse.getWindowHandle().getIsZero(),
                    "the mouse is bound to a window exactly when the game has one");
            Mouse.setWindowHandle(game.getWindow().getHandle());
            Mouse.SetPosition(17, 23);
        }
        assertThrows(IllegalStateException.class, Mouse::GetState);
    }

    @Test
    void NativeGamePadPollingAndVibrationReachTheCurrentGame() {
        assertThrows(IllegalStateException.class,
                () -> GamePad.GetState(PlayerIndex.One));
        try (GamePadGame game = new GamePadGame()) {
            game.RunOneFrame();
            assertNotNull(game.defaultState);
            assertNotNull(game.circularState);
            assertNotNull(game.capabilities);
            assertEquals(game.defaultState.getIsConnected(), game.capabilities.getIsConnected());
            assertTrue(game.defaultState.getTriggers().getLeft() >= 0.0f);
            assertTrue(game.defaultState.getTriggers().getLeft() <= 1.0f);
            assertTrue(game.defaultState.getTriggers().getRight() >= 0.0f);
            assertTrue(game.defaultState.getTriggers().getRight() <= 1.0f);
        }
        assertThrows(IllegalStateException.class,
                () -> GamePad.GetCapabilities(PlayerIndex.One));
    }

    @Test
    void NativeTextureStreamAndSpriteBatchRoundTripAndReleaseBeforeGame() {
        GraphicsGame game = new GraphicsGame();
        game.Run();

        assertEquals(2, game.decodedWidth);
        assertEquals(2, game.decodedHeight);
        assertEquals(SurfaceFormat.Color, game.decodedFormat);
        assertArrayEquals(new Color[] {
                new Color(255, 0, 0), new Color(0, 255, 0),
                new Color(0, 0, 255), new Color(255, 255, 255)
        }, game.readBack);
        assertTrue(game.encodedPngBytes > 0);
        assertEquals(2, game.frames);
        assertFalse(game.texture.getIsDisposed());

        game.close();
        assertTrue(game.texture.getIsDisposed());
        assertTrue(game.decoded.getIsDisposed());
        assertTrue(game.spriteBatch.getIsDisposed());
    }

    @Test
    void NativeTextureTransfersCoverArrayWindowsRectanglesAndMipLevels() {
        try (TextureTransferGame game = new TextureTransferGame()) {
            game.RunOneFrame();
            assertTrue(game.completed);
        }
    }

    @Test
    void NativeBackBufferReadbackPreservesArrayWindowsAndReportsBackendSupportHonestly() {
        try (BackBufferReadbackGame game = new BackBufferReadbackGame()) {
            game.RunOneFrame();
            assertTrue(game.routeReached);
            if (game.supported) {
                assertEquals(new Color(12, 34, 56, 78), game.pixel);
            }
        }
    }

    @Test
    void NativeOwnershipStressPreservesHandlesAcrossFailedCreationAndRelease() {
        try (OwnershipStressGame game = new OwnershipStressGame()) {
            game.RunOneFrame();
            assertEquals(200, game.completedCycles);
            assertEquals(100, game.completedAudioCycles);
            assertTrue(game.failedAudioCreationRecovered);
            assertTrue(game.failedCreationRecovered);
            assertTrue(game.retainedReleaseRecovered);
            assertTrue(game.wrongThreadReleaseRecovered);
        }
    }

    @Test
    void NativeCubeTexturesAndRenderTargetsPreserveMetadataIdentityAndLifetime() {
        try (RenderTargetGame game = new RenderTargetGame()) {
            game.RunOneFrame();
            assertTrue(game.completed);
            assertTrue(game.cubeRouteReached);
            assertTrue(game.renderTarget2DRouteReached);
            assertTrue(game.renderTargetCubeRouteReached);
        }
    }

    @Test
    void NativeVertexIndexAndDrawRoutesReportHeadlessCapabilityWithoutLeakingState() {
        VertexIndexRouteGame game = new VertexIndexRouteGame();
        try (game) {
            game.RunOneFrame();
            assertTrue(game.completed);
            assertEquals(150, game.routedDrawCalls);
            // A draw with nothing bound is XNA's InvalidOperationException: CNA answers
            // INVALID_STATE for it (it answered INTERNAL before its exception barrier learned
            // std::logic_error, BINDFIX-003).
            assertArrayEquals(new int[]{3, 3, 3, 3, 3, 3}, game.drawResults);
            assertTrue(game.bufferRouteReached);
            if (game.bufferRoutesSupported) {
                assertEquals(25, game.completedBufferCycles);
                assertEquals(25, game.autoUnboundBufferCycles);
                assertEquals(0, game.refusedBufferCreations);
                // Buffers bound but no effect applied: XNA's InvalidOperationException, which
                // CNA answers as INVALID_STATE since BINDFIX-003.
                assertEquals(3, game.boundDrawResult);
                assertFalse(game.liveVertexBuffer.getIsDisposed());
                assertFalse(game.liveIndexBuffer.getIsDisposed());

                CnaNativeException vertexRelease = assertThrows(
                        CnaNativeException.class, game.liveVertexBuffer::close);
                assertEquals(3, vertexRelease.getResult());
                CnaNativeException indexRelease = assertThrows(
                        CnaNativeException.class, game.liveIndexBuffer::close);
                assertEquals(3, indexRelease.getResult());
                assertFalse(game.liveVertexBuffer.getIsDisposed());
                assertFalse(game.liveIndexBuffer.getIsDisposed());

                game.releaseLiveBindings = true;
                game.RunOneFrame();
                assertTrue(game.liveBindingsReleased);
                game.liveVertexBuffer.close();
                game.liveVertexBuffer.close();
                game.liveIndexBuffer.close();
                game.liveIndexBuffer.close();
                assertTrue(game.liveVertexBuffer.getIsDisposed());
                assertTrue(game.liveIndexBuffer.getIsDisposed());
                assertFalse(game.teardownVertexBuffer.getIsDisposed());
                assertFalse(game.teardownIndexBuffer.getIsDisposed());
            } else {
                assertEquals(0, game.completedBufferCycles);
                assertEquals(2, game.refusedBufferCreations);
            }
        }
        if (game.bufferRoutesSupported) {
            assertTrue(game.teardownVertexBuffer.getIsDisposed());
            assertTrue(game.teardownIndexBuffer.getIsDisposed());
        }
    }

    private static final class ProbeGame extends Game {
        private final List<String> events = new ArrayList<>();
        private final int frameLimit;
        private int frames;

        private ProbeGame(int frameLimit) {
            this.frameLimit = frameLimit;
        }

        @Override protected void Initialize() { events.add("Initialize"); }
        @Override protected void LoadContent() { events.add("LoadContent"); }
        @Override protected void BeginRun() { events.add("BeginRun"); }
        @Override protected void Update(GameTime gameTime) { events.add("Update"); }
        @Override protected void Draw(GameTime gameTime) {
            events.add("Draw");
            if (++frames == frameLimit) Exit();
        }
        @Override protected void EndRun() { events.add("EndRun"); }
        @Override protected void UnloadContent() { events.add("UnloadContent"); }
    }

    private static final class StepGame extends Game {
        private int updates;
        private int draws;
        private LateComponent lateComponent;

        @Override
        protected void Update(GameTime gameTime) {
            updates++;
            if (lateComponent == null) {
                lateComponent = new LateComponent(this);
                getComponents().add(lateComponent);
            }
        }

        @Override
        protected void Draw(GameTime gameTime) {
            draws++;
        }
    }

    private static final class LateComponent extends GameComponent {
        private boolean initialized;

        private LateComponent(Game game) {
            super(game);
        }

        @Override
        public void Initialize() {
            initialized = true;
        }
    }

    private static final class KeyboardGame extends Game {
        private KeyboardState first;
        private KeyboardState second;

        @Override
        protected void Update(GameTime gameTime) {
            first = Keyboard.GetState();
            second = Keyboard.GetState(Microsoft.Xna.Framework.PlayerIndex.One);
        }
    }

    private static final class MouseGame extends Game {
        private MouseState state;

        @Override
        protected void Update(GameTime gameTime) {
            state = Mouse.GetState();
        }
    }

    private static final class GamePadGame extends Game {
        private GamePadState defaultState;
        private GamePadState circularState;
        private GamePadCapabilities capabilities;
        private boolean vibrationApplied;

        @Override
        protected void Update(GameTime gameTime) {
            defaultState = GamePad.GetState(PlayerIndex.One);
            circularState = GamePad.GetState(PlayerIndex.One, GamePadDeadZone.Circular);
            capabilities = GamePad.GetCapabilities(PlayerIndex.One);
            vibrationApplied = GamePad.SetVibration(PlayerIndex.One, 0.0f, 0.0f);
        }
    }

    private static final class AdapterGame extends Game {
        private List<GraphicsAdapter> adapters;
        private boolean defaultAdapter;
        private String description;
        private String deviceName;
        private DisplayMode currentMode;
        private DisplayModeCollection supportedModes;
        private GraphicsAdapter.FormatSelectionResult backBufferSelection;
        private boolean monitorNotSupported;

        @Override
        protected void Update(GameTime gameTime) {
            adapters = GraphicsAdapter.getAdapters();
            GraphicsAdapter adapter = GraphicsAdapter.getDefaultAdapter();
            defaultAdapter = adapter.getIsDefaultAdapter();
            description = adapter.getDescription();
            deviceName = adapter.getDeviceName();
            currentMode = adapter.getCurrentDisplayMode();
            supportedModes = adapter.getSupportedDisplayModes();
            adapter.IsProfileSupported(GraphicsProfile.Reach);
            backBufferSelection = adapter.QueryBackBufferFormat(
                    GraphicsProfile.Reach, SurfaceFormat.Color, DepthFormat.Depth24, 0);
            try {
                adapter.getMonitorHandle();
            } catch (CnaNativeException expected) {
                monitorNotSupported = expected.getResult() == 6;
            }
        }
    }

    private static final class DeviceFoundationGame extends Game {
        private final List<String> resetEvents = new ArrayList<>();
        private final AtomicInteger createdCount = new AtomicInteger();
        private final AtomicInteger removedDuringDispatch = new AtomicInteger();
        private final AtomicInteger disposingCount = new AtomicInteger();
        private final AtomicReference<EventHandler<ResourceCreatedEventArgs>> removableCreated =
                new AtomicReference<>();
        private GraphicsDeviceStatus status;
        private GraphicsProfile profile;
        private GraphicsAdapter adapter;
        private DisplayMode displayMode;
        private PresentationParameters presentationParameters;
        private Rectangle scissor;
        private Color blendFactor;
        private int multiSampleMask;
        private int referenceStencil;
        private Viewport viewport;
        private Texture2D firstTexture;
        private Object firstCreatedResource;
        private Object firstDestroyedResource;
        private String destroyedName;
        private Object destroyedTag;
        private boolean presentRouteReached;
        private String listenerFailureMessage;

        @Override
        protected void Initialize() {
            GraphicsDevice device = getGraphicsDevice();
            device.addDeviceResettingListener((sender, args) -> resetEvents.add("resetting"));
            device.addDeviceResetListener((sender, args) -> resetEvents.add("reset"));
            device.addDisposingListener((sender, args) -> disposingCount.incrementAndGet());
            device.addResourceCreatedListener((sender, args) -> {
                assertSame(device, sender);
                if (firstCreatedResource == null) {
                    firstCreatedResource = args.getResource();
                }
                createdCount.incrementAndGet();
                device.removeResourceCreatedListener(removableCreated.get());
            });
            removableCreated.set((sender, args) -> removedDuringDispatch.incrementAndGet());
            device.addResourceCreatedListener(removableCreated.get());
            device.addResourceDestroyedListener((sender, args) -> {
                assertSame(device, sender);
                ResourceDestroyedEventArgs value = args;
                if (firstDestroyedResource == null) {
                    firstDestroyedResource = NativeBindings.currentGraphicsResourceEvent();
                    destroyedName = value.getName();
                    destroyedTag = value.getTag();
                }
            });
        }

        @Override
        protected void LoadContent() {
            GraphicsDevice device = getGraphicsDevice();
            firstTexture = new Texture2D(device, 1, 1);
            firstTexture.setName("event-texture");
            firstTexture.setTag("event-tag");
            firstTexture.close();
            try (Texture2D secondTexture = new Texture2D(device, 1, 1)) {
                // A second event proves listener removal during CopyOnWrite dispatch.
                assertEquals(1, secondTexture.getWidth());
            }
        }

        @Override
        protected void Update(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();
            status = device.getGraphicsDeviceStatus();
            profile = device.getGraphicsProfile();
            adapter = device.getAdapter();
            displayMode = device.getDisplayMode();
            presentationParameters = device.getPresentationParameters();

            Rectangle requestedScissor = new Rectangle(3, 4, 11, 12);
            device.setScissorRectangle(requestedScissor);
            requestedScissor.X = 99;
            scissor = device.getScissorRectangle();

            Color requestedBlend = new Color(9, 17, 33, 65);
            device.setBlendFactor(requestedBlend);
            requestedBlend.setR(200);
            blendFactor = device.getBlendFactor();

            device.setMultiSampleMask(0x13579bdf);
            multiSampleMask = device.getMultiSampleMask();
            device.setReferenceStencil(23);
            referenceStencil = device.getReferenceStencil();

            Viewport requestedViewport = new Viewport(7, 8, 40, 30);
            requestedViewport.setMinDepth(0.25f);
            requestedViewport.setMaxDepth(0.75f);
            device.setViewport(requestedViewport);
            requestedViewport.setX(100);
            viewport = device.getViewport();

            device.Clear(ClearOptions.Target, new Color(1, 2, 3, 4), 1.0f, 0);
            device.Clear(
                    ClearOptions.Target,
                    new Microsoft.Xna.Framework.Vector4(0.1f, 0.2f, 0.3f, 0.4f),
                    1.0f,
                    0);

            device.Reset();
            EventHandler<EventArgs> throwing = (sender, args) -> {
                throw new IllegalStateException("device reset listener failure");
            };
            device.addDeviceResetListener(throwing);
            try {
                device.Reset();
                fail("Reset should rethrow the contained Java listener failure");
            } catch (IllegalStateException expected) {
                listenerFailureMessage = expected.getMessage();
            } finally {
                device.removeDeviceResetListener(throwing);
            }
            // Ignore the second reset's event pair when checking the first ordered pair.
            resetEvents.subList(2, resetEvents.size()).clear();

            try {
                device.Present();
                presentRouteReached = true;
            } catch (CnaNativeException expected) {
                assertEquals(6, expected.getResult());
                presentRouteReached = true;
            }
        }
    }

    private static final class GraphicsStateGame extends Game {
        private boolean completed;

        /** HiDef: the separate alpha blending this game sets is HiDef-only in XNA. */
        private GraphicsStateGame() {
            new GraphicsDeviceManager(this).setGraphicsProfile(GraphicsProfile.HiDef);
        }

        @Override
        protected void Update(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();

            assertSame(BlendState.Opaque, device.getBlendState());
            assertSame(device.getBlendState(), device.getBlendState());
            assertSame(device, device.getBlendState().getGraphicsDevice());
            assertSame(DepthStencilState.Default, device.getDepthStencilState());
            assertSame(RasterizerState.CullCounterClockwise, device.getRasterizerState());

            // Min and Max take One/One factors in XNA, and CNA enforces the same.
            BlendState blend = new BlendState();
            blend.setColorSourceBlend(Blend.One);
            blend.setColorDestinationBlend(Blend.One);
            blend.setColorBlendFunction(BlendFunction.Min);
            blend.setColorWriteChannels(ColorWriteChannels.Red.Or(ColorWriteChannels.Blue));
            Color blendColor = new Color(4, 8, 16, 32);
            blend.setBlendFactor(blendColor);
            blendColor.setR(99);
            blend.setMultiSampleMask(0x10203040);
            device.setBlendState(blend);
            assertSame(blend, device.getBlendState());
            assertEquals(BlendFunction.Min, device.getBlendState().getColorBlendFunction());
            assertEquals(new Color(4, 8, 16, 32), device.getBlendState().getBlendFactor());
            assertEquals(ColorWriteChannels.FromValue(5),
                    device.getBlendState().getColorWriteChannels());
            assertThrows(IllegalStateException.class,
                    () -> blend.setColorSourceBlend(Blend.One));

            BlendState disposedUnbound = new BlendState();
            disposedUnbound.close();
            assertThrows(IllegalStateException.class,
                    () -> device.setBlendState(disposedUnbound));
            blend.close();
            assertDoesNotThrow(() -> device.setBlendState(blend));
            assertSame(blend, device.getBlendState());
            device.setBlendState(BlendState.Opaque);

            DepthStencilState depth = new DepthStencilState();
            depth.setDepthBufferEnable(true);
            depth.setDepthBufferWriteEnable(false);
            depth.setDepthBufferFunction(CompareFunction.GreaterEqual);
            depth.setStencilEnable(true);
            depth.setReferenceStencil(37);
            device.setDepthStencilState(depth);
            assertSame(depth, device.getDepthStencilState());
            assertEquals(CompareFunction.GreaterEqual,
                    device.getDepthStencilState().getDepthBufferFunction());
            assertEquals(37, device.getDepthStencilState().getReferenceStencil());
            assertThrows(IllegalStateException.class,
                    () -> depth.setStencilEnable(false));
            device.setDepthStencilState(DepthStencilState.Default);
            depth.close();

            RasterizerState rasterizer = new RasterizerState();
            rasterizer.setCullMode(CullMode.None);
            rasterizer.setDepthBias(0.125f);
            rasterizer.setSlopeScaleDepthBias(-0.5f);
            rasterizer.setScissorTestEnable(true);
            device.setRasterizerState(rasterizer);
            assertSame(rasterizer, device.getRasterizerState());
            assertEquals(CullMode.None, device.getRasterizerState().getCullMode());
            assertEquals(0.125f, device.getRasterizerState().getDepthBias());
            assertTrue(device.getRasterizerState().getScissorTestEnable());
            device.setRasterizerState(RasterizerState.CullCounterClockwise);
            rasterizer.close();

            SamplerStateCollection samplers = device.getSamplerStates();
            assertSame(samplers, device.getSamplerStates());
            assertSame(SamplerState.LinearWrap, samplers.get(0));
            SamplerState sampler = new SamplerState();
            sampler.setFilter(TextureFilter.PointMipLinear);
            sampler.setAddressU(TextureAddressMode.Clamp);
            sampler.setAddressV(TextureAddressMode.Mirror);
            sampler.setMaxAnisotropy(7);
            sampler.setMaxMipLevel(2);
            sampler.setMipMapLevelOfDetailBias(-0.25f);
            samplers.set(0, sampler);
            assertSame(sampler, samplers.get(0));
            assertEquals(TextureFilter.PointMipLinear, samplers.get(0).getFilter());
            assertEquals(TextureAddressMode.Mirror, samplers.get(0).getAddressV());
            assertThrows(IllegalStateException.class,
                    () -> sampler.setFilter(TextureFilter.Linear));
            sampler.close();
            assertDoesNotThrow(() -> samplers.set(0, sampler));
            assertSame(sampler, samplers.get(0));
            samplers.set(0, SamplerState.LinearWrap);
            assertThrows(IndexOutOfBoundsException.class, () -> samplers.get(-1));
            assertThrows(IndexOutOfBoundsException.class, () -> samplers.get(16));
            assertThrows(NullPointerException.class, () -> samplers.set(0, null));

            assertSame(device.getVertexSamplerStates(), device.getVertexSamplerStates());
            assertSame(device.getVertexTextures(), device.getVertexTextures());
            if (device.getGraphicsProfile() == GraphicsProfile.Reach) {
                assertThrows(IndexOutOfBoundsException.class,
                        () -> device.getVertexSamplerStates().get(0));
                assertThrows(IndexOutOfBoundsException.class,
                        () -> device.getVertexTextures().get(0));
            }

            TextureCollection textures = device.getTextures();
            assertSame(textures, device.getTextures());
            assertNull(textures.get(0));
            Texture2D texture = new Texture2D(device, 1, 1);
            textures.set(0, texture);
            assertSame(texture, textures.get(0));
            texture.close();
            assertNull(textures.get(0));
            assertThrows(IllegalStateException.class, () -> textures.set(0, texture));
            assertDoesNotThrow(() -> textures.set(0, null));
            assertThrows(IndexOutOfBoundsException.class, () -> textures.get(-1));
            assertThrows(IndexOutOfBoundsException.class, () -> textures.get(16));

            completed = true;
        }
    }

    private static final class SpriteBatchStateGame extends Game {
        private Texture2D texture;
        private SpriteBatch batch;
        private boolean completed;

        /** HiDef: separate alpha blending, which this game begins with, is HiDef-only in XNA. */
        private SpriteBatchStateGame() {
            new GraphicsDeviceManager(this).setGraphicsProfile(GraphicsProfile.HiDef);
        }

        @Override
        protected void LoadContent() {
            texture = new Texture2D(getGraphicsDevice(), 1, 1);
            texture.SetData(new Color[]{Color.White});
            batch = new SpriteBatch(getGraphicsDevice());
        }

        @Override
        protected void Draw(GameTime gameTime) {
            assertThrows(IllegalStateException.class, batch::End);
            assertThrows(IllegalStateException.class,
                    () -> batch.Draw(texture, Microsoft.Xna.Framework.Vector2.getZero(), Color.White));
            assertThrows(IllegalStateException.class,
                    () -> batch.Draw(null, Microsoft.Xna.Framework.Vector2.getZero(), Color.White));

            batch.Begin();
            assertThrows(IllegalStateException.class, batch::Begin);
            batch.End();

            batch.Begin(SpriteSortMode.Deferred, null, null, null, null);
            batch.Draw(texture, Microsoft.Xna.Framework.Vector2.getZero(), Color.White);
            batch.End();

            BlendState custom = new BlendState();
            custom.setColorSourceBlend(Blend.SourceAlpha);
            batch.Begin(SpriteSortMode.Immediate, custom);
            assertThrows(IllegalStateException.class,
                    () -> custom.setColorSourceBlend(Blend.One));
            custom.close();
            batch.Draw(texture, Microsoft.Xna.Framework.Vector2.getZero(), Color.White);
            batch.End();

            BlendState disposed = new BlendState();
            disposed.close();
            assertThrows(IllegalStateException.class,
                    () -> batch.Begin(SpriteSortMode.Deferred, disposed));
            assertDoesNotThrow(() -> {
                batch.Begin();
                batch.End();
            });

            assertThrows(NullPointerException.class,
                    () -> batch.Begin(null, BlendState.AlphaBlend));
            assertDoesNotThrow(() -> {
                batch.Begin(SpriteSortMode.Deferred, null);
                batch.End();
            });

            Texture2D disposedTexture = new Texture2D(getGraphicsDevice(), 1, 1);
            disposedTexture.close();
            batch.Begin();
            assertThrows(IllegalStateException.class,
                    () -> batch.Draw(
                            disposedTexture,
                            Microsoft.Xna.Framework.Vector2.getZero(),
                            Color.White));
            batch.End();

            SpriteBatch disposedBatch = new SpriteBatch(getGraphicsDevice());
            disposedBatch.close();
            disposedBatch.close();
            assertThrows(IllegalStateException.class, disposedBatch::Begin);
            assertThrows(IllegalStateException.class, disposedBatch::End);

            completed = true;
        }
    }

    private static final class ContentSpriteFontGame extends Game {
        private final GraphicsDeviceManager graphicsManager;
        private final Path contentRoot;
        private boolean completed;
        private int drawStringCalls;
        private boolean firstTextureDisposedAfterUnload;
        private boolean firstFontDisposedAfterUnload;
        private boolean missingAssetReportedAsContentFailure;

        private ContentSpriteFontGame(Path contentRoot) {
            this.contentRoot = contentRoot;
            graphicsManager = new GraphicsDeviceManager(this);
        }

        @Override
        protected void LoadContent() {
            Color[] pixels = new Color[16 * 24];
            for (int index = 0; index < pixels.length; index++) {
                pixels[index] = Color.White;
            }
            try (Texture2D atlas = new Texture2D(getGraphicsDevice(), 16, 24);
                 OutputStream output = Files.newOutputStream(contentRoot.resolve("atlas.png"))) {
                atlas.SetData(pixels);
                atlas.SaveAsPng(output, 16, 24);
            } catch (IOException exception) {
                throw new IllegalStateException("Could not create the SpriteFont atlas fixture", exception);
            }
            try {
                Files.writeString(contentRoot.resolve("font.cnj"), """
                        {
                          "cnjVersion": 1,
                          "type": "SpriteFont",
                          "texture": "atlas.png",
                          "lineSpacing": 24,
                          "spacing": 1.5,
                          "defaultCharacter": "?",
                          "glyphs": [
                            { "char": 63, "source": [0, 0, 16, 24], "crop": [0, 0, 16, 24], "kerning": [1, 14, 1] },
                            { "char": 65, "source": [0, 0, 16, 24], "crop": [0, 0, 16, 24], "kerning": [1, 14, 1] }
                          ]
                        }
                        """);
            } catch (IOException exception) {
                throw new IllegalStateException("Could not create the SpriteFont CNJ fixture", exception);
            }
            getContent().setRootDirectory(contentRoot.toString());
        }

        @Override
        protected void Draw(GameTime gameTime) {
            try {
                getContent().Load(SpriteFont.class, "does-not-exist");
                fail("A missing native content asset must fail");
            } catch (Microsoft.Xna.Framework.Content.ContentLoadException expected) {
                missingAssetReportedAsContentFailure = true;
            }

            Texture2D texture = getContent().Load(Texture2D.class, "atlas");
            assertSame(texture, getContent().Load(Texture2D.class, "atlas"));
            assertSame(texture, getContent().Load(Texture2D.class, "ATLAS"));
            assertSame(texture, getContent().Load(Texture2D.class, "folder/../atlas"));
            assertThrows(IllegalStateException.class,
                    () -> getContent().setRootDirectory(contentRoot.resolve("other").toString()));
            assertEquals(16, texture.getWidth());
            assertEquals(24, texture.getHeight());

            SpriteFont font = getContent().Load(SpriteFont.class, "font");
            assertSame(font, getContent().Load(SpriteFont.class, "font"));
            assertEquals(24, font.getLineSpacing());
            assertEquals(1.5f, font.getSpacing());
            assertEquals('?', font.getDefaultCharacter());
            // A SpriteFont's character map is looked up by binary search, so the glyph
            // table is strictly ascending and Characters reports that order.
            assertEquals(List.of('?', 'A'), font.getCharacters());
            assertEquals(font.MeasureString("AA"),
                    font.MeasureString(new StringBuilder("AA")));
            assertTrue(font.MeasureString("AA").X > 0.0f);

            font.setLineSpacing(23);
            font.setSpacing(1.5f);
            assertEquals(23, font.getLineSpacing());
            assertEquals(1.5f, font.getSpacing());
            assertEquals('?', font.getDefaultCharacter());
            assertThrows(IllegalArgumentException.class,
                    () -> font.setDefaultCharacter('\u20ac'));
            assertEquals('?', font.getDefaultCharacter());

            try (SpriteBatch batch = new SpriteBatch(getGraphicsDevice())) {
                Vector2 position = new Vector2(4.0f, 8.0f);
                batch.Begin();
                batch.DrawString(font, "A", position, Color.White);
                drawStringCalls++;
                batch.DrawString(font, new StringBuilder("B"), position, Color.White);
                drawStringCalls++;
                batch.DrawString(font, "C", position, Color.White,
                        0.1f, new Vector2(1.0f, 2.0f), 0.75f,
                        Microsoft.Xna.Framework.Graphics.SpriteEffects.None, 0.2f);
                drawStringCalls++;
                batch.DrawString(font, new StringBuilder("D"), position, Color.White,
                        0.1f, new Vector2(1.0f, 2.0f), 0.75f,
                        Microsoft.Xna.Framework.Graphics.SpriteEffects.None, 0.2f);
                drawStringCalls++;
                batch.DrawString(font, "E", position, Color.White,
                        0.1f, new Vector2(1.0f, 2.0f), new Vector2(0.75f, 1.25f),
                        Microsoft.Xna.Framework.Graphics.SpriteEffects.None, 0.2f);
                drawStringCalls++;
                batch.DrawString(font, new StringBuilder("F"), position, Color.White,
                        0.1f, new Vector2(1.0f, 2.0f), new Vector2(0.75f, 1.25f),
                        Microsoft.Xna.Framework.Graphics.SpriteEffects.None, 0.2f);
                drawStringCalls++;
                batch.End();
            }

            getContent().Unload();
            firstTextureDisposedAfterUnload = texture.getIsDisposed();
            firstFontDisposedAfterUnload = assertThrows(
                    IllegalStateException.class, () -> font.MeasureString("closed")) != null;
            getContent().setRootDirectory(contentRoot.toString());

            SpriteFont reloaded = getContent().Load(SpriteFont.class, "font");
            assertNotSame(font, reloaded);
            assertTrue(reloaded.MeasureString("Reloaded").X > 0.0f);
            assertSame(graphicsManager, getServices().GetService(IGraphicsDeviceManager.class));
            completed = true;
        }
    }

    private static final class TextureTransferGame extends Game {
        private boolean completed;

        @Override
        protected void Update(GameTime gameTime) {
            try (Texture2D texture = new Texture2D(
                    getGraphicsDevice(), 4, 4, true, SurfaceFormat.Color)) {
                assertEquals(3, texture.getLevelCount());

                Color[] source = new Color[20];
                for (int index = 0; index < 16; index++) {
                    source[index + 2] = new Color(index + 1, index + 2, index + 3, 255);
                }
                texture.SetData(source, 2, 16);
                source[2].setR(200);

                Color[] destination = new Color[21];
                Color leftSentinel = new Color(91, 92, 93, 94);
                Color rightSentinel = new Color(81, 82, 83, 84);
                destination[2] = leftSentinel;
                destination[19] = rightSentinel;
                texture.GetData(destination, 3, 16);
                assertSame(leftSentinel, destination[2]);
                assertSame(rightSentinel, destination[19]);
                assertEquals(new Color(1, 2, 3, 255), destination[3]);
                assertEquals(new Color(16, 17, 18, 255), destination[18]);

                Rectangle center = new Rectangle(1, 1, 2, 2);
                Color[] replacement = {
                        new Color(201, 1, 2, 3), new Color(202, 4, 5, 6),
                        new Color(203, 7, 8, 9), new Color(204, 10, 11, 12)
                };
                texture.SetData(0, center, replacement, 0, replacement.length);

                Color[] full = new Color[16];
                texture.GetData(full);
                assertEquals(replacement[0], full[5]);
                assertEquals(replacement[1], full[6]);
                assertEquals(replacement[2], full[9]);
                assertEquals(replacement[3], full[10]);

                Color[] partial = new Color[8];
                Color partialLeft = new Color(11, 12, 13, 14);
                Color partialRight = new Color(21, 22, 23, 24);
                partial[1] = partialLeft;
                partial[6] = partialRight;
                texture.GetData(0, center, partial, 2, 4);
                assertSame(partialLeft, partial[1]);
                assertArrayEquals(replacement,
                        new Color[]{partial[2], partial[3], partial[4], partial[5]});
                assertSame(partialRight, partial[6]);

                Color[] mip = {
                        new Color(31, 32, 33, 34), new Color(41, 42, 43, 44),
                        new Color(51, 52, 53, 54), new Color(61, 62, 63, 64)
                };
                texture.SetData(1, null, mip, 0, mip.length);
                Color[] mipReadback = new Color[4];
                texture.GetData(1, null, mipReadback, 0, mipReadback.length);
                assertArrayEquals(mip, mipReadback);

                assertThrows(IllegalStateException.class,
                        () -> texture.GetData(3, null, new Color[1], 0, 1));
                assertThrows(IllegalArgumentException.class,
                        () -> texture.GetData(
                                0, new Rectangle(3, 3, 2, 2), new Color[4], 0, 4));
                assertThrows(IllegalArgumentException.class,
                        () -> texture.GetData(0, null, new Color[15], 0, 15));
                assertThrows(IndexOutOfBoundsException.class,
                        () -> texture.GetData(0, null, new Color[16], 0, 0));
                assertThrows(IndexOutOfBoundsException.class,
                        () -> texture.SetData(0, null, new Color[16], 16, 1));
                assertThrows(UnsupportedOperationException.class,
                        () -> texture.GetData(
                                0, null,
                                new Microsoft.Xna.Framework.Vector2[16], 0, 16));
            }
            completed = true;
        }
    }

    private static final class BackBufferReadbackGame extends Game {
        private boolean routeReached;
        private boolean supported;
        private Color pixel;

        @Override
        protected void Draw(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();
            Color clear = new Color(12, 34, 56, 78);
            device.Clear(clear);

            int[] info = NativeBindings.getGraphicsDeviceBackBufferInfo(device);
            assertTrue(info[0] > 0);
            assertTrue(info[1] > 0);
            assertEquals(SurfaceFormat.Color.ordinal(), info[2]);

            Color left = new Color(101, 102, 103, 104);
            Color right = new Color(111, 112, 113, 114);
            Color[] destination = {left, null, right};
            try {
                device.GetBackBufferData(
                        new Rectangle(0, 0, 1, 1), destination, 1, 1);
                supported = true;
                pixel = destination[1];
                assertSame(left, destination[0]);
                assertSame(right, destination[2]);
            } catch (CnaNativeException unavailable) {
                assertEquals(6, unavailable.getResult());
            }

            assertThrows(NullPointerException.class,
                    () -> device.GetBackBufferData((Color[])null));
            assertThrows(IllegalArgumentException.class,
                    () -> device.GetBackBufferData(new Color[0]));
            assertThrows(IndexOutOfBoundsException.class,
                    () -> device.GetBackBufferData(new Color[1], 0, 0));
            assertThrows(IllegalArgumentException.class,
                    () -> device.GetBackBufferData(
                            new Rectangle(0, 0, 2, 1), new Color[1], 0, 1));
            assertThrows(IllegalArgumentException.class,
                    () -> device.GetBackBufferData(
                            new Rectangle(info[0], 0, 1, 1), new Color[1], 0, 1));
            assertThrows(UnsupportedOperationException.class,
                    () -> device.GetBackBufferData(new Integer[1]));
            routeReached = true;
        }
    }

    private static final class OwnershipStressGame extends Game {
        private int completedCycles;
        private int completedAudioCycles;
        private boolean failedCreationRecovered;
        private boolean failedAudioCreationRecovered;
        private boolean retainedReleaseRecovered;
        private boolean wrongThreadReleaseRecovered;

        @Override
        protected void Draw(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();
            for (int cycle = 0; cycle < 200; cycle++) {
                Texture2D texture = new Texture2D(device, 1, 1);
                texture.SetData(new Color[]{new Color(cycle, cycle * 3, cycle * 7, 255)});
                SpriteBatch batch = new SpriteBatch(device);
                batch.Begin();
                batch.Draw(texture, Microsoft.Xna.Framework.Vector2.getZero(), Color.White);
                batch.End();
                batch.close();
                batch.close();
                texture.close();
                texture.close();
                assertTrue(batch.getIsDisposed());
                assertTrue(texture.getIsDisposed());
                completedCycles++;
            }

            int[] pcm = new int[640];
            for (int cycle = 0; cycle < 100; cycle++) {
                SoundEffect effect = new SoundEffect(pcm, 8_000, AudioChannels.Mono);
                SoundEffectInstance instance = effect.CreateInstance();
                instance.setVolume(0.5f);
                instance.Stop();
                if ((cycle & 1) == 0) {
                    effect.close();
                    assertTrue(instance.getIsDisposed());
                } else {
                    instance.close();
                    effect.close();
                }
                effect.close();

                try (DynamicSoundEffectInstance dynamic =
                             new DynamicSoundEffectInstance(8_000, AudioChannels.Mono)) {
                    EventHandler<EventArgs> listener = (sender, args) -> { };
                    dynamic.addBufferNeededListener(listener);
                    dynamic.SubmitBuffer(pcm, 0, 320);
                    dynamic.removeBufferNeededListener(listener);
                    dynamic.Stop();
                }
                completedAudioCycles++;
            }

            assertThrows(CnaNativeException.class, () -> NativeAudio.createSoundEffect(
                    new byte[]{0, 0}, 0, 2, 8_000, 0, 0, 1));
            try (SoundEffect recovered = new SoundEffect(pcm, 8_000, AudioChannels.Mono)) {
                assertFalse(recovered.getIsDisposed());
                failedAudioCreationRecovered = true;
            }

            assertThrows(CnaNativeException.class, () -> Texture2D.FromStream(
                    device, new ByteArrayInputStream(new byte[]{1, 2, 3, 4})));
            try (Texture2D recovered = new Texture2D(device, 1, 1)) {
                recovered.SetData(new Color[]{Color.White});
                failedCreationRecovered = true;
            }

            try (SpriteBatch batch = new SpriteBatch(device)) {
                Texture2D retained = new Texture2D(device, 1, 1);
                retained.SetData(new Color[]{Color.White});
                batch.Begin();
                batch.Draw(retained, Microsoft.Xna.Framework.Vector2.getZero(), Color.White);
                CnaNativeException refused = assertThrows(
                        CnaNativeException.class, retained::close);
                assertEquals(3, refused.getResult());
                assertFalse(retained.getIsDisposed());
                batch.End();
                retained.close();
                assertTrue(retained.getIsDisposed());
                retainedReleaseRecovered = true;
            }

            Texture2D threadAffine = new Texture2D(device, 1, 1);
            threadAffine.SetData(new Color[]{Color.White});
            AtomicReference<Throwable> wrongThreadFailure = new AtomicReference<>();
            Thread wrongThread = new Thread(() -> {
                try {
                    threadAffine.close();
                } catch (Throwable failure) {
                    wrongThreadFailure.set(failure);
                }
            }, "cna-java-wrong-thread-release");
            wrongThread.start();
            assertTimeoutPreemptively(Duration.ofSeconds(5), () -> wrongThread.join());
            assertFalse(wrongThread.isAlive());
            CnaNativeException threadFailure = assertInstanceOf(
                    CnaNativeException.class, wrongThreadFailure.get());
            assertEquals(8, threadFailure.getResult());
            assertFalse(threadAffine.getIsDisposed());
            Color[] readback = new Color[1];
            threadAffine.GetData(readback);
            assertEquals(Color.White, readback[0]);
            threadAffine.close();
            assertTrue(threadAffine.getIsDisposed());
            wrongThreadReleaseRecovered = true;
        }
    }

    private static final class VertexIndexRouteGame extends Game {
        private int routedDrawCalls;
        private int completedBufferCycles;
        private int autoUnboundBufferCycles;
        private int refusedBufferCreations;
        private int[] drawResults;
        private Integer boundDrawResult;
        private VertexBuffer liveVertexBuffer;
        private IndexBuffer liveIndexBuffer;
        private VertexBuffer teardownVertexBuffer;
        private IndexBuffer teardownIndexBuffer;
        private boolean releaseLiveBindings;
        private boolean liveBindingsReleased;
        private boolean bufferRouteReached;
        private boolean bufferRoutesSupported;
        private boolean completed;

        /** HiDef: the 32-bit index buffers this game creates are HiDef-only in XNA. */
        private VertexIndexRouteGame() {
            new GraphicsDeviceManager(this).setGraphicsProfile(GraphicsProfile.HiDef);
        }

        @Override
        protected void Draw(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();
            if (releaseLiveBindings) {
                device.SetVertexBuffers(null);
                device.setIndices(null);
                teardownVertexBuffer = new VertexBuffer(
                        device, VertexPositionColor.VertexDeclaration,
                        3, BufferUsage.None);
                teardownIndexBuffer = new IndexBuffer(
                        device, IndexElementSize.SixteenBits,
                        3, BufferUsage.None);
                liveBindingsReleased = true;
                return;
            }
            device.SetVertexBuffers(null);
            assertEquals(0, device.GetVertexBuffers().length);
            device.setIndices(null);
            assertNull(device.getIndices());

            VertexPositionColor[] vertices = {
                    new VertexPositionColor(new Vector3(0.0f, 0.0f, 0.0f), Color.Red),
                    new VertexPositionColor(new Vector3(1.0f, 0.0f, 0.0f), Color.Green),
                    new VertexPositionColor(new Vector3(0.0f, 1.0f, 0.0f), Color.Blue)
            };
            short[] indices16 = {0, 1, 2};
            int[] indices32 = {0, 1, 2};

            for (int iteration = 0; iteration < 25; iteration++) {
                int[] current = {
                        nativeResult(() -> device.DrawPrimitives(
                                PrimitiveType.TriangleList, 0, 1)),
                        nativeResult(() -> device.DrawIndexedPrimitives(
                                PrimitiveType.TriangleList, 0, 0, 3, 0, 1)),
                        nativeResult(() -> device.DrawInstancedPrimitives(
                                PrimitiveType.TriangleList, 0, 0, 3, 0, 1, 1)),
                        nativeResult(() -> device.DrawUserPrimitives(
                                PrimitiveType.TriangleList, vertices, 0, 1)),
                        nativeResult(() -> device.DrawUserIndexedPrimitives(
                                PrimitiveType.TriangleList, vertices, 0, 3,
                                indices16, 0, 1)),
                        nativeResult(() -> device.DrawUserIndexedPrimitives(
                                PrimitiveType.TriangleList, vertices, 0, 3,
                                indices32, 0, 1))
                };
                if (drawResults == null) {
                    drawResults = current;
                } else {
                    assertArrayEquals(drawResults, current);
                }
                routedDrawCalls += 6;
            }

            exerciseBufferRoutes(device);

            if (bufferRoutesSupported) {
                liveVertexBuffer = new VertexBuffer(
                        device, VertexPositionColor.VertexDeclaration,
                        3, BufferUsage.None);
                liveVertexBuffer.SetData(vertices);
                liveIndexBuffer = new IndexBuffer(
                        device, IndexElementSize.SixteenBits,
                        3, BufferUsage.None);
                liveIndexBuffer.SetData(new Short[]{0, 1, 2});
                device.SetVertexBuffer(liveVertexBuffer);
                device.setIndices(liveIndexBuffer);
            }

            try (Texture2D recovery = new Texture2D(device, 1, 1)) {
                recovery.SetData(new Color[]{new Color(7, 8, 9, 10)});
                Color[] readback = new Color[1];
                recovery.GetData(readback);
                assertEquals(new Color(7, 8, 9, 10), readback[0]);
            }
            completed = true;
        }

        @SuppressWarnings("try") // Explicit close calls verify buffer idempotence before try cleanup.
        private void exerciseBufferRoutes(GraphicsDevice device) {
            for (int iteration = 0; iteration < 25; iteration++) {
                try (VertexBuffer vertexBuffer = new VertexBuffer(
                             device, VertexPositionColor.VertexDeclaration,
                             3, BufferUsage.None);
                     IndexBuffer indexBuffer16 = new IndexBuffer(
                             device, IndexElementSize.SixteenBits,
                             3, BufferUsage.None);
                     IndexBuffer indexBuffer32 = new IndexBuffer(
                             device, IndexElementSize.ThirtyTwoBits,
                             3, BufferUsage.None)) {
                    bufferRoutesSupported = true;
                    assertEquals(3, vertexBuffer.getVertexCount());
                    assertEquals(BufferUsage.None, vertexBuffer.getBufferUsage());
                    assertSame(VertexPositionColor.VertexDeclaration,
                            vertexBuffer.getVertexDeclaration());

                    VertexPositionColor first = new VertexPositionColor(
                            new Vector3(iteration + 0.25f, 1.5f, -2.0f),
                            new Color(1, 2, 3, 4));
                    VertexPositionColor second = new VertexPositionColor(
                            new Vector3(4.0f, 5.0f, 6.0f), new Color(7, 8, 9, 10));
                    VertexPositionColor third = new VertexPositionColor(
                            new Vector3(-7.0f, 8.0f, -9.0f), new Color(11, 12, 13, 14));
                    VertexPositionColor expectedFirst = new VertexPositionColor(first);
                    vertexBuffer.SetData(new VertexPositionColor[]{first, second, third});
                    first.Position.X = -999.0f;
                    first.Color.setR(255);
                    VertexPositionColor replacement = new VertexPositionColor(
                            new Vector3(10.0f, 11.0f, 12.0f),
                            new Color(21, 22, 23, 24));
                    vertexBuffer.SetData(
                            16, new VertexPositionColor[]{replacement}, 0, 1, 16);
                    VertexPositionColor[] vertexReadback = new VertexPositionColor[3];
                    vertexBuffer.GetData(vertexReadback);
                    assertArrayEquals(
                            new VertexPositionColor[]{expectedFirst, replacement, third},
                            vertexReadback);

                    indexBuffer16.SetData(new Short[]{0, 1, 2});
                    indexBuffer16.SetData(2, new Short[]{7}, 0, 1);
                    Short[] indexReadback16 = new Short[3];
                    indexBuffer16.GetData(indexReadback16);
                    assertArrayEquals(new Short[]{0, 7, 2}, indexReadback16);
                    assertThrows(UnsupportedOperationException.class,
                            () -> indexBuffer16.GetData(
                                    2, new Short[1], 0, 1));

                    Long[] indexValues32 = {0L, 0x8000_0000L, 0xffff_ffffL};
                    indexBuffer32.SetData(indexValues32);
                    Long[] indexReadback32 = new Long[3];
                    indexBuffer32.GetData(indexReadback32);
                    assertArrayEquals(indexValues32, indexReadback32);

                    device.SetVertexBuffers(new VertexBufferBinding[]{
                            new VertexBufferBinding(vertexBuffer, 1, 0)});
                    VertexBufferBinding[] bindings = device.GetVertexBuffers();
                    assertEquals(1, bindings.length);
                    assertSame(vertexBuffer, bindings[0].getVertexBuffer());
                    assertEquals(1, bindings[0].getVertexOffset());
                    assertEquals(0, bindings[0].getInstanceFrequency());
                    device.setIndices(indexBuffer16);
                    assertSame(indexBuffer16, device.getIndices());

                    int currentBoundDrawResult = nativeResult(() ->
                            device.DrawIndexedPrimitives(
                                    PrimitiveType.TriangleList, 0, 0, 3, 0, 1));
                    if (boundDrawResult == null) {
                        boundDrawResult = currentBoundDrawResult;
                    } else {
                        assertEquals(boundDrawResult.intValue(), currentBoundDrawResult);
                    }

                    vertexBuffer.close();
                    assertEquals(0, device.GetVertexBuffers().length);
                    indexBuffer16.close();
                    assertNull(device.getIndices());
                    vertexBuffer.close();
                    indexBuffer16.close();
                    indexBuffer32.close();
                    indexBuffer32.close();
                    assertTrue(vertexBuffer.getIsDisposed());
                    assertTrue(indexBuffer16.getIsDisposed());
                    assertTrue(indexBuffer32.getIsDisposed());
                    autoUnboundBufferCycles++;
                    completedBufferCycles++;
                } catch (CnaNativeException unavailable) {
                    if (iteration != 0 || unavailable.getResult() != 6) {
                        throw unavailable;
                    }
                    refusedBufferCreations++;
                    assertHeadlessNotSupported(() -> new IndexBuffer(
                            device, IndexElementSize.SixteenBits,
                            3, BufferUsage.None));
                    refusedBufferCreations++;
                    break;
                }
            }
            bufferRouteReached = true;
        }

        private static void assertHeadlessNotSupported(Executable operation) {
            CnaNativeException failure = assertThrows(CnaNativeException.class, operation);
            assertEquals(6, failure.getResult(), failure.getMessage());
        }

        private static int nativeResult(Executable operation) {
            try {
                operation.execute();
                return 0;
            } catch (CnaNativeException failure) {
                return failure.getResult();
            } catch (Throwable failure) {
                throw new AssertionError("Unexpected Java-side draw failure", failure);
            }
        }
    }

    private static final class RenderTargetGame extends Game {
        private boolean completed;
        private boolean cubeRouteReached;
        private boolean renderTarget2DRouteReached;
        private boolean renderTargetCubeRouteReached;

        @Override
        protected void Draw(GameTime gameTime) {
            GraphicsDevice device = getGraphicsDevice();
            assertArrayEquals(new RenderTargetBinding[0], device.GetRenderTargets());
            device.SetRenderTargets((RenderTargetBinding[])null);

            try (TextureCube cube = new TextureCube(
                    device, 4, true, SurfaceFormat.Color)) {
                assertEquals(4, cube.getSize());
                assertEquals(3, cube.getLevelCount());
                assertEquals(SurfaceFormat.Color, cube.getFormat());
                Color[] face = new Color[16];
                for (int index = 0; index < face.length; index++) {
                    face[index] = new Color(index + 1, index + 2, index + 3, 255);
                }
                try {
                    cube.SetData(CubeMapFace.NegativeZ, face);
                    face[0].setR(200);
                    Color[] readback = new Color[18];
                    Color left = new Color(91, 92, 93, 94);
                    Color right = new Color(81, 82, 83, 84);
                    readback[0] = left;
                    readback[17] = right;
                    cube.GetData(CubeMapFace.NegativeZ, readback, 1, 16);
                    assertSame(left, readback[0]);
                    assertEquals(new Color(1, 2, 3, 255), readback[1]);
                    assertEquals(new Color(16, 17, 18, 255), readback[16]);
                    assertSame(right, readback[17]);

                    Color[] mip = {
                            new Color(31, 32, 33, 34), new Color(41, 42, 43, 44),
                            new Color(51, 52, 53, 54), new Color(61, 62, 63, 64)
                    };
                    cube.SetData(CubeMapFace.PositiveY, 1, null, mip, 0, 4);
                    Color[] mipReadback = new Color[4];
                    cube.GetData(CubeMapFace.PositiveY, 1, null, mipReadback, 0, 4);
                    assertArrayEquals(mip, mipReadback);
                } catch (CnaNativeException unavailable) {
                    assertEquals(6, unavailable.getResult());
                }
                assertThrows(IllegalArgumentException.class,
                        () -> cube.GetData(CubeMapFace.PositiveX, new Color[15]));
                assertThrows(UnsupportedOperationException.class,
                        () -> cube.GetData(CubeMapFace.PositiveX, new Integer[16]));
                cubeRouteReached = true;
            }

            RenderTarget2D target2D = new RenderTarget2D(
                    device, 8, 4, true, SurfaceFormat.Color,
                    DepthFormat.Depth24, 0,
                    Microsoft.Xna.Framework.Graphics.RenderTargetUsage.PreserveContents);
            assertEquals(8, target2D.getWidth());
            assertEquals(4, target2D.getHeight());
            assertEquals(SurfaceFormat.Color, target2D.getFormat());
            assertEquals(DepthFormat.Depth24, target2D.getDepthStencilFormat());
            assertEquals(0, target2D.getMultiSampleCount());
            assertEquals(
                    Microsoft.Xna.Framework.Graphics.RenderTargetUsage.PreserveContents,
                    target2D.getRenderTargetUsage());
            assertFalse(target2D.getIsContentLost());
            AtomicInteger contentLost = new AtomicInteger();
            EventHandler<EventArgs> contentLostListener =
                    (sender, args) -> contentLost.incrementAndGet();
            target2D.addContentLostListener(contentLostListener);

            boolean target2DBound = false;
            try {
                device.SetRenderTarget(target2D);
                target2DBound = true;
                RenderTargetBinding[] bindings = device.GetRenderTargets();
                assertEquals(1, bindings.length);
                assertSame(target2D, bindings[0].getRenderTarget());
                assertEquals(CubeMapFace.PositiveX, bindings[0].getCubeMapFace());
                device.SetRenderTargets(new RenderTargetBinding[]{
                        new RenderTargetBinding(target2D)});
                assertSame(target2D, device.GetRenderTargets()[0].getRenderTarget());

                CnaNativeException retained = assertThrows(
                        CnaNativeException.class, target2D::close);
                assertEquals(3, retained.getResult());
                assertFalse(target2D.getIsDisposed());
            } catch (CnaNativeException unavailable) {
                assertEquals(6, unavailable.getResult());
            } finally {
                if (target2DBound) {
                    device.SetRenderTarget((RenderTarget2D)null);
                }
            }
            target2D.removeContentLostListener(contentLostListener);
            target2D.close();
            target2D.close();
            assertEquals(0, contentLost.get());
            assertTrue(target2D.getIsDisposed());
            renderTarget2DRouteReached = true;

            try {
                RenderTargetCube targetCube = new RenderTargetCube(
                        device, 4, false, SurfaceFormat.Color,
                        DepthFormat.None, 0,
                        Microsoft.Xna.Framework.Graphics.RenderTargetUsage.DiscardContents);
                try {
                    assertEquals(4, targetCube.getSize());
                    assertFalse(targetCube.getIsContentLost());
                    boolean cubeBound = false;
                    try {
                        device.SetRenderTarget(targetCube, CubeMapFace.NegativeY);
                        cubeBound = true;
                        RenderTargetBinding[] bindings = device.GetRenderTargets();
                        assertEquals(1, bindings.length);
                        assertSame(targetCube, bindings[0].getRenderTarget());
                        assertEquals(CubeMapFace.NegativeY, bindings[0].getCubeMapFace());
                    } catch (CnaNativeException unavailable) {
                        assertEquals(6, unavailable.getResult());
                    } finally {
                        if (cubeBound) {
                            device.SetRenderTarget((RenderTargetCube)null, CubeMapFace.PositiveX);
                        }
                    }
                } finally {
                    targetCube.close();
                    targetCube.close();
                }
            } catch (CnaNativeException unavailable) {
                assertEquals(6, unavailable.getResult());
            }
            renderTargetCubeRouteReached = true;

            RenderTargetBinding empty = new RenderTargetBinding();
            RenderTargetBinding emptyCopy = new RenderTargetBinding(empty);
            assertNull(emptyCopy.getRenderTarget());
            assertEquals(CubeMapFace.PositiveX, emptyCopy.getCubeMapFace());
            assertThrows(IllegalArgumentException.class,
                    () -> device.SetRenderTargets(new RenderTargetBinding[]{emptyCopy}));
            assertArrayEquals(new RenderTargetBinding[0], device.GetRenderTargets());
            completed = true;
        }
    }

    private static final class GraphicsGame extends Game {
        private final Color[] original = {
                new Color(255, 0, 0), new Color(0, 255, 0),
                new Color(0, 0, 255), new Color(255, 255, 255)
        };
        private Color[] readBack;
        private Texture2D texture;
        private Texture2D decoded;
        private SpriteBatch spriteBatch;
        private int decodedWidth;
        private int decodedHeight;
        private SurfaceFormat decodedFormat;
        private int encodedPngBytes;
        private int frames;

        @Override
        protected void LoadContent() {
            texture = new Texture2D(getGraphicsDevice(), 2, 2);
            texture.SetData(original);
            original[0].setG(99);
            readBack = new Color[4];
            texture.GetData(readBack);

            ByteArrayOutputStream encoded = new ByteArrayOutputStream();
            texture.SaveAsPng(encoded, 2, 2);
            encodedPngBytes = encoded.size();
            decoded = Texture2D.FromStream(
                    getGraphicsDevice(), new ByteArrayInputStream(encoded.toByteArray()));
            decodedWidth = decoded.getWidth();
            decodedHeight = decoded.getHeight();
            decodedFormat = decoded.getFormat();
            spriteBatch = new SpriteBatch(getGraphicsDevice());
        }

        @Override
        protected void Draw(GameTime gameTime) {
            getGraphicsDevice().Clear(Color.CornflowerBlue);
            spriteBatch.Begin();
            spriteBatch.Draw(decoded, new Rectangle(0, 0, 32, 32), Color.White);
            spriteBatch.End();
            if (++frames == 2) {
                Exit();
            }
        }
    }
}
