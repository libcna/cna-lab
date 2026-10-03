package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Media.Song;

import org.openeggbert.cna.internal.generated.NativeCnbRoutes;

import java.net.URI;
import java.time.Duration;
import java.util.Objects;

/**
 * A song as {@code .cnb} carries it: metadata and a reference, never the audio.
 *
 * <p><strong>The media stays outside the file, deliberately.</strong> A song can be hundreds of
 * megabytes and wants streaming, so embedding it would force the whole thing through the
 * container's chunk machinery before its first second could play. What the file records is the
 * logical name of the media beside it, as the document's single external reference, which is what
 * makes the dependency visible to a build tool.
 *
 * <p>Because the reference is <em>logical</em>, turning one into a playable {@link Song} needs
 * something only the caller knows -- where their content actually lives on this machine. That is
 * why {@link #toSong(URI)} asks for the resolved location rather than inventing a lookup rule.
 *
 * @param StreamReference the logical name of the media file to stream; never empty
 * @param Name the song's display name; empty when the compiler recorded none
 * @param Duration the recorded duration; zero when the compiler could not determine it
 */
public record CnbSong(String StreamReference, String Name, Duration Duration) {

    /**
     * Encodes a song as a complete {@code .cnb} file.
     *
     * @param streamReference the logical name of the media to stream; must not be empty
     * @param name the display name, which may be empty
     * @param duration the duration, or {@link java.time.Duration#ZERO} when it is not known
     * @param contentName the source content name to record
     * @return the whole file
     * @throws CnbFormatException when the stream reference is empty or malformed
     */
    public static byte[] encode(
            String streamReference, String name, Duration duration, String contentName) {
        CnbExtension.requireAvailable();
        Objects.requireNonNull(streamReference, "streamReference");
        Objects.requireNonNull(name, "name");
        Objects.requireNonNull(duration, "duration");
        Objects.requireNonNull(contentName, "contentName");
        if (duration.isNegative()) {
            throw new IllegalArgumentException("duration must not be negative");
        }
        byte[] reference = CnbExtension.utf8(streamReference);
        byte[] display = CnbExtension.utf8(name);
        byte[] content = CnbExtension.utf8(contentName);
        int milliseconds = Math.toIntExact(duration.toMillis());
        long[] size = new long[1];
        int probe = NativeCnbRoutes.cnbEncodeSong(
                reference, display, milliseconds, content, new byte[0], size);
        if (probe != CnbExtension.RESULT_BUFFER_TOO_SMALL) {
            CnbExtension.check("CnbSong.encode", probe);
        }
        byte[] destination = new byte[Math.toIntExact(size[0])];
        long[] written = new long[1];
        CnbExtension.check("CnbSong.encode", NativeCnbRoutes.cnbEncodeSong(
                reference, display, milliseconds, content, destination, written));
        return CnbExtension.trim(destination, written[0]);
    }

    /**
     * Creates an ordinary XNA {@link Song} pointing at the resolved media.
     *
     * <p>The display name crosses from the file; the location does not, because the file records a
     * logical name and only the caller knows what it resolves to here.
     *
     * @param media where the media actually is on this machine
     * @return the song, which the media player can play
     */
    public Song toSong(URI media) {
        Objects.requireNonNull(media, "media");
        return Song.FromUri(Name, media);
    }

    static CnbSong read(CnbDocument document, long handle) {
        int[] milliseconds = new int[1];
        CnbExtension.check("CnbDocument.decodeSong", NativeCnbRoutes
                .cnbDecodeSongDurationMilliseconds(handle, milliseconds));
        String reference = CnbExtension.text("CnbDocument.decodeSong",
                bytes -> NativeCnbRoutes.cnbDecodeSongStreamReferenceSize(handle, bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbDecodeSongStreamReference(handle, destination, bytes));
        String name = CnbExtension.text("CnbDocument.decodeSong",
                bytes -> NativeCnbRoutes.cnbDecodeSongNameSize(handle, bytes),
                (destination, bytes) ->
                        NativeCnbRoutes.cnbDecodeSongName(handle, destination, bytes));
        Objects.requireNonNull(document, "document");
        return new CnbSong(reference, name, java.time.Duration.ofMillis(
                Integer.toUnsignedLong(milliseconds[0])));
    }
}
