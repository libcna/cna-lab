package Microsoft.Xna.Framework.Content;

import Microsoft.Xna.Framework.ServiceProvider;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.IGraphicsDeviceService;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Media.Video;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import org.openeggbert.cna.internal.NativeBindings;
import System.Action;

import java.io.IOException;
import java.io.InputStream;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.NotDirectoryException;
import java.nio.file.Path;
import java.util.LinkedHashMap;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;

/** XNA content facade with CNA-backed Texture2D and SpriteFont loading. */
public class ContentManager implements AutoCloseable {

    private final Map<String, Object> assets = new LinkedHashMap<>();
    private final List<Object> disposableAssets = new ArrayList<>();
    private final ServiceProvider serviceProvider;
    private String rootDirectory = "";
    private boolean closed;

    public ContentManager(ServiceProvider serviceProvider) {
        this.serviceProvider = Objects.requireNonNull(serviceProvider, "serviceProvider");
    }

    public ContentManager(ServiceProvider serviceProvider, String rootDirectory) {
        this(serviceProvider);
        this.rootDirectory = Objects.requireNonNull(rootDirectory, "rootDirectory");
    }

    public final String getRootDirectory() {
        ensureOpen();
        return rootDirectory;
    }

    public final void setRootDirectory(String value) {
        ensureOpen();
        Objects.requireNonNull(value, "value");
        if (!assets.isEmpty()) {
            throw new IllegalStateException(
                    "The content root directory cannot be changed after an asset has been loaded");
        }
        rootDirectory = value;
    }

    public final ServiceProvider getServiceProvider() {
        return serviceProvider;
    }

    /** Normative Java projection of XNA {@code Load<T>(String)} using a class token. */
    public <T> T Load(Class<T> assetType, String assetName) {
        ensureOpen();
        Objects.requireNonNull(assetType, "assetType");
        Objects.requireNonNull(assetName, "assetName");
        if (assetName.isEmpty()) {
            throw new IllegalArgumentException("assetName must not be empty");
        }
        String assetKey = assetKey(assetName);
        Object cached = assets.get(assetKey);
        if (cached != null) {
            if (!assetType.isInstance(cached)) {
                throw new ContentLoadException("Asset was cached with a different Java type: " + assetName);
            }
            return assetType.cast(cached);
        }
        Object loaded;
        boolean loadedByManagedReader = false;
        try {
            if (hasManagedAsset(assetName)) {
                loaded = ReadAsset(assetType, assetName, null);
                loadedByManagedReader = true;
            } else if (assetType == Texture2D.class) {
                loaded = NativeBindings.loadContentTexture2D(
                        this, graphicsDevice(), rootDirectory, assetName);
            } else if (assetType == SpriteFont.class) {
                loaded = NativeBindings.loadContentSpriteFont(
                        this, graphicsDevice(), rootDirectory, assetName);
            } else {
                loaded = ReadAsset(assetType, assetName, null);
                loadedByManagedReader = true;
            }
        } catch (ContentLoadException exception) {
            throw exception;
        } catch (RuntimeException exception) {
            throw new ContentLoadException(
                    "Could not load content asset " + assetName + ": "
                            + exception.getMessage(), exception);
        }
        if (loaded == null && assetType.isPrimitive()) {
            throw new ContentLoadException(
                    "Content asset '" + assetName + "' produced null for " + assetType.getName());
        }
        if (!loadedByManagedReader
                && (loaded instanceof AutoCloseable || loaded instanceof SpriteFont)) {
            disposableAssets.add(loaded);
        }
        assets.put(assetKey, loaded);
        return assetType.cast(loaded);
    }

    /** Managed XNB worker; independent of CNA's loose-file loader registry. */
    protected final <T> T ReadAsset(
            Class<T> assetType,
            String assetName,
            Action<AutoCloseable> recordDisposableObject) {
        ensureOpen();
        Objects.requireNonNull(assetType, "assetType");
        Objects.requireNonNull(assetName, "assetName");
        int disposableStart = disposableAssets.size();
        try (InputStream input = OpenStream(assetName);
             ContentReader reader = ContentReader.create(
                     this, input, assetName, recordDisposableObject)) {
            return reader.readAsset(assetType, new ContentTypeReaderManager());
        } catch (ContentLoadException exception) {
            cleanupPartialLoad(disposableStart, recordDisposableObject, exception);
            throw exception;
        } catch (RuntimeException | IOException exception) {
            ContentLoadException failure = new ContentLoadException(
                    "Could not deserialize content asset '" + assetName + "'",
                    exception instanceof RuntimeException runtime
                            ? runtime : new UncheckedIOException((IOException) exception));
            cleanupPartialLoad(disposableStart, recordDisposableObject, failure);
            throw failure;
        }
    }

