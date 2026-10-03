package org.openeggbert.cna.extensions.graphics;

/**
 * Which shading language a renderer wants a source-built effect written in.
 *
 * <p>A CNA extension: XNA 4.0 loads compiled effect bytecode and never asks. A source-built
 * {@link ShaderEffect} is renderer-specific text, and the renderer's <em>name</em> is not a safe
 * way to guess which text -- it is wrong in a build carrying several renderers and meaningless
 * for one that picks its native API per process. The ordinals are CNA's own
 * {@code CNA_SHADER_DIALECT_*} values.
 */
public enum ShaderDialect {

    /**
     * The active renderer has not declared one.
     *
     * <p>Read this as "do not guess", not as "no shaders": it is the answer to refuse to build
     * sources from rather than one to fall back on.
     */
    Unknown,

    /** Desktop OpenGL GLSL, {@code #version 3xx core} or {@code 4xx core}. */
    GlslDesktop,

    /** OpenGL ES and WebGL GLSL, {@code #version 100} or {@code 300 es}. */
    GlslEs,

    /** GLSL compiled to SPIR-V, where {@code location}, {@code set} and {@code binding} are required. */
    GlslVulkan,

    /** Direct3D High Level Shader Language. */
    Hlsl,

    /** Metal Shading Language. */
    Msl,

    /** WebGPU Shading Language. */
    Wgsl;

    static ShaderDialect of(int value) {
        ShaderDialect[] all = values();
        if (value < 0 || value >= all.length) {
            throw new IllegalStateException("CNA reported shader dialect " + value
                    + ", which this ABI does not name");
        }
        return all[value];
    }
}
