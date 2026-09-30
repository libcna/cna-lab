
import { getBackend } from "../../internal/backend.js";
import type {
  BoundingSphereSnapshot,
  TextureTransformSnapshot,
  ImageBasedLightSnapshot,
  CnaComputeBackend,
  ClusterBoundsSnapshot,
  CnaEffectBackend,
  CnaGraphicsExtensionBackend,
} from "../../internal/backend.js";
import { BoundingBox } from "../../Microsoft/Xna/Framework/BoundingBox.js";
import { BoundingSphere } from "../../Microsoft/Xna/Framework/BoundingSphere.js";
import { Matrix } from "../../Microsoft/Xna/Framework/Matrix.js";
import { Vector2 } from "../../Microsoft/Xna/Framework/Vector2.js";
import { Vector3 } from "../../Microsoft/Xna/Framework/Vector3.js";
import { Vector4 } from "../../Microsoft/Xna/Framework/Vector4.js";
import { NativeUnavailableError } from "../../internal/native-error.js";
import { Color } from "../../Microsoft/Xna/Framework/Color.js";
import type { GraphicsDevice } from "../../Microsoft/Xna/Framework/Graphics/GraphicsDevice.js";
import {
  graphicsDeviceBackendForInternalUse,
  resolveGraphicsDeviceHandleForInternalUse,
} from "../../internal/graphics-device-registry.js";
import { adoptNativeEffectForInternalUse, Effect } from
  "../../Microsoft/Xna/Framework/Graphics/Effect.js";
import { resolveEffectHandleForInternalUse } from
  "../../Microsoft/Xna/Framework/Graphics/Effect.js";
import type { IDisposable } from "../../Microsoft/Xna/Framework/Contracts.js";
import { Texture2D } from "../../Microsoft/Xna/Framework/Graphics/Texture2D.js";
import { resolveTexture2DHandleForInternalUse } from
  "../../Microsoft/Xna/Framework/Graphics/Texture2D.js";
import { TextureCube } from "../../Microsoft/Xna/Framework/Graphics/TextureCube.js";
import { Texture3D } from "../../Microsoft/Xna/Framework/Graphics/Texture3D.js";
import { resolveTexture3DHandleForInternalUse } from
  "../../Microsoft/Xna/Framework/Graphics/Texture3D.js";
import { Rectangle } from "../../Microsoft/Xna/Framework/Rectangle.js";
import {
  resolveTextureCubeHandleForInternalUse,
} from "../../Microsoft/Xna/Framework/Graphics/TextureCube.js";
import type { NativeHandle } from "../../internal/ownership.js";

/** How a PBR material's alpha is interpreted. */
export enum AlphaMode {
  Opaque = 0,
  Mask = 1,
  Blend = 2,
}

function extensions(): CnaGraphicsExtensionBackend {
  const backend = getBackend();
  if (!backend.IsAvailable || backend.GraphicsExtensions == null) {
    throw new NativeUnavailableError(
      `CNA extended graphics requires a loaded backend: ${backend.Detail}`,
    );
  }
  return backend.GraphicsExtensions;
}

function postProcessDeviceHandle(graphicsDevice: GraphicsDevice): NativeHandle {
  if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
  return resolveGraphicsDeviceHandleForInternalUse(graphicsDevice);
}

export { IsGraphicsExtensionLayerAvailable } from "../runtime/index.js";


/* --- renderer capabilities ----------------------------------------------------------------------
 *
 * XNA 4.0 had a two-value GraphicsProfile; CNA answers per capability, so a game asks before it
 * relies on one and a renderer that lacks it says so instead of throwing. A device handle may be
 * borrowed only inside a game lifecycle callback -- the same rule XNA's own graphics resources
 * follow.
 */

/** A capability a renderer either implements or does not, asked before it is used. */
export enum GraphicsCapability {
  ThreeD = 0,
  DepthStencilBuffer = 1,
  MultiSampleAntiAliasing = 2,
  MultipleRenderTargets = 3,
  AnisotropicFiltering = 4,
  WireFrame = 5,
  OcclusionQuery = 6,
  CustomEffects = 7,
  Texture3D = 8,
  MultiStreamVertexInput = 9,
  Instancing = 10,
  StencilBuffer = 11,
  AdditiveBlending = 12,
  CompiledEffects = 13,
  FloatRenderTargets = 14,
  HalfFloatRenderTargets = 15,
  HalfFloatTextureLinearFiltering = 16,
  ComputeShaders = 17,
  IndirectDraw = 18,
}

function compute(): CnaComputeBackend {
  const backend = getBackend();
  if (!backend.IsAvailable || backend.Compute == null) {
    throw new NativeUnavailableError(
      `CNA's compute path requires a loaded backend that has it: ${backend.Detail}`,
    );
  }
  return backend.Compute;
}

function axisIndex(axis: number): number {
  if (!Number.isInteger(axis) || axis < 0 || axis > 2) {
    throw new RangeError("a work-group axis must be 0, 1 or 2");
  }
  return axis;
}

/**
 * What a renderer can do, asked rather than assumed.
 *
 * XNA had `GraphicsProfile`, a two-value tier that stood in for a capability list. CNA answers per
 * capability, and this is that query. It is the honest precondition for everything else in this
 * section: `Supports(device, GraphicsCapability.ComputeShaders)` is `false` on a renderer with no
 * compute, and a game that asks gets to offer a different path instead of catching a refusal.
 */
export const GraphicsDeviceCapabilities = {
  /** Whether the device's renderer implements a capability. */
  Supports(graphicsDevice: GraphicsDevice, capability: GraphicsCapability): boolean {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    if (!Number.isInteger(capability) || capability < 0 || capability > 18) {
      throw new RangeError("capability must be a GraphicsCapability");
    }
    return compute().supportsGraphicsCapability(
      resolveGraphicsDeviceHandleForInternalUse(graphicsDevice), capability,
    );
  },

  /** The largest number of work groups a dispatch may ask for along one axis. */
  MaxComputeWorkGroupCount(graphicsDevice: GraphicsDevice, axis: number): number {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    // Validated before the backend is asked for, so a bad axis is a bad axis whether or not a
    // library is loaded.
    const index = axisIndex(axis);
    return compute().getMaxComputeWorkGroupCount(
      resolveGraphicsDeviceHandleForInternalUse(graphicsDevice), index,
    );
  },

  /** The largest `local_size` a shader may declare along one axis. */
  MaxComputeWorkGroupSize(graphicsDevice: GraphicsDevice, axis: number): number {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    // Validated before the backend is asked for, so a bad axis is a bad axis whether or not a
    // library is loaded.
    const index = axisIndex(axis);
    return compute().getMaxComputeWorkGroupSize(
      resolveGraphicsDeviceHandleForInternalUse(graphicsDevice), index,
    );
  },

  /** The largest number of invocations one work group may contain. */
  MaxComputeWorkGroupInvocations(graphicsDevice: GraphicsDevice): number {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    return compute().getMaxComputeWorkGroupInvocations(
      resolveGraphicsDeviceHandleForInternalUse(graphicsDevice),
    );
  },
} as const;

