package Microsoft.Xna.Framework;

import Microsoft.Xna.Framework.Graphics.DepthFormat;
import Microsoft.Xna.Framework.Graphics.GraphicsAdapter;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.GraphicsProfile;
import Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService;
import Microsoft.Xna.Framework.Graphics.PresentationParameters;
import Microsoft.Xna.Framework.Graphics.PresentInterval;
import Microsoft.Xna.Framework.Graphics.RenderTargetUsage;
import Microsoft.Xna.Framework.Graphics.SurfaceFormat;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGameHandle;
import org.openeggbert.cna.internal.NativeGraphicsDeviceManagerHandle;

import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** XNA graphics preference and device-lifecycle manager backed by CNA when the game becomes native. */
@SuppressWarnings("this-escape")
public class GraphicsDeviceManager
        implements IGraphicsDeviceManager, IGraphicsDeviceService, AutoCloseable {

    public static final int DefaultBackBufferWidth = 800;
    public static final int DefaultBackBufferHeight = 480;

    private final Game game;
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> deviceCreatedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> deviceDisposingListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> deviceResetListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> deviceResettingListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> disposedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<PreparingDeviceSettingsEventArgs>>
            preparingDeviceSettingsListeners = new CopyOnWriteArrayList<>();

    private NativeGraphicsDeviceManagerHandle nativeManager;
    private GraphicsProfile graphicsProfile = GraphicsProfile.Reach;
    private SurfaceFormat preferredBackBufferFormat = SurfaceFormat.Color;
    private int preferredBackBufferWidth = DefaultBackBufferWidth;
    private int preferredBackBufferHeight = DefaultBackBufferHeight;
    private DepthFormat preferredDepthStencilFormat = DepthFormat.Depth24;
    private DisplayOrientation supportedOrientations = DisplayOrientation.Default;
    private boolean synchronizeWithVerticalRetrace = true;
    private boolean fullScreen;
    private boolean preferMultiSampling;
    private boolean disposeInvoked;
    private boolean disposedEventRaised;
    private boolean closed;
    private Throwable pendingListenerFailure;

    public GraphicsDeviceManager(Game game) {
        this.game = Objects.requireNonNull(game, "game");
        game.attachGraphicsManager(this);
        boolean registeredManager = false;
        try {
            game.getServices().AddService(IGraphicsDeviceManager.class, this);
            registeredManager = true;
            game.getServices().AddService(IGraphicsDeviceService.class, this);
        } catch (RuntimeException failure) {
            if (registeredManager) {
                game.getServices().RemoveService(IGraphicsDeviceManager.class);
            }
            game.detachGraphicsManager(this);
            throw failure;
        }
    }

    public final GraphicsDevice getGraphicsDevice() {
        ensureOpen();
        return game.getGraphicsDevice();
    }

    public final GraphicsProfile getGraphicsProfile() {
        ensureOpen();
        return graphicsProfile;
    }

    public final void setGraphicsProfile(GraphicsProfile value) {
        ensureOpen();
        GraphicsProfile next = Objects.requireNonNull(value, "value");
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerGraphicsProfile(nativeManager, next.ordinal());
        }
        graphicsProfile = next;
    }

    public final boolean getIsFullScreen() {
        ensureOpen();
        return fullScreen;
    }

    public final void setIsFullScreen(boolean value) {
        ensureOpen();
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerIsFullScreen(nativeManager, value);
        }
        fullScreen = value;
    }

    public final boolean getPreferMultiSampling() {
        ensureOpen();
        return preferMultiSampling;
    }

    public final void setPreferMultiSampling(boolean value) {
        ensureOpen();
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerPreferMultiSampling(nativeManager, value);
        }
        preferMultiSampling = value;
    }

    public final SurfaceFormat getPreferredBackBufferFormat() {
        ensureOpen();
        return preferredBackBufferFormat;
    }

    public final void setPreferredBackBufferFormat(SurfaceFormat value) {
        ensureOpen();
        SurfaceFormat next = Objects.requireNonNull(value, "value");
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerPreferredBackBufferFormat(
                    nativeManager, next.ordinal());
        }
        preferredBackBufferFormat = next;
    }

    public final int getPreferredBackBufferHeight() {
        ensureOpen();
        return preferredBackBufferHeight;
    }

    public final void setPreferredBackBufferHeight(int value) {
        ensureOpen();
        requirePositiveBackBufferDimension(value, "PreferredBackBufferHeight");
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerPreferredBackBufferHeight(nativeManager, value);
        }
        preferredBackBufferHeight = value;
    }

    public final int getPreferredBackBufferWidth() {
        ensureOpen();
        return preferredBackBufferWidth;
    }

    public final void setPreferredBackBufferWidth(int value) {
        ensureOpen();
        requirePositiveBackBufferDimension(value, "PreferredBackBufferWidth");
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerPreferredBackBufferWidth(nativeManager, value);
        }
        preferredBackBufferWidth = value;
    }

    public final DepthFormat getPreferredDepthStencilFormat() {
        ensureOpen();
        return preferredDepthStencilFormat;
    }

    public final void setPreferredDepthStencilFormat(DepthFormat value) {
        ensureOpen();
        DepthFormat next = Objects.requireNonNull(value, "value");
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerPreferredDepthStencilFormat(
                    nativeManager, next.ordinal());
        }
        preferredDepthStencilFormat = next;
    }

    public final DisplayOrientation getSupportedOrientations() {
        ensureOpen();
        return supportedOrientations;
    }

    public final void setSupportedOrientations(DisplayOrientation value) {
        ensureOpen();
        DisplayOrientation next = Objects.requireNonNull(value, "value");
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerSupportedOrientations(
                    nativeManager, next.getValue());
        }
        supportedOrientations = next;
    }

    public final boolean getSynchronizeWithVerticalRetrace() {
        ensureOpen();
        return synchronizeWithVerticalRetrace;
    }

    public final void setSynchronizeWithVerticalRetrace(boolean value) {
        ensureOpen();
        if (nativeManager != null) {
            NativeBindings.setGraphicsDeviceManagerSynchronizeWithVerticalRetrace(
                    nativeManager, value);
        }
        synchronizeWithVerticalRetrace = value;
    }

    public final void addDeviceCreatedListener(EventHandler<EventArgs> listener) {
        deviceCreatedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDeviceCreatedListener(EventHandler<EventArgs> listener) {
        deviceCreatedListeners.remove(listener);
    }

    public final void addDeviceDisposingListener(EventHandler<EventArgs> listener) {
        deviceDisposingListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDeviceDisposingListener(EventHandler<EventArgs> listener) {
        deviceDisposingListeners.remove(listener);
    }

    public final void addDeviceResetListener(EventHandler<EventArgs> listener) {
        deviceResetListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDeviceResetListener(EventHandler<EventArgs> listener) {
        deviceResetListeners.remove(listener);
    }

    public final void addDeviceResettingListener(EventHandler<EventArgs> listener) {
        deviceResettingListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDeviceResettingListener(EventHandler<EventArgs> listener) {
        deviceResettingListeners.remove(listener);
    }

    public final void addDisposedListener(EventHandler<EventArgs> listener) {
        disposedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeDisposedListener(EventHandler<EventArgs> listener) {
        disposedListeners.remove(listener);
    }

    public final void addPreparingDeviceSettingsListener(
            EventHandler<PreparingDeviceSettingsEventArgs> listener) {
        preparingDeviceSettingsListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removePreparingDeviceSettingsListener(
            EventHandler<PreparingDeviceSettingsEventArgs> listener) {
        preparingDeviceSettingsListeners.remove(listener);
    }

    public final void ApplyChanges() {
        NativeGraphicsDeviceManagerHandle handle = ensureNativeManager();
        NativeBindings.applyGraphicsDeviceManagerChanges(handle);
        refreshNativePreferences(handle);
        rethrowPendingListenerFailure();
        NativeBindings.rethrowGraphicsDeviceListenerFailure(game.getGraphicsDevice());
    }

    public final void ToggleFullScreen() {
        NativeGraphicsDeviceManagerHandle handle = ensureNativeManager();
        NativeBindings.toggleGraphicsDeviceManagerFullScreen(handle);
        refreshNativePreferences(handle);
        rethrowPendingListenerFailure();
        NativeBindings.rethrowGraphicsDeviceListenerFailure(game.getGraphicsDevice());
    }

    @Override
    public final void CreateDevice() {
        NativeGraphicsDeviceManagerHandle handle = ensureNativeManager();
        NativeBindings.createGraphicsDeviceManagerDevice(handle);
        refreshNativePreferences(handle);
        rethrowPendingListenerFailure();
        NativeBindings.rethrowGraphicsDeviceListenerFailure(game.getGraphicsDevice());
    }

    @Override
    public final boolean BeginDraw() {
        NativeGraphicsDeviceManagerHandle handle = ensureNativeManager();
        boolean result = NativeBindings.beginGraphicsDeviceManagerDraw(handle);
        rethrowPendingListenerFailure();
        NativeBindings.rethrowGraphicsDeviceListenerFailure(game.getGraphicsDevice());
        return result;
    }

    @Override
    public final void EndDraw() {
        NativeGraphicsDeviceManagerHandle handle = ensureNativeManager();
        NativeBindings.endGraphicsDeviceManagerDraw(handle);
        rethrowPendingListenerFailure();
        NativeBindings.rethrowGraphicsDeviceListenerFailure(game.getGraphicsDevice());
    }

    protected boolean CanResetDevice(GraphicsDeviceInformation newDeviceInfo) {
        Objects.requireNonNull(newDeviceInfo, "newDeviceInfo");
        return graphicsProfile == newDeviceInfo.getGraphicsProfile();
    }

    protected GraphicsDeviceInformation FindBestDevice(boolean anySuitableDevice) {
        throw new UnsupportedOperationException(
                "CNA's C ABI does not expose the manager's negotiated device-candidate search");
    }

    protected void RankDevices(List<GraphicsDeviceInformation> foundDevices) {
        Objects.requireNonNull(foundDevices, "foundDevices");
        throw new UnsupportedOperationException(
                "CNA's C ABI does not expose the manager's platform device-ranking operation");
    }

    protected void OnDeviceCreated(Object sender, EventArgs args) {
        invoke(deviceCreatedListeners, sender, args);
    }

    protected void OnDeviceDisposing(Object sender, EventArgs args) {
        invoke(deviceDisposingListeners, sender, args);
    }

    protected void OnDeviceReset(Object sender, EventArgs args) {
        invoke(deviceResetListeners, sender, args);
    }

    protected void OnDeviceResetting(Object sender, EventArgs args) {
        invoke(deviceResettingListeners, sender, args);
    }

    protected void OnPreparingDeviceSettings(
            Object sender,
            PreparingDeviceSettingsEventArgs args) {
        for (EventHandler<PreparingDeviceSettingsEventArgs> listener
                : preparingDeviceSettingsListeners) {
            listener.invoke(sender, args);
        }
    }

    protected void Dispose(boolean disposing) {
        if (!disposing || closed) {
            return;
        }
        RuntimeException failure = null;
        if (!disposeInvoked) {
            disposeInvoked = true;
            removeServices();
            if (nativeManager == null) {
                try {
                    dispatchDisposedEvent();
                } catch (RuntimeException exception) {
                    failure = exception;
                }
            } else {
                try {
                    NativeBindings.disposeGraphicsDeviceManager(nativeManager);
                } catch (RuntimeException exception) {
                    failure = exception;
                }
            }
        }
        if (nativeManager != null) {
            try {
                nativeManager.close();
                nativeManager = null;
            } catch (RuntimeException exception) {
                failure = appendFailure(failure, exception);
            }
        }
        try {
            rethrowPendingListenerFailure();
        } catch (RuntimeException exception) {
            failure = appendFailure(failure, exception);
        }
        if (nativeManager == null) {
            closed = true;
            game.detachGraphicsManager(this);
            clearListeners();
        }
        if (failure != null) {
            throw failure;
        }
    }

    @Override
    public final void close() {
        Dispose(true);
    }

    final synchronized void attachNative(NativeGameHandle nativeGame) {
        ensureOpen();
        if (nativeManager != null) {
            return;
        }
        NativeGraphicsDeviceManagerHandle created =
                NativeBindings.createGraphicsDeviceManager(nativeGame, this);
        try {
            NativeBindings.setGraphicsDeviceManagerGraphicsProfile(created, graphicsProfile.ordinal());
            NativeBindings.setGraphicsDeviceManagerIsFullScreen(created, fullScreen);
            NativeBindings.setGraphicsDeviceManagerPreferMultiSampling(created, preferMultiSampling);
            NativeBindings.setGraphicsDeviceManagerPreferredBackBufferFormat(
                    created, preferredBackBufferFormat.ordinal());
            NativeBindings.setGraphicsDeviceManagerPreferredBackBufferWidth(
                    created, preferredBackBufferWidth);
            NativeBindings.setGraphicsDeviceManagerPreferredBackBufferHeight(
                    created, preferredBackBufferHeight);
            NativeBindings.setGraphicsDeviceManagerPreferredDepthStencilFormat(
                    created, preferredDepthStencilFormat.ordinal());
            NativeBindings.setGraphicsDeviceManagerSynchronizeWithVerticalRetrace(
                    created, synchronizeWithVerticalRetrace);
            NativeBindings.setGraphicsDeviceManagerSupportedOrientations(
                    created, supportedOrientations.getValue());
            nativeManager = created;
        } catch (RuntimeException failure) {
            try {
                created.close();
            } catch (RuntimeException closeFailure) {
                failure.addSuppressed(closeFailure);
            }
            throw failure;
        }
    }

    final void closeFromGame() {
        close();
    }

    final void setSupportedOrientationsFromWindow(DisplayOrientation value) {
        setSupportedOrientations(value);
    }

    @SuppressWarnings("unused")
    private void nativeGraphicsDeviceManagerEvent(int event) {
        try {
            switch (event) {
                case 0 -> dispatchDisposedEvent();
                case 1 -> OnDeviceCreated(this, EventArgs.Empty);
                case 2 -> OnDeviceDisposing(this, EventArgs.Empty);
                case 3 -> OnDeviceReset(this, EventArgs.Empty);
                case 4 -> OnDeviceResetting(this, EventArgs.Empty);
                default -> throw new IllegalArgumentException(
                        "Unknown native graphics-device-manager event " + event);
            }
        } catch (Throwable failure) {
            recordListenerFailure(failure);
        }
    }

    @SuppressWarnings("unused")
    private int[] nativePreparingDeviceSettings(int[] nativeValue) {
        int[] original = nativeValue.clone();
        try {
            GraphicsDeviceInformation information = fromNativeInformation(nativeValue);
            OnPreparingDeviceSettings(this, new PreparingDeviceSettingsEventArgs(information));
            return toNativeInformation(information, original[11]);
        } catch (Throwable failure) {
            recordListenerFailure(failure);
            return original;
        }
    }

    private NativeGraphicsDeviceManagerHandle ensureNativeManager() {
        ensureOpen();
        if (nativeManager == null) {
            game.prepareNativeGraphicsManager(this);
        }
        if (nativeManager == null) {
            throw new IllegalStateException("GraphicsDeviceManager native creation did not complete");
        }
        return nativeManager;
    }

    private void refreshNativePreferences(NativeGraphicsDeviceManagerHandle handle) {
        graphicsProfile = enumValue(GraphicsProfile.values(),
                NativeBindings.getGraphicsDeviceManagerGraphicsProfile(handle), "GraphicsProfile");
        fullScreen = NativeBindings.getGraphicsDeviceManagerIsFullScreen(handle);
        preferMultiSampling = NativeBindings.getGraphicsDeviceManagerPreferMultiSampling(handle);
        preferredBackBufferFormat = enumValue(SurfaceFormat.values(),
                NativeBindings.getGraphicsDeviceManagerPreferredBackBufferFormat(handle),
                "BackBufferFormat");
        preferredBackBufferWidth =
                NativeBindings.getGraphicsDeviceManagerPreferredBackBufferWidth(handle);
        preferredBackBufferHeight =
                NativeBindings.getGraphicsDeviceManagerPreferredBackBufferHeight(handle);
        preferredDepthStencilFormat = enumValue(DepthFormat.values(),
                NativeBindings.getGraphicsDeviceManagerPreferredDepthStencilFormat(handle),
                "DepthStencilFormat");
        synchronizeWithVerticalRetrace =
                NativeBindings.getGraphicsDeviceManagerSynchronizeWithVerticalRetrace(handle);
        supportedOrientations = DisplayOrientation.FromValue(
                NativeBindings.getGraphicsDeviceManagerSupportedOrientations(handle));
    }

    private GraphicsDeviceInformation fromNativeInformation(int[] value) {
        if (value.length != 12) {
            throw new IllegalArgumentException("Invalid native graphics-device-information length");
        }
        GraphicsDeviceInformation information = new GraphicsDeviceInformation();
        if (value[0] >= 0) {
            if (value[0] == 0) {
                information.setAdapter(GraphicsAdapter.getDefaultAdapter());
            } else {
                List<GraphicsAdapter> adapters = GraphicsAdapter.getAdapters();
                if (value[0] >= adapters.size()) {
                    throw new IllegalArgumentException(
                            "CNA proposed unavailable graphics adapter index " + value[0]);
                }
                information.setAdapter(adapters.get(value[0]));
            }
        }
        information.setGraphicsProfile(enumValue(
                GraphicsProfile.values(), value[1], "GraphicsProfile"));
        PresentationParameters parameters = new PresentationParameters();
        parameters.setBackBufferFormat(
                enumValue(SurfaceFormat.values(), value[2], "BackBufferFormat"));
        parameters.setBackBufferWidth(value[3]);
        parameters.setBackBufferHeight(value[4]);
        parameters.setDepthStencilFormat(
                enumValue(DepthFormat.values(), value[5], "DepthStencilFormat"));
        parameters.setMultiSampleCount(value[6]);
        parameters.setPresentationInterval(
                enumValue(PresentInterval.values(), value[7], "PresentationInterval"));
        parameters.setDisplayOrientation(DisplayOrientation.FromValue(value[8]));
        parameters.setRenderTargetUsage(
                enumValue(RenderTargetUsage.values(), value[9], "RenderTargetUsage"));
        parameters.setIsFullScreen(value[10] != 0);
        information.setHeadlessExtension(value[11] != 0);
        information.setPresentationParameters(parameters);
        return information;
    }

    private int[] toNativeInformation(GraphicsDeviceInformation information, int originalHeadless) {
        Objects.requireNonNull(information, "information");
        PresentationParameters parameters =
                Objects.requireNonNull(information.getPresentationParameters(), "PresentationParameters");
        if (!parameters.getDeviceWindowHandle().getIsZero()) {
            throw new IllegalArgumentException(
                    "CNA's mutable device-settings ABI cannot change DeviceWindowHandle");
        }
        GraphicsAdapter adapter = Objects.requireNonNull(information.getAdapter(), "Adapter");
        int adapterIndex = adapter == GraphicsAdapter.getDefaultAdapter()
                ? 0 : GraphicsAdapter.getAdapters().indexOf(adapter);
        if (adapterIndex < 0) {
            throw new IllegalArgumentException("Adapter was not issued by CNA-Java");
        }
        return new int[] {
                adapterIndex,
                Objects.requireNonNull(information.getGraphicsProfile(), "GraphicsProfile").ordinal(),
                Objects.requireNonNull(parameters.getBackBufferFormat(), "BackBufferFormat").ordinal(),
                parameters.getBackBufferWidth(),
                parameters.getBackBufferHeight(),
                Objects.requireNonNull(parameters.getDepthStencilFormat(), "DepthStencilFormat").ordinal(),
                parameters.getMultiSampleCount(),
                Objects.requireNonNull(parameters.getPresentationInterval(), "PresentationInterval").ordinal(),
                Objects.requireNonNull(parameters.getDisplayOrientation(), "DisplayOrientation").getValue(),
                Objects.requireNonNull(parameters.getRenderTargetUsage(), "RenderTargetUsage").ordinal(),
                parameters.getIsFullScreen() ? 1 : 0,
                originalHeadless
        };
    }

    private void dispatchDisposedEvent() {
        if (disposedEventRaised) {
            return;
        }
        disposedEventRaised = true;
        invoke(disposedListeners, this, EventArgs.Empty);
    }

    private void removeServices() {
        if (game.getServices().GetService(IGraphicsDeviceManager.class) == this) {
            game.getServices().RemoveService(IGraphicsDeviceManager.class);
        }
        if (game.getServices().GetService(IGraphicsDeviceService.class) == this) {
            game.getServices().RemoveService(IGraphicsDeviceService.class);
        }
    }

    private void recordListenerFailure(Throwable failure) {
        synchronized (this) {
            if (pendingListenerFailure == null) {
                pendingListenerFailure = failure;
            } else {
                pendingListenerFailure.addSuppressed(failure);
            }
        }
    }

    private void rethrowPendingListenerFailure() {
        Throwable failure;
        synchronized (this) {
            failure = pendingListenerFailure;
            pendingListenerFailure = null;
        }
        if (failure instanceof RuntimeException runtime) {
            throw runtime;
        }
        if (failure instanceof Error error) {
            throw error;
        }
        if (failure != null) {
            throw new IllegalStateException("GraphicsDeviceManager listener failed", failure);
        }
    }

    private void ensureOpen() {
        if (closed || disposeInvoked) {
            throw new IllegalStateException("GraphicsDeviceManager is already closed");
        }
    }

    private void clearListeners() {
        deviceCreatedListeners.clear();
        deviceDisposingListeners.clear();
        deviceResetListeners.clear();
        deviceResettingListeners.clear();
        disposedListeners.clear();
        preparingDeviceSettingsListeners.clear();
    }

    private static void invoke(
            CopyOnWriteArrayList<EventHandler<EventArgs>> listeners,
            Object sender,
            EventArgs args) {
        for (EventHandler<EventArgs> listener : listeners) {
            listener.invoke(sender, args);
        }
    }

    private static void requirePositiveBackBufferDimension(int value, String property) {
        if (value <= 0) {
            throw new IllegalArgumentException(property + " must be positive");
        }
    }

    private static <T> T enumValue(T[] values, int index, String name) {
        if (index < 0 || index >= values.length) {
            throw new IllegalArgumentException("CNA returned an invalid " + name + " value " + index);
        }
        return values[index];
    }

    private static RuntimeException appendFailure(
            RuntimeException previous,
            RuntimeException next) {
        if (previous == null) {
            return next;
        }
        previous.addSuppressed(next);
        return previous;
    }
}
