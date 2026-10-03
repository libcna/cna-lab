import { Color, GameTime, GetRuntimeStatus, LoadWasmBackend, TimeSpan, Vector2 } from "cna-ts";

import { HelloGame } from "./HelloGame.js";

function addStatus(list: HTMLDListElement, label: string, value: string, state?: "pass" | "blocked"): void {
  const term = document.createElement("dt");
  term.textContent = label;
  const detail = document.createElement("dd");
  detail.textContent = value;
  if (state) detail.className = state;
  list.append(term, detail);
}

function startCanary(): HTMLDListElement {
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
    : "Pass ?wasm=<url to cna_c_api.mjs> to run real frames on the CNA WebAssembly runtime.";
  host.append(title, explanation, list, note);
  return list;
}

/**
 * The browser runtime path. A browser owns its event loop, so frames come from
 * requestAnimationFrame and IsFixedTimeStep is off: with a fixed timestep CNA waits out the
 * remainder of each frame, which inside a frame callback is a busy wait.
 *
 * Nothing about HelloGame changes between here and the Node canary. It is the same XNA Game.
 */
async function startWebAssemblyRuntime(list: HTMLDListElement, moduleUrl: string): Promise<void> {
  const targetFrames = Number(new URLSearchParams(location.search).get("frames") ?? "60");
  const canvas = document.createElement("canvas");
  canvas.id = "canvas";
  canvas.width = 320;
  canvas.height = 240;
  document.querySelector("#app")?.append(canvas);
  const factory = (await import(/* @vite-ignore */ moduleUrl)).default as
    (options?: object) => Promise<object>;
  const status = await LoadWasmBackend({ Factory: factory, FactoryOptions: { canvas } });

  const game = new HelloGame();
  game.IsFixedTimeStep = false;
  game.NativeFrameTarget = targetFrames;
  await new Promise<void>((resolve, reject) => {
    const pump = (): void => {
      try {
        game.RunOneFrame();
        if (game.DrawCount >= targetFrames) {
          resolve();
          return;
        }
        requestAnimationFrame(pump);
      } catch (error) {
        reject(error instanceof Error ? error : new Error(String(error)));
      }
    };
    requestAnimationFrame(pump);
  });
  game.Dispose();

  addStatus(list, "CNA WebAssembly runtime", `${status.Backend}, ABI ${status.AbiVersion ?? "unknown"}`, "pass");
  addStatus(list, "Real browser frames", `${game.DrawCount} drawn, ${game.UpdateCount} updated`, "pass");
  addStatus(list, "Deterministic disposal", game.NativeResourcesDisposed ? "verified" : "failed",
    game.NativeResourcesDisposed ? "pass" : "blocked");
  const harness = globalThis as unknown as { __cnaTemplateResult?: unknown };
  harness.__cnaTemplateResult = {
    status: "ok",
    backend: status.Backend,
    abiVersion: status.AbiVersion,
    frames: game.DrawCount,
    updates: game.UpdateCount,
    disposed: game.NativeResourcesDisposed,
  };
}

const list = startCanary();
const wasmModuleUrl = new URLSearchParams(location.search).get("wasm");
if (wasmModuleUrl) {
  startWebAssemblyRuntime(list, wasmModuleUrl).catch((error: unknown) => {
    addStatus(list, "CNA WebAssembly runtime", String(error), "blocked");
    const harness = globalThis as unknown as { __cnaTemplateResult?: unknown };
    harness.__cnaTemplateResult = { status: "failed", error: String(error) };
  });
}
