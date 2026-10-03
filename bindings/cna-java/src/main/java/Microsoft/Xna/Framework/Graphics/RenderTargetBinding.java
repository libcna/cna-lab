package Microsoft.Xna.Framework.Graphics;

import java.util.Objects;

/** Immutable Java value projection of one XNA render-target binding. */
public final class RenderTargetBinding {

    private final Texture renderTarget;
    private final CubeMapFace cubeMapFace;

    public RenderTargetBinding() {
        renderTarget = null;
        cubeMapFace = CubeMapFace.PositiveX;
    }

    public RenderTargetBinding(RenderTargetBinding value) {
        RenderTargetBinding snapshot = Objects.requireNonNull(value, "value");
        renderTarget = snapshot.renderTarget;
        cubeMapFace = snapshot.cubeMapFace;
    }

    public RenderTargetBinding(RenderTarget2D renderTarget) {
        this.renderTarget = Objects.requireNonNull(renderTarget, "renderTarget");
        cubeMapFace = CubeMapFace.PositiveX;
    }

    public RenderTargetBinding(RenderTargetCube renderTarget, CubeMapFace cubeMapFace) {
        this.renderTarget = Objects.requireNonNull(renderTarget, "renderTarget");
        this.cubeMapFace = Objects.requireNonNull(cubeMapFace, "cubeMapFace");
    }

    public static RenderTargetBinding fromRenderTarget2D(RenderTarget2D renderTarget) {
        return new RenderTargetBinding(renderTarget);
    }

    public CubeMapFace getCubeMapFace() {
        return cubeMapFace;
    }

    public Texture getRenderTarget() {
        return renderTarget;
    }
}
