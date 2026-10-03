// SPDX-License-Identifier: MS-PL
//
// The extended-graphics facade's handle-only half: the CRT and depth effects, the debug drawer and
// the shader-effect family.
//
// Every member here is generated from three facts and no judgement: the member's own signature in
// `CnaGraphicsExtensionBackend`, the route the Node-API bridge proves it against, and the C
// declaration of that route -- which is what decides whether a `number` argument is truncated to an
// integer or passed as a float, because `size` is an `int32_t` in one route here and a `float` in
// the next. Nothing chooses a helper by the look of a name.
//
// It is a separate file from its siblings for the reason the others are: the extended-graphics
// interface is 603 members, and a facade that size is only readable if each part of it is one
// family. What this part has in common is that none of it writes a CNA structure into wasm memory;
// the members that do are in the file below it.

import type { Vector3Snapshot } from "../backend.js";
import type { NativeHandle } from "../ownership.js";
import { WasmEngineStructures } from "./engine-structures.js";

/**
 * A handle value CNA cannot issue, written into a handle output before the call that fills it.
 *
 * A route that answers "there is no texture here" leaves its handle output untouched, so what a
 * caller reads out of it is whatever the scope allocation last held. Poisoning makes that
 * observable: a binding that takes the handle without asking whether there is one answers with a
 * value no CNA object has, instead of with a zero that looks exactly like a correct absence.
 */
const POISONED_HANDLE = 0xDEADBEEFDEADBEEFn;

export abstract class WasmEngineState extends WasmEngineStructures {

  public override createCrtEffect(device: NativeHandle): NativeHandle {
    return this.routes.outHandle("cna_crt_effect_create", device);
  }

  public override getCrtScanlineIntensity(effect: NativeHandle): number {
    return this.mem.float("cna_crt_effect_get_scanline_intensity", effect);
  }

  public override setCrtScanlineIntensity(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_crt_effect_set_scanline_intensity", effect, value);
  }

  public override getCrtCurvature(effect: NativeHandle): number {
    return this.mem.float("cna_crt_effect_get_curvature", effect);
  }

  public override setCrtCurvature(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_crt_effect_set_curvature", effect, value);
  }

  public override getCrtVignetteIntensity(effect: NativeHandle): number {
    return this.mem.float("cna_crt_effect_get_vignette_intensity", effect);
  }

  public override setCrtVignetteIntensity(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_crt_effect_set_vignette_intensity", effect, value);
  }

  public override getCrtMaskIntensity(effect: NativeHandle): number {
    return this.mem.float("cna_crt_effect_get_mask_intensity", effect);
  }

  public override setCrtMaskIntensity(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_crt_effect_set_mask_intensity", effect, value);
  }

  public override getCrtMaskType(effect: NativeHandle): number {
    return this.mem.u32("cna_crt_effect_get_mask_type", effect);
  }

  public override setCrtMaskType(effect: NativeHandle, maskType: number): void {
    this.routes.invoke("cna_crt_effect_set_mask_type", effect, maskType);
  }

  public override createDepthEffect(device: NativeHandle): NativeHandle {
    return this.routes.outHandle("cna_depth_effect_create", device);
  }

  public override getDepthEffectMode(effect: NativeHandle): number {
    return this.mem.u32("cna_depth_effect_get_mode", effect);
  }

  public override setDepthEffectMode(effect: NativeHandle, mode: number): void {
    this.routes.invoke("cna_depth_effect_set_mode", effect, mode);
  }

  public override getDepthEffectDitherMode(effect: NativeHandle): number {
    return this.mem.u32("cna_depth_effect_get_dither_mode", effect);
  }

  public override setDepthEffectDitherMode(effect: NativeHandle, mode: number): void {
    this.routes.invoke("cna_depth_effect_set_dither_mode", effect, mode);
  }

  public override createDebugDraw(graphicsDevice: NativeHandle): NativeHandle {
    return this.routes.outHandle("cna_debug_draw_create", graphicsDevice);
  }

  public override destroyDebugDraw(debug: NativeHandle): void {
    this.routes.invoke("cna_debug_draw_destroy", debug);
  }

