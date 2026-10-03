package Microsoft.Xna.Framework.Audio;

import java.util.Objects;

/** Immutable renderer identity returned by {@link AudioEngine}. */
public final class RendererDetail {
    private final String friendlyName;
    private final String rendererId;

    public RendererDetail() {
        friendlyName = null;
        rendererId = null;
    }

    public RendererDetail(RendererDetail value) {
        RendererDetail selected = Objects.requireNonNull(value, "value");
        friendlyName = selected.friendlyName;
        rendererId = selected.rendererId;
    }

    RendererDetail(String friendlyName, String rendererId) {
        this.friendlyName = friendlyName;
        this.rendererId = rendererId;
    }

    public String getFriendlyName() { return friendlyName; }
    public String getRendererId() { return rendererId; }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof RendererDetail value
                && Objects.equals(friendlyName, value.friendlyName)
                && Objects.equals(rendererId, value.rendererId);
    }

    @Override
    public int hashCode() {
        return Objects.hashCode(friendlyName) ^ Objects.hashCode(rendererId);
    }

    @Override
    public String toString() {
        return "Microsoft.Xna.Framework.Audio.RendererDetail";
    }
}
