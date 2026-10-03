package Microsoft.Xna.Framework.Content;

import java.io.ByteArrayOutputStream;
import java.util.Objects;

/** XNA's LZX frame layer around a persistent 64 KiB LZX decoder. */
final class XnbLzxDecompression {
    private static final int DEFAULT_FRAME_SIZE = 0x8000;
    private static final int MAX_DECOMPRESSED_SIZE = 256 * 1024 * 1024;

    private XnbLzxDecompression() {
    }

    static byte[] decompress(byte[] compressed, int decompressedSize, String assetName) {
        Objects.requireNonNull(compressed, "compressed");
        if (decompressedSize < 0 || decompressedSize > MAX_DECOMPRESSED_SIZE) {
            throw failure(assetName, "invalid decompressed size " + decompressedSize);
        }

        LzxDecoder decoder = new LzxDecoder(16);
        ByteArrayOutputStream output = new ByteArrayOutputStream(decompressedSize);
        int position = 0;

        while (position < compressed.length) {
            if (compressed.length - position < 2) {
                throw failure(assetName, "truncated LZX block header");
            }

            int high = Byte.toUnsignedInt(compressed[position]);
            int low = Byte.toUnsignedInt(compressed[position + 1]);
            int frameSize = DEFAULT_FRAME_SIZE;
            int blockSize;
            int headerSize;
            if (high == 0xff) {
                if (compressed.length - position < 5) {
                    throw failure(assetName, "truncated extended LZX block header");
                }
                frameSize = low << 8 | Byte.toUnsignedInt(compressed[position + 2]);
                blockSize = Byte.toUnsignedInt(compressed[position + 3]) << 8
                        | Byte.toUnsignedInt(compressed[position + 4]);
                headerSize = 5;
            } else {
                blockSize = high << 8 | low;
                headerSize = 2;
            }

            if (frameSize == 0 || blockSize == 0) {
                if (output.size() != decompressedSize) {
                    throw failure(assetName, frameSize == 0
                            ? "invalid LZX frame length 0"
                            : "invalid LZX block length 0");
                }
                for (int i = position; i < compressed.length; i++) {
                    if (compressed[i] != 0) {
                        throw failure(assetName, "invalid data after the LZX end marker");
                    }
                }
                position = compressed.length;
                break;
            }
            if (frameSize > DEFAULT_FRAME_SIZE) {
                throw failure(assetName, "invalid LZX frame length " + frameSize);
            }
            if (frameSize > decompressedSize - output.size()) {
                throw failure(assetName, "LZX frame exceeds the declared decompressed size");
            }

            int blockStart = position + headerSize;
            if (blockSize > compressed.length - blockStart) {
                throw failure(assetName, "truncated LZX block");
            }

            int before = output.size();
            if (decoder.decompress(compressed, blockStart, blockSize, output, frameSize) != 0
                    || output.size() - before != frameSize) {
                throw failure(assetName, "LZX decompression failed");
            }
            position = blockStart + blockSize;
        }

        if (output.size() != decompressedSize) {
            throw failure(assetName, "decompressed size " + output.size()
                    + " does not match declared size " + decompressedSize);
        }
        return output.toByteArray();
    }

    private static ContentLoadException failure(String assetName, String reason) {
        return new ContentLoadException("'" + assetName + ".xnb' is invalid: " + reason);
    }
}
