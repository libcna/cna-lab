package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.List;

/**
 * What compiling one {@code .cnj} model produced, before it becomes a file.
 *
 * <p>The difference from {@link CnjResult} is where the compile stops. That one produces the
 * {@code .cnb} bytes; this one produces the model description itself, so a build step can inspect
 * or change it -- strip a mesh, retarget a material, check a bone count against a budget -- and
 * then encode it with {@link Cnb#encodeModel}.
 *
 * <p>{@link #takeModel()} <strong>moves</strong> the model out rather than lending it, so this
 * result can be closed immediately afterwards and a caller cannot end up holding a model that a
 * released result had been keeping alive. Taking it twice fails.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op.
 */
public final class CnjModelResult implements AutoCloseable {

    private final long handle;
    private boolean closed;

    CnjModelResult(long handle) {
        this.handle = handle;
    }

    /**
     * Moves the compiled model into its own handle.
     *
     * @return the model data, which the caller closes
     */
    public CnbModelData takeModel() {
        long[] model = new long[1];
        CnbExtension.check("CnjModelResult.takeModel",
                NativeCnbRoutes.cnbModelFromCnjTakeModel(open(), model));
        return new CnbModelData(model[0]);
    }

    /**
     * Returns the source files whose contents the compile absorbed.
     *
     * <p>Not the same list {@link CnjResult#getAbsorbedFiles()} gives. That one always names the
     * document first, because a file was written and the document went into it. Nothing is
     * written here, so the list holds only the sidecars the model actually pulled in, and a model
     * with none reports none.
     */
    public List<String> getAbsorbedFiles() {
        long result = open();
        return CnbExtension.list("CnjModelResult.getAbsorbedFiles",
                count -> NativeCnbRoutes.cnbModelFromCnjGetAbsorbedFileCount(result, count),
                (index, bytes) -> NativeCnbRoutes
                        .cnbModelFromCnjGetAbsorbedFileSize(result, index, bytes),
                (index, destination, bytes) -> NativeCnbRoutes
                        .cnbModelFromCnjCopyAbsorbedFile(result, index, destination, bytes));
    }

    /** Returns what the compiled model still points at from outside itself. */
    public List<String> getExternalReferences() {
        long result = open();
        return CnbExtension.list("CnjModelResult.getExternalReferences",
                count -> NativeCnbRoutes
                        .cnbModelFromCnjGetExternalReferenceCount(result, count),
                (index, bytes) -> NativeCnbRoutes
                        .cnbModelFromCnjGetExternalReferenceSize(result, index, bytes),
                (index, destination, bytes) -> NativeCnbRoutes
                        .cnbModelFromCnjCopyExternalReference(result, index, destination, bytes));
    }

    /** Releases the result. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnjModelResult.close",
                NativeCnbRoutes.cnbModelFromCnjDestroy(handle));
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnjModelResult is closed");
            }
        }
        return handle;
    }
}
