#!/usr/bin/env node
/**
 * Upstream finding 11's sequence, run where a regression can only kill itself.
 *
 * Opening CNA's test-backend camera, destroying it, and then opening the platform's own camera was
 * a use-after-free in CNA 0.21.0 (a process-wide provider left dangling by the destroy). CNA fixed
 * it; this prints SURVIVED and exits 0 on a repaired artifact.
 */
import path from "node:path";

import { Game, GraphicsDeviceManager, LoadNodeNativeBackend } from "../../dist/index.js";
import { CnaCamera } from "../../dist/extensions/devices/index.js";

await LoadNodeNativeBackend({
  CnaLibrary: path.resolve(process.env.CNA_NATIVE_LIBRARY),
  BridgeModule: path.resolve(process.env.CNA_NODE_BRIDGE ?? "build/cna_node_bridge.node"),
});

class CrashProbe extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
  }

  LoadContent() {
    const test = CnaCamera.OpenForTests();
    test.Dispose();
    const platform = CnaCamera.Open();
    platform.Dispose();
    console.log("SURVIVED");
    this.Exit();
    super.LoadContent();
  }

  Draw(gameTime) {
    this.Exit();
    super.Draw(gameTime);
  }
}

const game = new CrashProbe();
await game.Run();
game.Dispose();
