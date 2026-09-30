//! Regression test for a content-loaded `Model`'s teardown (RUST-UPSTREAM-021).
//!
//! `MeshResource::~MeshResource` used to move an empty `detachedValue` over a
//! content-loaded part's live `value`, and `~PartResource` then dereferenced
//! it: releasing such a model faulted, and so did merely leaking it, because
//! the C API's handle registry runs the same destructor at process exit. CNA
//! fixed it upstream (`BINDFIX-006`); re-measured against ABI 0.35.
//!
//! This runs in **child processes** because the old failure was a fault, not
//! a result code. Two stages run here and both must now exit cleanly:
//!
//! * `destroy` releases the model and carries on;
//! * `leak` never releases it and leaves it to the registry at exit.
//!
//! Full write-up: `RUST-UPSTREAM-021` in `docs/upstream-findings.md`.

use std::path::{Path, PathBuf};
use std::process::Command;

use cna::extensions::content::NativeContentManager;
use cna::extensions::native_model::NativeModel;
use cna::Microsoft::Xna::Framework::Graphics::{
    GraphicsDevice, GraphicsProfile, PresentationParameters,
};
use cna::Microsoft::Xna::Framework::GraphicsDeviceInformation;
use cna::{CnaError, ErrorCategory, Result};

const STAGE: &str = "CNA_RUST_MODEL_DESTROY_STAGE";

/// One triangle under two nodes.
///
/// A glTF with no mesh at all is not a usable control: CNA's importer refuses
/// it outright -- "contains no mesh instances to import" -- so there is no such
/// thing as a loaded model without a part to compare against. The control is
/// the hand-built model in the C probe instead.
const ONE_PART: &str = r#"{
  "asset": { "version": "2.0" }, "scene": 0,
  "scenes": [ { "nodes": [ 0 ] } ],
  "nodes": [ { "name": "Root", "children": [ 1 ], "mesh": 0 }, { "name": "Child" } ],
  "meshes": [ { "name": "Triangle",
                "primitives": [ { "attributes": { "POSITION": 0 } } ] } ],
  "accessors": [ { "bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3",
                   "min": [ 0.0, 0.0, 0.0 ], "max": [ 1.0, 1.0, 0.0 ] } ],
  "bufferViews": [ { "buffer": 0, "byteOffset": 0, "byteLength": 36 } ],
  "buffers": [ { "byteLength": 36, "uri": "data:application/octet-stream;base64,AAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAA" } ] }"#;

fn content_root() -> PathBuf {
    let root = Path::new(env!("CARGO_TARGET_TMPDIR")).join("model-destroy-repro");
    std::fs::create_dir_all(&root).expect("content root");
    std::fs::write(root.join("onepart.gltf"), ONE_PART).expect("write the one-part asset");
    root
}

fn host() -> Option<(GraphicsDevice, NativeContentManager)> {
    let parameters = PresentationParameters::new();
    parameters.SetBackBufferWidth(64);
    parameters.SetBackBufferHeight(64);
    let device = match GraphicsDevice::new(
        &GraphicsDeviceInformation::new().Adapter(),
        GraphicsProfile::HiDef,
        &parameters,
    ) {
        Ok(device) => device,
        Err(CnaError::Native {
            category: ErrorCategory::Platform,
            ref message,
            ..
        }) if message.contains("platform window id") => {
            println!("REPRO: skip -- this renderer needs a window: {message}");
            return None;
        }
        Err(error) => panic!(
            "independent GraphicsDevice construction failed with something other than \
             the renderer's no-window refusal, which is the only failure this skips. \
             Seen once under a parallel full-suite run and not reproduced since, so the \
             exact text matters: {error:?}"
        ),
    };
    let root = content_root();
    let manager = NativeContentManager::new(&device, root.to_str().expect("utf-8 root"))
        .expect("native content manager");
    Some((device, manager))
}

fn run_stage(stage: &str) -> Result<()> {
    let Some((_device, manager)) = host() else {
        return Ok(());
    };
    let model = NativeModel::load(&manager, "onepart")?;
    let meshes = model.mesh_count()?;
    let parts: u64 = model
        .meshes()?
        .iter()
        .map(|mesh| mesh.part_count().unwrap_or(0))
        .sum();
    println!("REPRO: loaded: {meshes} mesh(es), {parts} part(s)");
    if stage == "leak" {
        // Never released. The fault still comes, at process exit, from the same
        // destructor -- which is the point of this stage.
        core::mem::forget(model);
        println!("REPRO: leaked");
        return Ok(());
    }
    model.release()?;
    println!("REPRO: destroyed");
    Ok(())
}

#[test]
fn destroying_or_leaking_a_content_loaded_model_is_clean() {
    if std::env::var_os("CNA_NATIVE_LIBRARY").is_none() {
        return;
    }
    if let Ok(stage) = std::env::var(STAGE) {
        run_stage(&stage).expect("the staged load and destroy");
        println!("REPRO: survived {stage}");
        return;
    }

    let exe = std::env::current_exe().expect("this test binary");
    let mut outcomes = Vec::new();
    for stage in ["destroy", "leak"] {
        let output = Command::new(&exe)
            .args([
                "--test-threads=1",
                "--nocapture",
                "--exact",
                "destroying_or_leaking_a_content_loaded_model_is_clean",
            ])
            .env(STAGE, stage)
            .output()
            .expect("run the staged child");
        let text = String::from_utf8_lossy(&output.stdout).into_owned()
            + &String::from_utf8_lossy(&output.stderr);
        outcomes.push((stage, output.status, text));
    }

    for (stage, status, text) in &outcomes {
        println!(
            "--- stage {stage}: status={status:?}, {} bytes captured ---",
            text.len()
        );
        for line in text.lines().filter(|line| line.contains("REPRO:")) {
            println!("    {}", line.trim());
        }
    }

    let (_, destroy_status, destroy_text) = &outcomes[0];
    if destroy_text.contains("REPRO: skip") {
        println!("SKIP: this renderer cannot make a windowless device");
        return;
    }
    assert!(
        destroy_text.contains("REPRO: loaded"),
        "the asset must load before its teardown can be measured"
    );

    // Stage one: releasing the model comes back.
    assert!(
        destroy_status.success() && destroy_text.contains("REPRO: survived"),
        "destroying a content-loaded model with a mesh part failed -- RUST-UPSTREAM-021 \
         is back. status={destroy_status:?}"
    );

    // Stage two: leaving it to the registry at exit is clean too.
    let (_, leak_status, leak_text) = &outcomes[1];
    assert!(
        leak_text.contains("REPRO: leaked") && leak_status.success(),
        "a leaked content-loaded model faulted at process exit -- RUST-UPSTREAM-021 is \
         back. status={leak_status:?}"
    );
}
