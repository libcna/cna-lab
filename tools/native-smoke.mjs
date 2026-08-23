#!/usr/bin/env node

import assert from "node:assert/strict";
import path from "node:path";

import {
  Color,
  Game,
  Graphics,
  GraphicsDeviceManager,
  GetRuntimeStatus,
  Input,
  LoadNodeNativeBackend,
} from "cna-ts";

const library = process.env.CNA_NATIVE_LIBRARY;
const bridge = process.env.CNA_NODE_BRIDGE;
if (!library || !bridge) {
  throw new Error("set CNA_NATIVE_LIBRARY and CNA_NODE_BRIDGE to explicit absolute paths");
}
const frames = Number.parseInt(process.env.CNA_NATIVE_FRAMES ?? "60", 10);
if (!Number.isSafeInteger(frames) || frames <= 0) throw new Error("CNA_NATIVE_FRAMES must be positive");

const status = await LoadNodeNativeBackend({
  CnaLibrary: path.resolve(library),
  BridgeModule: path.resolve(bridge),
});
assert.equal(status.AbiVersion, "0.7.0");

class NativeSmokeGame extends Game {
  #frames = 0;
  #texture = null;

  constructor() {
    super();
    this.Manager = new GraphicsDeviceManager(this);
  }

  LoadContent() {
    this.#texture = new Graphics.Texture2D(this.GraphicsDevice, 4, 4);
  }

  Update(gameTime) {
    super.Update(gameTime);
    assert.ok(Input.Keyboard.GetState().GetPressedKeys().length >= 0);
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    assert.equal(this.#texture.GraphicsDevice, this.GraphicsDevice);
    this.#frames += 1;
    if (this.#frames >= frames) this.Exit();
    super.Draw(gameTime);
  }

  UnloadContent() {
    this.#texture?.Dispose();
    this.#texture = null;
    super.UnloadContent();
  }

  get Frames() { return this.#frames; }
}

const game = new NativeSmokeGame();
await game.Run();
assert.equal(game.Frames, frames);
game.Dispose();
const finalStatus = GetRuntimeStatus();
console.log(`CNA_TS_NODE_NATIVE_SMOKE=PASS FRAMES=${frames} RENDERER=${finalStatus.RendererInfo?.Name ?? "unavailable"}`);
