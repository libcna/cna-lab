#!/usr/bin/env node

import assert from "node:assert/strict";
import path from "node:path";

import {
  GetRuntimeStatus,
  LoadNodeNativeBackend,
} from "cna-ts";

import { HelloGame } from "../build/native-smoke/HelloGame.js";

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

const game = new HelloGame();
game.NativeFrameTarget = frames;
await game.Run();
assert.equal(game.DrawCount, frames);
assert.equal(game.NativeInputPollCount, frames);
game.Dispose();
assert.equal(game.NativeResourcesDisposed, true);
const finalStatus = GetRuntimeStatus();
console.log(
  `CNA_TS_NODE_NATIVE_2D=PASS FRAMES=${frames} ` +
  `FROM_STREAM=PASS SPRITE_BATCH_DRAW=PASS INPUT=PASS ` +
  `RENDERER=${finalStatus.RendererInfo?.Name ?? "unavailable"}`,
);
