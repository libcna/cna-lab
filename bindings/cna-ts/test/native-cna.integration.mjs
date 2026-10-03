import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import test, { after } from "node:test";
import { spawnSync } from "node:child_process";
import { pathToFileURL } from "node:url";

import {
  Audio,
  BoundingBox,
  BoundingFrustum,
  BoundingSphere,
  Color,
  Content,
  Game,
  Graphics,
  GraphicsDeviceManager,
  GetRuntimeStatus,
  Input,
  LoadNodeNativeBackend,
  Media,
  PlayerIndex,
  Rectangle,
  GamerServices,
  Storage,
  TimeSpan,
  TitleContainer,
  Matrix,
  NativeUnavailableError,
  Point,
  Quaternion,
  Vector2,
  Vector3,
  Vector4,
} from "../dist/index.js";
import { CNA_ABI_MAJOR, CNA_ABI_MINOR } from "../dist/internal/abi.js";
import * as computeExtensions from "../dist/extensions/graphics/index.js";
import * as extensionsModule from "../dist/extensions/index.js";
import * as devicesModule from "../dist/extensions/devices/index.js";
import * as inputModule from "../dist/extensions/input/index.js";
import * as sensorsModule from "../dist/extensions/sensors/index.js";
import * as guideExtensions from "../dist/extensions/gamer-services/index.js";
import { getBackend } from "../dist/internal/backend.js";
import { CnaResult } from "../dist/internal/cna-results.js";
import { resolveGraphicsDeviceHandleForInternalUse } from
  "../dist/internal/graphics-device-registry.js";
import {
  getVertexBufferRawForInternalUse,
  setVertexBufferRawForInternalUse,
} from "../dist/Microsoft/Xna/Framework/Graphics/VertexBuffer.js";
import {
  getIndexBufferRawForInternalUse,
  setIndexBufferRawForInternalUse,
} from "../dist/Microsoft/Xna/Framework/Graphics/IndexBuffer.js";
import {
  compressedXnb, modelVertexBytes, modelXnb, spriteFontXnb, textureXnb,
} from "./fixtures/xnb.mjs";

const library = process.env.CNA_NATIVE_LIBRARY;
if (!library) {
  throw new Error(
    `CNA_NATIVE_LIBRARY must name an existing CNA C ABI ${CNA_ABI_MAJOR}.${CNA_ABI_MINOR}.x shared library`,
  );
}
const nativeStorageHome = fs.mkdtempSync(path.join(os.tmpdir(), "cna-ts-native-storage-"));
process.env.XDG_DATA_HOME = nativeStorageHome;
after(() => fs.rmSync(nativeStorageHome, { recursive: true, force: true }));
const bridge = path.resolve(process.env.CNA_NODE_BRIDGE ?? "build/cna_node_bridge.node");
// Counted from the adapter source rather than restated here, so the runtime-reported import count
// is cross-checked against the declarations that produced it instead of against a copied number.
const EXPECTED_IMPORTED_SYMBOLS = (
  fs.readFileSync(new URL("../native/cna_node_bridge.c", import.meta.url), "utf8")
    .match(/LOAD_REQUIRED\([^\n]*?"cna_[A-Za-z0-9_]+"\)/g) ?? []
).length;
const status = await LoadNodeNativeBackend({
  CnaLibrary: path.resolve(library),
  BridgeModule: bridge,
});

class NativeProbeGame extends Game {
  constructor(frameTarget, leaveTextureLive = false) {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.frameTarget = frameTarget;
    this.leaveTextureLive = leaveTextureLive;
    this.updates = 0;
    this.draws = 0;
    this.texture = null;
    this.spriteBatch = null;
    this.inputPolls = 0;
    this.graphicsRouteEvidence = Object.create(null);
  }

  qualifyHeadlessRoute(name, action, allowedResults = [6]) {
    try {
      const value = action();
      this.graphicsRouteEvidence[name] = "SUCCESS";
      return value;
    } catch (error) {
      assert.ok(
        allowedResults.includes(error?.cnaResult),
        `${name} returned unexpected CNA result ${error?.cnaResult}`,
      );
      this.graphicsRouteEvidence[name] = error.cnaResult === 6
        ? "HEADLESS_NOT_SUPPORTED"
        : "HEADLESS_PIPELINE_UNAVAILABLE";
      return null;
    }
  }

  LoadContent() {
    assert.equal(this.Window, this.Window, "Game.Window must preserve facade identity");
    assert.equal(this.Window.Handle, 0n, "HEADLESS has no XNA round-trip window token");
    assert.equal(typeof this.Window.AllowUserResizing, "boolean");
    assert.equal(typeof this.Window.ClientBounds.Width, "number");
    assert.equal(typeof this.Window.CurrentOrientation, "number");
    assert.equal(typeof this.Window.ScreenDeviceName, "string");
    this.Window.Title = "cna-ts native probe";
    assert.equal(this.Window.Title, "cna-ts native probe");
    const titlePackage = JSON.parse(new TextDecoder().decode(TitleContainer.OpenStream("package.json")));
    assert.equal(titlePackage.name, "cna-ts");
    this.texture = new Graphics.Texture2D(
      this.GraphicsDevice,
      4,
      4,
      false,
      Graphics.SurfaceFormat.Color,
    );
    const pixels = Array.from({ length: 16 }, (_value, index) =>
      new Color(index, 255 - index, index * 3, 255));
    this.texture.SetData(pixels);
    const roundTrip = new Array(16);
    this.texture.GetData(roundTrip);
    assert.deepEqual(
      roundTrip.map((value) => value.PackedValue),
      pixels.map((value) => value.PackedValue),
    );

    const region = [Color.Red, Color.Green, Color.Blue, Color.White];
    const regionSource = [Color.Black, Color.Black, ...region, Color.Black, Color.Black];
    this.texture.SetData(0, new Rectangle(1, 1, 2, 2), regionSource, 2, region.length);
    const regionRoundTrip = new Array(8);
    this.texture.GetData(0, new Rectangle(1, 1, 2, 2), regionRoundTrip, 2, 4);
    assert.deepEqual(
      regionRoundTrip.slice(2, 6).map((value) => value.PackedValue),
      region.map((value) => value.PackedValue),
    );

    this.mipTexture = new Graphics.Texture2D(
      this.GraphicsDevice, 4, 4, true, Graphics.SurfaceFormat.Color,
    );
    const mipPixels = [Color.Red, Color.Green, Color.Blue, Color.White];
    this.mipTexture.SetData(1, null, mipPixels, 0, 4);
    const mipRoundTrip = new Array(4);
    this.mipTexture.GetData(1, null, mipRoundTrip, 0, 4);
    assert.deepEqual(
      mipRoundTrip.map((value) => value.PackedValue),
      mipPixels.map((value) => value.PackedValue),
    );

    const encoded = Uint8Array.from(Buffer.from(
      "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=",
      "base64",
    ));
    this.decodedTexture = Graphics.Texture2D.FromStream(this.GraphicsDevice, encoded);
    assert.deepEqual([this.decodedTexture.Width, this.decodedTexture.Height], [1, 1]);
    const png = new Uint8Array(1024);
    this.decodedTexture.SaveAsPng(png, 1, 1);
    assert.deepEqual([...png.subarray(0, 8)], [137, 80, 78, 71, 13, 10, 26, 10]);
    this.spriteBatch = new Graphics.SpriteBatch(this.GraphicsDevice);
    const fontBytes = compressedXnb(spriteFontXnb());
    const modelBytes = compressedXnb(modelXnb());
    const externalTextureBytes = compressedXnb(textureXnb());
    class FontContentManager extends Content.ContentManager {
      OpenStream(assetName) {
        if (assetName === "SyntheticFont") return fontBytes;
        if (assetName === "Models\\SyntheticModel") return modelBytes;
        if (assetName === "Textures\\Atlas") return externalTextureBytes;
        throw new Error(`Unknown synthetic asset ${assetName}`);
      }
    }
    this.fontContent = new FontContentManager({
      GetService: (type) => type === Graphics.GraphicsDevice ? this.GraphicsDevice : null,
    });
    this.font = this.fontContent.Load(Graphics.SpriteFont, "SyntheticFont");
    assert.equal(this.fontContent.Load(Graphics.SpriteFont, ".\\SyntheticFont"), this.font);
    assert.deepEqual(
      [this.font.MeasureString("A?").X, this.font.MeasureString("A?").Y],
      [9, 8],
    );
    this.model = this.fontContent.Load(Graphics.Model, "Models/SyntheticModel");
    assert.equal(this.fontContent.Load(Graphics.Model, "Models\\SyntheticModel"), this.model);
    assert.equal(this.model.Root, this.model.Bones.Get("Root"));
    assert.equal(this.model.Meshes.Get("Triangle").ParentBone, this.model.Root);
    const loadedPart = this.model.Meshes.Get(0).MeshParts.Get(0);
    assert.equal(loadedPart.VertexBuffer.VertexCount, 3);
    assert.equal(loadedPart.IndexBuffer.IndexCount, 3);
    assert.equal(loadedPart.Effect, this.model.Meshes.Get(0).Effects.Get(0));
    assert.deepEqual(
      [...getVertexBufferRawForInternalUse(loadedPart.VertexBuffer)],
      modelVertexBytes(),
    );
    assert.deepEqual(
      [...getIndexBufferRawForInternalUse(loadedPart.IndexBuffer)],
      [0, 0, 1, 0, 2, 0],
    );
    assert.deepEqual(loadedPart.Effect.DiffuseColor, Vector3.One);
    this.externalTexture = this.fontContent.Load(Graphics.Texture2D, "Textures/Atlas");
    assert.equal(loadedPart.Effect.Texture, this.externalTexture);
    this.loadedModelVertexBuffer = loadedPart.VertexBuffer;
    this.loadedModelIndexBuffer = loadedPart.IndexBuffer;
    this.loadedModelEffect = loadedPart.Effect;
    this.stockEffects = [
      new Graphics.BasicEffect(this.GraphicsDevice),
      new Graphics.AlphaTestEffect(this.GraphicsDevice),
      new Graphics.DualTextureEffect(this.GraphicsDevice),
      new Graphics.EnvironmentMapEffect(this.GraphicsDevice),
      new Graphics.SkinnedEffect(this.GraphicsDevice),
    ];
    for (const effect of this.stockEffects) {
      assert.ok(effect.Techniques.Count > 0);
      assert.ok(effect.CurrentTechnique.Passes.Count > 0);
      effect.CurrentTechnique.Passes.Get(0).Apply();
      effect.OnApply();
    }
    this.graphicsRouteEvidence["stock effect construction"] = "SUCCESS";
    this.graphicsRouteEvidence["stock effect execution"] = "SUCCESS";
    const clonedStock = this.stockEffects[0].Clone();
    clonedStock.CurrentTechnique.Passes.Get(0).Apply();
    clonedStock.Dispose();
    const disposedEffect = new Graphics.BasicEffect(this.GraphicsDevice);
    const disposedPass = disposedEffect.CurrentTechnique.Passes.Get(0);
    disposedEffect.Dispose();
    assert.throws(() => disposedPass.Apply(), { name: "ObjectDisposedException" });
    const retainedTexture = new Graphics.Texture2D(this.GraphicsDevice, 1, 1);
    const retainingEffect = new Graphics.BasicEffect(this.GraphicsDevice);
    retainingEffect.Texture = retainedTexture;
    retainingEffect.TextureEnabled = true;
    retainingEffect.CurrentTechnique.Passes.Get(0).Apply();
    retainedTexture.Dispose();
    assert.equal(retainedTexture.IsDisposed, true);
    retainingEffect.Dispose();

    const cnaSource = path.resolve(
      process.env.CNA_SOURCE_PATH ?? new URL("../../cna", import.meta.url).pathname);
    const compiledBytes = fs.readFileSync(path.join(
      cnaSource, "modules/renderers/fna3d/effects/CnaConformanceEffect.fxb",
    ));
    for (let attempt = 0; attempt < 3; attempt += 1) {
      assert.throws(
        () => new Graphics.Effect(this.GraphicsDevice, [...compiledBytes]),
        (error) => error.operation === "cna_effect_create_compiled" && error.cnaResult === 6,
      );
    }
    this.graphicsRouteEvidence["compiled Effect route"] = "HEADLESS_NOT_SUPPORTED";
    const declaration = new Graphics.VertexDeclaration(12, [
      new Graphics.VertexElement(
        0,
        Graphics.VertexElementFormat.Vector3,
        Graphics.VertexElementUsage.Position,
        0,
      ),
    ]);
    this.vertexBuffer = new Graphics.VertexBuffer(
      this.GraphicsDevice, declaration, 3, Graphics.BufferUsage.None,
    );
    const vertexBytes = Uint8Array.from({ length: 36 }, (_value, index) => index);
    setVertexBufferRawForInternalUse(this.vertexBuffer, vertexBytes);
    assert.deepEqual(getVertexBufferRawForInternalUse(this.vertexBuffer), vertexBytes);
    this.indexBuffer = new Graphics.IndexBuffer(
      this.GraphicsDevice,
      Graphics.IndexElementSize.SixteenBits,
      3,
      Graphics.BufferUsage.None,
    );
    const indexBytes = Uint8Array.from([0, 0, 1, 0, 2, 0]);
    setIndexBufferRawForInternalUse(this.indexBuffer, indexBytes);
    assert.deepEqual(getIndexBufferRawForInternalUse(this.indexBuffer), indexBytes);

    assert.equal(this.GraphicsDevice.GraphicsDeviceStatus, Graphics.GraphicsDeviceStatus.Normal);
    this.blendState = new Graphics.BlendState();
    this.depthState = new Graphics.DepthStencilState();
    this.rasterizerState = new Graphics.RasterizerState();
    this.samplerState = new Graphics.SamplerState();
    this.GraphicsDevice.BlendState = this.blendState;
    this.GraphicsDevice.DepthStencilState = this.depthState;
    this.GraphicsDevice.RasterizerState = this.rasterizerState;
    this.GraphicsDevice.SamplerStates.Set(0, this.samplerState);
    assert.equal(this.GraphicsDevice.BlendState, this.blendState);
    assert.equal(this.GraphicsDevice.DepthStencilState, this.depthState);
    assert.equal(this.GraphicsDevice.RasterizerState, this.rasterizerState);
    assert.equal(this.GraphicsDevice.SamplerStates.Get(0), this.samplerState);
    this.GraphicsDevice.Textures.Set(0, this.texture);
    assert.equal(this.GraphicsDevice.Textures.Get(0), this.texture);
    this.GraphicsDevice.Textures.Set(0, null);
    assert.equal(this.GraphicsDevice.Textures.Get(0), null);

    const vertices = [
      new Graphics.VertexPositionColor(new Vector3(0, 0, 0), Color.Red),
      new Graphics.VertexPositionColor(new Vector3(1, 0, 0), Color.Green),
      new Graphics.VertexPositionColor(new Vector3(0, 1, 0), Color.Blue),
    ];
    this.userVertices = vertices;
    this.dynamicVertexBuffer = this.qualifyHeadlessRoute("dynamic vertex buffer", () => {
      const buffer = new Graphics.DynamicVertexBuffer(
        this.GraphicsDevice, Graphics.VertexPositionColor, 3, Graphics.BufferUsage.None,
      );
      try {
        buffer.SetData(vertices, 0, 3, Graphics.SetDataOptions.Discard);
        const roundTrip = new Array(3);
        buffer.GetData(roundTrip, 0, 3);
        assert.deepEqual(
          roundTrip.map((value) => value.ToString()),
          vertices.map((value) => value.ToString()),
        );
        assert.equal(buffer.IsContentLost, false);
        return buffer;
      } catch (error) {
        buffer.Dispose();
        throw error;
      }
    });
    this.dynamicIndexBuffer = this.qualifyHeadlessRoute("dynamic index buffer", () => {
      const buffer = new Graphics.DynamicIndexBuffer(
        this.GraphicsDevice, Graphics.IndexElementSize.SixteenBits, 3,
        Graphics.BufferUsage.None,
      );
      try {
        buffer.SetData([0, 1, 2], 0, 3, Graphics.SetDataOptions.NoOverwrite);
        const roundTrip = new Array(3);
        buffer.GetData(roundTrip);
        assert.deepEqual(roundTrip, [0, 1, 2]);
        assert.equal(buffer.IsContentLost, false);
        return buffer;
      } catch (error) {
        buffer.Dispose();
        throw error;
      }
    });

    this.renderTarget = this.qualifyHeadlessRoute("RenderTarget2D creation", () => {
      const target = new Graphics.RenderTarget2D(this.GraphicsDevice, 4, 4);
      assert.deepEqual([target.Width, target.Height], [4, 4]);
      assert.equal(target.Format, Graphics.SurfaceFormat.Color);
      assert.equal(target.IsContentLost, false);
      return target;
    });
    this.renderTargetCube = this.qualifyHeadlessRoute("RenderTargetCube creation", () => {
      const target = new Graphics.RenderTargetCube(
        this.GraphicsDevice, 4, false, Graphics.SurfaceFormat.Color, Graphics.DepthFormat.None,
      );
      assert.equal(target.Size, 4);
      assert.equal(target.Format, Graphics.SurfaceFormat.Color);
      assert.equal(target.IsContentLost, false);
      return target;
    });

    this.qualifyHeadlessRoute("Texture3D lifecycle", () => {
      const texture = new Graphics.Texture3D(
        this.GraphicsDevice, 2, 2, 2, false, Graphics.SurfaceFormat.Color,
      );
      try {
        const values = Array.from({ length: 8 }, (_value, index) =>
          new Color(index, index + 1, index + 2, 255));
        texture.SetData(values);
        const output = new Array(8);
        texture.GetData(output);
        assert.deepEqual(
          output.map((value) => value.PackedValue),
          values.map((value) => value.PackedValue),
        );
      } finally {
        texture.Dispose();
        texture.Dispose();
      }
    });
    this.qualifyHeadlessRoute("TextureCube lifecycle", () => {
      const texture = new Graphics.TextureCube(
        this.GraphicsDevice, 2, false, Graphics.SurfaceFormat.Color,
      );
      try {
        const values = [Color.Red, Color.Green, Color.Blue, Color.White];
        texture.SetData(Graphics.CubeMapFace.PositiveX, values);
        const output = new Array(4);
        texture.GetData(Graphics.CubeMapFace.PositiveX, output);
        assert.deepEqual(
          output.map((value) => value.PackedValue),
          values.map((value) => value.PackedValue),
        );
      } finally {
        texture.Dispose();
        texture.Dispose();
      }
    });
    this.occlusionQuery = this.qualifyHeadlessRoute("OcclusionQuery lifecycle", () => {
      const query = new Graphics.OcclusionQuery(this.GraphicsDevice);
      try {
        assert.equal(query.IsComplete, false);
        query.Begin();
        query.End();
        if (query.IsComplete) assert.ok(Number.isInteger(query.PixelCount));
        query.Begin();
        query.End();
        return query;
      } catch (error) {
        query.Dispose();
        query.Dispose();
        throw error;
      }
    });
  }

