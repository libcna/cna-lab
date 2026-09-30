#!/usr/bin/env node

/**
 * The same public XNA API on a **windowed, GPU-backed** CNA renderer.
 *
 * Everything else this package qualifies on Node runs against a HEADLESS renderer, which is honest
 * but leaves a real question open: does the binding's drawing path produce pixels, or only reach
 * routes that return success? This answers it by clearing a render target to an exact colour and
 * reading every texel back — the same evidence the browser slice produces on WebGL2, on the desktop
 * this time.
 *
 * It is opt-in and skips with a reason, because it needs three things the default qualification
 * does not: a CNA library built with a windowed renderer (`CNA_WINDOWED_LIBRARY`), a display for it
 * to open a window on, and a bridge built against the same ABI. Run it under `xvfb-run` on a host
 * with no screen -- and note that `xvfb-run` alone is not enough on a Wayland session, which is why
 * `preferTheDisplayWeWereGiven()` runs below before the backend loads. That module says what goes
 * wrong without it, and it is not a test failure: it is windows on somebody's desktop.
 *
 * ```sh
 * CNA_WINDOWED_LIBRARY=/path/to/libcna_c_api.so CNA_NODE_BRIDGE=build/cna_node_bridge.node \
 *   xvfb-run -a node --test test/windowed-renderer.integration.mjs
 * ```
 */

import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { after } from "node:test";

import { requiredSuite } from "./support/required-suite.mjs";
import { preferTheDisplayWeWereGiven } from "./support/windowed-display.mjs";

import {
  Audio,
  BoundingBox,
  BoundingFrustum,
  Color,
  Game,
  Graphics,
  GraphicsDeviceManager,
  LoadNodeNativeBackend,
  Matrix,
  Rectangle,
  TimeSpan,
  Vector2,
  Vector3,
  Vector4,
} from "../dist/index.js";

/** A Matrix as the sixteen numbers a projection needs, in the order XNA names them. */
function matrixRow(matrix) {
  return [
    matrix.M11, matrix.M12, matrix.M13, matrix.M14,
    matrix.M21, matrix.M22, matrix.M23, matrix.M24,
    matrix.M31, matrix.M32, matrix.M33, matrix.M34,
    matrix.M41, matrix.M42, matrix.M43, matrix.M44,
  ];
}
import * as extensionsModule from "../dist/extensions/index.js";
import { CnaResult } from "../dist/internal/cna-results.js";
import * as computeModule from "../dist/extensions/graphics/index.js";

const library = process.env.CNA_WINDOWED_LIBRARY;
const display = process.env.DISPLAY;
// Optional for a developer with no windowed CNA to hand; `CNA_REQUIRE_WINDOWED_TESTS=1` makes the
// missing environment a named failure and requires the suite to prove it executed.
const { test, skip } = requiredSuite({
  label: "windowed-renderer",
  envVar: "CNA_REQUIRE_WINDOWED_TESTS",
  counter: "WINDOWED_TESTS",
  blocked: library
    ? (display ? null : "no DISPLAY; run this under xvfb-run or on a session with a screen")
    : "set CNA_WINDOWED_LIBRARY to a CNA library built with a windowed renderer",
});

const storageHome = fs.mkdtempSync(path.join(os.tmpdir(), "cna-ts-windowed-"));
after(() => fs.rmSync(storageHome, { recursive: true, force: true }));
process.env.XDG_DATA_HOME = storageHome;

if (!skip) {
  // Before the library initializes its video subsystem: honour DISPLAY, so a run under
  // xvfb-run reaches the virtual server instead of the user's desktop.
  preferTheDisplayWeWereGiven();
    await LoadNodeNativeBackend({
    CnaLibrary: path.resolve(library),
    BridgeModule: path.resolve(process.env.CNA_NODE_BRIDGE ?? "build/cna_node_bridge.node"),
  });
}

/**
 * The first microphone's buffer duration as it was before this process wrote to it. Each test
 * builds its own Game, but the device is shared, so only the first probe sees the untouched value.
 */
let pristineBufferDurationMs = null;

/** The exact colour the render target is cleared to; every channel distinct, none of them 0 or 255. */
const CLEAR = new Color(12, 34, 56, 255);

class WindowedProbeGame extends Game {
  constructor(frames) {
    super();
    this.graphics = new GraphicsDeviceManager(this);
    this.graphics.PreferredBackBufferWidth = 320;
    this.graphics.PreferredBackBufferHeight = 240;
    this.frameTarget = frames;
    this.frames = 0;
    this.evidence = Object.create(null);
  }

