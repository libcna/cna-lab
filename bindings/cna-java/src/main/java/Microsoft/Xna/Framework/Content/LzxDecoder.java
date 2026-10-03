package Microsoft.Xna.Framework.Content;

import java.io.ByteArrayOutputStream;
import java.util.Arrays;

/**
 * Stateful LZX decoder used by XNB containers.
 *
 * <p>This is a Java port of CNA's MS-PL LZX decoder, which in turn follows the
 * FNA/libmspack decoder. One instance must be retained for every frame in one
 * XNB because the window, Huffman tables, and repeated offsets are shared
 * across frame boundaries.</p>
 */
final class LzxDecoder {
    private static final int MIN_MATCH = 2;
    private static final int NUM_CHARS = 256;
    private static final int PRETREE_NUM_ELEMENTS = 20;
    private static final int ALIGNED_NUM_ELEMENTS = 8;
    private static final int NUM_PRIMARY_LENGTHS = 7;
    private static final int NUM_SECONDARY_LENGTHS = 249;

    private static final int PRETREE_MAX_SYMBOLS = PRETREE_NUM_ELEMENTS;
    private static final int PRETREE_TABLE_BITS = 6;
    private static final int MAINTREE_MAX_SYMBOLS = NUM_CHARS + 50 * 8;
    private static final int MAINTREE_TABLE_BITS = 12;
    private static final int LENGTH_MAX_SYMBOLS = NUM_SECONDARY_LENGTHS + 1;
    private static final int LENGTH_TABLE_BITS = 12;
    private static final int ALIGNED_MAX_SYMBOLS = ALIGNED_NUM_ELEMENTS;
    private static final int ALIGNED_TABLE_BITS = 7;
    private static final int LENTABLE_SAFETY = 64;

    private static final int[] EXTRA_BITS = buildExtraBits();
    private static final int[] POSITION_BASE = buildPositionBase();

    private static final int BLOCK_INVALID = 0;
    private static final int BLOCK_VERBATIM = 1;
    private static final int BLOCK_ALIGNED = 2;
    private static final int BLOCK_UNCOMPRESSED = 3;

    private int r0;
    private int r1;
    private int r2;
    private final int mainElements;
    private boolean headerRead;
    private int blockType = BLOCK_INVALID;
    private int blockLength;
    private int blockRemaining;
    private int framesRead;
    private int intelFileSize;
    private int intelCurrentPosition;
    private boolean intelStarted;

    private final int[] pretreeTable;
    private final byte[] pretreeLengths;
    private final int[] maintreeTable;
    private final byte[] maintreeLengths;
    private final int[] lengthTable;
    private final byte[] lengthLengths;
    private final int[] alignedTable;
    private final byte[] alignedLengths;

    private final byte[] window;
    private final int windowSize;
    private int windowPosition;

    LzxDecoder(int windowExponent) {
        if (windowExponent < 15 || windowExponent > 21) {
            throw new ContentLoadException("Unsupported LZX window size (must be 15-21)");
        }

        windowSize = 1 << windowExponent;
        window = new byte[windowSize];
        Arrays.fill(window, (byte) 0xdc);

        int positionSlots;
        if (windowExponent == 20) {
            positionSlots = 42;
        } else if (windowExponent == 21) {
            positionSlots = 50;
        } else {
            positionSlots = windowExponent << 1;
        }

        r0 = r1 = r2 = 1;
        mainElements = NUM_CHARS + (positionSlots << 3);

        pretreeTable = new int[(1 << PRETREE_TABLE_BITS) + (PRETREE_MAX_SYMBOLS << 1)];
        pretreeLengths = new byte[PRETREE_MAX_SYMBOLS + LENTABLE_SAFETY];
        maintreeTable = new int[(1 << MAINTREE_TABLE_BITS) + (MAINTREE_MAX_SYMBOLS << 1)];
        maintreeLengths = new byte[MAINTREE_MAX_SYMBOLS + LENTABLE_SAFETY];
        lengthTable = new int[(1 << LENGTH_TABLE_BITS) + (LENGTH_MAX_SYMBOLS << 1)];
        lengthLengths = new byte[LENGTH_MAX_SYMBOLS + LENTABLE_SAFETY];
        alignedTable = new int[(1 << ALIGNED_TABLE_BITS) + (ALIGNED_MAX_SYMBOLS << 1)];
        alignedLengths = new byte[ALIGNED_MAX_SYMBOLS + LENTABLE_SAFETY];
    }

