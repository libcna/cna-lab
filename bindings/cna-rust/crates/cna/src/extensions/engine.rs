//! What remains of CNA's engine-layer surface after ABI 0.30: the debug-line
//! drawer, the image-based-light and indirect-draw argument values, and the
//! standalone model mesh part.
//!
//! None of this is XNA. CNA ABI 0.30 (`MOD-RETIRE-1`) removed `engine_layer.h`
//! -- the render pipeline, post-process chain, shadow maps, clustered
//! lighting, light probes, particles, compute and the optional PBR material --
//! and this module's bindings for those routes went with it. What stays is
//! what `graphics_ext.h` and the model extension routes still export.
//! `cna_image_based_light_ext_init` and the two indirect-draw `_init` routes
//! survive without a consumer route in 0.35; their values are kept because the
//! routes are still exported and still describe a layout a caller can check.
//!
//! ## Ownership
//!
//! [`DebugDraw`] and [`NativeMeshPart`] are `OWNED`: each holds a handle it
//! releases exactly once. CNA counts both against the parent game's owned
//! children and refuses to destroy a game while one is outstanding, so each
//! also registers with its device: whichever comes first, the value's own
//! `Drop` or the device's shutdown, releases it, and the second finds nothing
//! to do.

#![allow(clippy::missing_errors_doc)]

use std::sync::{Arc, Mutex};

use cna_sys as sys;

use crate::error::{CnaError, Result};
use crate::graphics::{
    GraphicsDevice, IndexBuffer, OwnedEngineChild, PrimitiveType, Texture2D, TextureCube,
    VertexBuffer, VertexPositionColor,
};
use crate::native::Native;
use crate::value::{BoundingBox, BoundingFrustum, BoundingSphere, Color, Matrix, Vector3};

/// One owned engine handle, released exactly once.
///
/// The handle lives behind a mutex because two paths can release it and only
/// one may call CNA: the value's own `Drop`, and the device shutdown that has
/// to happen before CNA will destroy the game.
struct EngineHandle {
    native: Arc<Native>,
    handle: Mutex<sys::CNA_Handle>,
    destroy: unsafe extern "C" fn(sys::CNA_Handle) -> sys::CNA_Result,
    released: &'static str,
}

impl EngineHandle {
    fn get(&self) -> Result<sys::CNA_Handle> {
        let handle = *self
            .handle
            .lock()
            .unwrap_or_else(std::sync::PoisonError::into_inner);
        if handle == sys::CNA_INVALID_HANDLE {
            return Err(CnaError::InvalidInput(self.released));
        }
        Ok(handle)
    }

    /// Releases the handle, keeping it when CNA refuses to destroy it.
    ///
    /// The refusal is a reachable state, not a theoretical one: upstream
    /// declines to destroy an object while a counted borrow taken from it is
    /// still outstanding. Clearing the slot first and reporting the error
    /// afterwards would drop the only handle anyone had to a live native
    /// object -- every later call would answer "has been released" and the
    /// process would abort at exit with the child still owned. So the slot is
    /// cleared only once the destroy has actually succeeded.
    fn release(&self) -> Result<()> {
        let mut guard = self
            .handle
            .lock()
            .unwrap_or_else(std::sync::PoisonError::into_inner);
        let handle = *guard;
        if handle == sys::CNA_INVALID_HANDLE {
            return Ok(());
        }
        // SAFETY: the handle was published by this object's own create route
        // and is released exactly once, here -- the slot is cleared only on
        // success, so a refused destroy leaves it callable rather than lost.
        self.native.check(unsafe { (self.destroy)(handle) })?;
        *guard = sys::CNA_INVALID_HANDLE;
        Ok(())
    }
}

impl OwnedEngineChild for EngineHandle {
    fn release_native(&self) -> Result<()> {
        self.release()
    }
}

pub(crate) fn matrix_to_native(value: Matrix) -> sys::CNA_Matrix {
    native_matrix(value)
}

pub(crate) fn matrix_from_native(value: sys::CNA_Matrix) -> Matrix {
    from_native_matrix(value)
}

fn native_matrix(value: Matrix) -> sys::CNA_Matrix {
    sys::CNA_Matrix {
        m11: value.M11,
        m12: value.M12,
        m13: value.M13,
        m14: value.M14,
        m21: value.M21,
        m22: value.M22,
        m23: value.M23,
        m24: value.M24,
        m31: value.M31,
        m32: value.M32,
        m33: value.M33,
        m34: value.M34,
        m41: value.M41,
        m42: value.M42,
        m43: value.M43,
        m44: value.M44,
    }
}