  LoadContent() {
    const device = this.GraphicsDevice;
    const record = (name, body) => {
      try {
        this.evidence[name] = body();
      } catch (error) {
        this.evidence[name] = `${error.constructor.name}: ${(error.message ?? "").slice(0, 70)}`;
      }
    };
    // Per-capability answers, each paired with its bit so the test can hold it against the
    // renderer's own capability mask -- a different CNA route entirely.
    record("capabilities", () => {
      const { GraphicsCapability, GraphicsDeviceCapabilities } = computeModule;
      const supported = GraphicsDeviceCapabilities.Supports(
        device, GraphicsCapability.ComputeShaders,
      );
      const capabilities = Object.entries(GraphicsCapability)
        .filter(([, bit]) => typeof bit === "number")
        .map(([name, bit]) => [name, bit, GraphicsDeviceCapabilities.Supports(device, bit)]);
      const result = { supported, capabilities };
      if (!supported) return result;
      const limits = () => ({
        countX: GraphicsDeviceCapabilities.MaxComputeWorkGroupCount(device, 0),
        sizeX: GraphicsDeviceCapabilities.MaxComputeWorkGroupSize(device, 0),
        sizeZ: GraphicsDeviceCapabilities.MaxComputeWorkGroupSize(device, 2),
        invocations: GraphicsDeviceCapabilities.MaxComputeWorkGroupInvocations(device),
      });
      result.limits = limits();
      // Read again after the device has drawn: upstream finding 9 once had every limit go to zero
      // at the first Clear while the capability kept answering true.
      device.Clear(CLEAR);
      result.afterDraw = {
        ...limits(),
        stillSupported: GraphicsDeviceCapabilities.Supports(
          device, GraphicsCapability.ComputeShaders,
        ),
      };
      return result;
    });

    this.spriteBatch = new Graphics.SpriteBatch(device);
    this.texture = new Graphics.Texture2D(device, 2, 2);
    this.texture.SetData([Color.Red, Color.Green, Color.Blue, Color.White]);

    const renderer = extensionsModule.GetRendererInfo();
    this.evidence.renderer = {
      name: renderer.Name,
      type: renderer.RendererType,
      maxTextureDimension: renderer.MaxTextureDimension,
      capabilityFlags: renderer.CapabilityFlags.toString(16),
    };

    // The whole reason this file exists: an off-screen target, cleared through the public API and
    // read back texel by texel. On HEADLESS this cannot be asked; on a real renderer it is the
    // difference between "the route returned success" and "the GPU produced these pixels".
    const target = new Graphics.RenderTarget2D(device, 4, 4);
    try {
      device.SetRenderTarget(target);
      this.evidence.boundCount = device.GetRenderTargets().length;
      device.Clear(CLEAR);
      device.SetRenderTarget(null);
      this.evidence.unboundCount = device.GetRenderTargets().length;
      const readback = new Array(16);
      target.GetData(readback);
      this.evidence.targetPixels = readback.map((color) => color.PackedValue);
      this.evidence.targetInfo = {
        width: target.Width,
        height: target.Height,
        isContentLost: target.IsContentLost,
      };
    } finally {
      target.Dispose();
    }

    // The window, which HEADLESS has none of. Title and client bounds are real state here, so a
    // write followed by a read is evidence rather than a round trip through this package.
    const window = this.Window;
    record("windowTitleRoundTrip", () => {
      const original = window.Title;
      window.Title = "cna-ts windowed qualification";
      const written = window.Title;
      window.Title = original;
      return { written, restored: window.Title === original };
    });
    record("windowBounds", () => {
      const bounds = window.ClientBounds;
      return { width: bounds.Width, height: bounds.Height };
    });
    record("windowHandleIsBigInt", () => typeof window.Handle === "bigint");
    record("windowScreenDeviceName", () => window.ScreenDeviceName);
    record("windowAllowUserResizing", () => {
      const original = window.AllowUserResizing;
      window.AllowUserResizing = !original;
      const flipped = window.AllowUserResizing;
      window.AllowUserResizing = original;
      return { original, flipped, restored: window.AllowUserResizing === original };
    });
    record("windowOrientation", () => window.CurrentOrientation);

    // Volume and cube textures, which the capability inventory used to report as unavailable on
    // "the current qualified backend". That was measured on HEADLESS, where both creation routes
    // really do answer NOT_SUPPORTED, and generalised too far: a renderer with a device creates
    // both. This is the evidence for the corrected claim, so a regression is a failing test rather
    // than a stale sentence.
    record("volumeAndCubeTextures", () => {
      const volume = new Graphics.Texture3D(device, 4, 4, 4, false, Graphics.SurfaceFormat.Color);
      const cube = new Graphics.TextureCube(device, 8, false, Graphics.SurfaceFormat.Color);
      try {
        // A round trip through the volume, so this is storage rather than only construction.
        const written = new Array(4 * 4 * 4).fill(new Color(9, 8, 7, 255));
        written[0] = new Color(1, 2, 3, 255);
        volume.SetData(written);
        const read = new Array(written.length).fill(new Color(0, 0, 0, 0));
        volume.GetData(read);
        return {
          volume: [volume.Width, volume.Height, volume.Depth],
          cubeSize: cube.Size,
          firstTexel: read[0].PackedValue,
          lastTexel: read[read.length - 1].PackedValue,
          expectedFirst: written[0].PackedValue,
          expectedLast: written[written.length - 1].PackedValue,
        };
      } finally {
        volume.Dispose();
        cube.Dispose();
      }
    });

    // The graphics adapter, which needs a live device and therefore a callback like this one.
    record("adapter", () => {
      const adapter = Graphics.GraphicsAdapter.DefaultAdapter;
      const mode = adapter.CurrentDisplayMode;
      return {
        count: Graphics.GraphicsAdapter.Adapters.length,
        description: adapter.Description,
        deviceName: adapter.DeviceName,
        modeWidth: mode.Width,
        modeHeight: mode.Height,
        modeFormat: mode.Format,
        supportedModes: [...adapter.SupportedDisplayModes].length,
        reach: adapter.IsProfileSupported(Graphics.GraphicsProfile.Reach),
        hiDef: adapter.IsProfileSupported(Graphics.GraphicsProfile.HiDef),
        isDeviceAdapter: device.Adapter === adapter,
      };
    });
    // The three screen effects that remain in CNA's extension layer: the CRT and the colour-depth
    // effect, which are ordinary effects a SpriteBatch draws through, and the ASCII effect.
    record("screenEffects", () => {
      const {
        AsciiPostProcessEffect, AsciiQuantizeMode, CrtEffect, CrtMaskType, DepthEffect,
        DepthEffectMode, DitherMode, IsGraphicsExtensionLayerAvailable,
      } = computeModule;
      const owned = [];
      const read = (texture, count) => {
        const pixels = new Array(count);
        texture.GetData(pixels);
        return pixels.map((color) => [color.R, color.G, color.B, color.A]);
      };
      try {
        let probe;
        try {
          probe = CrtEffect.Create(device);
        } catch (error) {
          return {
            layerAbsent: true,
            cnaResult: error.cnaResult,
            extensionLayer: IsGraphicsExtensionLayerAvailable(),
          };
        }
        probe.Dispose();
        const result = {};

        // --- the CRT effect, one full-screen quad through a SpriteBatch ---------------------------
        // A flat grey source, so the pattern the shader adds is the only thing in the output. Every
        // parameter is turned off first: that identity is what makes each later change attributable.
        // One quad covering the whole target is the use CNA documents for this effect.
        const FLAT = 200;
        const GRID = 8;
        const flat = new Graphics.Texture2D(device, GRID, GRID);
        owned.push(flat);
        flat.SetData(new Array(GRID * GRID).fill(0).map(() => new Color(FLAT, FLAT, FLAT, 255)));
        const batch = new Graphics.SpriteBatch(device);
        owned.push(batch);
        const crtThrough = (tune) => {
          const effect = CrtEffect.Create(device);
          const target = new Graphics.RenderTarget2D(device, GRID, GRID);
          try {
            CrtEffect.SetScanlineIntensity(effect, 0);
            CrtEffect.SetCurvature(effect, 0);
            CrtEffect.SetVignetteIntensity(effect, 0);
            CrtEffect.SetMaskIntensity(effect, 0);
            tune?.(effect);
            device.SetRenderTarget(target);
            device.Clear(new Color(0, 0, 0, 255));
            batch.Begin(
              Graphics.SpriteSortMode.Immediate, Graphics.BlendState.Opaque,
              Graphics.SamplerState.PointClamp, Graphics.DepthStencilState.None, null, effect);
            batch.Draw(flat, new Rectangle(0, 0, GRID, GRID), Color.White);
            batch.End();
            device.SetRenderTarget(null);
            return read(target, GRID * GRID);
          } finally {
            target.Dispose();
            effect.Dispose();
          }
        };
        result.crt = {
          flatValue: FLAT,
          grid: GRID,
          allOff: crtThrough(),
          scanlinesHalf: crtThrough((e) => CrtEffect.SetScanlineIntensity(e, 0.5)),
          scanlinesQuarter: crtThrough((e) => CrtEffect.SetScanlineIntensity(e, 0.25)),
          vignette: crtThrough((e) => CrtEffect.SetVignetteIntensity(e, 1)),
        };
        const crtState = CrtEffect.Create(device);
        try {
          result.crt.defaults = [
            CrtEffect.GetScanlineIntensity(crtState), CrtEffect.GetCurvature(crtState),
            CrtEffect.GetVignetteIntensity(crtState), CrtEffect.GetMaskIntensity(crtState),
            CrtEffect.GetMaskType(crtState),
          ];
          // Every value here differs from the default it replaces, so a setter that did nothing
          // would be caught rather than agreeing with itself.
          CrtEffect.SetScanlineIntensity(crtState, 0.4375);
          CrtEffect.SetCurvature(crtState, 0.5);
          CrtEffect.SetVignetteIntensity(crtState, 0.75);
          CrtEffect.SetMaskIntensity(crtState, 0.125);
          CrtEffect.SetMaskType(crtState, CrtMaskType.ShadowMask);
          result.crt.set = [
            CrtEffect.GetScanlineIntensity(crtState), CrtEffect.GetCurvature(crtState),
            CrtEffect.GetVignetteIntensity(crtState), CrtEffect.GetMaskIntensity(crtState),
            CrtEffect.GetMaskType(crtState),
          ];
          result.crt.technique = crtState.CurrentTechnique.Name;
        } finally {
          crtState.Dispose();
        }

        const depthEffect = DepthEffect.Create(device);
        try {
          result.depthEffect = {
            defaults: [DepthEffect.GetMode(depthEffect), DepthEffect.GetDitherMode(depthEffect)],
            technique: depthEffect.CurrentTechnique.Name,
          };
          DepthEffect.SetMode(depthEffect, DepthEffectMode.Grayscale1Bit);
          DepthEffect.SetDitherMode(depthEffect, DitherMode.Bayer8X8);
          result.depthEffect.set = [
            DepthEffect.GetMode(depthEffect), DepthEffect.GetDitherMode(depthEffect),
          ];
        } finally {
          depthEffect.Dispose();
        }

        // --- the ASCII effect's grid ------------------------------------------------------------------
        // The grid comes from the SOURCE size over the cell size, not from the rectangle it is drawn
        // into: a 32x24 source in 8x12 cells is 4x2 no matter how large the destination is.
        const asciiSource = new Graphics.Texture2D(device, 32, 24);
        owned.push(asciiSource);
        asciiSource.SetData(new Array(32 * 24).fill(0).map(
          (_, i) => new Color(i % 256, (i * 3) % 256, (i * 7) % 256, 255),
        ));
        const asciiTarget = new Graphics.RenderTarget2D(device, 64, 48);
        const ascii = new AsciiPostProcessEffect(device);
        try {
          const before = ascii.LastGridDimensions;
          const drawWith = (width, height) => {
            ascii.SetCellSize(width, height);
            device.SetRenderTarget(asciiTarget);
            device.Clear(new Color(0, 0, 0, 255));
            try {
              ascii.Draw(asciiSource, new Rectangle(0, 0, 64, 48));
            } finally {
              device.SetRenderTarget(null);
            }
            const grid = ascii.LastGridDimensions;
            return [grid.Columns, grid.Rows];
          };
          result.ascii = {
            sourceSize: [asciiSource.Width, asciiSource.Height],
            destinationSize: [asciiTarget.Width, asciiTarget.Height],
            beforeAnyDraw: [before.Columns, before.Rows],
            cell8x12: drawWith(8, 12),
            cell4x4: drawWith(4, 4),
            cell16x8: drawWith(16, 8),
            defaultCell: (() => {
              const fresh = new AsciiPostProcessEffect(device);
              try {
                return [fresh.CellSize.Width, fresh.CellSize.Height, fresh.QuantizeMode];
              } finally {
                fresh.Dispose();
              }
            })(),
          };
          ascii.QuantizeMode = AsciiQuantizeMode.BlackWhite;
          result.ascii.quantizeAfterSet = ascii.QuantizeMode;
        } finally {
          ascii.Dispose();
          asciiTarget.Dispose();
        }
        return result;
      } finally {
        for (const resource of owned.reverse()) {
          try {
            resource.Dispose();
          } catch (error) {
            (this.evidence.screenEffectsCleanup ??= []).push(
              `${error.constructor.name}: ${(error.message ?? "").slice(0, 90)}`,
            );
          }
        }
      }
    });
    // The physically-based effects: compiled effects with named techniques, and the per-field and
    // per-slot state CNA keeps on them.
    record("pbrEffects", () => {
      const {
        AlphaMode, IsGraphicsExtensionLayerAvailable, PbrEffect, PbrTextureSlot, SkinnedPbrEffect,
      } = computeModule;
      const owned = [];
      try {
        let effect;
        try {
          effect = PbrEffect.Create(device);
        } catch (error) {
          return {
            layerAbsent: true,
            cnaResult: error.cnaResult,
            extensionLayer: IsGraphicsExtensionLayerAvailable(),
          };
        }
        owned.push(effect);
        const skinned = SkinnedPbrEffect.Create(device);
        owned.push(skinned);
        const result = {
          technique: effect.CurrentTechnique.Name,
          passCount: effect.CurrentTechnique.Passes.Count,
          skinnedTechnique: skinned.CurrentTechnique.Name,
        };
        // A stock effect's pass applies for real here; HEADLESS answers not-supported.
        result.apply = (() => {
          try {
            effect.CurrentTechnique.Passes.Get(0).Apply();
            return "SUCCESS";
          } catch (error) {
            return `result ${error.cnaResult}`;
          }
        })();

        // Every value written differs from the default it replaces.
        PbrEffect.SetMetallicFactor(effect, 0.25);
        PbrEffect.SetIor(effect, 1.75);
        PbrEffect.SetAlphaMode(effect, AlphaMode.Blend);
        PbrEffect.SetDoubleSided(effect, true);
        PbrEffect.SetTextureCoordinateSet(effect, PbrTextureSlot.Normal, 1);
        result.throughAccessors = {
          metallic: PbrEffect.GetMetallicFactor(effect),
          ior: PbrEffect.GetIor(effect),
          alphaMode: PbrEffect.GetAlphaMode(effect),
          doubleSided: PbrEffect.GetDoubleSided(effect),
          normalSet: PbrEffect.GetTextureCoordinateSet(effect, PbrTextureSlot.Normal),
        };

        const texture = new Graphics.Texture2D(device, 2, 2);
        owned.push(texture);
        texture.SetData([
          new Color(200, 100, 40, 255), new Color(40, 200, 100, 255),
          new Color(100, 40, 200, 255), new Color(255, 255, 255, 255),
        ]);
        result.slots = {
          initial: PbrEffect.GetTexture(effect, PbrTextureSlot.BaseColor) !== 0n,
        };
        PbrEffect.SetTexture(effect, PbrTextureSlot.BaseColor, texture);
        result.slots.afterSet = PbrEffect.GetTexture(effect, PbrTextureSlot.BaseColor) !== 0n;
        result.slots.otherSlotAfterSet =
          PbrEffect.GetTexture(effect, PbrTextureSlot.Normal) !== 0n;
        PbrEffect.SetTexture(effect, PbrTextureSlot.BaseColor, null);
        result.slots.afterClear = PbrEffect.GetTexture(effect, PbrTextureSlot.BaseColor) !== 0n;

        // The skinned effect's own state, on a renderer that has real shaders.
        SkinnedPbrEffect.SetWeightsPerVertex(skinned, 2);
        SkinnedPbrEffect.SetBoneTransforms(
          skinned, [Matrix.Identity, Matrix.CreateTranslation(new Vector3(3, 4, 5))]);
        const bones = SkinnedPbrEffect.GetBoneTransforms(skinned, 2);
        result.skinned = {
          weights: SkinnedPbrEffect.GetWeightsPerVertex(skinned),
          count: bones.length,
          translation: [bones[1].M41, bones[1].M42, bones[1].M43],
        };
        return result;
      } finally {
        for (const resource of owned.reverse()) {
          try {
            resource.Dispose();
          } catch (error) {
            (this.evidence.pbrCleanup ??= []).push(
              `${error.constructor.name}: ${(error.message ?? "").slice(0, 90)}`,
            );
          }
        }
      }
    });

    record("shaderEffect", () => {
      const { ShaderEffect, IsGraphicsExtensionLayerAvailable } = computeModule;
      const N = 4;
      const VERTEX = `#version 300 es
precision highp float;
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;
out vec2 TexCoord;
uniform mat4 projection;
void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); TexCoord = aTexCoord; }
`;
      const owned = [];
      try {
        let probe;
        try {
          probe = new ShaderEffect(device, VERTEX, `#version 300 es
precision highp float;
in vec2 TexCoord;
out vec4 FragColor;
uniform float uValue;
void main() { FragColor = vec4(uValue, 0.0, 0.0, 1.0); }
`);
        } catch (error) {
          return {
            layerAbsent: true,
            cnaResult: error.cnaResult,
            extensionLayer: IsGraphicsExtensionLayerAvailable(),
          };
        }
        owned.push(probe);
        const result = { valid: probe.IsEffectValid, compileError: probe.CompileError.slice(0, 200) };
        if (!result.valid) return result;

        const white = new Graphics.Texture2D(device, 1, 1);
        owned.push(white);
        white.SetData([new Color(255, 255, 255, 255)]);
        const batch = new Graphics.SpriteBatch(device);
        owned.push(batch);
        const target = new Graphics.RenderTarget2D(device, N, N);
        owned.push(target);

        const runFindingTwentyTwo = (result) => {
          // --- finding 22 -------------------------------------------------------------------------
          // How many Begin/Draw/End pairs an effect has had decides whether its draw appears. The
          // target is rebound and cleared before every run, so nothing carries over but the effect.
          const runFlat = (effect, value) => {
            device.SetRenderTarget(target);
            device.Clear(new Color(0, 0, 0, 255));
            batch.Begin(
              Graphics.SpriteSortMode.Immediate, Graphics.BlendState.Opaque, null,
              Graphics.DepthStencilState.None, null, effect);
            effect.SetUniformFloat("uValue", value);
            batch.Draw(white, new Rectangle(0, 0, N, N), Color.White);
            batch.End();
            device.SetRenderTarget(null);
            const pixels = new Array(N * N);
            target.GetData(pixels);
            return [pixels[0].R, pixels[0].G, pixels[0].B, pixels[0].A];
          };
            // A texture bound to sampler unit 1, sampled by a shader that reads only unit 1. Unit 0
          // is SpriteBatch's own texture, so a binding that went there instead leaves unit 1
          // unbound and the shader samples nothing.
          const sampler = new ShaderEffect(device, VERTEX, `#version 300 es
precision highp float;
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D uExtra;
void main() { FragColor = texture(uExtra, TexCoord); }
`);
          const extra = new Graphics.Texture2D(device, 1, 1);
          extra.SetData([new Color(10, 200, 60, 255)]);
          // Pushed before the effect that will borrow it: the cleanup below disposes in reverse, so
          // the effect goes first and gives the borrow back before the texture is asked to go.
          owned.push(extra);
          owned.push(sampler);
          sampler.SetUniformInt("uExtra", 1);
          sampler.SetTexture(1, extra);
          const sampleOnce = () => {
            device.SetRenderTarget(target);
            device.Clear(new Color(0, 0, 0, 255));
            batch.Begin(
              Graphics.SpriteSortMode.Immediate, Graphics.BlendState.Opaque, null,
              Graphics.DepthStencilState.None, null, sampler);
            sampler.SetUniformInt("uExtra", 1);
            batch.Draw(white, new Rectangle(0, 0, N, N), Color.White);
            batch.End();
            device.SetRenderTarget(null);
            const pixels = new Array(N * N);
            target.GetData(pixels);
            return [pixels[0].R, pixels[0].G, pixels[0].B, pixels[0].A];
          };
          sampleOnce();  // finding 22: the first draw of a fresh effect is lost
          result.sampledUnitOne = sampleOnce();

        result.firstDraw = runFlat(probe, 0.125);
          result.secondDraw = runFlat(probe, 0.125);
          result.thirdDraw = runFlat(probe, 0.125);
          const second = new ShaderEffect(device, VERTEX, `#version 300 es
  precision highp float;
  in vec2 TexCoord;
  out vec4 FragColor;
  uniform float uValue;
  void main() { FragColor = vec4(uValue, 0.0, 0.0, 1.0); }
  `);
          owned.push(second);
          result.freshEffectFirstDraw = runFlat(second, 0.125);
          result.freshEffectSecondDraw = runFlat(second, 0.125);
          // And the same batch with no custom effect at all, which draws correctly the first time.
          device.SetRenderTarget(target);
          device.Clear(new Color(0, 0, 0, 255));
          batch.Begin(Graphics.SpriteSortMode.Immediate, Graphics.BlendState.Opaque);
          batch.Draw(white, new Rectangle(0, 0, N, N), new Color(40, 80, 120, 255));
          batch.End();
          device.SetRenderTarget(null);
          const plain = new Array(N * N);
          target.GetData(plain);
          result.plainFirstDraw = [plain[0].R, plain[0].G, plain[0].B, plain[0].A];
          // The workaround, which is also the diagnosis: one Apply outside any batch is enough.
          const applied = new ShaderEffect(device, VERTEX, `#version 300 es
  precision highp float;
  in vec2 TexCoord;
  out vec4 FragColor;
  uniform float uValue;
  void main() { FragColor = vec4(uValue, 0.0, 0.0, 1.0); }
  `);
          owned.push(applied);
          applied.CurrentTechnique.Passes.Get(0).Apply();
          result.preAppliedFirstDraw = runFlat(applied, 0.125);
          return result;
        };

        return runFindingTwentyTwo(result);
      } finally {
        for (const resource of owned.reverse()) {
          try {
            resource.Dispose();
          } catch (error) {
            (this.evidence.shaderEffectCleanup ??= []).push(
              `${error.constructor.name}: ${(error.message ?? "").slice(0, 90)}`,
            );
          }
        }
      }
    });

    // --- the audio backend, which is the other thing this artifact has and HEADLESS does not ----
    //
    // This build is `CNA_AUDIO_PLATFORM=SDL3`; the one the default qualification uses is `NULL`.
    // Two capability rows said microphones enumerate as none and that playback "verifies state and
    // lifetime only", and both were true of the NULL backend and of nothing else. Asked here, the
    // same routes answer with real hardware and a real state machine.
    record("microphones", () => Audio.Microphone.All.map((microphone) => ({
      Name: microphone.Name,
      SampleRate: microphone.SampleRate,
      IsHeadset: microphone.IsHeadset,
      State: microphone.State,
      BufferDurationMs: microphone.BufferDuration.TotalMilliseconds,
      SampleSizeFor100ms: microphone.GetSampleSizeInBytes(TimeSpan.FromMilliseconds(100)),
    })));
    record("defaultMicrophone", () => Audio.Microphone.Default?.Name ?? null);

    // Upstream finding 28. Each value is offered to a microphone that has not been written to, and
    // the answer recorded; the assertions below compare the whole table against XNA's own IL rather
    // than against what CNA happens to do. Capture is deliberately never started: enumerating
    // devices and configuring a buffer touch no audio, and opening a capture stream would record
    // from this host's real microphone.
    record("bufferDurationRange", () => {
      const microphone = Audio.Microphone.All[0];
      if (!microphone) return null;
      // The buffer duration is device state for the process, not for this Game, and the sweep
      // below cannot put it back: the value it started at is the one value the setter refuses,
      // which is the finding. So the pristine reading is taken once, before anything writes.
      pristineBufferDurationMs ??= microphone.BufferDuration.TotalMilliseconds;
      const initial = pristineBufferDurationMs;
      const rows = [50, 90, 100, 500, 990, 1000, 1100, 1500, 2500, 60000].map((milliseconds) => {
        try {
          microphone.BufferDuration = TimeSpan.FromMilliseconds(milliseconds);
          return [milliseconds, "accepted", microphone.BufferDuration.TotalMilliseconds];
        } catch (error) {
          return [milliseconds, error.constructor.name, null];
        }
      });
      return { initial, rows };
    });

    // The playback state machine. On NULL audio every one of these answers Stopped, so the
    // transitions are the part that only a real audio backend can produce. The fixture is a square
    // wave whose duration is arithmetic -- 4410 frames at 22050 Hz is exactly 200 ms -- so the
    // duration below is predicted rather than read back.
    record("soundEffect", () => {
      const frames = 4410;
      const bytes = new Array(frames * 2);
      for (let index = 0; index < frames; index += 1) {
        const sample = (index % 100) < 50 ? 8000 : -8000;
        bytes[index * 2] = sample & 0xff;
        bytes[index * 2 + 1] = (sample >> 8) & 0xff;
      }
      const effect = new Audio.SoundEffect(bytes, 22050, Audio.AudioChannels.Mono);
      const instance = effect.CreateInstance();
      try {
        const states = { initial: instance.State };
        instance.Play(); states.playing = instance.State;
        instance.Pause(); states.paused = instance.State;
        instance.Resume(); states.resumed = instance.State;
        instance.Stop(); states.stopped = instance.State;
        instance.Volume = 0.5;
        instance.Pitch = -0.25;
        instance.Pan = 0.75;
        return {
          states,
          durationMs: effect.Duration.TotalMilliseconds,
          frames, sampleRate: 22050,
          Volume: instance.Volume, Pitch: instance.Pitch, Pan: instance.Pan,
        };
      } finally {
        instance.Dispose();
        effect.Dispose();
      }
    });

    // A stock effect on a renderer that has real shaders. HEADLESS constructs one and refuses to
    // execute it, so this is a branch the default qualification cannot reach.
    const basic = new Graphics.BasicEffect(device);
    try {
      basic.VertexColorEnabled = true;
      const pass = basic.CurrentTechnique.Passes.Get(0);
      try {
        pass.Apply();
        this.evidence.stockEffectApply = "SUCCESS";
      } catch (error) {
        this.evidence.stockEffectApply = `result ${error.cnaResult}`;
      }
    } finally {
      basic.Dispose();
    }
    super.LoadContent();
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.spriteBatch.Begin();
    this.spriteBatch.Draw(this.texture, new Vector2(16, 16), Color.White);
    this.spriteBatch.End();

    this.frames += 1;
    if (this.frames >= this.frameTarget) this.Exit();
    super.Draw(gameTime);
  }

