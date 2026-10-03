//! What remains of CNA's engine-layer surface, against the live library.
//!
//! CNA ABI 0.30 removed `engine_layer.h`; what stays is the debug-line drawer
//! (`graphics_ext.h`, a `CNA_CNAEXT` build option), the standalone model mesh
//! part, the image-based-light and indirect-draw argument values, the core
//! `PbrEffect`, and the model extensions -- skinned models, animation players,
//! morph targets and a scene's own clips. Every assertion below is a value: a
//! line count, a vertex position, a round-tripped scalar, an exact refusal.

use std::sync::{Arc, Mutex};

use cna::extensions::content::{
    AssetTypeId, CnbDocument, CnbLoader, CnbLoaderRegistry, CnbWriter, NativeContentManager,
    ReadLimits,
};
use cna::extensions::engine::{
    DebugDraw, ImageBasedLight, IndirectDrawArguments, IndirectDrawIndexedArguments,
    NativeMeshPart,
};
use cna::extensions::models::{
    create_infinite_perspective_field_of_view, AnimationClip, AnimationPlayer, BoneTrack,
    ClipTargetSpace, Keyframe, ModelAnimations, MorphTargetData, MorphTargetDelta,
    MorphWeightKeyframe, MorphWeightTrack, SkinnedModel, SkinningData,
};
use cna::extensions::pbr::{AlphaMode, PbrEffect, SkinnedPbrEffect, TextureSlot, TextureTransform};
use cna::Microsoft::Xna::Framework::Graphics::{
    BufferUsage, IndexBuffer, IndexElementSize, PrimitiveType, SamplerState, SurfaceFormat,
    Texture2D, TextureAddressMode, TextureCube, TextureFilter, VertexBuffer, VertexPositionColor,
    VertexPositionNormalTexture,
};
use cna::Microsoft::Xna::Framework::{
    BoundingBox, BoundingFrustum, BoundingSphere, Color, Game, GameContext, Matrix, Quaternion,
    Vector3,
};
use cna::{run_for_frames, GameState, GameStateAccess, Result};

/// One CNA `Game` at a time.
///
/// CNA's game host is process-global -- the renderer, the window and the
/// device manager are all one per process -- so two `run_for_frames` calls on
/// two test threads race for the same native state.
static ONE_GAME_AT_A_TIME: Mutex<()> = Mutex::new(());

fn triple(value: Vector3) -> (f32, f32, f32) {
    (value.X, value.Y, value.Z)
}

/// A camera looking down -Z from the origin, with a 90-degree field of view.
fn look_down_minus_z() -> (Matrix, Matrix) {
    (
        Matrix::CreateLookAt(
            Vector3::Zero,
            Vector3::from_x_and_y_and_z(0.0, 0.0, -1.0),
            Vector3::Up,
        ),
        Matrix::CreatePerspectiveFieldOfView(std::f32::consts::FRAC_PI_2, 1.0, 1.0, 100.0),
    )
}

fn unit_box_at(x: f32, y: f32, z: f32) -> BoundingBox {
    BoundingBox::new(
        Vector3::from_x_and_y_and_z(x - 0.5, y - 0.5, z - 0.5),
        Vector3::from_x_and_y_and_z(x + 0.5, y + 0.5, z + 0.5),
    )
}

/// What the debug-draw run measured.
#[derive(Default)]
struct DebugDrawFindings {
    available: bool,
    refusal: Option<String>,
    line_counts: Vec<(&'static str, i32)>,
    vertices_depth_tested: usize,
    vertices_overlay: usize,
    first_line: Option<((f32, f32, f32), u32)>,
    depth_tested_round_trip: Option<(bool, bool)>,
}

struct DebugDrawGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<DebugDrawFindings>>,
}

impl GameStateAccess for DebugDrawGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

impl Game for DebugDrawGame {
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let device = game.GraphicsDevice()?;
        let mut findings = DebugDrawFindings {
            available: cna::extensions::graphics::is_available()?,
            ..DebugDrawFindings::default()
        };
        if !findings.available {
            findings.refusal = DebugDraw::new(&device).err().map(|error| error.to_string());
            *self.findings.lock().expect("findings") = findings;
            return Ok(());
        }
        let (view, projection) = look_down_minus_z();
        let debug = DebugDraw::new(&device)?;
        findings.line_counts.push(("empty", debug.line_count()?));
        debug.add_line(
            Vector3::Zero,
            Vector3::from_x_and_y_and_z(1.0, 0.0, 0.0),
            Color::Red,
        )?;
        findings.line_counts.push(("one line", debug.line_count()?));
        debug.add_box(unit_box_at(0.0, 0.0, -5.0), Color::Lime)?;
        findings.line_counts.push(("plus a box", debug.line_count()?));
        debug.add_cross(Vector3::Zero, 1.0, Color::White)?;
        findings.line_counts.push(("plus a cross", debug.line_count()?));
        debug.add_sphere(Vector3::Zero, 1.0, Color::Blue, 8)?;
        findings.line_counts.push(("plus a sphere", debug.line_count()?));
        debug.add_bounding_sphere(
            BoundingSphere {
                Center: Vector3::Zero,
                Radius: 2.0,
            },
            Color::Yellow,
            8,
        )?;
        findings
            .line_counts
            .push(("plus a bounding sphere", debug.line_count()?));
        debug.add_frustum(&BoundingFrustum::new(view * projection), Color::Magenta)?;
        findings.line_counts.push(("plus a frustum", debug.line_count()?));

        let depth_vertices = debug.vertices(true)?;
        let overlay_vertices = debug.vertices(false)?;
        findings.vertices_depth_tested = depth_vertices.len();
        findings.vertices_overlay = overlay_vertices.len();
        let queued = if depth_vertices.is_empty() {
            &overlay_vertices
        } else {
            &depth_vertices
        };
        findings.first_line = queued.first().map(|vertex| {
            (
                (vertex.Position.X, vertex.Position.Y, vertex.Position.Z),
                vertex.Color.PackedValue(),
            )
        });

        let before = debug.is_depth_tested()?;
        debug.set_depth_tested(!before)?;
        findings.depth_tested_round_trip = Some((before, debug.is_depth_tested()?));
        debug.set_depth_tested(before)?;

        debug.clear()?;
        findings.line_counts.push(("cleared", debug.line_count()?));

        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
fn debug_drawing_counts_exactly_what_it_produces() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(DebugDrawFindings::default()));
    let game = DebugDrawGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with a debug drawer");

    let findings = findings.lock().expect("findings");
    if !findings.available {
        // The drawer is a CNA_CNAEXT build option; without it the route must
        // refuse rather than hand out a drawer that draws nothing.
        let refusal = findings.refusal.as_deref().expect("a refusal without the layer");
        println!("graphics extensions absent; DebugDraw refused: {refusal}");
        return;
    }

    println!("line counts: {:?}", findings.line_counts);
    let counts: std::collections::HashMap<&str, i32> =
        findings.line_counts.iter().copied().collect();
    assert_eq!(counts["empty"], 0, "a fresh drawer has nothing queued");
    assert_eq!(counts["one line"], 1, "a line is one line");
    assert_eq!(counts["plus a box"], 1 + 12, "a box is its twelve edges");
    assert_eq!(counts["plus a cross"], 1 + 12 + 3, "a cross is three segments");
    assert_eq!(
        counts["plus a sphere"],
        1 + 12 + 3 + 8 * 3,
        "an eight-segment sphere is three eight-segment rings"
    );
    assert_eq!(
        counts["plus a bounding sphere"],
        1 + 12 + 3 + 8 * 3 + 8 * 3,
        "and a bounding sphere is drawn exactly the same way"
    );
    assert_eq!(
        counts["plus a frustum"],
        1 + 12 + 3 + 8 * 3 + 8 * 3 + 12,
        "a frustum is twelve edges like any other box"
    );
    assert_eq!(counts["cleared"], 0, "clearing leaves nothing queued");

    // Two vertices per line, and every line landed in the queue the drawer's
    // own depth-test flag names.
    let total_vertices = findings.vertices_depth_tested + findings.vertices_overlay;
    assert_eq!(
        total_vertices,
        counts["plus a frustum"] as usize * 2,
        "every queued line is two vertices, across both queues"
    );
    let (depth_tested, after) = findings
        .depth_tested_round_trip
        .expect("the depth-test flag was read");
    let (used, empty) = if depth_tested {
        (findings.vertices_depth_tested, findings.vertices_overlay)
    } else {
        (findings.vertices_overlay, findings.vertices_depth_tested)
    };
    assert_eq!(used, total_vertices, "the lines went into the queue the flag selects");
    assert_eq!(empty, 0, "and the other queue stayed empty");
    let (position, color) = findings.first_line.expect("a first vertex");
    assert_eq!(
        position,
        (0.0, 0.0, 0.0),
        "the first vertex is where the first line started"
    );
    assert_eq!(
        color,
        Color::Red.PackedValue(),
        "and carries the colour that line was given"
    );
    assert_ne!(depth_tested, after, "the depth-test flag round-trips");
}

