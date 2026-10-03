package Microsoft.Xna.Framework;

/** Ordered, enableable game-loop update participant. */
public interface IUpdateable {

    boolean getEnabled();

    int getUpdateOrder();

    void addEnabledChangedListener(EventHandler<EventArgs> listener);

    void removeEnabledChangedListener(EventHandler<EventArgs> listener);

    void addUpdateOrderChangedListener(EventHandler<EventArgs> listener);

    void removeUpdateOrderChangedListener(EventHandler<EventArgs> listener);

    void Update(GameTime gameTime);
}