  UnloadContent() {
    this.spriteBatch?.Dispose();
    this.texture?.Dispose();
    super.UnloadContent();
  }
}

/*
 * Every renderer here reads a render target back correctly.
 *
 * That was not true earlier: `docs/upstream-cna-findings.md` item 7 recorded OPENGLES3 answering
 * every render-target readback with zeros, and this file asserted those zeros rather than skipping
 * the check -- which is what made the repair visible the moment it landed. CNA fixed it in
 * 48ab0de7f, "separate frame context handoff from operation leases", and the assertion below is
 * now the ordinary one again.
 */

test("a windowed CNA renderer produces the exact pixels the public API asked for", { skip }, async () => {
  const game = new WindowedProbeGame(60);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // The renderer is a real one, and it says so itself rather than being labelled here.
  assert.notEqual(evidence.renderer.name, "HEADLESS", "this file is for a windowed renderer");
  assert.ok(evidence.renderer.maxTextureDimension >= 2048, "a real GPU reports a real texture limit");

  assert.equal(evidence.boundCount, 1);
  assert.equal(evidence.unboundCount, 0);
  assert.deepEqual([evidence.targetInfo.width, evidence.targetInfo.height], [4, 4]);
  assert.equal(evidence.targetInfo.isContentLost, false);

  const expected = CLEAR.PackedValue;
  assert.equal(evidence.targetPixels.length, 16);
  // Sixteen texels, each exactly the colour Clear was given. This is the assertion that separates
  // a drawing path from a dispatch path, and it now holds on every renderer this file runs on.
  assert.deepEqual(
    evidence.targetPixels, new Array(16).fill(expected),
    `${evidence.renderer.name} did not read its render target back exactly`,
  );
  const readback = "EXACT";

  assert.equal(game.frames, 60);
  console.log(
    `CNA_TS_WINDOWED_RENDERER=PASS RENDERER=${evidence.renderer.name} ` +
    `MAX_TEXTURE=${evidence.renderer.maxTextureDimension} ` +
    `CAPABILITY_FLAGS=0x${evidence.renderer.capabilityFlags} ` +
    `RENDER_TARGET_READBACK=${readback} STOCK_EFFECT_APPLY=${evidence.stockEffectApply}`,
  );
});