    private static int[] buildExtraBits() {
        int[] bits = new int[52];
        for (int i = 0, j = 0; i <= 50; i += 2) {
            bits[i] = bits[i + 1] = j;
            if (i != 0 && j < 17) {
                j++;
            }
        }
        return bits;
    }

    private static int[] buildPositionBase() {
        int[] bases = new int[51];
        for (int i = 0, value = 0; i <= 50; i++) {
            bases[i] = value;
            value += 1 << EXTRA_BITS[i];
        }
        return bases;
    }

    private static int makeDecodeTable(int symbolCount, int tableBits,
            byte[] lengths, int[] table) {
        int bitNumber = 1;
        long position = 0;
        long tableMask = 1L << tableBits;
        long bitMask = tableMask >>> 1;
        int nextSymbol = (int) bitMask;

        while (bitNumber <= tableBits) {
            for (int symbol = 0; symbol < symbolCount; symbol++) {
                if (Byte.toUnsignedInt(lengths[symbol]) == bitNumber) {
                    int leaf = (int) position;
                    position += bitMask;
                    if (position > tableMask) {
                        return -1;
                    }
                    for (long fill = bitMask; fill-- > 0;) {
                        table[leaf++] = symbol;
                    }
                }
            }
            bitMask >>>= 1;
            bitNumber++;
        }

        if (position != tableMask) {
            for (int symbol = (int) position; symbol < tableMask; symbol++) {
                table[symbol] = 0;
            }

            position <<= 16;
            tableMask <<= 16;
            bitMask = 1L << 15;

            while (bitNumber <= 16) {
                for (int symbol = 0; symbol < symbolCount; symbol++) {
                    if (Byte.toUnsignedInt(lengths[symbol]) == bitNumber) {
                        int leaf = (int) (position >>> 16);
                        for (int fill = 0; fill < bitNumber - tableBits; fill++) {
                            if (leaf < 0 || leaf >= table.length
                                    || (nextSymbol << 1) + 1 >= table.length) {
                                return -1;
                            }
                            if (table[leaf] == 0) {
                                table[nextSymbol << 1] = 0;
                                table[(nextSymbol << 1) + 1] = 0;
                                table[leaf] = nextSymbol++;
                            }
                            leaf = table[leaf] << 1;
                            if (((position >>> (15 - fill)) & 1) != 0) {
                                leaf++;
                            }
                        }
                        if (leaf < 0 || leaf >= table.length) {
                            return -1;
                        }
                        table[leaf] = symbol;
                        position += bitMask;
                        if (position > tableMask) {
                            return -1;
                        }
                    }
                }
                bitMask >>>= 1;
                bitNumber++;
            }
        }

        if (position == tableMask) {
            return 0;
        }
        for (int symbol = 0; symbol < symbolCount; symbol++) {
            if (lengths[symbol] != 0) {
                return -1;
            }
        }
        return 0;
    }

    private int readLengths(byte[] lengths, int first, int last, BitBuffer bits) {
        for (int i = 0; i < PRETREE_NUM_ELEMENTS; i++) {
            pretreeLengths[i] = (byte) bits.readBits(4);
        }
        if (makeDecodeTable(PRETREE_MAX_SYMBOLS, PRETREE_TABLE_BITS,
                pretreeLengths, pretreeTable) != 0) {
            return -1;
        }

        for (int index = first; index < last;) {
            int symbol = readHuffmanSymbol(pretreeTable, pretreeLengths,
                    PRETREE_MAX_SYMBOLS, PRETREE_TABLE_BITS, bits);
            if (symbol < 0) {
                return -1;
            }
            if (symbol == 17 || symbol == 18) {
                int count = bits.readBits(symbol == 17 ? 4 : 5) + (symbol == 17 ? 4 : 20);
                if (count > last - index) {
                    return -1;
                }
                while (count-- > 0) {
                    lengths[index++] = 0;
                }
            } else if (symbol == 19) {
                int count = bits.readBits(1) + 4;
                int delta = readHuffmanSymbol(pretreeTable, pretreeLengths,
                        PRETREE_MAX_SYMBOLS, PRETREE_TABLE_BITS, bits);
                if (delta < 0 || count > last - index) {
                    return -1;
                }
                int value = Byte.toUnsignedInt(lengths[index]) - delta;
                if (value < 0) {
                    value += 17;
                }
                while (count-- > 0) {
                    lengths[index++] = (byte) value;
                }
            } else {
                int value = Byte.toUnsignedInt(lengths[index]) - symbol;
                if (value < 0) {
                    value += 17;
                }
                lengths[index++] = (byte) value;
            }
        }
        return 0;
    }