fn from_native_matrix(value: sys::CNA_Matrix) -> Matrix {
    Matrix::new(
        value.m11, value.m12, value.m13, value.m14, value.m21, value.m22, value.m23, value.m24,
        value.m31, value.m32, value.m33, value.m34, value.m41, value.m42, value.m43, value.m44,
    )
}

fn native_vector3(value: Vector3) -> sys::CNA_Vector3 {
    sys::CNA_Vector3 {
        x: value.X,
        y: value.Y,
        z: value.Z,
    }
}

fn from_native_vector3(value: sys::CNA_Vector3) -> Vector3 {
    Vector3::from_x_and_y_and_z(value.x, value.y, value.z)
}

fn native_bounds(value: BoundingBox) -> sys::CNA_BoundingBox {
    sys::CNA_BoundingBox {
        min: native_vector3(value.Min),
        max: native_vector3(value.Max),
    }
}

fn native_color(value: Color) -> sys::CNA_Color {
    sys::CNA_Color {
        r: value.R(),
        g: value.G(),
        b: value.B(),
        a: value.A(),
    }
}

/// Immediate-mode line drawing for diagnostics.
///
/// `OWNED`. Everything it draws is a line list, so
/// [`DebugDraw::line_count`] and [`DebugDraw::vertices`] are exact values: a
/// box is twelve lines, a cross is three, and a sphere is however many
/// segments it was asked for, three times over.
pub struct DebugDraw {
    core: Arc<EngineHandle>,
    native: Arc<Native>,
}

impl DebugDraw {
    /// Creates the debug drawer on a device.
    pub fn new(device: &GraphicsDevice) -> Result<Self> {
        let native = device.state_native();
        let mut handle = sys::CNA_INVALID_HANDLE;
        // SAFETY: the device handle is live and the output is a live local.
        native.check(unsafe { (native.engine.debug_draw_create)(device.handle()?, &mut handle) })?;
        let core = Arc::new(EngineHandle {
            native: Arc::clone(native),
            handle: Mutex::new(handle),
            destroy: native.engine.debug_draw_destroy,
            released: "the debug drawer has been released",
        });
        let child: Arc<dyn OwnedEngineChild> = Arc::clone(&core) as Arc<dyn OwnedEngineChild>;
        device.register_engine_child(&child);
        Ok(Self {
            core,
            native: Arc::clone(native),
        })
    }

    /// Opens a batch for a camera.
    pub fn begin(&self, view: Matrix, projection: Matrix) -> Result<()> {
        let handle = self.core.get()?;
        let view = native_matrix(view);
        let projection = native_matrix(projection);
        // SAFETY: the handle is owned and both matrices are borrowed for the call.
        self.native
            .check(unsafe { (self.native.engine.debug_draw_begin)(handle, &view, &projection) })
    }

    /// Draws the batch and closes it.
    pub fn end(&self) -> Result<()> {
        let handle = self.core.get()?;
        // SAFETY: the handle is owned.
        self.native
            .check(unsafe { (self.native.engine.debug_draw_end)(handle) })
    }

    /// Discards everything queued without drawing it.
    pub fn clear(&self) -> Result<()> {
        let handle = self.core.get()?;
        // SAFETY: the handle is owned.
        self.native
            .check(unsafe { (self.native.engine.debug_draw_clear)(handle) })
    }

    /// How many lines are queued.
    pub fn line_count(&self) -> Result<i32> {
        let handle = self.core.get()?;
        let mut value = 0_i32;
        // SAFETY: the handle is owned and the output is a live local.
        self.native
            .check(unsafe { (self.native.engine.debug_draw_get_line_count)(handle, &mut value) })?;
        Ok(value)
    }

    /// Whether the lines are depth tested against the scene.
    pub fn is_depth_tested(&self) -> Result<bool> {
        let handle = self.core.get()?;
        let mut value: sys::CNA_Bool = 0;
        // SAFETY: the handle is owned and the output is a live local.
        self.native
            .check(unsafe { (self.native.engine.debug_draw_is_depth_tested)(handle, &mut value) })?;
        Ok(value != 0)
    }

    /// Turns depth testing on or off.
    pub fn set_depth_tested(&self, value: bool) -> Result<()> {
        let handle = self.core.get()?;
        // SAFETY: the handle is owned and the flag is a canonical boolean.
        self.native.check(unsafe {
            (self.native.engine.debug_draw_set_depth_tested)(handle, u8::from(value))
        })
    }

