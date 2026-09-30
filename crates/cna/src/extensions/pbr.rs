//! CNA's physically based effects: `PbrEffect` and `SkinnedPbrEffect`.
//!
//! None of this is XNA. `BasicEffect` had a diffuse colour and a specular
//! power; there is no metallic factor, no roughness, no index of refraction
//! and no HDR anywhere in `Microsoft.Xna.Framework.Graphics`. Putting any of it
//! there would mean declaring members Microsoft never did, so it lives here.
//!
//! The effects are core CNA routes in `effects.h`. The material value, the
//! material extensions, the glTF material bridge and the pipeline settings
//! that used to sit beside them were engine-layer routes, and CNA ABI 0.30
//! removed them; their bindings went with them.

#![allow(clippy::missing_errors_doc)]

use std::sync::Arc;

use cna_sys as sys;

use crate::error::{CnaError, Result};
use crate::graphics::{GraphicsDevice, Texture2D};
use crate::native::Native;
use crate::value::{Matrix, Vector3};

/// How a material's alpha is interpreted.
#[derive(Clone, Copy, Debug, Default, Eq, Hash, Ord, PartialEq, PartialOrd)]
#[non_exhaustive]
pub enum AlphaMode {
    #[default]
    Opaque,
    /// Alpha-tested against a cutoff.
    Mask,
    Blend,
}

impl AlphaMode {
    const fn from_native(value: sys::CNA_AlphaModeEXT) -> Option<Self> {
        Some(match value {
            sys::CNA_ALPHA_MODE_OPAQUE_EXT => Self::Opaque,
            sys::CNA_ALPHA_MODE_MASK_EXT => Self::Mask,
            sys::CNA_ALPHA_MODE_BLEND_EXT => Self::Blend,
            _ => return None,
        })
    }

    const fn to_native(self) -> sys::CNA_AlphaModeEXT {
        match self {
            Self::Opaque => sys::CNA_ALPHA_MODE_OPAQUE_EXT,
            Self::Mask => sys::CNA_ALPHA_MODE_MASK_EXT,
            Self::Blend => sys::CNA_ALPHA_MODE_BLEND_EXT,
        }
    }
}

/// A physically based effect owned by this value.
///
/// Needs the engine layer. Construction is where that is discovered: a library
/// without it refuses here rather than at the first property set.
pub struct PbrEffect {
    native: Arc<Native>,
    handle: sys::CNA_EffectHandle,
    device: GraphicsDevice,
}

macro_rules! scalar_property {
    ($get:ident, $set:ident, $native_get:ident, $native_set:ident, $type:ty, $doc:literal) => {
        #[doc = $doc]
        pub fn $get(&self) -> Result<$type> {
            let mut value = <$type>::default();
            // SAFETY: the handle is owned and the output is a live local.
            self.native
                .check(unsafe { (self.native.runtime.$native_get)(self.handle, &mut value) })?;
            Ok(value)
        }

        #[doc = $doc]
        pub fn $set(&self, value: $type) -> Result<()> {
            // SAFETY: the handle is owned and the value is by value.
            self.native
                .check(unsafe { (self.native.runtime.$native_set)(self.handle, value) })
        }
    };
}

impl PbrEffect {
    /// Creates an effect on a device.
    pub fn new(device: &GraphicsDevice) -> Result<Self> {
        let native = device.state_native();
        let mut handle = sys::CNA_INVALID_HANDLE;
        // SAFETY: the device handle is live and the output is a live local.
        native.check(unsafe {
            (native.runtime.pbr_effect_create)(device.handle()?, &mut handle)
        })?;
        Ok(Self {
            native: Arc::clone(native),
            handle,
            device: device.clone(),
        })
    }

    /// The device this effect belongs to.
    #[must_use]
    pub const fn graphics_device(&self) -> &GraphicsDevice {
        &self.device
    }