/// What the native-mesh-part run measured.
#[derive(Default)]
struct MeshPartFindings {
    part: Option<(i32, i32, i32, i32)>,
    part_primitive_type: Vec<(&'static str, String)>,
    empty_part: Option<(i32, i32)>,
}

struct MeshPartGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<MeshPartFindings>>,
}

impl GameStateAccess for MeshPartGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

impl Game for MeshPartGame {
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let device = game.GraphicsDevice()?;
        let mut findings = MeshPartFindings::default();

        // One triangle, as a native mesh part.
        let declaration = VertexPositionColor::VertexDeclaration();
        let vertices = VertexBuffer::new(&device, declaration, 3, BufferUsage::None)?;
        vertices.SetData(&[
            VertexPositionColor::new(Vector3::Zero, Color::Red),
            VertexPositionColor::new(Vector3::UnitX, Color::Lime),
            VertexPositionColor::new(Vector3::UnitY, Color::Blue),
        ])?;
        let indices =
            IndexBuffer::new(&device, IndexElementSize::SixteenBits, 3, BufferUsage::None)?;
        indices.SetData(&[0_u16, 1, 2])?;

        let part = NativeMeshPart::new(Some(vertices), Some(indices), 3, 1, 0, 0)?;
        findings.part = Some((
            part.num_vertices()?,
            part.primitive_count()?,
            part.start_index()?,
            part.vertex_offset()?,
        ));
        findings
            .part_primitive_type
            .push(("as created", format!("{:?}", part.primitive_type()?)));
        part.set_primitive_type(PrimitiveType::LineStrip)?;
        findings
            .part_primitive_type
            .push(("after setting", format!("{:?}", part.primitive_type()?)));

        // A part with no buffers and no primitives is a legal value.
        let empty = NativeMeshPart::new(None, None, 0, 0, 0, 0)?;
        findings.empty_part = Some((empty.num_vertices()?, empty.primitive_count()?));

        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
fn a_native_mesh_part_preserves_its_counts_and_primitive_type() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(MeshPartFindings::default()));
    let game = MeshPartGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with a native mesh part");

    let findings = findings.lock().expect("findings");
    assert_eq!(
        findings.part,
        Some((3, 1, 0, 0)),
        "the four counts are preserved verbatim, as the header says"
    );
    println!("primitive type: {:?}", findings.part_primitive_type);
    assert_eq!(
        findings.part_primitive_type[0].1, "TriangleList",
        "a part starts as a triangle list"
    );
    assert_eq!(
        findings.part_primitive_type[1].1, "LineStrip",
        "and the primitive type round-trips"
    );
    assert_eq!(findings.empty_part, Some((0, 0)), "an empty part holds nothing");
}

/// What the value-initialiser run measured.
#[derive(Default)]
struct ValueFindings {
    indirect_defaults: Option<([u32; 4], [u32; 5])>,
    ibl_defaults: Option<(bool, i32, f32)>,
    ibl_states: Vec<(&'static str, bool)>,
}

struct ValueGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<ValueFindings>>,
}

impl GameStateAccess for ValueGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

impl Game for ValueGame {
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let device = game.GraphicsDevice()?;
        let mut findings = ValueFindings {
            indirect_defaults: Some((
                IndirectDrawArguments::canonical_defaults()?.to_words(),
                IndirectDrawIndexedArguments::canonical_defaults()?.to_words(),
            )),
            ..ValueFindings::default()
        };
        let mut ibl = ImageBasedLight::canonical_defaults()?;
        findings.ibl_defaults = Some((
            ibl.is_valid()?,
            ibl.prefiltered_mip_count(),
            ibl.intensity(),
        ));
        // The cubes need a renderer with cube storage; the validity rule is
        // about the three slots, not about what the textures hold.
        if let (Ok(irradiance), Ok(specular)) = (
            TextureCube::new(&device, 4, false, SurfaceFormat::Color),
            TextureCube::new(&device, 8, true, SurfaceFormat::Color),
        ) {
            let lut = Texture2D::new(&device, 8, 8)?;
            ibl.set_irradiance(Some(irradiance));
            findings.ibl_states.push(("with one texture", ibl.is_valid()?));
            ibl.set_prefiltered_specular(Some(specular), 4);
            findings.ibl_states.push(("with two", ibl.is_valid()?));
            ibl.set_brdf_lut(Some(lut));
            findings.ibl_states.push(("with all three", ibl.is_valid()?));
            ibl.set_prefiltered_specular(None, 4);
            findings
                .ibl_states
                .push(("with the specular cube taken away", ibl.is_valid()?));
        }
        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
fn the_argument_values_come_from_the_library_and_validate_what_they_name() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(ValueFindings::default()));
    let game = ValueGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with the value initialisers");

    let findings = findings.lock().expect("findings");
    let (plain, indexed) = findings.indirect_defaults.expect("indirect defaults");
    assert_eq!(plain, [0, 0, 0, 0], "a default non-indexed argument block draws nothing");
    assert_eq!(indexed, [0, 0, 0, 0, 0], "and neither does a default indexed one");

    let (valid, mips, intensity) = findings.ibl_defaults.expect("image-based light defaults");
    assert!(!valid, "a light with no textures cannot shade");
    assert!(mips >= 1, "the default mip count is at least one: {mips}");
    assert!(intensity > 0.0, "and the default intensity is not zero");
    println!("image-based light states: {:?}", findings.ibl_states);
    if !findings.ibl_states.is_empty() {
        assert_eq!(
            findings.ibl_states,
            vec![
                ("with one texture", false),
                ("with two", false),
                ("with all three", true),
                ("with the specular cube taken away", false),
            ],
            "a nearly complete light is invalid, which is the failure the check exists for"
        );
    }
}