    /// Queues one line.
    pub fn add_line(&self, from: Vector3, to: Vector3, color: Color) -> Result<()> {
        let handle = self.core.get()?;
        let from = native_vector3(from);
        let to = native_vector3(to);
        // SAFETY: the handle is owned and both points are borrowed for the call.
        self.native.check(unsafe {
            (self.native.engine.debug_draw_add_line)(handle, &from, &to, native_color(color))
        })
    }

    /// Queues the twelve edges of a box.
    pub fn add_box(&self, bounds: BoundingBox, color: Color) -> Result<()> {
        let handle = self.core.get()?;
        let bounds = native_bounds(bounds);
        // SAFETY: the handle is owned and the bounds are borrowed for the call.
        self.native.check(unsafe {
            (self.native.engine.debug_draw_add_box)(handle, &bounds, native_color(color))
        })
    }

    /// Queues three axis-aligned segments through a point.
    pub fn add_cross(&self, centre: Vector3, size: f32, color: Color) -> Result<()> {
        let handle = self.core.get()?;
        let centre = native_vector3(centre);
        // SAFETY: the handle is owned and the point is borrowed for the call.
        self.native.check(unsafe {
            (self.native.engine.debug_draw_add_cross)(handle, &centre, size, native_color(color))
        })
    }

    /// Queues three rings approximating a sphere.
    pub fn add_sphere(
        &self,
        centre: Vector3,
        radius: f32,
        color: Color,
        segments: i32,
    ) -> Result<()> {
        let handle = self.core.get()?;
        let centre = native_vector3(centre);
        // SAFETY: the handle is owned and the point is borrowed for the call.
        self.native.check(unsafe {
            (self.native.engine.debug_draw_add_sphere)(
                handle,
                &centre,
                radius,
                native_color(color),
                segments,
            )
        })
    }

    /// Queues the same three rings around a bounding sphere.
    pub fn add_bounding_sphere(
        &self,
        sphere: BoundingSphere,
        color: Color,
        segments: i32,
    ) -> Result<()> {
        let handle = self.core.get()?;
        let sphere = sys::CNA_BoundingSphere {
            center: native_vector3(sphere.Center),
            radius: sphere.Radius,
        };
        // SAFETY: the handle is owned and the sphere is borrowed for the call.
        self.native.check(unsafe {
            (self.native.engine.debug_draw_add_bounding_sphere)(
                handle,
                &sphere,
                native_color(color),
                segments,
            )
        })
    }

    /// Queues a camera frustum's own twelve edges.
    pub fn add_frustum(&self, frustum: &BoundingFrustum, color: Color) -> Result<()> {
        let handle = self.core.get()?;
        let frustum = sys::CNA_BoundingFrustum {
            matrix: native_matrix(frustum.Matrix()),
        };
        // SAFETY: the handle is owned and the frustum is by value.
        self.native.check(unsafe {
            (self.native.engine.debug_draw_add_frustum)(handle, frustum, native_color(color))
        })
    }

    /// The queued lines as vertices, ready to draw elsewhere.
    ///
    /// `depth_tested` selects which of the two queues to read: the drawer keeps
    /// them apart because they need different device state, and a caller
    /// reading only one and finding it short would otherwise have no way to
    /// tell which.
    pub fn vertices(&self, depth_tested: bool) -> Result<Vec<VertexPositionColor>> {
        let handle = self.core.get()?;
        let mut required = 0_u64;
        // SAFETY: a null destination with zero capacity asks for the count.
        let probe = unsafe {
            (self.native.engine.debug_draw_copy_vertices)(
                handle,
                u8::from(depth_tested),
                core::ptr::null_mut(),
                0,
                &mut required,
            )
        };
        if probe != sys::CNA_RESULT_SUCCESS && probe != sys::CNA_RESULT_BUFFER_TOO_SMALL {
            self.native.check(probe)?;
        }
        let capacity = usize::try_from(required)
            .map_err(|_| CnaError::InvalidInput("the vertex count does not fit in memory"))?;
        if capacity == 0 {
            return Ok(Vec::new());
        }
        let mut buffer = vec![sys::CNA_VertexPositionColor::default(); capacity];
        let mut count = 0_u64;
        // SAFETY: the handle is owned and the destination holds `capacity`
        // writable vertices, which is the count passed alongside it.
        self.native.check(unsafe {
            (self.native.engine.debug_draw_copy_vertices)(
                handle,
                u8::from(depth_tested),
                buffer.as_mut_ptr(),
                required,
                &mut count,
            )
        })?;
        let count = usize::try_from(count)
            .map_err(|_| CnaError::InvalidInput("CNA reported more vertices than fit in memory"))?;
        Ok(buffer
            .into_iter()
            .take(count.min(capacity))
            .map(|vertex| VertexPositionColor {
                Position: from_native_vector3(vertex.position),
                Color: Color::from_r_and_g_and_b_and_a_as_int32_and_int32_and_int32_and_int32(
                    i32::from(vertex.color.r),
                    i32::from(vertex.color.g),
                    i32::from(vertex.color.b),
                    i32::from(vertex.color.a),
                ),
            })
            .collect())
    }