  Update(gameTime) {
    super.Update(gameTime);
    if (this.inputPolls === 0) {
      assert.deepEqual(Input.Keyboard.GetState().GetPressedKeys(), []);
      assert.equal(typeof Input.Mouse.GetState().X, "number");
      assert.equal(Input.GamePad.GetState(PlayerIndex.One).IsConnected, false);
      assert.equal(Input.GamePad.GetCapabilities(PlayerIndex.One).IsConnected, false);
      assert.equal(Input.GamePad.SetVibration(PlayerIndex.One, 0, 0), false);
      assert.equal(Input.Touch.TouchPanel.GetState().IsConnected, false);
      assert.equal(Input.Touch.TouchPanel.GetCapabilities().IsConnected, false);
      assert.equal(Input.Touch.TouchPanel.IsGestureAvailable, false);
      assert.equal(Input.Mouse.WindowHandle, 0n);
      assert.equal(Input.Touch.TouchPanel.WindowHandle, 0n);
      Input.Mouse.SetPosition(0, 0);
      this.inputPolls += 1;
    }
    this.updates += 1;
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    assert.equal(this.texture.GraphicsDevice, this.GraphicsDevice);
    if (this.draws === 0) {
      this.qualifyHeadlessRoute("Model.Draw", () =>
        this.model.Draw(Matrix.Identity, Matrix.Identity, Matrix.Identity), [6, 12]);
      if (this.dynamicVertexBuffer && this.dynamicIndexBuffer) {
        this.qualifyHeadlessRoute("vertex buffer binding", () => {
          this.GraphicsDevice.SetVertexBuffer(this.dynamicVertexBuffer);
          assert.equal(this.GraphicsDevice.GetVertexBuffers()[0].VertexBuffer, this.dynamicVertexBuffer);
        });
        this.qualifyHeadlessRoute("index buffer binding", () => {
          this.GraphicsDevice.Indices = this.dynamicIndexBuffer;
          assert.equal(this.GraphicsDevice.Indices, this.dynamicIndexBuffer);
        });
        if (this.GraphicsDevice.GetVertexBuffers().length > 0 && this.GraphicsDevice.Indices) {
          this.qualifyHeadlessRoute("DrawPrimitives", () =>
            this.GraphicsDevice.DrawPrimitives(Graphics.PrimitiveType.TriangleList, 0, 1), [6, 12]);
          this.qualifyHeadlessRoute("DrawIndexedPrimitives", () =>
            this.GraphicsDevice.DrawIndexedPrimitives(
              Graphics.PrimitiveType.TriangleList, 0, 0, 3, 0, 1,
            ), [6, 12]);
          this.qualifyHeadlessRoute("DrawInstancedPrimitives", () =>
            this.GraphicsDevice.DrawInstancedPrimitives(
              Graphics.PrimitiveType.TriangleList, 0, 0, 3, 0, 1, 2,
            ), [6, 12]);
        }
      }
      this.qualifyHeadlessRoute("DrawUserPrimitives", () =>
        this.GraphicsDevice.DrawUserPrimitives(
          Graphics.PrimitiveType.TriangleList, this.userVertices, 0, 1,
        ), [6, 12]);
      this.qualifyHeadlessRoute("DrawUserIndexedPrimitives", () =>
        this.GraphicsDevice.DrawUserIndexedPrimitives(
          Graphics.PrimitiveType.TriangleList, this.userVertices, 0, 3, [0, 1, 2], 0, 1,
        ), [6, 12]);
      if (this.renderTarget) {
        this.qualifyHeadlessRoute("render target binding", () => {
          this.GraphicsDevice.SetRenderTarget(this.renderTarget);
          assert.equal(this.GraphicsDevice.GetRenderTargets()[0].RenderTarget, this.renderTarget);
          this.GraphicsDevice.SetRenderTarget(null);
          assert.equal(this.GraphicsDevice.GetRenderTargets().length, 0);
        });
      }
      if (this.renderTargetCube) {
        this.qualifyHeadlessRoute("cube render target binding", () => {
          this.GraphicsDevice.SetRenderTarget(
            this.renderTargetCube, Graphics.CubeMapFace.NegativeZ,
          );
          const binding = this.GraphicsDevice.GetRenderTargets()[0];
          assert.equal(binding.RenderTarget, this.renderTargetCube);
          assert.equal(binding.CubeMapFace, Graphics.CubeMapFace.NegativeZ);
          this.GraphicsDevice.SetRenderTarget(null);
          assert.equal(this.GraphicsDevice.GetRenderTargets().length, 0);
        });
      }
    }
    if (this.draws === 0) {
      this.spriteBatch.Begin(
        Graphics.SpriteSortMode.Deferred,
        this.blendState,
        this.samplerState,
        this.depthState,
        this.rasterizerState,
        this.loadedModelEffect,
      );
      assert.throws(
        () => this.loadedModelEffect.Dispose(),
        { name: "InvalidOperationException", message: /active SpriteBatch interval/ },
      );
      this.graphicsRouteEvidence["effect SpriteBatch.Begin"] = "SUCCESS";
    } else {
      this.spriteBatch.Begin();
    }
    this.spriteBatch.Draw(this.texture, new Vector2(this.draws % 32, 12), Color.White);
    this.spriteBatch.Draw(
      this.decodedTexture,
      new Rectangle(48, 12, 8, 8),
      Color.White,
    );
    this.spriteBatch.DrawString(this.font, "A?", new Vector2(64, 12), Color.White);
    this.spriteBatch.End();
    this.draws += 1;
    if (this.draws >= this.frameTarget) this.Exit();
    super.Draw(gameTime);
  }

  UnloadContent() {
    this.spriteBatch?.Dispose();
    this.spriteBatch?.Dispose();
    this.spriteBatch = null;
    for (const effect of this.stockEffects ?? []) effect.Dispose();
    this.stockEffects = null;
    this.fontContent?.Dispose();
    assert.equal(this.externalTexture?.IsDisposed, true);
    this.fontContent = null;
    this.externalTexture = null;
    this.font = null;
    this.model = null;
    assert.equal(this.loadedModelVertexBuffer?.IsDisposed, true);
    assert.equal(this.loadedModelIndexBuffer?.IsDisposed, true);
    assert.equal(this.loadedModelEffect?.IsDisposed, true);
    this.vertexBuffer?.Dispose();
    this.vertexBuffer?.Dispose();
    this.vertexBuffer = null;
    this.indexBuffer?.Dispose();
    this.indexBuffer?.Dispose();
    this.indexBuffer = null;
    if (!this.leaveTextureLive) {
      this.dynamicVertexBuffer?.Dispose();
      this.dynamicVertexBuffer?.Dispose();
      this.dynamicVertexBuffer = null;
      this.dynamicIndexBuffer?.Dispose();
      this.dynamicIndexBuffer?.Dispose();
      this.dynamicIndexBuffer = null;
      this.renderTarget?.Dispose();
      this.renderTarget?.Dispose();
      this.renderTarget = null;
      this.renderTargetCube?.Dispose();
      this.renderTargetCube?.Dispose();
      this.renderTargetCube = null;
      this.occlusionQuery?.Dispose();
      this.occlusionQuery?.Dispose();
      this.occlusionQuery = null;
    }
    this.mipTexture?.Dispose();
    this.mipTexture = null;
    if (!this.leaveTextureLive) {
      this.decodedTexture?.Dispose();
      this.decodedTexture = null;
      this.texture?.Dispose();
      this.texture?.Dispose();
      this.texture = null;
    }
    super.UnloadContent();
  }
}

test("loads an exact real CNA ABI and only the audited symbols", () => {
  assert.equal(status.Backend, "node-native");
  assert.equal(status.IsAvailable, true);
  // The loaded artifact must fall inside the generation this package targets. The patch component
  // is free, so the assertion names the window rather than one exact build.
  const [major, minor] = String(status.AbiVersion).split(".").map(Number);
  assert.equal(major, CNA_ABI_MAJOR);
  assert.equal(minor, CNA_ABI_MINOR);
  assert.equal(status.ImportedSymbolCount, EXPECTED_IMPORTED_SYMBOLS);
});

class ContentLostProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    // ABI 0.9 made ContentLost a real event on renderers whose API can lose a device. The
    // subscription and its deterministic release are what a HEADLESS renderer can prove; the event
    // firing is not, because HEADLESS has no device to lose.
    let raised = 0;
    const target = new Graphics.RenderTarget2D(this.GraphicsDevice, 16, 16);
    const onLost = () => { raised += 1; };
    target.ContentLost.Add(onLost);
    this.evidence.renderTargetSubscribed = true;
    this.evidence.renderTargetIsContentLost = target.IsContentLost;
    target.Dispose();
    this.evidence.renderTargetDisposedAfterSubscription = target.IsDisposed;

    const vertex = new Graphics.DynamicVertexBuffer(
      this.GraphicsDevice, Graphics.VertexPositionColor.VertexDeclaration, 4,
      Graphics.BufferUsage.WriteOnly,
    );
    vertex.ContentLost.Add(onLost);
    this.evidence.vertexSubscribed = true;
    this.evidence.vertexIsContentLost = vertex.IsContentLost;
    vertex.Dispose();
    // Disposing twice must stay harmless with a live registration behind the resource.
    vertex.Dispose();
    this.evidence.vertexDisposedTwice = vertex.IsDisposed;

    const index = new Graphics.DynamicIndexBuffer(
      this.GraphicsDevice, Graphics.IndexElementSize.SixteenBits, 6, Graphics.BufferUsage.WriteOnly,
    );
    index.ContentLost.Add(onLost);
    this.evidence.indexSubscribed = true;
    index.Dispose();

    this.evidence.raised = raised;
    this.Exit();
    super.LoadContent();
  }
}

test("ContentLost subscriptions reach CNA and release with their resource", async () => {
  const game = new ContentLostProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();
  assert.equal(evidence.renderTargetSubscribed, true);
  assert.equal(evidence.vertexSubscribed, true);
  assert.equal(evidence.indexSubscribed, true);
  assert.equal(evidence.renderTargetIsContentLost, false);
  assert.equal(evidence.vertexIsContentLost, false);
  assert.equal(evidence.renderTargetDisposedAfterSubscription, true);
  assert.equal(evidence.vertexDisposedTwice, true);
  // HEADLESS cannot lose a device, so the producer never runs here. This asserts the honest
  // number rather than pretending a renderer raised an event it has no way to raise.
  assert.equal(evidence.raised, 0);
});

class HostDeviceProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    // Availability first. Every route in this family exists in every CNA build and refuses where
    // the extension layer is compiled out, so a refusal is about the build rather than about the
    // machine -- and reading it the other way is exactly the mistake this question prevents.
    this.evidence.layerAvailable = devicesModule.CnaDevices.IsAvailable();
    if (!this.evidence.layerAvailable) {
      try {
        devicesModule.CnaDevices.GetHostInfo();
        this.evidence.refusedWithoutLayer = null;
      } catch (error) {
        this.evidence.refusedWithoutLayer = error.cnaResult;
      }
      this.Exit();
      super.LoadContent();
      return;
    }

    const host = devicesModule.CnaDevices.GetHostInfo();
    this.evidence.host = {
      cores: host.LogicalCpuCoreCount,
      ram: host.SystemRamMegabytes,
      powerState: host.Power.State,
      batteryPercent: host.Power.BatteryPercent,
      secondsRemaining: host.Power.SecondsRemaining,
      contentScale: host.Display.ContentScale,
      safeArea: [
        host.Display.SafeArea.X, host.Display.SafeArea.Y,
        host.Display.SafeArea.Width, host.Display.SafeArea.Height,
      ],
    };
    this.evidence.locales = devicesModule.CnaDevices.GetPreferredLocales()
      .map((locale) => ({ language: locale.Language, country: locale.Country }));
    this.evidence.clipboardAccepted = devicesModule.CnaDevices.SetClipboardText("cna-ts");
    const cameras = devicesModule.CnaDevices.GetCameras();
    this.evidence.cameras = {
      supported: cameras.IsSupported,
      count: cameras.Devices.length,
      names: cameras.Devices.map((camera) => camera.Name),
    };
    this.Exit();
    super.LoadContent();
  }
}

test("the extended device layer reports the host truthfully, absences included", async () => {
  const game = new HostDeviceProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();
  assert.equal(typeof evidence.layerAvailable, "boolean");
  if (!evidence.layerAvailable) {
    assert.equal(evidence.refusedWithoutLayer, 6, "NOT_SUPPORTED without the device layer");
    return;
  }

  // A fact about this machine, not a placeholder, and checked against an independent count.
  assert.ok(evidence.host.cores >= 1, `expected at least one core, saw ${evidence.host.cores}`);
  assert.equal(evidence.host.cores, os.cpus().length, "CNA counts the same cores Node does");

  // Memory is a different story on this build and the honest number is zero. SDL answers it, and
  // the HEADLESS platform initialises no SDL subsystem to answer with, so CNA reports none. That
  // is recorded rather than asserted away: a windowed build reports the real figure, and this
  // assertion accepts either without inventing one.
  assert.ok(evidence.host.ram >= 0, `expected a non-negative memory figure, saw ${evidence.host.ram}`);
  // Zero is honest where no SDL subsystem was initialised to answer; otherwise it is the host's
  // real figure in megabytes, which Node can cross-check to within rounding.
  if (evidence.host.ram !== 0) {
    const nodeMegabytes = os.totalmem() / (1024 * 1024);
    assert.ok(Math.abs(evidence.host.ram - nodeMegabytes) / nodeMegabytes < 0.05,
      `CNA reports ${evidence.host.ram} MB and Node ${Math.round(nodeMegabytes)} MB`);
  }

  // The power state is one of CNA's six identities. Which one depends on the machine, so the
  // assertion is the range rather than a value this host happens to have today.
  assert.ok(
    Object.values(devicesModule.PowerState).includes(evidence.host.powerState),
    `unexpected power state ${evidence.host.powerState}`,
  );
  // An absent charge is null, never a number: a consumer comparing a percentage against a low
  // threshold must not read "no battery fitted" as "nearly flat".
  for (const value of [evidence.host.batteryPercent, evidence.host.secondsRemaining]) {
    assert.ok(value === null || value >= 0, `expected null or a non-negative value, saw ${value}`);
  }
  if (evidence.host.powerState === devicesModule.PowerState.NoBattery) {
    assert.equal(evidence.host.batteryPercent, null, "no battery has no charge to report");
  }

  // A windowless session answers a zero content scale and an empty safe area -- CNA's documented
  // answer rather than a failure to read one. A windowed one answers its window's real figures.
  if (GetRuntimeStatus().RendererInfo?.Name === "HEADLESS") {
    assert.equal(evidence.host.contentScale, 0, "a windowless session has no content scale");
    assert.deepEqual(evidence.host.safeArea, [0, 0, 0, 0]);
  } else {
    assert.ok(evidence.host.contentScale > 0, `a window has a content scale: ${evidence.host.contentScale}`);
    assert.ok(evidence.host.safeArea[2] > 0 && evidence.host.safeArea[3] > 0, "and a safe area");
  }

  // Locales come back as language/country pairs in the platform's own preference order.
  assert.ok(Array.isArray(evidence.locales));
  for (const locale of evidence.locales) {
    assert.match(locale.language, /^[a-z]{2,3}$/i, `unexpected language ${locale.language}`);
    assert.equal(typeof locale.country, "string");
  }

  // A platform with no clipboard answers false. Either answer is truthful; a throw would not be.
  assert.equal(typeof evidence.clipboardAccepted, "boolean");

  // "The platform has no cameras" and "the platform has camera support and none attached" are
  // different situations, and the inventory keeps them apart.
  assert.equal(typeof evidence.cameras.supported, "boolean");
  assert.equal(evidence.cameras.names.length, evidence.cameras.count);
  if (!evidence.cameras.supported) assert.equal(evidence.cameras.count, 0);
});

class GamerServicesProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    const gs = GamerServices;
    // Before Initialize, CNA already knows the dispatcher is not initialised. It does *not* refuse
    // the pump, though -- measured, not assumed -- so the ordering rule XNA enforces is enforced
    // here, which is where a projection's job is.
    this.evidence.initializedBefore = gs.GamerServicesDispatcher.IsInitialized;
    try {
      gs.GamerServicesDispatcher.Update();
      this.evidence.updateBeforeInitialize = "allowed";
    } catch (error) {
      this.evidence.updateBeforeInitialize = error.cnaResult ?? error.constructor.name;
    }

    gs.GamerServicesDispatcher.Initialize(this.Services);
    this.evidence.initializedAfter = gs.GamerServicesDispatcher.IsInitialized;
    gs.GamerServicesDispatcher.Update();
    this.evidence.updateAfterInitialize = "SUCCESS";

    // A platform window handle, not a CNA handle: CNA stores it verbatim and nothing dereferences
    // it. It round-trips as a bigint so a high address cannot be rounded on the way through.
    const handle = 0x1234_5678_9abc_def0n;
    gs.GamerServicesDispatcher.WindowHandle = handle;
    this.evidence.windowHandle = gs.GamerServicesDispatcher.WindowHandle;
    this.evidence.windowHandleIsBigInt = typeof this.evidence.windowHandle === "bigint";

    // Guide state now lives in CNA, so a native gamer-services component and this class cannot
    // disagree. Each value is written and read back through the runtime rather than a local field.
    this.evidence.guideVisible = gs.Guide.IsVisible;
    this.evidence.trialBefore = gs.Guide.IsTrialMode;
    gs.Guide.SimulateTrialMode = true;
    this.evidence.simulateAfterSet = gs.Guide.SimulateTrialMode;
    this.evidence.trialAfterSimulating = gs.Guide.IsTrialMode;
    gs.Guide.SimulateTrialMode = false;
    this.evidence.trialAfterClearing = gs.Guide.IsTrialMode;

    // The screen saver is a *platform display* property in CNA, not title state: with no platform
    // displays the getter answers true and the setter does nothing. Recording that is the point --
    // a projection that cached the write locally would report a screen saver it had not disabled.
    // CNA describes its fallback adapter as "Default Display" exactly when the platform
    // enumerated no display, so the adapter says which of the two cases this host is.
    this.evidence.platformDisplays = Graphics.GraphicsAdapter.DefaultAdapter.Description !== "Default Display";
    gs.Guide.IsScreenSaverEnabled = false;
    this.evidence.screenSaverAfterDisable = gs.Guide.IsScreenSaverEnabled;
    gs.Guide.IsScreenSaverEnabled = true;
    this.evidence.screenSaverAfterEnable = gs.Guide.IsScreenSaverEnabled;

    gs.Guide.NotificationPosition = gs.NotificationPosition.TopLeft;
    this.evidence.notificationPosition = gs.Guide.NotificationPosition;
    gs.Guide.NotificationPosition = gs.NotificationPosition.BottomCenter;
    this.evidence.notificationPositionRestored = gs.Guide.NotificationPosition;

    // Everything that needs a real signed-in user still refuses. A fabricated gamer would be worse
    // than the exception XNA itself raises where the platform is absent.
    // Nobody is signed in, and nobody is invented: the collection is empty rather than holding a
    // placeholder gamer, which is what "do not fabricate a signed-in user" looks like in practice.
    this.evidence.signedInGamerCount = gs.Gamer.SignedInGamers.Count;
    // What still needs a platform this host does not have. `BeginShowMessageBox` used to be in
    // this list and is not any more: CNA draws that screen itself, so it works -- see the Guide
    // test below. It is started and *answered* here rather than merely started, because a Guide
    // screen left pending would refuse the next one.
    for (const [name, call] of [
      ["ShowSignIn", () => gs.Guide.ShowSignIn(1, false)],
      ["DelayNotifications", () => gs.Guide.DelayNotifications(TimeSpan.Zero)],
    ]) {
      try {
        call();
        this.evidence[`${name}Refusal`] = "allowed";
      } catch (error) {
        this.evidence[`${name}Refusal`] = error.constructor.name;
      }
    }
    try {
      const box = gs.Guide.BeginShowMessageBox(
        "t", "m", ["a"], 0, gs.MessageBoxIcon.None, null, null,
      );
      guideExtensions.CnaGuide.ForTests.ClickMessageBoxButton(0);
      this.evidence.MessageBoxAnswer = gs.Guide.EndShowMessageBox(box);
    } catch (error) {
      this.evidence.MessageBoxAnswer = error.constructor.name;
    }
    this.Exit();
    super.LoadContent();
  }
}

test("gamer services has a real dispatcher and Guide state, and still refuses a fabricated gamer", async () => {
  const game = new GamerServicesProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  assert.equal(evidence.initializedBefore, false);
  // XNA raises InvalidOperationException for a pump before Initialize; CNA accepts one, so the
  // projection is what holds the line. Reading CNA's own IsInitialized rather than a mirrored flag
  // keeps the guard honest: it refuses because the runtime says it is not initialised.
  assert.equal(evidence.updateBeforeInitialize, "InvalidOperationException");
  assert.equal(evidence.initializedAfter, true);
  assert.equal(evidence.updateAfterInitialize, "SUCCESS");

  assert.equal(evidence.windowHandleIsBigInt, true);
  assert.equal(evidence.windowHandle, 0x1234_5678_9abc_def0n, "a 64-bit handle survives the round trip");

  assert.equal(evidence.guideVisible, false, "no guide screen is in front of anyone here");
  assert.equal(evidence.trialBefore, false);
  assert.equal(evidence.simulateAfterSet, true);
  // CNA keeps IsTrialMode and SimulateTrialMode apart; XNA's IsTrialMode is the disjunction,
  // because simulating a trial exists precisely so a full title reports one. The projection
  // combines them, and this is the assertion that would notice if it stopped.
  assert.equal(evidence.trialAfterSimulating, true, "simulating a trial changes what a game branches on");
  assert.equal(evidence.trialAfterClearing, false);
  // With no platform displays CNA's screen-saver flag is read-only in effect: the getter answers
  // true and the setter is a no-op. With a display the write takes. Either way the projection
  // reports what the platform says rather than what it was told, which is the whole reason this
  // state moved into CNA.
  assert.equal(evidence.screenSaverAfterDisable, !evidence.platformDisplays,
    evidence.platformDisplays ? "a platform display: the write takes" : "no platform displays: the write cannot take");
  assert.equal(evidence.screenSaverAfterEnable, true);
  assert.equal(evidence.notificationPosition, GamerServices.NotificationPosition.TopLeft);
  assert.equal(evidence.notificationPositionRestored, GamerServices.NotificationPosition.BottomCenter);

  assert.equal(evidence.signedInGamerCount, 0, "no gamer is fabricated where none is signed in");
  assert.equal(evidence.ShowSignInRefusal, "GamerServicesNotAvailableException");
  assert.equal(evidence.DelayNotificationsRefusal, "GamerServicesNotAvailableException");
  // BeginShowMessageBox is no longer a refusal: CNA draws that screen itself, so the XNA API
  // works and answers with the button that was pressed. What still refuses is what needs a real
  // platform -- a sign-in screen and a notification delay.
  assert.equal(evidence.MessageBoxAnswer, 0, "the Guide's message box works and answers");
});

class SensorProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    this.evidence.support = { ...sensorsModule.CnaSensors.GetSupport() };

    // Constructing one on a device that has none is allowed, so a game can build its options
    // screen without branching first. What is *not* allowed is getting a reading out of it.
    const accelerometer = new sensorsModule.Accelerometer();
    try {
      this.evidence.state = accelerometer.State;
      this.evidence.isDataValid = accelerometer.IsDataValid;

      // The whole point of this family: a device with no accelerometer must not read as one lying
      // perfectly still. Zeroes here would be a measurement a game could integrate.
      try {
        accelerometer.CurrentValue;
        this.evidence.currentValue = "returned a reading";
      } catch (error) {
        this.evidence.currentValue = error.constructor.name;
      }

      try {
        accelerometer.Start();
        this.evidence.start = "SUCCESS";
      } catch (error) {
        this.evidence.start = `result ${error.cnaResult}`;
      }
      this.evidence.stateAfterStart = accelerometer.State;

      // The update interval is a request, and reading it back is what says whether it took.
      const before = accelerometer.TimeBetweenUpdates.Ticks;
      accelerometer.TimeBetweenUpdates = TimeSpan.FromMilliseconds(50);
      this.evidence.interval = {
        before: String(before),
        after: String(accelerometer.TimeBetweenUpdates.Ticks),
        requested: String(TimeSpan.FromMilliseconds(50).Ticks),
      };

      try {
        accelerometer.Stop();
        this.evidence.stop = "SUCCESS";
      } catch (error) {
        this.evidence.stop = `result ${error.cnaResult}`;
      }
    } finally {
      accelerometer.Dispose();
      accelerometer.Dispose();
      this.evidence.disposedTwice = accelerometer.IsDisposed;
    }
    try {
      new sensorsModule.Accelerometer().Dispose();
      this.evidence.reconstructed = true;
    } catch {
      this.evidence.reconstructed = false;
    }
    this.Exit();
    super.LoadContent();
  }
}

test("a sensor that is not there reports absence rather than a zero reading", async () => {
  const game = new SensorProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // Four families, each answered by the platform rather than assumed.
  for (const [name, supported] of Object.entries(evidence.support)) {
    assert.equal(typeof supported, "boolean", `${name} must answer with a boolean`);
  }

  assert.ok(
    Object.values(sensorsModule.SensorState).includes(evidence.state),
    `unexpected sensor state ${evidence.state}`,
  );
  if (!evidence.support.Accelerometer) {
    // This host has no accelerometer, and the whole family says so consistently: the state is
    // NotSupported, no data is valid, and asking for a value refuses instead of returning zeroes a
    // game would integrate into an orientation nothing ever measured.
    assert.equal(evidence.state, sensorsModule.SensorState.NotSupported);
    assert.equal(evidence.isDataValid, false);
    assert.equal(evidence.currentValue, "InvalidOperationException");
  }
  assert.equal(typeof evidence.isDataValid, "boolean");

  // Start and Stop either work or name the result they refused with; neither is assumed.
  for (const value of [evidence.start, evidence.stop]) {
    assert.ok(value === "SUCCESS" || /^result \d+$/.test(value), `unexpected sensor evidence ${value}`);
  }

  // The interval is a request. Whether the platform took it is read back rather than assumed.
  assert.match(evidence.interval.before, /^\d+$/);
  assert.match(evidence.interval.after, /^\d+$/);
  assert.equal(evidence.disposedTwice, true);
  assert.equal(evidence.reconstructed, true, "a released sensor does not block the next one");
});

class NativeAudioProbeGame extends Game {
  constructor(mediaUri) {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.bufferNeeded = 0;
    this.mediaUri = mediaUri;
    this.storagePromise = null;
  }

  LoadContent() {
    this.storagePromise = selectStorage();
    const backend = getBackend().Audio;
    assert.ok(backend);
    // The host's capture devices, whatever they are: none on a NULL-audio build, the machine's
    // real ones on an SDL3 one. What must hold either way is that Default is one of All -- XNA has
    // no synthetic "default device" entry -- and absent exactly when All is empty.
    const microphones = Audio.Microphone.All;
    assert.ok(Array.isArray(microphones));
    for (const microphone of microphones) assert.equal(typeof microphone.Name, "string");
    if (microphones.length === 0) assert.equal(Audio.Microphone.Default, undefined);
    else assert.ok(microphones.includes(Audio.Microphone.Default), "Default is one of All");

    Audio.SoundEffect.MasterVolume = 0.75;
    Audio.SoundEffect.DistanceScale = 2;
    Audio.SoundEffect.DopplerScale = 0.5;
    Audio.SoundEffect.SpeedOfSound = 340;
    assert.equal(backend.getMasterVolume(), Math.fround(0.75));
    assert.equal(backend.getDistanceScale(), Math.fround(2));
    assert.equal(backend.getDopplerScale(), Math.fround(0.5));
    assert.equal(backend.getSpeedOfSound(), Math.fround(340));
    assert.throws(
      () => new Audio.AudioEngine("/tmp/cna-ts-missing-settings.xgs"),
      (error) => error.operation === "cna_audio_engine_create" && Number.isInteger(error.cnaResult),
    );

    this.sound = new Audio.SoundEffect(Array(320).fill(0), 8000, Audio.AudioChannels.Mono);
    assert.equal(this.sound.Name, "");
    this.sound.Name = "native-audio-probe";
    assert.equal(this.sound.Name, "native-audio-probe");
    // Whether a fire-and-forget voice starts is the audio platform's answer: false where the
    // artifact has no mixer (NULL audio), true where it has one. Everything below that depends on
    // playback follows the same answer rather than assuming either.
    const mixing = this.sound.Play(0.5, 0.25, -0.25);
    assert.equal(typeof mixing, "boolean");
    this.mixing = mixing;

    const disposableChild = this.sound.CreateInstance();
    disposableChild.Dispose();
    disposableChild.Dispose();
    assert.equal(disposableChild.IsDisposed, true);

    this.instance = this.sound.CreateInstance();
    assert.equal(this.instance.State, Audio.SoundState.Stopped);
    this.instance.Pause();
    assert.equal(this.instance.State, Audio.SoundState.Stopped);
    this.instance.Volume = 0.5;
    this.instance.Pitch = -0.25;
    this.instance.Pan = 0.25;
    this.instance.IsLooped = true;
    this.instance.Apply3D(new Audio.AudioListener(), new Audio.AudioEmitter());
    // ABI 0.9.0 changed this contract: Apply3D previously refused every listener count but one and
    // now accepts any positive count, the nearest listener deciding the applied attenuation.
    this.instance.Apply3D(
      [new Audio.AudioListener(), new Audio.AudioListener()], new Audio.AudioEmitter(),
    );
    // The other half of that contract change: a playing instance that was never positioned refuses,
    // because starting playback fixes the choice between 3D and pan.
    const unpositioned = this.sound.CreateInstance();
    unpositioned.Play();
    assert.throws(
      () => unpositioned.Apply3D(new Audio.AudioListener(), new Audio.AudioEmitter()),
      (error) => error.cnaResult === 3,
    );
    unpositioned.Dispose();
    this.instance.Play();
    assert.equal(
      this.instance.State, mixing ? Audio.SoundState.Playing : Audio.SoundState.Stopped,
      "an instance plays exactly when the platform has a mixer",
    );
    this.instance.Resume();
    this.instance.Stop(false);

    this.dynamic = new Audio.DynamicSoundEffectInstance(8000, Audio.AudioChannels.Mono);
    this.dynamic.SubmitBuffer(Array(320).fill(0));
    assert.equal(this.dynamic.PendingBufferCount, 1);
    const onNeeded = () => {
      this.bufferNeeded += 1;
      this.dynamic.BufferNeeded.Remove(onNeeded);
      this.dynamic.SubmitBuffer(Array(320).fill(0));
    };
    this.dynamic.BufferNeeded.Add(onNeeded);
    this.dynamic.Play();

    const sources = Media.MediaSource.GetAvailableMediaSources();
    assert.ok(sources.length >= 1);
    assert.equal(typeof sources[0].Name, "string");
    assert.equal(typeof sources[0].MediaSourceType, "number");
    assert.equal(typeof Media.MediaPlayer.GameHasControl, "boolean");
    Media.MediaPlayer.Volume = 0.5;
    Media.MediaPlayer.IsMuted = true;
    Media.MediaPlayer.IsRepeating = true;
    Media.MediaPlayer.IsShuffled = false;
    Media.MediaPlayer.IsVisualizationEnabled = true;
    this.song = Media.Song.FromUri("generated-silence", this.mediaUri);
    Media.MediaPlayer.Play(this.song);
    assert.equal(Media.MediaPlayer.State, Media.MediaState.Playing);
    Media.MediaPlayer.Pause();
    Media.MediaPlayer.Resume();
    Media.MediaPlayer.MoveNext();
    Media.MediaPlayer.MovePrevious();
    assert.equal(typeof Media.MediaPlayer.PlayPosition.Ticks, "bigint");
    const visualization = new Media.VisualizationData();
    Media.MediaPlayer.GetVisualizationData(visualization);
    assert.equal(visualization.Frequencies.length, 256);
    Media.MediaPlayer.Stop();

    this.videoPlayer = new Media.VideoPlayer();
    assert.equal(this.videoPlayer.State, Media.MediaState.Stopped);
    assert.equal(this.videoPlayer.PlayPosition.Ticks, 0n);
    this.videoPlayer.IsLooped = true;
    this.videoPlayer.IsMuted = true;
    this.videoPlayer.Volume = 0.25;
    this.videoPlayer.Pause();
    this.videoPlayer.Resume();
    this.videoPlayer.Stop();
    assert.throws(() => this.videoPlayer.GetTexture(), /No video has been played/);

    // The frame identity CNA added so a borrowed frame texture could be projected at all. Before
    // any decode it is an absence with a zero generation -- a state, not a failure -- and reading
    // it must not fabricate a texture or advance the count.
    const before = extensionsModule.GetVideoFrameIdentity(this.videoPlayer);
    assert.equal(before.IsAvailable, false);
    assert.equal(before.Generation, 0n);
    assert.equal(typeof before.PresentationTimeSeconds, "number");
    assert.ok(before.PresentationTimeSeconds < 0, "no frame has no presentation time");
    const again = extensionsModule.GetVideoFrameIdentity(this.videoPlayer);
    assert.equal(again.Generation, before.Generation, "asking does not advance the frame");

    // Control-path evidence for the projection itself: it now distinguishes "nothing has played"
    // from "playing but nothing decoded yet", and neither invents a Texture2D. Actual decode
    // progression is fixture-pending -- no redistributable video is available on this host.
    this.videoPlayerFrameEvidence = {
      available: before.IsAvailable,
      generation: before.Generation,
      presentationTime: before.PresentationTimeSeconds,
    };
  }

  Update() {
    assert.ok(this.bufferNeeded <= 1);
    this.Exit();
  }
}

test("executes typed CNA Audio/XACT, Media/Video and Storage routes", async () => {
  const directory = fs.mkdtempSync(path.join(os.tmpdir(), "cna-ts-media-native-"));
  const filename = path.join(directory, "silence.wav");
  fs.writeFileSync(filename, silentWave());
  try {
    const game = new NativeAudioProbeGame(pathToFileURL(filename));
    await game.Run();
    const storageDevice = await game.storagePromise;
    assert.equal(storageDevice.IsConnected, true);
    assert.ok(storageDevice.FreeSpace > 0n);
    assert.ok(storageDevice.TotalSpace > 0n);
    const container = await openStorage(storageDevice, "native-tests");
    assert.equal(await openStorage(storageDevice, "native-tests"), container);
    assert.equal(container.DisplayName, "native-tests");
    container.CreateDirectory("saves");
    assert.equal(container.DirectoryExists("saves"), true);
    assert.deepEqual(container.GetDirectoryNames("sav*"), ["saves"]);
    container.CreateFile("slot.dat");
    assert.equal(container.FileExists("slot.dat"), true);
    assert.deepEqual(container.GetFileNames("*.dat"), ["slot.dat"]);
    assert.equal(container.OpenFile("slot.dat", 3).byteLength, 0);
    container.DeleteFile("slot.dat");
    container.DeleteDirectory("saves");
    container.Dispose();
    container.Dispose();
    storageDevice.DeleteContainer("native-tests");
    const liveContainer = await openStorage(storageDevice, "parent-owned");
    assert.equal(game.bufferNeeded, 1);
    game.Dispose();
    assert.equal(game.instance.IsDisposed, true);
    assert.equal(game.dynamic.IsDisposed, true);
    assert.equal(game.sound.IsDisposed, true);
    assert.equal(game.videoPlayer.IsDisposed, true);
    assert.equal(liveContainer.IsDisposed, true);
    assert.equal(storageDevice.IsConnected, false);
    game.instance.Dispose();
    game.dynamic.Dispose();
    game.sound.Dispose();
    game.videoPlayer.Dispose();
    game.song.Dispose();
    game.Dispose();
  } finally {
    fs.rmSync(directory, { recursive: true, force: true });
  }
});

function selectStorage() {
  return new Promise((resolve, reject) => {
    Storage.StorageDevice.BeginShowSelector((result) => {
      try { resolve(Storage.StorageDevice.EndShowSelector(result)); } catch (error) { reject(error); }
    }, null);
  });
}

function openStorage(device, name) {
  return new Promise((resolve, reject) => {
    device.BeginOpenContainer(name, (result) => {
      try { resolve(device.EndOpenContainer(result)); } catch (error) { reject(error); }
    }, null);
  });
}

function silentWave() {
  const sampleCount = 800;
  const output = Buffer.alloc(44 + sampleCount * 2);
  output.write("RIFF", 0, "ascii");
  output.writeUInt32LE(output.length - 8, 4);
  output.write("WAVEfmt ", 8, "ascii");
  output.writeUInt32LE(16, 16);
  output.writeUInt16LE(1, 20);
  output.writeUInt16LE(1, 22);
  output.writeUInt32LE(8000, 24);
  output.writeUInt32LE(16000, 28);
  output.writeUInt16LE(2, 32);
  output.writeUInt16LE(16, 34);
  output.write("data", 36, "ascii");
  output.writeUInt32LE(sampleCount * 2, 40);
  return output;
}

for (const frameCount of [60, 600]) {
  test(`executes ${frameCount} CNA-owned frames with graphics resources`, async () => {
    const game = new NativeProbeGame(frameCount);
    await game.Run();
    // Disposed on every path: a game left alive by a failed assertion refuses every later
    // cna_game_create in this process, which turns one failure into all of them.
    try {
      // XNA's fixed time step runs extra Updates before a Draw whenever a frame took longer than
      // the step, so on a loaded host there can be more Updates than Draws -- never fewer.
      assert.equal(game.draws, frameCount);
      assert.ok(game.updates >= frameCount, `${game.updates} updates for ${frameCount} draws`);
      assert.equal(game.inputPolls, 1);
      assert.equal(game.graphicsRouteEvidence["effect SpriteBatch.Begin"], "SUCCESS");
      assert.equal(game.graphicsRouteEvidence["stock effect construction"], "SUCCESS");
      assert.equal(game.graphicsRouteEvidence["stock effect execution"], "SUCCESS");
      assert.equal(game.graphicsRouteEvidence["compiled Effect route"], "HEADLESS_NOT_SUPPORTED");
      assert.equal(game.graphicsRouteEvidence["Model.Draw"], "SUCCESS");
      assert.equal(game.graphicsRouteEvidence["RenderTarget2D creation"], "SUCCESS");
      assert.equal(game.graphicsRouteEvidence["RenderTargetCube creation"], "SUCCESS");
      assert.equal(game.graphicsRouteEvidence["cube render target binding"], "SUCCESS");
      assert.ok(game.graphicsRouteEvidence["DrawUserPrimitives"]);
      assert.ok(game.graphicsRouteEvidence["DrawUserIndexedPrimitives"]);
      game.Dispose();
    } finally {
      game.Dispose();
    }
  });
}

test("parent shutdown deterministically releases live graphics families", async () => {
  const game = new NativeProbeGame(2, true);
  await game.Run();
  game.Dispose();
  assert.equal(game.texture.IsDisposed, true);
  assert.equal(game.dynamicVertexBuffer?.IsDisposed, true);
  assert.equal(game.dynamicIndexBuffer?.IsDisposed, true);
  assert.equal(game.renderTarget?.IsDisposed, true);
  assert.equal(game.renderTargetCube?.IsDisposed, true);
  // An occlusion query is refused where the renderer or the device's profile has none (XNA's Reach
  // has none), so it is asserted released only where it was made at all.
  if (game.occlusionQuery == null) {
    assert.equal(game.graphicsRouteEvidence["OcclusionQuery lifecycle"], "HEADLESS_NOT_SUPPORTED");
  } else {
    assert.equal(game.occlusionQuery.IsDisposed, true);
  }
  game.texture.Dispose();
  game.texture.Dispose();
  game.dynamicVertexBuffer?.Dispose();
  game.dynamicIndexBuffer?.Dispose();
  game.renderTarget?.Dispose();
  game.renderTargetCube?.Dispose();
  game.occlusionQuery?.Dispose();
  game.Dispose();
});

test("repeats native Game creation and destruction", async () => {
  for (let iteration = 0; iteration < 3; iteration += 1) {
    const game = new NativeProbeGame(3);
    await game.Run();
    game.Dispose();
  }
});

test("reports renderer identity from CNA rather than a binding label", () => {
  const renderer = GetRuntimeStatus().RendererInfo;
  assert.equal(renderer.Name, "HEADLESS");
  assert.equal(typeof renderer.RendererType, "number");
  assert.equal(typeof renderer.CapabilityFlags, "bigint");
  assert.ok(renderer.MaxTextureDimension > 0);
  assert.equal(renderer.CapabilityFlags & (1n << 7n), 1n << 7n, "HEADLESS custom effects");
  assert.equal(renderer.CapabilityFlags & (1n << 13n), 0n, "HEADLESS compiled effects");
});

class ExtendedInputProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    const { Haptics, Joysticks } = inputModule;

    // Enumeration first, because everything else takes an identifier it produces. A host with no
    // joystick must report none rather than throwing: "there are none" is an answer a game acts on.
    this.evidence.joystickCount = Joysticks.Count;
    const devices = Joysticks.Enumerate();
    this.evidence.enumerated = devices.length;
    this.evidence.enumerationIsFrozen = Object.isFrozen(devices);
    this.evidence.joysticks = devices.map((device) => ({
      id: device.Id, name: device.Name, type: device.Type,
    }));

    // An identifier no device has. Measured rather than assumed: CNA does not refuse it, it
    // answers *absent* -- a disconnected descriptor with zero counts and an empty name, which is
    // exactly the convention XNA's own GamePad.GetCapabilities follows for a missing controller.
    // That is the contract worth pinning, because a binding that invented a plausible descriptor
    // for an absent device would be the defect this family exists to avoid.
    const unknown = this.evidence.joystickCount + 1000;
    this.evidence.unknownCapabilities = { ...Joysticks.GetCapabilities(unknown) };
    const unknownState = Joysticks.CaptureState(unknown);
    this.evidence.unknownState = {
      axes: unknownState.Axes.length,
      buttons: unknownState.Buttons.length,
      hats: unknownState.Hats.length,
      balls: unknownState.Balls.length,
    };

    // Where a joystick does exist, its capabilities and a captured state must agree with each other.
    if (devices.length > 0) {
      const id = devices[0].Id;
      const capabilities = Joysticks.GetCapabilities(id);
      const state = Joysticks.CaptureState(id);
      this.evidence.first = {
        capabilities: { ...capabilities },
        axes: state.Axes.length,
        buttons: state.Buttons.length,
        hats: state.Hats.length,
        balls: state.Balls.length,
        axisRangeOk: state.Axes.every((value) => Number.isInteger(value) && value >= -32768 && value <= 32767),
        buttonsAreBooleans: state.Buttons.every((value) => typeof value === "boolean"),
        ballsArePoints: state.Balls.every((value) => value instanceof Point),
      };
    }

    this.evidence.hapticCount = Haptics.Count;
    const haptics = Haptics.Enumerate();
    this.evidence.hapticsEnumerated = haptics.length;
    // The same rule for haptics, and this is the stronger half of it: opening an identifier no
    // device has produces an object that reports itself closed and **declines every operation**.
    // A device that silently accepted PlayRumble while doing nothing would be worse than an
    // exception, because a game would offer the setting.
    const absent = Haptics.Open(this.evidence.hapticCount + 1000);
    try {
      this.evidence.absentHaptic = {
        isOpen: absent.IsOpen,
        name: absent.Name,
        capabilities: { ...absent.Capabilities },
        initRumble: absent.InitializeRumble(),
        playRumble: absent.PlayRumble(0.5, 100),
        stopRumble: absent.StopRumble(),
        setGain: absent.SetGain(50),
      };
    } finally {
      absent.Dispose();
    }
    this.evidence.absentHapticDisposed = absent.IsDisposed;
    try {
      absent.PlayRumble(0.5, 100);
      this.evidence.afterDispose = "accepted";
    } catch (error) {
      this.evidence.afterDispose = error.constructor.name;
    }
    if (haptics.length > 0) {
      const device = Haptics.Open(haptics[0].Id);
      try {
        this.evidence.haptic = {
          name: device.Name,
          isOpen: device.IsOpen,
          capabilities: { ...device.Capabilities },
          initRumble: device.InitializeRumble(),
          playRumble: device.PlayRumble(0.5, 100),
          stopRumble: device.StopRumble(),
        };
      } finally {
        device.Dispose();
      }
      this.evidence.hapticDisposedTwice = (() => {
        const second = Haptics.Open(haptics[0].Id);
        second.Dispose();
        second.Dispose();
        return second.IsDisposed;
      })();
    }

    // Argument validation happens in TypeScript, before anything reaches CNA, so it is the same
    // refusal whether or not a device is attached.
    const refusals = {};
    const record = (name, body) => {
      try {
        body();
        refusals[name] = "accepted";
      } catch (error) {
        refusals[name] = error.constructor.name;
      }
    };
    record("negativeJoystickId", () => Joysticks.GetCapabilities(-1));
    record("fractionalJoystickId", () => Joysticks.CaptureState(1.5));
    record("negativeHapticId", () => Haptics.Open(-1));
    this.evidence.refusals = refusals;
    super.LoadContent();
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.Exit();
    super.Draw(gameTime);
  }
}

test("CNA's raw joysticks and haptics answer, and report absence as absence", async () => {
  const game = new ExtendedInputProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // Whatever this host has, the two counts and the two enumerations must agree with each other.
  assert.equal(typeof evidence.joystickCount, "number");
  assert.equal(evidence.enumerated, evidence.joystickCount, "the enumeration is the count");
  assert.equal(evidence.enumerationIsFrozen, true, "an enumeration is a snapshot, not a live list");
  for (const joystick of evidence.joysticks) {
    assert.equal(typeof joystick.id, "number");
    assert.equal(typeof joystick.name, "string");
    assert.ok(
      Object.values(inputModule.JoystickType).includes(joystick.type),
      `unexpected joystick type ${joystick.type}`,
    );
  }

  // An identifier a thousand past the end. CNA answers absence rather than refusing, and the whole
  // descriptor says so together: not connected, no axes, no buttons, no hats, no balls, no name and
  // no GUID. A binding that filled in a plausible-looking device here would fail every line.
  assert.equal(evidence.unknownCapabilities.IsConnected, false);
  assert.deepEqual(
    [
      evidence.unknownCapabilities.AxisCount, evidence.unknownCapabilities.ButtonCount,
      evidence.unknownCapabilities.HatCount, evidence.unknownCapabilities.BallCount,
    ],
    [0, 0, 0, 0],
  );
  assert.equal(evidence.unknownCapabilities.Name, "");
  assert.equal(evidence.unknownCapabilities.Guid, "");
  assert.equal(evidence.unknownCapabilities.Type, inputModule.JoystickType.Unknown);
  assert.deepEqual(evidence.unknownState, { axes: 0, buttons: 0, hats: 0, balls: 0 });

  // The haptic half, which is the stronger claim: an absent device reports itself closed, offers
  // no features, and **declines every operation** rather than accepting one it cannot perform.
  const absent = evidence.absentHaptic;
  assert.equal(absent.isOpen, false);
  assert.equal(absent.name, "");
  assert.equal(absent.capabilities.Features, 0, "no effect is supported by a device that is not there");
  assert.equal(absent.capabilities.RumbleSupported, false);
  assert.equal(absent.capabilities.IsOpen, false);
  for (const key of ["initRumble", "playRumble", "stopRumble", "setGain"]) {
    assert.equal(absent[key], false, `${key} must be declined by an absent device`);
  }
  assert.equal(evidence.absentHapticDisposed, true);
  assert.equal(evidence.afterDispose, "ObjectDisposedException", "a released device refuses by name");

  // Where a device exists, the capability counts and the captured arrays must be the same numbers:
  // a state whose arrays disagreed with the capabilities would mean one of them was invented.
  if (evidence.first) {
    const { capabilities, axes, buttons, hats, balls } = evidence.first;
    assert.equal(axes, capabilities.AxisCount);
    assert.equal(buttons, capabilities.ButtonCount);
    assert.equal(hats, capabilities.HatCount);
    assert.equal(balls, capabilities.BallCount);
    assert.equal(evidence.first.axisRangeOk, true, "an axis is a raw int16");
    assert.equal(evidence.first.buttonsAreBooleans, true);
    assert.equal(evidence.first.ballsArePoints, true, "a trackball's motion is an XNA Point");
    assert.equal(typeof capabilities.Guid, "string");
  } else {
    // No joystick is attached to this host, which is the ordinary case for a headless build
    // machine. That is recorded rather than skipped: the family answered, and its answer was none.
    assert.equal(evidence.joystickCount, 0);
  }

  assert.equal(typeof evidence.hapticCount, "number");
  assert.equal(evidence.hapticsEnumerated, evidence.hapticCount);
  if (evidence.haptic) {
    assert.equal(evidence.haptic.isOpen, true);
    assert.equal(typeof evidence.haptic.name, "string");
    assert.equal(typeof evidence.haptic.capabilities.Features, "number");
    // Every haptic operation reports whether the device accepted it, which is what a game branches
    // on. None of them is assumed to have worked.
    for (const key of ["initRumble", "playRumble", "stopRumble"]) {
      assert.equal(typeof evidence.haptic[key], "boolean", `${key} must report acceptance`);
    }
    assert.equal(evidence.hapticDisposedTwice, true, "disposing twice is idempotent");
  } else {
    assert.equal(evidence.hapticCount, 0);
  }

  // Argument validation is this package's, not CNA's, so it is the same answer on any host.
  assert.equal(evidence.refusals.negativeJoystickId, "ArgumentException");
  assert.equal(evidence.refusals.fractionalJoystickId, "ArgumentException");
  assert.equal(evidence.refusals.negativeHapticId, "ArgumentException");
});

test("the extended input families refuse outside a game rather than answering", () => {
  // Every one of these is a property of a platform a game opened. Asking without one must name the
  // problem rather than returning an empty list a caller would read as "no devices".
  for (const [name, body] of Object.entries({
    JoystickCount: () => inputModule.Joysticks.Count,
    JoystickEnumerate: () => inputModule.Joysticks.Enumerate(),
    HapticCount: () => inputModule.Haptics.Count,
    HapticEnumerate: () => inputModule.Haptics.Enumerate(),
  })) {
    assert.throws(body, /requires an active native Game|active native Game/, `${name} answered outside a game`);
  }
});

class TextInputProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    const { CnaTextInput, MouseCursor, MouseCursorStock, TextInputType } = inputModule;
    // CNA publishes this so one test cannot leak registrations into the next.
    CnaTextInput.ForTests.Reset();

    this.evidence.activeBefore = CnaTextInput.IsActive;
    const record = (name, body) => {
      try {
        body();
        this.evidence[name] = "SUCCESS";
      } catch (error) {
        this.evidence[name] = `result ${error.cnaResult ?? error.constructor.name}`;
      }
    };
    record("start", () => CnaTextInput.Start());
    this.evidence.activeAfterStart = CnaTextInput.IsActive;
    record("stop", () => CnaTextInput.Stop());
    this.evidence.activeAfterStop = CnaTextInput.IsActive;
    record("typedStart", () => CnaTextInput.Start(TextInputType.Email));
    this.evidence.activeAfterTypedStart = CnaTextInput.IsActive;
    this.evidence.screenKeyboard = CnaTextInput.IsScreenKeyboardShown;
    record("rectangle", () => CnaTextInput.SetInputRectangle(new Rectangle(4, 8, 120, 24)));

    // Committed characters. Deliberately not ASCII: a Latin letter with a diacritic, a CJK
    // ideograph, and a non-BMP emoji delivered as the surrogate pair the platform actually sends.
    // A binding that treated a code unit as a byte, or that reassembled the pair itself, fails.
    const typed = [];
    const characters = CnaTextInput.OnCharacter((character) => typed.push(character));
    try {
      for (const unit of [...("aé漢字")].flatMap((c) => [...c].map((u) => u.charCodeAt(0)))) {
        CnaTextInput.ForTests.RaiseCharacter(unit);
      }
      // U+1F600 GRINNING FACE, as its high and low surrogates.
      CnaTextInput.ForTests.RaiseCharacter(0xd83d);
      CnaTextInput.ForTests.RaiseCharacter(0xde00);
    } finally {
      characters.Dispose();
    }
    this.evidence.typedUnits = [...typed];
    this.evidence.typedJoined = typed.join("");
    this.evidence.charactersDisposed = characters.IsDisposed;

    // A disposed subscription must stop receiving. Raising again after disposal and finding the
    // list unchanged is what proves the unsubscribe reached CNA rather than only this package.
    const before = typed.length;
    CnaTextInput.ForTests.RaiseCharacter(0x0041);
    this.evidence.afterUnsubscribe = typed.length - before;

    // Composition. The text is UTF-8 on CNA's side and its selection is two independent numbers.
    const editing = [];
    const editingSubscription = CnaTextInput.OnEditing((value) => editing.push(value));
    try {
      CnaTextInput.ForTests.RaiseEditing("にほんご", 2, 1);
      CnaTextInput.ForTests.RaiseEditing("", 0, 0);
    } finally {
      editingSubscription.Dispose();
    }
    this.evidence.editing = editing.map((value) => ({
      text: value.Text, start: value.Start, length: value.Length,
    }));

    // Candidate lists, which is the part of an IME a game has to draw itself.
    const candidates = [];
    const candidateSubscription = CnaTextInput.OnCandidates((value) => candidates.push(value));
    try {
      CnaTextInput.ForTests.RaiseCandidates(["日本語", "にほんご", "ニホンゴ"], 1, true);
      CnaTextInput.ForTests.RaiseCandidates([], -1, false);
    } finally {
      candidateSubscription.Dispose();
    }
    this.evidence.candidates = candidates.map((value) => ({
      list: [...value.Candidates], selected: value.Selected, horizontal: value.IsHorizontal,
    }));

    // An exception out of a handler must not unwind into compiled C. The raise must return and the
    // next one must still be delivered.
    const survived = [];
    const throwing = CnaTextInput.OnCharacter(() => { throw new Error("handler failure"); });
    const following = CnaTextInput.OnCharacter((character) => survived.push(character));
    try {
      CnaTextInput.ForTests.RaiseCharacter(0x0042);
      this.evidence.raiseAfterThrow = "returned";
    } catch (error) {
      this.evidence.raiseAfterThrow = error.constructor.name;
    } finally {
      throwing.Dispose();
      following.Dispose();
    }
    this.evidence.survivedHandler = [...survived];

    CnaTextInput.Stop();
    CnaTextInput.ForTests.Reset();

    // The cursor family. A stock cursor is an owned handle; applying it and disposing it twice are
    // both real operations on this platform even where no window shows one.
    const cursor = MouseCursor.GetStock(MouseCursorStock.Hand);
    try {
      cursor.Apply();
      this.evidence.cursorApplied = "SUCCESS";
    } catch (error) {
      this.evidence.cursorApplied = `result ${error.cnaResult ?? error.constructor.name}`;
    }
    cursor.Dispose();
    cursor.Dispose();
    this.evidence.cursorDisposed = cursor.IsDisposed;
    try {
      cursor.Apply();
      this.evidence.cursorAfterDispose = "accepted";
    } catch (error) {
      this.evidence.cursorAfterDispose = error.constructor.name;
    }

    // A cursor built from a texture copies the image, so the texture can go first.
    const texture = new Graphics.Texture2D(this.GraphicsDevice, 2, 2);
    texture.SetData([Color.Red, Color.Green, Color.Blue, Color.White]);
    try {
      const built = inputModule.MouseCursor.FromTexture2D(texture, 1, 1);
      built.Dispose();
      this.evidence.textureCursor = "SUCCESS";
    } catch (error) {
      this.evidence.textureCursor = `result ${error.cnaResult ?? error.constructor.name}`;
    } finally {
      texture.Dispose();
    }
    super.LoadContent();
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.Exit();
    super.Draw(gameTime);
  }
}

test("typed text and IME composition reach the extension, non-ASCII included", async () => {
  const game = new TextInputProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // Start, Stop, the typed start and the input rectangle all succeed. Whether text input becomes
  // *active* is a different question and this platform answers it honestly: HEADLESS has no window
  // to start input on, so IsActive stays false throughout. That is measured rather than assumed --
  // the first version of this test expected true and was wrong -- and it is reported as the
  // platform's answer rather than papered over, exactly as the GameWindow families are.
  for (const name of ["start", "stop", "typedStart", "rectangle"]) {
    assert.equal(evidence[name], "SUCCESS", `${name} was refused`);
  }
  assert.equal(evidence.activeBefore, false);
  assert.equal(evidence.activeAfterStart, false, "no window on HEADLESS: input cannot become active");
  assert.equal(evidence.activeAfterStop, false);
  assert.equal(evidence.activeAfterTypedStart, false);
  assert.equal(evidence.screenKeyboard, false, "no window, so no software keyboard");

  // Every committed character arrives as exactly one UTF-16 code unit -- which for the emoji means
  // two calls, a high surrogate and a low one. That is what the platform sends, and the caller's
  // accumulator is what rejoins them; this package does not reassemble on the caller's behalf.
  assert.deepEqual(
    evidence.typedUnits, ["a", "é", "漢", "字", "\ud83d", "\ude00"],
    "one code unit per event, surrogates delivered separately",
  );
  assert.equal(evidence.typedJoined, "aé漢字😀", "appending the units rebuilds the text exactly");
  // The Latin letter with a diacritic is one code unit and the emoji is two, which is the whole
  // reason this is asserted in units rather than in characters.
  assert.equal(evidence.typedUnits.length, 6);
  assert.equal([...evidence.typedJoined].length, 5, "five characters from six code units");

  assert.equal(evidence.charactersDisposed, true);
  assert.equal(evidence.afterUnsubscribe, 0, "a disposed subscription receives nothing further");

  // Composition text is UTF-8 across the boundary and its selection is two independent numbers.
  assert.deepEqual(evidence.editing, [
    { text: "にほんご", start: 2, length: 1 },
    { text: "", start: 0, length: 0 },
  ]);

  // Candidate lists, in order, with the selection and the orientation kept apart from them.
  assert.deepEqual(evidence.candidates, [
    { list: ["日本語", "にほんご", "ニホンゴ"], selected: 1, horizontal: true },
    { list: [], selected: -1, horizontal: false },
  ]);

  // A throwing handler must not unwind into compiled C, and must not stop the next subscriber.
  assert.equal(evidence.raiseAfterThrow, "returned", "a handler exception is contained at the boundary");
  assert.deepEqual(evidence.survivedHandler, ["B"], "a later handler still runs");

  // The cursor family. Whether this platform shows one is its business; the ownership is not.
  assert.ok(
    evidence.cursorApplied === "SUCCESS" || /^result /.test(evidence.cursorApplied),
    `unexpected cursor evidence ${evidence.cursorApplied}`,
  );
  assert.equal(evidence.cursorDisposed, true);
  assert.equal(evidence.cursorAfterDispose, "ObjectDisposedException");
  assert.ok(
    evidence.textureCursor === "SUCCESS" || /^result /.test(evidence.textureCursor),
    `unexpected texture-cursor evidence ${evidence.textureCursor}`,
  );
});

class ExtendedSensorProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    const { CnaSensorTestHooks, Compass, Gyroscope, Motion, SensorState } = sensorsModule;
    const record = (name, body) => {
      try {
        this.evidence[name] = body();
      } catch (error) {
        this.evidence[name] = `${error.constructor.name}`;
      }
    };

    // ---- absence, first. This host has none of these three, and every one must say so ----------
    const compass = new Compass();
    const gyroscope = new Gyroscope();
    const motion = new Motion();
    this.evidence.absent = {
      compassState: compass.State,
      compassValid: compass.IsDataValid,
      gyroscopeState: gyroscope.State,
      gyroscopeValid: gyroscope.IsDataValid,
      motionState: motion.State,
      motionValid: motion.IsDataValid,
      // CNA's canonical default, before any backend exists to answer for it.
      motionNorth: motion.IsAttitudeNorthReferenced,
    };
    record("compassValueWhenAbsent", () => { compass.CurrentValue; return "returned a reading"; });
    record("gyroscopeValueWhenAbsent", () => { gyroscope.CurrentValue; return "returned a reading"; });
    record("motionValueWhenAbsent", () => { motion.CurrentValue; return "returned a reading"; });

    // ---- the data path, through CNA's own injection hooks --------------------------------------
    // Every component gets a value no other component has, so a transposed or substituted field is
    // visible. This is injection evidence: nothing physical was measured.
    CnaSensorTestHooks.SetCompassBackend(compass, true, true);
    compass.Start();
    const compassTimestamp = 638_000_000_000_000_000n;
    CnaSensorTestHooks.InjectCompassReading(compass, {
      HeadingAccuracy: 2.5,
      MagneticHeading: 91.25,
      TrueHeading: 88.75,
      MagnetometerReading: new Vector3(11.5, -22.25, 33.125),
      TimestampTicks: compassTimestamp,
      TimestampOffset: TimeSpan.FromTicks(36_000_000_000n),
    });
    record("compassAfterInjection", () => {
      const reading = compass.CurrentValue;
      return {
        state: compass.State,
        valid: compass.IsDataValid,
        headingAccuracy: reading.HeadingAccuracy,
        magneticHeading: reading.MagneticHeading,
        trueHeading: reading.TrueHeading,
        magnetometer: [
          reading.MagnetometerReading.X,
          reading.MagnetometerReading.Y,
          reading.MagnetometerReading.Z,
        ],
        timestampTicks: String(reading.TimestampTicks),
        offsetTicks: String(reading.TimestampOffset.Ticks),
      };
    });

    // The gyroscope is the exception, and it is measured rather than assumed. CNA gives the
    // compass and the motion sensor a full synthetic *backend*; the gyroscope gets only a support
    // override, so starting it still needs a real platform sensor service and this host has none.
    // The injection call is accepted but produces no valid data, which is the honest outcome.
    CnaSensorTestHooks.SetGyroscopeSupported(gyroscope, true);
    this.evidence.gyroscopeStateAfterSupportOverride = gyroscope.State;
    record("gyroscopeStart", () => { gyroscope.Start(); return "SUCCESS"; });
    record("gyroscopeInject", () => {
      CnaSensorTestHooks.InjectGyroscopeReading(gyroscope, new Vector3(0.25, -0.5, 0.75));
      return "accepted";
    });
    this.evidence.gyroscopeValidAfterInjection = gyroscope.IsDataValid;
    record("gyroscopeValueAfterInjection", () => {
      gyroscope.CurrentValue;
      return "returned a reading";
    });

    // Installed *without* a north reference first, so the flag is proved to be read rather than
    // defaulted: a game drawing a compass rose branches on this, and a getter that always agreed
    // with CNA's default would pass a one-sided check.
    CnaSensorTestHooks.SetMotionBackend(motion, true, true, false);
    this.evidence.motionNorthWhenNotReferenced = motion.IsAttitudeNorthReferenced;
    CnaSensorTestHooks.SetMotionBackend(motion, true, true, true);
    this.evidence.motionNorthWhenReferenced = motion.IsAttitudeNorthReferenced;
    motion.Start();
    const motionTimestamp = 638_000_000_000_000_001n;
    CnaSensorTestHooks.InjectMotionReading(motion, {
      Attitude: {
        Pitch: 0.125, Roll: -0.25, Yaw: 1.5,
        Quaternion: new Quaternion(0.1, 0.2, 0.3, 0.4),
        // Sixteen distinguishable values, so a transposed or shifted matrix read fails.
        RotationMatrix: new Matrix(
          1, 2, 3, 4,
          5, 6, 7, 8,
          9, 10, 11, 12,
          13, 14, 15, 16,
        ),
        TimestampTicks: motionTimestamp,
        TimestampOffset: TimeSpan.FromTicks(36_000_000_000n),
      },
      DeviceAcceleration: new Vector3(1.5, 2.5, 3.5),
      DeviceRotationRate: new Vector3(-1.25, -2.25, -3.25),
      Gravity: new Vector3(0, -9.80665, 0),
      TimestampTicks: motionTimestamp,
      TimestampOffset: TimeSpan.FromTicks(36_000_000_000n),
    });
    record("motionAfterInjection", () => {
      const reading = motion.CurrentValue;
      return {
        state: motion.State,
        valid: motion.IsDataValid,
        northReferenced: motion.IsAttitudeNorthReferenced,
        pitch: reading.Attitude.Pitch,
        roll: reading.Attitude.Roll,
        yaw: reading.Attitude.Yaw,
        quaternion: [
          reading.Attitude.Quaternion.X, reading.Attitude.Quaternion.Y,
          reading.Attitude.Quaternion.Z, reading.Attitude.Quaternion.W,
        ],
        matrixDiagonal: [
          reading.Attitude.RotationMatrix.M11, reading.Attitude.RotationMatrix.M22,
          reading.Attitude.RotationMatrix.M33, reading.Attitude.RotationMatrix.M44,
        ],
        matrixFirstRow: [
          reading.Attitude.RotationMatrix.M11, reading.Attitude.RotationMatrix.M12,
          reading.Attitude.RotationMatrix.M13, reading.Attitude.RotationMatrix.M14,
        ],
        acceleration: [
          reading.DeviceAcceleration.X, reading.DeviceAcceleration.Y, reading.DeviceAcceleration.Z,
        ],
        rotationRate: [
          reading.DeviceRotationRate.X, reading.DeviceRotationRate.Y, reading.DeviceRotationRate.Z,
        ],
        gravity: [reading.Gravity.X, reading.Gravity.Y, reading.Gravity.Z],
        timestampTicks: String(reading.TimestampTicks),
      };
    });

    // The interval is a request, read back rather than assumed.
    compass.TimeBetweenUpdates = TimeSpan.FromTicks(200_000n);
    this.evidence.compassInterval = String(compass.TimeBetweenUpdates.Ticks);

    // A backend cannot be swapped underneath a running acquisition: CNA refuses, which is the
    // right answer -- a reading whose source changed mid-stream would be neither the old sensor's
    // nor the new one's.
    record("motionBackendSwapWhileStarted", () => {
      CnaSensorTestHooks.SetMotionBackend(motion, false, false, false);
      return "accepted";
    });
    // Stopped, the swap is allowed -- and removing the synthetic backend must take the readings
    // with it. A sensor that kept answering after its source was withdrawn would be reporting a
    // stale measurement as a current one.
    motion.Stop();
    this.evidence.motionStateAfterStop = motion.State;
    CnaSensorTestHooks.SetMotionBackend(motion, false, false, false);
    this.evidence.motionAfterRemoval = {
      state: motion.State,
      isDataValid: motion.IsDataValid,
    };
    record("motionValueAfterRemoval", () => { motion.CurrentValue; return "returned a reading"; });

    compass.Stop();
    for (const sensor of [compass, gyroscope, motion]) {
      sensor.Dispose();
      sensor.Dispose();
    }
    this.evidence.disposed = [compass.IsDisposed, gyroscope.IsDisposed, motion.IsDisposed];
    record("compassAfterDispose", () => { compass.State; return "answered"; });
    this.evidence.states = { NotSupported: SensorState.NotSupported, Ready: SensorState.Ready };
    super.LoadContent();
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.Exit();
    super.Draw(gameTime);
  }
}