    scalar_property!(
        metallic_factor, set_metallic_factor,
        pbr_effect_get_metallic_factor, pbr_effect_set_metallic_factor, f32,
        "How metallic the surface is, from 0 through 1."
    );
    scalar_property!(
        roughness_factor, set_roughness_factor,
        pbr_effect_get_roughness_factor, pbr_effect_set_roughness_factor, f32,
        "How rough the surface is, from 0 through 1."
    );
    scalar_property!(
        alpha, set_alpha,
        pbr_effect_get_alpha, pbr_effect_set_alpha, f32,
        "Material opacity."
    );
    scalar_property!(
        alpha_cutoff, set_alpha_cutoff,
        pbr_effect_get_alpha_cutoff, pbr_effect_set_alpha_cutoff, f32,
        "The threshold `AlphaMode::Mask` tests against."
    );
    scalar_property!(
        normal_scale, set_normal_scale,
        pbr_effect_get_normal_scale, pbr_effect_set_normal_scale, f32,
        "Normal-map intensity, where 1 is full strength."
    );
    scalar_property!(
        occlusion_strength, set_occlusion_strength,
        pbr_effect_get_occlusion_strength, pbr_effect_set_occlusion_strength, f32,
        "Ambient-occlusion strength, from 0 through 1."
    );
    scalar_property!(
        ior, set_ior,
        pbr_effect_get_ior, pbr_effect_set_ior, f32,
        "Index of refraction."
    );
    scalar_property!(
        specular_factor, set_specular_factor,
        pbr_effect_get_specular_factor, pbr_effect_set_specular_factor, f32,
        "Specular strength."
    );

    /// The albedo (base colour) factor.
    pub fn diffuse_color(&self) -> Result<Vector3> {
        let mut value = sys::CNA_Vector3::default();
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_get_diffuse_color)(self.handle, &mut value)
        })?;
        Ok(Vector3 {
            X: value.x,
            Y: value.y,
            Z: value.z,
        })
    }

    /// Sets the albedo (base colour) factor.
    pub fn set_diffuse_color(&self, value: Vector3) -> Result<()> {
        let native_value = sys::CNA_Vector3 {
            x: value.X,
            y: value.Y,
            z: value.Z,
        };
        // SAFETY: the vector is passed by value.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_set_diffuse_color)(self.handle, native_value)
        })
    }

    /// The emissive factor.
    pub fn emissive_factor(&self) -> Result<Vector3> {
        let mut value = sys::CNA_Vector3::default();
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_get_emissive_factor)(self.handle, &mut value)
        })?;
        Ok(Vector3 {
            X: value.x,
            Y: value.y,
            Z: value.z,
        })
    }

    /// Sets the emissive factor.
    pub fn set_emissive_factor(&self, value: Vector3) -> Result<()> {
        let native_value = sys::CNA_Vector3 {
            x: value.X,
            y: value.Y,
            z: value.Z,
        };
        // SAFETY: the vector is passed by value.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_set_emissive_factor)(self.handle, native_value)
        })
    }

    /// How the material's alpha is interpreted.
    pub fn alpha_mode(&self) -> Result<AlphaMode> {
        let mut value = 0;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_get_alpha_mode)(self.handle, &mut value)
        })?;
        AlphaMode::from_native(value).ok_or(CnaError::UnsupportedRuntime(
            "CNA named an alpha mode this build does not know",
        ))
    }

    /// Sets how the material's alpha is interpreted.
    pub fn set_alpha_mode(&self, value: AlphaMode) -> Result<()> {
        // SAFETY: the identity is checked and passed by value.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_set_alpha_mode)(self.handle, value.to_native())
        })
    }

    /// Whether the surface is rendered from both sides.
    pub fn double_sided(&self) -> Result<bool> {
        let mut value = sys::CNA_FALSE;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_get_double_sided)(self.handle, &mut value)
        })?;
        Ok(value != sys::CNA_FALSE)
    }

    /// Sets whether the surface is rendered from both sides.
    pub fn set_double_sided(&self, value: bool) -> Result<()> {
        // SAFETY: the flag is passed by value.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_set_double_sided)(self.handle, u8::from(value))
        })
    }

    /// Whether the effect samples per-vertex colour.
    pub fn vertex_color_enabled(&self) -> Result<bool> {
        let mut value = sys::CNA_FALSE;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_get_vertex_color_enabled)(self.handle, &mut value)
        })?;
        Ok(value != sys::CNA_FALSE)
    }

    /// Sets whether the effect samples per-vertex colour.
    pub fn set_vertex_color_enabled(&self, value: bool) -> Result<()> {
        // SAFETY: the flag is passed by value.
        self.native.check(unsafe {
            (self.native.runtime.pbr_effect_set_vertex_color_enabled)(self.handle, u8::from(value))
        })
    }
}

impl Drop for PbrEffect {
    fn drop(&mut self) {
        // SAFETY: the handle is owned by this value and released exactly once.
        let _ = unsafe { (self.native.effect_destroy)(self.handle) };
    }
}

