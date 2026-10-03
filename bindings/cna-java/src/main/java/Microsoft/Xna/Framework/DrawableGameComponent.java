package Microsoft.Xna.Framework;

import Microsoft.Xna.Framework.Graphics.GraphicsDevice;

import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/** Managed XNA drawable component with ordered visibility events. */
public class DrawableGameComponent extends GameComponent implements IDrawable {

    private final CopyOnWriteArrayList<EventHandler<EventArgs>> drawOrderChangedListeners =
            new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<EventHandler<EventArgs>> visibleChangedListeners =
            new CopyOnWriteArrayList<>();
    private int drawOrder;
    private boolean visible = true;
    private boolean contentLoaded;

    public DrawableGameComponent(Game game) {
        super(game);
    }

    @Override
    public final int getDrawOrder() {
        return drawOrder;
    }

    public final void setDrawOrder(int value) {
        if (drawOrder != value) {
            drawOrder = value;
            OnDrawOrderChanged(this, EventArgs.Empty);
        }
    }

    public final GraphicsDevice getGraphicsDevice() {
        return getGame().getGraphicsDevice();
    }

    @Override
    public final boolean getVisible() {
        return visible;
    }

    public final void setVisible(boolean value) {
        if (visible != value) {
            visible = value;
            OnVisibleChanged(this, EventArgs.Empty);
        }
    }

    @Override
    public void Initialize() {
        super.Initialize();
        if (!contentLoaded) {
            LoadContent();
            contentLoaded = true;
        }
    }

    @Override
    public void Draw(GameTime gameTime) {
        Objects.requireNonNull(gameTime, "gameTime");
    }

    protected void LoadContent() {
    }

    protected void UnloadContent() {
    }

    @Override
    public final void addDrawOrderChangedListener(EventHandler<EventArgs> listener) {
        drawOrderChangedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    @Override
    public final void removeDrawOrderChangedListener(EventHandler<EventArgs> listener) {
        drawOrderChangedListeners.remove(listener);
    }

    @Override
    public final void addVisibleChangedListener(EventHandler<EventArgs> listener) {
        visibleChangedListeners.add(Objects.requireNonNull(listener, "listener"));
    }

    @Override
    public final void removeVisibleChangedListener(EventHandler<EventArgs> listener) {
        visibleChangedListeners.remove(listener);
    }

    protected void OnDrawOrderChanged(Object sender, EventArgs args) {
        for (EventHandler<EventArgs> listener : drawOrderChangedListeners) {
            listener.invoke(sender, args);
        }
    }

    protected void OnVisibleChanged(Object sender, EventArgs args) {
        for (EventHandler<EventArgs> listener : visibleChangedListeners) {
            listener.invoke(sender, args);
        }
    }

    @Override
    protected void Dispose(boolean disposing) {
        try {
            if (disposing && contentLoaded) {
                UnloadContent();
                contentLoaded = false;
            }
        } finally {
            drawOrderChangedListeners.clear();
            visibleChangedListeners.clear();
            super.Dispose(disposing);
        }
    }
}
