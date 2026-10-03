package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Vector3;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

/** Draws one avatar, posed either by an animation or by an explicit set of bone transforms. */
public class AvatarRenderer implements AutoCloseable {

    /** The number of bones in every avatar skeleton. */
    public static final int BoneCount = 71;

    private final long handle;
    private boolean disposed;

    public AvatarRenderer(AvatarDescription avatarDescription, boolean useLoadingEffect) {
        Objects.requireNonNull(avatarDescription, "avatarDescription");
        NativeGamerServices.requireAvailable("AvatarRenderer");
        long[] renderer = new long[1];
        NativeGamerServices.check("AvatarRenderer",
                NativeGamerServicesRoutes.avatarRendererCreate(
                        avatarDescription.handle(), useLoadingEffect, renderer));
        handle = renderer[0];
    }

    /**
     * Returns the renderer's native handle.
     *
     * <p>Package-private on purpose: XNA has no such member, and the handle is
     * <strong>borrowed</strong> -- this renderer owns it and releases it on Dispose.
     */
    long handle() {
        return handle;
    }

    public AvatarRenderer(AvatarDescription avatarDescription) {
        this(avatarDescription, true);
    }

    /** XNA's protected lifetime hook; a subclass overrides it and calls {@code super}. */
    protected void Dispose(boolean disposing) {
        if (disposing) {
            NativeGamerServices.check("AvatarRenderer.Dispose",
                    NativeGamerServicesRoutes.avatarRendererDestroy(handle));
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

    /**
     * Draws the avatar posed by an animation.
     *
     * <p>An {@link AvatarAnimation} is drawn through its native clip. Any other
     * {@link IAvatarAnimation} a title implements is drawn through its bone transforms and
     * expression, which is the same data XNA reads from it.
     */
    public final void Draw(IAvatarAnimation animation) {
        Objects.requireNonNull(animation, "animation");
        if (animation instanceof AvatarAnimation native_) {
            NativeGamerServices.check("AvatarRenderer.Draw",
                    NativeGamerServicesRoutes.avatarRendererDrawAnimation(
                            handle, native_.handle()));
            return;
        }
        Draw(animation.getBoneTransforms(), animation.getExpression());
    }

    public final void Draw(List<Matrix> bones, AvatarExpression expression) {
        Objects.requireNonNull(bones, "bones");
        Objects.requireNonNull(expression, "expression");
        float[] values = new float[bones.size() * 16];
        for (int index = 0; index < bones.size(); index++) {
            System.arraycopy(AvatarMatrices.values(bones.get(index)), 0, values, index * 16, 16);
        }
        NativeGamerServices.check("AvatarRenderer.Draw",
                NativeGamerServicesRoutes.avatarRendererDrawBones(handle, values,
                        AvatarMatrices.values(expression)));
    }

    public final Vector3 getAmbientLightColor() {
        return lighting(2);
    }

    public final void setAmbientLightColor(Vector3 value) {
        setLighting(getLightColor(), getLightDirection(), value);
    }

    /** Returns an unmodifiable snapshot of the avatar's bind pose. */
    public final List<Matrix> getBindPose() {
        List<Matrix> pose = new ArrayList<>(BoneCount);
        for (int index = 0; index < BoneCount; index++) {
            float[] values = new float[16];
            NativeGamerServices.check("AvatarRenderer.BindPose",
                    NativeGamerServicesRoutes.avatarRendererGetBindPoseAt(handle, index, values));
            pose.add(AvatarMatrices.of(values));
        }
        return Collections.unmodifiableList(pose);
    }

    public final boolean getIsDisposed() {
        if (disposed) {
            return true;
        }
        return info()[1] != 0L;
    }

    public final Vector3 getLightColor() {
        return lighting(0);
    }

    public final void setLightColor(Vector3 value) {
        setLighting(value, getLightDirection(), getAmbientLightColor());
    }

    public final Vector3 getLightDirection() {
        return lighting(1);
    }

    public final void setLightDirection(Vector3 value) {
        setLighting(getLightColor(), value, getAmbientLightColor());
    }

    /** Returns an unmodifiable snapshot of each bone's parent index. */
    public final List<Integer> getParentBones() {
        List<Integer> parents = new ArrayList<>(BoneCount);
        for (int index = 0; index < BoneCount; index++) {
            int[] parent = new int[1];
            NativeGamerServices.check("AvatarRenderer.ParentBones",
                    NativeGamerServicesRoutes.avatarRendererGetParentBoneAt(
                            handle, index, parent));
            parents.add(parent[0]);
        }
        return Collections.unmodifiableList(parents);
    }

    public final Matrix getProjection() {
        return transform(2);
    }

    public final void setProjection(Matrix value) {
        setTransforms(getWorld(), getView(), value);
    }

    public final AvatarRendererState getState() {
        return AvatarRendererState.values()[(int) info()[0]];
    }

    public final Matrix getView() {
        return transform(1);
    }

    public final void setView(Matrix value) {
        setTransforms(getWorld(), value, getProjection());
    }

    public final Matrix getWorld() {
        return transform(0);
    }

    public final void setWorld(Matrix value) {
        setTransforms(value, getView(), getProjection());
    }

    private Vector3 lighting(int which) {
        float[] color = new float[3];
        float[] direction = new float[3];
        float[] ambient = new float[3];
        NativeGamerServices.check("AvatarRenderer.Lighting",
                NativeGamerServicesRoutes.avatarRendererGetLighting(
                        handle, color, direction, ambient));
        float[] chosen = switch (which) {
            case 0 -> color;
            case 1 -> direction;
            default -> ambient;
        };
        return new Vector3(chosen[0], chosen[1], chosen[2]);
    }

    private void setLighting(Vector3 color, Vector3 direction, Vector3 ambient) {
        NativeGamerServices.check("AvatarRenderer.Lighting",
                NativeGamerServicesRoutes.avatarRendererSetLighting(handle,
                        new float[] {color.X, color.Y, color.Z},
                        new float[] {direction.X, direction.Y, direction.Z},
                        new float[] {ambient.X, ambient.Y, ambient.Z}));
    }

    private Matrix transform(int which) {
        float[] world = new float[16];
        float[] view = new float[16];
        float[] projection = new float[16];
        NativeGamerServices.check("AvatarRenderer.Transforms",
                NativeGamerServicesRoutes.avatarRendererGetTransforms(
                        handle, world, view, projection));
        return AvatarMatrices.of(switch (which) {
            case 0 -> world;
            case 1 -> view;
            default -> projection;
        });
    }

    private void setTransforms(Matrix world, Matrix view, Matrix projection) {
        NativeGamerServices.check("AvatarRenderer.Transforms",
                NativeGamerServicesRoutes.avatarRendererSetTransforms(handle,
                        AvatarMatrices.values(Objects.requireNonNull(world, "world")),
                        AvatarMatrices.values(Objects.requireNonNull(view, "view")),
                        AvatarMatrices.values(Objects.requireNonNull(projection, "projection"))));
    }

    private long[] info() {
        long[] values = new long[3];
        NativeGamerServices.check("AvatarRenderer",
                NativeGamerServicesRoutes.avatarRendererGetInfo(handle, new byte[2], values));
        return values;
    }
}
