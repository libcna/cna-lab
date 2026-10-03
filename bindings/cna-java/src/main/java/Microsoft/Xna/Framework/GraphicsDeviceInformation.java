package Microsoft.Xna.Framework;

import Microsoft.Xna.Framework.Graphics.GraphicsAdapter;
import Microsoft.Xna.Framework.Graphics.GraphicsProfile;
import Microsoft.Xna.Framework.Graphics.PresentationParameters;

import java.util.Objects;

/** Mutable device proposal delivered while graphics settings are being prepared. */
public class GraphicsDeviceInformation {

    private GraphicsAdapter adapter = GraphicsAdapter.getDefaultAdapter();
    private GraphicsProfile graphicsProfile = GraphicsProfile.Reach;
    private PresentationParameters presentationParameters = new PresentationParameters();
    private boolean headlessExtension;

    public GraphicsDeviceInformation() {
    }

    public final GraphicsAdapter getAdapter() {
        return adapter;
    }

    public final void setAdapter(GraphicsAdapter value) {
        adapter = Objects.requireNonNull(value, "value");
    }

    public final GraphicsProfile getGraphicsProfile() {
        return graphicsProfile;
    }

    public final void setGraphicsProfile(GraphicsProfile value) {
        graphicsProfile = Objects.requireNonNull(value, "value");
    }

    public final PresentationParameters getPresentationParameters() {
        return presentationParameters;
    }

    public final void setPresentationParameters(PresentationParameters value) {
        presentationParameters = value;
    }

    public final GraphicsDeviceInformation Clone() {
        GraphicsDeviceInformation clone = new GraphicsDeviceInformation();
        clone.adapter = adapter;
        clone.graphicsProfile = graphicsProfile;
        clone.presentationParameters = presentationParameters.Clone();
        clone.headlessExtension = headlessExtension;
        return clone;
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof GraphicsDeviceInformation other)
                || adapter != other.adapter
                || graphicsProfile != other.graphicsProfile) {
            return false;
        }
        PresentationParameters left = presentationParameters;
        PresentationParameters right = other.presentationParameters;
        return left.getBackBufferWidth() == right.getBackBufferWidth()
                && left.getBackBufferHeight() == right.getBackBufferHeight()
                && left.getBackBufferFormat() == right.getBackBufferFormat()
                && left.getDepthStencilFormat() == right.getDepthStencilFormat()
                && left.getMultiSampleCount() == right.getMultiSampleCount()
                && left.getDisplayOrientation().equals(right.getDisplayOrientation())
                && left.getPresentationInterval() == right.getPresentationInterval()
                && left.getRenderTargetUsage() == right.getRenderTargetUsage()
                && left.getDeviceWindowHandle().equals(right.getDeviceWindowHandle())
                && left.getIsFullScreen() == right.getIsFullScreen();
    }

    @Override
    public int hashCode() {
        PresentationParameters value = presentationParameters;
        return graphicsProfile.ordinal()
                ^ adapter.hashCode()
                ^ Integer.hashCode(value.getBackBufferWidth())
                ^ Integer.hashCode(value.getBackBufferHeight())
                ^ value.getBackBufferFormat().ordinal()
                ^ value.getDepthStencilFormat().ordinal()
                ^ Integer.hashCode(value.getMultiSampleCount())
                ^ value.getDisplayOrientation().hashCode()
                ^ value.getPresentationInterval().ordinal()
                ^ value.getRenderTargetUsage().ordinal()
                ^ value.getDeviceWindowHandle().hashCode()
                ^ Boolean.hashCode(value.getIsFullScreen());
    }

    final boolean getHeadlessExtension() {
        return headlessExtension;
    }

    final void setHeadlessExtension(boolean value) {
        headlessExtension = value;
    }
}