    /** Normative readable-stream projection of XNA's protected virtual OpenStream member. */
    protected InputStream OpenStream(String assetName) {
        String requestedName = assetName == null ? "" : assetName;
        Path path;
        try {
            String fileName = cleanAssetName(requestedName + ".xnb");
            path = Path.of(rootDirectory).resolve(fileName).normalize();
            return Files.newInputStream(path);
        } catch (NoSuchFileException | NotDirectoryException exception) {
            throw new ContentLoadException(
                    "Could not find content asset '" + requestedName + "'",
                    new UncheckedIOException(exception));
        } catch (IOException exception) {
            throw new ContentLoadException(
                    "Could not open content asset '" + requestedName + "'",
                    new UncheckedIOException(exception));
        } catch (RuntimeException exception) {
            throw new ContentLoadException(
                    "Could not open content asset '" + requestedName + "'", exception);
        }
    }

    public void Unload() {
        ensureOpen();
        RuntimeException failure = null;
        for (int index = disposableAssets.size() - 1; index >= 0; index--) {
            Object asset = disposableAssets.get(index);
            try {
                if (asset instanceof SpriteFont spriteFont) {
                    NativeBindings.closeSpriteFont(spriteFont);
                } else if (asset instanceof Video video) {
                    org.openeggbert.cna.internal.NativeMedia.closeVideo(video);
                } else if (asset instanceof AutoCloseable closeable) {
                    closeable.close();
                }
            } catch (Exception exception) {
                RuntimeException cause = exception instanceof RuntimeException runtime
                        ? runtime : new RuntimeException(exception);
                if (failure == null) {
                    failure = cause;
                } else {
                    failure.addSuppressed(cause);
                }
            }
        }
        try {
            assets.clear();
            disposableAssets.clear();
            try {
                NativeBindings.unloadContentManager(this);
            } catch (RuntimeException exception) {
                if (failure == null) {
                    failure = exception;
                } else {
                    failure.addSuppressed(exception);
                }
            }
        } finally {
            assets.clear();
            disposableAssets.clear();
        }
        if (failure != null) {
            throw new ContentLoadException("Failed to unload content", failure);
        }
    }

    public final void Dispose() {
        if (closed) {
            return;
        }
        Dispose(true);
    }

    @Override
    public final void close() {
        Dispose();
    }

    protected void Dispose(boolean disposing) {
        if (closed) {
            return;
        }
        RuntimeException failure = null;
        try {
            if (disposing) {
                Unload();
            }
        } catch (RuntimeException exception) {
            failure = exception;
        } finally {
            try {
                NativeBindings.closeContentManager(this);
            } catch (RuntimeException exception) {
                if (failure == null) failure = exception;
                else failure.addSuppressed(exception);
            }
            closed = true;
        }
        if (failure != null) throw failure;
    }

    final void recordDisposableObject(AutoCloseable value) {
        disposableAssets.add(Objects.requireNonNull(value, "value"));
    }

    final void recordManagedNativeObject(Object value) {
        if (!(value instanceof SpriteFont) && !(value instanceof Video)) {
            throw new IllegalArgumentException("Unsupported managed native content object");
        }
        disposableAssets.add(value);
    }

    final GraphicsDevice graphicsDeviceForContentReader() {
        return graphicsDevice();
    }

    boolean hasManagedAsset(String assetName) {
        try {
            return Files.isRegularFile(Path.of(rootDirectory).resolve(
                    cleanAssetName(Objects.requireNonNull(assetName, "assetName") + ".xnb"))
                    .normalize());
        } catch (RuntimeException exception) {
            return false;
        }
    }

    private GraphicsDevice graphicsDevice() {
        Object service = serviceProvider.GetService(IGraphicsDeviceService.class);
        if (!(service instanceof IGraphicsDeviceService graphicsService)) {
            throw new ContentLoadException(
                    "ContentManager requires an IGraphicsDeviceService for native assets");
        }
        GraphicsDevice graphicsDevice = graphicsService.getGraphicsDevice();
        if (graphicsDevice == null) {
            throw new ContentLoadException("The graphics-device service returned no device");
        }
        return graphicsDevice;
    }

    private static String assetKey(String assetName) {
        return cleanAssetName(assetName).toLowerCase(Locale.ROOT);
    }

    private static String cleanAssetName(String assetName) {
        return Path.of(assetName.replace('\\', '/')).normalize().toString().replace('\\', '/');
    }

    private void ensureOpen() {
        if (closed) {
            throw new IllegalStateException("ContentManager is already closed");
        }
    }

    private void cleanupPartialLoad(
            int start,
            Action<AutoCloseable> externalRecorder,
            RuntimeException failure) {
        if (externalRecorder != null) {
            return;
        }
        for (int index = disposableAssets.size() - 1; index >= start; index--) {
            Object value = disposableAssets.remove(index);
            try {
                if (value instanceof SpriteFont font) {
                    NativeBindings.closeSpriteFont(font);
                } else if (value instanceof Video video) {
                    org.openeggbert.cna.internal.NativeMedia.closeVideo(video);
                } else if (value instanceof AutoCloseable closeable) {
                    closeable.close();
                }
            } catch (Exception closeFailure) {
                failure.addSuppressed(closeFailure);
            }
        }
    }
}