test("a windowed CNA renderer reports a real graphics adapter", { skip }, async () => {
  const game = new WindowedProbeGame(2);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // GraphicsAdapter needs a live device, so a windowed renderer is where it can be asked for real.
  const adapter = evidence.adapter;
  assert.equal(typeof adapter, "object", `adapter read failed: ${adapter}`);
  assert.ok(adapter.count >= 1, "a windowed renderer reports at least one adapter");
  assert.equal(typeof adapter.description, "string");
  assert.ok(adapter.description.length > 0);
  assert.equal(typeof adapter.deviceName, "string");
  assert.ok(adapter.modeWidth > 0 && adapter.modeHeight > 0);
  assert.equal(adapter.modeFormat, Graphics.SurfaceFormat.Color);
  assert.ok(adapter.supportedModes >= 1);
  assert.equal(typeof adapter.reach, "boolean");
  assert.equal(typeof adapter.hiDef, "boolean");
  if (adapter.hiDef) assert.equal(adapter.reach, true, "HiDef implies Reach");
  assert.equal(adapter.isDeviceAdapter, true, "the device's adapter is the default one");
  console.log(
    `CNA_TS_WINDOWED_ADAPTER=PASS RENDERER=${evidence.renderer.name} ` +
    `ADAPTERS=${adapter.count} MODE=${adapter.modeWidth}x${adapter.modeHeight} ` +
    `MODES=${adapter.supportedModes} REACH=${adapter.reach} HIDEF=${adapter.hiDef}`,
  );
});

