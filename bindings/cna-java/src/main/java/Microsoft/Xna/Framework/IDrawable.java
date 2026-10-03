package Microsoft.Xna.Framework;

/** Ordered, visibility-controlled game-loop drawing participant. */
public interface IDrawable {

    int getDrawOrder();

    boolean getVisible();

    void addDrawOrderChangedListener(EventHandler<EventArgs> listener);

    void removeDrawOrderChangedListener(EventHandler<EventArgs> listener);

    void addVisibleChangedListener(EventHandler<EventArgs> listener);

    void removeVisibleChangedListener(EventHandler<EventArgs> listener);

    void Draw(GameTime gameTime);
}
