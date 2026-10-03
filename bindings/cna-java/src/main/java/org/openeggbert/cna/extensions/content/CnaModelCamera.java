package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Matrix;

/**
 * A camera authored into a model's scene.
 *
 * <p>A CNA extension with no XNA counterpart at all: XNA's {@code Model} carries geometry and
 * bones and nothing else, so a camera the artist placed was lost at import.
 *
 * @param Name the camera's name, empty when the asset gave it none
 * @param SceneNodeIndex the scene node the camera hangs from
 * @param Projection the camera's projection matrix
 * @param WorldTransform where the camera sits in the scene
 * @param IsPerspective whether the projection is perspective rather than orthographic
 * @param AspectRatio the aspect ratio, meaningful when {@code HasAuthoredAspectRatio}
 * @param FieldOfView the vertical field of view in radians, for a perspective camera
 * @param NearPlaneDistance the near clip distance
 * @param FarPlaneDistance the far clip distance, meaningless when {@code HasInfiniteFarPlane}
 * @param HasInfiniteFarPlane whether the camera's far plane is at infinity
 * @param HasAuthoredAspectRatio whether the asset stated an aspect ratio rather than leaving it
 *     to the viewport
 */
public record CnaModelCamera(
        String Name,
        long SceneNodeIndex,
        Matrix Projection,
        Matrix WorldTransform,
        boolean IsPerspective,
        float AspectRatio,
        float FieldOfView,
        float NearPlaneDistance,
        float FarPlaneDistance,
        boolean HasInfiniteFarPlane,
        boolean HasAuthoredAspectRatio) {

    /**
     * Builds a perspective projection with no far plane, the way CNA does.
     *
     * <p>A CNA extension: every XNA perspective factory takes a far plane, and a glTF camera may
     * declare none -- clamping one in would move geometry the source meant to stay visible. The
     * matrix comes from CNA rather than from four lines here, so a camera authored with an
     * infinite far plane is projected with exactly the bits CNA would have used.
     *
     * @param fieldOfView the vertical field of view in radians
     * @param aspectRatio width divided by height
     * @param nearPlaneDistance the near clip distance
     * @return the projection
     */
    public static Matrix CreateInfinitePerspectiveFieldOfView(float fieldOfView,
            float aspectRatio, float nearPlaneDistance) {
        CnbExtension.requireAvailable();
        float[] leaves = new float[16];
        CnbExtension.check("CnaModelCamera.CreateInfinitePerspectiveFieldOfView",
                org.openeggbert.cna.internal.generated.NativeModelExtensionRoutes
                        .matrixCreateInfinitePerspectiveFieldOfViewExt(fieldOfView, aspectRatio,
                                nearPlaneDistance, leaves));
        return CnaSkeleton.matricesOf(leaves, 1).get(0);
    }

    /** The four integral leaves CNA's descriptor declares, in order. */
    long[] toIntegralLeaves() {
        return new long[] {SceneNodeIndex, IsPerspective ? 1L : 0L,
                HasInfiniteFarPlane ? 1L : 0L, HasAuthoredAspectRatio ? 1L : 0L};
    }

    /** The thirty-six floating leaves: two matrices, then the four camera scalars. */
    float[] toFloatingLeaves() {
        float[] leaves = new float[36];
        System.arraycopy(CnaSkeleton.matrices(java.util.List.of(Projection)), 0, leaves, 0, 16);
        System.arraycopy(CnaSkeleton.matrices(java.util.List.of(WorldTransform)), 0, leaves,
                16, 16);
        leaves[32] = AspectRatio;
        leaves[33] = FieldOfView;
        leaves[34] = NearPlaneDistance;
        leaves[35] = FarPlaneDistance;
        return leaves;
    }
}
