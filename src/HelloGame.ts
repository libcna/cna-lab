import {
  Color,
  FrameworkDispatcher,
  Game,
  GameComponent,
  type GameTime,
  Graphics,
  GraphicsDeviceManager,
  Input,
  Matrix,
  PlayerIndex,
  Rectangle,
  Vector2,
} from "cna-ts";

// A tiny, freely reproducible 1x1 PNG. Raw images belong to Texture2D.FromStream,
// not ContentManager.Load (which is reserved for compiled XNB assets).
const SPRITE_PNG = Uint8Array.from([
  137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82,
  0, 0, 0, 1, 0, 0, 0, 1, 8, 4, 0, 0, 0, 181, 28, 12, 2,
  0, 0, 0, 11, 73, 68, 65, 84, 120, 218, 99, 100, 248, 15, 0,
  1, 5, 1, 1, 39, 24, 227, 102, 0, 0, 0, 0, 73, 69, 78, 68,
  174, 66, 96, 130,
]);

class MotionService {
  public readonly SpeedScale = 1;
}

class ManagedCanaryComponent extends GameComponent {
  public InitializeCount = 0;
  public UpdateCount = 0;

  public override Initialize(): void {
    this.InitializeCount += 1;
    super.Initialize();
  }

  public override Update(gameTime: GameTime): void {
    void gameTime;
    this.UpdateCount += 1;
    super.Update(gameTime);
  }
}

/**
 * Portable managed canary plus a real XNA-style CNA 2D game when a native backend
 * is explicitly loaded. SimulateFrame never claims or requires native graphics.
 */
export class HelloGame extends Game {
  #position = Vector2.Zero;
  #velocity = new Vector2(48, 32);
  #updateCount = 0;
  #drawCount = 0;
  #nativeInputPollCount = 0;
  #nativeRunning = false;
  #nativeResourcesDisposed = false;
  #managedInitialized = false;
  #texture: Graphics.Texture2D | null = null;
  #spriteBatch: Graphics.SpriteBatch | null = null;
  readonly #motionService = new MotionService();
  readonly #component: ManagedCanaryComponent;
  readonly #keyboard = new Input.KeyboardState([Input.Keys.Space]);

  /** Set before Run to stop the native demo after an exact number of real draw frames. */
  public NativeFrameTarget: number | null = null;

  public constructor() {
    super();
    new GraphicsDeviceManager(this);
    this.Services.AddService(MotionService, this.#motionService);
    this.#component = new ManagedCanaryComponent(this);
    this.Components.Add(this.#component);
  }

  public get Position(): Vector2 {
    return new Vector2(this.#position.X, this.#position.Y);
  }

  public get UpdateCount(): number {
    return this.#updateCount;
  }

  public get DrawCount(): number { return this.#drawCount; }
  public get NativeInputPollCount(): number { return this.#nativeInputPollCount; }
  public get NativeResourcesDisposed(): boolean { return this.#nativeResourcesDisposed; }

  public get ComponentInitializeCount(): number { return this.#component.InitializeCount; }
  public get ComponentUpdateCount(): number { return this.#component.UpdateCount; }
  public get ServiceRoundTrip(): boolean {
    return this.Services.GetService(MotionService) === this.#motionService;
  }
  public get SpacePressed(): boolean { return this.#keyboard.IsKeyDown(Input.Keys.Space); }

  public get RotatedUnitX(): Vector2 {
    return Vector2.Transform(Vector2.UnitX, Matrix.CreateRotationZ(Math.PI / 2));
  }

  /** Executes deterministic update logic without claiming native Game.Run support. */
  public SimulateFrame(gameTime: GameTime): void {
    if (!this.#managedInitialized) {
      this.Initialize();
      this.#managedInitialized = true;
    }
    this.Update(gameTime);
  }

  protected override LoadContent(): void {
    this.#texture = Graphics.Texture2D.FromStream(this.GraphicsDevice, SPRITE_PNG);
    this.#spriteBatch = new Graphics.SpriteBatch(this.GraphicsDevice);
    this.#nativeResourcesDisposed = false;
    super.LoadContent();
  }

  protected override BeginRun(): void {
    this.#nativeRunning = true;
    super.BeginRun();
  }

  protected override Update(gameTime: GameTime): void {
    if (this.#nativeRunning) {
      FrameworkDispatcher.Update();
      const keyboard = Input.Keyboard.GetState();
      const mouse = Input.Mouse.GetState();
      const gamePad = Input.GamePad.GetState(PlayerIndex.One);
      // Poll the real CNA input routes while keeping autonomous motion deterministic
      // for HEADLESS test runs where no device is connected.
      void keyboard.GetPressedKeys();
      void mouse.X;
      void gamePad.IsConnected;
      this.#nativeInputPollCount += 1;
    }
    const seconds = gameTime.ElapsedGameTime.TotalSeconds;
    const scale = this.#motionService.SpeedScale;
    this.#position = Vector2.Add(
      this.#position,
      Vector2.Multiply(this.#velocity, seconds * scale),
    );
    this.#updateCount += 1;
    super.Update(gameTime);
  }

  protected override Draw(gameTime: GameTime): void {
    if (this.#texture == null || this.#spriteBatch == null) {
      throw new Error("Native sprite resources were not loaded");
    }
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.#spriteBatch.Begin();
    this.#spriteBatch.Draw(
      this.#texture,
      new Rectangle(Math.floor(this.#position.X) % 96, Math.floor(this.#position.Y) % 64, 12, 12),
      Color.White,
    );
    this.#spriteBatch.End();
    this.#drawCount += 1;
    if (this.NativeFrameTarget != null && this.#drawCount >= this.NativeFrameTarget) this.Exit();
    super.Draw(gameTime);
  }

  protected override UnloadContent(): void {
    this.#spriteBatch?.Dispose();
    this.#spriteBatch = null;
    this.#texture?.Dispose();
    this.#texture = null;
    this.#nativeResourcesDisposed = true;
    super.UnloadContent();
  }
}
