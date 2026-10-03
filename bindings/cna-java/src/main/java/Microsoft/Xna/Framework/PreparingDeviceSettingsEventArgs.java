package Microsoft.Xna.Framework;

/** Event arguments carrying one mutable graphics-device proposal. */
public class PreparingDeviceSettingsEventArgs extends EventArgs {

    private final GraphicsDeviceInformation graphicsDeviceInformation;

    public PreparingDeviceSettingsEventArgs(GraphicsDeviceInformation graphicsDeviceInformation) {
        this.graphicsDeviceInformation = graphicsDeviceInformation;
    }

    public final GraphicsDeviceInformation getGraphicsDeviceInformation() {
        return graphicsDeviceInformation;
    }
}
