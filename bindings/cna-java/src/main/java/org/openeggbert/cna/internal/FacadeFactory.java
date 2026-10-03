package org.openeggbert.cna.internal;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.Graphics.Effect;
import Microsoft.Xna.Framework.Graphics.GraphicsDevice;
import Microsoft.Xna.Framework.Graphics.Model;
import Microsoft.Xna.Framework.Graphics.SpriteFont;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Graphics.Texture3D;
import Microsoft.Xna.Framework.Graphics.TextureCube;
import Microsoft.Xna.Framework.Content.ContentManager;
import Microsoft.Xna.Framework.Content.ContentReader;
import Microsoft.Xna.Framework.Media.MediaPlayer;
import Microsoft.Xna.Framework.Media.VisualizationData;
import Microsoft.Xna.Framework.GamerServices.AvatarAnimation;
import Microsoft.Xna.Framework.GamerServices.AvatarRenderer;
import Microsoft.Xna.Framework.Media.Video;
import Microsoft.Xna.Framework.Net.AvailableNetworkSession;
import Microsoft.Xna.Framework.Media.VideoSoundtrackType;
import Microsoft.Xna.Framework.Storage.StorageContainer;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/** Constructs strict cross-package facades without widening their XNA constructor contracts. */
public final class FacadeFactory {

    private static final Constructor<GraphicsDevice> GRAPHICS_DEVICE = graphicsDeviceConstructor();
    private static final Constructor<Texture2D> TEXTURE_2D = texture2DConstructor();
    private static final Method TEXTURE_2D_INITIALIZE = texture2DInitializeMethod();
    private static final Constructor<Texture3D> TEXTURE_3D = texture3DConstructor();
    private static final Method TEXTURE_3D_INITIALIZE = texture3DInitializeMethod();
    private static final Constructor<TextureCube> TEXTURE_CUBE = textureCubeConstructor();
    private static final Method TEXTURE_CUBE_INITIALIZE = textureCubeInitializeMethod();
    private static final Constructor<SpriteFont> SPRITE_FONT = spriteFontConstructor();
    private static final Method MODEL_READ = modelReadMethod();
    private static final Method MEDIA_PLAYER_DISPATCH = mediaPlayerMethod(
            "dispatchNativeEvent", int.class);
    private static final Method MEDIA_PLAYER_RELEASE = mediaPlayerMethod(
            "releaseGameScopedState");
    private static final Method VISUALIZATION_SET = visualizationMethod();
    private static final Constructor<Video> VIDEO = videoConstructor();
    private static final Method VIDEO_SOUNDTRACK_VALUE = videoSoundtrackValueMethod();
    private static final Method AVAILABLE_SESSION_HANDLE = availableSessionHandleMethod();
    private static final Constructor<AvailableNetworkSession> AVAILABLE_SESSION =
            availableSessionConstructor();
    private static final Method AVATAR_RENDERER_HANDLE =
            hiddenHandle(AvatarRenderer.class);
    private static final Method AVATAR_ANIMATION_HANDLE =
            hiddenHandle(AvatarAnimation.class);
    private static final Method SIGNED_IN_GAMER_HANDLE =
            hiddenHandle(Microsoft.Xna.Framework.GamerServices.Gamer.class);
    private static final Method EFFECT_ADOPT_EXTENSION = effectAdoptExtensionMethod();
    private static final Method EFFECT_ADOPT_BORROWED = effectAdoptBorrowedMethod();
    private static final Method CONTENT_MANAGER_DEVICE = contentManagerDeviceMethod();
    private static final Method TEXTURE_2D_LEVEL_BYTES = texture2DLevelBytesMethod();
    private static final Method STORAGE_CONTAINER_HANDLE = storageContainerHandleMethod();

    private FacadeFactory() {
    }

    public static GraphicsDevice createGraphicsDevice(Game game) {
        try {
            return GRAPHICS_DEVICE.newInstance(game);
        } catch (InstantiationException | IllegalAccessException exception) {
            throw new IllegalStateException("Cannot construct the GraphicsDevice facade", exception);
        } catch (InvocationTargetException exception) {
            Throwable cause = exception.getCause();
            if (cause instanceof RuntimeException runtime) {
                throw runtime;
            }
            throw new IllegalStateException("GraphicsDevice facade construction failed", cause);
        }
    }

