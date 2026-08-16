package com.openeggbert.cna.template;

import Microsoft.Xna.Framework.*;
import Microsoft.Xna.Framework.Graphics.*;
import Microsoft.Xna.Framework.Input.*;
import java.util.ArrayList;
import java.util.List;

public class HelloGame extends Game {
    private GraphicsDeviceManager _graphics;
    private SpriteBatch _spriteBatch;
    private BasicEffect _cubeEffect;
    private Texture2D _logo;
    private Texture2D _solid;
    private Vector2 _position;
    private Vector2 _velocity;
    private String _rendererName = "Unknown";
    private float _animationSeconds;
    private boolean _supports3D;
    private boolean _supportsDepth;
    private final boolean _smokeTest;
    private int _drawnFrames;

    private static final float MAXIMUM_2D_LOGO_SCALE = 1.08f;
    private static final float RENDERER_BANNER_SECONDS = 5.0f;
    private static final float ANIMATION_SPEED = 2.0f;
    private static final int SMOKE_TEST_FRAMES = 3;

    private static final VertexPositionTexture[] CUBE_VERTICES = createLogoCubeVertices();

    public HelloGame() {
        this(false);
    }

    public HelloGame(boolean smokeTest) {
        _graphics = new GraphicsDeviceManager(this);
        getContent().setRootDirectory("Content");
        setIsMouseVisible(true);
        _smokeTest = smokeTest;
        // getWindow().setTitle("cna-java-template - HelloGame");
    }

    @Override
    protected void Initialize() {
        super.Initialize();
    }

    @Override
    protected void LoadContent() {
        _spriteBatch = new SpriteBatch(getGraphicsDevice());
        _logo = getContent().load(Texture2D.class, "logo");

        _solid = new Texture2D(getGraphicsDevice(), 1, 1);
        _solid.setData(new Color[] { Color.WHITE });

        _rendererName = getRendererName();
        // getWindow().setTitle("cna-java-template - HelloGame (" + _rendererName + ")");

        _supports3D = supportsCapability("ThreeD");
        _supportsDepth = supportsCapability("DepthStencilBuffer");

        if (_supports3D) {
            _cubeEffect = new BasicEffect(getGraphicsDevice());
            _cubeEffect.setTextureEnabled(true);
            _cubeEffect.setTexture(_logo);
            _cubeEffect.setLightingEnabled(false);
            _cubeEffect.setVertexColorEnabled(false);
        }

        Viewport viewport = getGraphicsDevice().getViewport();
        _position = new Vector2(viewport.getWidth() * 0.5f, viewport.getHeight() * 0.5f);
        _velocity = new Vector2(104.0f, 74.0f);

        reportRendererCapabilities();
    }

    private String getRendererName() {
        try {
            // Assume GraphicsDevice might have a way to get name
            return "CNA (Java)";
        } catch (Exception e) {
            return "Java/XNA";
        }
    }

    private boolean supportsCapability(String capability) {
        // Future-proof: assume Reach/HiDef capability logic
        if ("ThreeD".equals(capability)) return true;
        if ("DepthStencilBuffer".equals(capability)) return true;
        return false;
    }

    private void reportRendererCapabilities() {
        System.out.println("cna-java-template: renderer " + _rendererName);
        System.out.println("  3D pipeline     : " + (_supports3D ? "yes" : "no (2D only)"));
        System.out.println("  depth/stencil   : " + (_supportsDepth ? "yes" : "no"));
    }

    @Override
    protected void Update(GameTime gameTime) {
        if (Keyboard.getState().isKeyDown(Keys.ESCAPE)) {
            Exit();
            return;
        }

        float deltaTime = (float) gameTime.getElapsedGameTime().getTotalSeconds();
        _animationSeconds += deltaTime;

        if (!_supports3D) {
            float movementDelta = Math.min(deltaTime, 0.1f);
            _position.x += _velocity.x * movementDelta;
            _position.y += _velocity.y * movementDelta;

            Viewport viewport = getGraphicsDevice().getViewport();
            float logoWidth = _logo.getWidth();
            float logoHeight = _logo.getHeight();
            float logoSize = Math.max(logoWidth, logoHeight);
            float halfExtent = logoSize * 0.5f * MAXIMUM_2D_LOGO_SCALE * 1.15f;

            float minX = Math.min(halfExtent, viewport.getWidth() * 0.5f);
            float minY = Math.min(halfExtent, viewport.getHeight() * 0.5f);
            float maxX = Math.max(minX, viewport.getWidth() - minX);
            float maxY = Math.max(minY, viewport.getHeight() - minY);

            if (_position.x < minX) { _position.x = minX; _velocity.x = Math.abs(_velocity.x); }
            else if (_position.x > maxX) { _position.x = maxX; _velocity.x = -Math.Abs(_velocity.x); }

            if (_position.y < minY) { _position.y = minY; _velocity.y = Math.abs(_velocity.y); }
            else if (_position.y > maxY) { _position.y = maxY; _velocity.y = -Math.Abs(_velocity.y); }
        }

        super.Update(gameTime);
    }