test("a windowed CNA GameWindow reports and accepts real state", { skip }, async () => {
  const game = new WindowedProbeGame(2);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // The title is the clearest write-then-read on a window: HEADLESS has none, so this branch is
  // unreachable in the default qualification.
  const title = evidence.windowTitleRoundTrip;
  assert.equal(typeof title, "object", `window title failed: ${title}`);
  assert.equal(title.written, "cna-ts windowed qualification", "the title CNA reports is the one set");
  assert.equal(title.restored, true, "and it can be put back");

  const bounds = evidence.windowBounds;
  assert.equal(typeof bounds, "object", `client bounds failed: ${bounds}`);
  // Measured rather than assumed, because the renderers disagree and both answers are honest.
  // OPENGLES3 and SDL_RENDERER report the 320x240 the manager asked for; SOFTWARE reports 0x0,
  // because it presents through a surface rather than a sized client area. So what is asserted is:
  // the numbers are non-negative, and *where a renderer reports a client area at all* it is the one
  // the GraphicsDeviceManager requested -- which is the part that would break if the request never
  // reached the window.
  assert.ok(Number.isInteger(bounds.width) && bounds.width >= 0, `bad width ${bounds.width}`);
  assert.ok(Number.isInteger(bounds.height) && bounds.height >= 0, `bad height ${bounds.height}`);
  if (bounds.width > 0) {
    assert.deepEqual(
      [bounds.width, bounds.height], [320, 240],
      "a renderer that reports a client area must report the requested one",
    );
  }

  assert.equal(evidence.windowHandleIsBigInt, true, "a native window handle stays a bigint");
  assert.equal(typeof evidence.windowScreenDeviceName, "string");
  assert.equal(typeof evidence.windowOrientation, "number");

  // AllowUserResizing is a real platform flag on a windowed renderer. Whether the platform accepts
  // the flip is its business; what is asserted is that the value is read back rather than
  // remembered here, and that the original is restored either way.
  const resizing = evidence.windowAllowUserResizing;
  assert.equal(typeof resizing, "object", `resizing failed: ${resizing}`);
  assert.equal(typeof resizing.original, "boolean");
  assert.equal(typeof resizing.flipped, "boolean");
  assert.equal(resizing.restored, true);
  console.log(
    `CNA_TS_WINDOWED_WINDOW=PASS RENDERER=${evidence.renderer.name} ` +
    `CLIENT=${bounds.width}x${bounds.height} SCREEN=${evidence.windowScreenDeviceName} ` +
    `RESIZING_FLIPPED=${resizing.original !== resizing.flipped}`,
  );
});

test("a windowed CNA renderer answers every capability the way its own mask does", { skip }, async () => {
  const game = new WindowedProbeGame(2);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  const caps = evidence.capabilities;
  assert.equal(typeof caps, "object", `capability probe failed: ${caps}`);
  assert.equal(typeof caps.supported, "boolean");
  // Each per-capability answer must equal the matching bit of the renderer's own capability
  // bitmask, which reaches this test through a different CNA route (GetRendererInfo): a query that
  // answered one constant for every argument, or read the wrong capability for an index, disagrees
  // with the mask on any renderer whose bits are not all alike.
  const mask = BigInt(`0x${evidence.renderer.capabilityFlags}`);
  assert.equal(
    caps.capabilities.length, Object.values(computeModule.GraphicsCapability)
      .filter((value) => typeof value === "number").length,
    "every GraphicsCapability is asked",
  );
  for (const [name, bit, answered] of caps.capabilities) {
    assert.equal(
      answered, ((mask >> BigInt(bit)) & 1n) === 1n,
      `${name} (bit ${bit}) disagrees with the renderer's own capability mask`,
    );
  }
  assert.equal(
    caps.capabilities.find(([name]) => name === "ComputeShaders")?.[2], caps.supported,
    "the enum member and the query must name the same capability",
  );
  assert.equal(
    caps.capabilities.filter(([, , answered]) => answered).length,
    [...mask.toString(2)].filter((bit) => bit === "1").length,
    "as many capabilities answer true as the renderer's mask has bits set",
  );

  if (!caps.supported) {
    console.log(`CNA_TS_WINDOWED_CAPABILITIES=PASS COMPUTE=NOT_SUPPORTED RENDERER=${evidence.renderer.name}`);
    return;
  }
  // Limits a renderer that reports compute must be able to state, on both sides of a draw.
  assert.ok(caps.limits.countX > 0 && caps.limits.sizeX > 0);
  assert.ok(caps.limits.invocations >= 16, "a compute renderer takes at least one 16-wide group");
  assert.ok(
    caps.limits.sizeX >= caps.limits.sizeZ,
    "the per-axis limits are read per axis, not one value repeated",
  );
  assert.deepEqual(
    [caps.afterDraw.countX, caps.afterDraw.sizeX, caps.afterDraw.invocations],
    [caps.limits.countX, caps.limits.sizeX, caps.limits.invocations],
    "the work-group limits must not change because something drew (upstream finding 9, fixed)",
  );
  assert.equal(caps.afterDraw.stillSupported, true, "and the capability query still agrees");
  console.log(
    `CNA_TS_WINDOWED_CAPABILITIES=PASS MASK=${evidence.renderer.capabilityFlags} ` +
    `WORK_GROUP=${caps.limits.sizeX}x${caps.limits.invocations}`,
  );
});