test("the compass, gyroscope and motion sensor report absence, then carry a real reading", async () => {
  const game = new ExtendedSensorProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // Absence first. A heading of zero and a rotation rate of zero are both perfectly plausible
  // measurements, so returning them for "no sensor" would be indistinguishable from a real one.
  assert.equal(evidence.absent.compassState, evidence.states.NotSupported);
  assert.equal(evidence.absent.gyroscopeState, evidence.states.NotSupported);
  assert.equal(evidence.absent.motionState, evidence.states.NotSupported);
  assert.deepEqual(
    [evidence.absent.compassValid, evidence.absent.gyroscopeValid, evidence.absent.motionValid],
    [false, false, false],
  );
  // CNA's documented default for an attitude with no backend behind it is north-referenced; what
  // matters is that the flag follows the backend rather than staying at that default, which the
  // two assertions further down establish.
  assert.equal(evidence.absent.motionNorth, true, "CNA's default before any backend exists");
  assert.equal(evidence.compassValueWhenAbsent, "InvalidOperationException");
  assert.equal(evidence.gyroscopeValueWhenAbsent, "InvalidOperationException");
  assert.equal(evidence.motionValueWhenAbsent, "InvalidOperationException");

  // The data path, through CNA's own injection hooks. This is injection evidence: no physical
  // compass, gyroscope or motion sensor exists on this host and none was measured.
  const compass = evidence.compassAfterInjection;
  assert.equal(typeof compass, "object", `compass injection failed: ${compass}`);
  assert.equal(compass.state, evidence.states.Ready);
  assert.equal(compass.valid, true);
  // Three headings with three different values. The two headings are separate measurements --
  // magnetic and true north differ by the local declination -- so a reader that returned one for
  // both would fail here rather than round-trip.
  assert.equal(compass.headingAccuracy, 2.5);
  assert.equal(compass.magneticHeading, 91.25);
  assert.equal(compass.trueHeading, 88.75);
  assert.deepEqual(compass.magnetometer, [11.5, -22.25, 33.125]);
  // A timestamp is 100-nanosecond ticks since year one and does not survive a double, so it
  // crosses as a bigint. This exact value would be wrong by hundreds of ticks through a Number.
  assert.equal(compass.timestampTicks, "638000000000000000");
  assert.equal(compass.offsetTicks, "36000000000");

  // The gyroscope's data path is *not* reachable on this host, and that is recorded rather than
  // worked around. CNA gives the compass and the motion sensor a full synthetic backend through
  // cna_compass_set_test_backend_ext and cna_motion_set_test_backend_ext; the gyroscope has only
  // cna_gyroscope_set_supported_for_tests_ext, which flips the support answer without installing
  // anything to read. So Start still needs a platform sensor service, HEADLESS has none, and the
  // injection is accepted while producing nothing valid. Asserting a reading here would mean
  // asserting something that did not happen.
  assert.equal(
    evidence.gyroscopeStateAfterSupportOverride, evidence.states.NotSupported,
    "a support override alone does not make the sensor readable",
  );
  assert.equal(evidence.gyroscopeStart, "Error", "no sensor service: starting is refused");
  assert.equal(evidence.gyroscopeInject, "accepted", "the injection route itself works");
  assert.equal(
    evidence.gyroscopeValidAfterInjection, false,
    "an injection with no backend behind it produces no valid reading",
  );
  assert.equal(evidence.gyroscopeValueAfterInjection, "InvalidOperationException");

  const motion = evidence.motionAfterInjection;
  assert.equal(typeof motion, "object", `motion injection failed: ${motion}`);
  assert.equal(motion.state, evidence.states.Ready);
  assert.equal(motion.valid, true);
  assert.equal(motion.northReferenced, true, "the synthetic backend was installed north-referenced");
  // Both directions, which is what proves the flag is read from the backend rather than defaulted.
  assert.equal(evidence.motionNorthWhenNotReferenced, false);
  assert.equal(evidence.motionNorthWhenReferenced, true);
  assert.deepEqual([motion.pitch, motion.roll, motion.yaw], [0.125, -0.25, 1.5]);
  assert.deepEqual(motion.quaternion, [0.1, 0.2, 0.3, 0.4].map((v) => Math.fround(v)));
  // The matrix is sixteen distinguishable values, so a transposed read is caught: the first row
  // is 1..4 and the diagonal is 1, 6, 11, 16.
  assert.deepEqual(motion.matrixFirstRow, [1, 2, 3, 4]);
  assert.deepEqual(motion.matrixDiagonal, [1, 6, 11, 16]);
  // Three vectors with disjoint value sets: acceleration positive, rotation rate negative, gravity
  // a single physical constant on one axis. Substituting any for any other fails.
  assert.deepEqual(motion.acceleration, [1.5, 2.5, 3.5]);
  assert.deepEqual(motion.rotationRate, [-1.25, -2.25, -3.25]);
  assert.deepEqual(motion.gravity, [0, Math.fround(-9.80665), 0]);
  assert.equal(motion.timestampTicks, "638000000000000001");

  assert.equal(evidence.compassInterval, "200000", "the interval request is read back");
  assert.equal(
    evidence.motionBackendSwapWhileStarted, "Error",
    "a backend cannot be swapped underneath a running acquisition",
  );
  // What withdrawing a backend actually does, measured rather than assumed -- and the three parts
  // do not agree with each other. Stopping moves the state to Disabled; removing the backend
  // leaves the state there and leaves IsDataValid reporting true; but reading the value refuses
  // with "the sensor is not supported on this device". So a caller that trusted IsDataValid would
  // be told there is a reading and then refused it. That disagreement is recorded in
  // docs/upstream-cna-findings.md, and this assertion is what notices if CNA changes it.
  assert.equal(evidence.motionStateAfterStop, 5, "SensorState.Disabled");
  assert.equal(evidence.motionAfterRemoval.state, 5, "removal leaves the state where Stop put it");
  // Upstream finding 5, fixed in CNA (BINDFIX-016): withdrawing the backend now withdraws the
  // reading too, so IsDataValid and the value agree that there is none.
  assert.equal(evidence.motionAfterRemoval.isDataValid, false, "IsDataValid no longer claims a reading");
  assert.notEqual(
    evidence.motionValueAfterRemoval, "returned a reading",
    "...and the value refuses, in agreement",
  );
  assert.deepEqual(evidence.disposed, [true, true, true]);
  assert.equal(evidence.compassAfterDispose, "ObjectDisposedException");
});

class GraphicsAdapterProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    const { GraphicsAdapter, GraphicsProfile, SurfaceFormat, DepthFormat } = Graphics;
    const record = (name, body) => {
      try {
        this.evidence[name] = body();
      } catch (error) {
        this.evidence[name] = `${error.constructor.name}: ${(error.message ?? "").slice(0, 80)}`;
      }
    };

    // The adapter list is read here rather than at device creation, because CNA permits borrowing
    // the device handle only inside a lifecycle callback. That is the whole reason the projection
    // is lazy, and this is where a game would ask anyway.
    record("count", () => GraphicsAdapter.Adapters.length);
    record("adapter", () => {
      const adapter = GraphicsAdapter.DefaultAdapter;
      return {
        description: adapter.Description,
        deviceName: adapter.DeviceName,
        vendorId: adapter.VendorId,
        deviceId: adapter.DeviceId,
        revision: adapter.Revision,
        subSystemId: adapter.SubSystemId,
        isDefault: adapter.IsDefaultAdapter,
        monitorHandleIsBigInt: typeof adapter.MonitorHandle === "bigint",
      };
    });
    record("currentMode", () => {
      const mode = GraphicsAdapter.DefaultAdapter.CurrentDisplayMode;
      return {
        width: mode.Width,
        height: mode.Height,
        format: mode.Format,
        aspectRatio: mode.AspectRatio,
      };
    });
    record("supportedModes", () => {
      const modes = [...GraphicsAdapter.DefaultAdapter.SupportedDisplayModes];
      return {
        count: modes.length,
        first: modes.length > 0
          ? { width: modes[0].Width, height: modes[0].Height, format: modes[0].Format }
          : null,
      };
    });
    record("profiles", () => ({
      reach: GraphicsAdapter.DefaultAdapter.IsProfileSupported(GraphicsProfile.Reach),
      hiDef: GraphicsAdapter.DefaultAdapter.IsProfileSupported(GraphicsProfile.HiDef),
    }));
    record("backBufferQuery", () => {
      const result = GraphicsAdapter.DefaultAdapter.QueryBackBufferFormat(
        GraphicsProfile.HiDef, SurfaceFormat.Color, DepthFormat.Depth24, 0,
      );
      return { ...result };
    });
    record("renderTargetQuery", () => {
      const result = GraphicsAdapter.DefaultAdapter.QueryRenderTargetFormat(
        GraphicsProfile.HiDef, SurfaceFormat.Color, DepthFormat.Depth24Stencil8, 0,
      );
      return { ...result };
    });
    // The device's own adapter must be the same object the static list hands out, not a copy.
    record("deviceAdapterIsDefault", () =>
      this.GraphicsDevice.Adapter === GraphicsAdapter.DefaultAdapter);
    // Asking twice must give the same objects: the list is read once and cached, so a caller
    // holding an adapter is not holding one of several.
    record("stableIdentity", () =>
      GraphicsAdapter.Adapters[0] === GraphicsAdapter.Adapters[0] &&
      GraphicsAdapter.DefaultAdapter === GraphicsAdapter.DefaultAdapter);
    // The two format routes must be *different* routes. Reach + Single is a case where they
    // genuinely disagree: a single-channel float is not a legal back buffer, so that query refuses
    // and falls back to Color, while the same format is a perfectly good render target and that
    // query accepts it exactly. A projection that called one route for both would return the same
    // answer twice and fail here -- which an earlier version of this test did not catch, because
    // it compared two triples that happen to agree.
    record("divergentQueries", () => {
      const adapter = GraphicsAdapter.DefaultAdapter;
      const backBuffer = adapter.QueryBackBufferFormat(
        GraphicsProfile.Reach, SurfaceFormat.Single, DepthFormat.None, 0,
      );
      // Reach has no floating-point render target at all (XNA's profile table), so the render
      // target half asks HiDef, where Single is legal.
      const reachTarget = adapter.QueryRenderTargetFormat(
        GraphicsProfile.Reach, SurfaceFormat.Single, DepthFormat.None, 0,
      );
      const renderTarget = adapter.QueryRenderTargetFormat(
        GraphicsProfile.HiDef, SurfaceFormat.Single, DepthFormat.None, 0,
      );
      return {
        backBuffer: { success: backBuffer.Success, format: backBuffer.Format },
        reachTarget: { success: reachTarget.Success, format: reachTarget.Format },
        renderTarget: { success: renderTarget.Success, format: renderTarget.Format },
      };
    });
    // A format the renderer will not give exactly must report so rather than claiming success.
    record("impossibleQuery", () => {
      const result = GraphicsAdapter.DefaultAdapter.QueryBackBufferFormat(
        GraphicsProfile.Reach, SurfaceFormat.Color, DepthFormat.Depth24Stencil8, 64,
      );
      return { success: result.Success, samples: result.MultiSampleCount };
    });
    super.LoadContent();
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.Exit();
    super.Draw(gameTime);
  }
}

test("GraphicsAdapter reports CNA's real adapter, its modes and its format answers", async () => {
  const game = new GraphicsAdapterProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // This is the strict XNA type that was projected structurally and could not be filled until CNA's
  // fourteen adapter routes were bound. It is filled now, on this renderer and on a windowed one.
  assert.equal(evidence.count, 1, `expected one adapter, saw ${JSON.stringify(evidence.count)}`);

  const adapter = evidence.adapter;
  assert.equal(typeof adapter, "object", `adapter read failed: ${adapter}`);
  // With no platform display CNA answers its fallback identity; with one, Description is the
  // platform's display name. DeviceName is XNA's synthetic primary display name in both cases.
  assert.equal(typeof adapter.description, "string");
  assert.ok(adapter.description.length > 0, "an adapter is described");
  assert.equal(adapter.deviceName, "\\\\.\\DISPLAY1", "XNA's canonical primary display name");
  assert.equal(adapter.isDefault, true);
  assert.equal(typeof adapter.vendorId, "number");
  assert.equal(typeof adapter.deviceId, "number");
  assert.ok(adapter.vendorId !== 0 || adapter.deviceId !== 0, "an adapter has some identity");
  // Four separate integer fields, so a projection reading one for another would show here.
  assert.notEqual(adapter.vendorId, adapter.deviceId);
  assert.equal(typeof adapter.revision, "number");
  assert.equal(typeof adapter.subSystemId, "number");
  // XNA's MonitorHandle is an IntPtr; a renderer with no native monitor answers zero rather than
  // failing the whole adapter, and it must still cross as a bigint.
  assert.equal(adapter.monitorHandleIsBigInt, true);

  // The current mode, and the aspect ratio CNA derives from it rather than one computed here.
  const mode = evidence.currentMode;
  assert.equal(typeof mode, "object", `current mode failed: ${mode}`);
  assert.ok(mode.width > 0 && mode.height > 0, `implausible mode ${mode.width}x${mode.height}`);
  if (adapter.description === "Default Display") {
    assert.deepEqual([mode.width, mode.height], [800, 480], "the fallback adapter's one mode");
  }
  assert.equal(mode.format, Graphics.SurfaceFormat.Color);
  assert.ok(
    Math.abs(mode.aspectRatio - mode.width / mode.height) < 1e-5,
    `aspect ratio ${mode.aspectRatio} does not match ${mode.width}x${mode.height}`,
  );

  // The supported-mode list is a real list from CNA, and its first entry is a real mode.
  const modes = evidence.supportedModes;
  assert.equal(typeof modes, "object", `supported modes failed: ${modes}`);
  assert.ok(modes.count >= 1, "an adapter supports at least one mode");
  assert.ok(modes.first.width > 0 && modes.first.height > 0);

  // Both XNA profiles, answered by CNA rather than assumed. Reach is the subset of HiDef, so a
  // renderer supporting HiDef must support Reach; the converse is not required.
  const profiles = evidence.profiles;
  assert.equal(typeof profiles, "object", `profile query failed: ${profiles}`);
  assert.equal(typeof profiles.reach, "boolean");
  assert.equal(typeof profiles.hiDef, "boolean");
  if (profiles.hiDef) assert.equal(profiles.reach, true, "HiDef implies Reach");

  // The two format queries are separate routes and must answer separately: a back-buffer query and
  // a render-target query for the same triple are different questions.
  for (const name of ["backBufferQuery", "renderTargetQuery"]) {
    const query = evidence[name];
    assert.equal(typeof query, "object", `${name} failed: ${query}`);
    assert.equal(typeof query.Success, "boolean");
    assert.equal(query.Format, Graphics.SurfaceFormat.Color, `${name} kept the requested format`);
    assert.equal(typeof query.MultiSampleCount, "number");
  }

  // A 64-sample back buffer is not something any of these renderers gives exactly, so the query
  // must report an inexact match and a sample count it can actually provide. A projection that
  // always answered Success would pass every assertion above and fail this one.
  const impossible = evidence.impossibleQuery;
  assert.equal(typeof impossible, "object", `impossible query failed: ${impossible}`);
  assert.equal(impossible.success, false, "64x multisampling is not an exact match anywhere here");
  assert.ok(
    impossible.samples < 64,
    `CNA must answer with a count it can provide, not the 64 it was asked for (got ${impossible.samples})`,
  );

  // The two format queries are separate CNA routes, and here they must give separate answers.
  const divergent = evidence.divergentQueries;
  assert.equal(typeof divergent, "object", `divergent query failed: ${divergent}`);
  assert.equal(
    divergent.backBuffer.success, false,
    "SurfaceFormat.Single is not a legal Reach back buffer",
  );
  assert.equal(
    divergent.backBuffer.format, Graphics.SurfaceFormat.Color,
    "and the back-buffer query falls back to Color",
  );
  assert.equal(
    divergent.reachTarget.success, false,
    "Reach has no floating-point render target either (XNA's profile table)",
  );
  assert.equal(
    divergent.renderTarget.success, true,
    "...while under HiDef the same format is a perfectly good render target",
  );
  assert.equal(divergent.renderTarget.format, Graphics.SurfaceFormat.Single);

  assert.equal(evidence.deviceAdapterIsDefault, true, "GraphicsDevice.Adapter is the default one");
  assert.equal(evidence.stableIdentity, true, "the adapter list is read once, not per access");
});


class StandaloneDeviceProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.manager.PreferredBackBufferWidth = 320;
    this.manager.PreferredBackBufferHeight = 240;
    this.evidence = Object.create(null);
  }

  LoadContent() {
    const { GraphicsAdapter, GraphicsProfile, PresentationParameters, Texture2D } = Graphics;
    const record = (name, body) => {
      try {
        this.evidence[name] = body();
      } catch (error) {
        this.evidence[name] = { refused: error.constructor.name, message: error.message };
      }
    };

    // XNA's public constructor needs an adapter, and CNA's route indexes adapters the way its own
    // enumeration reports them -- which needs a live device. So this runs inside LoadContent, with
    // the game's device already up, and builds a *second* device beside it.
    const adapter = GraphicsAdapter.DefaultAdapter;
    const parameters = new PresentationParameters();
    // Deliberately different from the game's 320x240, so a device that answered with the game's
    // state instead of its own is visible rather than plausible.
    parameters.BackBufferWidth = 64;
    parameters.BackBufferHeight = 48;

    record("standalone", () => {
      const own = new Graphics.GraphicsDevice(adapter, GraphicsProfile.Reach, parameters);
      try {
        const texture = new Texture2D(own, 2, 2);
        texture.SetData([Color.Red, Color.Green, Color.Blue, Color.White]);
        const readback = new Array(4);
        texture.GetData(readback);
        const result = {
          isDistinctObject: own !== this.GraphicsDevice,
          profile: own.GraphicsProfile,
          gameProfile: this.GraphicsDevice.GraphicsProfile,
          size: [own.PresentationParameters.BackBufferWidth, own.PresentationParameters.BackBufferHeight],
          gameSize: [
            this.GraphicsDevice.PresentationParameters.BackBufferWidth,
            this.GraphicsDevice.PresentationParameters.BackBufferHeight,
          ],
          viewport: [own.Viewport.Width, own.Viewport.Height],
          adapterIsTheOneGiven: own.Adapter === adapter,
          texels: readback.map((color) => color.PackedValue),
          textureDeviceIsTheStandaloneOne: texture.GraphicsDevice === own,
          isDisposedBefore: own.IsDisposed,
        };
        own.Clear(new Color(12, 34, 56, 255));
        result.clearAccepted = true;
        // A resource from one device is not usable with another: XNA's rule, and this is the first
        // place in this package where two devices exist at once to check it.
        try {
          new Graphics.SpriteBatch(this.GraphicsDevice).Draw(texture, Vector2.Zero, Color.White);
          result.crossDeviceDraw = "ACCEPTED";
        } catch (error) {
          result.crossDeviceDraw = error.constructor.name;
        }
        texture.Dispose();
        own.Dispose();
        result.isDisposedAfter = own.IsDisposed;
        // Disposing twice is harmless, and the game's own device is untouched by any of it.
        own.Dispose();
        result.gameDeviceStillWorks = this.GraphicsDevice.Viewport.Width === 320;
        return result;
      } catch (error) {
        return { failed: `${error.constructor.name}: ${error.message}` };
      }
    });

    record("refusals", () => {
      const attempts = {};
      const attempt = (name, body) => {
        try {
          const made = body();
          made?.Dispose?.();
          attempts[name] = "ACCEPTED";
        } catch (error) {
          attempts[name] = error.constructor.name;
        }
      };
      attempt("nullAdapter",
        () => new Graphics.GraphicsDevice(null, GraphicsProfile.Reach, parameters));
      attempt("nullParameters",
        () => new Graphics.GraphicsDevice(adapter, GraphicsProfile.Reach, null));
      attempt("foreignAdapter", () => new Graphics.GraphicsDevice(
        Object.create(Object.getPrototypeOf(adapter)), GraphicsProfile.Reach, parameters,
      ));
      return attempts;
    });

    // Two devices at once, each with its own presentation parameters, so neither can be reading
    // the other's state.
    record("twoAtOnce", () => {
      const first = new PresentationParameters();
      first.BackBufferWidth = 16;
      first.BackBufferHeight = 16;
      const second = new PresentationParameters();
      second.BackBufferWidth = 128;
      second.BackBufferHeight = 96;
      const a = new Graphics.GraphicsDevice(adapter, GraphicsProfile.Reach, first);
      const b = new Graphics.GraphicsDevice(adapter, GraphicsProfile.HiDef, second);
      try {
        return {
          sizes: [
            [a.PresentationParameters.BackBufferWidth, a.PresentationParameters.BackBufferHeight],
            [b.PresentationParameters.BackBufferWidth, b.PresentationParameters.BackBufferHeight],
          ],
          profiles: [a.GraphicsProfile, b.GraphicsProfile],
          distinct: a !== b,
        };
      } finally {
        a.Dispose();
        b.Dispose();
      }
    });

    // Disposing a *manager*-created device must not release the game's native lifetime -- that
    // handle belongs to the game, and freeing it here would tear down everything. Read directly
    // off the lifetime rather than inferred from a later symptom, because the symptom of getting
    // this wrong is a use-after-free rather than a failed assertion.
    record("managerDeviceDisposal", () => {
      const gameLifetime = getBackend().ParentLifetime;
      const before = gameLifetime.State;
      this.GraphicsDevice.Dispose();
      this.deviceDisposed = true;
      return {
        before,
        after: gameLifetime.State,
        deviceReportsDisposed: this.GraphicsDevice.IsDisposed,
      };
    });

    this.Exit();
    super.LoadContent();
  }

  Draw(gameTime) {
    if (!this.deviceDisposed) this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.Exit();
    super.Draw(gameTime);
  }
}

test("XNA's public GraphicsDevice constructor makes a real second device", async () => {
  const game = new StandaloneDeviceProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  const own = evidence.standalone;
  assert.equal(typeof own, "object", `standalone device failed: ${JSON.stringify(own)}`);
  assert.equal(own.failed, undefined, own.failed);
  assert.equal(own.isDistinctObject, true, "a caller-created device is not the game's");
  assert.equal(own.profile, Graphics.GraphicsProfile.Reach, "it keeps the profile it was given");
  // Its presentation parameters are its own, not the manager's: 64x48 against the game's 320x240.
  assert.deepEqual(own.size, [64, 48], "the device reports the parameters it was constructed with");
  assert.deepEqual(own.gameSize, [320, 240], "and the game's device still reports the manager's");
  assert.deepEqual(own.viewport, [64, 48], "its viewport follows its own back buffer");
  assert.equal(own.adapterIsTheOneGiven, true, "GraphicsDevice.Adapter is the adapter passed in");

  // A texture created on that device round-trips its exact four texels. This is the assertion that
  // separates "a handle came back" from "a real device that stores real pixels".
  assert.deepEqual(
    own.texels,
    [Color.Red, Color.Green, Color.Blue, Color.White].map((color) => color.PackedValue),
    "four exact texels through a caller-created device",
  );
  assert.equal(own.textureDeviceIsTheStandaloneOne, true, "the texture belongs to that device");
  assert.equal(own.clearAccepted, true);

  // XNA forbids using a resource from one device with another, and two devices existing at once is
  // the only way to check it.
  assert.notEqual(
    own.crossDeviceDraw, "ACCEPTED",
    "a texture from one device must not be drawable through another device's SpriteBatch",
  );

  assert.equal(own.isDisposedBefore, false);
  assert.equal(own.isDisposedAfter, true, "Dispose releases a device this package owns");
  assert.equal(
    own.gameDeviceStillWorks, true,
    "and releasing it leaves the game's own device untouched",
  );

  assert.deepEqual(evidence.refusals, {
    nullAdapter: "ArgumentNullException",
    nullParameters: "ArgumentNullException",
    foreignAdapter: "ArgumentException",
  });

  // Two caller-created devices alive at once, each answering with its own state.
  const two = evidence.twoAtOnce;
  assert.equal(typeof two, "object", `two devices failed: ${JSON.stringify(two)}`);
  assert.equal(two.distinct, true);
  assert.deepEqual(two.sizes, [[16, 16], [128, 96]], "neither device reads the other's parameters");
  assert.deepEqual(
    two.profiles, [Graphics.GraphicsProfile.Reach, Graphics.GraphicsProfile.HiDef],
    "nor the other's profile",
  );

  // The ownership rule, stated where it can be checked: a caller-created device owns its handle
  // and releases it; a manager-created one does not, and disposing it must leave the game's native
  // lifetime alive. Without this, a Dispose that released whatever lifetime it found would pass
  // every other assertion here and free the game's handle in production.
  const managed = evidence.managerDeviceDisposal;
  assert.equal(typeof managed, "object", `manager disposal failed: ${JSON.stringify(managed)}`);
  assert.equal(managed.before, "active", "the game's native lifetime is live before the test");
  assert.equal(
    managed.after, "active",
    "disposing a manager-created GraphicsDevice must not release the game's native lifetime",
  );
  assert.equal(managed.deviceReportsDisposed, true, "while the device itself does report disposed");

  console.log(
    `CNA_TS_NATIVE_STANDALONE_DEVICE=PASS SIZE=${own.size.join("x")} ` +
    `GAME_SIZE=${own.gameSize.join("x")} TEXELS=EXACT CROSS_DEVICE=${own.crossDeviceDraw} ` +
    `TWO_AT_ONCE=${two.sizes.map((s) => s.join("x")).join("|")}`,
  );
});


class CameraProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    const { CameraState, CnaCamera, CnaCameraTestHooks } = devicesModule;
    const record = (name, body) => {
      try {
        this.evidence[name] = body();
      } catch (error) {
        this.evidence[name] = { refused: error.constructor.name, cnaResult: error.cnaResult };
      }
    };

    // The platform's real camera, on a machine that has none. Opening still succeeds -- that is
    // CNA's documented contract -- and the state is what says there is nothing there.
    record("real", () => {
      const camera = CnaCamera.Open();
      try {
        return {
          state: camera.State,
          width: camera.FrameWidth,
          height: camera.FrameHeight,
          isTestBackend: camera.IsTestBackend,
        };
      } finally {
        camera.Dispose();
      }
    });

    record("hooksRefuseRealCamera", () => {
      const camera = CnaCamera.Open();
      try {
        const attempts = {};
        for (const [name, body] of [
          ["setState", () => CnaCameraTestHooks.SetState(camera, CameraState.Ready)],
          ["setFrame", () => CnaCameraTestHooks.SetFrame(camera, 1, 1, new Uint8Array(4))],
          ["clearFrame", () => CnaCameraTestHooks.ClearFrame(camera)],
        ]) {
          try {
            body();
            attempts[name] = "ACCEPTED";
          } catch (error) {
            attempts[name] = error.constructor.name;
          }
        }
        return attempts;
      } finally {
        camera.Dispose();
      }
    });

    record("test", () => {
      const camera = CnaCamera.OpenForTests();
      const texture = new Graphics.Texture2D(this.GraphicsDevice, 2, 2);
      const wrongSize = new Graphics.Texture2D(this.GraphicsDevice, 4, 4);
      try {
        const result = {
          isTestBackend: camera.IsTestBackend,
          initialState: camera.State,
          initialSize: [camera.FrameWidth, camera.FrameHeight],
        };
        // Every state CNA lets a caller inject, each read back. Closed and NotSupported are
        // refused for a device that was opened, which is CNA's own rule and recorded as such.
        result.states = [
          CameraState.Opening, CameraState.Denied, CameraState.Ready, CameraState.Lost,
        ].map((state) => {
          CnaCameraTestHooks.SetState(camera, state);
          return [state, camera.State];
        });
        result.refusedStates = [CameraState.Closed, CameraState.NotSupported].map((state) => {
          try {
            CnaCameraTestHooks.SetState(camera, state);
            return "ACCEPTED";
          } catch (error) {
            return `result ${error.cnaResult}`;
          }
        });

        // Four distinct pixels, so a frame copied from the wrong place, in the wrong order, or
        // channel-swapped is a different number rather than a coincidence.
        const rgba = Uint8Array.from([
          255, 0, 0, 255,
          0, 255, 0, 255,
          0, 0, 255, 255,
          8, 16, 32, 64,
        ]);
        CnaCameraTestHooks.SetFrame(camera, 2, 2, rgba);
        result.afterPublish = {
          state: camera.State,
          size: [camera.FrameWidth, camera.FrameHeight],
        };

        result.acquired = camera.TryAcquireFrame(texture);
        const readback = new Array(4);
        texture.GetData(readback);
        result.texels = readback.map((color) => color.PackedValue);

        // A texture whose size does not match the frame is refused, the same way as no frame.
        result.wrongSize = camera.TryAcquireFrame(wrongSize);
        // The frame stays available until it is replaced.
        result.acquiredAgain = camera.TryAcquireFrame(texture);

        // A second, different frame replaces the first.
        const second = Uint8Array.from([
          1, 2, 3, 4,
          5, 6, 7, 8,
          9, 10, 11, 12,
          13, 14, 15, 16,
        ]);
        CnaCameraTestHooks.SetFrame(camera, 2, 2, second);
        camera.TryAcquireFrame(texture);
        const replaced = new Array(4);
        texture.GetData(replaced);
        result.replacedTexels = replaced.map((color) => color.PackedValue);

        CnaCameraTestHooks.ClearFrame(camera);
        result.afterClear = { state: camera.State, acquired: camera.TryAcquireFrame(texture) };

        camera.Dispose();
        result.disposedTwice = (() => { camera.Dispose(); return camera.IsDisposed; })();
        try {
          camera.State;
          result.readAfterDispose = "ACCEPTED";
        } catch (error) {
          result.readAfterDispose = error.constructor.name;
        }
        return result;
      } finally {
        texture.Dispose();
        wrongSize.Dispose();
      }
    });

    // The injection hooks must refuse a camera that is not CNA's test backend, or a test could
    // fabricate a reading for a device the platform actually owns.
    record("argumentRefusals", () => {
      const camera = CnaCamera.OpenForTests();
      try {
        const attempts = {};
        for (const [name, body] of [
          ["shortFrame", () => CnaCameraTestHooks.SetFrame(camera, 2, 2, new Uint8Array(4))],
          ["negativeWidth", () => CnaCameraTestHooks.SetFrame(camera, -1, 2, new Uint8Array(16))],
          ["badState", () => CnaCameraTestHooks.SetState(camera, 99)],
          ["notATexture", () => camera.TryAcquireFrame(null)],
          ["notACamera", () => CnaCameraTestHooks.SetState({}, CameraState.Ready)],
        ]) {
          try {
            body();
            attempts[name] = "ACCEPTED";
          } catch (error) {
            attempts[name] = error.constructor.name;
          }
        }
        return attempts;
      } finally {
        camera.Dispose();
      }
    });

    this.Exit();
    super.LoadContent();
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.Exit();
    super.Draw(gameTime);
  }
}

test("a camera frame reaches a caller-owned texture, exactly", async () => {
  const game = new CameraProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();
  const { CameraState } = devicesModule;

  // No verification machine has a camera, and that is the answer being asserted: opening the
  // platform's own still succeeds, and the state is what reports the absence.
  const real = evidence.real;
  assert.equal(typeof real, "object", `real camera failed: ${JSON.stringify(real)}`);
  if (real.refused !== undefined) {
    // An artifact built without CNA_DEVICES has no camera family at all, and says so with CNA's
    // own NOT_SUPPORTED rather than by pretending there is no camera.
    assert.equal(real.cnaResult, 6, `the camera was refused by something other than CNA: ${real.refused}`);
    console.log("CNA_TS_NATIVE_CAMERA=NO_DEVICE_LAYER");
    return;
  }
  assert.equal(real.isTestBackend, false);
  // Whether this host has a camera is the host's answer: NotSupported with no frame format where
  // there is none, and a live state with a real format where there is one. Opening succeeds either
  // way, which is CNA's contract.
  assert.ok(Object.values(CameraState).includes(real.state), `unknown camera state ${real.state}`);
  if (real.state === CameraState.NotSupported) {
    assert.deepEqual([real.width, real.height], [0, 0], "no camera reports no frame format");
  } else {
    assert.ok(real.width >= 0 && real.height >= 0, `a frame format: ${real.width}x${real.height}`);
  }

  const probe = evidence.test;
  assert.equal(typeof probe, "object", `test camera failed: ${JSON.stringify(probe)}`);
  assert.equal(probe.isTestBackend, true);
  assert.equal(probe.initialState, CameraState.Opening, "a test camera starts opening");
  assert.deepEqual(probe.initialSize, [0, 0], "with no frame format yet");

  // Every injectable state round-trips as itself, so the hook writes through to CNA rather than
  // being remembered here.
  assert.deepEqual(probe.states, [
    [CameraState.Opening, CameraState.Opening],
    [CameraState.Denied, CameraState.Denied],
    [CameraState.Ready, CameraState.Ready],
    [CameraState.Lost, CameraState.Lost],
  ]);
  // And CNA refuses the two a device it opened can never be in.
  assert.deepEqual(
    probe.refusedStates, ["result 1", "result 1"],
    "Closed and NotSupported are refused for an opened device, which is CNA's own rule",
  );

  // Publishing a frame reports the camera ready and fixes its format -- what a real camera does
  // when it starts producing.
  assert.equal(probe.afterPublish.state, CameraState.Ready);
  assert.deepEqual(probe.afterPublish.size, [2, 2]);

  // The four exact texels, through the real acquisition path into a texture this package owns.
  assert.equal(probe.acquired, true);
  assert.deepEqual(
    probe.texels,
    [
      new Color(255, 0, 0, 255).PackedValue,
      new Color(0, 255, 0, 255).PackedValue,
      new Color(0, 0, 255, 255).PackedValue,
      new Color(8, 16, 32, 64).PackedValue,
    ],
    "the frame's four pixels arrive in order, with their channels the right way round",
  );

  assert.equal(
    probe.wrongSize, false,
    "a texture whose size does not match the frame is refused, not resized",
  );
  assert.equal(probe.acquiredAgain, true, "the frame stays available until it is replaced");
  // A second frame really replaces the first: sixteen different bytes, sixteen different results.
  assert.deepEqual(
    probe.replacedTexels,
    [
      new Color(1, 2, 3, 4).PackedValue,
      new Color(5, 6, 7, 8).PackedValue,
      new Color(9, 10, 11, 12).PackedValue,
      new Color(13, 14, 15, 16).PackedValue,
    ],
    "publishing a second frame replaces the first rather than being ignored",
  );
  assert.notDeepEqual(probe.replacedTexels, probe.texels);

  assert.equal(probe.afterClear.acquired, false, "a cleared camera has no frame to give");
  assert.notEqual(
    probe.afterClear.state, CameraState.Ready,
    "and it is no longer ready",
  );
  assert.equal(probe.disposedTwice, true, "disposing twice is harmless");
  assert.equal(probe.readAfterDispose, "NativeUnavailableError", "a disposed camera refuses");

  // The injection hooks are for CNA's test backend only: a real device cannot be given a reading.
  assert.deepEqual(evidence.hooksRefuseRealCamera, {
    setState: "InvalidOperationException",
    setFrame: "InvalidOperationException",
    clearFrame: "InvalidOperationException",
  });

  assert.deepEqual(evidence.argumentRefusals, {
    shortFrame: "RangeError",
    negativeWidth: "RangeError",
    badState: "RangeError",
    notATexture: "TypeError",
    notACamera: "TypeError",
  });

  console.log(
    `CNA_TS_NATIVE_CAMERA=PASS REAL=${CameraState[real.state]} ` +
    `TEST_FRAME=${probe.afterPublish.size.join("x")} TEXELS=EXACT REPLACED=EXACT ` +
    `WRONG_SIZE_REFUSED=PASS HOOKS_REFUSE_REAL_DEVICE=PASS`,
  );
});

test("opening the platform camera after a test camera is safe (upstream finding 11, fixed)", () => {
  // CNA 0.21.0 left a dangling process-wide provider behind a destroyed test camera, so the next
  // platform camera dereferenced freed memory; CNA fixed it (BINDFIX-011, ClearCameraIf on
  // destroy). Still run in its own process, so a regression takes down the child rather than the
  // runner. An artifact without the device layer refuses the test camera with NOT_SUPPORTED.
  const script = new URL("fixtures/camera-test-backend-then-platform.mjs", import.meta.url);
  const child = spawnSync(process.execPath, [script.pathname], {
    env: { ...process.env, CNA_NATIVE_LIBRARY: library, CNA_NODE_BRIDGE: bridge },
    encoding: "utf8",
    timeout: 120_000,
  });
  const output = `${child.stdout ?? ""}${child.stderr ?? ""}`;
  assert.equal(child.signal, null, `the child died on ${child.signal}: ${output.slice(-400)}`);
  if (/does not contain the extended device layer/.test(output)) {
    assert.match(output, /CNA result 6/, "the absence is CNA's own NOT_SUPPORTED");
    console.log("CNA_TS_NATIVE_CAMERA_SEQUENCE=NO_DEVICE_LAYER");
    return;
  }
  assert.equal(child.status, 0, output.slice(-400));
  assert.match(output, /SURVIVED/);
  console.log("CNA_TS_NATIVE_CAMERA_SEQUENCE=SURVIVED SEQUENCE=test-backend-create,destroy,platform-create");
});



class GuideProbeGame extends Game {
  constructor() {
    super();
    this.manager = new GraphicsDeviceManager(this);
    this.evidence = Object.create(null);
  }