/// How one texture's coordinates are transformed before sampling.
///
/// glTF's `KHR_texture_transform`, per slot.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct TextureTransform {
    /// Translation, applied after scaling and rotation.
    pub offset: (f32, f32),
    /// Per-axis scale.
    pub scale: (f32, f32),
    /// Counter-clockwise rotation, in radians.
    pub rotation: f32,
}

impl TextureTransform {
    const fn from_native(value: sys::CNA_TextureTransformEXT) -> Self {
        Self {
            offset: (value.offset.x, value.offset.y),
            scale: (value.scale.x, value.scale.y),
            rotation: value.rotation,
        }
    }

    const fn to_native(self) -> sys::CNA_TextureTransformEXT {
        sys::CNA_TextureTransformEXT {
            struct_size: core::mem::size_of::<sys::CNA_TextureTransformEXT>() as u32,
            struct_version: 1,
            offset: sys::CNA_Vector2 {
                x: self.offset.0,
                y: self.offset.1,
            },
            scale: sys::CNA_Vector2 {
                x: self.scale.0,
                y: self.scale.1,
            },
            rotation: self.rotation,
        }
    }
}

/// The number of per-slot state entries a material carries.
///
/// Seven, in the importer's own order -- base colour, normal,
/// metallic-roughness, occlusion, emissive, specular, specular colour. This is
/// deliberately **not** the same as the eight texture *names*
/// [`CnbMaterialTexture`](crate::extensions::content::CnbMaterialTexture)
/// addresses, which include `DualTextureEffect`'s second layer; upstream warns
/// that confusing the two index spaces is a real trap, so they are separate
/// types here and neither can be passed where the other belongs.
pub const TEXTURE_SLOT_COUNT: usize = 7;

/// A material's per-slot state entry.
#[derive(Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
#[non_exhaustive]
pub enum TextureSlot {
    BaseColor,
    Normal,
    MetallicRoughness,
    Occlusion,
    Emissive,
    Specular,
    SpecularColor,
}

impl TextureSlot {
    /// Every slot, in the ABI's own order.
    ///
    /// This is the order `CNA_PbrMaterialEXT::texture_coordinate_sets` and
    /// `::texture_transforms` are indexed in, and the order the
    /// `CNA_PBR_TEXTURE_*` identities are numbered in: base colour, normal,
    /// metallic-roughness, **emissive, occlusion**, specular, specular colour.
    ///
    /// Emissive before occlusion, which is not the order this enum's variants
    /// happen to be declared in and not the order a reader expects. Getting it
    /// the other way round silently reads and writes the wrong slot, which is
    /// what it used to do.
    pub const ALL: [Self; TEXTURE_SLOT_COUNT] = [
        Self::BaseColor,
        Self::Normal,
        Self::MetallicRoughness,
        Self::Emissive,
        Self::Occlusion,
        Self::Specular,
        Self::SpecularColor,
    ];

    /// This slot's position in the ABI's per-slot arrays.
    ///
    /// The same number as the slot's `CNA_PBR_TEXTURE_*` identity, which is why
    /// [`Self::to_native`] is this cast rather than a second table that could
    /// drift from it.
    const fn index(self) -> usize {
        match self {
            Self::BaseColor => 0,
            Self::Normal => 1,
            Self::MetallicRoughness => 2,
            Self::Emissive => 3,
            Self::Occlusion => 4,
            Self::Specular => 5,
            Self::SpecularColor => 6,
        }
    }

    /// The `CNA_PBR_TEXTURE_*` identity for this slot.
    pub(crate) const fn to_native(self) -> sys::CNA_PbrTextureSlot {
        self.index() as sys::CNA_PbrTextureSlot
    }
}

impl PbrEffect {
}

/// A physically based effect that also skins its vertices.
///
/// `OWNED`. The same material model as [`PbrEffect`], plus the bone palette an
/// animated mesh needs -- and CNA refuses the material routes when the handle
/// is the wrong kind of effect, which is why the two are separate types here
/// rather than one with a flag.
pub struct SkinnedPbrEffect {
    native: Arc<Native>,
    handle: sys::CNA_EffectHandle,
    device: GraphicsDevice,
}

impl SkinnedPbrEffect {
    /// Creates the effect on a device.
    pub fn new(device: &GraphicsDevice) -> Result<Self> {
        let native = device.state_native();
        let mut handle = sys::CNA_INVALID_HANDLE;
        // SAFETY: the device handle is live for the call and the output is a
        // live local.
        native.check(unsafe {
            (native.runtime.skinned_pbr_effect_create)(device.handle()?, &mut handle)
        })?;
        Ok(Self {
            native: Arc::clone(native),
            handle,
            device: device.clone(),
        })
    }