    private static int readHuffmanSymbol(int[] table, byte[] lengths,
            int symbolCount, int tableBits, BitBuffer bits) {
        bits.ensureBits(16);
        int index = bits.peekBits(tableBits);
        if (index < 0 || index >= table.length) {
            return -1;
        }
        int symbol = table[index];
        if (symbol >= symbolCount) {
            int mask = 1 << (32 - tableBits);
            do {
                mask >>>= 1;
                symbol = (symbol << 1) | ((bits.buffer() & mask) != 0 ? 1 : 0);
                if (mask == 0 || symbol < 0 || symbol >= table.length) {
                    return -1;
                }
                symbol = table[symbol];
            } while (symbol >= symbolCount);
        }
        if (symbol < 0 || symbol >= lengths.length) {
            return -1;
        }
        int length = Byte.toUnsignedInt(lengths[symbol]);
        if (length == 0 || length > bits.bitsLeft()) {
            return -1;
        }
        bits.removeBits(length);
        return symbol;
    }

    int decompress(byte[] input, int inputOffset, int inputLength,
            ByteArrayOutputStream output, int outputLength) {
        InputCursor source = new InputCursor(input, inputOffset);
        BitBuffer bits = new BitBuffer(source);
        int startPosition = inputOffset;

        int localWindowPosition = windowPosition;
        int localR0 = r0;
        int localR1 = r1;
        int localR2 = r2;
        int remainingOutput = outputLength;

        bits.initialize();
        if (!headerRead) {
            int intel = bits.readBits(1);
            if (intel != 0) {
                int high = bits.readBits(16);
                int low = bits.readBits(16);
                intelFileSize = (high << 16) | low;
            }
            headerRead = true;
        }

        while (remainingOutput > 0) {
            if (blockRemaining == 0) {
                if (blockType == BLOCK_UNCOMPRESSED) {
                    if ((blockLength & 1) != 0) {
                        source.readByte();
                    }
                    bits.initialize();
                }

                blockType = bits.readBits(3);
                blockRemaining = blockLength = (bits.readBits(16) << 8) | bits.readBits(8);
                if (blockLength <= 0) {
                    return -1;
                }

                switch (blockType) {
                    case BLOCK_ALIGNED:
                        for (int i = 0; i < ALIGNED_NUM_ELEMENTS; i++) {
                            alignedLengths[i] = (byte) bits.readBits(3);
                        }
                        if (makeDecodeTable(ALIGNED_MAX_SYMBOLS, ALIGNED_TABLE_BITS,
                                alignedLengths, alignedTable) != 0) {
                            return -1;
                        }
                        if (readMainTrees(bits) != 0) {
                            return -1;
                        }
                        break;
                    case BLOCK_VERBATIM:
                        if (readMainTrees(bits) != 0) {
                            return -1;
                        }
                        break;
                    case BLOCK_UNCOMPRESSED:
                        intelStarted = true;
                        bits.ensureBits(16);
                        if (bits.bitsLeft() > 16) {
                            source.seekRelative(-2);
                        }
                        localR0 = source.readLittleEndianInt();
                        localR1 = source.readLittleEndianInt();
                        localR2 = source.readLittleEndianInt();
                        if (source.failed()) {
                            return -1;
                        }
                        break;
                    default:
                        return -1;
                }
            }

            if (source.position() > startPosition + inputLength) {
                if (source.position() > startPosition + inputLength + 2 || bits.bitsLeft() < 16) {
                    return -1;
                }
            }

            while (blockRemaining > 0 && remainingOutput > 0) {
                int run = Math.min(blockRemaining, remainingOutput);
                remainingOutput -= run;
                blockRemaining -= run;

                localWindowPosition &= windowSize - 1;
                if (localWindowPosition + run > windowSize) {
                    return -1;
                }

                switch (blockType) {
                    case BLOCK_VERBATIM:
                        int[] verbatimState = {localWindowPosition, localR0, localR1, localR2};
                        if (decodeCompressedRun(run, false, bits, verbatimState) != 0) {
                            return -1;
                        }
                        localWindowPosition = verbatimState[0];
                        localR0 = verbatimState[1];
                        localR1 = verbatimState[2];
                        localR2 = verbatimState[3];
                        break;
                    case BLOCK_ALIGNED:
                        int[] alignedState = {localWindowPosition, localR0, localR1, localR2};
                        if (decodeCompressedRun(run, true, bits, alignedState) != 0) {
                            return -1;
                        }
                        localWindowPosition = alignedState[0];
                        localR0 = alignedState[1];
                        localR1 = alignedState[2];
                        localR2 = alignedState[3];
                        break;
                    case BLOCK_UNCOMPRESSED:
                        if (!source.copyTo(window, localWindowPosition, run,
                                startPosition + inputLength)) {
                            return -1;
                        }
                        localWindowPosition += run;
                        break;
                    default:
                        return -1;
                }
            }
        }

        int outputStart = localWindowPosition == 0 ? windowSize : localWindowPosition;
        outputStart -= outputLength;
        if (outputStart < 0 || outputStart + outputLength > window.length) {
            return -1;
        }
        output.write(window, outputStart, outputLength);

        windowPosition = localWindowPosition;
        r0 = localR0;
        r1 = localR1;
        r2 = localR2;

        // XNB assets do not use CAB's Intel E8 transform. Matching FNA/CNA, reject it.
        if (framesRead++ < 32768 && intelFileSize != 0) {
            if (outputLength <= 6 || !intelStarted) {
                intelCurrentPosition += outputLength;
            }
            return -1;
        }
        return 0;
    }

