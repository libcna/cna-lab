import { Game, type GameTime, Vector2 } from "cna-ts";

/**
 * Small portable game-state slice. It deliberately uses only strict XNA names and
 * remains runnable as a managed canary while a real CNA backend artifact is absent.
 */
export class HelloGame extends Game {
  #position = Vector2.Zero;
  #velocity = new Vector2(48, 32);
  #updateCount = 0;

  public get Position(): Vector2 {
    return new Vector2(this.#position.X, this.#position.Y);
  }

  public get UpdateCount(): number {
    return this.#updateCount;
  }

  /** Executes deterministic update logic without claiming native Game.Run support. */
  public SimulateFrame(gameTime: GameTime): void {
    this.Update(gameTime);
  }

  protected override Update(gameTime: GameTime): void {
    const seconds = gameTime.ElapsedGameTime.TotalSeconds;
    this.#position = Vector2.Add(this.#position, Vector2.Multiply(this.#velocity, seconds));
    this.#updateCount += 1;
    super.Update(gameTime);
  }
}