test("a windowed CNA renderer runs 600 frames without drift", { skip }, async () => {
  const game = new WindowedProbeGame(600);
  await game.Run();
  const frames = game.frames;
  const effect = game.evidence.stockEffectApply;
  game.Dispose();
  assert.equal(frames, 600);
  // A stock effect either applies for real here or names the result it refused with. Both are
  // recorded; neither is assumed.
  assert.ok(
    effect === "SUCCESS" || /^result \d+$/.test(effect),
    `unexpected stock-effect evidence ${effect}`,
  );
});

test("a windowed CNA renderer draws the screen effects to the pixels their own models predict", { skip }, async () => {
  const game = new WindowedProbeGame(6);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  const fx = evidence.screenEffects;
  assert.equal(typeof fx, "object", `the screen-effect block did not run: ${fx}`);
  if (fx.layerAbsent) {
    assert.equal(fx.extensionLayer, false, "a layer that is present must not refuse to make an effect");
    console.log(`CNA_TS_WINDOWED_SCREEN_EFFECTS=SKIPPED_NO_LAYER RESULT=${fx.cnaResult}`);
    return;
  }
  assert.equal(
    evidence.screenEffectsCleanup, undefined, `cleanup failed: ${evidence.screenEffectsCleanup}`);

  // --- the CRT effect -------------------------------------------------------------------------------
  const flat = fx.crt.flatValue;
  const grid = fx.crt.grid;
  const crtRow = (pixels, row) => pixels.slice(row * grid, row * grid + grid);
  const isFlat = (pixels, value) => pixels.every((texel) => texel.slice(0, 3).every((c) => c === value));
  assert.ok(isFlat(fx.crt.allOff, flat), "a CRT with every parameter at zero must be an exact copy");
  // Scanlines darken alternate rows by exactly the intensity, which is the whole model.
  for (const [name, intensity] of [["scanlinesHalf", 0.5], ["scanlinesQuarter", 0.25]]) {
    const pixels = fx.crt[name];
    const dark = Math.round(flat * (1 - intensity));
    for (let row = 0; row < grid; row += 1) {
      const expected = row % 2 === 0 ? dark : flat;
      assert.ok(
        isFlat(crtRow(pixels, row), expected),
        `${name} row ${row}: expected a flat ${expected}, got ${JSON.stringify(crtRow(pixels, row)[0])}`,
      );
    }
  }
  assert.notDeepEqual(fx.crt.scanlinesHalf, fx.crt.scanlinesQuarter, "and the intensity must matter");
  // A vignette is radial: symmetric under both mirrors, and strictly darker towards the corner.
  const vignette = fx.crt.vignette;
  const at = (x, y) => vignette[y * grid + x][0];
  for (let y = 0; y < grid; y += 1) {
    for (let x = 0; x < grid; x += 1) {
      assert.equal(at(x, y), at(grid - 1 - x, y), `the vignette is not left-right symmetric at ${x},${y}`);
      assert.equal(at(x, y), at(x, grid - 1 - y), `the vignette is not top-bottom symmetric at ${x},${y}`);
    }
  }
  for (let step = 1; step < grid / 2; step += 1) {
    assert.ok(
      at(step, step) > at(step - 1, step - 1),
      `the vignette must brighten towards the centre: ${at(step - 1, step - 1)} then ${at(step, step)}`,
    );
  }
  assert.ok(at(0, 0) < flat / 2, `the corner of a full vignette is barely lit, not ${at(0, 0)}`);
  assert.ok(
    at(grid / 2, grid / 2) > flat * 0.9,
    `and its centre is nearly untouched, not ${at(grid / 2, grid / 2)}`,
  );
  // The parameters CNA reports, and the values it takes back. Every one written differs from the
  // default it replaced, so a setter that did nothing would be caught.
  const [scan, curve, vig, mask, maskType] = fx.crt.defaults;
  assert.ok(scan > 0 && scan < 1 && curve > 0 && vig > 0 && mask > 0, `odd CRT defaults: ${fx.crt.defaults}`);
  assert.deepEqual(fx.crt.set, [0.4375, 0.5, 0.75, 0.125, computeModule.CrtMaskType.ShadowMask]);
  for (let index = 0; index < 5; index += 1) {
    assert.notEqual(fx.crt.set[index], fx.crt.defaults[index], `CRT parameter ${index} was set to its default`);
  }
  assert.equal(maskType, computeModule.CrtMaskType.ApertureGrille);
  assert.equal(typeof fx.crt.technique, "string");
  assert.ok(fx.crt.technique.length > 0, "a CRT effect has a named technique");

  // --- the depth effect ------------------------------------------------------------------------------
  // No pixel claim here: this effect quantises a depth input that a fullscreen colour blit does not
  // supply, so what is qualified is the state CNA keeps -- VERIFIED_NATIVE_STATE, not VERIFIED_PIXEL.
  assert.deepEqual(
    fx.depthEffect.defaults,
    [computeModule.DepthEffectMode.Color16Bit, computeModule.DitherMode.None],
  );
  assert.deepEqual(
    fx.depthEffect.set,
    [computeModule.DepthEffectMode.Grayscale1Bit, computeModule.DitherMode.Bayer8X8],
  );
  assert.ok(fx.depthEffect.technique.length > 0);

  // --- the ASCII grid -----------------------------------------------------------------------------------
  // The grid is the SOURCE size over the cell size. The destination rectangle is four times the
  // source's area here and never appears in any of the three answers, which is the point.
  assert.deepEqual(fx.ascii.sourceSize, [32, 24]);
  assert.deepEqual(fx.ascii.destinationSize, [64, 48]);
  assert.deepEqual(fx.ascii.beforeAnyDraw, [0, 0], "nothing has been quantised yet");
  for (const [cell, expected] of [[[8, 12], [4, 2]], [[4, 4], [8, 6]], [[16, 8], [2, 3]]]) {
    const key = `cell${cell[0]}x${cell[1]}`;
    assert.deepEqual(
      fx.ascii[key], expected,
      `a ${fx.ascii.sourceSize.join("x")} source in ${cell.join("x")} cells is ` +
      `${expected.join("x")}, not ${fx.ascii[key]}`,
    );
    assert.deepEqual(
      expected,
      [fx.ascii.sourceSize[0] / cell[0], fx.ascii.sourceSize[1] / cell[1]],
      "and that is exactly the source divided by the cell",
    );
  }
  assert.deepEqual(fx.ascii.defaultCell.slice(0, 2), [8, 8]);
  assert.equal(fx.ascii.defaultCell[2], computeModule.AsciiQuantizeMode.Color);

  assert.equal(fx.ascii.quantizeAfterSet, computeModule.AsciiQuantizeMode.BlackWhite);

  console.log(
    `CNA_TS_WINDOWED_SCREEN_EFFECTS=PASS CRT_SCANLINES=EXACT CRT_VIGNETTE=RADIAL ` +
    `ASCII_GRID=${fx.ascii.cell8x12.join("x")}/${fx.ascii.cell4x4.join("x")}/${fx.ascii.cell16x8.join("x")}`,
  );
});