function vectorSnapshot(vector: Vector3, what: string): { X: number; Y: number; Z: number } {
  if (vector == null) throw new TypeError(`${what} is required`);
  return { X: vector.X, Y: vector.Y, Z: vector.Z };
}

function toVector3(snapshot: { readonly X: number; readonly Y: number; readonly Z: number }): Vector3 {
  return new Vector3(snapshot.X, snapshot.Y, snapshot.Z);
}

/** The sixteen numbers CNA reads a matrix from, in XNA's own row order. */
function matrixValues(matrix: Matrix, what: string): number[] {
  if (matrix == null) throw new TypeError(`${what} is required`);
  return [
    matrix.M11, matrix.M12, matrix.M13, matrix.M14,
    matrix.M21, matrix.M22, matrix.M23, matrix.M24,
    matrix.M31, matrix.M32, matrix.M33, matrix.M34,
    matrix.M41, matrix.M42, matrix.M43, matrix.M44,
  ];
}

function toMatrix(values: readonly number[]): Matrix {
  return new Matrix(
    values[0]!, values[1]!, values[2]!, values[3]!,
    values[4]!, values[5]!, values[6]!, values[7]!,
    values[8]!, values[9]!, values[10]!, values[11]!,
    values[12]!, values[13]!, values[14]!, values[15]!,
  );
}


function effectBackendFor(device: GraphicsDevice): CnaEffectBackend {
  const backend = graphicsDeviceBackendForInternalUse(device).Effects;
  if (backend == null) {
    throw new NativeUnavailableError("a shadow caster effect needs the CNA Effect backend");
  }
  return backend;
}

function boundsSnapshot(bounds: BoundingBox): ClusterBoundsSnapshot {
  if (bounds == null) throw new TypeError("bounds is required");
  return {
    Min: vectorSnapshot(bounds.Min, "bounds.Min"),
    Max: vectorSnapshot(bounds.Max, "bounds.Max"),
  };
}


function assertPositiveCounts(counts: Readonly<Record<string, number>>): void {
  for (const [name, value] of Object.entries(counts)) {
    if (!Number.isInteger(value) || value <= 0) {
      throw new RangeError(`${name} must be a positive integer`);
    }
  }
}

/** Whether the ASCII effect keeps the scene's colour or reduces it to black and white. */
export enum AsciiQuantizeMode {
  BlackWhite = 0,
  Color = 1,
}

/** The shadow-mask pattern a CRT effect lays over the image. */
export enum CrtMaskType {
  None = 0,
  ApertureGrille = 1,
  ShadowMask = 2,
}

/** The colour depth a depth effect reduces the image to. */
export enum DepthEffectMode {
  Color16Bit = 0,
  Color8Bit = 1,
  Grayscale4Bit = 2,
  Grayscale2Bit = 3,
  Grayscale1Bit = 4,
  Palette256 = 5,
  Palette16 = 6,
}

/** The ordered pattern a depth effect dithers with, or none. */
export enum DitherMode {
  None = 0,
  Bayer4X4 = 1,
  Bayer8X8 = 2,
}

/**
 * CNA's ASCII post-process effect.
 *
 * It reduces the frame to a grid of cells and draws a character per cell. The grid follows from the
 * cell size and the destination rectangle, and {@link LastGridDimensions} reports what the last
 * draw actually used — which is the only way to know, because the rectangle decides it.
 */
export class AsciiPostProcessEffect implements IDisposable {
  #handle: NativeHandle | null;

