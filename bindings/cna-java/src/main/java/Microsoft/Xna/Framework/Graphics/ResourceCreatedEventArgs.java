package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.EventArgs;

/** Snapshot of the resource reported by a graphics-device creation event. */
public final class ResourceCreatedEventArgs extends EventArgs {

    private final Object resource;

    ResourceCreatedEventArgs(Object resource) {
        this.resource = resource;
    }

    public Object getResource() {
        return resource;
    }
}