    @Override
    protected void Draw(GameTime gameTime) {
        if (_supports3D && _supportsDepth) {
            getGraphicsDevice().clear(ClearOptions.TARGET | ClearOptions.DEPTH_BUFFER, Color.CORNFLOWER_BLUE, 1.0f, 0);
        } else {
            getGraphicsDevice().clear(Color.CORNFLOWER_BLUE);
        }

        if (_supports3D) {
            draw3DLogoCube();
        } else {
            draw2DLogo();
        }

        if (_animationSeconds < RENDERER_BANNER_SECONDS) {
            drawRendererBanner();
        }

        super.Draw(gameTime);

        if (_smokeTest && ++_drawnFrames >= SMOKE_TEST_FRAMES) {
            System.out.println("cna-java-template: smoke test drew " + _drawnFrames + " frames; exiting");
            Exit();
        }
    }

    private void draw2DLogo() {
        float motionSeconds = _animationSeconds * ANIMATION_SPEED;
        float scale = 0.96f + 0.12f * (float) Math.sin(motionSeconds * 0.65f);
        float rotation = 0.11f * (float) Math.sin(motionSeconds * 0.55f);
        Vector2 origin = new Vector2(_logo.getWidth() * 0.5f, _logo.getHeight() * 0.5f);

        _spriteBatch.begin();
        _spriteBatch.draw(_logo, _position, null, Color.WHITE, rotation, origin, scale, SpriteEffects.NONE, 0f);
        _spriteBatch.end();
    }

    private void draw3DLogoCube() {
        Viewport viewport = getGraphicsDevice().getViewport();
        float aspectRatio = (float) viewport.getWidth() / Math.max(1, viewport.getHeight());
        float motionSeconds = _animationSeconds * ANIMATION_SPEED;
        float scale = 0.88f + 0.10f * (float) Math.sin(motionSeconds * 0.48f);
        float moveX = 1.15f * (float) Math.sin(motionSeconds * 0.24f);
        float moveY = 0.70f * (float) Math.sin(motionSeconds * 0.19f + 1.1f);

        _cubeEffect.setWorld(Matrix.multiply(
                Matrix.createScale(scale),
                Matrix.multiply(
                        Matrix.createRotationX(motionSeconds * 0.22f),
                        Matrix.multiply(
                                Matrix.createRotationY(motionSeconds * 0.31f),
                                Matrix.multiply(
                                        Matrix.createRotationZ(motionSeconds * 0.13f),
                                        Matrix.createTranslation(moveX, moveY, 0.0f)
                                )
                        )
                )
        ));
        _cubeEffect.setView(Matrix.createLookAt(new Vector3(0.0f, 0.0f, 6.0f), Vector3.ZERO, Vector3.UP));
        _cubeEffect.setProjection(Matrix.createPerspectiveFieldOfView(0.78539816339f, aspectRatio, 0.1f, 100.0f));

        getGraphicsDevice().setBlendState(BlendState.OPAQUE);
        getGraphicsDevice().setDepthStencilState(_supportsDepth ? DepthStencilState.DEFAULT : DepthStencilState.NONE);
        getGraphicsDevice().setRasterizerState(RasterizerState.CULL_NONE);

        for (EffectPass pass : _cubeEffect.getCurrentTechnique().getPasses()) {
            pass.apply();
            getGraphicsDevice().drawUserPrimitives(PrimitiveType.TRIANGLE_LIST, CUBE_VERTICES, 0, CUBE_VERTICES.length / 3);
        }
    }

