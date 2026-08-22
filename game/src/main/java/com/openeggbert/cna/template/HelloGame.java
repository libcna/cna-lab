package com.openeggbert.cna.template;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameTime;
import Microsoft.Xna.Framework.GraphicsDeviceManager;

/** Small truthful desktop canary for the currently implemented CNA-Java slice. */
public final class HelloGame extends Game {

    private final GraphicsDeviceManager graphics;
    private final int frameLimit;
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
    protected void Update(GameTime gameTime) {
        super.Update(gameTime);
    }

    @Override
    protected void Draw(GameTime gameTime) {
        getGraphicsDevice().Clear(Color.CornflowerBlue);
        drawnFrames++;
        if (frameLimit > 0 && drawnFrames >= frameLimit) {
            Exit();
        }
        super.Draw(gameTime);
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