/// What the device-backed PBR run measured.
#[derive(Default)]
struct DeviceBackedPbrFindings {
    scalars: Vec<(&'static str, f32, f32)>,
    colors: Vec<(&'static str, (f32, f32, f32), (f32, f32, f32))>,
    flags: Vec<(&'static str, bool)>,
    alpha_modes: Vec<(String, String)>,
    slots: Vec<(String, i32, i32, TextureTransform, TextureTransform)>,
    bad_coordinate_set: Option<String>,
    device_reachable: bool,
    skinned_weights: Option<i32>,
}

struct DeviceBackedPbrGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<DeviceBackedPbrFindings>>,
}

impl GameStateAccess for DeviceBackedPbrGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

impl Game for DeviceBackedPbrGame {
    #[allow(clippy::too_many_lines)]
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let device = game.GraphicsDevice()?;
        let mut findings = DeviceBackedPbrFindings::default();

        // `PbrEffect` is CNA's *effects* module: `cna_pbr_effect_create`
        // carries no `CNA_CNAEXT` guard, so it works on every build.
        let effect = PbrEffect::new(&device)?;

        // Distinguishable values, so a property read back from a neighbouring
        // slot is visible rather than plausible.
        effect.set_metallic_factor(0.125)?;
        effect.set_roughness_factor(0.375)?;
        effect.set_alpha(0.625)?;
        effect.set_alpha_cutoff(0.875)?;
        effect.set_normal_scale(2.5)?;
        effect.set_occlusion_strength(0.25)?;
        effect.set_ior(1.45)?;
        effect.set_specular_factor(0.75)?;
        for (name, written, read) in [
            ("metallic", 0.125_f32, effect.metallic_factor()?),
            ("roughness", 0.375, effect.roughness_factor()?),
            ("alpha", 0.625, effect.alpha()?),
            ("alpha cutoff", 0.875, effect.alpha_cutoff()?),
            ("normal scale", 2.5, effect.normal_scale()?),
            ("occlusion", 0.25, effect.occlusion_strength()?),
            ("index of refraction", 1.45, effect.ior()?),
            ("specular", 0.75, effect.specular_factor()?),
        ] {
            findings.scalars.push((name, written, read));
        }

        effect.set_diffuse_color(Vector3::from_x_and_y_and_z(0.1, 0.2, 0.3))?;
        effect.set_emissive_factor(Vector3::from_x_and_y_and_z(0.4, 0.5, 0.6))?;
        findings.colors.push((
            "albedo",
            (0.1, 0.2, 0.3),
            triple(effect.diffuse_color()?),
        ));
        findings.colors.push((
            "emissive",
            (0.4, 0.5, 0.6),
            triple(effect.emissive_factor()?),
        ));

        effect.set_double_sided(true)?;
        effect.set_vertex_color_enabled(true)?;
        findings
            .flags
            .push(("double sided", effect.double_sided()?));
        findings
            .flags
            .push(("vertex colour", effect.vertex_color_enabled()?));

        for mode in [AlphaMode::Opaque, AlphaMode::Mask, AlphaMode::Blend] {
            effect.set_alpha_mode(mode)?;
            findings
                .alpha_modes
                .push((format!("{mode:?}"), format!("{:?}", effect.alpha_mode()?)));
        }

        // Every slot gets a different transform and coordinate set, so a slot
        // read back from a neighbour is visible rather than plausible.
        for (ordinal, slot) in TextureSlot::ALL.into_iter().enumerate() {
            let ordinal_f = ordinal as f32;
            let transform = TextureTransform {
                offset: (ordinal_f, ordinal_f + 0.5),
                scale: (1.0 + ordinal_f, 2.0 + ordinal_f),
                rotation: 0.25 * ordinal_f,
            };
            effect.set_texture_coordinate_set(slot, (ordinal as i32) % 2)?;
            effect.set_texture_transform(slot, transform)?;
            findings.slots.push((
                format!("{slot:?}"),
                (ordinal as i32) % 2,
                effect.texture_coordinate_set(slot)?,
                transform,
                effect.texture_transform(slot)?,
            ));
        }
        findings.bad_coordinate_set = effect
            .set_texture_coordinate_set(TextureSlot::BaseColor, 5)
            .err()
            .map(|error| error.to_string());
        findings.device_reachable = effect.graphics_device().PresentationParameters().is_ok();

        let skinned = SkinnedPbrEffect::new(&device)?;
        skinned.set_weights_per_vertex(2)?;
        findings.skinned_weights = Some(skinned.weights_per_vertex()?);

        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
fn the_pbr_effect_round_trips_on_a_device_the_renderer_actually_made() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(DeviceBackedPbrFindings::default()));
    let game = DeviceBackedPbrGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with a PbrEffect on the game's own device");

    let findings = findings.lock().expect("findings");

    println!("scalars: {:?}", findings.scalars);
    assert_eq!(
        findings.scalars.len(),
        8,
        "every scalar the effect carries was written and read"
    );
    for (name, written, read) in &findings.scalars {
        assert_eq!(
            written, read,
            "{name} came back as {read} rather than {written}"
        );
    }
    println!("colours: {:?}", findings.colors);
    for (name, written, read) in &findings.colors {
        assert_eq!(written, read, "{name} came back as {read:?}");
    }
    assert_eq!(
        findings.flags,
        vec![("double sided", true), ("vertex colour", true)],
        "both flags round-trip"
    );
    println!("alpha modes: {:?}", findings.alpha_modes);
    for (written, read) in &findings.alpha_modes {
        assert_eq!(written, read, "every alpha mode survives its own round trip");
    }

    println!("slots: {} measured", findings.slots.len());
    assert_eq!(findings.slots.len(), 7, "there are seven per-slot entries");
    for (slot, written_set, read_set, written_transform, read_transform) in &findings.slots {
        assert_eq!(
            written_set, read_set,
            "slot {slot} kept its own coordinate set"
        );
        assert_eq!(
            written_transform, read_transform,
            "slot {slot} kept its own transform"
        );
    }
    let refused = findings
        .bad_coordinate_set
        .as_deref()
        .expect("an out-of-range coordinate set is refused");
    println!("coordinate set 5: {refused}");
    assert!(
        findings.device_reachable,
        "the effect knows its device, and it is the one that made it"
    );
    assert_eq!(
        findings.skinned_weights,
        Some(2),
        "the skinned effect keeps its weights per vertex"
    );
}

/// A Rust asset a registered loader builds, so the test can prove the loader
/// really ran and read the document it was given.
#[derive(Debug, PartialEq)]
struct LoadedProbeAsset {
    origin: String,
    asset_name: String,
    container: (u16, u16),
}

/// Counts its calls, and can be told to fail or to panic.
struct CountingLoader {
    calls: Arc<std::sync::atomic::AtomicUsize>,
    fail: bool,
    panic: bool,
}

impl CnbLoader for CountingLoader {
    fn load(
        &self,
        document: &CnbDocument,
        asset_name: &str,
    ) -> Result<Arc<dyn std::any::Any + Send + Sync>> {
        self.calls
            .fetch_add(1, std::sync::atomic::Ordering::SeqCst);
        if self.panic {
            panic!("a loader panic must not unwind into C");
        }
        if self.fail {
            return Err(cna::CnaError::InvalidInput(
                "this loader refuses every document",
            ));
        }
        Ok(Arc::new(LoadedProbeAsset {
            origin: document.origin()?,
            asset_name: asset_name.to_owned(),
            container: document.container_version()?,
        }))
    }
}

fn custom_cnb_document(type_name: &str, content_name: &str) -> Result<Vec<u8>> {
    let asset_type = AssetTypeId::custom(type_name)?;
    let writer = CnbWriter::new(asset_type, 1)?;
    writer.set_metadata(type_name, content_name)?;
    writer.build()
}

/// What the device-backed loader run measured.
#[derive(Default)]
struct LoaderFindings {
    round_trip: Option<(String, String, bool)>,
    calls: usize,
    withdrawn: Option<(bool, bool)>,
    contained: Vec<(&'static str, bool, usize, bool)>,
}

struct LoaderGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<LoaderFindings>>,
}

impl GameStateAccess for LoaderGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

impl Game for LoaderGame {
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let device = game.GraphicsDevice()?;
        let manager = NativeContentManager::new(&device, "")?;
        let mut findings = LoaderFindings::default();

        let type_name = "CnaRust.Engine.RoundTripAsset";
        let asset_type = AssetTypeId::custom(type_name)?;
        let calls = Arc::new(std::sync::atomic::AtomicUsize::new(0));
        let registration = CnbLoaderRegistry::register(
            type_name,
            Arc::new(CountingLoader {
                calls: Arc::clone(&calls),
                fail: false,
                panic: false,
            }),
        )?;

        let bytes = custom_cnb_document(type_name, "round-trip")?;
        let document = CnbDocument::parse(&bytes, "round-trip.cnb", ReadLimits::default())?;
        let loader = CnbLoaderRegistry::resolve_for_document(&document)?;
        let object = loader.invoke(&document, &manager, "round-trip")?;
        findings.calls = calls.load(std::sync::atomic::Ordering::SeqCst);
        let asset = object
            .downcast_ref::<LoadedProbeAsset>()
            .ok_or(cna::CnaError::InvalidInput("the loader built another type"))?;
        findings.round_trip = Some((
            asset.origin.clone(),
            asset.asset_name.clone(),
            asset.container == document.container_version()?,
        ));

        drop(registration);
        findings.withdrawn = Some((
            CnbLoaderRegistry::is_registered(asset_type)?,
            CnbLoaderRegistry::resolve_for_document(&document).is_err(),
        ));

