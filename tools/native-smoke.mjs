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
const extensionsSmoke = process.argv.includes("--extensions-smoke");

const status = await LoadNodeNativeBackend({
  CnaLibrary: path.resolve(library),
  BridgeModule: path.resolve(bridge),
});
// A template pins nothing about which CNA generation it consumes: the package rejects a library
// outside the window it targets, so what is left to check here is that a real one loaded and that
// it reported an actual version. CNA_EXPECTED_ABI asks for one exactly, for a pinned deployment.
assert.equal(status.Backend, "node-native");
assert.match(String(status.AbiVersion), /^\d+\.\d+\.\d+$/);
if (process.env.CNA_EXPECTED_ABI) assert.equal(status.AbiVersion, process.env.CNA_EXPECTED_ABI);

const game = new HelloGame();
game.NativeFrameTarget = frames;
await game.Run();
assert.equal(game.DrawCount, frames);
assert.equal(game.NativeInputPollCount, frames);
game.Dispose();
assert.equal(game.NativeResourcesDisposed, true);
const finalStatus = GetRuntimeStatus();
console.log(
  `CNA_TS_NODE_NATIVE_2D=PASS FRAMES=${frames} ABI=${status.AbiVersion} ` +
  `FROM_STREAM=PASS SPRITE_BATCH_DRAW=PASS INPUT=PASS ` +
  `RENDERER=${finalStatus.RendererInfo?.Name ?? "unavailable"}`,
);

if (extensionsSmoke) {
  // Opt-in: the modern CNA surface a game does not need in order to be an XNA game. The default
  // HelloGame never touches it, which is the point of it being a separate subpath.
  const {
    CnaLog, CnaLogCategory, CnaLogLevel, GetPlatformInfo, GraphicsRendererType,
    IsGraphicsExtensionLayerAvailable, RendererSelection,
  } = await import("cna-ts/extensions/runtime");

  const platform = GetPlatformInfo();
  assert.ok(platform.Name.length > 0);
  const selection = RendererSelection.GetState();
  assert.ok(Object.values(GraphicsRendererType).includes(selection.Selected));
  const available = RendererSelection.GetAvailable();
  assert.ok(available.length >= 1);
  for (const renderer of available) {
    assert.equal(renderer.IsAvailable, true);
    assert.ok(renderer.CategoryName.length > 0 && renderer.MaturityName.length > 0);
  }
  assert.equal(RendererSelection.TryParseName("definitely-not-a-renderer"), null);

  const level = CnaLog.GetMinimumLevel();
  CnaLog.Write(CnaLogLevel.Fatal, CnaLogCategory.Test, "cna-ts template extension smoke");
  assert.equal(CnaLog.GetMinimumLevel(), level);

  // Structural presence is not availability: the routes exist in every CNA build and answer
  // NOT_SUPPORTED where the layer was compiled out, so the truthful branch is reported either way.
  const layer = IsGraphicsExtensionLayerAvailable();

  const { CreatePbrMaterial, CreateRenderPipelineSettings, RenderQuality, TonemappingMode } =
    await import("cna-ts/extensions/graphics");
  // Pure value operations: CNA documents these as answering in either build.
  const material = CreatePbrMaterial();
  assert.ok(material.RoughnessFactor >= 0 && material.RoughnessFactor <= 1);
  assert.equal(material.AlbedoColor.A, 255);
  const pipelineSettings = CreateRenderPipelineSettings();
  assert.ok(pipelineSettings.Exposure > 0);
  assert.ok(Object.values(TonemappingMode).includes(pipelineSettings.TonemappingMode));
  assert.ok(Object.values(RenderQuality).includes(pipelineSettings.RenderQuality));
  console.log(
    `CNA_TS_EXTENSIONS_RUNTIME=PASS PLATFORM=${platform.Name} ` +
    `SELECTED=${GraphicsRendererType[selection.Selected] ?? selection.Selected} ` +
    `AVAILABLE=${available.map((renderer) => renderer.Name).join("|")} ` +
    `FALLBACKS=${RendererSelection.GetFallbacks().length} ` +
    `GRAPHICS_EXTENSION_LAYER=${layer ? "AVAILABLE" : "NOT_SUPPORTED_BACKEND"} ` +
    `PBR_DEFAULTS=PASS TONEMAPPING=${TonemappingMode[pipelineSettings.TonemappingMode]} ` +
    `RENDER_QUALITY=${RenderQuality[pipelineSettings.RenderQuality]}`,
  );
}