    /// Releases the drawer now rather than at drop.
    pub fn release(&self) -> Result<()> {
        self.core.release()
    }
}

impl Drop for DebugDraw {
    fn drop(&mut self) {
        let _ = self.core.release();
    }
}

/// The three textures a PBR shader needs to shade from an environment, and how
/// bright they are.
///
/// The textures are `BORROWED`: the structure records them and never owns them,
/// so this value holds the Rust resources that keep them alive for exactly as
/// long as it names them.
pub struct ImageBasedLight {
    irradiance: Option<TextureCube>,
    prefiltered_specular: Option<TextureCube>,
    brdf_lut: Option<Texture2D>,
    prefiltered_mip_count: i32,
    intensity: f32,
    native: Arc<Native>,
}

impl ImageBasedLight {
    /// CNA's own defaults, asked of the library rather than restated here.
    ///
    /// The three textures start unbound, which is exactly the state
    /// [`is_valid`](Self::is_valid) answers `false` for.
    pub fn canonical_defaults() -> Result<Self> {
        let native = Native::process()?;
        let mut value = sys::CNA_ImageBasedLightEXT::default();
        // SAFETY: the structure is a caller-owned versioned output.
        native.check(unsafe { (native.engine.image_based_light_ext_init)(&mut value) })?;
        Ok(Self {
            irradiance: None,
            prefiltered_specular: None,
            brdf_lut: None,
            prefiltered_mip_count: value.prefiltered_mip_count,
            intensity: value.intensity,
            native,
        })
    }

    /// Gives the light its diffuse irradiance cube.
    pub fn set_irradiance(&mut self, value: Option<TextureCube>) {
        self.irradiance = value;
    }

    /// Gives the light its prefiltered specular cube and that cube's mip count.
    ///
    /// The two go together on purpose: pairing a cube with a mip count from a
    /// different one is the failure this structure exists to prevent.
    pub fn set_prefiltered_specular(&mut self, value: Option<TextureCube>, mip_count: i32) {
        self.prefiltered_specular = value;
        self.prefiltered_mip_count = mip_count;
    }

    /// Gives the light its BRDF lookup table.
    pub fn set_brdf_lut(&mut self, value: Option<Texture2D>) {
        self.brdf_lut = value;
    }

    /// How bright the light is.
    #[must_use]
    pub const fn intensity(&self) -> f32 {
        self.intensity
    }

    /// Sets it.
    pub const fn set_intensity(&mut self, value: f32) {
        self.intensity = value;
    }

    /// How many mip levels the prefiltered cube has.
    #[must_use]
    pub const fn prefiltered_mip_count(&self) -> i32 {
        self.prefiltered_mip_count
    }

    /// Whether the light is complete enough to shade with.
    ///
    /// All three textures must be present and the mip count at least one. A
    /// light that is *nearly* complete is the failure this answers: it does not
    /// look like a mismatch, it looks like a scene lit slightly wrong.
    pub fn is_valid(&self) -> Result<bool> {
        let value = self.to_native()?;
        let mut valid = 0_u8;
        // SAFETY: the structure is borrowed for the call and the output is a
        // live local.
        self.native.check(unsafe {
            (self.native.engine.image_based_light_ext_is_valid)(&value, &mut valid)
        })?;
        Ok(valid != 0)
    }

    fn to_native(&self) -> Result<sys::CNA_ImageBasedLightEXT> {
        Ok(sys::CNA_ImageBasedLightEXT {
            struct_size: core::mem::size_of::<sys::CNA_ImageBasedLightEXT>() as u32,
            struct_version: 1,
            irradiance: match self.irradiance.as_ref() {
                Some(texture) => texture.native_handle()?,
                None => sys::CNA_INVALID_HANDLE,
            },
            prefiltered_specular: match self.prefiltered_specular.as_ref() {
                Some(texture) => texture.native_handle()?,
                None => sys::CNA_INVALID_HANDLE,
            },
            brdf_lut: match self.brdf_lut.as_ref() {
                Some(texture) => texture.handle()?,
                None => sys::CNA_INVALID_HANDLE,
            },
            prefiltered_mip_count: self.prefiltered_mip_count,
            intensity: self.intensity,
        })
    }
}