        // A failing loader and a panicking one are both contained: neither
        // unwinds into C, and the process is still usable afterwards.
        for (label, fail, panic) in [("failing", true, false), ("panicking", false, true)] {
            let type_name = format!("CnaRust.Engine.{label}Asset");
            let calls = Arc::new(std::sync::atomic::AtomicUsize::new(0));
            let registration = CnbLoaderRegistry::register(
                &type_name,
                Arc::new(CountingLoader {
                    calls: Arc::clone(&calls),
                    fail,
                    panic,
                }),
            )?;
            let bytes = custom_cnb_document(&type_name, label)?;
            let document = CnbDocument::parse(&bytes, "contained.cnb", ReadLimits::default())?;
            let loader = CnbLoaderRegistry::resolve_for_document(&document)?;
            let refused = loader.invoke(&document, &manager, label).is_err();
            findings.contained.push((
                label,
                refused,
                calls.load(std::sync::atomic::Ordering::SeqCst),
                CnbLoaderRegistry::is_registered(registration.asset_type())?,
            ));
            drop(registration);
        }

        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
fn a_rust_loader_runs_on_the_device_the_renderer_actually_made() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(LoaderFindings::default()));
    let game = LoaderGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with a registered Rust loader");

    let findings = findings.lock().expect("findings");
    let (origin, name, container) = findings
        .round_trip
        .clone()
        .expect("the loader produced its Rust value");
    assert_eq!(findings.calls, 1, "the loader really ran, exactly once");
    assert_eq!(origin, "round-trip.cnb", "and read the document's origin");
    assert_eq!(name, "round-trip", "and the asset name it was invoked with");
    assert!(
        container,
        "and the container version out of the borrowed document rather than a default"
    );
    assert_eq!(
        findings.withdrawn,
        Some((false, true)),
        "dropping the registration withdraws it, and a withdrawn loader no longer resolves"
    );

    println!("contained: {:?}", findings.contained);
    for (label, refused, calls, still_registered) in &findings.contained {
        assert!(refused, "a {label} loader fails the load");
        assert_eq!(*calls, 1, "and the {label} loader ran");
        assert!(
            still_registered,
            "and the process is still usable afterwards, which is the point of \
             containing a panic rather than letting it unwind into C"
        );
    }
}