  public constructor(graphicsDevice: GraphicsDevice) {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    this.#handle = extensions().createAsciiEffect(
      resolveGraphicsDeviceHandleForInternalUse(graphicsDevice),
    );
  }

  /** Whether the effect has been released. */
  public get IsDisposed(): boolean { return this.#handle == null; }

  #active(): NativeHandle {
    if (this.#handle == null) throw new NativeUnavailableError("the ASCII effect is disposed");
    return this.#handle;
  }

  /** How many texels of the source one character covers. */
  public get CellSize(): { readonly Width: number; readonly Height: number } {
    const size = extensions().getAsciiCellSize(this.#active());
    return Object.freeze({ Width: size.Width, Height: size.Height });
  }

  /** Sets it. A cell of one texel is a character per texel, which is a very large grid. */
  public SetCellSize(width: number, height: number): void {
    assertPositiveCounts({ width, height });
    extensions().setAsciiCellSize(this.#active(), width, height);
  }

  /** Whether the characters keep the scene's colour. */
  public get QuantizeMode(): AsciiQuantizeMode {
    return extensions().getAsciiQuantizeMode(this.#active()) as AsciiQuantizeMode;
  }
  public set QuantizeMode(value: AsciiQuantizeMode) {
    if (!Number.isInteger(value)) throw new TypeError("QuantizeMode must be an AsciiQuantizeMode");
    extensions().setAsciiQuantizeMode(this.#active(), value);
  }

  /** The grid the last {@link Draw} used: the destination divided by the cell size. */
  public get LastGridDimensions(): { readonly Columns: number; readonly Rows: number } {
    const size = extensions().getAsciiLastGridDimensions(this.#active());
    return Object.freeze({ Columns: size.Width, Rows: size.Height });
  }

  /** Draws the source as characters into a rectangle of whatever target is bound. */
  public Draw(source: Texture2D, destination: Rectangle): void {
    if (source == null) throw new TypeError("source is required");
    if (destination == null) throw new TypeError("destination is required");
    extensions().drawAsciiEffect(
      this.#active(), resolveTexture2DHandleForInternalUse(source),
      {
        X: Math.trunc(destination.X), Y: Math.trunc(destination.Y),
        Width: Math.trunc(destination.Width), Height: Math.trunc(destination.Height),
      },
    );
  }

  /** Releases the handle. Disposing twice is harmless. */
  public Dispose(): void {
    const handle = this.#handle;
    if (handle == null) return;
    this.#handle = null;
    extensions().destroyAsciiEffect(handle);
  }
}

/**
 * A CRT: scanlines, a curved glass, a vignette and a shadow mask.
 *
 * An ordinary `Effect`, so it goes wherever an effect goes -- a `SpriteBatch` begin, for one.
 */
export class CrtEffect {
  private constructor() { /* created through Create */ }

  /** Makes one, as a real `Effect` the caller owns and disposes. */
  public static Create(graphicsDevice: GraphicsDevice): Effect {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    return adoptNativeEffectForInternalUse(
      graphicsDevice, effectBackendFor(graphicsDevice),
      extensions().createCrtEffect(resolveGraphicsDeviceHandleForInternalUse(graphicsDevice)),
    );
  }

  /** How dark the scanlines are. */
  public static GetScanlineIntensity(effect: Effect): number {
    return extensions().getCrtScanlineIntensity(resolveEffectHandleForInternalUse(effect));
  }
  public static SetScanlineIntensity(effect: Effect, value: number): void {
    extensions().setCrtScanlineIntensity(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"),
    );
  }

  /** How much the glass bulges. */
  public static GetCurvature(effect: Effect): number {
    return extensions().getCrtCurvature(resolveEffectHandleForInternalUse(effect));
  }
  public static SetCurvature(effect: Effect, value: number): void {
    extensions().setCrtCurvature(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"),
    );
  }

  /** How dark the corners go. */
  public static GetVignetteIntensity(effect: Effect): number {
    return extensions().getCrtVignetteIntensity(resolveEffectHandleForInternalUse(effect));
  }
  public static SetVignetteIntensity(effect: Effect, value: number): void {
    extensions().setCrtVignetteIntensity(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"),
    );
  }

  /** How strongly the shadow mask shows. */
  public static GetMaskIntensity(effect: Effect): number {
    return extensions().getCrtMaskIntensity(resolveEffectHandleForInternalUse(effect));
  }
  public static SetMaskIntensity(effect: Effect, value: number): void {
    extensions().setCrtMaskIntensity(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"),
    );
  }

  /** Which mask pattern it is. */
  public static GetMaskType(effect: Effect): CrtMaskType {
    return extensions().getCrtMaskType(resolveEffectHandleForInternalUse(effect)) as CrtMaskType;
  }
  public static SetMaskType(effect: Effect, value: CrtMaskType): void {
    if (!Number.isInteger(value)) throw new TypeError("value must be a CrtMaskType");
    extensions().setCrtMaskType(resolveEffectHandleForInternalUse(effect), value);
  }
}

/**
 * Reduces the frame's colour depth, with or without an ordered dither.
 *
 * The other half of the retro pair with {@link CrtEffect}, and an `Effect` on the same terms.
 */
export class DepthEffect {
  private constructor() { /* created through Create */ }

  /** Makes one, as a real `Effect` the caller owns and disposes. */
  public static Create(graphicsDevice: GraphicsDevice): Effect {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    return adoptNativeEffectForInternalUse(
      graphicsDevice, effectBackendFor(graphicsDevice),
      extensions().createDepthEffect(resolveGraphicsDeviceHandleForInternalUse(graphicsDevice)),
    );
  }

  /** Which colour depth the frame is reduced to. */
  public static GetMode(effect: Effect): DepthEffectMode {
    return extensions().getDepthEffectMode(
      resolveEffectHandleForInternalUse(effect),
    ) as DepthEffectMode;
  }
  public static SetMode(effect: Effect, mode: DepthEffectMode): void {
    if (!Number.isInteger(mode)) throw new TypeError("mode must be a DepthEffectMode");
    extensions().setDepthEffectMode(resolveEffectHandleForInternalUse(effect), mode);
  }

  /** Which ordered pattern the reduction dithers with, or none. */
  public static GetDitherMode(effect: Effect): DitherMode {
    return extensions().getDepthEffectDitherMode(
      resolveEffectHandleForInternalUse(effect),
    ) as DitherMode;
  }
  public static SetDitherMode(effect: Effect, mode: DitherMode): void {
    if (!Number.isInteger(mode)) throw new TypeError("mode must be a DitherMode");
    extensions().setDepthEffectDitherMode(resolveEffectHandleForInternalUse(effect), mode);
  }
}

function finite(value: number, what: string): number {
  if (typeof value !== "number" || !Number.isFinite(value)) {
    throw new TypeError(`${what} must be a finite number`);
  }
  return value;
}

function wholeNumber(value: number, what: string): number {
  if (!Number.isInteger(value)) throw new TypeError(`${what} must be an integer`);
  return value;
}

/* ================================================================================================
 * Physically-based materials, their glTF extensions, and the effects that carry them
 * ==============================================================================================*/

/**
 * One texture slot on a PBR material, in the order CNA numbers them.
 *
 * The last two are `KHR_materials_specular`, which is why they are the two the frozen
 * {@link PbrMaterial} has no room for.
 */
export enum PbrTextureSlot {
  BaseColor = 0,
  Normal = 1,
  MetallicRoughness = 2,
  Emissive = 3,
  Occlusion = 4,
  Specular = 5,
  SpecularColor = 6,
}

/**
 * A `KHR_texture_transform`: the selected UV is scaled, then rotated, then translated.
 *
 * Independent of which packed UV channel the slot samples — that is the coordinate set.
 */
export interface TextureTransform {
  /** Translation applied last. */
  Offset: Vector2;
  /** Per-axis scale applied first. */
  Scale: Vector2;
  /** Counter-clockwise rotation in radians, applied between the two. */
  Rotation: number;
}

function textureHandle(texture: Texture2D | null | undefined): NativeHandle {
  return texture == null ? 0n : resolveTexture2DHandleForInternalUse(texture);
}

function transformSnapshot(value: TextureTransform, what: string): TextureTransformSnapshot {
  if (value == null) throw new TypeError(`${what} is required`);
  return {
    Offset: { X: finite(value.Offset?.X, `${what}.Offset.X`), Y: finite(value.Offset?.Y, `${what}.Offset.Y`) },
    Scale: { X: finite(value.Scale?.X, `${what}.Scale.X`), Y: finite(value.Scale?.Y, `${what}.Scale.Y`) },
    Rotation: finite(value.Rotation, `${what}.Rotation`),
  };
}

function toTransform(value: TextureTransformSnapshot): TextureTransform {
  return {
    Offset: new Vector2(value.Offset.X, value.Offset.Y),
    Scale: new Vector2(value.Scale.X, value.Scale.Y),
    Rotation: value.Rotation,
  };
}

/** A neutral texture transform: no offset, unit scale, no rotation — whatever CNA calls neutral. */
export function CreateTextureTransform(): TextureTransform {
  return toTransform(extensions().getDefaultTextureTransform());
}

/**
 * The physically-based effect, as a real `Effect` the caller owns.
 *
 * Statics rather than a wrapper class, the way {@link CrtEffect} is: what CNA hands back is an
 * ordinary effect handle that every effect route already accepts, so wrapping it would take that
 * away. A whole material goes on with {@link ApplyMaterial} and comes back with
 * {@link ExtractMaterial}; the individual accessors reach the same state one field at a time.
 */
export class PbrEffect {
  private constructor() { /* created through Create */ }

  /** Makes one, as a real `Effect` the caller owns and disposes. */
  public static Create(graphicsDevice: GraphicsDevice): Effect {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    return adoptNativeEffectForInternalUse(
      graphicsDevice, effectBackendFor(graphicsDevice),
      extensions().createPbrEffect(resolveGraphicsDeviceHandleForInternalUse(graphicsDevice)),
    );
  }

  /** The base colour, as a linear vector rather than the material's eight-bit colour. */
  public static GetDiffuseColor(effect: Effect): Vector3 {
    return toVector3(extensions().getPbrEffectDiffuseColor(resolveEffectHandleForInternalUse(effect)));
  }
  public static SetDiffuseColor(effect: Effect, value: Vector3): void {
    extensions().setPbrEffectDiffuseColor(
      resolveEffectHandleForInternalUse(effect), vectorSnapshot(value, "value"));
  }

  /** The material's opacity. */
  public static GetAlpha(effect: Effect): number {
    return extensions().getPbrEffectAlpha(resolveEffectHandleForInternalUse(effect));
  }
  public static SetAlpha(effect: Effect, value: number): void {
    extensions().setPbrEffectAlpha(resolveEffectHandleForInternalUse(effect), finite(value, "value"));
  }

  /** How metallic the surface is. */
  public static GetMetallicFactor(effect: Effect): number {
    return extensions().getPbrEffectMetallicFactor(resolveEffectHandleForInternalUse(effect));
  }
  public static SetMetallicFactor(effect: Effect, value: number): void {
    extensions().setPbrEffectMetallicFactor(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"));
  }

  /** How rough it is. */
  public static GetRoughnessFactor(effect: Effect): number {
    return extensions().getPbrEffectRoughnessFactor(resolveEffectHandleForInternalUse(effect));
  }
  public static SetRoughnessFactor(effect: Effect, value: number): void {
    extensions().setPbrEffectRoughnessFactor(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"));
  }

  /** What the surface emits. */
  public static GetEmissiveFactor(effect: Effect): Vector3 {
    return toVector3(
      extensions().getPbrEffectEmissiveFactor(resolveEffectHandleForInternalUse(effect)));
  }
  public static SetEmissiveFactor(effect: Effect, value: Vector3): void {
    extensions().setPbrEffectEmissiveFactor(
      resolveEffectHandleForInternalUse(effect), vectorSnapshot(value, "value"));
  }

  /** `KHR_materials_ior`. */
  public static GetIor(effect: Effect): number {
    return extensions().getPbrEffectIor(resolveEffectHandleForInternalUse(effect));
  }
  public static SetIor(effect: Effect, value: number): void {
    extensions().setPbrEffectIor(resolveEffectHandleForInternalUse(effect), finite(value, "value"));
  }

  /** `KHR_materials_specular` strength. */
  public static GetSpecularFactor(effect: Effect): number {
    return extensions().getPbrEffectSpecularFactor(resolveEffectHandleForInternalUse(effect));
  }
  public static SetSpecularFactor(effect: Effect, value: number): void {
    extensions().setPbrEffectSpecularFactor(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"));
  }

  /** `KHR_materials_specular` colour. */
  public static GetSpecularColorFactor(effect: Effect): Vector3 {
    return toVector3(
      extensions().getPbrEffectSpecularColorFactor(resolveEffectHandleForInternalUse(effect)));
  }
  public static SetSpecularColorFactor(effect: Effect, value: Vector3): void {
    extensions().setPbrEffectSpecularColorFactor(
      resolveEffectHandleForInternalUse(effect), vectorSnapshot(value, "value"));
  }

  /** How strongly the normal map is applied. */
  public static GetNormalScale(effect: Effect): number {
    return extensions().getPbrEffectNormalScale(resolveEffectHandleForInternalUse(effect));
  }
  public static SetNormalScale(effect: Effect, value: number): void {
    extensions().setPbrEffectNormalScale(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"));
  }

  /** How strongly ambient occlusion is applied. */
  public static GetOcclusionStrength(effect: Effect): number {
    return extensions().getPbrEffectOcclusionStrength(resolveEffectHandleForInternalUse(effect));
  }
  public static SetOcclusionStrength(effect: Effect, value: number): void {
    extensions().setPbrEffectOcclusionStrength(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"));
  }

  /** How the material's alpha is read. */
  public static GetAlphaMode(effect: Effect): AlphaMode {
    return extensions().getPbrEffectAlphaMode(
      resolveEffectHandleForInternalUse(effect)) as AlphaMode;
  }
  public static SetAlphaMode(effect: Effect, value: AlphaMode): void {
    extensions().setPbrEffectAlphaMode(
      resolveEffectHandleForInternalUse(effect), wholeNumber(value, "value"));
  }

  /** The alpha below which a masked material is discarded. */
  public static GetAlphaCutoff(effect: Effect): number {
    return extensions().getPbrEffectAlphaCutoff(resolveEffectHandleForInternalUse(effect));
  }
  public static SetAlphaCutoff(effect: Effect, value: number): void {
    extensions().setPbrEffectAlphaCutoff(
      resolveEffectHandleForInternalUse(effect), finite(value, "value"));
  }

  /** Whether both faces are drawn. */
  public static GetDoubleSided(effect: Effect): boolean {
    return extensions().getPbrEffectDoubleSided(resolveEffectHandleForInternalUse(effect));
  }
  public static SetDoubleSided(effect: Effect, value: boolean): void {
    extensions().setPbrEffectDoubleSided(resolveEffectHandleForInternalUse(effect), Boolean(value));
  }

  /** Whether the lit result is encoded back to sRGB. */
  public static GetEncodeOutputToSrgb(effect: Effect): boolean {
    return extensions().getPbrEffectEncodeOutputToSrgb(resolveEffectHandleForInternalUse(effect));
  }
  public static SetEncodeOutputToSrgb(effect: Effect, value: boolean): void {
    extensions().setPbrEffectEncodeOutputToSrgb(
      resolveEffectHandleForInternalUse(effect), Boolean(value));
  }

  /** Whether a vertex colour attribute multiplies the base colour. */
  public static GetVertexColorEnabled(effect: Effect): boolean {
    return extensions().getPbrEffectVertexColorEnabled(resolveEffectHandleForInternalUse(effect));
  }
  public static SetVertexColorEnabled(effect: Effect, value: boolean): void {
    extensions().setPbrEffectVertexColorEnabled(
      resolveEffectHandleForInternalUse(effect), Boolean(value));
  }

  /** The handle in one slot, or `0n` when the slot is empty. The effect does not own it. */
  public static GetTexture(effect: Effect, slot: PbrTextureSlot): NativeHandle {
    return extensions().getPbrEffectTexture(
      resolveEffectHandleForInternalUse(effect), wholeNumber(slot, "slot"));
  }
  public static SetTexture(effect: Effect, slot: PbrTextureSlot, texture: Texture2D | null): void {
    extensions().setPbrEffectTexture(
      resolveEffectHandleForInternalUse(effect), wholeNumber(slot, "slot"), textureHandle(texture));
  }

  /** Which packed vertex UV channel a slot samples. */
  public static GetTextureCoordinateSet(effect: Effect, slot: PbrTextureSlot): number {
    return extensions().getPbrEffectTextureCoordinateSet(
      resolveEffectHandleForInternalUse(effect), wholeNumber(slot, "slot"));
  }
  public static SetTextureCoordinateSet(
    effect: Effect, slot: PbrTextureSlot, value: number,
  ): void {
    extensions().setPbrEffectTextureCoordinateSet(
      resolveEffectHandleForInternalUse(effect), wholeNumber(slot, "slot"),
      wholeNumber(value, "value"));
  }

  /** A slot's `KHR_texture_transform`. */
  public static GetTextureTransform(effect: Effect, slot: PbrTextureSlot): TextureTransform {
    return toTransform(extensions().getPbrEffectTextureTransform(
      resolveEffectHandleForInternalUse(effect), wholeNumber(slot, "slot")));
  }
  public static SetTextureTransform(
    effect: Effect, slot: PbrTextureSlot, transform: TextureTransform,
  ): void {
    extensions().setPbrEffectTextureTransform(
      resolveEffectHandleForInternalUse(effect), wholeNumber(slot, "slot"),
      transformSnapshot(transform, "transform"));
  }

  /** Whether a slot's samples are sRGB-encoded. */
  public static GetTextureIsSrgb(effect: Effect, slot: PbrTextureSlot): boolean {
    return extensions().getPbrEffectTextureIsSrgb(
      resolveEffectHandleForInternalUse(effect), wholeNumber(slot, "slot"));
  }
  public static SetTextureIsSrgb(effect: Effect, slot: PbrTextureSlot, value: boolean): void {
    extensions().setPbrEffectTextureIsSrgb(
      resolveEffectHandleForInternalUse(effect), wholeNumber(slot, "slot"), Boolean(value));
  }
}

/** The same effect with a skinning skeleton behind it. Every {@link PbrEffect} static applies. */
export class SkinnedPbrEffect {
  private constructor() { /* created through Create */ }

  /** Makes one, as a real `Effect` the caller owns and disposes. */
  public static Create(graphicsDevice: GraphicsDevice): Effect {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    return adoptNativeEffectForInternalUse(
      graphicsDevice, effectBackendFor(graphicsDevice),
      extensions().createSkinnedPbrEffect(
        resolveGraphicsDeviceHandleForInternalUse(graphicsDevice)),
    );
  }

  /** How many bones influence each vertex. */
  public static GetWeightsPerVertex(effect: Effect): number {
    return extensions().getSkinnedPbrEffectWeightsPerVertex(
      resolveEffectHandleForInternalUse(effect));
  }
  public static SetWeightsPerVertex(effect: Effect, value: number): void {
    extensions().setSkinnedPbrEffectWeightsPerVertex(
      resolveEffectHandleForInternalUse(effect), wholeNumber(value, "value"));
  }

  /** Puts the skeleton's bone transforms on the effect. */
  public static SetBoneTransforms(effect: Effect, transforms: readonly Matrix[]): void {
    if (!Array.isArray(transforms)) throw new TypeError("transforms must be an array of matrices");
    extensions().setSkinnedPbrEffectBoneTransforms(
      resolveEffectHandleForInternalUse(effect),
      transforms.map((matrix, index) => matrixValues(matrix, `transforms[${index}]`)));
  }

  /** Reads back the first `count` of them. */
  public static GetBoneTransforms(effect: Effect, count: number): Matrix[] {
    return extensions().getSkinnedPbrEffectBoneTransforms(
      resolveEffectHandleForInternalUse(effect), wholeNumber(count, "count"),
    ).map((values) => toMatrix(values));
  }
}

/* ================================================================================================
 * Culling, and drawing many copies of one thing
 * ==============================================================================================*/

function sphereSnapshot(sphere: BoundingSphere, what: string): BoundingSphereSnapshot {
  if (sphere == null) throw new TypeError(`${what} is required`);
  return {
    Center: vectorSnapshot(sphere.Center, `${what}.Center`),
    Radius: finite(sphere.Radius, `${what}.Radius`),
  };
}


/* ================================================================================================
 * The debug drawer
 * ==============================================================================================*/

/** One vertex of the line list a debug drawer builds. */
export interface DebugVertex {
  /** Where it is, in world space. */
  readonly Position: Vector3;
  /** What colour it is. */
  readonly Color: Color;
}

/**
 * Lines drawn over a scene to show what is going on in it.
 *
 * Everything it draws is a line list, and {@link GetVertices} hands the whole list back — so what a
 * gizmo actually consists of can be *read* rather than looked at, which is why nothing here needs a
 * renderer to be checked. A box is twelve edges at eight corners; a cross is three lines through a
 * point; a frustum's eight corners are the ones `BoundingFrustum.GetCorners` gives for the same
 * matrix.
 *
 * The depth-tested and overlay lists are separate. {@link DepthTested} chooses which one an added
 * shape lands in, and {@link GetVertices} reads either.
 */
export class DebugDraw implements IDisposable {
  #handle: NativeHandle | null;

  public constructor(graphicsDevice: GraphicsDevice) {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    this.#handle = extensions().createDebugDraw(
      resolveGraphicsDeviceHandleForInternalUse(graphicsDevice));
  }

  #active(): NativeHandle {
    if (this.#handle == null) throw new NativeUnavailableError("the debug drawer is disposed");
    return this.#handle;
  }

  /** Whether it has been released. */
  public get IsDisposed(): boolean { return this.#handle == null; }

  /** Releases it. Harmless twice. */
  public Dispose(): void {
    const handle = this.#handle;
    if (handle == null) return;
    this.#handle = null;
    extensions().destroyDebugDraw(handle);
  }

  /** Whether shapes added from now on go in the depth-tested list rather than the overlay. */
  public get DepthTested(): boolean {
    return extensions().isDebugDrawDepthTested(this.#active());
  }
  public set DepthTested(value: boolean) {
    extensions().setDebugDrawDepthTested(this.#active(), Boolean(value));
  }

  /** How many lines are queued across both lists. */
  public get LineCount(): number {
    return extensions().getDebugDrawLineCount(this.#active());
  }

  /** Starts a frame's worth of drawing, against a camera. */
  public Begin(view: Matrix, projection: Matrix): void {
    extensions().beginDebugDraw(
      this.#active(), matrixValues(view, "view"), matrixValues(projection, "projection"));
  }

  /** Draws everything queued and empties the queue. */
  public End(): void { extensions().endDebugDraw(this.#active()); }

  /** Empties the queue without drawing it. */
  public Clear(): void { extensions().clearDebugDraw(this.#active()); }

  /** One line. */
  public AddLine(from: Vector3, to: Vector3, color: Color): void {
    extensions().addDebugDrawLine(
      this.#active(), vectorSnapshot(from, "from"), vectorSnapshot(to, "to"),
      packedColor(color, "color"));
  }

  /** A box's twelve edges. */
  public AddBox(bounds: BoundingBox, color: Color): void {
    extensions().addDebugDrawBox(this.#active(), boundsSnapshot(bounds), packedColor(color, "color"));
  }

  /** A sphere, as three rings of `segments` sides each. */
  public AddSphere(centre: Vector3, radius: number, color: Color, segments: number): void {
    extensions().addDebugDrawSphere(
      this.#active(), vectorSnapshot(centre, "centre"), finite(radius, "radius"),
      packedColor(color, "color"), wholeNumber(segments, "segments"));
  }

  /** The same, from a `BoundingSphere`. */
  public AddBoundingSphere(sphere: BoundingSphere, color: Color, segments: number): void {
    extensions().addDebugDrawBoundingSphere(
      this.#active(), sphereSnapshot(sphere, "sphere"), packedColor(color, "color"),
      wholeNumber(segments, "segments"));
  }

  /** A frustum's twelve edges, from the view-projection a `BoundingFrustum` holds. */
  public AddFrustum(viewProjection: Matrix, color: Color): void {
    extensions().addDebugDrawFrustum(
      this.#active(), matrixValues(viewProjection, "viewProjection"), packedColor(color, "color"));
  }

  /** Three lines through a point, one along each axis, `size` long in each direction. */
  public AddCross(position: Vector3, size: number, color: Color): void {
    extensions().addDebugDrawCross(
      this.#active(), vectorSnapshot(position, "position"), finite(size, "size"),
      packedColor(color, "color"));
  }

  /**
   * The line list itself, two vertices per line, in the order the shapes were added.
   *
   * This is what makes the drawer checkable without drawing anything: a caller — or a test — can
   * read exactly which lines a gizmo turned into.
   */
  public GetVertices(depthTested: boolean): DebugVertex[] {
    return extensions().getDebugDrawVertices(this.#active(), Boolean(depthTested))
      .map((vertex) => {
        const color = new Color(0, 0, 0, 0);
        color.PackedValue = vertex.Color;
        return { Position: toVector3(vertex.Position), Color: color };
      });
  }
}

function packedColor(color: Color, what: string): number {
  if (color == null) throw new TypeError(`${what} is required`);
  return color.PackedValue;
}

/**
 * The adoption channel `Effect` keeps for a handle CNA already made.
 *
 * `ShaderEffect` is an `Effect` — that is the whole point of it, because `SpriteBatch.Begin` and
 * `GraphicsDevice`'s draw calls take an `Effect` and a custom shader is only useful if it can go
 * where the stock ones go. The cast reaches the same implementation-only constructor
 * {@link adoptNativeEffectForInternalUse} uses; the public overloads are unchanged.
 */
const AdoptedEffect = Effect as unknown as new (
  graphicsDevice: GraphicsDevice,
  effectCode: undefined,
  adoptedDescription: undefined,
  adoptedNative: { readonly Backend: CnaEffectBackend; readonly Handle: NativeHandle },
) => Effect;

function effectsBackendFor(device: GraphicsDevice): CnaEffectBackend {
  const backend = graphicsDeviceBackendForInternalUse(device).Effects;
  if (backend == null) throw new NativeUnavailableError("CNA effect routes are unavailable");
  return backend;
}

/**
 * An `Effect` built from GLSL source the game wrote itself.
 *
 * Everywhere a stock effect goes this goes: `SpriteBatch.Begin(…, shaderEffect)` and the device's
 * own draw calls both take it, because it *is* an `Effect`. {@link World}, {@link View} and
 * {@link Projection} are the same three matrices every stock effect exposes and are forwarded to
 * the compiled program under those names, so a shader written against an original XNA sample's
 * uniform names works unchanged.
 *
 * **Constructing one does not mean it compiled.** CNA is explicit that success here means the
 * object exists, and that whether a renderer compiles at construction — or looks at the source at
 * all — is renderer-specific and deliberately not normalised. Ask {@link IsEffectValid}, and read
 * {@link CompileError} for why when it says no. A failed compile is not an exception, because on
 * several renderers `GraphicsCapability.CustomEffects` is true while GLSL is never compiled at all
 * (HEADLESS and SOFTWARE accept and ignore it; Vulkan wants SPIR-V), and throwing would turn a
 * documented capability boundary into a crash.
 *
 * The one thing settled everywhere is that two empty sources are refused.
 */
export class ShaderEffect extends AdoptedEffect {
  public constructor(
    graphicsDevice: GraphicsDevice, vertexSource: string, fragmentSource: string,
  ) {
    if (graphicsDevice == null) throw new TypeError("graphicsDevice is required");
    if (typeof vertexSource !== "string" || typeof fragmentSource !== "string") {
      throw new TypeError("both shader sources must be strings");
    }
    const backend = effectsBackendFor(graphicsDevice);
    const handle = extensions().createShaderEffect(
      postProcessDeviceHandle(graphicsDevice), vertexSource, fragmentSource);
    super(graphicsDevice, undefined, undefined, { Backend: backend, Handle: handle });
  }

  #handle(): NativeHandle { return resolveEffectHandleForInternalUse(this); }

  /** Whether the renderer compiled and linked the program. */
  public get IsEffectValid(): boolean {
    return extensions().isShaderEffectValid(this.#handle());
  }

  /** Whether the native compiled-program renderer behind it is still alive. */
  public get HasRenderer(): boolean {
    return extensions().shaderEffectHasRenderer(this.#handle());
  }

  /**
   * The compiler's log from a failed compile, or `""` when it linked — or when the renderer keeps
   * no log, which is a different thing from having nothing to say. {@link IsEffectValid} says
   * *whether*; this says *why*.
   */
  public get CompileError(): string {
    return extensions().getShaderEffectCompileError(this.#handle());
  }

  /** The world matrix, forwarded to the program as `World`. */
  public get World(): Matrix { return toMatrix(extensions().getShaderEffectWorld(this.#handle())); }
  public set World(value: Matrix) {
    extensions().setShaderEffectWorld(this.#handle(), matrixValues(value, "World"));
  }

  /** The view matrix, forwarded to the program as `View`. */
  public get View(): Matrix { return toMatrix(extensions().getShaderEffectView(this.#handle())); }
  public set View(value: Matrix) {
    extensions().setShaderEffectView(this.#handle(), matrixValues(value, "View"));
  }

  /** The projection matrix, forwarded to the program as `Projection`. */
  public get Projection(): Matrix {
    return toMatrix(extensions().getShaderEffectProjection(this.#handle()));
  }
  public set Projection(value: Matrix) {
    extensions().setShaderEffectProjection(this.#handle(), matrixValues(value, "Projection"));
  }

  /** Sets a `mat4` uniform. One matrix, whatever size the uniform declares — see {@link SetUniformMat4Array}. */
  public SetUniformMat4(name: string, value: Matrix): void {
    extensions().setShaderEffectUniformMatrix(
      this.#handle(), uniformName(name), matrixValues(value, "value"));
  }

  /** Sets a `vec4` uniform. */
  public SetUniformVec4(name: string, value: Vector4): void {
    if (value == null) throw new TypeError("value is required");
    extensions().setShaderEffectUniformVector4(this.#handle(), uniformName(name), {
      X: finite(value.X, "value.X"), Y: finite(value.Y, "value.Y"),
      Z: finite(value.Z, "value.Z"), W: finite(value.W, "value.W"),
    });
  }

  /** Sets a `vec3` uniform. */
  public SetUniformVec3(name: string, value: Vector3): void {
    extensions().setShaderEffectUniformVector3(
      this.#handle(), uniformName(name), vectorSnapshot(value, "value"));
  }

  /** Sets a `vec2` uniform. */
  public SetUniformVec2(name: string, value: Vector2): void {
    if (value == null) throw new TypeError("value is required");
    extensions().setShaderEffectUniformVector2(this.#handle(), uniformName(name), {
      X: finite(value.X, "value.X"), Y: finite(value.Y, "value.Y"),
    });
  }

  /** Sets a scalar `float` uniform. */
  public SetUniformFloat(name: string, value: number): void {
    extensions().setShaderEffectUniformFloat(this.#handle(), uniformName(name), finite(value, "value"));
  }

  /** Sets a scalar `int` uniform — which is also how a sampler is pointed at a unit. */
  public SetUniformInt(name: string, value: number): void {
    extensions().setShaderEffectUniformInt32(
      this.#handle(), uniformName(name), wholeNumber(value, "value"));
  }

  /** Sets a `float[]` uniform. The count is the number of scalars. */
  public SetUniformFloatArray(name: string, values: readonly number[]): void {
    extensions().setShaderEffectUniformFloatArray(
      this.#handle(), uniformName(name), numberList(values, "values"));
  }

  /** Sets a `vec2[]` uniform. */
  public SetUniformVec2Array(name: string, values: readonly Vector2[]): void {
    if (!Array.isArray(values)) throw new TypeError("values must be an array");
    extensions().setShaderEffectUniformVector2Array(
      this.#handle(), uniformName(name),
      values.map((value, index) => {
        if (value == null) throw new TypeError(`values[${index}] is required`);
        return { X: finite(value.X, `values[${index}].X`), Y: finite(value.Y, `values[${index}].Y`) };
      }));
  }

  /**
   * Sets a `vec3[]` uniform, from three floats per element.
   *
   * Separate from {@link SetUniformFloatArray} for a reason worth stating: GL rejects filling a
   * `vec3[]` from a float array as a type mismatch, leaves the uniform at its default, and reports
   * nothing the caller sees.
   */
  public SetUniformVec3Array(name: string, values: readonly Vector3[]): void {
    if (!Array.isArray(values)) throw new TypeError("values must be an array");
    const flat: number[] = [];
    for (const [index, value] of values.entries()) {
      if (value == null) throw new TypeError(`values[${index}] is required`);
      flat.push(
        finite(value.X, `values[${index}].X`), finite(value.Y, `values[${index}].Y`),
        finite(value.Z, `values[${index}].Z`));
    }
    extensions().setShaderEffectUniformVec3Array(this.#handle(), uniformName(name), flat);
  }

  /**
   * Sets a `mat4[]` uniform — a skinning palette, typically.
   *
   * Separate from {@link SetUniformMat4}, which uploads exactly one matrix however large the
   * uniform is: filling a palette with it leaves every element past the first at its default. The
   * `name[0]` spelling GLSL uses for the first element is tried too, so either form works.
   */
  public SetUniformMat4Array(name: string, values: readonly Matrix[]): void {
    if (!Array.isArray(values)) throw new TypeError("values must be an array");
    const flat: number[] = [];
    for (const [index, value] of values.entries()) {
      flat.push(...matrixValues(value, `values[${index}]`));
    }
    extensions().setShaderEffectUniformMat4Array(this.#handle(), uniformName(name), flat);
  }

  /**
   * Declares the std140 uniform block this effect's parameters live in.
   *
   * Required on a renderer whose shading dialect has no loose uniforms — every SPIR-V target — and
   * harmlessly ignored everywhere else, so the call can sit unconditionally beside the
   * construction. An empty `names` clears any previous declaration.
   */
  public DeclareUniformBlock(
    blockSizeBytes: number, names: readonly string[], offsets: readonly number[],
  ): void {
    if (!Array.isArray(names) || !Array.isArray(offsets)) {
      throw new TypeError("names and offsets must be arrays");
    }
    if (names.length !== offsets.length) {
      throw new TypeError("names and offsets must be the same length");
    }
    extensions().declareShaderEffectUniformBlock(
      this.#handle(), wholeNumber(blockSizeBytes, "blockSizeBytes"),
      names.map((value, index) => uniformName(value, `names[${index}]`)),
      offsets.map((value, index) => wholeNumber(value, `offsets[${index}]`)));
  }

  /**
   * Binds a texture to one of this effect's sampler units.
   *
   * Unit 0 is normally driven by the caller — `SpriteBatch` binds its own texture there — so this
   * is for the extra units a custom shader samples directly, the way real XNA's
   * `GraphicsDevice.Textures[unit]` is.
   */
  public SetTexture(unit: number, texture: Texture2D): void;
  /** The same, for a `samplerCube`. */
  public SetTexture(unit: number, texture: TextureCube): void;
  /** The same, for a `sampler3D`. */
  public SetTexture(unit: number, texture: Texture3D): void;
  public SetTexture(unit: number, texture: Texture2D | TextureCube | Texture3D): void {
    if (texture == null) throw new TypeError("texture is required");
    const slot = wholeNumber(unit, "unit");
    if (texture instanceof Texture3D) {
      extensions().setShaderEffectTexture3D(
        this.#handle(), slot, resolveTexture3DHandleForInternalUse(texture));
      return;
    }
    if (texture instanceof TextureCube) {
      extensions().setShaderEffectTextureCube(
        this.#handle(), slot, resolveTextureCubeHandleForInternalUse(texture));
      return;
    }
    extensions().setShaderEffectTexture2D(
      this.#handle(), slot, resolveTexture2DHandleForInternalUse(texture));
  }
}

function uniformName(value: string, what = "name"): string {
  if (typeof value !== "string" || value.length === 0) {
    throw new TypeError(`${what} must be a non-empty uniform name`);
  }
  return value;
}

function numberList(values: readonly number[], what: string): number[] {
  if (!Array.isArray(values)) throw new TypeError(`${what} must be an array`);
  return values.map((value, index) => finite(value, `${what}[${index}]`));
}

/** One image-based light: the three textures a PBR shader needs, and how bright they are. */
export interface ImageBasedLight {
  /** The irradiance cube, or `null`. */
  readonly Irradiance: TextureCube | null;
  /** The prefiltered specular cube, or `null`. */
  readonly PrefilteredSpecular: TextureCube | null;
  /** The BRDF lookup texture, or `null`. */
  readonly BrdfLut: Texture2D | null;
  /** How many mip levels the prefiltered cube has; at least one. */
  readonly PrefilteredMipCount: number;
  /** Scalar multiplier on the light. */
  readonly Intensity: number;
}

function imageBasedLightSnapshot(light: ImageBasedLight): ImageBasedLightSnapshot {
  if (light == null) throw new TypeError("light is required");
  return {
    Irradiance: light.Irradiance == null
      ? 0n : resolveTextureCubeHandleForInternalUse(light.Irradiance),
    PrefilteredSpecular: light.PrefilteredSpecular == null
      ? 0n : resolveTextureCubeHandleForInternalUse(light.PrefilteredSpecular),
    BrdfLut: light.BrdfLut == null ? 0n : resolveTexture2DHandleForInternalUse(light.BrdfLut),
    PrefilteredMipCount: wholeNumber(light.PrefilteredMipCount, "PrefilteredMipCount"),
    Intensity: finite(light.Intensity, "Intensity"),
  };
}

/**
 * The image-based-light value CNA's PBR shading is described by: its defaults and the rule that
 * says whether one is complete enough to shade with. CNA keeps both as value routes in every build.
 */
export const ImageBasedLighting = {
  /** An image-based light with CNA's own defaults: no textures, so not yet {@link IsLightValid}. */
  DefaultLight(): ImageBasedLight {
    const snapshot = extensions().createDefaultImageBasedLight();
    // The three textures are borrowed handles the structure only records, so they come back as
    // nothing this package can hand out as an owned object.
    return {
      Irradiance: null,
      PrefilteredSpecular: null,
      BrdfLut: null,
      PrefilteredMipCount: snapshot.PrefilteredMipCount,
      Intensity: snapshot.Intensity,
    };
  },

  /**
   * Whether an image-based light is complete enough to shade with: all three textures present and
   * at least one mip. A light that is *nearly* complete does not look like a mismatch -- it looks
   * like a scene lit slightly wrong, which is the failure this answers.
   */
  IsLightValid(light: ImageBasedLight): boolean {
    return extensions().isImageBasedLightValid(imageBasedLightSnapshot(light));
  },
};

/**
 * The arguments of an indirect draw, in the exact layout the GPU reads: **sixteen bytes, four
 * 32-bit words**.
 *
 * An indirect draw takes its counts from a buffer the GPU itself wrote, so the CPU never learns how
 * many instances survived a culling pass — it just draws. This is the command format that buffer
 * has to hold.
 */
export interface IndirectDrawArguments {
  /** How many vertices to fetch. */
  readonly VertexCount: number;
  /** How many instances to draw; one for an ordinary draw, zero to draw nothing. */
  readonly InstanceCount: number;
  /** The first vertex, in elements of the bound stream. */
  readonly FirstVertex: number;
  /**
   * The first instance.
   *
   * **Must be zero on GL ES.** ES 3.1 has no base-instance parameter and the word is required to be
   * zero; a non-zero value there is undefined rather than diagnosed, and cannot be checked
   * anywhere — by the time the draw runs the value lives in GPU memory.
   */
  readonly BaseInstance: number;
}

/** The same, indexed: **twenty bytes, five words**. */
export interface IndirectDrawIndexedArguments {
  /** How many indices to fetch. */
  readonly IndexCount: number;
  /** How many instances to draw. */
  readonly InstanceCount: number;
  /** The first index, in index elements. */
  readonly FirstIndex: number;
  /** Added to every decoded index, in vertex elements; signed, as the API is. */
  readonly BaseVertex: number;
  /** The first instance; must be zero on GL ES, for the reason above. */
  readonly BaseInstance: number;
}

/**
 * Draws whose counts and offsets the GPU reads from a buffer, rather than the CPU passing them.
 *
 * This is what makes GPU culling worth doing: a compute pass writes how many instances survived
 * into a storage buffer, and the draw reads it there. The CPU never learns the number, so it never
 * has to wait for it — which is the whole latency the readback would cost.
 *
 * The argument buffer must hold {@link IndirectDrawArguments} or
 * {@link IndirectDrawIndexedArguments} at the byte offset given, in exactly the layout above.
 */
export const IndirectDraw = {
  /** All-zero arguments, which draw nothing — CNA's own defaults. */
  DefaultArguments(): IndirectDrawArguments {
    return extensions().createDefaultIndirectDrawArguments();
  },

  /** The same for an indexed draw. */
  DefaultIndexedArguments(): IndirectDrawIndexedArguments {
    return extensions().createDefaultIndirectDrawIndexedArguments();
  },

  /** The four words of a non-indexed command, in the order the GPU reads them. */
  PackArguments(value: IndirectDrawArguments): Uint32Array {
    if (value == null) throw new TypeError("value is required");
    return Uint32Array.from([
      wholeNumber(value.VertexCount, "VertexCount"),
      wholeNumber(value.InstanceCount, "InstanceCount"),
      wholeNumber(value.FirstVertex, "FirstVertex"),
      wholeNumber(value.BaseInstance, "BaseInstance"),
    ]);
  },

  /**
   * The five words of an indexed command.
   *
   * `BaseVertex` is **signed** — the API adds it to every decoded index and allows it to be
   * negative — so it is written through an `Int32Array` view of the same buffer rather than
   * coerced to unsigned.
   */
  PackIndexedArguments(value: IndirectDrawIndexedArguments): Uint32Array {
    if (value == null) throw new TypeError("value is required");
    const words = new Uint32Array(5);
    words[0] = wholeNumber(value.IndexCount, "IndexCount");
    words[1] = wholeNumber(value.InstanceCount, "InstanceCount");
    words[2] = wholeNumber(value.FirstIndex, "FirstIndex");
    new Int32Array(words.buffer)[3] = wholeNumber(value.BaseVertex, "BaseVertex");
    words[4] = wholeNumber(value.BaseInstance, "BaseInstance");
    return words;
  },
} as const;