    /// The device this effect belongs to.
    #[must_use]
    pub const fn graphics_device(&self) -> &GraphicsDevice {
        &self.device
    }

    /// How many bone weights each vertex carries.
    pub fn weights_per_vertex(&self) -> Result<i32> {
        let mut value = 0_i32;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.runtime.skinned_pbr_effect_get_weights_per_vertex)(self.handle, &mut value)
        })?;
        Ok(value)
    }

    /// Sets it.
    pub fn set_weights_per_vertex(&self, value: i32) -> Result<()> {
        // SAFETY: the handle is owned.
        self.native.check(unsafe {
            (self.native.runtime.skinned_pbr_effect_set_weights_per_vertex)(self.handle, value)
        })
    }

    /// Uploads the bone palette.
    pub fn set_bone_transforms(&self, transforms: &[Matrix]) -> Result<()> {
        let native_transforms: Vec<sys::CNA_Matrix> = transforms
            .iter()
            .copied()
            .map(crate::extensions::engine::matrix_to_native)
            .collect();
        // SAFETY: the handle is owned and the array is borrowed for the call
        // with its own length.
        self.native.check(unsafe {
            (self.native.runtime.skinned_pbr_effect_set_bone_transforms)(
                self.handle,
                native_transforms.as_ptr(),
                native_transforms.len() as u64,
            )
        })
    }

    /// Reads the bone palette back.
    pub fn bone_transforms(&self, requested: usize) -> Result<Vec<Matrix>> {
        let mut buffer = vec![sys::CNA_Matrix::default(); requested];
        let mut count = 0_u64;
        // SAFETY: the handle is owned and the destination holds `requested`
        // writable matrices, which is the capacity passed alongside it.
        self.native.check(unsafe {
            (self.native.runtime.skinned_pbr_effect_copy_bone_transforms)(
                self.handle,
                requested as u64,
                buffer.as_mut_ptr(),
                buffer.len() as u64,
                &mut count,
            )
        })?;
        let count = usize::try_from(count)
            .map_err(|_| CnaError::InvalidInput("CNA reported more bones than fit in memory"))?;
        Ok(buffer
            .into_iter()
            .take(count.min(requested))
            .map(crate::extensions::engine::matrix_from_native)
            .collect())
    }
}

impl Drop for SkinnedPbrEffect {
    fn drop(&mut self) {
        // SAFETY: the handle is owned by this value and released exactly once.
        // CNA counts it against the parent game's owned children and refuses to
        // destroy a game while one is outstanding, so leaving it to the process
        // would abort at shutdown rather than leak quietly.
        let _ = unsafe { (self.native.effect_destroy)(self.handle) };
    }
}

/// The per-slot texture state, on a live effect rather than in a material value.
///
/// These read and write what the effect is set to *now*, including the
/// textures themselves.
impl PbrEffect {
    /// Binds a texture to one slot, or clears it with `None`.
    pub fn set_texture(&self, slot: TextureSlot, texture: Option<&Texture2D>) -> Result<()> {
        let handle = match texture {
            Some(texture) => texture.handle()?,
            None => sys::CNA_INVALID_HANDLE,
        };
        // SAFETY: both handles belong to live values and the slot is by value.
        self.native.check(unsafe {
            (self.native.pbr_effect_set_texture)(self.handle, slot.to_native(), handle)
        })
    }

    /// Whether a slot has a texture bound, and which handle it is.
    ///
    /// The identity rather than a [`Texture2D`]: the texture is the effect's,
    /// not the caller's, and handing back an owning Rust value would promise a
    /// lifetime this side does not control. It is enough to tell two slots'
    /// textures apart and to see whether one is bound at all.
    pub fn texture_identity(&self, slot: TextureSlot) -> Result<Option<u64>> {
        let mut present = sys::CNA_FALSE;
        let mut handle = sys::CNA_INVALID_HANDLE;
        // SAFETY: the handle is owned and both outputs are live locals.
        self.native.check(unsafe {
            (self.native.pbr_effect_get_texture)(
                self.handle,
                slot.to_native(),
                &mut present,
                &mut handle,
            )
        })?;
        Ok((present != sys::CNA_FALSE).then_some(handle))
    }