/// What the skinned-model run measured.
#[derive(Default)]
struct SkinnedFindings {
    empty: (u64, u64, u64),
    skeleton: (u64, Vec<i32>, bool, bool),
    clips: Vec<(String, Option<(f64, u64)>)>,
    track: Option<(i32, usize, f64, (f32, f32, f32))>,
    poses: Vec<(&'static str, f64, (f32, f32, f32))>,
    looped: Vec<(&'static str, (f32, f32, f32))>,
    missing_clip: bool,
    removed_clip: (u64, bool),
    parts: Vec<(&'static str, u64, String, bool)>,
    owned: Option<(u64, u64, u64, u64)>,
    attached: (u64, u64),
    moved: Vec<(&'static str, u64, u64)>,
    returned_on_refusal: Option<(String, bool)>,
}

struct SkinnedGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<SkinnedFindings>>,
}

impl GameStateAccess for SkinnedGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

/// A two-bone skeleton: a root and a child hanging off it.
fn two_bone_skeleton() -> (Vec<i32>, Vec<Matrix>, Vec<Matrix>) {
    (
        vec![-1, 0],
        vec![
            Matrix::Identity,
            Matrix::CreateTranslation(Vector3::from_x_and_y_and_z(0.0, 1.0, 0.0)),
        ],
        vec![
            Matrix::Identity,
            Matrix::CreateTranslation(Vector3::from_x_and_y_and_z(0.0, -1.0, 0.0)),
        ],
    )
}

/// A clip that slides bone one from the origin to x = 4 over two seconds.
fn slide_clip() -> AnimationClip {
    let frame = |time: f64, x: f32| Keyframe {
        time_seconds: time,
        translation: Vector3::from_x_and_y_and_z(x, 0.0, 0.0),
        rotation: Quaternion {
            X: 0.0,
            Y: 0.0,
            Z: 0.0,
            W: 1.0,
        },
        scale: Vector3::from_x_and_y_and_z(1.0, 1.0, 1.0),
    };
    AnimationClip {
        duration_seconds: 2.0,
        tracks: vec![BoneTrack {
            bone_index: 1,
            keyframes: vec![frame(0.0, 0.0), frame(2.0, 4.0)],
        }],
    }
}

impl Game for SkinnedGame {
    #[allow(clippy::too_many_lines)]
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let device = game.GraphicsDevice()?;
        let mut findings = SkinnedFindings::default();

        let empty = SkinnedModel::new()?;
        findings.empty = (empty.bone_count()?, empty.clip_count()?, empty.part_count()?);

        let (parents, bind, inverse) = two_bone_skeleton();
        let model = SkinnedModel::with_skeleton(
            &parents,
            &bind,
            &inverse,
            &[("slide".to_owned(), slide_clip())],
        )?;
        let read_bind = model.bind_pose_local()?;
        let read_inverse = model.inverse_bind_pose_global()?;
        findings.skeleton = (
            model.bone_count()?,
            model.parent_bone_indices()?,
            read_bind == bind,
            read_inverse == inverse,
        );

        findings.clips.push((
            model.clip_name_at(0)?,
            model
                .clip_info("slide")?
                .map(|info| (info.duration_seconds, info.track_count)),
        ));
        findings.missing_clip = model.clip_info("nothing-of-that-name")?.is_none();

        let track = model.clip_track("slide", 0)?;
        findings.track = Some((
            track.bone_index,
            track.keyframes.len(),
            track.keyframes[1].time_seconds,
            triple(track.keyframes[1].translation),
        ));

        // The pose at three positions along the clip. The child bone slides
        // from zero to four over two seconds, so the middle is two.
        for (name, position) in [("start", 0.0_f64), ("middle", 1.0), ("end", 2.0)] {
            let pose = model.compute_bone_transforms("slide", position, false)?;
            findings.poses.push((
                name,
                position,
                (pose[1].M41, pose[1].M42, pose[1].M43),
            ));
        }
        // Past the end, clamping and looping disagree: clamped stays at the
        // last pose, looped wraps back towards the first.
        for (name, loop_clip) in [("clamped past the end", false), ("looped past the end", true)] {
            let pose = model.compute_bone_transforms("slide", 3.0, loop_clip)?;
            findings
                .looped
                .push((name, (pose[1].M41, pose[1].M42, pose[1].M43)));
        }

        model.remove_clip("slide")?;
        findings.removed_clip = (model.clip_count()?, model.clip_info("slide")?.is_none());
        model.set_clip("slide", &slide_clip())?;

        // A renderable part, which the model retains for itself.
        let declaration = VertexPositionColor::VertexDeclaration();
        let vertices = VertexBuffer::new(&device, declaration, 3, BufferUsage::None)?;
        let indices =
            IndexBuffer::new(&device, IndexElementSize::SixteenBits, 3, BufferUsage::None)?;
        let part = NativeMeshPart::new(None, None, 3, 1, 0, 0)?;
        let texture = Texture2D::new(&device, 4, 4)?;
        findings
            .parts
            .push(("before", model.part_count()?, String::new(), false));
        model
            .add_part("body", &vertices, &indices, part, Some(texture))
            .map_err(|refused| refused.error)?;
        findings.parts.push((
            "with a textured part",
            model.part_count()?,
            model.part_name_at(0)?,
            model.part_has_texture_at(0)?,
        ));
        let counts = model.owned_resource_counts()?;
        findings.owned = Some((
            counts.vertex_buffers,
            counts.index_buffers,
            counts.parts,
            counts.textures,
        ));

        // A second model of the same skeleton hands its parts over.
        // Its own part, not this model's: CNA refuses a `ModelMeshPart` that
        // already belongs to a live model, which is the rule that stops two
        // models sharing one part's lifetime.
        let other = SkinnedModel::with_skeleton(&parents, &bind, &inverse, &[])?;
        // A refused `add_part` hands the part and its texture back, unconsumed.
        // A released model is the refusal that needs no invalid state anywhere
        // else: the part never reaches CNA, so if the caller does not get it
        // back it has simply been dropped.
        let closed = SkinnedModel::new()?;
        closed.release()?;
        let offered = NativeMeshPart::new(None, None, 3, 1, 0, 0)?;
        let offered_texture = Texture2D::new(&device, 2, 2)?;
        findings.returned_on_refusal =
            match closed.add_part("nowhere", &vertices, &indices, offered, Some(offered_texture)) {
                Ok(()) => None,
                Err(refused) => Some((
                    refused.error.to_string(),
                    // Both came back, and both are still usable -- which a
                    // consumed value would not be.
                    refused.part.primitive_count().is_ok()
                        && refused.texture.is_some_and(|texture| texture.Width() == 2),
                )),
            };
        let other_part = NativeMeshPart::new(None, None, 3, 1, 0, 0)?;
        other
            .add_part("arm", &vertices, &indices, other_part, None)
            .map_err(|refused| refused.error)?;
        model.attach_parts(&other)?;
        findings.attached = (model.part_count()?, other.part_count()?);

        // Move construction leaves the source valid but empty.
        findings
            .moved
            .push(("before the move", model.bone_count()?, model.part_count()?));
        let moved = model.move_out()?;
        findings
            .moved
            .push(("the source after it", model.bone_count()?, model.part_count()?));
        findings
            .moved
            .push(("the destination", moved.bone_count()?, moved.part_count()?));
        // And move-assignment does the same into an existing model.
        model.move_assign_from(&moved)?;
        findings
            .moved
            .push(("assigned back", model.bone_count()?, model.part_count()?));
        findings
            .moved
            .push(("its source now", moved.bone_count()?, moved.part_count()?));

        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
#[allow(clippy::too_many_lines)]
fn a_skinned_model_evaluates_the_pose_the_skinned_effect_needs() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(SkinnedFindings::default()));
    let game = SkinnedGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with a skinned model");

    let findings = findings.lock().expect("findings");
    assert_eq!(
        findings.empty,
        (0, 0, 0),
        "a default model has no bones, no clips and no parts"
    );

    let (bones, parents, bind_kept, inverse_kept) = findings.skeleton.clone();
    assert_eq!(bones, 2, "the skeleton has the bones it was given");
    assert_eq!(parents, vec![-1, 0], "with the parent indices it was given");
    assert!(bind_kept, "and the local bind pose comes back unchanged");
    assert!(
        inverse_kept,
        "and the inverse global bind pose, which is a different array"
    );

    println!("clips: {:?}", findings.clips);
    let (name, info) = findings.clips[0].clone();
    assert_eq!(name, "slide", "the clip is stored under the name it was given");
    let (duration, tracks) = info.expect("the clip is found by name");
    assert!(
        (duration - 2.0).abs() < 1e-9,
        "with its duration: {duration}"
    );
    assert_eq!(tracks, 1, "and its track count");
    assert!(
        findings.missing_clip,
        "and a name no clip has answers None rather than failing"
    );

    let (bone, frames, last_time, last_translation) = findings.track.expect("a track");
    assert_eq!(bone, 1, "the track drives the bone it names");
    assert_eq!(frames, 2, "and carries both keyframes");
    assert!((last_time - 2.0).abs() < 1e-9, "at the times they were given");
    assert_eq!(
        last_translation,
        (4.0, 0.0, 0.0),
        "and the translations they were given"
    );

    // The pose really is evaluated: the child bone slides from zero to four
    // over two seconds, so the middle is two. A model that returned the bind
    // pose, or the same matrix at every position, fails here.
    println!("poses: {:?}", findings.poses);
    let x = |name: &str| -> f32 {
        findings
            .poses
            .iter()
            .find(|(label, _, _)| *label == name)
            .expect("a recorded pose")
            .2
             .0
    };
    assert!(
        (x("start") - 0.0).abs() < 1e-4,
        "at the start the bone is where the first keyframe puts it: {}",
        x("start")
    );
    assert!(
        (x("middle") - 2.0).abs() < 1e-3,
        "halfway through it is halfway between them: {}",
        x("middle")
    );
    assert!(
        (x("end") - 4.0).abs() < 1e-4,
        "and at the end it is where the last one puts it: {}",
        x("end")
    );

    println!("past the end: {:?}", findings.looped);
    let clamped = findings.looped[0].1;
    let looped = findings.looped[1].1;
    assert!(
        (clamped.0 - 4.0).abs() < 1e-4,
        "clamping past the end holds the last pose: {clamped:?}"
    );
    assert!(
        looped != clamped,
        "and looping wraps instead, which is the whole difference: {looped:?}"
    );

    assert_eq!(
        findings.removed_clip,
        (0, true),
        "removing a clip removes it, and it is no longer found by name"
    );

    println!("parts: {:?}", findings.parts);
    assert_eq!(findings.parts[0].1, 0, "a model starts with no parts");
    assert_eq!(findings.parts[1].1, 1, "adding one adds one");
    assert_eq!(
        findings.parts[1].2, "body",
        "stored under the name it was given"
    );
    assert!(findings.parts[1].3, "and it carries the texture it was given");

    // The model retains what it was given: one of each, and the texture too.
    assert_eq!(
        findings.owned,
        Some((1, 1, 1, 1)),
        "the model owns one vertex buffer, one index buffer, one part and one texture"
    );

    // A mesh part belongs to one live model, and a refusal hands it back.
    println!("returned on refusal: {:?}", findings.returned_on_refusal);
    let (message, usable) = findings
        .returned_on_refusal
        .clone()
        .expect("adding a part to a released model is refused");
    assert!(
        message.contains("released"),
        "and refused for the model being released: {message}"
    );
    assert!(
        usable,
        "and the part and its texture both come back usable rather than consumed"
    );
    assert_eq!(
        findings.attached,
        (2, 0),
        "attaching moves the other model's parts across and leaves it with none"
    );

    // A move leaves the source valid but empty -- not invalid, which is the
    // distinction that makes this a content transfer rather than a handle one.
    println!("moves: {:?}", findings.moved);
    assert_eq!(findings.moved[0], ("before the move", 2, 2));
    assert_eq!(
        findings.moved[1],
        ("the source after it", 0, 0),
        "the source is left valid but empty"
    );
    assert_eq!(
        findings.moved[2],
        ("the destination", 2, 2),
        "and the destination has everything"
    );
    assert_eq!(
        findings.moved[3],
        ("assigned back", 2, 2),
        "move-assignment does the same into an existing model"
    );
    assert_eq!(
        findings.moved[4],
        ("its source now", 0, 0),
        "and empties its source too"
    );
}

/// What the skinning-data and animation-player run measured.
#[derive(Default)]
struct AnimationFindings {
    type_name: String,
    shape: (u64, u64, Vec<i32>),
    prefix: (usize, usize),
    partial_prefix_refused: bool,
    clip_name: String,
    clip_info: Option<(f64, u64)>,
    target_space: Vec<(&'static str, String)>,
    root: (i32, String),
    before_start: (Option<(f64, u64)>, String, f64),
    after_start: (Option<(f64, u64)>, String, f64),
    positions: Vec<(&'static str, f64)>,
    arrays: Vec<(&'static str, usize)>,
    skin_differs: bool,
    world_moves: Vec<(&'static str, f32)>,
    data_alive_after_player: bool,
}

struct AnimationGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<AnimationFindings>>,
}

impl GameStateAccess for AnimationGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

impl Game for AnimationGame {
    #[allow(clippy::too_many_lines)]
    fn LoadContent(&mut self, _game: &mut GameContext<'_>) -> Result<()> {
        let mut findings = AnimationFindings::default();
        let (parents, bind, inverse) = two_bone_skeleton();

        // A partial root prefix is refused: it would place the rest of the
        // skeleton somewhere arbitrary.
        findings.partial_prefix_refused = SkinningData::new(
            &parents,
            &bind,
            &inverse,
            &[Matrix::Identity],
            &[],
        )
        .is_err();

        let data = Arc::new(SkinningData::new(
            &parents,
            &bind,
            &inverse,
            &[Matrix::Identity, Matrix::Identity],
            &[("slide".to_owned(), slide_clip())],
        )?);
        findings.type_name = data.type_name()?;
        findings.shape = (
            data.bone_count()?,
            data.clip_count()?,
            data.skeleton_hierarchy()?,
        );
        findings.prefix = (
            data.skeleton_root_prefix()?.len(),
            data.bind_pose()?.len(),
        );
        findings.clip_name = data.clip_name_at(0)?;
        findings.clip_info = data
            .clip_info("slide")?
            .map(|info| (info.duration_seconds, info.track_count));

        findings.target_space.push((
            "as created",
            format!("{:?}", data.clip_target_space(0)?),
        ));
        data.set_clip_target_space(0, ClipTargetSpace::SceneNode)?;
        findings
            .target_space
            .push(("after setting", format!("{:?}", data.clip_target_space(0)?)));
        data.set_clip_target_space(0, ClipTargetSpace::JointPalette)?;

        data.set_skeleton_root_node_index(7)?;
        data.set_skeleton_root_name("hips")?;
        findings.root = (data.skeleton_root_node_index()?, data.skeleton_root_name()?);

        let player = AnimationPlayer::new(&data)?;
        findings.before_start = (
            player
                .current_clip()?
                .map(|info| (info.duration_seconds, info.track_count)),
            player.current_clip_name()?,
            player.current_position()?,
        );
        player.start_clip("slide")?;
        findings.after_start = (
            player
                .current_clip()?
                .map(|info| (info.duration_seconds, info.track_count)),
            player.current_clip_name()?,
            player.current_position()?,
        );

        // Seeking and advancing are different: the first sets the position,
        // the second adds to it.
        player.update(1.0, false, false)?;
        findings.positions.push(("sought to one", player.current_position()?));
        player.update(0.5, true, false)?;
        findings
            .positions
            .push(("advanced by a half", player.current_position()?));
        player.update(1.0, false, false)?;
        findings.positions.push(("sought back to one", player.current_position()?));

        let bones = player.bone_transforms()?;
        let world = player.world_transforms()?;
        let skin = player.skin_transforms()?;
        findings.arrays.push(("bone", bones.len()));
        findings.arrays.push(("world", world.len()));
        findings.arrays.push(("skin", skin.len()));
        // The three are different things: world is bone composed down the
        // hierarchy, skin is world times the inverse bind pose.
        findings.skin_differs = skin != world;

        for (name, position) in [("start", 0.0_f64), ("middle", 1.0), ("end", 2.0)] {
            player.update(position, false, false)?;
            let world = player.world_transforms()?;
            findings.world_moves.push((name, world[1].M41));
        }

        // Releasing the player leaves the data usable, which is what the
        // retention is for.
        player.release()?;
        findings.data_alive_after_player = data.bone_count().is_ok();

        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
#[allow(clippy::too_many_lines)]
fn an_animation_player_composes_the_three_transform_arrays_a_skinned_draw_needs() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(AnimationFindings::default()));
    let game = AnimationGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with an animation player");

    let findings = findings.lock().expect("findings");
    println!("type name: {:?}", findings.type_name);
    assert!(
        !findings.type_name.is_empty(),
        "the data knows the name the content pipeline writes for it"
    );
    assert!(
        findings.partial_prefix_refused,
        "a root prefix covering only some bones is refused rather than padded"
    );

    let (bones, clips, hierarchy) = findings.shape.clone();
    assert_eq!(bones, 2, "the skeleton has the bones it was given");
    assert_eq!(clips, 1, "and the clip it was given");
    assert_eq!(hierarchy, vec![-1, 0], "with the hierarchy it was given");
    assert_eq!(
        findings.prefix,
        (2, 2),
        "a full root prefix is kept, one matrix per bone"
    );
    assert_eq!(findings.clip_name, "slide", "the clip keeps its name");
    let (duration, tracks) = findings.clip_info.expect("the clip is found by name");
    assert!((duration - 2.0).abs() < 1e-9, "and its duration");
    assert_eq!(tracks, 1, "and its track count");

    println!("target space: {:?}", findings.target_space);
    assert_eq!(
        findings.target_space[0].1, "JointPalette",
        "a clip targets the joint palette by default"
    );
    assert_eq!(
        findings.target_space[1].1, "SceneNode",
        "and the space round-trips -- the two are never interchangeable"
    );
    assert_eq!(
        findings.root,
        (7, "hips".to_owned()),
        "the skeleton root index and name both round-trip"
    );

    // A player has no clip until one is started, and the empty state is a
    // *state* rather than a failure.
    println!("before start: {:?}", findings.before_start);
    assert_eq!(findings.before_start.0, None, "no clip before one is started");
    assert!(findings.before_start.1.is_empty(), "and no clip name");
    let (info, name, position) = findings.after_start.clone();
    assert_eq!(
        info,
        Some((2.0, 1)),
        "starting a clip makes it the current one, with its shape"
    );
    assert_eq!(name, "slide", "and its name");
    assert_eq!(position, 0.0, "and starts it at the beginning");

    // Seeking and advancing are different operations, which one flag chooses
    // between: a binding that passed the wrong one would still move the clock.
    println!("positions: {:?}", findings.positions);
    let at = |name: &str| -> f64 {
        findings
            .positions
            .iter()
            .find(|(label, _)| *label == name)
            .expect("a recorded position")
            .1
    };
    assert!((at("sought to one") - 1.0).abs() < 1e-9, "seeking sets the position");
    assert!(
        (at("advanced by a half") - 1.5).abs() < 1e-9,
        "advancing adds to it: {}",
        at("advanced by a half")
    );
    assert!(
        (at("sought back to one") - 1.0).abs() < 1e-9,
        "and seeking again sets it rather than adding"
    );

    println!("arrays: {:?}", findings.arrays);
    for (name, length) in &findings.arrays {
        assert_eq!(*length, 2, "the {name} array has one entry per bone");
    }
    assert!(
        findings.skin_differs,
        "the skin transforms are the world transforms times the inverse bind pose, \
         so the two arrays are not the same thing"
    );

    // The pose really moves: the child bone's world translation follows the
    // clip, exactly as the skinned model's own evaluation does.
    println!("world moves: {:?}", findings.world_moves);
    let x = |name: &str| -> f32 {
        findings
            .world_moves
            .iter()
            .find(|(label, _)| *label == name)
            .expect("a recorded world transform")
            .1
    };
    assert!((x("start") - 0.0).abs() < 1e-4, "at the start: {}", x("start"));
    assert!(
        (x("middle") - 2.0).abs() < 1e-3,
        "halfway through: {}",
        x("middle")
    );
    assert!((x("end") - 4.0).abs() < 1e-4, "and at the end: {}", x("end"));

    assert!(
        findings.data_alive_after_player,
        "releasing the player leaves its skinning data usable, \
         which is what retaining it is for"
    );
}

/// What the morph-target run measured.
#[derive(Default)]
struct MorphFindings {
    type_name: String,
    shape: (i32, u64, usize),
    deltas: Vec<(&'static str, usize, (f32, f32, f32))>,
    weights: Vec<(&'static str, Vec<f32>)>,
    blends: Vec<(&'static str, Vec<f32>)>,
    wrong_weight_count_refused: bool,
    stride_48: Option<std::result::Result<i32, String>>,
    track_info: (u64, bool, bool),
    keyframe: Option<(f64, Vec<f32>)>,
    linear: Vec<(f64, Vec<f32>)>,
    stepped: Vec<(f64, Vec<f32>)>,
    flat_normals: Vec<(&'static str, bool)>,
    triangles: Vec<u32>,
    part: Vec<(&'static str, bool)>,
    morph_without_buffer: Option<String>,
}

struct MorphGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<MorphFindings>>,
}

impl GameStateAccess for MorphGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

/// The one stride this test uses: position, normal and texture coordinate.
///
/// CNA's C API accepts only 32, 52 or 56 bytes here -- see the test's own note
/// on why that list is narrower than the renderer's.
const MORPH_STRIDE: i32 = 32;

/// Three vertices in the stride-32 layout: position, normal, texture.
fn base_pose() -> Vec<u8> {
    let mut bytes = Vec::new();
    for position in [[0.0_f32, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0]] {
        for value in position {
            bytes.extend_from_slice(&value.to_le_bytes());
        }
        // Normal, then texture coordinate: untouched by a position-only morph,
        // and present so the stride is one the API accepts.
        for value in [0.0_f32, 0.0, 1.0, 0.0, 0.0] {
            bytes.extend_from_slice(&value.to_le_bytes());
        }
    }
    bytes
}

/// The three position floats of the first vertex of a blended buffer.
fn read_positions(bytes: &[u8]) -> Vec<f32> {
    bytes[..12]
        .chunks_exact(4)
        .map(|word| f32::from_le_bytes([word[0], word[1], word[2], word[3]]))
        .collect()
}

impl Game for MorphGame {
    #[allow(clippy::too_many_lines)]
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let device = game.GraphicsDevice()?;
        let mut findings = MorphFindings::default();
        let base = base_pose();

        // Two targets: the first pushes every vertex one unit along x, the
        // second pushes them one unit along y.
        let along = |axis: usize| MorphTargetDelta {
            position_deltas: (0..3)
                .map(|_| {
                    let mut delta = [0.0_f32; 3];
                    delta[axis] = 1.0;
                    Vector3::from_x_and_y_and_z(delta[0], delta[1], delta[2])
                })
                .collect(),
            normal_deltas: Vec::new(),
        };
        let targets = vec![along(0), along(1)];

        // A two-keyframe weight track: the first target fades out as the
        // second fades in.
        let track = MorphWeightTrack {
            keyframes: vec![
                MorphWeightKeyframe {
                    time_seconds: 0.0,
                    weights: vec![1.0, 0.0],
                    in_tangents: Vec::new(),
                    out_tangents: Vec::new(),
                },
                MorphWeightKeyframe {
                    time_seconds: 2.0,
                    weights: vec![0.0, 1.0],
                    in_tangents: Vec::new(),
                    out_tangents: Vec::new(),
                },
            ],
            step_interpolation: false,
            cubic_spline: false,
        };

        let data = MorphTargetData::new(&base, MORPH_STRIDE, &targets, &[0.0, 0.0], &track)?;
        findings.type_name = data.type_name()?;
        findings.shape = (
            data.stride()?,
            data.target_count()?,
            data.base_vertex_bytes()?.len(),
        );
        for (name, index) in [("the x target", 0_u64), ("the y target", 1)] {
            let deltas = data.position_deltas(index)?;
            findings
                .deltas
                .push((name, deltas.len(), triple(deltas[0])));
        }
        findings
            .deltas
            .push(("its normals", data.normal_deltas(0)?.len(), (0.0, 0.0, 0.0)));

        findings.weights.push(("as created", data.weights()?));
        data.set_weights(&[0.25, 0.75])?;
        findings.weights.push(("after setting", data.weights()?));

        // Blending is the whole point: the base pose plus each target times its
        // weight. The first vertex starts at the origin, so the blended x and y
        // are the weights themselves.
        for (name, weights) in [
            ("neither target", vec![0.0_f32, 0.0]),
            ("all of the first", vec![1.0, 0.0]),
            ("all of the second", vec![0.0, 1.0]),
            ("half of each", vec![0.5, 0.5]),
        ] {
            let blended = data.blend(&weights)?;
            findings
                .blends
                .push((name, read_positions(&blended)[..3].to_vec()));
        }
        findings.wrong_weight_count_refused = data.blend(&[1.0]).is_err();

        // The stride list the C API enforces is narrower than the renderer's
        // own table: 48 is `PositionNormalTangentTextureStream`, which the
        // canonical blender handles and this route refuses.
        findings.stride_48 = Some(
            MorphTargetData::new(
                &vec![0_u8; 48 * 3],
                48,
                &targets,
                &[0.0, 0.0],
                &MorphWeightTrack::default(),
            )
            .and_then(|data| data.stride())
            .map_err(|error| error.to_string()),
        );

        findings.track_info = data.weight_track_info()?;
        let keyframe = data.weight_keyframe(1)?;
        findings.keyframe = Some((keyframe.time_seconds, keyframe.weights.clone()));

        // The track evaluates as a pure function of itself.
        for time in [0.0_f64, 1.0, 2.0] {
            findings.linear.push((time, track.evaluate(time)?));
        }
        let stepped = MorphWeightTrack {
            step_interpolation: true,
            ..track.clone()
        };
        for time in [0.0_f64, 1.0, 2.0] {
            findings.stepped.push((time, stepped.evaluate(time)?));
        }

        findings
            .flat_normals
            .push(("as created", data.recompute_flat_normals()?));
        data.set_triangle_indices(&[0, 1, 2])?;
        findings.triangles = data.triangle_indices()?;
        data.set_recompute_flat_normals(true)?;
        findings
            .flat_normals
            .push(("after asking for them", data.recompute_flat_normals()?));

        // A mesh part copies the data rather than borrowing it -- and needs a
        // vertex buffer of the same stride to upload the blend into, which is
        // what upstream refuses a part without.
        let declaration = VertexPositionNormalTexture::VertexDeclaration();
        let vertices = VertexBuffer::new(&device, declaration, 3, BufferUsage::None)?;
        let indices =
            IndexBuffer::new(&device, IndexElementSize::SixteenBits, 3, BufferUsage::None)?;
        // Attaching the data to a part with no vertex buffer is allowed; it is
        // *uploading* the blend that has nowhere to go, so that is where the
        // refusal lands.
        let bufferless = NativeMeshPart::new(None, None, 3, 1, 0, 0)?;
        bufferless.set_morph_target_data(Some(&data))?;
        findings.morph_without_buffer = bufferless
            .set_morph_weights(&[0.5, 0.5])
            .err()
            .map(|error| error.to_string());
        let part = NativeMeshPart::new(Some(vertices), Some(indices), 3, 1, 0, 0)?;
        findings
            .part
            .push(("before", part.has_morph_target_data()?));
        part.set_morph_target_data(Some(&data))?;
        findings
            .part
            .push(("with data", part.has_morph_target_data()?));
        part.set_morph_weights(&[0.5, 0.5])?;
        part.set_morph_target_data(None)?;
        findings
            .part
            .push(("after clearing", part.has_morph_target_data()?));

        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
#[allow(clippy::too_many_lines)]
fn morph_targets_blend_the_base_pose_by_the_weights_they_are_given() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(MorphFindings::default()));
    let game = MorphGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with morph target data");

    let findings = findings.lock().expect("findings");
    println!("type name: {:?}", findings.type_name);
    assert!(!findings.type_name.is_empty(), "the data knows its own name");
    assert_eq!(
        findings.shape,
        (MORPH_STRIDE, 2, MORPH_STRIDE as usize * 3),
        "the stride, target count and base-pose size are the ones it was given"
    );

    println!("deltas: {:?}", findings.deltas);
    assert_eq!(
        findings.deltas[0],
        ("the x target", 3, (1.0, 0.0, 0.0)),
        "the first target moves along x"
    );
    assert_eq!(
        findings.deltas[1],
        ("the y target", 3, (0.0, 1.0, 0.0)),
        "and the second along y"
    );
    assert_eq!(
        findings.deltas[2].1, 0,
        "a target with no normal deltas reports none rather than zeroes"
    );

    println!("weights: {:?}", findings.weights);
    assert_eq!(findings.weights[0].1, vec![0.0, 0.0], "the weights start where they were set");
    assert_eq!(findings.weights[1].1, vec![0.25, 0.75], "and round-trip");

    // The blend is the arithmetic, not a state flag: the first vertex sits at
    // the origin, so its blended position *is* the weight vector.
    println!("blends: {:?}", findings.blends);
    let blend = |name: &str| -> Vec<f32> {
        findings
            .blends
            .iter()
            .find(|(label, _)| *label == name)
            .expect("a recorded blend")
            .1
            .clone()
    };
    assert_eq!(
        blend("neither target"),
        vec![0.0, 0.0, 0.0],
        "with no weight the base pose comes back unchanged"
    );
    assert_eq!(
        blend("all of the first"),
        vec![1.0, 0.0, 0.0],
        "all of the x target moves the vertex one along x"
    );
    assert_eq!(
        blend("all of the second"),
        vec![0.0, 1.0, 0.0],
        "and all of the y target one along y"
    );
    assert_eq!(
        blend("half of each"),
        vec![0.5, 0.5, 0.0],
        "and half of each is half of each, which is what makes this a blend"
    );
    assert!(
        findings.wrong_weight_count_refused,
        "a weight array that does not match the target count is refused rather than padded"
    );

    // Stride 48 is what a metallic-roughness glTF mesh gets. The C entry point
    // used to refuse it against a stale literal list (RUST-UPSTREAM-024, fixed
    // upstream by CNA `BINDFIX-007`); it now asks the canonical stride table.
    println!("stride 48: {:?}", findings.stride_48);
    assert_eq!(
        findings.stride_48,
        Some(Ok(48)),
        "the C API accepts the stride its own renderer uses for a PBR mesh"
    );

    assert_eq!(
        findings.track_info,
        (2, false, false),
        "the stored track keeps its keyframes and its interpolation flags"
    );
    let (time, weights) = findings.keyframe.clone().expect("a stored keyframe");
    assert!((time - 2.0).abs() < 1e-9, "read back at its own time");
    assert_eq!(weights, vec![0.0, 1.0], "with its own weights");

    // Linear and stepped sampling differ in exactly one place: between the
    // keyframes. At the keyframes themselves they agree.
    println!("linear: {:?}", findings.linear);
    println!("stepped: {:?}", findings.stepped);
    assert_eq!(findings.linear[0].1, vec![1.0, 0.0], "both start at the first keyframe");
    assert_eq!(findings.linear[2].1, vec![0.0, 1.0], "and end at the last");
    assert_eq!(
        findings.linear[1].1,
        vec![0.5, 0.5],
        "linear sampling halfway is halfway between them"
    );
    assert_eq!(
        findings.stepped[1].1,
        vec![1.0, 0.0],
        "and stepped sampling holds the lower keyframe instead"
    );
    assert_eq!(
        findings.stepped[0].1, findings.linear[0].1,
        "the two agree at a keyframe"
    );

    assert_eq!(
        findings.flat_normals,
        vec![("as created", false), ("after asking for them", true)],
        "flat-normal recomputation is off until asked for"
    );
    assert_eq!(
        findings.triangles,
        vec![0, 1, 2],
        "and the triangle list it walks round-trips"
    );

    // A part with no vertex buffer accepts the *data* and refuses the *upload*:
    // the two are separate steps, and the failure lands on the one that needs
    // somewhere to write.
    let without = findings
        .morph_without_buffer
        .as_deref()
        .expect("uploading a blend with no vertex buffer is refused");
    assert!(
        without.contains("no vertex buffer"),
        "and refused for the missing buffer: {without}"
    );
    assert_eq!(
        findings.part,
        vec![
            ("before", false),
            ("with data", true),
            ("after clearing", false),
        ],
        "a mesh part takes a copy of the morph data and can be cleared again"
    );
}

/// What the scene-animation, sampler-slot and infinite-projection run measured.
#[derive(Default)]
struct SceneAnimationFindings {
    type_name: String,
    counts: (u64, u64),
    names: Vec<String>,
    clips: Vec<(f64, u64, String)>,
    after_setting_space: String,
    samplers: Vec<(String, String, String)>,
    infinite: Option<(f32, f32, f32, f32)>,
    finite: Option<(f32, f32)>,
    bad_projection: bool,
}

struct SceneAnimationGame {
    state: Arc<GameState>,
    findings: Arc<Mutex<SceneAnimationFindings>>,
}

impl GameStateAccess for SceneAnimationGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

impl Game for SceneAnimationGame {
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let device = game.GraphicsDevice()?;
        let mut findings = SceneAnimationFindings::default();

        let empty = ModelAnimations::new(&[])?;
        let animations = ModelAnimations::new(&[
            ("walk".to_owned(), slide_clip()),
            ("wave".to_owned(), slide_clip()),
        ])?;
        findings.type_name = animations.type_name()?;
        findings.counts = (empty.clip_count()?, animations.clip_count()?);
        for index in 0..animations.clip_count()? {
            findings.names.push(animations.clip_name_at(index)?);
            let (info, space) = animations.clip_at(index)?;
            findings.clips.push((
                info.duration_seconds,
                info.track_count,
                format!("{space:?}"),
            ));
        }
        // A scene-node clip is the whole reason this type exists separately
        // from a skeleton's.
        animations.set_clip_target_space_at(0, ClipTargetSpace::SceneNode)?;
        findings.after_setting_space = format!("{:?}", animations.clip_at(0)?.1);

        // Every slot has its own sampler, and they are independent.
        let part = NativeMeshPart::new(None, None, 3, 1, 0, 0)?;
        let mut anisotropic = SamplerState::new();
        anisotropic.SetFilter(TextureFilter::Anisotropic);
        anisotropic.SetAddressU(TextureAddressMode::Mirror);
        part.set_sampler_state(TextureSlot::Normal, &anisotropic)?;
        for slot in [TextureSlot::BaseColor, TextureSlot::Normal] {
            let state = part.sampler_state(slot, &device)?;
            findings.samplers.push((
                format!("{slot:?}"),
                format!("{:?}", state.Filter()),
                format!("{:?}", state.AddressU()),
            ));
        }

        // An infinite projection has no far plane, so its third column differs
        // from a finite one's while the rest of the matrix matches.
        let infinite = create_infinite_perspective_field_of_view(
            std::f32::consts::FRAC_PI_2,
            1.0,
            1.0,
        )?;
        let finite =
            Matrix::CreatePerspectiveFieldOfView(std::f32::consts::FRAC_PI_2, 1.0, 1.0, 100.0);
        findings.infinite = Some((infinite.M11, infinite.M22, infinite.M33, infinite.M43));
        findings.finite = Some((finite.M33, finite.M43));
        findings.bad_projection =
            create_infinite_perspective_field_of_view(0.0, 1.0, 1.0).is_err();

        *self.findings.lock().expect("findings") = findings;
        Ok(())
    }
}

#[test]
fn a_scene_carries_its_own_clips_and_a_gltf_camera_may_have_no_far_plane() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let findings = Arc::new(Mutex::new(SceneAnimationFindings::default()));
    let game = SceneAnimationGame {
        state: Arc::new(GameState::default()),
        findings: Arc::clone(&findings),
    };
    run_for_frames(game, 1).expect("one frame with scene animations");

    let findings = findings.lock().expect("findings");
    assert!(!findings.type_name.is_empty(), "the set knows its own name");
    assert_eq!(
        findings.counts,
        (0, 2),
        "an empty set holds nothing and a two-clip set holds two"
    );
    assert_eq!(
        findings.names,
        vec!["walk".to_owned(), "wave".to_owned()],
        "the clips keep the names they were given, in order"
    );
    println!("clips: {:?}", findings.clips);
    for (duration, tracks, space) in &findings.clips {
        assert!((duration - 2.0).abs() < 1e-9, "each keeps its duration");
        assert_eq!(*tracks, 1, "and its track count");
        assert_eq!(
            space, "JointPalette",
            "and targets the joint palette until told otherwise"
        );
    }
    assert_eq!(
        findings.after_setting_space, "SceneNode",
        "a scene-node clip is what this type exists for, and the space round-trips"
    );

    // The sampler slots are independent: setting one leaves the others alone.
    println!("samplers: {:?}", findings.samplers);
    assert_eq!(
        findings.samplers[0].1, "Linear",
        "a slot never set keeps the default filter"
    );
    assert_eq!(findings.samplers[0].2, "Wrap", "and the default addressing");
    assert_eq!(
        findings.samplers[1].1, "Anisotropic",
        "and the slot that was set keeps what it was given"
    );
    assert_eq!(findings.samplers[1].2, "Mirror", "including its addressing");

    // An infinite projection agrees with a finite one everywhere the far plane
    // does not reach, and differs exactly where it does.
    let (m11, m22, m33, m43) = findings.infinite.expect("an infinite projection");
    let (finite_m33, finite_m43) = findings.finite.expect("a finite one");
    println!("infinite: {m11} {m22} {m33} {m43} | finite: {finite_m33} {finite_m43}");
    assert!(
        (m11 - 1.0).abs() < 1e-5 && (m22 - 1.0).abs() < 1e-5,
        "a ninety-degree field of view at aspect one scales both axes by one"
    );
    assert!(
        (m33 + 1.0).abs() < 1e-5,
        "and with no far plane the depth scale is exactly -1: {m33}"
    );
    assert!(
        (m33 - finite_m33).abs() > 1e-3 && (m43 - finite_m43).abs() > 1e-3,
        "which is not what a finite projection gives: {m33}/{m43} against \
         {finite_m33}/{finite_m43}"
    );
    assert!(
        findings.bad_projection,
        "a zero field of view is refused rather than producing a degenerate matrix"
    );
}
