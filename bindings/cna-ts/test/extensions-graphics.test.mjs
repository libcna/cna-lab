import assert from "node:assert/strict";
import test from "node:test";

import {
  GraphicsCapability,
  GraphicsDeviceCapabilities,
  ImageBasedLighting,
  IndirectDraw,
  IsGraphicsExtensionLayerAvailable,
} from "../dist/extensions/graphics/index.js";
import { NativeUnavailableError } from "../dist/index.js";

test("the extended graphics layer lives outside Microsoft.Xna.Framework", async () => {
  const xna = await import("../dist/xna.js");
  const graphics = xna.Microsoft.Xna.Framework.Graphics;
  for (const name of [
    "AsciiPostProcessEffect", "CrtEffect", "DepthEffect", "DebugDraw", "PbrEffect",
    "SkinnedPbrEffect", "ShaderEffect", "ImageBasedLighting", "IndirectDraw", "GraphicsCapability",
    "GraphicsDeviceCapabilities",
  ]) {
    assert.equal(name in xna, false, `${name} must not leak into the strict XNA surface`);
    assert.equal(name in graphics, false, `${name} must not appear in the XNA Graphics namespace`);
  }
});

test("every extended graphics entry point refuses truthfully with no backend", () => {
  for (const call of [
    () => IsGraphicsExtensionLayerAvailable(),
    () => ImageBasedLighting.DefaultLight(),
    () => IndirectDraw.DefaultArguments(),
    () => IndirectDraw.DefaultIndexedArguments(),
    () => GraphicsDeviceCapabilities.Supports({}, GraphicsCapability.ComputeShaders),
    () => GraphicsDeviceCapabilities.MaxComputeWorkGroupCount({}, 0),
    () => GraphicsDeviceCapabilities.MaxComputeWorkGroupSize({}, 0),
    () => GraphicsDeviceCapabilities.MaxComputeWorkGroupInvocations({}),
  ]) {
    assert.throws(call, NativeUnavailableError, call.toString());
  }
});

test("the indirect command format packs without a backend, signed where the GPU reads signed", () => {
  assert.deepEqual(
    [...IndirectDraw.PackArguments({ VertexCount: 6, InstanceCount: 2, FirstVertex: 3, BaseInstance: 0 })],
    [6, 2, 3, 0],
  );
  const indexed = IndirectDraw.PackIndexedArguments(
    { IndexCount: 12, InstanceCount: 4, FirstIndex: 6, BaseVertex: -5, BaseInstance: 0 });
  assert.deepEqual([...indexed], [12, 4, 6, 0xffff_fffb, 0]);
  assert.equal(new Int32Array(indexed.buffer)[3], -5);
  assert.throws(() => IndirectDraw.PackArguments(null), TypeError);
  assert.throws(
    () => IndirectDraw.PackArguments({ VertexCount: 1.5, InstanceCount: 1, FirstVertex: 0, BaseInstance: 0 }),
    TypeError,
  );
});
