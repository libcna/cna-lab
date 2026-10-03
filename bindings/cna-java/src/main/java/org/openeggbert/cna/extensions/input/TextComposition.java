package org.openeggbert.cna.extensions.input;

import java.nio.charset.StandardCharsets;

/**
 * The draft text an input method is composing, before the player commits it.
 *
 * <p>A CNA extension. XNA has no notion of composition at all: it reports committed characters
 * and nothing else, so a game cannot show a Japanese or Chinese player what they are typing
 * before it is accepted.
 *
 * <p><strong>{@code ByteStart} and {@code ByteLength} are byte offsets into UTF-8</strong>, which
 * is what the host reports and what CNA passes through without validating. They are not
 * character counts and they are not Java string indices; {@link #getStart()} and {@link #getEnd()}
 * convert them, and say so when they cannot.
 *
 * @param Text the draft as the host has it so far, possibly empty
 * @param ByteStart the active editing region's byte offset, exactly as the host reported it
 * @param ByteLength the active editing region's byte length, exactly as the host reported it
 */
public record TextComposition(String Text, int ByteStart, int ByteLength) {

    /**
     * Returns where the active region starts as a Java string index.
     *
     * @return the index, or -1 when the host's byte offset does not fall on a character boundary
     *     of this text -- CNA forwards the host's values unchecked, so that is possible
     */
    public int getStart() {
        return characterIndex(ByteStart);
    }

    /**
     * Returns where the active region ends as a Java string index, exclusive.
     *
     * @return the index, or -1 when the host's byte offsets do not name a character boundary
     */
    public int getEnd() {
        if (ByteLength < 0) {
            return -1;
        }
        return characterIndex(ByteStart + ByteLength);
    }

    /**
     * Returns the text of the active region.
     *
     * @return the substring, or {@code null} when the host's byte offsets do not name one
     */
    public String getActiveText() {
        int start = getStart();
        int end = getEnd();
        if (start < 0 || end < 0 || end < start) {
            return null;
        }
        return Text.substring(start, end);
    }

    /**
     * Maps a UTF-8 byte offset to a Java string index.
     *
     * <p>Walking the encoded form is the only correct way: one character can be one, two or three
     * UTF-8 bytes and can occupy two Java chars, so neither string length is a byte count.
     */
    private int characterIndex(int byteOffset) {
        if (byteOffset < 0) {
            return -1;
        }
        if (byteOffset == 0) {
            return 0;
        }
        int bytes = 0;
        int index = 0;
        while (index < Text.length()) {
            int codePoint = Text.codePointAt(index);
            int width = String.valueOf(Character.toChars(codePoint))
                    .getBytes(StandardCharsets.UTF_8).length;
            if (bytes == byteOffset) {
                return index;
            }
            if (bytes > byteOffset) {
                return -1;
            }
            bytes += width;
            index += Character.charCount(codePoint);
        }
        return bytes == byteOffset ? index : -1;
    }
}