    private int readMainTrees(BitBuffer bits) {
        if (readLengths(maintreeLengths, 0, 256, bits) != 0
                || readLengths(maintreeLengths, 256, mainElements, bits) != 0
                || makeDecodeTable(MAINTREE_MAX_SYMBOLS, MAINTREE_TABLE_BITS,
                        maintreeLengths, maintreeTable) != 0) {
            return -1;
        }
        if (maintreeLengths[0xe8] != 0) {
            intelStarted = true;
        }
        if (readLengths(lengthLengths, 0, NUM_SECONDARY_LENGTHS, bits) != 0
                || makeDecodeTable(LENGTH_MAX_SYMBOLS, LENGTH_TABLE_BITS,
                        lengthLengths, lengthTable) != 0) {
            return -1;
        }
        return 0;
    }

    private int decodeCompressedRun(int run, boolean aligned, BitBuffer bits, int[] state) {
        int windowPosition = state[0];
        int localR0 = state[1];
        int localR1 = state[2];
        int localR2 = state[3];
        int remaining = run;

        while (remaining > 0) {
            int mainElement = readHuffmanSymbol(maintreeTable, maintreeLengths,
                    MAINTREE_MAX_SYMBOLS, MAINTREE_TABLE_BITS, bits);
            if (mainElement < 0) {
                return -1;
            }
            if (mainElement < NUM_CHARS) {
                window[windowPosition++] = (byte) mainElement;
                remaining--;
                continue;
            }

            mainElement -= NUM_CHARS;
            int matchLength = mainElement & NUM_PRIMARY_LENGTHS;
            if (matchLength == NUM_PRIMARY_LENGTHS) {
                int footer = readHuffmanSymbol(lengthTable, lengthLengths,
                        LENGTH_MAX_SYMBOLS, LENGTH_TABLE_BITS, bits);
                if (footer < 0) {
                    return -1;
                }
                matchLength += footer;
            }
            matchLength += MIN_MATCH;
            if (matchLength > remaining) {
                return -1;
            }
            int copiedLength = matchLength;

            int slot = mainElement >> 3;
            int matchOffset;
            if (slot > 2) {
                if (slot >= EXTRA_BITS.length || slot >= POSITION_BASE.length) {
                    return -1;
                }
                int extra = EXTRA_BITS[slot];
                matchOffset = POSITION_BASE[slot] - 2;
                if (aligned) {
                    if (extra > 3) {
                        int verbatim = bits.readBits(extra - 3);
                        int low = readHuffmanSymbol(alignedTable, alignedLengths,
                                ALIGNED_MAX_SYMBOLS, ALIGNED_TABLE_BITS, bits);
                        if (low < 0) {
                            return -1;
                        }
                        matchOffset += (verbatim << 3) + low;
                    } else if (extra == 3) {
                        int low = readHuffmanSymbol(alignedTable, alignedLengths,
                                ALIGNED_MAX_SYMBOLS, ALIGNED_TABLE_BITS, bits);
                        if (low < 0) {
                            return -1;
                        }
                        matchOffset += low;
                    } else if (extra > 0) {
                        matchOffset += bits.readBits(extra);
                    } else {
                        matchOffset = 1;
                    }
                } else if (slot != 3) {
                    matchOffset += bits.readBits(extra);
                } else {
                    matchOffset = 1;
                }
                localR2 = localR1;
                localR1 = localR0;
                localR0 = matchOffset;
            } else if (slot == 0) {
                matchOffset = localR0;
            } else if (slot == 1) {
                matchOffset = localR1;
                localR1 = localR0;
                localR0 = matchOffset;
            } else {
                matchOffset = localR2;
                localR2 = localR0;
                localR0 = matchOffset;
            }

            if (matchOffset <= 0 || matchOffset > windowSize) {
                return -1;
            }

            int destination = windowPosition;
            int source;
            if (windowPosition >= matchOffset) {
                source = destination - matchOffset;
            } else {
                source = destination + windowSize - matchOffset;
                int wrapped = matchOffset - windowPosition;
                if (wrapped < matchLength) {
                    int first = wrapped;
                    matchLength -= first;
                    windowPosition += first;
                    while (first-- > 0) {
                        window[destination++] = window[source++];
                    }
                    source = 0;
                }
            }
            windowPosition += matchLength;
            while (matchLength-- > 0) {
                window[destination++] = window[source++];
            }
            remaining -= copiedLength;
        }

        state[0] = windowPosition;
        state[1] = localR0;
        state[2] = localR1;
        state[3] = localR2;
        return 0;
    }

