package Microsoft.Xna.Framework;

import Microsoft.Xna.Framework.Content.ContentManager;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGameHandle;
import org.openeggbert.cna.internal.FacadeFactory;

import java.time.Duration;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.IdentityHashMap;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;

/** XNA 4.0 game lifecycle facade backed by CNA's stable C ABI through JNI. */
public class Game implements AutoCloseable {

    private static final long TICKS_PER_SECOND = 10_000_000L;
    private static final long DEFAULT_TARGET_TICKS = 166_667L;

    private final GameComponentCollection components = new GameComponentCollection();
    private final GameServiceContainer services = new GameServiceContainer();
    private final LaunchParameters launchParameters = new LaunchParameters();
    private final GameWindow window;
    private final GraphicsDevice graphicsDevice;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> activatedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> deactivatedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> exitingListeners =
            new CopyOnWriteArrayList<>();
    private ContentManager content;
    private NativeGameHandle nativeGame;
    private GraphicsDeviceManager graphicsManager;
    private Duration inactiveSleepTime = Duration.ZERO;
    private Duration targetElapsedTime = durationFromTicks(DEFAULT_TARGET_TICKS);
    private boolean fixedTimeStep = true;
    private boolean initializing;
    private boolean initialized;
    private boolean closing;
    private boolean closed;
    private boolean disposedEventRaised;
    private boolean hasRun;
    private boolean exitRequested;
    private boolean mouseVisible;

    @SuppressWarnings("this-escape")
    public Game() {
        content = new ContentManager(services);
        graphicsDevice = FacadeFactory.createGraphicsDevice(this);
        window = new CnaGameWindow(this, getClass().getSimpleName());
        components.addComponentAddedListener(this::initializeAddedComponent);
    }

    /** Runs the native CNA loop until {@link #Exit()} is requested. */
    public final void Run() {
        ensureOpen();
        if (hasRun) {
            throw new IllegalStateException("Game.Run may only be called once");
        }
        hasRun = true;
        NativeBindings.run(ensureNativeGame());
        window.rethrowPendingListenerFailure();
        NativeBindings.rethrowGraphicsDeviceListenerFailure(graphicsDevice);
    }

    /** Initializes if needed, advances exactly one CNA frame, and returns. */
    public final void RunOneFrame() {
        ensureOpen();
        NativeBindings.runOneFrame(ensureNativeGame());
        window.rethrowPendingListenerFailure();
        NativeBindings.rethrowGraphicsDeviceListenerFailure(graphicsDevice);
    }

    /** Requests normal game-loop termination at CNA's next safe point. */
    public final void Exit() {
        ensureOpen();
        exitRequested = true;
        if (nativeGame != null && !nativeGame.isClosed()) {
            NativeBindings.requestExit(nativeGame);
        }
    }

    public final void ResetElapsedTime() {
        ensureOpen();
        NativeBindings.resetElapsedTime(ensureNativeGame());
    }

    public final void SuppressDraw() {
        ensureOpen();
        NativeBindings.suppressDraw(ensureNativeGame());
    }

    public final void Tick() {
        ensureOpen();
        NativeBindings.tick(ensureNativeGame());
        window.rethrowPendingListenerFailure();
        NativeBindings.rethrowGraphicsDeviceListenerFailure(graphicsDevice);
    }

    public final GameComponentCollection getComponents() {
        return components;
    }

    public final ContentManager getContent() {
        return content;
    }

    public final void setContent(ContentManager value) {
        ensureOpen();
        content = Objects.requireNonNull(value, "value");
    }

    public final GraphicsDevice getGraphicsDevice() {
        return graphicsDevice;
    }

    public final Duration getInactiveSleepTime() {
        if (hasNativeGame()) {
            inactiveSleepTime = durationFromTicks(NativeBindings.getInactiveSleepTime(nativeGame));
        }
        return inactiveSleepTime;
    }

    public final void setInactiveSleepTime(Duration value) {
        ensureOpen();
        long ticks = durationTicks(Objects.requireNonNull(value, "value"));
        if (ticks < 0L) {
            throw new IllegalArgumentException("InactiveSleepTime must not be negative");
        }
        inactiveSleepTime = durationFromTicks(ticks);
        if (hasNativeGame()) {
            NativeBindings.setInactiveSleepTime(nativeGame, ticks);
        }
    }

