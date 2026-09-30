mod hello_game;

use cna::{run, run_for_frames, Result};
use hello_game::HelloGame;

#[derive(Clone, Copy, Debug)]
enum RunMode {
    Interactive,
    Frames(u64),
    ExtensionsSmoke,
}

fn parse_mode() -> core::result::Result<RunMode, String> {
    let mut arguments = std::env::args().skip(1);
    let mut mode = RunMode::Interactive;
    while let Some(argument) = arguments.next() {
        mode = match argument.as_str() {
            "--smoke-test" => RunMode::Frames(60),
            "--extensions-smoke" => RunMode::ExtensionsSmoke,
            "--stability-test" => RunMode::Frames(600),
            "--frames" => {
                let value = arguments
                    .next()
                    .ok_or("--frames requires a positive integer")?;
                let frames = value
                    .parse::<u64>()
                    .map_err(|_| "--frames requires a positive integer")?;
                if frames == 0 {
                    return Err("--frames requires a positive integer".to_owned());
                }
                RunMode::Frames(frames)
            }
            _ => return Err(format!("unknown argument: {argument}")),
        };
    }
    Ok(mode)
}

/// Prints what CNA itself says about the runtime it is about to use.
///
/// This is the CNA-only half of the binding: none of it exists in XNA 4.0, so
/// all of it comes from `cna::extensions` rather than from
/// `cna::Microsoft::Xna::Framework`. It is opt-in so the game the template is
/// actually demonstrating stays uncluttered.
fn extensions_smoke() -> Result<()> {
    use cna::extensions::runtime::{
        available_renderers, current_backend_category, current_backend_maturity, current_renderer,
        desktop_operating_system, platform, platform_name, renderer_selection_is_latched,
    };

    println!(
        "cna-rust-template: platform={:?} ({}) os={:?}",
        platform()?,
        platform_name()?,
        desktop_operating_system()?,
    );
    let current = current_renderer()?;
    println!(
        "cna-rust-template: renderer={} category={:?} maturity={:?} latched={}",
        current.name()?,
        current_backend_category()?,
        current_backend_maturity()?,
        renderer_selection_is_latched()?,
    );
    let available = available_renderers()?;
    println!(
        "cna-rust-template: {} renderer identity/identities compiled in",
        available.len()
    );
    for renderer in available {
        println!(
            "cna-rust-template:   {:?} category={:?} maturity={:?}",
            renderer.value(),
            renderer.category()?,
            renderer.maturity()?,
        );
    }
    content_smoke()?;
    standalone_device_smoke()?;
    graphics_extensions_smoke()
}

/// Reports whether this library carries CNA's extended graphics layer.
///
/// The layer (retro effects, DebugDraw) is a `CNA_CNAEXT` build option, so a
/// library without it answers false -- which is a fact to print, not a failure.
fn graphics_extensions_smoke() -> Result<()> {
    println!(
        "cna-rust-template: extended graphics layer available={}",
        cna::extensions::graphics::is_available()?
    );
    Ok(())
}

/// Builds a `GraphicsDevice` with no `Game`, and reads a `.cnb` model.
///
/// Two things a CNA-Rust program could not do before this template's current
/// generation: construct a graphics device on its own, and author and read
/// CNA's compiled model format. Both are one screen of code, which is the only
/// reason they are here -- this stays a starter template, not an engine demo.
fn standalone_device_smoke() -> Result<()> {
    use cna::Microsoft::Xna::Framework::Graphics::{
        GraphicsDevice, GraphicsProfile, PresentationParameters,
    };
    use cna::Microsoft::Xna::Framework::GraphicsDeviceInformation;

    // A device this program owns, with no game running.
    let parameters = PresentationParameters::new();
    parameters.SetBackBufferWidth(320);
    parameters.SetBackBufferHeight(240);
    let mut device = match GraphicsDevice::new(
        &GraphicsDeviceInformation::new().Adapter(),
        GraphicsProfile::Reach,
        &parameters,
    ) {
        Ok(device) => device,
        // Every GL-family renderer needs a platform surface for its context, so
        // a device with no game behind it is impossible there. That is a
        // renderer capability rather than a fault, and a canary that treated it
        // as one would tell a user on a real GPU that the binding is broken.
        // Only *that* refusal is tolerated; any other is still a failure.
        Err(error) if format!("{error}").contains("surface has no platform window id") => {
            println!(
                "cna-rust-template: this renderer cannot make a GraphicsDevice without a window, \
                 so the standalone-device half is skipped"
            );
            return standalone_model_smoke();
        }
        Err(error) => return Err(error),
    };
    let shape = device.PresentationParameters()?;
    println!(
        "cna-rust-template: standalone GraphicsDevice {}x{} profile={:?}",
        shape.BackBufferWidth(),
        shape.BackBufferHeight(),
        device.GraphicsProfile()?,
    );

    standalone_model_smoke()?;
    device.DisposeWithNoArguments()?;
    Ok(())
}

