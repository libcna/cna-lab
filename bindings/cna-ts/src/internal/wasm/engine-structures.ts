// SPDX-License-Identifier: MS-PL
//
// The extended-graphics members whose arguments or answers are CNA structures.
//
// Seventeen structures cross this boundary -- a PBR material with twenty-five members, a pipeline
// settings block with forty-seven, two glTF sources, two light descriptions, a cascade state, a
// BRDF table entry, two indirect-draw argument blocks and a cullable instance -- and every one of
// them is **growable**: `struct_size` selects which fields CNA reads, so a zeroed buffer asks it to
// read a structure of no size. Every marshaller below therefore begins with CNA's own initializer.
//
// The marshallers are paired field by field between the C declaration and the TypeScript snapshot,
// by name, and the pairing is required to be **total in both directions**: a snapshot member with
// no field, or a field with no member, is an error rather than a silently dropped value. The one
// exception is `CNA_PbrMaterial`, whose defaults query answers a structure whose texture handles
// are always invalid and whose snapshot is the values half of it.
//
// Three naming facts had to be written down rather than worked around: CNA suffixes several fields
// `_ext` where the member has no suffix, spells one of them `doff_number`, and spells a `CNA_Bool`
// as one byte where its neighbouring enumeration is four.

import type {
  BlendStateSnapshot,
  BoundingSphereSnapshot,
  ClusterBoundsSnapshot,
  DebugVertexSnapshot,
  ImageBasedLightSnapshot,
  IndirectDrawArgumentsSnapshot,
  IndirectDrawIndexedArgumentsSnapshot,
  RasterizerStateSnapshot,
  TextureTransformSnapshot,
  Vector2Snapshot,
  Vector3Snapshot,
  Vector4Snapshot,
} from "../backend.js";
import type { NativeHandle } from "../ownership.js";
import { WasmGraphicsExtensionCore } from "./graphics-ext-core.js";
import { WASM_STRUCT_LAYOUTS } from "./layout.js";
import { allocateStruct, WasmStruct, type WasmScope } from "./module.js";

/** A `CNA_Vector2` nested inside a structure, read at its measured offset. */
function readVector2(structure: WasmStruct, field: string): Vector2Snapshot {
  const [X, Y] = structure.getF32Array(field) as [number, number];
  return { X, Y };
}

function writeVector2(structure: WasmStruct, field: string, value: Vector2Snapshot): void {
  structure.setF32Array(field, [value.X, value.Y]);
}







export abstract class WasmEngineStructures extends WasmGraphicsExtensionCore {

  public override getDefaultTextureTransform(): TextureTransformSnapshot {
    const scope = this.routes.scope();
    try {
      const structure = this.#allocTextureTransform(scope);
      this.routes.invoke("cna_texture_transform_ext_init", structure.pointer);
      return this.#readTextureTransform(structure);
    } finally {
      scope.dispose();
    }
  }

  public override createDefaultImageBasedLight(): ImageBasedLightSnapshot {
    const scope = this.routes.scope();
    try {
      const structure = this.#allocImageBasedLight(scope);
      this.routes.invoke("cna_image_based_light_ext_init", structure.pointer);
      return this.#readImageBasedLight(structure);
    } finally {
      scope.dispose();
    }
  }

