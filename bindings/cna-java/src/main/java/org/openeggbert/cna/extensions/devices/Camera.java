package org.openeggbert.cna.extensions.devices;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import org.openeggbert.cna.internal.NativeBindings;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeDeviceExtensionRoutes;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * One of the host's cameras, delivering frames into a texture the game owns.
 *
 * <p>A CNA extension: XNA had no camera. {@link #getIsSupported()} and {@link #getCameras()} say
 * what the host has, {@link #Open()} opens the default camera, and {@link #TryAcquireFrame}
 * copies the newest frame into a {@link Texture2D} sized to {@link #getFrameWidth()} by
 * {@link #getFrameHeight()}. Having no frame ready, and a texture of the wrong size, are both an
 * ordinary {@code false} rather than a failure, as CNA documents.
 *
 * <p>{@link #OpenWithTestBackend()} opens a camera over CNA's own test backend instead, which
 * never touches the host: {@link #SetTestFrame} and {@link #SetTestState} decide what it
 * produces. It installs a process-wide override that closing the camera removes again.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op.
 */
public final class Camera implements AutoCloseable {

    /** One camera the host reports. */
    public record Info(String name, CameraPosition position) {
    }

    private final long handle;
    private boolean closed;

    private Camera(long handle) {
        this.handle = handle;
    }

    /** Reports whether the host has a camera service at all. */
    public static boolean getIsSupported() {
        boolean[] supported = new boolean[1];
        DeviceExtension.check("Camera.IsSupported",
                NativeDeviceExtensionRoutes.cameraGetIsSupportedExt(
                        DeviceExtension.game("Camera"), supported));
        return supported[0];
    }

    /** Returns the cameras the host reports, in its own order; empty when it has none. */
    public static List<Info> getCameras() {
        long game = DeviceExtension.game("Camera");
        long[] count = new long[1];
        DeviceExtension.check("Camera.Cameras",
                NativeDeviceExtensionRoutes.cameraGetCountExt(game, count));
        List<Info> cameras = new ArrayList<>();
        for (long index = 0; index < count[0]; index++) {
            final long at = index;
            String name = NativeGamerServices.text("Camera.Name",
                    out -> NativeDeviceExtensionRoutes.cameraGetNameSizeAtExt(game, at, out),
                    (buffer, out) -> NativeDeviceExtensionRoutes.cameraCopyNameAtExt(
                            game, at, buffer, out));
            long[] info = new long[1];
            DeviceExtension.check("Camera.Info",
                    NativeDeviceExtensionRoutes.cameraGetInfoAtExt(game, at, info));
            cameras.add(new Info(name, CameraPosition.fromValue(info[0])));
        }
        return Collections.unmodifiableList(cameras);
    }

    /** Opens the host's default camera. */
    public static Camera Open() {
        long[] created = new long[1];
        DeviceExtension.check("Camera.Open", NativeDeviceExtensionRoutes.cameraCreate(
                DeviceExtension.game("Camera"), created));
        return new Camera(created[0]);
    }

    /** Opens a camera over CNA's own test backend, which never touches the host. */
    public static Camera OpenWithTestBackend() {
        long[] created = new long[1];
        DeviceExtension.check("Camera.OpenWithTestBackend",
                NativeDeviceExtensionRoutes.cameraCreateWithTestBackendExt(
                        DeviceExtension.game("Camera"), created));
        return new Camera(created[0]);
    }

    public CameraState getState() {
        int[] state = new int[1];
        DeviceExtension.check("Camera.State",
                NativeDeviceExtensionRoutes.cameraGetStateExt(open(), state));
        return CameraState.fromValue(state[0]);
    }

    /** Returns the frame width in pixels, or zero before a frame format is known. */
    public int getFrameWidth() {
        int[] width = new int[1];
        DeviceExtension.check("Camera.FrameWidth",
                NativeDeviceExtensionRoutes.cameraGetFrameWidthExt(open(), width));
        return width[0];
    }

    /** Returns the frame height in pixels, or zero before a frame format is known. */
    public int getFrameHeight() {
        int[] height = new int[1];
        DeviceExtension.check("Camera.FrameHeight",
                NativeDeviceExtensionRoutes.cameraGetFrameHeightExt(open(), height));
        return height[0];
    }

    /**
     * Copies the newest frame into a texture the caller owns.
     *
     * @param texture a Color texture of exactly the frame's size
     * @return whether a frame was copied; {@code false} when none is ready or the size differs
     */
    public boolean TryAcquireFrame(Texture2D texture) {
        Objects.requireNonNull(texture, "texture");
        boolean[] acquired = new boolean[1];
        DeviceExtension.check("Camera.TryAcquireFrame",
                NativeDeviceExtensionRoutes.cameraTryAcquireFrameExt(open(),
                        NativeBindings.nativeResourceHandle(texture), acquired));
        return acquired[0];
    }

    /** Sets the frame a test-backend camera produces next. */
    public void SetTestFrame(int width, int height, Color[] pixels) {
        Objects.requireNonNull(pixels, "pixels");
        long[] leaves = new long[pixels.length * 4];
        for (int index = 0; index < pixels.length; index++) {
            Color pixel = Objects.requireNonNull(pixels[index], "pixel");
            leaves[index * 4] = pixel.getR();
            leaves[index * 4 + 1] = pixel.getG();
            leaves[index * 4 + 2] = pixel.getB();
            leaves[index * 4 + 3] = pixel.getA();
        }
        DeviceExtension.check("Camera.SetTestFrame",
                NativeDeviceExtensionRoutes.cameraSetTestFrameExt(open(), width, height, leaves));
    }

    /** Sets the state a test-backend camera reports. */
    public void SetTestState(CameraState state) {
        Objects.requireNonNull(state, "state");
        DeviceExtension.check("Camera.SetTestState",
                NativeDeviceExtensionRoutes.cameraSetTestStateExt(open(), state.ordinal()));
    }

    /** The default device information CNA fills in, which names no position. */
    static CameraPosition defaultPosition() {
        long[] info = new long[1];
        DeviceExtension.check("Camera.DefaultInfo",
                NativeDeviceExtensionRoutes.cameraDeviceInfoInit(info));
        return CameraPosition.fromValue(info[0]);
    }

    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        DeviceExtension.check("Camera.close", NativeDeviceExtensionRoutes.cameraDestroy(handle));
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("the camera is closed");
            }
        }
        return handle;
    }
}
