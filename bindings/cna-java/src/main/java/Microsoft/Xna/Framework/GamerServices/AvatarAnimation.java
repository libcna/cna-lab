package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Matrix;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.time.Duration;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/** One of XNA's built-in avatar animation clips, played through an {@link AvatarRenderer}. */
public class AvatarAnimation implements IAvatarAnimation, AutoCloseable {

    private final long handle;
    private boolean disposed;

    public AvatarAnimation(AvatarAnimationPreset animationPreset) {
        Objects.requireNonNull(animationPreset, "animationPreset");
        NativeGamerServices.requireAvailable("AvatarAnimation");
        long[] animation = new long[1];
        NativeGamerServices.check("AvatarAnimation",
                NativeGamerServicesRoutes.avatarAnimationCreate(
                        animationPreset.ordinal(), animation));
        handle = animation[0];
    }

    long handle() {
        return handle;
    }

    /**
     * Releases the animation.
     *
     * <p>This is XNA's protected lifetime hook. A subclass overrides it to release its own
     * resources and calls {@code super.Dispose(disposing)}.
     */
    protected void Dispose(boolean disposing) {
        if (disposing) {
            NativeGamerServices.check("AvatarAnimation.Dispose",
                    NativeGamerServicesRoutes.avatarAnimationDestroy(handle));
        }
    }

    public final void Dispose() {
        synchronized (this) {
            if (disposed) {
                return;
            }
            disposed = true;
        }
        Dispose(true);
    }

    @Override
    public final void close() {
        Dispose();
    }

    @Override
    public final void Update(Duration elapsedAnimationTime, boolean loop) {
        NativeGamerServices.check("AvatarAnimation.Update",
                NativeGamerServicesRoutes.avatarAnimationUpdate(handle,
                        NativeGamerServices.ticks(
                                Objects.requireNonNull(elapsedAnimationTime, "elapsedAnimationTime")),
                        loop));
    }

    /** Returns an unmodifiable snapshot: XNA's collection is read-only for the same reason. */
    @Override
    public final List<Matrix> getBoneTransforms() {
        int count = (int) info()[0];
        List<Matrix> transforms = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            float[] values = new float[16];
            NativeGamerServices.check("AvatarAnimation.BoneTransforms",
                    NativeGamerServicesRoutes.avatarAnimationGetBoneTransformAt(
                            handle, index, values));
            transforms.add(AvatarMatrices.of(values));
        }
        return Collections.unmodifiableList(transforms);
    }

    @Override
    public final Duration getCurrentPosition() {
        return NativeGamerServices.duration(info()[2]);
    }

    @Override
    public final void setCurrentPosition(Duration value) {
        NativeGamerServices.check("AvatarAnimation.CurrentPosition",
                NativeGamerServicesRoutes.avatarAnimationSetCurrentPosition(handle,
                        NativeGamerServices.ticks(Objects.requireNonNull(value, "value"))));
    }

    @Override
    public final AvatarExpression getExpression() {
        long[] values = new long[5];
        NativeGamerServices.check("AvatarAnimation.Expression",
                NativeGamerServicesRoutes.avatarAnimationGetExpression(handle, values));
        return AvatarMatrices.expression(values);
    }

    public final boolean getIsDisposed() {
        return disposed || info()[1] != 0L;
    }

    @Override
    public final Duration getLength() {
        return NativeGamerServices.duration(info()[3]);
    }

    private long[] info() {
        long[] values = new long[4];
        NativeGamerServices.check("AvatarAnimation",
                NativeGamerServicesRoutes.avatarAnimationGetInfo(handle, new byte[3], values));
        return values;
    }
}