  LoadContent() {
    // XNA's Guide needs gamer services initialized, as a game's GamerServicesComponent does.
    if (!GamerServices.GamerServicesDispatcher.IsInitialized) {
      GamerServices.GamerServicesDispatcher.Initialize(this.Services);
    }
    const { Guide, MessageBoxIcon } = GamerServices;
    const { CnaGuide } = guideExtensions;
    const record = (name, body) => {
      try {
        this.evidence[name] = body();
      } catch (error) {
        this.evidence[name] = { refused: error.constructor.name, message: error.message };
      }
    };

    record("messageBox", () => {
      const seen = [];
      const result = Guide.BeginShowMessageBox(
        "Quit?", "Save first?", ["Save", "Discard", "Cancel"], 1, MessageBoxIcon.Warning,
        (value) => seen.push(value), "my-state",
      );
      const opened = {
        isCompletedBefore: result.IsCompleted,
        completedSynchronously: result.CompletedSynchronously,
        asyncState: result.AsyncState,
        guideIsVisible: Guide.IsVisible,
        hasPending: CnaGuide.HasPendingMessageBox,
        // The focus button CNA holds is the one XNA was told, which is what shows the argument
        // reaching the pending screen rather than being remembered here.
        focusButton: CnaGuide.PendingMessageBox?.FocusButton,
      };
      // Answer it as the player would have. The continuation runs on this call.
      CnaGuide.ForTests.ClickMessageBoxButton(2);
      return {
        ...opened,
        callbackCount: seen.length,
        callbackGotTheSameResult: seen[0] === result,
        isCompletedAfter: result.IsCompleted,
        answer: Guide.EndShowMessageBox(result),
        hasPendingAfter: CnaGuide.HasPendingMessageBox,
        pendingAfter: CnaGuide.PendingMessageBox,
      };
    });

    // A different button, so the answer cannot be a constant.
    record("messageBoxSecondAnswer", () => {
      const result = Guide.BeginShowMessageBox(
        "Again", "Pick", ["Zero", "One", "Two"], 0, MessageBoxIcon.None,
        () => {}, null,
      );
      CnaGuide.ForTests.ClickMessageBoxButton(1);
      return Guide.EndShowMessageBox(result);
    });

    // XNA's ValidateShowMessageBoxArgs allows one to three buttons; a fourth is refused before
    // anything is shown.
    record("messageBoxFourButtons", () => {
      try {
        Guide.BeginShowMessageBox(
          "Again", "Pick", ["Zero", "One", "Two", "Three"], 0, MessageBoxIcon.None, () => {}, null,
        );
        return "ACCEPTED";
      } catch (error) {
        return `${error.constructor.name}(${error.cnaResult ?? "-"})`;
      }
    });

    record("keyboardInput", () => {
      const seen = [];
      const result = Guide.BeginShowKeyboardInput(
        0, "Name", "Enter your name", "Player", (value) => seen.push(value), "kb-state",
      );
      const pending = CnaGuide.PendingKeyboardInput;
      const opened = {
        hasPending: CnaGuide.HasPendingKeyboardInput,
        title: pending?.Title,
        description: pending?.Description,
        displayText: pending?.DisplayText,
        asyncState: result.AsyncState,
      };
      CnaGuide.ForTests.CancelKeyboardInput();
      return {
        ...opened,
        callbackCount: seen.length,
        callbackGotTheSameResult: seen[0] === result,
        isCompleted: result.IsCompleted,
        wasCanceled: CnaGuide.WasKeyboardInputCanceled,
        answer: Guide.EndShowKeyboardInput(result),
        hasPendingAfter: CnaGuide.HasPendingKeyboardInput,
      };
    });

    record("refusals", () => {
      const attempts = {};
      const attempt = (name, body) => {
        try {
          body();
          attempts[name] = "ACCEPTED";
        } catch (error) {
          attempts[name] = error.constructor.name;
        }
      };
      // Arguments this package refuses before CNA sees them.
      attempt("noButtons", () => Guide.BeginShowMessageBox(
        "t", "x", [], 0, MessageBoxIcon.None, () => {}, null,
      ));
      attempt("focusPastTheEnd", () => Guide.BeginShowMessageBox(
        "t", "x", ["A"], 1, MessageBoxIcon.None, () => {}, null,
      ));
      attempt("nullTitle", () => Guide.BeginShowMessageBox(
        null, "x", ["A"], 0, MessageBoxIcon.None, () => {}, null,
      ));
      // An End with a result the Guide never produced, and one for the wrong operation.
      attempt("foreignResult", () => Guide.EndShowMessageBox({ IsCompleted: true }));
      attempt("negativeClick", () => CnaGuide.ForTests.ClickMessageBoxButton(-1));
      // Answering when nothing is pending: CNA's own refusal, not this package's.
      attempt("clickWithNothingPending", () => CnaGuide.ForTests.ClickMessageBoxButton(0));
      attempt("endWithNothingBegun", () => Guide.EndShowMessageBox({ IsCompleted: true }));
      return attempts;
    });

    record("secondBoxWhileOnePending", () => {
      const first = Guide.BeginShowMessageBox(
        "First", "x", ["A"], 0, MessageBoxIcon.None, () => {}, null,
      );
      let second = "ACCEPTED";
      try {
        Guide.BeginShowMessageBox("Second", "y", ["B"], 0, MessageBoxIcon.None, () => {}, null);
      } catch (error) {
        second = `result ${error.cnaResult}`;
      }
      CnaGuide.ForTests.ClickMessageBoxButton(0);
      Guide.EndShowMessageBox(first);
      return second;
    });

    // An End before the operation completes is an error, not a wait.
    record("endBeforeCompletion", () => {
      const result = Guide.BeginShowMessageBox(
        "Pending", "x", ["A"], 0, MessageBoxIcon.None, () => {}, null,
      );
      let refusal;
      try {
        Guide.EndShowMessageBox(result);
        refusal = "ACCEPTED";
      } catch (error) {
        refusal = error.constructor.name;
      }
      CnaGuide.ForTests.ClickMessageBoxButton(0);
      Guide.EndShowMessageBox(result);
      return refusal;
    });

    this.Exit();
    super.LoadContent();
  }

  Draw(gameTime) {
    this.GraphicsDevice.Clear(Color.CornflowerBlue);
    this.Exit();
    super.Draw(gameTime);
  }
}

test("XNA's Guide really shows a message box and a keyboard, and CNA answers them", async () => {
  const game = new GuideProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  // --- the message box -----------------------------------------------------------------------
  const box = evidence.messageBox;
  assert.equal(typeof box, "object", `message box failed: ${JSON.stringify(box)}`);
  assert.equal(box.refused, undefined, `message box refused: ${JSON.stringify(box)}`);
  assert.equal(box.isCompletedBefore, false, "it does not complete inside the Begin call");
  assert.equal(box.completedSynchronously, false, "and says so");
  assert.equal(box.asyncState, "my-state", "the caller's state is carried through untouched");
  assert.equal(box.guideIsVisible, true, "Guide.IsVisible is true while a screen is up");
  assert.equal(box.hasPending, true);
  assert.equal(
    box.focusButton, 1,
    "the focus button CNA holds is the one XNA was given, not a default",
  );

  // The continuation runs exactly once, with the very result object Begin returned -- which is
  // what XNA's IAsyncResult contract means and what a caller's `EndShow*` needs.
  assert.equal(box.callbackCount, 1, "the continuation runs once");
  assert.equal(box.callbackGotTheSameResult, true, "with the result Begin handed back");
  assert.equal(box.isCompletedAfter, true);
  assert.equal(box.answer, 2, "and the answer is the button that was pressed");
  assert.equal(box.hasPendingAfter, false, "the screen is gone");
  assert.equal(box.pendingAfter, null);

  // A different button on a different box: the answer tracks the press rather than being fixed.
  assert.equal(evidence.messageBoxSecondAnswer, 1);
  assert.match(
    evidence.messageBoxFourButtons, /\(1\)$/,
    "a fourth button is CNA's INVALID_ARGUMENT, as XNA's own argument check refuses it",
  );

  // --- the keyboard --------------------------------------------------------------------------
  const keyboard = evidence.keyboardInput;
  assert.equal(typeof keyboard, "object", `keyboard failed: ${JSON.stringify(keyboard)}`);
  assert.equal(keyboard.hasPending, true);
  // Three different strings, read back from CNA rather than from this test's own variables.
  assert.equal(keyboard.title, "Name");
  assert.equal(keyboard.description, "Enter your name");
  assert.equal(keyboard.displayText, "Player", "the input starts with the default text it was given");
  assert.equal(keyboard.asyncState, "kb-state");
  assert.equal(keyboard.callbackCount, 1);
  assert.equal(keyboard.callbackGotTheSameResult, true);
  assert.equal(keyboard.isCompleted, true);
  assert.equal(keyboard.wasCanceled, true);
  assert.equal(
    keyboard.answer, null,
    "XNA returns null for a cancelled input, which is not an empty string",
  );
  assert.equal(keyboard.hasPendingAfter, false);

  // --- refusals -------------------------------------------------------------------------------
  const refusals = evidence.refusals;
  assert.equal(refusals.noButtons, "ArgumentException", "a message box needs a button");
  assert.equal(refusals.focusPastTheEnd, "ArgumentOutOfRangeException");
  assert.equal(refusals.nullTitle, "ArgumentNullException");
  assert.equal(refusals.foreignResult, "ArgumentException", "an unrelated async result is refused");
  assert.equal(refusals.negativeClick, "RangeError");
  assert.notEqual(
    refusals.clickWithNothingPending, "ACCEPTED",
    "answering a screen that was never shown is refused",
  );

  // Only one Guide screen at a time, which is CNA's own rule.
  assert.equal(
    evidence.secondBoxWhileOnePending, "result 3",
    "a second message box while one is pending is INVALID_STATE",
  );

  // And End before the answer is an error rather than a wait -- this is not a promise.
  assert.equal(evidence.endBeforeCompletion, "InvalidOperationException");

  console.log(
    `CNA_TS_NATIVE_GUIDE=PASS MESSAGE_BOX=${box.answer}/3 FOCUS=${box.focusButton} ` +
    `KEYBOARD=cancelled ASYNC_CONTRACT=PASS ONE_AT_A_TIME=PASS`,
  );
});

/** A Matrix as the sixteen numbers XNA names, in the order it names them. */
function matrixRowOf(matrix) {
  return [
    matrix.M11, matrix.M12, matrix.M13, matrix.M14,
    matrix.M21, matrix.M22, matrix.M23, matrix.M24,
    matrix.M31, matrix.M32, matrix.M33, matrix.M34,
    matrix.M41, matrix.M42, matrix.M43, matrix.M44,
  ];
}

/** The inverse of the `vector` reader the probes above use. */
function vectorOf([x, y, z]) { return new Vector3(x, y, z); }

test("every debug-draw shape is a line list that can be read rather than looked at", async () => {
  const { DebugDraw } = computeExtensions;
  const device = null; // filled in by the probe game below

  class DebugProbeGame extends Game {
    constructor() {
      super();
      this.manager = new GraphicsDeviceManager(this);
      this.evidence = Object.create(null);
    }

    LoadContent() {
      const graphics = computeExtensions;
      const RED = new Color(200, 100, 40, 255);
      const BLUE = new Color(10, 20, 250, 128);
      const record = (name, body) => {
        try {
          this.evidence[name] = body();
        } catch (error) {
          this.evidence[name] = `${error.constructor.name}(${error.cnaResult ?? "-"}): ` +
            `${(error.message ?? "").slice(0, 140)}`;
        }
      };
      // Every shape is built into its own drawer, so no shape's lines can be another's.
      const shape = (build) => {
        const drawer = new graphics.DebugDraw(this.GraphicsDevice);
        try {
          const empty = {
            lines: drawer.LineCount,
            depthTested: drawer.DepthTested,
            tested: drawer.GetVertices(true).length,
            overlay: drawer.GetVertices(false).length,
          };
          build(drawer);
          return {
            empty,
            lines: drawer.LineCount,
            vertices: drawer.GetVertices(true).map((vertex) => ({
              position: [vertex.Position.X, vertex.Position.Y, vertex.Position.Z],
              color: [vertex.Color.R, vertex.Color.G, vertex.Color.B, vertex.Color.A],
            })),
            overlay: drawer.GetVertices(false).length,
          };
        } finally {
          drawer.Dispose();
        }
      };
      record("line", () => shape(
        (drawer) => drawer.AddLine(new Vector3(1, 2, 3), new Vector3(4, 5, 6), RED)));
      record("cross", () => shape(
        (drawer) => drawer.AddCross(new Vector3(10, 20, 30), 2, RED)));
      // Asymmetric in all three axes and not centred on the origin, so a swapped Min and Max is a
      // different picture rather than the same eight corners in a different order.
      record("box", () => shape((drawer) => drawer.AddBox(
        new BoundingBox(new Vector3(1, 2, 3), new Vector3(4, 6, 9)), RED)));
      record("sphere8", () => shape(
        (drawer) => drawer.AddSphere(new Vector3(5, 0, 0), 3, RED, 8)));
      record("sphere16", () => shape(
        (drawer) => drawer.AddSphere(new Vector3(5, 0, 0), 3, RED, 16)));
      record("boundingSphere", () => shape((drawer) => drawer.AddBoundingSphere(
        new BoundingSphere(new Vector3(5, 0, 0), 3), RED, 8)));
      record("frustum", () => {
        const view = Matrix.CreateLookAt(new Vector3(0, 0, 10), Vector3.Zero, Vector3.Up);
        const projection = Matrix.CreatePerspectiveFieldOfView(Math.PI / 4, 1, 1, 20);
        const viewProjection = Matrix.Multiply(view, projection);
        const result = shape((drawer) => drawer.AddFrustum(viewProjection, RED));
        result.corners = new BoundingFrustum(viewProjection).GetCorners()
          .map((corner) => [corner.X, corner.Y, corner.Z]);
        return result;
      });
      record("lists", () => {
        const drawer = new graphics.DebugDraw(this.GraphicsDevice);
        try {
          drawer.DepthTested = true;
          drawer.AddLine(Vector3.Zero, new Vector3(1, 0, 0), RED);
          drawer.DepthTested = false;
          drawer.AddLine(Vector3.Zero, new Vector3(0, 1, 0), BLUE);
          const state = {
            tested: drawer.GetVertices(true).map((v) => [v.Position.X, v.Position.Y, v.Position.Z]),
            overlay: drawer.GetVertices(false).map((v) => [v.Position.X, v.Position.Y, v.Position.Z]),
            testedColor: (() => {
              const c = drawer.GetVertices(true)[0].Color;
              return [c.R, c.G, c.B, c.A];
            })(),
            overlayColor: (() => {
              const c = drawer.GetVertices(false)[0].Color;
              return [c.R, c.G, c.B, c.A];
            })(),
            lines: drawer.LineCount,
            depthTestedFlag: drawer.DepthTested,
          };
          drawer.Clear();
          state.afterClear = [
            drawer.GetVertices(true).length, drawer.GetVertices(false).length, drawer.LineCount,
          ];
          return state;
        } finally {
          drawer.Dispose();
        }
      });
      record("frame", () => {
        const drawer = new graphics.DebugDraw(this.GraphicsDevice);
        try {
          drawer.AddLine(Vector3.Zero, new Vector3(1, 0, 0), RED);
          const view = Matrix.CreateLookAt(new Vector3(0, 0, 10), Vector3.Zero, Vector3.Up);
          const projection = Matrix.CreatePerspectiveFieldOfView(Math.PI / 4, 1, 1, 100);
          const attempt = (body) => {
            try {
              body();
              return "OK";
            } catch (error) {
              return `result ${error.cnaResult}`;
            }
          };
          const before = drawer.LineCount;
          const begin = attempt(() => drawer.Begin(view, projection));
          const afterBegin = drawer.LineCount;
          drawer.AddLine(Vector3.Zero, new Vector3(0, 1, 0), RED);
          const queued = drawer.LineCount;
          const end = attempt(() => drawer.End());
          return { before, begin, afterBegin, queued, end, afterEnd: drawer.LineCount };
        } finally {
          drawer.Dispose();
        }
      });
      record("refusals", () => {
        const drawer = new graphics.DebugDraw(this.GraphicsDevice);
        const attempt = (body) => {
          try {
            body();
            return "SUCCEEDED";
          } catch (error) {
            return error.constructor.name;
          }
        };
        const answers = {
          nullFrom: attempt(() => drawer.AddLine(null, Vector3.Zero, RED)),
          nullColor: attempt(() => drawer.AddLine(Vector3.Zero, Vector3.Zero, null)),
          fractionalSegments: attempt(
            () => drawer.AddSphere(Vector3.Zero, 1, RED, 8.5)),
          nonFiniteRadius: attempt(
            () => drawer.AddSphere(Vector3.Zero, Number.NaN, RED, 8)),
        };
        drawer.Dispose();
        answers.disposedAdd = attempt(() => drawer.AddLine(Vector3.Zero, Vector3.Zero, RED));
        answers.disposedRead = attempt(() => drawer.GetVertices(true));
        answers.disposedTwice = attempt(() => drawer.Dispose());
        answers.isDisposed = drawer.IsDisposed;
        return answers;
      });
      this.Exit();
      super.LoadContent();
    }

    Update(gameTime) {
      this.Exit();
      super.Update(gameTime);
    }
  }

  const game = new DebugProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();
  assert.equal(typeof DebugDraw, "function");
  assert.equal(device, null);

  if (!computeExtensions.IsGraphicsExtensionLayerAvailable()) {
    // An artifact built without CNA_CNAEXT refuses the drawer with CNA's own NOT_SUPPORTED.
    assert.match(String(evidence.line), /\(6\): /,
      `the drawer was refused by something other than CNA: ${JSON.stringify(evidence.line)}`);
    console.log("CNA_TS_NATIVE_DEBUG_DRAW=NO_EXTENSION_LAYER");
    return;
  }
  const RED_BYTES = [200, 100, 40, 255];
  const near = (actual, expected, what, tolerance = 1e-4) => assert.ok(
    Math.abs(actual - expected) < tolerance, `${what}: ${actual} vs ${expected}`);
  const allRed = (shape) => {
    for (const vertex of shape.vertices) {
      assert.deepEqual(vertex.color, RED_BYTES, "every vertex carries the colour it was given");
    }
  };
  // A drawer starts empty, and every shape below lands in the depth-tested list, not the overlay.
  for (const [name, shape] of Object.entries(evidence)) {
    if (typeof shape !== "object" || shape.empty === undefined) continue;
    assert.deepEqual(
      [shape.empty.lines, shape.empty.tested, shape.empty.overlay], [0, 0, 0],
      `${name}: a fresh drawer holds nothing`,
    );
    assert.equal(shape.empty.depthTested, true, `${name}: and starts depth-tested`);
    assert.equal(shape.overlay, 0, `${name}: nothing went to the overlay list`);
    assert.equal(
      shape.vertices.length, shape.lines * 2,
      `${name}: a line list is two vertices per line`,
    );
    allRed(shape);
  }

  // --- a line is its own two endpoints -------------------------------------------------------------
  assert.equal(evidence.line.lines, 1);
  assert.deepEqual(
    evidence.line.vertices.map((vertex) => vertex.position), [[1, 2, 3], [4, 5, 6]],
    "a line is exactly the two points it was given, in that order",
  );

  // --- a cross is three lines through a point ------------------------------------------------------
  assert.equal(evidence.cross.lines, 3, "one per axis");
  const crossEnds = evidence.cross.vertices.map((vertex) => vertex.position);
  assert.deepEqual(
    crossEnds,
    [[8, 20, 30], [12, 20, 30], [10, 18, 30], [10, 22, 30], [10, 20, 28], [10, 20, 32]],
    "each reaches the size in both directions along its own axis and nowhere else",
  );

  // --- a box is twelve edges at eight corners --------------------------------------------------------
  assert.equal(evidence.box.lines, 12, "a box is twelve edges");
  // The exact edge list, in order: four edges around the far face, four around the near one, and
  // four joining them. A swapped Min and Max keeps the same eight corners and changes this order,
  // which is why the order is what is asserted.
  assert.deepEqual(
    evidence.box.vertices.map((vertex) => vertex.position),
    [
      [1, 6, 9], [4, 6, 9], [4, 6, 9], [4, 2, 9], [4, 2, 9], [1, 2, 9], [1, 2, 9], [1, 6, 9],
      [1, 6, 3], [4, 6, 3], [4, 6, 3], [4, 2, 3], [4, 2, 3], [1, 2, 3], [1, 2, 3], [1, 6, 3],
      [1, 6, 9], [1, 6, 3], [4, 6, 9], [4, 6, 3], [4, 2, 9], [4, 2, 3], [1, 2, 9], [1, 2, 3],
    ],
    "a box's twelve edges, in the order it emits them",
  );
  const corners = new Set(
    evidence.box.vertices.map((vertex) => vertex.position.join(",")));
  const expectedCorners = new Set();
  for (const x of [1, 4]) for (const y of [2, 6]) for (const z of [3, 9]) {
    expectedCorners.add(`${x},${y},${z}`);
  }
  assert.deepEqual(
    [...corners].sort(), [...expectedCorners].sort(),
    "and every vertex is one of the box's eight corners",
  );
  // Twelve edges over eight corners means each corner is an endpoint of exactly three of them.
  const corneruses = new Map();
  for (const vertex of evidence.box.vertices) {
    const key = vertex.position.join(",");
    corneruses.set(key, (corneruses.get(key) ?? 0) + 1);
  }
  for (const [corner, count] of corneruses) {
    assert.equal(count, 3, `corner ${corner} is used ${count} times, not three`);
  }

  // --- a sphere is three rings, and every vertex is on it ---------------------------------------------
  for (const [name, segments] of [["sphere8", 8], ["sphere16", 16]]) {
    const sphere = evidence[name];
    assert.equal(
      sphere.lines, segments * 3,
      `${name}: three rings of ${segments} segments each`,
    );
    for (const vertex of sphere.vertices) {
      const [x, y, z] = vertex.position;
      near(Math.hypot(x - 5, y, z), 3, `${name}: a vertex off the sphere`, 1e-3);
      // Each ring lies in one of the three axis planes through the centre, so exactly one
      // coordinate equals the centre's on every vertex.
      const onPlane = [Math.abs(x - 5), Math.abs(y), Math.abs(z)].filter((v) => v < 1e-3).length;
      assert.ok(
        onPlane >= 1,
        `${name}: a vertex at ${vertex.position} is not on any axis plane through the centre`,
      );
    }
  }
  assert.equal(
    evidence.sphere16.lines, evidence.sphere8.lines * 2,
    "twice the segments is twice the lines",
  );
  // A BoundingSphere draws exactly what the same centre and radius draw.
  assert.deepEqual(
    evidence.boundingSphere.vertices, evidence.sphere8.vertices,
    "AddBoundingSphere and AddSphere must agree for the same sphere",
  );

  // --- a frustum's corners are the ones BoundingFrustum gives ------------------------------------------
  const frustum = evidence.frustum;
  assert.equal(frustum.lines, 12, "a frustum is twelve edges, like a box");
  const frustumPoints = frustum.vertices.map((vertex) => vertex.position);
  for (const point of frustumPoints) {
    const match = frustum.corners.some((corner) =>
      corner.every((value, axis) => Math.abs(value - point[axis]) < 1e-3));
    assert.ok(
      match,
      `a frustum vertex at ${point} is not one of BoundingFrustum's own corners`,
    );
  }
  // And all eight corners are used, so it is the whole frustum rather than one face twice.
  for (const corner of frustum.corners) {
    assert.ok(
      frustumPoints.some((point) => point.every((v, axis) => Math.abs(v - corner[axis]) < 1e-3)),
      `BoundingFrustum's corner ${corner} was not drawn`,
    );
  }

  // --- the two lists, and clearing them -------------------------------------------------------------------
  const lists = evidence.lists;
  assert.deepEqual(lists.tested, [[0, 0, 0], [1, 0, 0]], "the depth-tested list has its own line");
  assert.deepEqual(lists.overlay, [[0, 0, 0], [0, 1, 0]], "and the overlay has its own");
  assert.deepEqual(lists.testedColor, RED_BYTES);
  assert.deepEqual(lists.overlayColor, [10, 20, 250, 128], "each keeps its own colour, alpha and all");
  assert.equal(lists.lines, 2, "and the line count is both lists together");
  assert.equal(lists.depthTestedFlag, false, "the flag is where it was last put");
  assert.deepEqual(lists.afterClear, [0, 0, 0], "clearing empties both lists");

  // --- a frame ----------------------------------------------------------------------------------------------
  assert.equal(evidence.frame.before, 1, "a line queued outside a frame is held");
  assert.equal(evidence.frame.begin, "OK", "a drawer takes a camera");
  assert.equal(
    evidence.frame.afterBegin, 0,
    "and beginning a frame starts it empty, discarding whatever was queued before it",
  );
  assert.equal(evidence.frame.queued, 1, "lines added inside the frame are held until it ends");
  assert.equal(evidence.frame.end, "OK");
  assert.equal(evidence.frame.afterEnd, 0, "ending a frame draws the queue and empties it");

  // --- refusals ------------------------------------------------------------------------------------------------
  const refusals = evidence.refusals;
  assert.equal(refusals.nullFrom, "TypeError");
  assert.equal(refusals.nullColor, "TypeError");
  assert.equal(refusals.fractionalSegments, "TypeError", "a segment count is a whole number");
  assert.equal(refusals.nonFiniteRadius, "TypeError");
  assert.equal(refusals.disposedAdd, "NativeUnavailableError");
  assert.equal(refusals.disposedRead, "NativeUnavailableError");
  assert.equal(refusals.disposedTwice, "SUCCEEDED", "disposing twice is harmless");
  assert.equal(refusals.isDisposed, true);

  console.log(
    `CNA_TS_NATIVE_DEBUG_DRAW=PASS LINE=2 CROSS=6 BOX=12_EDGES SPHERE=3x8/3x16 ` +
    `FRUSTUM=BoundingFrustum_CORNERS`,
  );
});

