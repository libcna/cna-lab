import {
    Game,
    GraphicsDeviceManager,
    SpriteBatch,
    Texture2D,
    Vector2,
    Vector3,
    Color,
    GameTime,
    Matrix,
    BasicEffect,
    VertexPositionTexture,
    PrimitiveType,
    GraphicsDeviceCapability,
    BlendState,
    DepthStencilState,
    RasterizerState
} from "@openeggbert/cna-js";
import { RendererBanner } from "./RendererBanner";

export class HelloGame extends Game {
    private graphics: GraphicsDeviceManager;
    private spriteBatch!: SpriteBatch;
    private logo!: Texture2D;
    private solidTexture!: Texture2D;
    private cubeEffect!: BasicEffect;
    private cubeVertices!: VertexPositionTexture[];

    private position: Vector2 = new Vector2(0, 0);
    private velocity: Vector2 = new Vector2(104, 74);
    private rendererName: string = "Unknown";
    private animationSeconds: number = 0;
    private supportsThreeD: boolean = false;
    private supportsDepth: boolean = false;
    private smokeTest: boolean = false;
    private drawnFrames: number = 0;

    constructor() {
        super();
        this.graphics = new GraphicsDeviceManager(this);
        this.Content.RootDirectory = "Content";
        
        // Parse smoke test flag from window or arguments
        if (typeof window !== 'undefined' && window.location.search.includes('smoke-test')) {
            this.smokeTest = true;
        }
    }

    protected Initialize(): void {
        this.graphics.PreferredBackBufferWidth = 1280;
        this.graphics.PreferredBackBufferHeight = 720;
        this.graphics.ApplyChanges();
        super.Initialize();
    }

    protected LoadContent(): void {
        this.spriteBatch = new SpriteBatch(this.GraphicsDevice);
        this.logo = this.Content.Load<Texture2D>(Texture2D, "logo");
        
        // Create 1x1 white texture for banner
        this.solidTexture = new Texture2D(this.GraphicsDevice, 1, 1);
        this.solidTexture.setData([new Color(255, 255, 255, 255)]);

        this.rendererName = this.GraphicsDevice.rendererName;
        this.supportsThreeD = this.GraphicsDevice.graphicsCapabilities.supportsCapability(GraphicsDeviceCapability.ThreeD);
        this.supportsDepth = this.GraphicsDevice.graphicsCapabilities.supportsCapability(GraphicsDeviceCapability.DepthStencilBuffer);

        if (this.supportsThreeD) {
            this.cubeEffect = new BasicEffect(this.GraphicsDevice);
            this.cubeEffect.textureEnabled = true;
            this.cubeEffect.texture = this.logo;
            this.CreateCubeVertices();
        }

        const viewport = this.GraphicsDevice.viewport;
        this.position = new Vector2(viewport.width / 2, viewport.height / 2);

        console.log(`cna-js-template: renderer ${this.rendererName}`);
        console.log(`  3D pipeline     : ${this.supportsThreeD ? "yes" : "no (2D only)"}`);
        console.log(`  depth/stencil   : ${this.supportsDepth ? "yes" : "no"}`);
    }

    private CreateCubeVertices(): void {
        this.cubeVertices = [];
        const addFace = (topLeft: Vector3, topRight: Vector3, bottomRight: Vector3, bottomLeft: Vector3) => {
            this.cubeVertices.push(new VertexPositionTexture(topLeft, new Vector2(0, 0)));
            this.cubeVertices.push(new VertexPositionTexture(topRight, new Vector2(1, 0)));
            this.cubeVertices.push(new VertexPositionTexture(bottomRight, new Vector2(1, 1)));
            this.cubeVertices.push(new VertexPositionTexture(topLeft, new Vector2(0, 0)));
            this.cubeVertices.push(new VertexPositionTexture(bottomRight, new Vector2(1, 1)));
            this.cubeVertices.push(new VertexPositionTexture(bottomLeft, new Vector2(0, 1)));
        };

        addFace(new Vector3(-1, 1, 1), new Vector3(1, 1, 1), new Vector3(1, -1, 1), new Vector3(-1, -1, 1));
        addFace(new Vector3(1, 1, -1), new Vector3(-1, 1, -1), new Vector3(-1, -1, -1), new Vector3(1, -1, -1));
        addFace(new Vector3(1, 1, 1), new Vector3(1, 1, -1), new Vector3(1, -1, -1), new Vector3(1, -1, 1));
        addFace(new Vector3(-1, 1, -1), new Vector3(-1, 1, 1), new Vector3(-1, -1, 1), new Vector3(-1, -1, -1));
        addFace(new Vector3(-1, 1, -1), new Vector3(1, 1, -1), new Vector3(1, 1, 1), new Vector3(-1, 1, 1));
        addFace(new Vector3(-1, -1, 1), new Vector3(1, -1, 1), new Vector3(1, -1, -1), new Vector3(-1, -1, -1));
    }

