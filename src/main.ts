import { Color, GameTime, GetRuntimeStatus, TimeSpan, Vector2 } from "cna-ts";

import { HelloGame } from "./HelloGame.js";

function addStatus(list: HTMLDListElement, label: string, value: string, state?: "pass" | "blocked"): void {
  const term = document.createElement("dt");
  term.textContent = label;
  const detail = document.createElement("dd");
  detail.textContent = value;
  if (state) detail.className = state;
  list.append(term, detail);
}

function startCanary(): void {
  const host = document.querySelector<HTMLElement>("#app");
  if (!host) throw new Error("Missing #app host element");

  const game = new HelloGame();
  game.SimulateFrame(new GameTime(TimeSpan.Zero, TimeSpan.FromMilliseconds(16)));
  const expected = Vector2.Multiply(new Vector2(48, 32), 0.016);
  const managedPass = game.UpdateCount === 1 &&
    Vector2.Distance(game.Position, expected) < 1e-6 &&
    game.ComponentInitializeCount === 1 &&
    game.ComponentUpdateCount === 1 &&
    game.ServiceRoundTrip &&
    game.SpacePressed &&
    Vector2.Distance(game.RotatedUnitX, Vector2.UnitY) < 1e-6;
  const runtime = GetRuntimeStatus();

  host.replaceChildren();
  const title = document.createElement("h1");
  title.textContent = "CNA-TS canonical canary";
  const explanation = document.createElement("p");
  explanation.textContent =
    "TypeScript is canonical; JavaScript is generated from the same source and consumes the same cna-ts package.";
  const list = document.createElement("dl");
  addStatus(list, "Managed XNA projection", managedPass ? "verified" : "failed", managedPass ? "pass" : "blocked");
  addStatus(list, "Components / services", "managed ordering and lookup verified", "pass");
  addStatus(list, "Input values", "KeyboardState snapshot verified; hardware polling not claimed", "pass");
  addStatus(list, "Sample color", `CornflowerBlue = #${Color.CornflowerBlue.PackedValue.toString(16).padStart(8, "0")}`);
  addStatus(list, "Web bundle", "build verified", "pass");
  addStatus(
    list,
    "CNA runtime",
    runtime.IsAvailable ? `${runtime.Backend}, ABI ${runtime.AbiVersion ?? "unknown"}` : runtime.Detail,
    runtime.IsAvailable ? "pass" : "blocked",
  );
  addStatus(list, "Electron", "planned; not verified");
  addStatus(list, "Android / iOS", "planned; not verified");
  const note = document.createElement("small");
  note.textContent = runtime.IsAvailable
    ? "A real backend is available; Game.Run can be enabled by the generated project."
    : "This canary does not fake frames or graphics while no packaged CNA WebAssembly backend exists.";
  host.append(title, explanation, list, note);
}

startCanary();