test("a shader effect is an Effect, and says whether the renderer looked at its source", async () => {
  const graphics = computeExtensions;

  const VERTEX = `#version 300 es
precision highp float;
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
uniform mat4 projection;
void main() { gl_Position = projection * vec4(aPos, 0.0, 1.0); TexCoord = aTexCoord; }
`;
  const FRAGMENT = `#version 300 es
precision highp float;
in vec2 TexCoord;
out vec4 FragColor;
uniform vec3 uTint;
uniform float uScale;
void main() { FragColor = vec4(uTint * uScale, 1.0); }
`;

  class ShaderProbeGame extends Game {
    constructor() {
      super();
      this.manager = new GraphicsDeviceManager(this);
      this.evidence = Object.create(null);
    }

    LoadContent() {
      const device = this.GraphicsDevice;
      const record = (name, body) => {
        try {
          this.evidence[name] = body();
        } catch (error) {
          this.evidence[name] = `${error.constructor.name}(${error.cnaResult ?? "-"}): ` +
            `${(error.message ?? "").slice(0, 160)}`;
        }
      };
      const outcome = (body) => {
        try {
          body();
          return "ok";
        } catch (error) {
          return `${error.constructor.name}(${error.cnaResult ?? "-"})`;
        }
      };

      record("effect", () => {
        const effect = new graphics.ShaderEffect(device, VERTEX, FRAGMENT);
        try {
          const shape = {
            isEffect: effect instanceof Graphics.Effect,
            techniques: effect.Techniques.Count,
            passes: effect.CurrentTechnique.Passes.Count,
            valid: effect.IsEffectValid,
            hasRenderer: effect.HasRenderer,
            compileError: effect.CompileError,
          };
          // The three IEffectMatrices properties, each a value that survives the round trip and
          // each different from the other two, so a getter wired to the wrong field is caught.
          effect.World = Matrix.CreateTranslation(new Vector3(1, 2, 3));
          effect.View = Matrix.CreateScale(2, 3, 4);
          effect.Projection = Matrix.CreateOrthographic(8, 6, 1, 100);
          const matrices = {
            world: [effect.World.M41, effect.World.M42, effect.World.M43],
            view: [effect.View.M11, effect.View.M22, effect.View.M33],
            projection: [effect.Projection.M11, effect.Projection.M22],
          };
          const uniforms = {
            mat4: outcome(() => effect.SetUniformMat4("uTransform", Matrix.Identity)),
            vec4: outcome(() => effect.SetUniformVec4("uColour", new Vector4(1, 2, 3, 4))),
            vec3: outcome(() => effect.SetUniformVec3("uTint", new Vector3(1, 0.5, 0.25))),
            vec2: outcome(() => effect.SetUniformVec2("uOffset", new Vector2(1, 2))),
            float: outcome(() => effect.SetUniformFloat("uScale", 0.5)),
            int: outcome(() => effect.SetUniformInt("uMode", 1)),
            floats: outcome(() => effect.SetUniformFloatArray("uWeights", [1, 2, 3])),
            vec2s: outcome(
              () => effect.SetUniformVec2Array("uTaps", [new Vector2(0, 1), new Vector2(2, 3)])),
            vec3s: outcome(() => effect.SetUniformVec3Array("uProbes", [new Vector3(1, 2, 3)])),
            mat4s: outcome(
              () => effect.SetUniformMat4Array("uBones", [Matrix.Identity, Matrix.Identity])),
            block: outcome(() => effect.DeclareUniformBlock(64, ["uTint", "uScale"], [0, 16])),
            blockCleared: outcome(() => effect.DeclareUniformBlock(0, [], [])),
          };
          const refusals = {
            emptyName: outcome(() => effect.SetUniformFloat("", 1)),
            missingName: outcome(() => effect.SetUniformFloat(null, 1)),
            infiniteValue: outcome(() => effect.SetUniformFloat("uScale", Number.NaN)),
            mismatchedBlock: outcome(() => effect.DeclareUniformBlock(16, ["a", "b"], [0])),
            notAnArray: outcome(() => effect.SetUniformFloatArray("uWeights", 3)),
          };
          // Read again, after every uniform setter has run. A setter wired to one of the three
          // transform routes instead of to its own would show up here and nowhere above.
          const matricesAfter = {
            world: [effect.World.M41, effect.World.M42, effect.World.M43],
            view: [effect.View.M11, effect.View.M22, effect.View.M33],
            projection: [effect.Projection.M11, effect.Projection.M22],
          };
          return { shape, matrices, matricesAfter, uniforms, refusals };
        } finally {
          effect.Dispose();
        }
      });

      // A texture bound to an effect is lent to it, so the texture refuses to be disposed while the
      // effect can still sample it. Recorded as measured rather than asserted from the header.
      record("boundTexture", () => {
        const effect = new graphics.ShaderEffect(device, VERTEX, FRAGMENT);
        const texture = new Graphics.Texture2D(device, 2, 2);
        texture.SetData(new Array(4).fill(0).map(() => new Color(10, 20, 30, 40)));
        const bound = outcome(() => effect.SetTexture(1, texture));
        const disposedWhileBound = outcome(() => texture.Dispose());
        effect.Dispose();
        const disposedAfterEffect = outcome(() => texture.Dispose());
        return { bound, disposedWhileBound, disposedAfterEffect };
      });

      // Nonsense source. CNA is explicit that a successful create says the object exists and not
      // that anything compiled, and that renderers differ; this records which kind this one is.
      record("nonsense", () => {
        const effect = new graphics.ShaderEffect(device, "not glsl at all", "nor is this");
        const answer = {
          valid: effect.IsEffectValid,
          hasRenderer: effect.HasRenderer,
          compileError: effect.CompileError,
        };
        effect.Dispose();
        return answer;
      });

      record("bothEmpty", () => outcome(() => new graphics.ShaderEffect(device, "", "").Dispose()));
      record("oneEmpty", () => {
        const effect = new graphics.ShaderEffect(device, VERTEX, "");
        const valid = effect.IsEffectValid;
        effect.Dispose();
        return { created: true, valid };
      });
      record("badArguments", () => ({
        nullDevice: outcome(() => new graphics.ShaderEffect(null, VERTEX, FRAGMENT)),
        numberSource: outcome(() => new graphics.ShaderEffect(device, 3, FRAGMENT)),
      }));

      this.Exit();
    }
  }

  const game = new ShaderProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();

  const effect = evidence.effect;
  assert.equal(typeof effect, "object", `the shader-effect probe failed: ${effect}`);

  // It is an Effect. That is the point of the class: SpriteBatch.Begin and the device's draw calls
  // take an Effect, and a custom shader is only useful where the stock ones go.
  assert.equal(effect.shape.isEffect, true, "a ShaderEffect must be an Effect");
  assert.equal(effect.shape.techniques, 1, "with one technique");
  assert.equal(effect.shape.passes, 1, "and one pass");
  assert.equal(typeof effect.shape.valid, "boolean");
  assert.equal(effect.shape.hasRenderer, true, "and a live renderer behind it");

  // The three matrices every stock effect exposes, each round-tripped and each distinct.
  assert.deepEqual(effect.matrices.world, [1, 2, 3], "World keeps the translation it was given");
  assert.deepEqual(effect.matrices.view, [2, 3, 4], "View keeps its scale");
  assert.deepEqual(
    effect.matrices.projection, [Math.fround(2 / 8), Math.fround(2 / 6)],
    "and Projection keeps an orthographic matrix's own two scales -- in float, which is what CNA " +
    "stores, so 2/6 comes back as the nearest float rather than the double this test computed",
  );
  assert.notDeepEqual(
    effect.matrices.world, effect.matrices.view,
    "the three are separate fields, not one repeated",
  );

  assert.deepEqual(
    effect.matricesAfter, effect.matrices,
    "and none of the uniform setters below writes into one of them: setting a mat4 uniform named " +
    "something else must not move World, View or Projection",
  );

  // Every uniform shape CNA offers, each reaching the renderer without complaint.
  for (const [name, answer] of Object.entries(effect.uniforms)) {
    assert.equal(answer, "ok", `setting a ${name} uniform failed: ${answer}`);
  }

  // The refusals, each a different mistake and each named by this binding rather than by a result
  // code from deep inside CNA.
  assert.equal(effect.refusals.emptyName, "TypeError(-)", "an empty uniform name is refused here");
  assert.equal(effect.refusals.missingName, "TypeError(-)");
  assert.equal(effect.refusals.infiniteValue, "TypeError(-)", "and so is a value that is not finite");
  assert.equal(
    effect.refusals.mismatchedBlock, "TypeError(-)",
    "a uniform block whose names and offsets differ in length is refused before CNA sees it",
  );
  assert.equal(effect.refusals.notAnArray, "TypeError(-)");

  // A bound texture is lent to the effect: it refuses to be disposed while the effect can sample
  // it, and disposes cleanly once the effect is gone. Measured, not assumed.
  const bound = evidence.boundTexture;
  assert.equal(typeof bound, "object", `the texture probe failed: ${bound}`);
  assert.equal(bound.bound, "ok", "binding a texture to a sampler unit succeeds");
  assert.equal(
    bound.disposedWhileBound, "AggregateError(-)",
    "and the texture then refuses to be disposed while the effect still holds it",
  );
  assert.equal(
    bound.disposedAfterEffect, "ok",
    "while disposing the effect first releases it, so the order is the only thing that matters",
  );

  // Nonsense source. CNA's header says in as many words that success here means the object exists,
  // that whether a renderer compiles at construction is renderer-specific, and that "the same
  // nonsense text is accepted by both, and afterwards one reports it valid while the other reports
  // it invalid". Both halves of that are asserted, each against the renderer this suite runs on.
  const nonsense = evidence.nonsense;
  assert.equal(typeof nonsense, "object", `the nonsense probe failed: ${nonsense}`);
  assert.equal(nonsense.hasRenderer, true, "the object exists whatever the renderer thought");
  if (nonsense.valid) {
    // HEADLESS: accepts the source and never compiles it, so it has nothing to report.
    assert.equal(
      nonsense.compileError, "",
      "a renderer that reports nonsense valid never compiled it, so it has no log",
    );
  } else {
    assert.ok(
      nonsense.compileError.length > 0,
      "a renderer that rejected the source must say why",
    );
  }
  // The one thing settled everywhere: two empty sources are refused identically, rather than left
  // to the renderer.
  assert.equal(evidence.bothEmpty, "Error(1)", "two empty sources are INVALID_ARGUMENT");
  assert.equal(
    evidence.oneEmpty.created, true,
    "while one empty source is not refused -- only both together are",
  );
  assert.equal(evidence.badArguments.nullDevice, "TypeError(-)");
  assert.equal(evidence.badArguments.numberSource, "TypeError(-)");

  console.log(
    `CNA_TS_NATIVE_SHADER_EFFECT=PASS IS_EFFECT=true UNIFORM_SHAPES=${Object.keys(effect.uniforms).length} ` +
    `NONSENSE_VALID=${nonsense.valid} TEXTURE_BORROW=REFUSES_WHILE_BOUND`,
  );
});

test("the graphics-extension value routes that remain answer with CNA's own defaults", async () => {
  const graphics = computeExtensions;

  // --- the indirect command format ----------------------------------------------------------------
  assert.deepEqual(
    graphics.IndirectDraw.DefaultArguments(),
    { VertexCount: 0, InstanceCount: 0, FirstVertex: 0, BaseInstance: 0 },
    "the default arguments are all zero, which draws nothing",
  );
  assert.deepEqual(
    graphics.IndirectDraw.DefaultIndexedArguments(),
    { IndexCount: 0, InstanceCount: 0, FirstIndex: 0, BaseVertex: 0, BaseInstance: 0 },
    "and so are the indexed ones, one word longer",
  );
  assert.deepEqual(
    [...graphics.IndirectDraw.PackArguments(
      { VertexCount: 6, InstanceCount: 2, FirstVertex: 3, BaseInstance: 0 })],
    [6, 2, 3, 0],
    "a non-indexed command is four words: vertices, instances, first vertex, base instance",
  );
  const indexed = graphics.IndirectDraw.PackIndexedArguments(
    { IndexCount: 12, InstanceCount: 4, FirstIndex: 6, BaseVertex: -5, BaseInstance: 0 });
  assert.deepEqual(
    [...indexed], [12, 4, 6, 0xffff_fffb, 0],
    "and an indexed one is five, with a SIGNED base vertex -- -5 is 0xFFFFFFFB, not a clamp to zero",
  );
  assert.equal(new Int32Array(indexed.buffer)[3], -5, "read back as signed it is -5 again");

  // --- an image-based light's defaults and its completeness rule -----------------------------------
  const light = graphics.ImageBasedLighting.DefaultLight();
  assert.deepEqual(
    [light.Irradiance, light.PrefilteredSpecular, light.BrdfLut, light.PrefilteredMipCount,
      light.Intensity],
    [null, null, null, 1, 1],
    "CNA's default light has no textures, one prefiltered mip and unit intensity",
  );
  assert.equal(graphics.ImageBasedLighting.IsLightValid(light), false, "so it is not yet complete");
  assert.equal(
    graphics.ImageBasedLighting.IsLightValid({ ...light, PrefilteredMipCount: 8 }), false,
    "and more mips do not make up for missing textures",
  );
  assert.throws(() => graphics.ImageBasedLighting.IsLightValid(null), TypeError);

  // The positive half needs real textures, so it asks a device for them.
  class LightProbeGame extends Game {
    constructor() {
      super();
      this.manager = new GraphicsDeviceManager(this);
      this.evidence = null;
    }

    LoadContent() {
      const device = this.GraphicsDevice;
      try {
        const irradiance = new Graphics.TextureCube(device, 1, false, Graphics.SurfaceFormat.Color);
        const specular = new Graphics.TextureCube(device, 1, false, Graphics.SurfaceFormat.Color);
        const lut = new Graphics.Texture2D(device, 1, 1);
        const complete = { ...light, Irradiance: irradiance, PrefilteredSpecular: specular, BrdfLut: lut };
        this.evidence = {
          complete: graphics.ImageBasedLighting.IsLightValid(complete),
          noMips: graphics.ImageBasedLighting.IsLightValid({ ...complete, PrefilteredMipCount: 0 }),
          noLut: graphics.ImageBasedLighting.IsLightValid({ ...complete, BrdfLut: null }),
        };
        lut.Dispose();
        specular.Dispose();
        irradiance.Dispose();
      } catch (error) {
        this.evidence = `${error.constructor.name}(${error.cnaResult ?? "-"})`;
      }
      this.Exit();
      super.LoadContent();
    }

    Update(gameTime) {
      this.Exit();
      super.Update(gameTime);
    }
  }
  const game = new LightProbeGame();
  await game.Run();
  const evidence = game.evidence;
  game.Dispose();
  if (typeof evidence === "string") {
    // A renderer without cube storage refuses the textures, not the rule; say which.
    assert.match(evidence, /NotSupported|NativeCapability/, `cube textures were refused: ${evidence}`);
    console.log(`CNA_TS_NATIVE_IMAGE_BASED_LIGHT=DEFAULTS_ONLY CUBE_TEXTURES=${evidence}`);
  } else {
    assert.deepEqual(
      evidence, { complete: true, noMips: false, noLut: false },
      "all three textures and at least one mip is complete; lose either and it is not",
    );
    console.log("CNA_TS_NATIVE_IMAGE_BASED_LIGHT=PASS");
  }
});