    protected Update(gameTime: GameTime): void {
        const dt = gameTime.elapsedGameTime.totalSeconds;
        this.animationSeconds += dt;

        if (!this.supportsThreeD) {
            const movementDelta = Math.min(dt, 0.1);
            this.position.x += this.velocity.x * movementDelta;
            this.position.y += this.velocity.y * movementDelta;

            const viewport = this.graphicsDevice.viewport;
            const logoWidth = this.logo.width;
            const logoHeight = this.logo.height;
            const halfExtent = Math.max(logoWidth, logoHeight) * 0.5 * 1.08 * 1.15;

            const minX = Math.min(halfExtent, viewport.width * 0.5);
            const minY = Math.min(halfExtent, viewport.height * 0.5);
            const maxX = Math.max(minX, viewport.width - minX);
            const maxY = Math.max(minY, viewport.height - minY);

            if (this.position.x < minX) {
                this.position.x = minX;
                this.velocity.x = Math.abs(this.velocity.x);
            } else if (this.position.x > maxX) {
                this.position.x = maxX;
                this.velocity.x = -Math.abs(this.velocity.x);
            }

            if (this.position.y < minY) {
                this.position.y = minY;
                this.velocity.y = Math.abs(this.velocity.y);
            } else if (this.position.y > maxY) {
                this.position.y = maxY;
                this.velocity.y = -Math.abs(this.velocity.y);
            }
        }

        super.Update(gameTime);
    }

    protected Draw(gameTime: GameTime): void {
        if (this.supportsThreeD && this.supportsDepth) {
            this.GraphicsDevice.Clear(Color.CornflowerBlue, 1.0, 0);
        } else {
            this.GraphicsDevice.Clear(Color.CornflowerBlue);
        }

        if (this.supportsThreeD) {
            this.Draw3D(gameTime);
        } else {
            this.Draw2D(gameTime);
        }

        if (this.animationSeconds < 5.0) {
            this.spriteBatch.Begin();
            RendererBanner.Draw(this.spriteBatch, this.solidTexture, this.rendererName, this.GraphicsDevice.Viewport.Width);
            this.spriteBatch.End();
        }

        if (this.smokeTest && ++this.drawnFrames >= 3) {
            console.log(`cna-js-template: smoke test drew ${this.drawnFrames} frames; exiting`);
            this.Exit();
        }

        super.Draw(gameTime);
    }

    private Draw2D(gameTime: GameTime): void {
        const motionSeconds = this.animationSeconds * 2.0;
        const scale = 0.96 + 0.12 * Math.sin(motionSeconds * 0.65);
        const rotation = 0.11 * Math.sin(motionSeconds * 0.55);
        const origin = new Vector2(this.logo.width / 2, this.logo.height / 2);

        this.spriteBatch.Begin();
        this.spriteBatch.Draw(
            this.logo,
            this.position,
            null,
            Color.White,
            rotation,
            origin,
            scale
        );
        this.spriteBatch.End();
    }

    private Draw3D(gameTime: GameTime): void {
        const viewport = this.GraphicsDevice.Viewport;
        const aspectRatio = viewport.Width / (viewport.Height || 1);
        const motionSeconds = this.animationSeconds * 2.0;
        const scale = 0.88 + 0.10 * Math.sin(motionSeconds * 0.48);
        const moveX = 1.15 * Math.sin(motionSeconds * 0.24);
        const moveY = 0.70 * Math.sin(motionSeconds * 0.19 + 1.1);

        const world = Matrix.createScale(scale)
            .multiply(Matrix.createRotationX(motionSeconds * 0.22))
            .multiply(Matrix.createRotationY(motionSeconds * 0.31))
            .multiply(Matrix.createRotationZ(motionSeconds * 0.13))
            .multiply(Matrix.createTranslation(moveX, moveY, 0));

        const view = Matrix.createLookAt(new Vector3(0, 0, 6), new Vector3(0, 0, 0), Vector3.up);
        const projection = Matrix.createPerspectiveFieldOfView(Math.PI / 4, aspectRatio, 0.1, 100);

        this.cubeEffect.World = world;
        this.cubeEffect.View = view;
        this.cubeEffect.Projection = projection;

        this.GraphicsDevice.BlendState = BlendState.Opaque;
        this.GraphicsDevice.DepthStencilState = this.supportsDepth ? DepthStencilState.Default : DepthStencilState.None;
        this.GraphicsDevice.RasterizerState = RasterizerState.CullNone;

        for (const pass of this.cubeEffect.CurrentTechnique.Passes) {
            pass.Apply();
            this.GraphicsDevice.DrawUserPrimitives(
                PrimitiveType.TriangleList,
                this.cubeVertices,
                0,
                12
            );
        }
    }

    public SetSmokeTest(enabled: boolean): void {
        this.smokeTest = enabled;
    }
}