    public final boolean getIsActive() {
        return hasNativeGame() && NativeBindings.getIsActive(nativeGame);
    }

    public final boolean getIsFixedTimeStep() {
        if (hasNativeGame()) {
            fixedTimeStep = NativeBindings.getFixedTimeStep(nativeGame);
        }
        return fixedTimeStep;
    }

    public final void setIsFixedTimeStep(boolean value) {
        ensureOpen();
        fixedTimeStep = value;
        if (hasNativeGame()) {
            NativeBindings.setFixedTimeStep(nativeGame, value);
        }
    }

    public final boolean getIsMouseVisible() {
        if (hasNativeGame()) {
            mouseVisible = NativeBindings.getMouseVisible(nativeGame);
        }
        return mouseVisible;
    }

    public final LaunchParameters getLaunchParameters() {
        return launchParameters;
    }

    public final void setIsMouseVisible(boolean value) {
        ensureOpen();
        mouseVisible = value;
        if (hasNativeGame()) {
            NativeBindings.setMouseVisible(nativeGame, value);
        }
    }

    public final GameServiceContainer getServices() {
        return services;
    }

    public final Duration getTargetElapsedTime() {
        if (hasNativeGame()) {
            targetElapsedTime = durationFromTicks(NativeBindings.getTargetElapsedTime(nativeGame));
        }
        return targetElapsedTime;
    }

    public final GameWindow getWindow() {
        return window;
    }

    public final void setTargetElapsedTime(Duration value) {
        ensureOpen();
        long ticks = durationTicks(Objects.requireNonNull(value, "value"));
        if (ticks <= 0L) {
            throw new IllegalArgumentException("TargetElapsedTime must be positive");
        }
        targetElapsedTime = durationFromTicks(ticks);
        if (hasNativeGame()) {
            NativeBindings.setTargetElapsedTime(nativeGame, ticks);
        }
    }

