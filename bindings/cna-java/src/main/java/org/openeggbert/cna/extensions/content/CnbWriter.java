package org.openeggbert.cna.extensions.content;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.nio.file.Path;
import java.util.List;
import java.util.Objects;

/**
 * Builds a {@code .cnb} file.
 *
 * <p>A CNA extension, and the half a tool needs: a Java build step can produce CNA's own compiled
 * content without shelling out to another program. The bytes it produces are the format's, not an
 * approximation of it -- the table of contents, the per-chunk checksums and the compression are
 * all CNA's own writer, so what comes out is what CNA's reader expects.
 *
 * <p>The handle is owned; {@link #close()} releases it and closing twice is a no-op.
 */
public final class CnbWriter implements AutoCloseable {

    /** CNA's own result for a buffer that could not hold the answer. */
    private static final int RESULT_BUFFER_TOO_SMALL = 14;

    private final long handle;
    private boolean closed;

    private CnbWriter(long handle) {
        this.handle = handle;
    }

    /**
     * Starts a file of one asset type.
     *
     * @param assetType what the file will hold
     * @param assetSchemaVersion the version of that asset's own schema
     * @return the writer, which the caller closes
     */
    public static CnbWriter of(CnbAssetType assetType, int assetSchemaVersion) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(assetType, "assetType");
        long[] writer = new long[1];
        CnbExtension.check("CnbWriter.of",
                NativeCnbRoutes.cnbWriterCreate(assetType.Id(), assetSchemaVersion, writer));
        return new CnbWriter(writer[0]);
    }

    /**
     * Records what the asset is and where it came from.
     *
     * @param assetTypeName the asset's canonical type name
     * @param contentName the source content name it was built from
     */
    public void setMetadata(String assetTypeName, String contentName) {
        Objects.requireNonNull(assetTypeName, "assetTypeName");
        Objects.requireNonNull(contentName, "contentName");
        CnbExtension.check("CnbWriter.setMetadata", NativeCnbRoutes.cnbWriterSetMetadata(
                open(), CnbExtension.utf8(assetTypeName), CnbExtension.utf8(contentName)));
    }

    /** CNA's own smallest alignment: byte-aligned, which is what "no special alignment" means. */
    public static final int NO_ALIGNMENT = 1;

    /**
     * Adds one chunk with no special alignment.
     *
     * @param type the four-character identifier
     * @param data the chunk's payload; CNA copies it
     * @param mandatory whether a reader that does not know the identifier must refuse the file
     *     rather than skip the chunk
     */
    public void addChunk(CnbChunkId type, byte[] data, boolean mandatory) {
        addChunk(type, data, mandatory, NO_ALIGNMENT);
    }

    /**
     * Adds one chunk.
     *
     * @param type the four-character identifier
     * @param data the chunk's payload; CNA copies it
     * @param mandatory whether a reader that does not know the identifier must refuse the file
     *     rather than skip the chunk
     * @param alignment the alignment the chunk's offset must have: a power of two from
     *     {@link #NO_ALIGNMENT} through 4096, which is what a memory-mapped reader needs and what
     *     CNA refuses anything else for
     */
    public void addChunk(CnbChunkId type, byte[] data, boolean mandatory, int alignment) {
        Objects.requireNonNull(type, "type");
        Objects.requireNonNull(data, "data");
        if (alignment < 1 || alignment > 4096 || Integer.bitCount(alignment) != 1) {
            throw new IllegalArgumentException(
                    "chunk alignment must be a power of two from 1 through 4096; got "
                    + alignment);
        }
        CnbExtension.check("CnbWriter.addChunk", NativeCnbRoutes.cnbWriterAddChunk(
                open(), type.Value(), data, mandatory ? 1 : 0, alignment));
    }

    /** Returns how many chunks the writer holds so far. */
    public int getChunkCount() {
        long[] count = new long[1];
        CnbExtension.check("CnbWriter.getChunkCount",
                NativeCnbRoutes.cnbWriterGetSchemaChunkCount(open(), count));
        return (int) count[0];
    }

    /**
     * Chooses how chunks are stored.
     *
     * @param codec the codec, which {@link CnbCompression#isSupported()} says this build can use
     * @param level the codec's own level
     */
    public void setCompression(CnbCompression codec, int level) {
        Objects.requireNonNull(codec, "codec");
        CnbExtension.check("CnbWriter.setCompression",
                NativeCnbRoutes.cnbWriterSetCompression(open(), codec.ordinal(), level));
    }

    /** Records the limits a reader of this file should use. */
    public void setLimits(CnbReadLimits limits) {
        Objects.requireNonNull(limits, "limits");
        CnbExtension.check("CnbWriter.setLimits",
                NativeCnbRoutes.cnbWriterSetLimits(open(), limits.encode()));
    }

    /** Returns the limits this writer records. */
    public CnbReadLimits getLimits() {
        long[] values = new long[7];
        CnbExtension.check("CnbWriter.getLimits",
                NativeCnbRoutes.cnbWriterGetLimits(open(), values));
        return CnbReadLimits.decode(values);
    }

    /**
     * Replaces the whole external-reference table.
     *
     * <p>The {@code XREF} table names the assets a file refers to by logical name rather than
     * embedding, and the schema's own indices are positions in it -- so the order given here is
     * the order the file records, and a reference's index is where it sits in this list.
     *
     * <p>CNA validates each name when the file is assembled, with the same function its reader
     * applies. Sharing the rule is what stops a writer producing a file its own reader refuses.
     *
     * @param references the table; an empty list clears it
     */
    public void setExternalReferences(List<CnbExternalReference> references) {
        Objects.requireNonNull(references, "references");
        long handle = open();
        // Clear then append in order, which is CNA's own whole-table setter: it takes one entry
        // at a time because each carries a string, and C cannot express the table as one
        // argument.
        CnbExtension.check("CnbWriter.setExternalReferences",
                NativeCnbRoutes.cnbWriterClearExternalReferences(handle));
        for (CnbExternalReference reference : references) {
            Objects.requireNonNull(reference, "reference");
            CnbExtension.check("CnbWriter.setExternalReferences",
                    NativeCnbRoutes.cnbWriterAddExternalReference(handle,
                            new long[] {reference.Flags(), reference.ExpectedAssetType().Id()},
                            CnbExtension.utf8(reference.LogicalName())));
        }
    }

    /**
     * Embeds a texture under a label, so one asset can carry another's pixels.
     *
     * @param texture the texture to embed; CNA copies it
     * @param label the name a reader will ask for it by
     */
    public void appendEmbeddedTexture2D(CnbTextureData texture, String label) {
        Objects.requireNonNull(texture, "texture");
        Objects.requireNonNull(label, "label");
        CnbExtension.check("CnbWriter.appendEmbeddedTexture2D",
                NativeCnbRoutes.cnbWriterAppendEmbeddedTexture2d(
                        open(), texture.handle(), CnbExtension.utf8(label)));
    }

    /** Builds the whole file into a byte array. */
    public byte[] build() {
        long handle = open();
        long[] size = new long[1];
        // CNA reports the size it needs without writing, so the buffer is asked for rather than
        // guessed at; a partial write is never produced.
        int probe = NativeCnbRoutes.cnbWriterBuild(handle, new byte[0], size);
        if (probe != RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("CnbWriter.build", probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("CnbWriter.build",
                NativeCnbRoutes.cnbWriterBuild(handle, destination, written));
        if (written[0] == destination.length) {
            return destination;
        }
        byte[] exact = new byte[Math.toIntExact(written[0])];
        System.arraycopy(destination, 0, exact, 0, exact.length);
        return exact;
    }

    /**
     * Writes the whole file to disk.
     *
     * @param path where to write it
     */
    public void writeTo(Path path) {
        Objects.requireNonNull(path, "path");
        CnbExtension.check("CnbWriter.writeTo", NativeCnbRoutes
                .cnbWriterWriteToFile(open(), CnbExtension.utf8(path.toString())));
    }

    /** Releases the writer. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbWriter.close", NativeCnbRoutes.cnbWriterDestroy(handle));
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnbWriter is closed");
            }
        }
        return handle;
    }
}
