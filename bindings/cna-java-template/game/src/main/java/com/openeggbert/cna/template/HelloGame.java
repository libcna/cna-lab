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
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Base64;

/** Small truthful desktop canary for the currently implemented CNA-Java slice. */
public final class HelloGame extends Game {

    private final GraphicsDeviceManager graphics;
    private final int frameLimit;
    private SpriteBatch spriteBatch;
    private Texture2D logo;
    private Texture2D compiledLogo;
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
        compiledLogo = loadCompiledLogo();
        if (compiledLogo == logo) {
            throw new IllegalStateException("Raw PNG and XNB loads must produce distinct textures");
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
        spriteBatch.Draw(compiledLogo, Vector2.Add(logoPosition, new Vector2(0.0f, 32.0f)), Color.White);
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
        getContent().Unload();
        compiledLogo = null;
        if (logo != null) {
            logo.close();
        }
        super.UnloadContent();
    }

    private Texture2D loadCompiledLogo() {
        Path contentDirectory = null;
        Path xnbPath = null;
        try {
            contentDirectory = Files.createTempDirectory("cna-java-template-content-");
            xnbPath = contentDirectory.resolve("cna-logo.xnb");
            InputStream resource = HelloGame.class.getResourceAsStream("/cna-logo.xnb.base64");
            if (resource == null) {
                throw new IllegalStateException("Missing cna-logo.xnb.base64 resource");
            }
            try (resource; InputStream decoded = Base64.getMimeDecoder().wrap(resource)) {
                Files.copy(decoded, xnbPath);
            }
            getContent().setRootDirectory(contentDirectory.toString());
            Texture2D loaded = getContent().Load(Texture2D.class, "cna-logo");
            if (loaded != getContent().Load(Texture2D.class, "CNA-LOGO")) {
                throw new IllegalStateException("ContentManager cache did not preserve XNB texture identity");
            }
            return loaded;
        } catch (IOException exception) {
            throw new IllegalStateException("Could not prepare the generated XNB fixture", exception);
        } finally {
            try {
                if (xnbPath != null) {
                    Files.deleteIfExists(xnbPath);
                }
                if (contentDirectory != null) {
                    Files.deleteIfExists(contentDirectory);
                }
            } catch (IOException exception) {
                throw new IllegalStateException("Could not remove the generated XNB fixture", exception);
            }
        }
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