  public override endDebugDraw(debug: NativeHandle): void {
    this.routes.invoke("cna_debug_draw_end", debug);
  }

  public override clearDebugDraw(debug: NativeHandle): void {
    this.routes.invoke("cna_debug_draw_clear", debug);
  }

  public override addDebugDrawSphere(
    debug: NativeHandle, centre: Vector3Snapshot, radius: number, color: number, segments: number,
  ): void {
    this.mem.withVector3(
      centre,
      (centrePointer) => this.routes.invoke(
          "cna_debug_draw_add_sphere", debug, centrePointer, radius, color, Math.trunc(segments),
      ),
    );
  }

  public override addDebugDrawFrustum(
    debug: NativeHandle, viewProjection: readonly number[], color: number,
  ): void {
    this.mem.withMatrix(
      viewProjection,
      (viewProjectionPointer) => this.routes.invoke(
          "cna_debug_draw_add_frustum", debug, viewProjectionPointer, color,
      ),
    );
  }

  public override addDebugDrawCross(
    debug: NativeHandle, position: Vector3Snapshot, size: number, color: number,
  ): void {
    this.mem.withVector3(
      position,
      (positionPointer) => this.routes.invoke(
          "cna_debug_draw_add_cross", debug, positionPointer, size, color,
      ),
    );
  }

  public override isDebugDrawDepthTested(debug: NativeHandle): boolean {
    return this.mem.bool("cna_debug_draw_is_depth_tested", debug);
  }

  public override setDebugDrawDepthTested(debug: NativeHandle, value: boolean): void {
    this.routes.invoke("cna_debug_draw_set_depth_tested", debug, value ? 1 : 0);
  }

  public override getDebugDrawLineCount(debug: NativeHandle): number {
    return this.mem.int("cna_debug_draw_get_line_count", debug);
  }

  public override isShaderEffectValid(effect: NativeHandle): boolean {
    return this.mem.bool("cna_shader_effect_is_valid", effect);
  }

  public override shaderEffectHasRenderer(effect: NativeHandle): boolean {
    return this.mem.bool("cna_shader_effect_has_renderer", effect);
  }

  public override getShaderEffectCompileError(effect: NativeHandle): string {
    return this.mem.probedString("cna_shader_effect_copy_compile_error_ext", effect);
  }

  public override setShaderEffectUniformFloat(
    effect: NativeHandle, name: string, value: number,
  ): void {
    this.mem.withStringView(
      name,
      (namePointer) => this.routes.invoke(
          "cna_shader_effect_set_uniform_float", effect, namePointer, value,
      ),
    );
  }

  public override setShaderEffectUniformInt32(
    effect: NativeHandle, name: string, value: number,
  ): void {
    this.mem.withStringView(
      name,
      (namePointer) => this.routes.invoke(
          "cna_shader_effect_set_uniform_int32", effect, namePointer, Math.trunc(value),
      ),
    );
  }

  public override setShaderEffectTexture2D(
    effect: NativeHandle, unit: number, texture: NativeHandle,
  ): void {
    this.routes.invoke("cna_shader_effect_set_texture2d", effect, Math.trunc(unit), texture);
  }

  public override setShaderEffectTextureCube(
    effect: NativeHandle, unit: number, texture: NativeHandle,
  ): void {
    this.routes.invoke("cna_shader_effect_set_texture_cube", effect, Math.trunc(unit), texture);
  }

  public override setShaderEffectTexture3D(
    effect: NativeHandle, unit: number, texture: NativeHandle,
  ): void {
    this.routes.invoke("cna_shader_effect_set_texture3d", effect, Math.trunc(unit), texture);
  }

  public override getShaderEffectWorld(effect: NativeHandle): readonly number[] {
    return this.mem.matrix("cna_shader_effect_get_world", effect);
  }

  public override setShaderEffectWorld(effect: NativeHandle, value: readonly number[]): void {
    this.mem.withMatrix(
      value,
      (valuePointer) => this.routes.invoke("cna_shader_effect_set_world", effect, valuePointer),
    );
  }

  public override getShaderEffectView(effect: NativeHandle): readonly number[] {
    return this.mem.matrix("cna_shader_effect_get_view", effect);
  }