    public static Texture2D createUninitializedTexture2D(GraphicsDevice graphicsDevice) {
        try {
            return TEXTURE_2D.newInstance(graphicsDevice);
        } catch (InstantiationException | IllegalAccessException exception) {
            throw new IllegalStateException("Cannot construct the Texture2D facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Texture2D facade construction failed", exception);
        }
    }

    public static void initializeTexture2D(Texture2D texture, int[] info) {
        try {
            TEXTURE_2D_INITIALIZE.invoke(texture, (Object)info);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot initialize the Texture2D facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Texture2D facade initialization failed", exception);
        }
    }

    public static Texture3D createUninitializedTexture3D(GraphicsDevice graphicsDevice) {
        try {
            return TEXTURE_3D.newInstance(graphicsDevice);
        } catch (InstantiationException | IllegalAccessException exception) {
            throw new IllegalStateException("Cannot construct the Texture3D facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Texture3D facade construction failed", exception);
        }
    }

    public static void initializeTexture3D(Texture3D texture, int[] info) {
        try {
            TEXTURE_3D_INITIALIZE.invoke(texture, (Object)info);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot initialize the Texture3D facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Texture3D facade initialization failed", exception);
        }
    }

    public static TextureCube createUninitializedTextureCube(GraphicsDevice graphicsDevice) {
        try {
            return TEXTURE_CUBE.newInstance(graphicsDevice);
        } catch (InstantiationException | IllegalAccessException exception) {
            throw new IllegalStateException("Cannot construct the TextureCube facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("TextureCube facade construction failed", exception);
        }
    }

    public static void initializeTextureCube(TextureCube texture, int[] info) {
        try {
            TEXTURE_CUBE_INITIALIZE.invoke(texture, (Object)info);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot initialize the TextureCube facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("TextureCube facade initialization failed", exception);
        }
    }

    public static SpriteFont createSpriteFont() {
        try {
            return SPRITE_FONT.newInstance();
        } catch (InstantiationException | IllegalAccessException exception) {
            throw new IllegalStateException("Cannot construct the SpriteFont facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("SpriteFont facade construction failed", exception);
        }
    }

    public static Model readModel(ContentReader input) {
        try {
            return (Model)MODEL_READ.invoke(null, input);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot invoke the hidden Model reader", exception);
        } catch (InvocationTargetException exception) {
            Throwable cause = exception.getCause();
            if (cause instanceof RuntimeException runtime) {
                throw runtime;
            }
            throw new IllegalStateException("Model reader failed", cause);
        }
    }

    public static void dispatchMediaPlayerEvent(int kind) {
        invokeStatic(MEDIA_PLAYER_DISPATCH, kind);
    }

    public static void releaseMediaPlayerState() {
        invokeStatic(MEDIA_PLAYER_RELEASE);
    }

    public static void setVisualizationData(
            VisualizationData data, float[] frequencies, float[] samples) {
        try {
            VISUALIZATION_SET.invoke(data, frequencies, samples);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot fill VisualizationData", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("VisualizationData update failed", exception);
        }
    }

    public static Video createVideo(GraphicsDevice graphicsDevice, String fileName,
            int durationMilliseconds, int width, int height, float framesPerSecond,
            int soundtrackType) {
        try {
            return VIDEO.newInstance(graphicsDevice, fileName, durationMilliseconds,
                    width, height, framesPerSecond, soundtrackType);
        } catch (InstantiationException | IllegalAccessException exception) {
            throw new IllegalStateException("Cannot construct the Video facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Video construction failed", exception);
        }
    }

    /**
     * Reads CNA's own identifier for a video soundtrack role.
     *
     * <p>The accessor is package-private in {@code Microsoft.Xna.Framework.Media} because XNA has
     * no such member and adding a public one would put a non-XNA member in an XNA type.
     */
    public static int videoSoundtrackValue(VideoSoundtrackType soundtrackType) {
        try {
            return (int) VIDEO_SOUNDTRACK_VALUE.invoke(soundtrackType);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException(
                    "Cannot read the hidden VideoSoundtrackType value", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("VideoSoundtrackType value read failed", exception);
        }
    }

    private static Method videoSoundtrackValueMethod() {
        try {
            Method method = VideoSoundtrackType.class.getDeclaredMethod("value");
            method.setAccessible(true);
            return method;
        } catch (NoSuchMethodException exception) {
            throw new IllegalStateException(
                    "VideoSoundtrackType has no hidden value accessor", exception);
        }
    }

    /**
     * Returns the native handle behind a discovered session.
     *
     * <p>The accessor is package-private in {@code Microsoft.Xna.Framework.Net} because XNA has
     * no such member. The handle is <strong>borrowed</strong>: the collection the search returned
     * owns it, and nothing outside that collection may release it.
     */
    public static long availableNetworkSessionHandle(AvailableNetworkSession session) {
        try {
            return (long) AVAILABLE_SESSION_HANDLE.invoke(session);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException(
                    "Cannot read the hidden AvailableNetworkSession handle", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("AvailableNetworkSession handle read failed", exception);
        }
    }

    /**
     * Wraps an owned discovered-session handle in the XNA type.
     *
     * <p>The constructor is package-private in {@code Microsoft.Xna.Framework.Net} because XNA
     * has none: a discovered session comes from a search. It takes ownership of the handle and
     * registers its release, so the caller must not release the handle itself.
     */
    public static AvailableNetworkSession createAvailableNetworkSession(long handle) {
        try {
            return AVAILABLE_SESSION.newInstance(handle);
        } catch (InstantiationException | IllegalAccessException exception) {
            throw new IllegalStateException(
                    "Cannot construct the AvailableNetworkSession facade", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("AvailableNetworkSession construction failed", exception);
        }
    }

    private static Constructor<AvailableNetworkSession> availableSessionConstructor() {
        try {
            Constructor<AvailableNetworkSession> constructor =
                    AvailableNetworkSession.class.getDeclaredConstructor(long.class);
            constructor.setAccessible(true);
            return constructor;
        } catch (NoSuchMethodException exception) {
            throw new IllegalStateException(
                    "AvailableNetworkSession has no hidden constructor", exception);
        }
    }

    /**
     * Returns the renderer's native handle, borrowed for the length of one call.
     *
     * <p>The accessor is package-private in {@code Microsoft.Xna.Framework.GamerServices}
     * because XNA has no such member. The renderer keeps ownership.
     */
    public static long avatarRendererHandle(AvatarRenderer renderer) {
        return hiddenHandleValue(AVATAR_RENDERER_HANDLE, "AvatarRenderer", renderer);
    }

    /**
     * Returns the animation's native handle, borrowed for the length of one call.
     *
     * <p>Package-private for the same reason, and the animation keeps ownership.
     */
    public static long avatarAnimationHandle(AvatarAnimation animation) {
        return hiddenHandleValue(AVATAR_ANIMATION_HANDLE, "AvatarAnimation", animation);
    }

    /**
     * Returns the gamer's native handle, borrowed for the length of one call.
     *
     * <p>The accessor is package-private in {@code Microsoft.Xna.Framework.GamerServices} because
     * XNA has no such member. The gamer keeps ownership.
     *
     * @param gamer the gamer to ask
     * @return its native handle
     */
    public static long signedInGamerHandle(
            Microsoft.Xna.Framework.GamerServices.SignedInGamer gamer) {
        return hiddenHandleValue(SIGNED_IN_GAMER_HANDLE, "SignedInGamer", gamer);
    }

    private static long hiddenHandleValue(Method accessor, String what, Object subject) {
        try {
            return (long) accessor.invoke(subject);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException(
                    "Cannot read the hidden " + what + " handle", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure(what + " handle read failed", exception);
        }
    }

    private static Method hiddenHandle(Class<?> owner) {
        try {
            Method method = owner.getDeclaredMethod("handle");
            method.setAccessible(true);
            return method;
        } catch (NoSuchMethodException exception) {
            throw new IllegalStateException(
                    owner.getSimpleName() + " has no hidden handle accessor", exception);
        }
    }

    private static Method availableSessionHandleMethod() {
        try {
            Method method = AvailableNetworkSession.class.getDeclaredMethod("handle");
            method.setAccessible(true);
            return method;
        } catch (NoSuchMethodException exception) {
            throw new IllegalStateException(
                    "AvailableNetworkSession has no hidden handle accessor", exception);
        }
    }

    private static Constructor<GraphicsDevice> graphicsDeviceConstructor() {
        try {
            Constructor<GraphicsDevice> constructor =
                    GraphicsDevice.class.getDeclaredConstructor(Game.class);
            if (!constructor.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the hidden GraphicsDevice constructor");
            }
            return constructor;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Constructor<Texture2D> texture2DConstructor() {
        try {
            Constructor<Texture2D> constructor =
                    Texture2D.class.getDeclaredConstructor(GraphicsDevice.class);
            if (!constructor.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the hidden Texture2D constructor");
            }
            return constructor;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method texture2DInitializeMethod() {
        try {
            Method method = Texture2D.class.getDeclaredMethod("initialize", int[].class);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to hidden Texture2D initialization");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Constructor<Texture3D> texture3DConstructor() {
        try {
            Constructor<Texture3D> constructor =
                    Texture3D.class.getDeclaredConstructor(GraphicsDevice.class);
            if (!constructor.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the hidden Texture3D constructor");
            }
            return constructor;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method texture3DInitializeMethod() {
        try {
            Method method = Texture3D.class.getDeclaredMethod("initialize", int[].class);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to hidden Texture3D initialization");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Constructor<TextureCube> textureCubeConstructor() {
        try {
            Constructor<TextureCube> constructor =
                    TextureCube.class.getDeclaredConstructor(GraphicsDevice.class);
            if (!constructor.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the hidden TextureCube constructor");
            }
            return constructor;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method textureCubeInitializeMethod() {
        try {
            Method method = TextureCube.class.getDeclaredMethod("initialize", int[].class);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to hidden TextureCube initialization");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    /**
     * Creates one of CNA's extended-layer effects as an ordinary XNA {@link Effect}.
     *
     * <p>Reached from {@code org.openeggbert.cna.extensions.graphics}. XNA declares no
     * constructor for an effect the runtime built, so widening one would put a member in the
     * strict contract that the reference API has no counterpart for.
     */
    public static Effect createExtensionEffect(GraphicsDevice graphicsDevice, int extensionEffect) {
        try {
            return (Effect) EFFECT_ADOPT_EXTENSION.invoke(null, graphicsDevice, extensionEffect);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot construct the extension Effect", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Extension Effect construction failed", exception);
        }
    }

    /**
     * Adopts an effect the extended layer already built as an ordinary XNA {@link Effect}.
     *
     * <p>Reached from {@code org.openeggbert.cna.extensions.graphics}, for a native effect that
     * exists already -- a shader-effect factory lends one, and disposing the facade gives the
     * lender its borrow back.
     */
    public static Effect createBorrowedEffect(GraphicsDevice graphicsDevice, long nativeEffect) {
        try {
            return (Effect) EFFECT_ADOPT_BORROWED.invoke(null, graphicsDevice, nativeEffect);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot construct the borrowed Effect", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Borrowed Effect construction failed", exception);
        }
    }

    /**
     * Adopts an effect CNA created and handed over, as an ordinary XNA {@link Effect}.
     *
     * <p>The same registration as {@link #createBorrowedEffect} and deliberately a different name,
     * because the ownership is the opposite: a shader effect built from source belongs to the
     * caller, and disposing the facade destroys it rather than giving a borrow back. The two share
     * a mechanism -- {@code cna_effect_destroy} is how both are released -- and nothing but the
     * name says which of the two a call site meant, so the name is what says it.
     *
     * @param graphicsDevice the device the effect belongs to
     * @param nativeEffect the handle CNA just returned
     * @return the facade, which the caller disposes
     */
    public static Effect createOwnedEffect(GraphicsDevice graphicsDevice, long nativeEffect) {
        return createBorrowedEffect(graphicsDevice, nativeEffect);
    }

    /**
     * Returns the graphics device a content manager loads against.
     *
     * <p>Reached from {@code org.openeggbert.cna.extensions.content}. XNA's ContentManager has no
     * public device accessor -- the CLR one reaches its service provider through internal access
     * -- so widening one would put a member in the strict contract the reference API has no
     * counterpart for.
     */
    public static GraphicsDevice contentManagerGraphicsDevice(ContentManager contentManager) {
        try {
            return (GraphicsDevice) CONTENT_MANAGER_DEVICE.invoke(contentManager);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException(
                    "Cannot reach the ContentManager graphics device", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("ContentManager graphics device lookup failed", exception);
        }
    }

    /**
     * Uploads one mip level's bytes to a texture exactly as a content format stored them.
     *
     * <p>Reached from {@code org.openeggbert.cna.extensions.content}. XNA declares only the
     * generic element-typed {@code SetData}, so widening a raw-byte overload would put a member
     * in the strict contract that the reference API has no counterpart for.
     */
    public static void setTexture2DLevelBytes(Texture2D texture, int level, byte[] bytes) {
        try {
            TEXTURE_2D_LEVEL_BYTES.invoke(texture, level, bytes);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot upload the texture level", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Texture level upload failed", exception);
        }
    }

    /**
     * Returns the CNA handle behind a storage container.
     *
     * <p>Reached only from the qualification that asks CNA whether it refuses a path escaping
     * the container, which the projection's own guard would otherwise hide.
     */
    public static long storageContainerHandle(StorageContainer container) {
        try {
            return (Long) STORAGE_CONTAINER_HANDLE.invoke(container);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot reach the container handle", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Container handle lookup failed", exception);
        }
    }

    private static Method storageContainerHandleMethod() {
        try {
            Method method = StorageContainer.class.getDeclaredMethod("requireOpen");
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the container handle");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method texture2DLevelBytesMethod() {
        try {
            Method method = Texture2D.class.getDeclaredMethod(
                    "setLevelBytes", int.class, byte[].class);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the raw texture level upload");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method contentManagerDeviceMethod() {
        try {
            Method method = ContentManager.class.getDeclaredMethod(
                    "graphicsDeviceForContentReader");
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the ContentManager graphics device");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method effectAdoptExtensionMethod() {
        try {
            Method method = Effect.class.getDeclaredMethod(
                    "adoptExtensionEffect", GraphicsDevice.class, int.class);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the hidden Effect extension factory");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method effectAdoptBorrowedMethod() {
        try {
            Method method = Effect.class.getDeclaredMethod(
                    "adoptBorrowedEffect", GraphicsDevice.class, long.class);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the hidden Effect adoption factory");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Constructor<SpriteFont> spriteFontConstructor() {
        try {
            Constructor<SpriteFont> constructor = SpriteFont.class.getDeclaredConstructor();
            if (!constructor.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to the hidden SpriteFont constructor");
            }
            return constructor;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method modelReadMethod() {
        try {
            Method method = Model.class.getDeclaredMethod("read", ContentReader.class);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException("The runtime denied access to the hidden Model reader");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method mediaPlayerMethod(String name, Class<?>... parameters) {
        try {
            Method method = MediaPlayer.class.getDeclaredMethod(name, parameters);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException("The runtime denied access to " + name);
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Method visualizationMethod() {
        try {
            Method method = VisualizationData.class.getDeclaredMethod(
                    "setNativeValues", float[].class, float[].class);
            if (!method.trySetAccessible()) {
                throw new IllegalStateException(
                        "The runtime denied access to VisualizationData storage");
            }
            return method;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static Constructor<Video> videoConstructor() {
        try {
            Constructor<Video> constructor = Video.class.getDeclaredConstructor(
                    GraphicsDevice.class, String.class, int.class, int.class, int.class,
                    float.class, int.class);
            if (!constructor.trySetAccessible()) {
                throw new IllegalStateException("The runtime denied access to the Video constructor");
            }
            return constructor;
        } catch (NoSuchMethodException exception) {
            throw new ExceptionInInitializerError(exception);
        }
    }

    private static void invokeStatic(Method method, Object... arguments) {
        try {
            method.invoke(null, arguments);
        } catch (IllegalAccessException exception) {
            throw new IllegalStateException("Cannot invoke hidden media bridge", exception);
        } catch (InvocationTargetException exception) {
            throw facadeFailure("Media bridge invocation failed", exception);
        }
    }

    private static IllegalStateException facadeFailure(
            String message, InvocationTargetException exception) {
        Throwable cause = exception.getCause();
        if (cause instanceof RuntimeException runtime) {
            throw runtime;
        }
        return new IllegalStateException(message, cause);
    }
}
