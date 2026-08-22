import {
  Game,
  GameComponent,
  type GameTime,
  Input,
  Matrix,
  Vector2,
} from "cna-ts";

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
 * Small portable game-state slice. It deliberately uses only strict XNA names and
 * remains runnable as a managed canary while a real CNA backend artifact is absent.
 */
export class HelloGame extends Game {
  #position = Vector2.Zero;
  #velocity = new Vector2(48, 32);
  #updateCount = 0;
  #managedInitialized = false;
  readonly #motionService = new MotionService();
  readonly #component: ManagedCanaryComponent;
  readonly #keyboard = new Input.KeyboardState([Input.Keys.Space]);

  public constructor() {
    super();
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

  protected override Update(gameTime: GameTime): void {
    const seconds = gameTime.ElapsedGameTime.TotalSeconds;
    const scale = this.#motionService.SpeedScale;
    this.#position = Vector2.Add(
      this.#position,
      Vector2.Multiply(this.#velocity, seconds * scale),
    );
    this.#updateCount += 1;
    super.Update(gameTime);
  }
}
