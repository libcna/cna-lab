package org.openeggbert.cna.extensions.graphics;

/**
 * One thing a renderer can or cannot do.
 *
 * <p>A CNA extension. XNA answers a fixed set of these through {@code GraphicsProfile}, which
 * describes a <em>tier</em> rather than a renderer: Reach and HiDef were chosen for the hardware
 * of 2010, and a game asking whether it may dispatch a compute shader today has nowhere in XNA to
 * ask. CNA answers each capability separately, and this is that question.
 *
 * <p>The constant order is CNA's own, because the C API identifies a capability by ordinal, and
 * {@link RendererCapabilities} is where they are asked.
 */
public enum GraphicsCapability {

    /** Three-dimensional rendering at all, as opposed to a 2D-only backend. */
    ThreeD,

    /** A depth/stencil buffer. */
    DepthStencilBuffer,

    /** Multisample anti-aliasing. */
    MultiSampleAntiAliasing,

    /** More than one render target bound at once. */
    MultipleRenderTargets,

    /** Anisotropic texture filtering. */
    AnisotropicFiltering,

    /** Wireframe fill mode. */
    WireFrame,

    /** Occlusion queries. */
    OcclusionQuery,

    /** Effects built from source the renderer compiles itself. */
    CustomEffects,

    /** Volume textures. */
    Texture3D,

    /** More than one vertex stream. */
    MultiStreamVertexInput,

    /** Hardware instancing. */
    Instancing,

    /** A stencil buffer. */
    StencilBuffer,

    /** Additive blending. */
    AdditiveBlending,

    /** Effects loaded from a compiled {@code .xnb} blob, which is a different question from
     * {@link #CustomEffects}: one is about a compiler, the other about a bytecode format. */
    CompiledEffects,

    /** Full-precision floating-point render targets. */
    FloatRenderTargets,

    /** Half-precision floating-point render targets. */
    HalfFloatRenderTargets,

    /** Linear filtering of a half-float texture, which some renderers allow the format but not
     * the filtering of. */
    HalfFloatTextureLinearFiltering,

    /**
     * Compute shaders.
     *
     * <p>Still answered by every renderer, although CNA ABI 0.30 removed the compute objects it
     * used to gate.
     */
    ComputeShaders,

    /**
     * Draws whose arguments the GPU reads from a buffer rather than the CPU passing them. Still
     * answered by every renderer, although CNA ABI 0.30 removed the indirect draw routes.
     */
    IndirectDraw;

    int toValue() {
        return ordinal();
    }
}
