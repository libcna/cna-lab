package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.nio.file.Path;
import java.util.Objects;

/**
 * Compiles {@code .cnj} documents into {@code .cnb} files.
 *
 * <p>A {@code .cnj} is a small JSON document describing one asset -- a curve's keys, a sprite
 * font's glyphs and the bitmap they sit in, a model and the meshes it pulls in -- and this is the
 * step a build runs over it. It is the last piece of the ingest half: {@link CnbImport} reads the
 * formats artists hand over, and this reads the documents that say what to do with them.
 *
 * <p>All eight asset types compile: {@code Curve}, {@code AnimationClip}, {@code Model},
 * {@code Texture2D}, {@code Texture3D}, {@code TextureCube}, {@code SpriteFont} and
 * {@code SoundEffect}. A document naming any other type is refused <em>by name</em> rather than
 * quietly producing an empty file, which is the behaviour a build wants: a typo in a type is a
 * build error, not a shipped asset that loads to nothing.
 */
public final class Cnj {

    private Cnj() {
    }

    /**
     * Compiles one document into a {@code .cnb} file image.
     *
     * @param cnjPath the document to compile
     * @param contentRoot the directory sidecar references resolve against, or null for the
     *        document's own parent directory, which is where CNA's content tools write them
     * @param contentName the logical asset name to record, or null for the document's stem
     * @return the compiled result, which the caller closes
     * @throws CnbFormatException when the document is malformed, names an unknown type, or names
     *         something the compiler cannot express
     */
    public static CnjResult compile(Path cnjPath, Path contentRoot, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(cnjPath, "cnjPath");
        long[] result = new long[1];
        CnbExtension.check("Cnj.compile", NativeCnbRoutes.cnbCompileCnj(
                CnbExtension.utf8(cnjPath.toString()),
                CnbExtension.utf8(contentRoot == null ? "" : contentRoot.toString()),
                CnbExtension.utf8(contentName == null ? "" : contentName),
                result));
        return new CnjResult(result[0]);
    }

    /**
     * Compiles one document into a {@code .cnb} file image, with CNA's defaults.
     *
     * @param cnjPath the document to compile
     * @return the compiled result, which the caller closes
     */
    public static CnjResult compile(Path cnjPath) {
        return compile(cnjPath, null, null);
    }

    /**
     * Compiles a model document into a model description rather than a file.
     *
     * <p>For a build step that wants to look at or change the model before it is written. See
     * {@link CnjModelResult}.
     *
     * @param cnjPath the document to compile
     * @param contentRoot the root the document's relative paths resolve against. Unlike
     *        {@link #compile}, this one has no default: a model naming a sidecar with a null root
     *        is refused, because a relative path then has nothing to be relative to. Pass the
     *        document's own directory for the usual layout.
     * @return the compiled result, which the caller closes
     * @throws CnbFormatException when the document is malformed, is not a model, or names a
     *         sidecar that does not resolve inside the content root
     */
    public static CnjModelResult buildModel(Path cnjPath, Path contentRoot) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(cnjPath, "cnjPath");
        long[] result = new long[1];
        CnbExtension.check("Cnj.buildModel", NativeCnbRoutes.cnbBuildModelFromCnj(
                CnbExtension.utf8(cnjPath.toString()),
                CnbExtension.utf8(contentRoot == null ? "" : contentRoot.toString()),
                result));
        return new CnjModelResult(result[0]);
    }
}