    private void drawRendererBanner() {
        Viewport viewport = getGraphicsDevice().getViewport();
        int viewportWidth = viewport.getWidth();
        int glyphColumns = Math.max(1, _rendererName.length() * 6 - 1);
        int pixelSize = Math.max(1, Math.min((viewportWidth - 48) / glyphColumns, 8));
        int textWidth = glyphColumns * pixelSize;
        int textHeight = 7 * pixelSize;
        int textX = (viewportWidth - textWidth) / 2;
        int textY = 28;
        int padding = 14;
        Color translucentWhite = new Color(255, 255, 255, 96);

        _spriteBatch.begin();
        _spriteBatch.draw(_solid, new Rectangle(textX - padding, textY - padding, textWidth + padding * 2, textHeight + padding * 2), translucentWhite);

        for (int i = 0; i < _rendererName.length(); i++) {
            byte[] rows = getGlyphRows(_rendererName.charAt(i));
            int charX = textX + i * 6 * pixelSize;

            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if ((rows[row] & (1 << (4 - col))) != 0) {
                        _spriteBatch.draw(_solid, new Rectangle(charX + col * pixelSize, textY + row * pixelSize, pixelSize, pixelSize), new Color(24, 36, 55, 255));
                    }
                }
            }
        }
        _spriteBatch.end();
    }

    private static byte[] getGlyphRows(char c) {
        switch (Character.toUpperCase(c)) {
            case 'A': return new byte[] { 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };
            case 'B': return new byte[] { 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e };
            case 'C': return new byte[] { 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e };
            case 'D': return new byte[] { 0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e };
            case 'E': return new byte[] { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f };
            case 'F': return new byte[] { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10 };
            case 'G': return new byte[] { 0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0e };
            case 'H': return new byte[] { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };
            case 'I': return new byte[] { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1f };
            case 'J': return new byte[] { 0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0c };
            case 'K': return new byte[] { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 };
            case 'L': return new byte[] { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f };
            case 'M': return new byte[] { 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11 };
            case 'N': return new byte[] { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 };
            case 'O': return new byte[] { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };
            case 'P': return new byte[] { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 };
            case 'Q': return new byte[] { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d };
            case 'R': return new byte[] { 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11 };
            case 'S': return new byte[] { 0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e };
            case 'T': return new byte[] { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };
            case 'U': return new byte[] { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };
            case 'V': return new byte[] { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04 };
            case 'W': return new byte[] { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a };
            case 'X': return new byte[] { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 };
            case 'Y': return new byte[] { 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 };
            case 'Z': return new byte[] { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f };
            case '0': return new byte[] { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e };
            case '1': return new byte[] { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e };
            case '2': return new byte[] { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f };
            case '3': return new byte[] { 0x1e, 0x01, 0x01, 0x0e, 0x01, 0x01, 0x1e };
            case '4': return new byte[] { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02 };
            case '5': return new byte[] { 0x1f, 0x10, 0x10, 0x1e, 0x01, 0x01, 0x1e };
            case '6': return new byte[] { 0x0e, 0x10, 0x10, 0x1e, 0x11, 0x11, 0x0e };
            case '7': return new byte[] { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 };
            case '8': return new byte[] { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e };
            case '9': return new byte[] { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x0e };
            case '_': return new byte[] { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f };
            case '-': return new byte[] { 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00 };
            case ' ': return new byte[] { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            case '(': return new byte[] { 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x02 };
            case ')': return new byte[] { 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08 };
            case '.': return new byte[] { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };
            default: return new byte[] { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 };
        }
    }

    private static VertexPositionTexture[] createLogoCubeVertices() {
        List<VertexPositionTexture> vertices = new ArrayList<>();

        addFace(vertices, new Vector3(-1, 1, 1), new Vector3(1, 1, 1), new Vector3(1, -1, 1), new Vector3(-1, -1, 1));
        addFace(vertices, new Vector3(1, 1, -1), new Vector3(-1, 1, -1), new Vector3(-1, -1, -1), new Vector3(1, -1, -1));
        addFace(vertices, new Vector3(1, 1, 1), new Vector3(1, 1, -1), new Vector3(1, -1, -1), new Vector3(1, -1, 1));
        addFace(vertices, new Vector3(-1, 1, -1), new Vector3(-1, 1, 1), new Vector3(-1, -1, 1), new Vector3(-1, -1, -1));
        addFace(vertices, new Vector3(-1, 1, -1), new Vector3(1, 1, -1), new Vector3(1, 1, 1), new Vector3(-1, 1, 1));
        addFace(vertices, new Vector3(-1, -1, 1), new Vector3(1, -1, 1), new Vector3(1, -1, -1), new Vector3(-1, -1, -1));

        return vertices.toArray(new VertexPositionTexture[0]);
    }

    private static void addFace(List<VertexPositionTexture> vertices, Vector3 tl, Vector3 tr, Vector3 br, Vector3 bl) {
        vertices.add(new VertexPositionTexture(tl, new Vector2(0, 0)));
        vertices.add(new VertexPositionTexture(tr, new Vector2(1, 0)));
        vertices.add(new VertexPositionTexture(br, new Vector2(1, 1)));
        vertices.add(new VertexPositionTexture(tl, new Vector2(0, 0)));
        vertices.add(new VertexPositionTexture(br, new Vector2(1, 1)));
        vertices.add(new VertexPositionTexture(bl, new Vector2(0, 1)));
    }
}