  public override setShaderEffectView(effect: NativeHandle, value: readonly number[]): void {
    this.mem.withMatrix(
      value,
      (valuePointer) => this.routes.invoke("cna_shader_effect_set_view", effect, valuePointer),
    );
  }

  public override getShaderEffectProjection(effect: NativeHandle): readonly number[] {
    return this.mem.matrix("cna_shader_effect_get_projection", effect);
  }

  public override setShaderEffectProjection(effect: NativeHandle, value: readonly number[]): void {
    this.mem.withMatrix(
      value,
      (valuePointer) => this.routes.invoke(
          "cna_shader_effect_set_projection", effect, valuePointer,
      ),
    );
  }

  public override createPbrEffect(graphicsDevice: NativeHandle): NativeHandle {
    return this.routes.outHandle("cna_pbr_effect_create", graphicsDevice);
  }

  public override createSkinnedPbrEffect(graphicsDevice: NativeHandle): NativeHandle {
    return this.routes.outHandle("cna_skinned_pbr_effect_create", graphicsDevice);
  }

  public override getPbrEffectAlpha(effect: NativeHandle): number {
    return this.mem.float("cna_pbr_effect_get_alpha", effect);
  }

  public override getPbrEffectAlphaCutoff(effect: NativeHandle): number {
    return this.mem.float("cna_pbr_effect_get_alpha_cutoff_ext", effect);
  }

  public override getPbrEffectAlphaMode(effect: NativeHandle): number {
    return this.mem.u32("cna_pbr_effect_get_alpha_mode_ext", effect);
  }

  public override getPbrEffectDiffuseColor(effect: NativeHandle): Vector3Snapshot {
    return this.mem.vector3("cna_pbr_effect_get_diffuse_color", effect);
  }

  public override getPbrEffectDoubleSided(effect: NativeHandle): boolean {
    return this.mem.bool("cna_pbr_effect_get_double_sided_ext", effect);
  }

  public override getPbrEffectEmissiveFactor(effect: NativeHandle): Vector3Snapshot {
    return this.mem.vector3("cna_pbr_effect_get_emissive_factor", effect);
  }

  public override getPbrEffectEncodeOutputToSrgb(effect: NativeHandle): boolean {
    return this.mem.bool("cna_pbr_effect_get_encode_output_to_srgb_ext", effect);
  }

  public override getPbrEffectIor(effect: NativeHandle): number {
    return this.mem.float("cna_pbr_effect_get_ior_ext", effect);
  }

  public override getPbrEffectMetallicFactor(effect: NativeHandle): number {
    return this.mem.float("cna_pbr_effect_get_metallic_factor", effect);
  }

  public override getPbrEffectNormalScale(effect: NativeHandle): number {
    return this.mem.float("cna_pbr_effect_get_normal_scale_ext", effect);
  }

  public override getPbrEffectOcclusionStrength(effect: NativeHandle): number {
    return this.mem.float("cna_pbr_effect_get_occlusion_strength_ext", effect);
  }

  public override getPbrEffectRoughnessFactor(effect: NativeHandle): number {
    return this.mem.float("cna_pbr_effect_get_roughness_factor", effect);
  }

  public override getPbrEffectSpecularColorFactor(effect: NativeHandle): Vector3Snapshot {
    return this.mem.vector3("cna_pbr_effect_get_specular_color_factor_ext", effect);
  }

  public override getPbrEffectSpecularFactor(effect: NativeHandle): number {
    return this.mem.float("cna_pbr_effect_get_specular_factor_ext", effect);
  }

