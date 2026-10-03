package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.util.List;

/**
 * What compiling one {@code .cnj} document produced.
 *
 * <p>The compiled file is the obvious part. The two lists are the part a build system needs.
 *
 * <p>{@link #getAbsorbedFiles()} names the sources whose contents are now <em>inside</em> the
 * {@code .cnb} and no longer need to ship -- the document itself first, then every sidecar it
 * pulled in. A build script watches those paths and rebuilds when one changes.
 * {@link #getExternalReferences()} names what the file still points at from outside, which is the
 * other half of the same question: what has to be shipped alongside it.
 *
 * <p>The paths are as the document wrote them, not resolved filesystem paths, so a script can
 * match them against what it generated -- including the authored subdirectory, which keeps
 * {@code art/ui/hero.png} and {@code art/world/hero.png} distinguishable.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op.
 */
public final class CnjResult implements AutoCloseable {

    private final long handle;
    private boolean closed;

    CnjResult(long handle) {
        this.handle = handle;
    }

    /** Returns the compiled {@code .cnb} file. */
    public byte[] getBytes() {
        long result = open();
        return CnbExtension.bytes("CnjResult.getBytes",
                (destination, count) -> NativeCnbRoutes
                        .cnbCnjResultCopyBytes(result, destination, count));
    }

    /** Returns the asset type written into the compiled file's header. */
    public CnbAssetType getAssetType() {
        int[] identifier = new int[1];
        CnbExtension.check("CnjResult.getAssetType",
                NativeCnbRoutes.cnbCnjResultGetAssetTypeId(open(), identifier));
        return new CnbAssetType(identifier[0]);
    }

    /**
     * Returns the compiler's own name for the asset type.
     *
     * <p>Worth asking rather than deriving: a {@code .cnj} may declare a type this build has no
     * constant for, and the name is what a diagnostic can say about it.
     */
    public String getAssetTypeName() {
        long result = open();
        return CnbExtension.text("CnjResult.getAssetTypeName",
                bytes -> NativeCnbRoutes.cnbCnjResultGetAssetTypeNameSize(result, bytes),
                (destination, bytes) -> NativeCnbRoutes
                        .cnbCnjResultCopyAssetTypeName(result, destination, bytes));
    }

    /**
     * Returns the source files whose contents are now inside the compiled file.
     *
     * @return the paths as the document wrote them, the document itself first
     */
    public List<String> getAbsorbedFiles() {
        long result = open();
        return CnbExtension.list("CnjResult.getAbsorbedFiles",
                count -> NativeCnbRoutes.cnbCnjResultGetAbsorbedFileCount(result, count),
                (index, bytes) -> NativeCnbRoutes
                        .cnbCnjResultGetAbsorbedFileSize(result, index, bytes),
                (index, destination, bytes) -> NativeCnbRoutes
                        .cnbCnjResultCopyAbsorbedFile(result, index, destination, bytes));
    }

    /** Returns what the compiled file still points at from outside itself. */
    public List<String> getExternalReferences() {
        long result = open();
        return CnbExtension.list("CnjResult.getExternalReferences",
                count -> NativeCnbRoutes.cnbCnjResultGetExternalReferenceCount(result, count),
                (index, bytes) -> NativeCnbRoutes
                        .cnbCnjResultGetExternalReferenceSize(result, index, bytes),
                (index, destination, bytes) -> NativeCnbRoutes
                        .cnbCnjResultCopyExternalReference(result, index, destination, bytes));
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
        CnbExtension.check("CnjResult.close", NativeCnbRoutes.cnbCnjResultDestroy(handle));
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnjResult is closed");
            }
        }
        return handle;
    }
}