test("a windowed CNA renderer compiles the physically-based effects and keeps their state", { skip }, async () => {
  const game = new WindowedProbeGame(6);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  const pbr = evidence.pbrEffects;
  assert.equal(typeof pbr, "object", `the PBR block did not run: ${pbr}`);
  if (pbr.layerAbsent) {
    assert.equal(pbr.extensionLayer, false);
    console.log(`CNA_TS_WINDOWED_PBR=SKIPPED_NO_LAYER RESULT=${pbr.cnaResult}`);
    return;
  }
  assert.equal(evidence.pbrCleanup, undefined, `cleanup failed: ${evidence.pbrCleanup}`);

  const { AlphaMode } = computeModule;
  // What only a real renderer answers: these are compiled effects with named techniques and
  // passes that execute, where HEADLESS constructs them and refuses.
  assert.equal(typeof pbr.technique, "string");
  assert.ok(pbr.technique.length > 0, "a PBR effect has a named technique");
  assert.ok(pbr.passCount >= 1, `and at least one pass, not ${pbr.passCount}`);
  assert.ok(pbr.skinnedTechnique.length > 0, "and so does the skinned one");
  assert.equal(pbr.apply, "SUCCESS", "its pass applies on a renderer with real shaders");

  assert.deepEqual(
    pbr.throughAccessors,
    { metallic: 0.25, ior: 1.75, alphaMode: AlphaMode.Blend, doubleSided: true, normalSet: 1 },
    "every field written reads back through CNA",
  );
  assert.deepEqual(
    pbr.slots,
    { initial: false, afterSet: true, otherSlotAfterSet: false, afterClear: false },
    "a slot starts empty, holds what was set, only that slot, and clears to empty",
  );

  assert.equal(pbr.skinned.weights, 2);
  assert.equal(pbr.skinned.count, 2);
  assert.deepEqual(pbr.skinned.translation, [3, 4, 5]);

  console.log(
    `CNA_TS_WINDOWED_PBR=PASS TECHNIQUE=${pbr.technique}/${pbr.passCount} APPLY=${pbr.apply} ` +
    `SLOTS=SET_AND_CLEAR`,
  );
});

test("a windowed CNA renderer runs a custom shader, and loses its first draw", { skip }, async () => {
  const game = new WindowedProbeGame(6);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  const fx = evidence.shaderEffect;
  assert.equal(typeof fx, "object", `the shader-effect block did not run: ${fx}`);
  if (fx.layerAbsent) {
    assert.equal(fx.extensionLayer, false, "a layer that is present must not refuse to make one");
    console.log(`CNA_TS_WINDOWED_SHADER_EFFECT=SKIPPED_NO_LAYER RESULT=${fx.cnaResult}`);
    return;
  }
  assert.equal(
    evidence.shaderEffectCleanup, undefined, `cleanup failed: ${evidence.shaderEffectCleanup}`);

  if (!fx.valid) {
    // An honest boundary: this renderer would not compile the source and said why.
    assert.ok(fx.compileError.length > 0, "a renderer that rejected the source must say why");
    console.log(
      `CNA_TS_WINDOWED_SHADER_EFFECT=NOT_COMPILED RENDERER=${evidence.renderer.name} ` +
      `ERROR=${JSON.stringify(fx.compileError.slice(0, 80))}`,
    );
    return;
  }
  assert.equal(fx.compileError, "", "a shader that compiled has an empty log");

  // --- upstream finding 22 ------------------------------------------------------------------------
  // A fresh ShaderEffect's FIRST SpriteBatch draw produces nothing. Asserted as it is, not worked
  // around, so the day it is repaired this file says so. Every run below rebinds and clears the
  // same 4x4 target first, so nothing carries over from one to the next except the effect itself.
  // 0.125 * 255 = 31.875, and the two rasterizers this host can run disagree about it by one:
  // the AMD Radeon 780M answers 32 and Mesa's llvmpipe answers 31. Both are a legal float-to-unorm8
  // conversion of the same shader output, so the assertion is the shader's arithmetic to within a
  // byte rather than one rasterizer's answer written down as the truth. What the test is actually
  // about -- a draw that produced *nothing* versus a draw that produced the colour -- is separated
  // by 31, not by 1, so nothing here is weakened.
  const DRAWN = 0.125 * 255;
  const nothing = [0, 0, 0, 255];  // the opaque black the target was cleared to
  const assertDrawn = (actual, what) => {
    assert.ok(
      Math.abs(actual[0] - DRAWN) <= 1,
      `${what}: red is ${actual[0]}, expected ${DRAWN} within a byte (actual ${actual})`,
    );
    assert.deepEqual(actual.slice(1), [0, 0, 255], `${what}: the other channels are exact`);
  };
  assert.deepEqual(
    fx.firstDraw, nothing,
    "a fresh ShaderEffect's first SpriteBatch draw produces nothing -- upstream finding 22; when " +
    "it is fixed this assertion is the one that fails",
  );
  assertDrawn(fx.secondDraw, "and its second draw is correct");
  assertDrawn(fx.thirdDraw, "as is every one after that");
  assert.deepEqual(
    fx.freshEffectFirstDraw, nothing,
    "a second, separately created effect loses its own first draw too, so it is once per effect " +
    "rather than once per process",
  );
  assertDrawn(fx.freshEffectSecondDraw, "the second effect's second draw");
  assert.deepEqual(
    fx.plainFirstDraw, [40, 80, 120, 255],
    "while the same SpriteBatch with no custom effect draws correctly the first time, so it is " +
    "the effect and not the batch",
  );
  // A texture bound to sampler unit 1 and sampled there, which is the unit the caller asked for
  // rather than the one SpriteBatch drives.
  assert.deepEqual(
    fx.sampledUnitOne, [10, 200, 60, 255],
    "a texture bound to unit 1 must be the one a shader sampling unit 1 reads -- a binding that " +
    "went to unit 0 instead leaves unit 1 unbound and the shader samples nothing",
  );

  assertDrawn(
    fx.preAppliedFirstDraw,
    "and one Apply outside any batch is enough to fix it -- which is the diagnosis as well as the " +
    "workaround: the first Apply does something the draw beside it needs and does it too late",
  );

  console.log(
    `CNA_TS_WINDOWED_SHADER_EFFECT=PASS RENDERER=${evidence.renderer.name} FIRST_DRAW_LOST=yes`,
  );
});

test("volume and cube textures execute on a renderer with a device", { skip }, async () => {
  const game = new WindowedProbeGame(1);
  await game.Run();
  const seen = game.evidence.volumeAndCubeTextures;
  game.Dispose();

  assert.equal(typeof seen, "object", `the probe failed: ${seen}`);
  assert.deepEqual(seen.volume, [4, 4, 4], "a Texture3D is created at the size it was asked for");
  assert.equal(seen.cubeSize, 8, "and a TextureCube at its own");
  assert.equal(
    seen.firstTexel, seen.expectedFirst,
    "a volume texel round-trips, so this is storage and not only construction",
  );
  assert.equal(seen.lastTexel, seen.expectedLast, "and so does the last one");
  assert.notEqual(
    seen.expectedFirst, seen.expectedLast,
    "the two texels differ, so a backend returning one colour everywhere would fail above",
  );
});

/**
 * A game that resizes itself, so the window event has a real stimulus.
 *
 * The capability inventory used to say the physical resize/orientation events could not be
 * qualified because HEADLESS "exposes no physical window or event stimulus". The first half is
 * true and the second is not, on a renderer with a window: `ApplyChanges` after changing the
 * preferred back buffer is a stimulus, and the event delivers with the new bounds.
 */
class ResizingProbeGame extends Game {
  constructor() {
    super();
    this.graphics = new GraphicsDeviceManager(this);
    this.graphics.PreferredBackBufferWidth = 320;
    this.graphics.PreferredBackBufferHeight = 240;
    this.ticks = 0;
    this.events = [];
    this.sizes = {};
  }

  Initialize() {
    const window = this.Window;
    this.handler = () => this.events.push([window.ClientBounds.Width, window.ClientBounds.Height]);
    window.ClientSizeChanged.Add(this.handler);
    this.sizes.initial = [window.ClientBounds.Width, window.ClientBounds.Height];
    super.Initialize();
  }

