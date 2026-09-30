// SPDX-License-Identifier: MS-PL
//
// The rest of CNA's post-process passes, in a browser.
//
// The previous slice bound the passes whose whole input is the frame that is already on screen:
// the blit, colour grading, the tonemapper, bloom, FXAA, chromatic aberration and film grain. The
// twelve here are the ones that read something *else* -- depth, normals, velocity, a sun, a light
// on screen, a custom effect -- and they were left unbound not because a browser could not run
// them but because nobody had asked the artifact.
//
// The artifact was asked. Every one of these created and answered
// `cna_post_process_pass_is_supported` with **true** on a WebGL 2.0 context in headless Chromium,
// against an artifact built `-DCNA_CNAEXT=ON`; against the default artifact every create answers
// `NOT_SUPPORTED` (6) and nothing here is reached at all. Both answers are CNA's, which is the
// point: this file adds no policy of its own about what a browser can run.
//
// What makes them *checkable* rather than merely reachable is that seven of them ship a pure
// scalar of the same arithmetic their shader does, reached by a different route:
//
//   cna_depth_of_field_pass_circle_of_confusion_millimetres   the thin-lens equation
//   cna_ssao_pass_sample_count_for_quality                    the quality-to-samples table
//   cna_ssao_pass_copy_kernel                                 the hemisphere CNA actually samples
//   cna_aerial_perspective_pass_air_mass_for_distance         the exponential air-mass integral
//   cna_aerial_perspective_pass_transmittance                 Rayleigh extinction per channel
//   cna_height_fog_pass_optical_depth                         the height-fog integral
//   cna_contact_shadow_pass_is_occluded                       the ray-vs-depth acceptance test
//   cna_contact_shadow_pass_combine_visibility                how the two shadow terms multiply
//   cna_spatial_upscale_pass_is_identity_scale                whether the upscaler is a no-op
//
// A test that compares a rendered texel to one of those is comparing two implementations of one
// specification. That is the same standard `tonemapChannel` and `extractBloomChannel` set for the
// first slice, and it is why these passes are bound here rather than proved by "the output
// changed".

import type {
  RectangleSnapshot,
  SizeSnapshot,
} from "../backend.js";
import type { NativeHandle } from "../ownership.js";
import { WasmEngineState } from "./engine-state.js";
import { allocateStruct } from "./module.js";

/** Written into every `int32_t` output before the call, so a route that writes none is visible. */
const POISONED_INT32 = -0x5f5f5f60;

export abstract class WasmPostProcessPasses extends WasmEngineState {

  public override createAsciiEffect(device: NativeHandle): NativeHandle {
    return this.mem.create("cna_ascii_post_process_effect_create", device);
  }

  public override destroyAsciiEffect(effect: NativeHandle): void {
    this.routes.invoke("cna_ascii_post_process_effect_destroy", effect);
  }

  /** A cell is two `int32_t` outputs rather than one, so a width read as a height is visible. */
  public override getAsciiCellSize(effect: NativeHandle): SizeSnapshot {
    return this.#twoInts("cna_ascii_post_process_effect_get_cell_size", effect);
  }

  public override setAsciiCellSize(effect: NativeHandle, width: number, height: number): void {
    this.routes.invoke(
      "cna_ascii_post_process_effect_set_cell_size", effect,
      Math.trunc(width), Math.trunc(height),
    );
  }

  public override getAsciiQuantizeMode(effect: NativeHandle): number {
    return this.mem.u32("cna_ascii_post_process_effect_get_quantize_mode", effect);
  }

  public override setAsciiQuantizeMode(effect: NativeHandle, mode: number): void {
    this.routes.invoke("cna_ascii_post_process_effect_set_quantize_mode", effect, mode);
  }

  /** The effect over a rectangle of whatever target is bound. */
  public override drawAsciiEffect(
    effect: NativeHandle, source: NativeHandle, destination: RectangleSnapshot,
  ): void {
    const scope = this.routes.scope();
    try {
      const rectangle = allocateStruct(this.routes.module, scope, "CNA_Rectangle", false);
      rectangle.setI32("x", Math.trunc(destination.X)).setI32("y", Math.trunc(destination.Y))
        .setI32("width", Math.trunc(destination.Width))
        .setI32("height", Math.trunc(destination.Height));
      this.routes.invoke(
        "cna_ascii_post_process_effect_draw", effect, source, rectangle.pointer);
    } finally {
      scope.dispose();
    }
  }

  /** The character grid the last draw produced. */
  public override getAsciiLastGridDimensions(effect: NativeHandle): SizeSnapshot {
    return this.#twoInts("cna_ascii_post_process_effect_get_last_grid_dimensions", effect);
  }

  /**
   * A route with two `int32_t` outputs rather than a structure.
   *
   * Both are poisoned before the call: reading back an initializer this binding wrote is how a
   * route that never writes its outputs passes a test about what it computed.
   */
  #twoInts(route: string, handle: NativeHandle): SizeSnapshot {
    const scope = this.routes.scope();
    try {
      const width = scope.allocate(4);
      const height = scope.allocate(4);
      const before = this.routes.view();
      before.setInt32(width, POISONED_INT32, true);
      before.setInt32(height, POISONED_INT32, true);
      this.routes.invoke(route, handle, width, height);
      const view = this.routes.view();
      return { Width: view.getInt32(width, true), Height: view.getInt32(height, true) };
    } finally {
      scope.dispose();
    }
  }
}
