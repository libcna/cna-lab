import { Game, GraphicsDeviceManager, SpriteBatch, Texture2D, Vector2, Color, GameTime } from "@openeggbert/cna-js";

export class HelloGame extends Game {
    private graphics: GraphicsDeviceManager;
    private spriteBatch!: SpriteBatch;
    private logo!: Texture2D;
    private position: Vector2 = new Vector2(0, 0);

    constructor() {
        super();
        this.graphics = new GraphicsDeviceManager(this);
        this.content.rootDirectory = "Content";
    }

    protected initialize(): void {
        this.graphics.preferredBackBufferWidth = 1280;
        this.graphics.preferredBackBufferHeight = 720;
        this.graphics.applyChanges();
        super.initialize();
    }

    protected loadContent(): void {
        this.spriteBatch = new SpriteBatch(this.graphicsDevice);
        this.logo = this.content.load<Texture2D>(Texture2D, "logo");
    }

    protected update(gameTime: GameTime): void {
        const time = gameTime.totalGameTime.totalSeconds;
        this.position.x = 640 + 200 * Math.sin(time);
        this.position.y = 360 + 200 * Math.cos(time);
        super.update(gameTime);
    }

    protected draw(gameTime: GameTime): void {
        this.graphicsDevice.clear(Color.cornflowerBlue);
        this.spriteBatch.begin();
        this.spriteBatch.draw(this.logo, this.position, Color.white);
        this.spriteBatch.end();
        super.draw(gameTime);
    }
}