/// The arguments a non-indexed indirect draw reads out of a buffer.
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
#[non_exhaustive]
pub struct IndirectDrawArguments {
    /// How many vertices to fetch.
    pub vertex_count: u32,
    /// How many instances to draw; one for an ordinary draw, zero to draw
    /// nothing.
    pub instance_count: u32,
    /// The first vertex, in elements of the bound stream.
    pub first_vertex: u32,
    /// The first instance.
    ///
    /// **Must be zero on GL ES.** ES 3.1 has no base-instance parameter and the
    /// word is required to be zero; a non-zero value there is undefined rather
    /// than diagnosed, and cannot be checked anywhere -- by the time the draw
    /// runs the value lives in GPU memory.
    pub base_instance: u32,
}

impl IndirectDrawArguments {
    /// CNA's own defaults, asked of the library rather than restated here.
    pub fn canonical_defaults() -> Result<Self> {
        let native = Native::process()?;
        let mut value = sys::CNA_IndirectDrawArguments::default();
        // SAFETY: the structure is a caller-owned output.
        native.check(unsafe { (native.engine.indirect_draw_arguments_init)(&mut value) })?;
        Ok(Self {
            vertex_count: value.vertex_count,
            instance_count: value.instance_count,
            first_vertex: value.first_vertex,
            base_instance: value.base_instance,
        })
    }

    /// The four words as a GPU buffer holds them.
    #[must_use]
    pub const fn to_words(self) -> [u32; 4] {
        [
            self.vertex_count,
            self.instance_count,
            self.first_vertex,
            self.base_instance,
        ]
    }
}

/// The arguments an indexed indirect draw reads out of a buffer.
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
#[non_exhaustive]
pub struct IndirectDrawIndexedArguments {
    /// How many indices to fetch.
    pub index_count: u32,
    /// How many instances to draw.
    pub instance_count: u32,
    /// The first index, in index elements.
    pub first_index: u32,
    /// Added to every decoded index, in vertex elements; signed, as the API is.
    pub base_vertex: i32,
    /// The first instance; must be zero on GL ES, for the reason
    /// [`IndirectDrawArguments::base_instance`] gives.
    pub base_instance: u32,
}

impl IndirectDrawIndexedArguments {
    /// CNA's own defaults, asked of the library rather than restated here.
    pub fn canonical_defaults() -> Result<Self> {
        let native = Native::process()?;
        let mut value = sys::CNA_IndirectDrawIndexedArguments::default();
        // SAFETY: the structure is a caller-owned output.
        native.check(unsafe { (native.engine.indirect_draw_indexed_arguments_init)(&mut value) })?;
        Ok(Self {
            index_count: value.index_count,
            instance_count: value.instance_count,
            first_index: value.first_index,
            base_vertex: value.base_vertex,
            base_instance: value.base_instance,
        })
    }

    /// The five words as a GPU buffer holds them, the base vertex reinterpreted
    /// as the signed value the API reads it back as.
    #[must_use]
    pub const fn to_words(self) -> [u32; 5] {
        [
            self.index_count,
            self.instance_count,
            self.first_index,
            self.base_vertex as u32,
            self.base_instance,
        ]
    }
}

/// A mesh part CNA owns.
///
/// `OWNED`. Deliberately **not** the crate's
/// [`ModelMeshPart`](crate::Microsoft::Xna::Framework::Graphics::ModelMeshPart),
/// which is a managed Rust projection with no native handle: the skinned-model
/// and morph-target extension routes take a `CNA_ModelMeshPartHandle`, and
/// nothing in the XNA projection can produce one. This type exists so those routes are reachable without publishing a raw
/// handle, and it lives in `cna::extensions` because it is not XNA.
///
/// Its two buffers are `RETAINED_DEPENDENCY`: CNA holds them by pointer, so
/// this value keeps the Rust resources alive for exactly as long as the part
/// names them.
pub struct NativeMeshPart {
    core: Arc<EngineHandle>,
    native: Arc<Native>,
    vertex_buffer: Option<VertexBuffer>,
    index_buffer: Option<IndexBuffer>,
}