    private static final class InputCursor {
        private final byte[] bytes;
        private int position;
        private boolean failed;

        InputCursor(byte[] bytes, int position) {
            this.bytes = bytes;
            this.position = position;
        }

        int readByte() {
            if (position >= bytes.length) {
                failed = true;
                return -1;
            }
            return Byte.toUnsignedInt(bytes[position++]);
        }

        int readLittleEndianInt() {
            int b0 = readByte();
            int b1 = readByte();
            int b2 = readByte();
            int b3 = readByte();
            if ((b0 | b1 | b2 | b3) < 0) {
                failed = true;
                return 0;
            }
            return b0 | b1 << 8 | b2 << 16 | b3 << 24;
        }

        boolean copyTo(byte[] target, int targetOffset, int count, int limit) {
            if (count < 0 || position < 0 || position + count > bytes.length
                    || position + count > limit || targetOffset < 0
                    || targetOffset + count > target.length) {
                failed = true;
                return false;
            }
            System.arraycopy(bytes, position, target, targetOffset, count);
            position += count;
            return true;
        }

        void seekRelative(int amount) {
            position += amount;
            if (position < 0 || position > bytes.length) {
                failed = true;
            }
        }

        int position() {
            return position;
        }

        boolean failed() {
            return failed;
        }
    }

    private static final class BitBuffer {
        private int buffer;
        private int bitsLeft;
        private final InputCursor source;

        BitBuffer(InputCursor source) {
            this.source = source;
        }

        void initialize() {
            buffer = 0;
            bitsLeft = 0;
        }

        void ensureBits(int bits) {
            while (bitsLeft < bits) {
                int low = source.readByte();
                int high = source.readByte();
                if (low < 0) {
                    low = 0xff;
                }
                if (high < 0) {
                    high = 0xff;
                }
                buffer |= ((high << 8) | low) << (16 - bitsLeft);
                bitsLeft += 16;
            }
        }

        int peekBits(int bits) {
            return buffer >>> (32 - bits);
        }

        void removeBits(int bits) {
            buffer <<= bits;
            bitsLeft -= bits;
        }

        int readBits(int bits) {
            if (bits == 0) {
                return 0;
            }
            ensureBits(bits);
            int value = peekBits(bits);
            removeBits(bits);
            return value;
        }

        int buffer() {
            return buffer;
        }

        int bitsLeft() {
            return bitsLeft;
        }
    }
}
