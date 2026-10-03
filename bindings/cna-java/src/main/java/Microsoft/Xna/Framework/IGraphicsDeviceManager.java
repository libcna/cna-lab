package Microsoft.Xna.Framework;

/** XNA contract used by {@link Game} to coordinate graphics-device drawing. */
public interface IGraphicsDeviceManager {

    boolean BeginDraw();

    void CreateDevice();

    void EndDraw();
}