impl NativeMeshPart {
    /// Creates a part over a vertex and index buffer.
    ///
    /// The four counts are preserved verbatim -- CNA validates nothing here --
    /// so they are what the drawing routes will believe.
    pub fn new(
        vertex_buffer: Option<VertexBuffer>,
        index_buffer: Option<IndexBuffer>,
        num_vertices: i32,
        primitive_count: i32,
        start_index: i32,
        vertex_offset: i32,
    ) -> Result<Self> {
        let native = Native::process()?;
        let vertex_handle = match vertex_buffer.as_ref() {
            Some(buffer) => buffer.handle()?,
            None => sys::CNA_INVALID_HANDLE,
        };
        let index_handle = match index_buffer.as_ref() {
            Some(buffer) => buffer.handle()?,
            None => sys::CNA_INVALID_HANDLE,
        };
        let mut handle = sys::CNA_INVALID_HANDLE;
        // SAFETY: both buffer handles are live for the call and the output is a
        // live local.
        native.check(unsafe {
            (native.engine.model_mesh_part_create)(
                vertex_handle,
                index_handle,
                num_vertices,
                primitive_count,
                start_index,
                vertex_offset,
                &mut handle,
            )
        })?;
        Ok(Self {
            core: Arc::new(EngineHandle {
                native: Arc::clone(&native),
                handle: Mutex::new(handle),
                destroy: native.engine.model_mesh_part_destroy,
                released: "the native mesh part has been released",
            }),
            native,
            vertex_buffer,
            index_buffer,
        })
    }

    fn count(
        &self,
        route: unsafe extern "C" fn(sys::CNA_Handle, *mut i32) -> sys::CNA_Result,
    ) -> Result<i32> {
        let handle = self.core.get()?;
        let mut value = 0_i32;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe { route(handle, &mut value) })?;
        Ok(value)
    }

    /// How many vertices the part draws from.
    pub fn num_vertices(&self) -> Result<i32> {
        self.count(self.native.engine.model_mesh_part_get_num_vertices)
    }

    /// How many primitives it draws.
    pub fn primitive_count(&self) -> Result<i32> {
        self.count(self.native.engine.model_mesh_part_get_primitive_count)
    }

    /// Where in the index buffer it starts.
    pub fn start_index(&self) -> Result<i32> {
        self.count(self.native.engine.model_mesh_part_get_start_index)
    }

    /// What is added to every decoded index.
    pub fn vertex_offset(&self) -> Result<i32> {
        self.count(self.native.engine.model_mesh_part_get_vertex_offset)
    }

    /// What the part draws.
    pub fn primitive_type(&self) -> Result<PrimitiveType> {
        let handle = self.core.get()?;
        let mut value: sys::CNA_PrimitiveType = 0;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.engine.model_mesh_part_get_primitive_type_ext)(handle, &mut value)
        })?;
        primitive_type_from_native(value)
    }

    /// Sets it.
    pub fn set_primitive_type(&self, value: PrimitiveType) -> Result<()> {
        let handle = self.core.get()?;
        // SAFETY: the handle is owned and the identity is canonical.
        self.native.check(unsafe {
            (self.native.engine.model_mesh_part_set_primitive_type_ext)(handle, value as u32)
        })
    }

    /// The vertex buffer this value is keeping alive for CNA.
    #[must_use]
    pub const fn vertex_buffer(&self) -> Option<&VertexBuffer> {
        self.vertex_buffer.as_ref()
    }

    /// The index buffer this value is keeping alive for CNA.
    #[must_use]
    pub const fn index_buffer(&self) -> Option<&IndexBuffer> {
        self.index_buffer.as_ref()
    }

    /// Releases the part now rather than at drop.
    pub fn release(&self) -> Result<()> {
        self.core.release()
    }

    pub(crate) fn native_handle(&self) -> Result<sys::CNA_ModelMeshPartHandle> {
        self.core.get()
    }

    pub(crate) fn api(&self) -> &Arc<Native> {
        &self.native
    }
}

impl Drop for NativeMeshPart {
    fn drop(&mut self) {
        let _ = self.core.release();
    }
}

/// The XNA identity of a native primitive type.
fn primitive_type_from_native(value: sys::CNA_PrimitiveType) -> Result<PrimitiveType> {
    Ok(match value {
        0 => PrimitiveType::TriangleList,
        1 => PrimitiveType::TriangleStrip,
        2 => PrimitiveType::LineList,
        3 => PrimitiveType::LineStrip,
        _ => return Err(CnaError::InvalidInput("native primitive type is unknown")),
    })
}