  Update(gameTime) {
    this.ticks += 1;
    if (this.ticks === 2) {
      this.graphics.PreferredBackBufferWidth = 512;
      this.graphics.PreferredBackBufferHeight = 384;
      this.graphics.ApplyChanges();
    }
    if (this.ticks === 4) {
      this.sizes.afterApply = [this.Window.ClientBounds.Width, this.Window.ClientBounds.Height];
      // Removed before the second resize, so the handler count proves removal rather than assuming it.
      this.Window.ClientSizeChanged.Remove(this.handler);
      this.graphics.PreferredBackBufferWidth = 400;
      this.graphics.PreferredBackBufferHeight = 300;
      this.graphics.ApplyChanges();
    }
    if (this.ticks === 6) {
      this.sizes.afterRemoval = [this.Window.ClientBounds.Width, this.Window.ClientBounds.Height];
      this.Exit();
    }
    super.Update(gameTime);
  }
}

test("a physical window raises ClientSizeChanged, and stops when unsubscribed", { skip }, async () => {
  const game = new ResizingProbeGame();
  await game.Run();
  const { events, sizes } = game;
  game.Dispose();

  assert.deepEqual(sizes.initial, [320, 240], "the window starts at the size that was asked for");
  assert.deepEqual(sizes.afterApply, [512, 384], "and ApplyChanges actually resizes it");
  assert.deepEqual(
    events, [[512, 384]],
    "the event fired exactly once, carrying the new bounds -- not the old ones, which is what a " +
    "handler invoked before the resize completed would have reported",
  );
  assert.deepEqual(
    sizes.afterRemoval, [400, 300],
    "the second resize happened too, so the absence of a second event is unsubscription rather " +
    "than a resize that never occurred",
  );
});

test("a windowed CNA build enumerates the host's real capture devices", { skip }, async () => {
  const game = new WindowedProbeGame(2);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // The capability row this replaces said "HEADLESS enumerates zero microphones", which was true
  // and was about the NULL audio backend rather than about CNA. This artifact is SDL3 audio, and
  // the same routes answer with the devices attached to the machine.
  const microphones = evidence.microphones;
  assert.ok(Array.isArray(microphones), `microphone enumeration failed: ${microphones}`);
  assert.ok(
    microphones.length > 0,
    "an SDL3 audio build enumerates the host's capture devices. Zero here means one of three " +
    "things, and they are worth telling apart before this assertion is relaxed: this windowed " +
    "build has CNA_AUDIO_PLATFORM=NULL rather than SDL3 (all three on this host are SDL3), the " +
    "host genuinely has no capture device attached, or the enumeration route regressed",
  );
  for (const microphone of microphones) {
    assert.equal(typeof microphone.Name, "string");
    assert.ok(microphone.Name.length > 0, "each device carries the name the host gave it");
    assert.ok(microphone.SampleRate > 0, `a real sample rate: ${microphone.SampleRate}`);
    assert.equal(typeof microphone.IsHeadset, "boolean");
    // XNA's MicrophoneState: Started = 0, Stopped = 1. Nothing here starts a capture.
    assert.equal(microphone.State, Audio.MicrophoneState.Stopped, "no capture was started");
    // GetSampleSizeInBytes is arithmetic on the rate: 100 ms of 16-bit mono, rounded down to a
    // whole block. Asserted against the rate the device reported rather than a constant.
    const expected = Math.floor(microphone.SampleRate * 0.1) * 2;
    assert.ok(
      Math.abs(microphone.SampleSizeFor100ms - expected) <= 2 * 2,
      `100 ms at ${microphone.SampleRate} Hz is about ${expected} bytes, got ` +
      `${microphone.SampleSizeFor100ms}`,
    );
  }
  // Every device is distinct and the default is one of them. This is as far as an assertion can
  // go here: how many capture devices this host has is only knowable from the routine under test,
  // so an enumeration that dropped its LAST device is indistinguishable from a host with one
  // fewer -- a planted truncation survives this test, and is recorded as surviving rather than
  // answered with a second call to the same C route dressed up as an oracle.
  const names = microphones.map((microphone) => microphone.Name);
  assert.equal(new Set(names).size, names.length, "no device is enumerated twice");
  assert.equal(
    typeof evidence.defaultMicrophone, "string",
    "one of them is the default device",
  );
  assert.ok(
    names.includes(evidence.defaultMicrophone),
    `the default device is one of the enumerated ones: ${evidence.defaultMicrophone} in ${names}`,
  );
  console.log(`CNA_TS_MICROPHONES=${microphones.length} DEFAULT=${evidence.defaultMicrophone}`);
});

test("Microphone.BufferDuration refuses its own default -- upstream finding 28", { skip }, async () => {
  const game = new WindowedProbeGame(2);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  const measured = evidence.bufferDurationRange;
  assert.equal(typeof measured, "object", `the probe failed: ${measured}`);
  const verdict = new Map(measured.rows.map((row) => [row[0], row[1]]));

  // XNA's contract, transcribed from Microphone::set_BufferDuration in Microsoft.Xna.Framework.dll:
  // `blt 100.` and `bgt 1000.` on get_TotalMilliseconds, then a 10 ms modulus. Both comparisons are
  // strict, so the range is [100, 1000] with both endpoints legal.
  const xnaAccepts = (milliseconds) =>
    milliseconds >= 100 && milliseconds <= 1000 && milliseconds % 10 === 0;

  // Where the two agree, they are asserted to agree.
  for (const milliseconds of [50, 90, 100, 500, 990, 60000]) {
    assert.equal(
      verdict.get(milliseconds) === "accepted", xnaAccepts(milliseconds),
      `${milliseconds} ms: CNA and XNA agree here`,
    );
  }

  // Where they differ, the difference is asserted rather than tolerated, so a repair fails here.
  // CNA reads TimeSpan's sub-second component where XNA reads the total, so 1000 ms arrives as 0.
  assert.equal(
    verdict.get(1000), "Error",
    "CNA refuses 1000 ms, which XNA accepts -- when this starts passing, finding 28 is fixed",
  );
  assert.equal(
    measured.initial, 1000,
    "and 1000 ms is the value the property reports before anything writes it, so reading the " +
    "property and assigning it straight back cannot complete",
  );
  for (const milliseconds of [1100, 1500, 2500]) {
    assert.equal(
      verdict.get(milliseconds), "accepted",
      `CNA accepts ${milliseconds} ms, which XNA refuses as above ${1000}`,
    );
    assert.equal(xnaAccepts(milliseconds), false, "and XNA's own IL refuses it");
  }
  // 60000 ms is refused by both, but only by coincidence: its sub-second component is zero, which
  // fails the lower bound rather than the upper one. Recorded so the agreement above is not read
  // as evidence that the upper bound works.
  assert.equal(verdict.get(60000), "Error");
});

test("a real audio backend runs the playback state machine, not just its lifetime", { skip }, async () => {
  const game = new WindowedProbeGame(2);
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  const sound = evidence.soundEffect;
  assert.equal(typeof sound, "object", `the probe failed: ${sound}`);

  // On the NULL audio backend every one of these is Stopped, which is what "verifies state and
  // lifetime only" meant. Four distinct transitions is the evidence a real mixer is behind them.
  assert.deepEqual(
    sound.states,
    {
      initial: Audio.SoundState.Stopped,
      playing: Audio.SoundState.Playing,
      paused: Audio.SoundState.Paused,
      resumed: Audio.SoundState.Playing,
      stopped: Audio.SoundState.Stopped,
    },
    "Play, Pause, Resume and Stop each move the instance to their own state",
  );

  // Duration is arithmetic on the fixture, not a value read back from the thing under test.
  const expectedMs = (sound.frames / sound.sampleRate) * 1000;
  assert.equal(expectedMs, 200, "4410 frames at 22050 Hz is 200 ms");
  assert.ok(
    Math.abs(sound.durationMs - expectedMs) < 1,
    `SoundEffect.Duration is the fixture's own length: ${sound.durationMs} vs ${expectedMs}`,
  );

  assert.equal(sound.Volume, 0.5);
  assert.equal(sound.Pitch, -0.25);
  assert.equal(sound.Pan, 0.75);

  // Stated as narrowly as it was measured: this is a real mixer running a real state machine over
  // a real device, and nobody listened to it. Audible output stays unverified here.
  console.log(`CNA_TS_AUDIO_STATE_MACHINE=PASS DURATION_MS=${sound.durationMs} AUDIBLE=UNVERIFIED`);
});