  public override isImageBasedLightValid(light: ImageBasedLightSnapshot): boolean {
    return this.#withImageBasedLight(
      light, (lightPointer) => this.mem.bool("cna_image_based_light_ext_is_valid", lightPointer),
    );
  }

  public override createDefaultIndirectDrawArguments(): IndirectDrawArgumentsSnapshot {
    const scope = this.routes.scope();
    try {
      const structure = this.#allocIndirectDrawArguments(scope);
      this.routes.invoke("cna_indirect_draw_arguments_init", structure.pointer);
      return this.#readIndirectDrawArguments(structure);
    } finally {
      scope.dispose();
    }
  }

  public override beginDebugDraw(
    debug: NativeHandle, view: readonly number[], projection: readonly number[],
  ): void {
    this.mem.withMatrix(
      view,
      (
        viewPointer) => this.mem.withMatrix(projection,
        (
          projectionPointer) => this.routes.invoke("cna_debug_draw_begin",
          debug,
          viewPointer,
          projectionPointer,
        ),
      ),
    );
  }

  public override addDebugDrawLine(
    debug: NativeHandle, from: Vector3Snapshot, to: Vector3Snapshot, color: number,
  ): void {
    this.mem.withVector3(
      from,
      (
        fromPointer) => this.mem.withVector3(to,
        (
          toPointer) => this.routes.invoke("cna_debug_draw_add_line",
          debug,
          fromPointer,
          toPointer,
          color,
        ),
      ),
    );
  }

  public override createShaderEffect(
    graphicsDevice: NativeHandle, vertexSource: string, fragmentSource: string,
  ): NativeHandle {
    return this.mem.withStringView(
      vertexSource,
      (
        vertexSourcePointer) => this.mem.withStringView(fragmentSource,
        (
          fragmentSourcePointer) => this.routes.outHandle("cna_shader_effect_create",
          graphicsDevice,
          vertexSourcePointer,
          fragmentSourcePointer,
        ),
      ),
    );
  }

  public override setShaderEffectUniformMatrix(
    effect: NativeHandle, name: string, value: readonly number[],
  ): void {
    this.mem.withStringView(
      name,
      (
        namePointer) => this.mem.withMatrix(value,
        (
          valuePointer) => this.routes.invoke("cna_shader_effect_set_uniform_matrix",
          effect,
          namePointer,
          valuePointer,
        ),
      ),
    );
  }

  public override setShaderEffectUniformVector3(
    effect: NativeHandle, name: string, value: Vector3Snapshot,
  ): void {
    this.mem.withStringView(
      name,
      (
        namePointer) => this.mem.withVector3(value,
        (
          valuePointer) => this.routes.invoke("cna_shader_effect_set_uniform_vector3",
          effect,
          namePointer,
          valuePointer,
        ),
      ),
    );
  }

  public override setShaderEffectUniformVector2(
    effect: NativeHandle, name: string, value: Vector2Snapshot,
  ): void {
    this.mem.withStringView(
      name,
      (
        namePointer) => this.mem.withVector2(value,
        (
          valuePointer) => this.routes.invoke("cna_shader_effect_set_uniform_vector2",
          effect,
          namePointer,
          valuePointer,
        ),
      ),
    );
  }

  public override setShaderEffectUniformFloatArray(
    effect: NativeHandle, name: string, values: readonly number[],
  ): void {
    const scope = this.routes.scope();
    try {
      const buffer = this.#floats(scope, values);
      this.mem.withStringView(name, (namePointer) => this.routes.invoke(
        "cna_shader_effect_set_uniform_float_array", effect, namePointer, buffer, BigInt(values.length)));
    } finally {
      scope.dispose();
    }
  }

  public override setShaderEffectUniformVec3Array(
    effect: NativeHandle, name: string, values: readonly number[],
  ): void {
    const scope = this.routes.scope();
    try {
      const buffer = this.#floats(scope, values);
      this.mem.withStringView(name, (namePointer) => this.routes.invoke(
        "cna_shader_effect_set_uniform_vec3_array", effect, namePointer, buffer, values.length));
    } finally {
      scope.dispose();
    }
  }

  public override setShaderEffectUniformMat4Array(
    effect: NativeHandle, name: string, values: readonly number[],
  ): void {
    const scope = this.routes.scope();
    try {
      const buffer = this.#floats(scope, values);
      this.mem.withStringView(name, (namePointer) => this.routes.invoke(
        "cna_shader_effect_set_uniform_mat4_array", effect, namePointer, buffer, values.length));
    } finally {
      scope.dispose();
    }
  }

  public override createDefaultIndirectDrawIndexedArguments(

  ): IndirectDrawIndexedArgumentsSnapshot {
    const scope = this.routes.scope();
    try {
      const structure = this.#allocIndirectDrawIndexedArguments(scope);
      this.routes.invoke("cna_indirect_draw_indexed_arguments_init", structure.pointer);
      return this.#readIndirectDrawIndexedArguments(structure);
    } finally {
      scope.dispose();
    }
  }

  public override getPbrEffectTextureCoordinateSet(effect: NativeHandle, slot: number): number {
    return this.mem.int("cna_pbr_effect_get_texture_coordinate_set_ext", effect, slot);
  }

  public override getPbrEffectTextureIsSrgb(effect: NativeHandle, slot: number): boolean {
    return this.mem.bool("cna_pbr_effect_get_texture_is_srgb_ext", effect, slot);
  }

  public override getPbrEffectTextureTransform(
    effect: NativeHandle, slot: number,
  ): TextureTransformSnapshot {
    const scope = this.routes.scope();
    try {
      const structure = this.#allocTextureTransform(scope);
      this.routes.invoke(
        "cna_pbr_effect_get_texture_transform_ext",
        effect,
        slot,
        structure.pointer,
      );
      return this.#readTextureTransform(structure);
    } finally {
      scope.dispose();
    }
  }

  public override setPbrEffectTextureCoordinateSet(
    effect: NativeHandle, slot: number, value: number,
  ): void {
    this.routes.invoke(
      "cna_pbr_effect_set_texture_coordinate_set_ext",
      effect,
      slot,
      Math.trunc(value),
    );
  }

  public override setPbrEffectTextureIsSrgb(
    effect: NativeHandle, slot: number, value: boolean,
  ): void {
    this.routes.invoke("cna_pbr_effect_set_texture_is_srgb_ext", effect, slot, value ? 1 : 0);
  }

  public override setPbrEffectTextureTransform(
    effect: NativeHandle, slot: number, transform: TextureTransformSnapshot,
  ): void {
    this.#withTextureTransform(
      transform,
      (
        transformPointer) => this.routes.invoke("cna_pbr_effect_set_texture_transform_ext",
        effect,
        slot,
        transformPointer,
      ),
    );
  }








  /** Reads a `CNA_TextureTransformEXT` a route has written. */
  #readTextureTransform(structure: WasmStruct): TextureTransformSnapshot {
    return {
      Offset: readVector2(structure, "offset"),
      Scale: readVector2(structure, "scale"),
      Rotation: structure.getF32("rotation"),
    };
  }

  /**
   * Writes one, after CNA's own initializer has filled it.
   *
   * The initializer runs first because these structures are growable: `struct_size` selects which
   * fields CNA reads, and a zeroed one asks it to read a structure of no size.
   */
  #withTextureTransform<T>(values: TextureTransformSnapshot, body: (pointer: number) => T): T {
    const scope = this.routes.scope();
    try {
      const structure = allocateStruct(this.routes.module, scope, "CNA_TextureTransformEXT");
      this.routes.invoke("cna_texture_transform_ext_init", structure.pointer);
      structure
        .setF32("rotation", values.Rotation);
      writeVector2(structure, "offset", values.Offset);
      writeVector2(structure, "scale", values.Scale);
      return body(structure.pointer);
    } finally {
      scope.dispose();
    }
  }

  /** Allocates one, initialised, for a route that fills it. */
  #allocTextureTransform(scope: WasmScope): WasmStruct {
    const structure = allocateStruct(this.routes.module, scope, "CNA_TextureTransformEXT");
    this.routes.invoke("cna_texture_transform_ext_init", structure.pointer);
    return structure;
  }












  /** Reads a `CNA_ImageBasedLightEXT` a route has written. */
  #readImageBasedLight(structure: WasmStruct): ImageBasedLightSnapshot {
    return {
      Irradiance: structure.getU64("irradiance"),
      PrefilteredSpecular: structure.getU64("prefiltered_specular"),
      BrdfLut: structure.getU64("brdf_lut"),
      PrefilteredMipCount: structure.getI32("prefiltered_mip_count"),
      Intensity: structure.getF32("intensity"),
    };
  }

  /**
   * Writes one, after CNA's own initializer has filled it.
   *
   * The initializer runs first because these structures are growable: `struct_size` selects which
   * fields CNA reads, and a zeroed one asks it to read a structure of no size.
   */
  #withImageBasedLight<T>(values: ImageBasedLightSnapshot, body: (pointer: number) => T): T {
    const scope = this.routes.scope();
    try {
      const structure = allocateStruct(this.routes.module, scope, "CNA_ImageBasedLightEXT");
      this.routes.invoke("cna_image_based_light_ext_init", structure.pointer);
      structure
        .setU64("irradiance", values.Irradiance)
        .setU64("prefiltered_specular", values.PrefilteredSpecular)
        .setU64("brdf_lut", values.BrdfLut)
        .setI32("prefiltered_mip_count", Math.trunc(values.PrefilteredMipCount))
        .setF32("intensity", values.Intensity);
      return body(structure.pointer);
    } finally {
      scope.dispose();
    }
  }

  /** Allocates one, initialised, for a route that fills it. */
  #allocImageBasedLight(scope: WasmScope): WasmStruct {
    const structure = allocateStruct(this.routes.module, scope, "CNA_ImageBasedLightEXT");
    this.routes.invoke("cna_image_based_light_ext_init", structure.pointer);
    return structure;
  }


















  /** Reads a `CNA_IndirectDrawArguments` a route has written. */
  #readIndirectDrawArguments(structure: WasmStruct): IndirectDrawArgumentsSnapshot {
    return {
      VertexCount: structure.getU32("vertex_count"),
      InstanceCount: structure.getU32("instance_count"),
      FirstVertex: structure.getU32("first_vertex"),
      BaseInstance: structure.getU32("base_instance"),
    };
  }

  /** Allocates one, initialised, for a route that fills it. */
  #allocIndirectDrawArguments(scope: WasmScope): WasmStruct {
    const structure = allocateStruct(this.routes.module, scope, "CNA_IndirectDrawArguments");
    this.routes.invoke("cna_indirect_draw_arguments_init", structure.pointer);
    return structure;
  }

  /** Reads a `CNA_IndirectDrawIndexedArguments` a route has written. */
  #readIndirectDrawIndexedArguments(structure: WasmStruct): IndirectDrawIndexedArgumentsSnapshot {
    return {
      IndexCount: structure.getU32("index_count"),
      InstanceCount: structure.getU32("instance_count"),
      FirstIndex: structure.getU32("first_index"),
      BaseVertex: structure.getI32("base_vertex"),
      BaseInstance: structure.getU32("base_instance"),
    };
  }

  /** Allocates one, initialised, for a route that fills it. */
  #allocIndirectDrawIndexedArguments(scope: WasmScope): WasmStruct {
    const structure = allocateStruct(this.routes.module, scope, "CNA_IndirectDrawIndexedArguments");
    this.routes.invoke("cna_indirect_draw_indexed_arguments_init", structure.pointer);
    return structure;
  }










  // --- the device's own state, read back --------------------------------------------------------

  public override getDeviceBlendState(device: NativeHandle): BlendStateSnapshot {
    const scope = this.routes.scope();
    try {
      const state = allocateStruct(this.routes.module, scope, "CNA_BlendState");
      this.routes.invoke("cna_graphics_device_get_blend_state", device, state.pointer);
      return {
        AlphaBlendFunction: state.getU32("alpha_blend_function"),
        AlphaDestinationBlend: state.getU32("alpha_destination_blend"),
        AlphaSourceBlend: state.getU32("alpha_source_blend"),
        ColorBlendFunction: state.getU32("color_blend_function"),
        ColorDestinationBlend: state.getU32("color_destination_blend"),
        ColorSourceBlend: state.getU32("color_source_blend"),
        ColorWriteChannels: state.getU32("color_write_channels"),
        ColorWriteChannels1: state.getU32("color_write_channels1"),
        ColorWriteChannels2: state.getU32("color_write_channels2"),
        ColorWriteChannels3: state.getU32("color_write_channels3"),
        BlendFactor: state.getU32("blend_factor"),
        MultiSampleMask: state.getI32("multi_sample_mask"),
      };
    } finally {
      scope.dispose();
    }
  }

  public override getDeviceRasterizerState(device: NativeHandle): RasterizerStateSnapshot {
    const scope = this.routes.scope();
    try {
      const state = allocateStruct(this.routes.module, scope, "CNA_RasterizerState");
      this.routes.invoke("cna_graphics_device_get_rasterizer_state", device, state.pointer);
      return {
        CullMode: state.getU32("cull_mode"),
        FillMode: state.getU32("fill_mode"),
        DepthBias: state.getF32("depth_bias"),
        SlopeScaleDepthBias: state.getF32("slope_scale_depth_bias"),
        MultiSampleAntiAlias: state.getU8("multi_sample_anti_alias") !== 0,
        ScissorTestEnable: state.getU8("scissor_test_enable") !== 0,
      };
    } finally {
      scope.dispose();
    }
  }

  // --- the debug draw ---------------------------------------------------------------------------

  public override addDebugDrawBox(
    debug: NativeHandle, bounds: ClusterBoundsSnapshot, color: number,
  ): void {
    const scope = this.routes.scope();
    try {
      this.routes.invoke(
        "cna_debug_draw_add_box", debug, this.mem.writeBounds(scope, bounds), color >>> 0);
    } finally {
      scope.dispose();
    }
  }

  public override addDebugDrawBoundingSphere(
    debug: NativeHandle, sphere: BoundingSphereSnapshot, color: number, segments: number,
  ): void {
    const scope = this.routes.scope();
    try {
      this.routes.invoke(
        "cna_debug_draw_add_bounding_sphere", debug, this.#writeSphere(scope, sphere),
        color >>> 0, Math.trunc(segments),
      );
    } finally {
      scope.dispose();
    }
  }

  /** Every vertex the debug draw accumulated, at the measured `CNA_VertexPositionColor` stride. */
  public override getDebugDrawVertices(
    debug: NativeHandle, depthTested: boolean,
  ): readonly DebugVertexSnapshot[] {
    const layout = WASM_STRUCT_LAYOUTS.CNA_VertexPositionColor;
    return this.mem.probedArray(
      "cna_debug_draw_copy_vertices", [debug, depthTested ? 1 : 0], layout.size,
      (base, written) => {
        const view = this.routes.view();
        return Array.from({ length: written }, (_, index) => {
          const at = base + layout.size * index;
          return {
            Position: {
              X: view.getFloat32(at + layout.fields.position.offset, true),
              Y: view.getFloat32(at + layout.fields.position.offset + 4, true),
              Z: view.getFloat32(at + layout.fields.position.offset + 8, true),
            },
            Color: view.getUint32(at + layout.fields.color.offset, true),
          };
        });
      });
  }

  // --- the shader effect's remaining uniforms ---------------------------------------------------

  /** A `CNA_Vector4` taken **by value**, which wasm32 lowers as a pointer to a caller copy. */
  public override setShaderEffectUniformVector4(
    effect: NativeHandle, name: string, value: Vector4Snapshot,
  ): void {
    const scope = this.routes.scope();
    try {
      const vector = scope.allocate(16);
      const view = this.routes.view();
      view.setFloat32(vector, value.X, true);
      view.setFloat32(vector + 4, value.Y, true);
      view.setFloat32(vector + 8, value.Z, true);
      view.setFloat32(vector + 12, value.W, true);
      this.mem.withStringView(name, (nameView) => this.routes.invoke(
        "cna_shader_effect_set_uniform_vector4", effect, nameView, vector));
    } finally {
      scope.dispose();
    }
  }

  public override setShaderEffectUniformVector2Array(
    effect: NativeHandle, name: string, values: readonly Vector2Snapshot[],
  ): void {
    const scope = this.routes.scope();
    try {
      const stride = WASM_STRUCT_LAYOUTS.CNA_Vector2.size;
      const buffer = scope.allocate(stride * Math.max(values.length, 1));
      const view = this.routes.view();
      values.forEach((value, index) => {
        view.setFloat32(buffer + index * stride, value.X, true);
        view.setFloat32(buffer + index * stride + 4, value.Y, true);
      });
      this.mem.withStringView(name, (nameView) => this.routes.invoke(
        "cna_shader_effect_set_uniform_vector2_array", effect, nameView, buffer,
        BigInt(values.length)));
    } finally {
      scope.dispose();
    }
  }

  /** A uniform block: an array of `CNA_StringView` and a parallel array of offsets. */
  public override declareShaderEffectUniformBlock(
    effect: NativeHandle, blockSizeBytes: number, names: readonly string[],
    offsets: readonly number[],
  ): void {
    if (names.length !== offsets.length) {
      throw new RangeError(
        `a uniform block has one offset per name: ${names.length} names, ` +
        `${offsets.length} offsets`);
    }
    const scope = this.routes.scope();
    try {
      const layout = WASM_STRUCT_LAYOUTS.CNA_StringView;
      const views = scope.allocate(layout.size * Math.max(names.length, 1));
      names.forEach((name, index) => {
        const text = scope.allocateUtf8(name);
        new WasmStruct(this.routes.module, "CNA_StringView", views + layout.size * index)
          .setPointer("data", text.pointer)
          .setU64("byte_length", BigInt(text.byteLength));
      });
      const offsetBuffer = scope.allocate(4 * Math.max(offsets.length, 1));
      const view = this.routes.view();
      offsets.forEach((offset, index) => {
        view.setInt32(offsetBuffer + index * 4, Math.trunc(offset), true);
      });
      this.routes.invoke(
        "cna_shader_effect_declare_uniform_block_ext", effect, Math.trunc(blockSizeBytes),
        views, offsetBuffer, BigInt(names.length),
      );
    } finally {
      scope.dispose();
    }
  }

  // --- the skinned PBR effect's bone palette ----------------------------------------------------

  public override getSkinnedPbrEffectBoneTransforms(
    effect: NativeHandle, count: number,
  ): readonly (readonly number[])[] {
    const stride = WASM_STRUCT_LAYOUTS.CNA_Matrix.size;
    const scope = this.routes.scope();
    try {
      const destination = scope.allocate(stride * Math.max(Math.trunc(count), 1));
      const written = scope.allocate(8);
      this.routes.invoke(
        "cna_skinned_pbr_effect_copy_bone_transforms", effect, BigInt(Math.trunc(count)),
        destination, BigInt(Math.trunc(count)), written,
      );
      const view = this.routes.view();
      const total = Number(view.getBigUint64(written, true));
      return Array.from({ length: total }, (_, index) => Array.from(
        { length: 16 },
        (__, element) => view.getFloat32(destination + index * stride + element * 4, true),
      ));
    } finally {
      scope.dispose();
    }
  }

  public override setSkinnedPbrEffectBoneTransforms(
    effect: NativeHandle, transforms: readonly (readonly number[])[],
  ): void {
    const scope = this.routes.scope();
    try {
      const stride = WASM_STRUCT_LAYOUTS.CNA_Matrix.size;
      const buffer = scope.allocate(stride * Math.max(transforms.length, 1));
      const view = this.routes.view();
      transforms.forEach((matrix, index) => {
        if (matrix.length !== 16) {
          throw new RangeError(`bone ${index} is ${matrix.length} floats, not sixteen`);
        }
        for (let element = 0; element < 16; element += 1) {
          view.setFloat32(buffer + index * stride + element * 4, matrix[element] as number, true);
        }
      });
      this.routes.invoke(
        "cna_skinned_pbr_effect_set_bone_transforms", effect, buffer,
        BigInt(transforms.length));
    } finally {
      scope.dispose();
    }
  }


  #writeSphere(scope: WasmScope, sphere: BoundingSphereSnapshot): number {
    const layout = WASM_STRUCT_LAYOUTS.CNA_BoundingSphere;
    const pointer = scope.allocate(layout.size);
    const view = this.routes.view();
    view.setFloat32(pointer + layout.fields.center.offset, sphere.Center.X, true);
    view.setFloat32(pointer + layout.fields.center.offset + 4, sphere.Center.Y, true);
    view.setFloat32(pointer + layout.fields.center.offset + 8, sphere.Center.Z, true);
    view.setFloat32(pointer + layout.fields.radius.offset, sphere.Radius, true);
    return pointer;
  }















  /** A variable-length `float` array, which is not a matrix however alike their types look. */
  #floats(scope: WasmScope, values: readonly number[]): number {
    const buffer = scope.allocate(4 * Math.max(values.length, 1));
    const view = this.routes.view();
    values.forEach((value, index) => view.setFloat32(buffer + index * 4, value, true));
    return buffer;
  }


}
