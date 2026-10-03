package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.EventArgs;

/** Snapshot of the name and tag reported by a graphics-device destruction event. */
public final class ResourceDestroyedEventArgs extends EventArgs {

    private final String name;
    private final Object tag;

    ResourceDestroyedEventArgs(String name, Object tag) {
        this.name = name;
        this.tag = tag;
    }

    public String getName() {
        return name;
    }

    public Object getTag() {
        return tag;
    }
}
