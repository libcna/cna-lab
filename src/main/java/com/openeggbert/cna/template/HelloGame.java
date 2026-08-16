package com.openeggbert.cna.template;

import com.openeggbert.cna.framework.*;
import com.openeggbert.cna.framework.graphics.*;
import com.openeggbert.cna.framework.math.*;

public class HelloGame extends Game {
    private GraphicsDeviceManager graphics;
    private SpriteBatch spriteBatch;
    private Texture2D logo;
    private Vector2 position = new Vector2();

    public HelloGame() {
        graphics = new GraphicsDeviceManager(this);
        getContent().setRootDirectory("Content");
    }

    @Override
    protected void initialize() {
        graphics.setPreferredBackBufferWidth(1280);
        graphics.setPreferredBackBufferHeight(720);
        graphics.applyChanges();
        super.initialize();
    }

    @Override
    protected void loadContent() {
        spriteBatch = new SpriteBatch(getGraphicsDevice());
        logo = getContent().load(Texture2D.class, "logo");
    }

    @Override
    protected void update(GameTime gameTime) {
        float time = (float) gameTime.getTotalGameTime().getTotalSeconds();
        position.x = 640.0f + 200.0f * (float) Math.sin(time);
        position.y = 360.0f + 200.0f * (float) Math.cos(time);
        super.update(gameTime);
    }

    @Override
    protected void draw(GameTime gameTime) {
        getGraphicsDevice().clear(Color.CORNFLOWER_BLUE);
        spriteBatch.begin();
        spriteBatch.draw(logo, position, Color.WHITE);
        spriteBatch.end();
        super.draw(gameTime);
    }
}
