package com.openeggbert.cna.template;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.GraphicsDeviceManager;
import Microsoft.Xna.Framework.Graphics.SpriteBatch;
import Microsoft.Xna.Framework.Graphics.Texture2D;
import Microsoft.Xna.Framework.Input.Keyboard;
import Microsoft.Xna.Framework.Input.Keys;
import Microsoft.Xna.Framework.Input.ButtonState;
import Microsoft.Xna.Framework.Input.Mouse;

import java.io.IOException;
import java.io.InputStream;
import java.util.Base64;

/** Small truthful desktop canary for the currently implemented CNA-Java slice. */
public final class HelloGame extends Game {

    private final GraphicsDeviceManager graphics;
    private final int frameLimit;
    private SpriteBatch spriteBatch;
    private Texture2D logo;
    private Vector2 logoPosition = new Vector2(16.0f);
    private int drawnFrames;

    public HelloGame(int frameLimit) {
        if (frameLimit < 0) {
            throw new IllegalArgumentException("frameLimit must not be negative");
        }
        this.frameLimit = frameLimit;
        graphics = new GraphicsDeviceManager(this);
        getWindow().setTitle("CNA-Java: " + getClass().getSimpleName());
        getContent().setRootDirectory("Content");
        setIsMouseVisible(true);
    }

    @Override
    protected void Initialize() {
        super.Initialize();
        System.out.println("cna-java-template: initialized");
    }

    @Override
    protected void LoadContent() {
        InputStream resource = HelloGame.class.getResourceAsStream("/cna-logo.png.base64");
        if (resource == null) {
            throw new IllegalStateException("Missing cna-logo.png.base64 resource");
        }
        try (resource; InputStream encoded = Base64.getMimeDecoder().wrap(resource)) {
            logo = Texture2D.FromStream(getGraphicsDevice(), encoded);
        } catch (IOException exception) {
            throw new IllegalStateException("Could not close the encoded PNG resource", exception);
        }
        spriteBatch = new SpriteBatch(getGraphicsDevice());
        super.LoadContent();
    }

    @Override
    protected void Update(GameTime gameTime) {
        if (Keyboard.GetState().IsKeyDown(Keys.Escape)
                || Mouse.GetState().getLeftButton() == ButtonState.Pressed) {
            Exit();
        }
        logoPosition.X += (float)gameTime.getElapsedGameTime().toNanos() / 20_000_000.0f;
        super.Update(gameTime);
    }

    @Override
    protected void Draw(GameTime gameTime) {
        getGraphicsDevice().Clear(Color.CornflowerBlue);
        spriteBatch.Begin();
        spriteBatch.Draw(logo, logoPosition, Color.White);
        spriteBatch.End();
        drawnFrames++;
        if (frameLimit > 0 && drawnFrames >= frameLimit) {
            Exit();
        }
        super.Draw(gameTime);
    }

    @Override
    protected void UnloadContent() {
        if (spriteBatch != null) {
            spriteBatch.close();
        }
        if (logo != null) {
            logo.close();
        }
        super.UnloadContent();
    }

    @Override
    protected void EndRun() {
        System.out.println("cna-java-template: completed " + drawnFrames + " frames");
        super.EndRun();
    }

    int getDrawnFrames() {
        return drawnFrames;
    }

    GraphicsDeviceManager getGraphicsManager() {
        return graphics;
    }
}