  /**
   * A texture slot, which answers **whether there is one** and then which -- two outputs, not one.
   *
   * A slot need not hold a texture, and CNA says so with a separate `CNA_Bool` rather than with an
   * invalid handle. Reading only the handle would turn "no texture" into whatever the allocation
   * held; an empty slot is `CNA_INVALID_HANDLE` here, which is what the public API reads as none.
   *
   * Both outputs are poisoned first. A scope allocation is not guaranteed to be zero, and a reader
   * that ignored the presence flag would otherwise answer "no texture" correctly whenever the
   * memory it reused happened to hold zero -- which is most of the time, and never when it
   * matters. `POISONED_HANDLE` is not a handle CNA can issue, so an empty slot that answers with it
   * is the binding having skipped the flag rather than CNA having answered.
   */
  public override getPbrEffectTexture(effect: NativeHandle, slot: number): NativeHandle {
    const scope = this.routes.scope();
    try {
      const present = scope.allocate(4);
      const handle = scope.allocate(8);
      this.routes.view().setUint32(present, 0xBAADF00D, true);
      this.routes.view().setBigUint64(handle, POISONED_HANDLE, true);
      this.routes.invoke("cna_pbr_effect_get_texture", effect, slot, present, handle);
      if (this.routes.view().getUint8(present) === 0) return 0n;
      return this.routes.view().getBigUint64(handle, true);
    } finally {
      scope.dispose();
    }
  }

  public override getPbrEffectVertexColorEnabled(effect: NativeHandle): boolean {
    return this.mem.bool("cna_pbr_effect_get_vertex_color_enabled_ext", effect);
  }

  public override getSkinnedPbrEffectWeightsPerVertex(effect: NativeHandle): number {
    return this.mem.int("cna_skinned_pbr_effect_get_weights_per_vertex", effect);
  }

  public override setPbrEffectAlpha(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_alpha", effect, value);
  }

  public override setPbrEffectAlphaCutoff(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_alpha_cutoff_ext", effect, value);
  }

  public override setPbrEffectAlphaMode(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_alpha_mode_ext", effect, value);
  }

  public override setPbrEffectDiffuseColor(effect: NativeHandle, value: Vector3Snapshot): void {
    this.mem.withVector3(
      value,
      (valuePointer) => this.routes.invoke(
          "cna_pbr_effect_set_diffuse_color", effect, valuePointer,
      ),
    );
  }

  public override setPbrEffectDoubleSided(effect: NativeHandle, value: boolean): void {
    this.routes.invoke("cna_pbr_effect_set_double_sided_ext", effect, value ? 1 : 0);
  }

  public override setPbrEffectEmissiveFactor(effect: NativeHandle, value: Vector3Snapshot): void {
    this.mem.withVector3(
      value,
      (valuePointer) => this.routes.invoke(
          "cna_pbr_effect_set_emissive_factor", effect, valuePointer,
      ),
    );
  }

  public override setPbrEffectEncodeOutputToSrgb(effect: NativeHandle, value: boolean): void {
    this.routes.invoke("cna_pbr_effect_set_encode_output_to_srgb_ext", effect, value ? 1 : 0);
  }

  public override setPbrEffectIor(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_ior_ext", effect, value);
  }

  public override setPbrEffectMetallicFactor(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_metallic_factor", effect, value);
  }

  public override setPbrEffectNormalScale(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_normal_scale_ext", effect, value);
  }

  public override setPbrEffectOcclusionStrength(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_occlusion_strength_ext", effect, value);
  }

  public override setPbrEffectRoughnessFactor(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_roughness_factor", effect, value);
  }

  public override setPbrEffectSpecularColorFactor(
    effect: NativeHandle, value: Vector3Snapshot,
  ): void {
    this.mem.withVector3(
      value,
      (valuePointer) => this.routes.invoke(
          "cna_pbr_effect_set_specular_color_factor_ext", effect, valuePointer,
      ),
    );
  }

  public override setPbrEffectSpecularFactor(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_pbr_effect_set_specular_factor_ext", effect, value);
  }

  public override setPbrEffectTexture(
    effect: NativeHandle, slot: number, texture: NativeHandle,
  ): void {
    this.routes.invoke("cna_pbr_effect_set_texture", effect, slot, texture);
  }

  public override setPbrEffectVertexColorEnabled(effect: NativeHandle, value: boolean): void {
    this.routes.invoke("cna_pbr_effect_set_vertex_color_enabled_ext", effect, value ? 1 : 0);
  }

  public override setSkinnedPbrEffectWeightsPerVertex(effect: NativeHandle, value: number): void {
    this.routes.invoke("cna_skinned_pbr_effect_set_weights_per_vertex", effect, Math.trunc(value));
  }
}
