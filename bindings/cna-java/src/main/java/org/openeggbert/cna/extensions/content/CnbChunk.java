package org.openeggbert.cna.extensions.content;

/**
 * One chunk of a {@code .cnb} file, as the table of contents describes it.
 *
 * <p>This is the entry, not the bytes: {@link CnbDocument#readChunk(int)} copies the payload and
 * {@link CnbDocument#openChunk(int)} gives a cursor over it.
 *
 * @param Id the four-character identifier
 * @param Mandatory whether a reader that does not understand the identifier must refuse the whole
 *     file rather than skip the chunk
 * @param Offset where the chunk's stored bytes begin in the file
 * @param StoredSize how many bytes it occupies in the file
 * @param UncompressedSize how many bytes it becomes, equal to {@code StoredSize} when it is
 *     stored uncompressed
 * @param Checksum the CRC-32C of the stored bytes
 * @param Compression how the chunk is stored
 * @param Alignment the alignment the writer gave the chunk's offset
 */
public record CnbChunk(
        CnbChunkId Id,
        boolean Mandatory,
        long Offset,
        long StoredSize,
        long UncompressedSize,
        long Checksum,
        CnbCompression Compression,
        long Alignment) {
}
