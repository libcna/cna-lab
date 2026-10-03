import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.DisplayOrientation;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.PlayerIndex;
import Microsoft.Xna.Framework.WindowHandle;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Input.Keyboard;
import Microsoft.Xna.Framework.Input.KeyboardState;
import Microsoft.Xna.Framework.Input.Keys;
import Microsoft.Xna.Framework.Input.ButtonState;
import Microsoft.Xna.Framework.Input.Mouse;
import Microsoft.Xna.Framework.Input.MouseState;
import Microsoft.Xna.Framework.Graphics.SpriteBatch;
import Microsoft.Xna.Framework.Graphics.Texture2D;

import java.io.InputStream;

public final class ProbeGame extends Game {
    private final GraphicsDeviceManager graphics;
    private final DisplayOrientation supportedOrientations;
    private final WindowHandle initialWindowHandle;
    private SpriteBatch spriteBatch;
    private Texture2D texture;

    public ProbeGame() {
        graphics = new GraphicsDeviceManager(this);
        supportedOrientations = DisplayOrientation.LandscapeLeft.Or(
                DisplayOrientation.LandscapeRight);
        initialWindowHandle = getWindow().getHandle();
        getContent().setRootDirectory("Content");
        setIsMouseVisible(true);
        GameComponent component = new GameComponent(this);
        component.setUpdateOrder(10);
        getComponents().add(component);
        getServices().AddService(GraphicsDeviceManager.class, graphics);
        if (initialWindowHandle.getIsZero() && supportedOrientations.getValue() == 0) {
            getWindow().setTitle("ProbeGame");
        }
    }

    @Override
    protected void LoadContent() {
        texture = new Texture2D(getGraphicsDevice(), 1, 1);
        texture.SetData(new Color[] {Color.White});
        spriteBatch = new SpriteBatch(getGraphicsDevice());
    }

    @Override
    protected void Update(GameTime gameTime) {
        KeyboardState keyboard = Keyboard.GetState(PlayerIndex.One);
        MouseState mouse = Mouse.GetState();
        if (keyboard.IsKeyDown(Keys.Escape)
                || mouse.getLeftButton() == ButtonState.Pressed
                || gameTime.getElapsedGameTime().isNegative()) {
            Exit();
        }
    }

    @Override
    protected void Draw(GameTime gameTime) {
        getGraphicsDevice().Clear(Color.CornflowerBlue);
        spriteBatch.Begin();
        spriteBatch.Draw(texture, new Microsoft.Xna.Framework.Vector2(16.0f), Color.White);
        spriteBatch.End();
    }

    @Override
    protected void UnloadContent() {
        spriteBatch.close();
        texture.close();
    }

    private Texture2D loadPng(InputStream stream) {
        return Texture2D.FromStream(getGraphicsDevice(), stream);
    }
}
