//! CNA's PBR effect against the live library, on an independent device.
//!
//! None of this is XNA -- `BasicEffect` has no metallic factor, no roughness
//! and no index of refraction -- so nothing here touches the strict
//! projection. The material values, material extensions and pipeline settings
//! this file used to qualify were engine-layer routes, removed in CNA ABI 0.30.

use cna::extensions::pbr::{AlphaMode, PbrEffect};
use cna::Microsoft::Xna::Framework::Graphics::{
    GraphicsDevice, GraphicsProfile, PresentationParameters,
};
use cna::Microsoft::Xna::Framework::{GraphicsDeviceInformation, Vector3};
use cna::{CnaError, ErrorCategory, Result};

fn device() -> Option<GraphicsDevice> {
    independent_device_or_skip(|| {
        let parameters = PresentationParameters::new();
        parameters.SetBackBufferWidth(64);
        parameters.SetBackBufferHeight(64);
        GraphicsDevice::new(
            &GraphicsDeviceInformation::new().Adapter(),
            GraphicsProfile::HiDef,
            &parameters,
        )
    })
}

/// A device with no `Game` anywhere, or `None` when this renderer cannot make one.
///
/// EasyGL's context needs a platform surface, so every GL-family renderer
/// refuses `cna_graphics_device_create` with a `Platform` failure naming the
/// missing window, while `HEADLESS` creates one happily. That is a renderer
/// capability rather than a binding fault, so it is reported and skipped --
/// but *only* that exact refusal. Any other failure is still a failure, which
/// is what keeps this from quietly hiding a regression.
fn independent_device_or_skip(build: impl FnOnce() -> Result<GraphicsDevice>) -> Option<GraphicsDevice> {
    match build() {
        Ok(device) => Some(device),
        Err(CnaError::Native {
            category: ErrorCategory::Platform,
            ref message,
            ..
        }) if message.contains("platform window id") => {
            println!("this renderer cannot create a device without a window: {message}");
            None
        }
        Err(error) => panic!(
            "independent GraphicsDevice construction failed with something other than \
             the renderer's no-window refusal, which is the only failure this skips. \
             Seen once under a parallel full-suite run and not reproduced since, so the \
             exact text matters: {error:?}"
        ),
    }
}

#[test]
fn a_pbr_effect_round_trips_every_scalar_it_carries() {
    if std::env::var_os("CNA_NATIVE_LIBRARY").is_none() {
        return;
    }
    // `PbrEffect` is CNA's *effects* module -- `cna_pbr_effect_create`
    // carries no `CNA_CNAEXT` guard -- so it round-trips on every build.
    let Some(device) = device() else { return };
    let effect = PbrEffect::new(&device).expect("a PbrEffect on an independent device");

    // Distinguishable values, so a property read back from a neighbouring
    // slot is visible rather than plausible.
    effect.set_metallic_factor(0.125).expect("metallic");
    effect.set_roughness_factor(0.375).expect("roughness");
    effect.set_alpha(0.625).expect("alpha");
    effect.set_alpha_cutoff(0.875).expect("cutoff");
    effect.set_normal_scale(2.5).expect("normal scale");
    effect.set_occlusion_strength(0.25).expect("occlusion");
    effect.set_ior(1.45).expect("ior");
    effect.set_specular_factor(0.75).expect("specular");
    effect.set_diffuse_color(Vector3::from_x_and_y_and_z(0.1, 0.2, 0.3)).expect("albedo");
    effect.set_emissive_factor(Vector3::from_x_and_y_and_z(0.4, 0.5, 0.6)).expect("emissive");
    effect.set_alpha_mode(AlphaMode::Mask).expect("alpha mode");
    effect.set_double_sided(true).expect("double sided");
    effect.set_vertex_color_enabled(true).expect("vertex colour");

    assert_eq!(effect.metallic_factor().expect("metallic"), 0.125);
    assert_eq!(effect.roughness_factor().expect("roughness"), 0.375);
    assert_eq!(effect.alpha().expect("alpha"), 0.625);
    assert_eq!(effect.alpha_cutoff().expect("cutoff"), 0.875);
    assert_eq!(effect.normal_scale().expect("normal scale"), 2.5);
    assert_eq!(effect.occlusion_strength().expect("occlusion"), 0.25);
    assert_eq!(effect.ior().expect("ior"), 1.45);
    assert_eq!(effect.specular_factor().expect("specular"), 0.75);
    assert_eq!(
        effect.diffuse_color().expect("albedo"),
        Vector3::from_x_and_y_and_z(0.1, 0.2, 0.3)
    );
    assert_eq!(
        effect.emissive_factor().expect("emissive"),
        Vector3::from_x_and_y_and_z(0.4, 0.5, 0.6)
    );
    assert_eq!(effect.alpha_mode().expect("alpha mode"), AlphaMode::Mask);
    assert!(effect.double_sided().expect("double sided"));
    assert!(effect.vertex_color_enabled().expect("vertex colour"));

    // Every alpha mode survives its own round trip, not only the one above.
    for mode in [AlphaMode::Opaque, AlphaMode::Mask, AlphaMode::Blend] {
        effect.set_alpha_mode(mode).expect("set alpha mode");
        assert_eq!(effect.alpha_mode().expect("alpha mode"), mode);
    }

    // The effect knows its device, and it is the one that made it.
    assert!(effect.graphics_device().PresentationParameters().is_ok());
}
