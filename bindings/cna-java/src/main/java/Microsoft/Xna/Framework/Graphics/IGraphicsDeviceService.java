package Microsoft.Xna.Framework.Graphics;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;

/** XNA service contract for retrieving and observing the active graphics device. */
public interface IGraphicsDeviceService {

    GraphicsDevice getGraphicsDevice();

    void addDeviceCreatedListener(EventHandler<EventArgs> listener);

    void removeDeviceCreatedListener(EventHandler<EventArgs> listener);

    void addDeviceDisposingListener(EventHandler<EventArgs> listener);

    void removeDeviceDisposingListener(EventHandler<EventArgs> listener);

    void addDeviceResetListener(EventHandler<EventArgs> listener);

    void removeDeviceResetListener(EventHandler<EventArgs> listener);

    void addDeviceResettingListener(EventHandler<EventArgs> listener);

    void removeDeviceResettingListener(EventHandler<EventArgs> listener);
}