/// Authors a `.cnb` model and reads it back, which needs no device at all.
fn standalone_model_smoke() -> Result<()> {
    use cna::extensions::content::{CnbDocument, CnbEffectKind, CnbModel, CnbModelPart, ReadLimits};

    let model = CnbModel::new()?;
    let identity = [
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0_f32,
    ];
    let root = model.add_bone("root", None, &identity)?;
    model.set_flags(false, true)?;
    let part = model.add_part(
        CnbModelPart {
            vertex_stride: 12,
            vertex_count: 3,
            index_count: 3,
            index_element_size: 2,
            primitive_topology: 4,
            primitive_count: 1,
            effect_kind: CnbEffectKind::Basic,
            vertex_color_enabled: false,
            unlit: false,
        },
        "triangle",
        "",
    )?;
    model.set_part_vertex_bytes(part, &[0_u8; 36])?;
    model.set_part_index_bytes(part, &[0, 0, 1, 0, 2, 0])?;
    model.add_mesh("body", Some(root), &[part as u32])?;

    let bytes = model.encode("template model")?;
    let document = CnbDocument::parse(&bytes, "template.cnb", ReadLimits::default())?;
    let decoded = document.decode_model()?;
    let info = decoded.info()?;
    println!(
        "cna-rust-template: .cnb model {} bytes, {} bone(s) {:?}, {} mesh(es), {} part(s) drawing {:?}",
        bytes.len(),
        info.bone_count,
        decoded.bone_name(0)?,
        info.mesh_count,
        info.part_count,
        decoded.part(part)?.effect_kind,
    );
    assert_eq!(
        decoded.mesh_part_indices(0)?,
        vec![part as u32],
        "the mesh draws the part it was given"
    );
    Ok(())
}

/// Round-trips a texture through CNA's own `.cnb` content container.
///
/// XNA has one content format and `ContentManager` reads it; this is the other
/// one, and it needs no asset on disk because the document is built here.
fn content_smoke() -> Result<()> {
    use cna::extensions::content::{CnbDocument, CnbTextureData, ReadLimits};

    let (width, height) = (4_u32, 2_u32);
    let rgba: Vec<u8> = (0..width * height)
        .flat_map(|index| {
            let value = (index * 17) as u8;
            [value, value.wrapping_add(1), value.wrapping_add(2), 0xFF]
        })
        .collect();

    let document_bytes = CnbTextureData::from_rgba8(width, height, &rgba)?
        .encode_texture2d("cna-rust-template smoke")?;
    let document = CnbDocument::parse(&document_bytes, "smoke.cnb", ReadLimits::default())?;
    let (major, minor) = document.container_version()?;
    let texture = document.decode_texture2d()?;
    let info = texture.info()?;
    let decoded = texture.level_bytes(0, 0)?;

    println!(
        "cna-rust-template: .cnb v{major}.{minor} {} bytes, asset={:?}, texture {}x{}          round-tripped {}",
        document_bytes.len(),
        document.asset_type()?.name()?,
        info.width,
        info.height,
        if decoded == rgba { "exactly" } else { "WITH LOSS" },
    );
    if decoded != rgba {
        return Err(cna::CnaError::InvalidInput(
            "the .cnb round trip did not return the original pixels",
        ));
    }
    Ok(())
}

fn execute(mode: RunMode) -> Result<()> {
    match mode {
        RunMode::Interactive => run(HelloGame::new()),
        RunMode::ExtensionsSmoke => extensions_smoke(),
        RunMode::Frames(frames) => {
            run_for_frames(HelloGame::new(), frames)?;
            println!(
                "cna-rust-template: completed {frames} real CNA draw frames and shut down cleanly"
            );
            Ok(())
        }
    }
}

fn main() {
    let result = parse_mode()
        .map_err(|message| {
            eprintln!("cna-rust-template: {message}");
            2
        })
        .and_then(|mode| {
            execute(mode).map_err(|error| {
                eprintln!("cna-rust-template: {error}");
                1
            })
        });
    if let Err(code) = result {
        std::process::exit(code);
    }
}
