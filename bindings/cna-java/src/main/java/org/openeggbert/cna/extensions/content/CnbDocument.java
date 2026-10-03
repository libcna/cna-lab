package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Curve;
import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * One parsed {@code .cnb} file.
 *
 * <p>A CNA extension, and deliberately not XNA's {@code .xnb}. The two are different formats with
 * different identities: {@code .xnb} is Microsoft's and {@code ContentManager.Load} reads it,
 * while {@code .cnb} is CNA's own, with a table of contents, per-chunk checksums and per-chunk
 * compression that {@code .xnb} has no equivalent for. Nothing here pretends one is the other.
 *
 * <p>Parsing is <strong>checked before it allocates</strong>. {@link CnbReadLimits} travels with
 * the document, so a header claiming an impossible chunk count or a chunk larger than the file is
 * refused at the header rather than after the allocation.
 *
 * <p>The handle is owned and closing twice is a no-op. Chunk payloads come back as fresh Java
 * arrays, so what a caller reads outlives the document.
 */
public final class CnbDocument implements AutoCloseable {

    private final long handle;
    private boolean closed;

    private CnbDocument(long handle) {
        this.handle = handle;
    }

    /** The native document, for the decoders in this package that read one whole asset. */
    long handle() {
        return open();
    }

    /**
     * Reports whether some bytes begin with the {@code .cnb} magic.
     *
     * <p>Cheap enough to use as a guard before deciding which reader a file belongs to, and it
     * allocates nothing.
     *
     * @param bytes the candidate bytes; only the first few are looked at
     * @return whether the magic matches
     */
    public static boolean hasMagic(byte[] bytes) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(bytes, "bytes");
        boolean[] present = new boolean[1];
        CnbExtension.check("CnbDocument.hasMagic",
                NativeCnbRoutes.cnbHasMagic(bytes, present));
        return present[0];
    }

    /** Returns the four magic bytes every {@code .cnb} file starts with. */
    public static byte[] magic() {
        CnbExtension.requireAvailable();
        byte[] destination = new byte[4];
        long[] written = new long[1];
        CnbExtension.check("CnbDocument.magic",
                NativeCnbRoutes.cnbCopyFormatMagic(destination, written));
        return destination;
    }

    /**
     * Parses bytes already in memory.
     *
     * @param bytes the whole file; CNA copies what it keeps
     * @param origin what to call the file in a diagnostic
     * @param limits what the parse refuses
     * @return the document, which the caller closes
     */
    public static CnbDocument parse(byte[] bytes, String origin, CnbReadLimits limits) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(bytes, "bytes");
        Objects.requireNonNull(origin, "origin");
        Objects.requireNonNull(limits, "limits");
        long[] document = new long[1];
        CnbExtension.check("CnbDocument.parse", NativeCnbRoutes.cnbDocumentParse(
                bytes, CnbExtension.utf8(origin), limits.encode(), document));
        return new CnbDocument(document[0]);
    }

    /**
     * Parses a file from disk.
     *
     * @param path the file to read
     * @param limits what the parse refuses
     * @return the document, which the caller closes
     */
    public static CnbDocument parse(Path path, CnbReadLimits limits) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(path, "path");
        Objects.requireNonNull(limits, "limits");
        long[] document = new long[1];
        CnbExtension.check("CnbDocument.parse", NativeCnbRoutes.cnbDocumentParseFile(
                CnbExtension.utf8(path.toString()), limits.encode(), document));
        return new CnbDocument(document[0]);
    }

    /** Returns what the parse was told to call this file in a diagnostic. */
    public String getOrigin() {
        long handle = open();
        return CnbExtension.text("CnbDocument.getOrigin",
                bytes -> NativeCnbRoutes.cnbDocumentGetOriginSize(handle, bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbDocumentCopyOrigin(handle, destination, bytes));
    }

    /** Returns the container format's major version. */
    public int getContainerMajor() {
        int[] major = new int[1];
        CnbExtension.check("CnbDocument.getContainerMajor",
                NativeCnbRoutes.cnbDocumentGetContainerMajor(open(), major));
        return major[0];
    }

    /** Returns the container format's minor version. */
    public int getContainerMinor() {
        int[] minor = new int[1];
        CnbExtension.check("CnbDocument.getContainerMinor",
                NativeCnbRoutes.cnbDocumentGetContainerMinor(open(), minor));
        return minor[0];
    }

    /** Returns what kind of asset the file holds. */
    public CnbAssetType getAssetType() {
        int[] id = new int[1];
        CnbExtension.check("CnbDocument.getAssetType",
                NativeCnbRoutes.cnbDocumentGetAssetTypeId(open(), id));
        return new CnbAssetType(id[0]);
    }

    /** Returns the version of the asset's own schema, which is separate from the container's. */
    public int getAssetSchemaVersion() {
        int[] version = new int[1];
        CnbExtension.check("CnbDocument.getAssetSchemaVersion",
                NativeCnbRoutes.cnbDocumentGetAssetSchemaVersion(open(), version));
        return version[0];
    }

    /** Returns the limits this document was parsed under. */
    public CnbReadLimits getLimits() {
        long[] values = new long[7];
        CnbExtension.check("CnbDocument.getLimits",
                NativeCnbRoutes.cnbDocumentGetLimits(open(), values));
        return CnbReadLimits.decode(values);
    }

    /**
     * Requires the file to be the asset a caller expects.
     *
     * <p>This is the check a loader makes first: a texture loader handed a model must say so
     * rather than misread it, and a schema version newer than the caller understands is refused
     * rather than guessed at.
     *
     * @param expected the asset type the caller can read
     * @param maximumSchemaVersion the newest schema version the caller understands
     */
    public void requireAsset(CnbAssetType expected, int maximumSchemaVersion) {
        Objects.requireNonNull(expected, "expected");
        CnbExtension.check("CnbDocument.requireAsset", NativeCnbRoutes
                .cnbDocumentRequireAsset(open(), expected.Id(), maximumSchemaVersion));
    }

    /** Returns every chunk the table of contents declares, in file order. */
    public List<CnbChunk> getChunks() {
        long handle = open();
        long[] count = new long[1];
        CnbExtension.check("CnbDocument.getChunks",
                NativeCnbRoutes.cnbDocumentGetChunkCount(handle, count));
        List<CnbChunk> chunks = new ArrayList<>((int) count[0]);
        for (long index = 0; index < count[0]; index++) {
            long[] entry = new long[9];
            CnbExtension.check("CnbDocument.getChunks",
                    NativeCnbRoutes.cnbDocumentGetChunk(handle, index, entry));
            // What "mandatory" means is a wire-format rule, so CNA is asked rather than the
            // flag bit being decoded here: a rule restated in Java is a rule that can drift.
            boolean[] mandatory = new boolean[1];
            CnbExtension.check("CnbDocument.getChunks",
                    NativeCnbRoutes.cnbChunkEntryIsMandatory(entry, mandatory));
            chunks.add(new CnbChunk(
                    new CnbChunkId((int) entry[3]),
                    mandatory[0],
                    entry[0], entry[1], entry[2],
                    entry[5] & 0xFFFFFFFFL,
                    CnbCompression.fromValue(entry[6]),
                    entry[7]));
        }
        return List.copyOf(chunks);
    }

    /**
     * Returns one chunk's payload, decompressed if it was stored compressed.
     *
     * @param index the chunk's position in {@link #getChunks()}
     * @return a fresh copy of the chunk's bytes
     */
    public byte[] readChunk(int index) {
        long handle = open();
        List<CnbChunk> chunks = getChunks();
        if (index < 0 || index >= chunks.size()) {
            throw new IndexOutOfBoundsException("chunk index " + index);
        }
        byte[] destination = new byte[Math.toIntExact(chunks.get(index).UncompressedSize())];
        long[] written = new long[1];
        CnbExtension.check("CnbDocument.readChunk", NativeCnbRoutes
                .cnbDocumentCopyChunkData(handle, index, destination, written));
        return destination;
    }

    /**
     * Opens a checked cursor over one chunk.
     *
     * @param index the chunk's position in {@link #getChunks()}
     * @return the reader, which the caller closes before this document
     */
    public CnbReader openChunk(int index) {
        long[] reader = new long[1];
        CnbExtension.check("CnbDocument.openChunk",
                NativeCnbRoutes.cnbDocumentOpenChunk(open(), index, reader));
        return new CnbReader(reader[0]);
    }

    /**
     * Returns the positions of every chunk of one kind.
     *
     * @param type the identifier to look for
     * @return the indices, in file order
     */
    public List<Integer> findAll(CnbChunkId type) {
        Objects.requireNonNull(type, "type");
        long handle = open();
        long[] count = new long[1];
        CnbExtension.check("CnbDocument.findAll",
                NativeCnbRoutes.cnbDocumentGetChunkCount(handle, count));
        long[] destination = new long[(int) count[0]];
        long[] found = new long[1];
        CnbExtension.check("CnbDocument.findAll", NativeCnbRoutes
                .cnbDocumentFindAll(handle, type.Value(), destination, found));
        List<Integer> indices = new ArrayList<>((int) found[0]);
        for (int index = 0; index < found[0]; index++) {
            indices.add((int) destination[index]);
        }
        return List.copyOf(indices);
    }

    /**
     * Returns the position of the one chunk of a kind.
     *
     * @param type the identifier to look for
     * @return the index, or -1 when the file has none
     * @throws CnbFormatException when the file has more than one, which a single-chunk schema
     *     forbids
     */
    public int findSingle(CnbChunkId type) {
        Objects.requireNonNull(type, "type");
        boolean[] found = new boolean[1];
        long[] index = new long[1];
        CnbExtension.check("CnbDocument.findSingle",
                NativeCnbRoutes.cnbDocumentFindSingle(open(), type.Value(), found, index));
        return found[0] ? (int) index[0] : -1;
    }

    /**
     * Returns the position of a chunk the caller requires.
     *
     * @param type the identifier that must be present exactly once
     * @return the index
     * @throws CnbFormatException when it is absent or repeated
     */
    public int requireSingle(CnbChunkId type) {
        Objects.requireNonNull(type, "type");
        long[] index = new long[1];
        CnbExtension.check("CnbDocument.requireSingle",
                NativeCnbRoutes.cnbDocumentRequireSingle(open(), type.Value(), index));
        return (int) index[0];
    }

    /**
     * Requires every mandatory chunk to be one of the kinds the caller understands.
     *
     * <p>This is the forward-compatibility rule the format is built around: a writer marks a
     * chunk mandatory when ignoring it would change what the asset means, so a reader that does
     * not know the identifier must refuse the file rather than quietly load half of it.
     *
     * @param knownTypes every identifier the caller can handle
     */
    public void requireMandatoryChunksUnderstood(List<CnbChunkId> knownTypes) {
        Objects.requireNonNull(knownTypes, "knownTypes");
        int[] types = new int[knownTypes.size()];
        for (int index = 0; index < types.length; index++) {
            types[index] = Objects.requireNonNull(knownTypes.get(index), "type").Value();
        }
        CnbExtension.check("CnbDocument.requireMandatoryChunksUnderstood", NativeCnbRoutes
                .cnbDocumentRequireMandatoryChunksUnderstood(open(), types));
    }

    /** Returns what the optional metadata chunk says, or an absent one when the file has none. */
    public CnbMetadata getMetadata() {
        long handle = open();
        byte[] reserved = new byte[3];
        long[] values = new long[2];
        CnbExtension.check("CnbDocument.getMetadata",
                NativeCnbRoutes.cnbDocumentGetMetadata(handle, reserved, values));
        boolean present = values[0] != 0L;
        if (!present) {
            return new CnbMetadata(false, "", "", (int) values[1]);
        }
        String assetTypeName = CnbExtension.text("CnbDocument.getMetadata",
                bytes -> NativeCnbRoutes
                        .cnbDocumentGetMetadataAssetTypeNameSize(handle, bytes),
                (destination, bytes) -> NativeCnbRoutes
                        .cnbDocumentCopyMetadataAssetTypeName(handle, destination, bytes));
        String contentName = CnbExtension.text("CnbDocument.getMetadata",
                bytes -> NativeCnbRoutes
                        .cnbDocumentGetMetadataContentNameSize(handle, bytes),
                (destination, bytes) -> NativeCnbRoutes
                        .cnbDocumentCopyMetadataContentName(handle, destination, bytes));
        return new CnbMetadata(true, assetTypeName, contentName, (int) values[1]);
    }

    /** Returns every asset this file refers to by logical name rather than embedding. */
    public List<CnbExternalReference> getExternalReferences() {
        long handle = open();
        long[] count = new long[1];
        CnbExtension.check("CnbDocument.getExternalReferences",
                NativeCnbRoutes.cnbDocumentGetExternalReferenceCount(handle, count));
        List<CnbExternalReference> references = new ArrayList<>((int) count[0]);
        byte[] diagnostics = "external reference".getBytes(StandardCharsets.UTF_8);
        for (long index = 0; index < count[0]; index++) {
            long[] values = new long[2];
            CnbExtension.check("CnbDocument.getExternalReferences", NativeCnbRoutes
                    .cnbDocumentGetExternalReference(handle, index, diagnostics, values));
            long position = index;
            String name = CnbExtension.text("CnbDocument.getExternalReferences",
                    bytes -> NativeCnbRoutes
                            .cnbDocumentGetExternalReferenceNameSize(handle, position, bytes),
                    (destination, bytes) -> NativeCnbRoutes
                            .cnbDocumentCopyExternalReferenceName(
                                    handle, position, destination, bytes));
            references.add(new CnbExternalReference(
                    name, new CnbAssetType((int) values[1]), (int) values[0]));
        }
        return List.copyOf(references);
    }

    /**
     * Reads the texture a {@code .cnb} file embeds under one label.
     *
     * @param label the label the writer gave the embedded texture
     * @return the texture data, which the caller closes
     */
    public CnbTextureData readEmbeddedTexture2D(String label) {
        Objects.requireNonNull(label, "label");
        long[] texture = new long[1];
        CnbExtension.check("CnbDocument.readEmbeddedTexture2D", NativeCnbRoutes
                .cnbDocumentReadEmbeddedTexture2d(open(), CnbExtension.utf8(label), texture));
        return new CnbTextureData(texture[0]);
    }

    /**
     * Decodes the whole file as a two-dimensional texture.
     *
     * @return the texture data, which the caller closes
     * @throws CnbFormatException when the file is not a Texture2D asset
     */
    public CnbTextureData decodeTexture2D() {
        long[] texture = new long[1];
        CnbExtension.check("CnbDocument.decodeTexture2D",
                NativeCnbRoutes.cnbDecodeTexture2d(open(), texture));
        return new CnbTextureData(texture[0]);
    }

    /**
     * Decodes the whole file as a compiled sound.
     *
     * @return the sound data, which the caller closes
     * @throws CnbFormatException when the file is not a SoundEffect asset, or its declared counts
     *         disagree with its payload
     */
    public CnbSoundEffectData decodeSoundEffect() {
        long[] sound = new long[1];
        CnbExtension.check("CnbDocument.decodeSoundEffect",
                NativeCnbRoutes.cnbDecodeSoundEffect(open(), sound));
        return new CnbSoundEffectData(sound[0]);
    }

    /**
     * Reads the file's song metadata and its streaming reference.
     *
     * @return the song's recorded name, duration and media reference
     * @throws CnbFormatException when the file is not a Song asset, or does not name exactly one
     *         external reference
     */
    public CnbSong decodeSong() {
        return CnbSong.read(this, open());
    }

    /**
     * Reads the file's video metadata and its streaming reference.
     *
     * @return the video's recorded shape, rate, duration, soundtrack type and media reference
     * @throws CnbFormatException when the file is not a Video asset, or its declarations are out
     *         of range
     */
    public CnbVideo decodeVideo() {
        return CnbVideo.read(open());
    }

    /**
     * Decodes the whole file as a sprite font.
     *
     * @return the font data, atlas and glyph table together, which the caller closes
     * @throws CnbFormatException when the file is not a SpriteFont asset
     */
    public CnbSpriteFontData decodeSpriteFont() {
        long[] font = new long[1];
        CnbExtension.check("CnbDocument.decodeSpriteFont",
                NativeCnbRoutes.cnbDecodeSpriteFont(open(), font));
        return new CnbSpriteFontData(font[0]);
    }

    /**
     * Decodes the whole file as an ordinary XNA curve.
     *
     * @return the curve, with its keys and its loop behaviour
     * @throws CnbFormatException when the file is not a Curve asset, or names an enumerator XNA
     *         has no constant for
     */
    public Curve decodeCurve() {
        return CnbCurve.decode(open());
    }

    /**
     * Decodes the whole file as a model.
     *
     * @return the model data, which the caller closes
     * @throws CnbFormatException when the file is not a Model asset, or its chunks disagree
     */
    public CnbModelData decodeModel() {
        long[] model = new long[1];
        CnbExtension.check("CnbDocument.decodeModel",
                NativeCnbRoutes.cnbDecodeModel(open(), model));
        return new CnbModelData(model[0]);
    }

    /**
     * Decodes the whole file as a cube texture.
     *
     * @return the texture data, with six faces, which the caller closes
     * @throws CnbFormatException when the file is not a TextureCube asset
     */
    public CnbTextureData decodeTextureCube() {
        long[] texture = new long[1];
        CnbExtension.check("CnbDocument.decodeTextureCube",
                NativeCnbRoutes.cnbDecodeTextureCube(open(), texture));
        return new CnbTextureData(texture[0]);
    }

    /**
     * Decodes the whole file as a volume texture.
     *
     * @return the texture data, which the caller closes
     * @throws CnbFormatException when the file is not a Texture3D asset
     */
    public CnbTextureData decodeTexture3D() {
        long[] texture = new long[1];
        CnbExtension.check("CnbDocument.decodeTexture3D",
                NativeCnbRoutes.cnbDecodeTexture3d(open(), texture));
        return new CnbTextureData(texture[0]);
    }

    /** Releases the document. Closing twice is a no-op. */
    @Override
    public void close() {
        synchronized (this) {
            if (closed) {
                return;
            }
            closed = true;
        }
        CnbExtension.check("CnbDocument.close", NativeCnbRoutes.cnbDocumentDestroy(handle));
    }

    private long open() {
        synchronized (this) {
            if (closed) {
                throw new IllegalStateException("This CnbDocument is closed");
            }
        }
        return handle;
    }
}
