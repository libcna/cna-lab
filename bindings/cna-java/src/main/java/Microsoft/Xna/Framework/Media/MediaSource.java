package Microsoft.Xna.Framework.Media;

import org.openeggbert.cna.internal.NativeMedia;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/** One media-library source enumerated by the current CNA game. */
public final class MediaSource {
    private final int index;
    private final MediaSourceType type;
    private final String name;

    MediaSource(int index, MediaSourceType type, String name) {
        this.index = index;
        this.type = type;
        this.name = name;
    }

    public static List<MediaSource> GetAvailableMediaSources() {
        int count = NativeMedia.getMediaSourceCount();
        ArrayList<MediaSource> values = new ArrayList<>(count);
        for (int index = 0; index < count; index++) {
            values.add(new MediaSource(index,
                    MediaSourceType.fromValue(NativeMedia.getMediaSourceType(index)),
                    NativeMedia.getMediaSourceName(index)));
        }
        return Collections.unmodifiableList(values);
    }

    public MediaSourceType getMediaSourceType() { return type; }
    public String getName() { return name; }
    @Override public String toString() { return name; }
    int index() { return index; }
}