    /// Which packed UV channel a slot samples.
    pub fn texture_coordinate_set(&self, slot: TextureSlot) -> Result<i32> {
        let mut value = 0_i32;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.pbr_effect_get_texture_coordinate_set_ext)(
                self.handle,
                slot.to_native(),
                &mut value,
            )
        })?;
        Ok(value)
    }

    /// Sets which packed UV channel a slot samples.
    pub fn set_texture_coordinate_set(&self, slot: TextureSlot, value: i32) -> Result<()> {
        // SAFETY: the handle is owned and both values are by value.
        self.native.check(unsafe {
            (self.native.pbr_effect_set_texture_coordinate_set_ext)(
                self.handle,
                slot.to_native(),
                value,
            )
        })
    }

    /// Whether a slot's texture is sampled as sRGB.
    ///
    /// A colour texture is; a normal map or a metallic-roughness map is not,
    /// and treating one as the other is a visible error rather than a subtle
    /// one.
    pub fn texture_is_srgb(&self, slot: TextureSlot) -> Result<bool> {
        let mut value = sys::CNA_FALSE;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.pbr_effect_get_texture_is_srgb_ext)(
                self.handle,
                slot.to_native(),
                &mut value,
            )
        })?;
        Ok(value != sys::CNA_FALSE)
    }

    /// Sets whether a slot's texture is sampled as sRGB.
    pub fn set_texture_is_srgb(&self, slot: TextureSlot, value: bool) -> Result<()> {
        // SAFETY: the handle is owned and both values are by value.
        self.native.check(unsafe {
            (self.native.pbr_effect_set_texture_is_srgb_ext)(
                self.handle,
                slot.to_native(),
                u8::from(value),
            )
        })
    }

    /// One slot's `KHR_texture_transform`.
    pub fn texture_transform(&self, slot: TextureSlot) -> Result<TextureTransform> {
        // The output's size and version headers must be set; CNA refuses a
        // zeroed one as malformed.
        let mut value = sys::CNA_TextureTransformEXT {
            struct_size: core::mem::size_of::<sys::CNA_TextureTransformEXT>() as u32,
            struct_version: 1,
            ..sys::CNA_TextureTransformEXT::default()
        };
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.pbr_effect_get_texture_transform_ext)(
                self.handle,
                slot.to_native(),
                &mut value,
            )
        })?;
        Ok(TextureTransform::from_native(value))
    }

    /// Sets one slot's `KHR_texture_transform`.
    pub fn set_texture_transform(
        &self,
        slot: TextureSlot,
        value: TextureTransform,
    ) -> Result<()> {
        let native_value = value.to_native();
        // SAFETY: the handle is owned and the transform outlives the call.
        self.native.check(unsafe {
            (self.native.pbr_effect_set_texture_transform_ext)(
                self.handle,
                slot.to_native(),
                &native_value,
            )
        })
    }

    /// Whether the effect encodes its output to sRGB.
    ///
    /// A pipeline that already writes to an sRGB render target must leave this
    /// off, or the encode happens twice and everything washes out.
    pub fn encode_output_to_srgb(&self) -> Result<bool> {
        let mut value = sys::CNA_FALSE;
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.pbr_effect_get_encode_output_to_srgb_ext)(self.handle, &mut value)
        })?;
        Ok(value != sys::CNA_FALSE)
    }

    /// Sets whether the effect encodes its output to sRGB.
    pub fn set_encode_output_to_srgb(&self, value: bool) -> Result<()> {
        // SAFETY: the handle is owned and the flag is by value.
        self.native.check(unsafe {
            (self.native.pbr_effect_set_encode_output_to_srgb_ext)(self.handle, u8::from(value))
        })
    }

    /// The `KHR_materials_specular` colour factor.
    pub fn specular_color_factor(&self) -> Result<Vector3> {
        let mut value = sys::CNA_Vector3::default();
        // SAFETY: the handle is owned and the output is a live local.
        self.native.check(unsafe {
            (self.native.pbr_effect_get_specular_color_factor_ext)(self.handle, &mut value)
        })?;
        Ok(Vector3 {
            X: value.x,
            Y: value.y,
            Z: value.z,
        })
    }

    /// Sets the `KHR_materials_specular` colour factor.
    pub fn set_specular_color_factor(&self, value: Vector3) -> Result<()> {
        // SAFETY: the handle is owned and the vector is by value.
        self.native.check(unsafe {
            (self.native.pbr_effect_set_specular_color_factor_ext)(
                self.handle,
                sys::CNA_Vector3 {
                    x: value.X,
                    y: value.Y,
                    z: value.Z,
                },
            )
        })
    }
}