    public final void addActivatedListener(EventHandler<EventArgs> listener) {
        activatedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeActivatedListener(EventHandler<EventArgs> listener) {
        activatedListeners.remove(listener);
    }

    public final void addDeactivatedListener(EventHandler<EventArgs> listener) {
        deactivatedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDeactivatedListener(EventHandler<EventArgs> listener) {
        deactivatedListeners.remove(listener);
    }

    public final void addDisposedListener(EventHandler<EventArgs> listener) {
        disposedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDisposedListener(EventHandler<EventArgs> listener) {
        disposedListeners.remove(listener);
    }

    public final void addExitingListener(EventHandler<EventArgs> listener) {
        exitingListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeExitingListener(EventHandler<EventArgs> listener) {
        exitingListeners.remove(listener);
    }

    protected void Initialize() {
        for (IGameComponent component : snapshot(components)) {
            component.Initialize();
        }
    }

    protected void LoadContent() {
    }

    protected void BeginRun() {
    }

    protected void Update(GameTime gameTime) {
        Objects.requireNonNull(gameTime, "gameTime");
        ArrayList<IUpdateable> updateables = new ArrayList<>();
        for (IGameComponent component : snapshot(components)) {
            if (component instanceof IUpdateable updateable && updateable.getEnabled()) {
                updateables.add(updateable);
            }
        }
        updateables.sort(Comparator.comparingInt(IUpdateable::getUpdateOrder));
        for (IUpdateable updateable : updateables) {
            updateable.Update(gameTime);
        }
    }

    protected boolean BeginDraw() {
        return true;
    }

    protected void Draw(GameTime gameTime) {
        Objects.requireNonNull(gameTime, "gameTime");
        ArrayList<IDrawable> drawables = new ArrayList<>();
        for (IGameComponent component : snapshot(components)) {
            if (component instanceof IDrawable drawable && drawable.getVisible()) {
                drawables.add(drawable);
            }
        }
        drawables.sort(Comparator.comparingInt(IDrawable::getDrawOrder));
        for (IDrawable drawable : drawables) {
            drawable.Draw(gameTime);
        }
    }

    protected void EndDraw() {
    }

    protected void EndRun() {
    }

    protected void UnloadContent() {
    }

    protected void OnActivated(Object sender, EventArgs args) {
        invoke(activatedListeners, sender, args);
    }

    protected void OnDeactivated(Object sender, EventArgs args) {
        invoke(deactivatedListeners, sender, args);
    }

    protected void OnExiting(Object sender, EventArgs args) {
        invoke(exitingListeners, sender, args);
    }

    protected boolean ShowMissingRequirementMessage(RuntimeException exception) {
        return false;
    }

    /** Releases component and facade state before the owned native game. */
    protected void Dispose(boolean disposing) {
        if (!disposing) {
            return;
        }
        RuntimeException failure = null;
        Set<IGameComponent> closedComponents =
                Collections.newSetFromMap(new IdentityHashMap<>());
        for (IGameComponent component : snapshot(components)) {
            if (closedComponents.add(component) && component instanceof AutoCloseable closeable) {
                failure = closeResource(closeable, failure);
            }
        }
        try {
            components.clear();
        } catch (RuntimeException exception) {
            failure = appendFailure(failure, exception);
        }
        failure = closeResource(content, failure);
        boolean nativeChildrenReleased = true;
        try {
            NativeBindings.closeGraphicsResources(this);
        } catch (RuntimeException exception) {
            nativeChildrenReleased = false;
            failure = appendFailure(failure, exception);
        }
        boolean managerReleased = graphicsManager == null;
        if (graphicsManager != null) {
            try {
                graphicsManager.closeFromGame();
            } catch (RuntimeException exception) {
                failure = appendFailure(failure, exception);
            }
            managerReleased = graphicsManager == null;
        }
        if (nativeChildrenReleased && managerReleased && nativeGame != null) {
            failure = closeResource(nativeGame, failure);
        }
        if (nativeGame == null || nativeGame.isClosed()) {
            window.clearEventListeners();
            failure = closeResource(graphicsDevice, failure);
        }
        if (failure != null) {
            throw failure;
        }
    }

    /** Repeated calls are harmless; native ownership is released at most once. */
    public final void Dispose() {
        if (closed) {
            return;
        }
        closing = true;
        RuntimeException failure = null;
        try {
            Dispose(true);
        } catch (RuntimeException exception) {
            failure = exception;
        }
        if (nativeGame == null || nativeGame.isClosed()) {
            closed = true;
            if (!disposedEventRaised) {
                disposedEventRaised = true;
                try {
                    invoke(disposedListeners, this, EventArgs.Empty);
                } catch (RuntimeException exception) {
                    failure = appendFailure(failure, exception);
                }
            }
            activatedListeners.clear();
            deactivatedListeners.clear();
            disposedListeners.clear();
            exitingListeners.clear();
        }
        if (failure != null) {
            throw failure;
        }
    }

    @Override
    public final void close() {
        Dispose();
    }

    final void attachGraphicsManager(GraphicsDeviceManager manager) {
        ensureOpen();
        if (graphicsManager != null && graphicsManager != manager) {
            throw new IllegalStateException("A Game already has a GraphicsDeviceManager");
        }
        graphicsManager = manager;
        if (hasNativeGame()) {
            manager.attachNative(nativeGame);
        }
    }

    final void detachGraphicsManager(GraphicsDeviceManager manager) {
        if (graphicsManager == manager) {
            graphicsManager = null;
        }
    }

    final void prepareNativeGraphicsManager(GraphicsDeviceManager manager) {
        ensureOpen();
        if (graphicsManager != manager) {
            throw new IllegalStateException("GraphicsDeviceManager is not attached to this Game");
        }
        manager.attachNative(ensureNativeGame());
    }

    final void setSupportedOrientationsFromWindow(DisplayOrientation value) {
        ensureOpen();
        if (graphicsManager == null) {
            throw new IllegalStateException(
                    "GameWindow supported orientations require a GraphicsDeviceManager");
        }
        graphicsManager.setSupportedOrientationsFromWindow(value);
    }

    final void prepareNativeWindow() {
        ensureOpen();
        ensureNativeGame();
    }

    final void setNativeWindowTitleIfCreated(String title) {
        ensureOpen();
        if (hasNativeGame()) {
            NativeBindings.setWindowTitle(this, title);
        }
    }

    @SuppressWarnings("unused")
    private void nativeInitialize() {
        NativeBindings.ensureGraphicsDeviceEvents(graphicsDevice);
        initializing = true;
        try {
            Initialize();
            initialized = true;
        } finally {
            initializing = false;
        }
    }

    @SuppressWarnings("unused")
    private void nativeLoadContent() {
        LoadContent();
    }

    @SuppressWarnings("unused")
    private void nativeBeginRun() {
        BeginRun();
    }

    @SuppressWarnings("unused")
    private void nativeUpdate(long totalTicks, long elapsedTicks, boolean slowly) {
        Update(gameTime(totalTicks, elapsedTicks, slowly));
    }

    @SuppressWarnings("unused")
    private boolean nativeBeginDraw() {
        org.openeggbert.cna.internal.NativeMedia.dispatchPendingEvents();
        org.openeggbert.cna.internal.NativeStorage.dispatchPendingEvents();
        return BeginDraw();
    }

    @SuppressWarnings("unused")
    private void nativeDraw(long totalTicks, long elapsedTicks, boolean slowly) {
        Draw(gameTime(totalTicks, elapsedTicks, slowly));
    }

    @SuppressWarnings("unused")
    private void nativeEndDraw() {
        EndDraw();
    }

    @SuppressWarnings("unused")
    private void nativeEndRun() {
        EndRun();
    }

    @SuppressWarnings("unused")
    private void nativeUnloadContent() {
        UnloadContent();
    }

    @SuppressWarnings("unused")
    private void nativeExiting() {
        exitRequested = true;
        OnExiting(this, EventArgs.Empty);
    }

    @SuppressWarnings("unused")
    private void nativeWindowEvent(int event) {
        window.dispatchNativeEvent(event);
    }

    private NativeGameHandle ensureNativeGame() {
        if (!hasNativeGame()) {
            nativeGame = NativeBindings.createGame(
                    this, window.getTitle(), fixedTimeStep, durationTicks(targetElapsedTime));
            NativeBindings.setInactiveSleepTime(nativeGame, durationTicks(inactiveSleepTime));
            NativeBindings.setMouseVisible(nativeGame, mouseVisible);
            if (exitRequested) {
                NativeBindings.requestExit(nativeGame);
            }
            if (graphicsManager != null) {
                graphicsManager.attachNative(nativeGame);
            }
        }
        return nativeGame;
    }

    private boolean hasNativeGame() {
        return nativeGame != null && !nativeGame.isClosed();
    }

    private void initializeAddedComponent(
            Object sender,
            GameComponentCollectionEventArgs args) {
        if (initializing || initialized) {
            args.getGameComponent().Initialize();
        }
    }

    private static GameTime gameTime(long totalTicks, long elapsedTicks, boolean slowly) {
        return new GameTime(durationFromTicks(totalTicks), durationFromTicks(elapsedTicks), slowly);
    }

    private static long durationTicks(Duration duration) {
        try {
            return Math.addExact(
                    Math.multiplyExact(duration.getSeconds(), TICKS_PER_SECOND),
                    duration.getNano() / 100L);
        } catch (ArithmeticException exception) {
            throw new IllegalArgumentException("Duration is outside the XNA TimeSpan range", exception);
        }
    }

    private static Duration durationFromTicks(long ticks) {
        long seconds = Math.floorDiv(ticks, TICKS_PER_SECOND);
        long remainingTicks = Math.floorMod(ticks, TICKS_PER_SECOND);
        return Duration.ofSeconds(seconds, remainingTicks * 100L);
    }

    private static void invoke(
            CopyOnWriteArrayList<EventHandler<EventArgs>> listeners,
            Object sender,
            EventArgs args) {
        for (EventHandler<EventArgs> listener : listeners) {
            listener.invoke(sender, args);
        }
    }

    private static RuntimeException closeResource(
            AutoCloseable resource,
            RuntimeException previous) {
        try {
            resource.close();
            return previous;
        } catch (RuntimeException exception) {
            return appendFailure(previous, exception);
        } catch (Exception exception) {
            IllegalStateException wrapped = new IllegalStateException("Failed to close game resource", exception);
            if (previous == null) {
                return wrapped;
            }
            previous.addSuppressed(wrapped);
            return previous;
        }
    }

    private static RuntimeException appendFailure(
            RuntimeException previous,
            RuntimeException exception) {
        if (previous == null) {
            return exception;
        }
        previous.addSuppressed(exception);
        return previous;
    }

    private void ensureOpen() {
        if (closed || closing) {
            throw new IllegalStateException("Game is already closed");
        }
    }

    private static ArrayList<IGameComponent> snapshot(GameComponentCollection source) {
        return new ArrayList<>(source);
    }
}
