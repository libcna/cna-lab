package Microsoft.Xna.Framework.Graphics;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;

/** Read-only snapshot of the display modes supported by one adapter. */
public class DisplayModeCollection implements Iterable<DisplayMode> {

    private final List<DisplayMode> displayModes;

    DisplayModeCollection(List<DisplayMode> displayModes) {
        this.displayModes = List.copyOf(displayModes);
    }

    public final Iterable<DisplayMode> get(SurfaceFormat format) {
        SurfaceFormat selected = Objects.requireNonNull(format, "format");
        ArrayList<DisplayMode> matches = new ArrayList<>();
        for (DisplayMode mode : displayModes) {
            if (mode.getFormat() == selected) {
                matches.add(mode);
            }
        }
        return matches;
    }

    public final Iterator<DisplayMode> GetEnumerator() {
        return displayModes.iterator();
    }

    @Override
    public final Iterator<DisplayMode> iterator() {
        return displayModes.iterator();
    }
}
