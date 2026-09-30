//! Regression test for the camera test backend's platform override
//! (RUST-UPSTREAM-020).
//!
//! `cna_camera_create_with_test_backend_ext` points CNA's *global* platform
//! override at the camera resource's provider. `cna_camera_destroy` used to free
//! that provider without clearing the override, so the next camera-list query
//! read freed memory (SIGSEGV). CNA fixed it upstream (`BINDFIX-011`); this pins
//! that the query after a destroy answers and the process exits cleanly.
//!
//! It runs in a **child process** because the old failure was a fault.

#![allow(non_snake_case)]

use std::process::Command;

use cna::extensions::devices::Camera;
use cna::Microsoft::Xna::Framework::{Game, GameContext};
use cna::{run_for_frames, CnaError, ErrorCategory, GameState, GameStateAccess, Result};

/// The env var that tells the child which half of the sequence to run.
const STAGE: &str = "CNA_RUST_CAMERA_REPRO_STAGE";

struct CameraGame {
    state: std::sync::Arc<GameState>,
}

impl GameStateAccess for CameraGame {
    fn game_state(&self) -> &std::sync::Arc<GameState> {
        &self.state
    }
}

impl Game for CameraGame {
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let stage = std::env::var(STAGE).unwrap_or_default();
        // Baseline: a test-backend camera, used and then destroyed, with
        // nothing consulting the platform list afterwards.
        // An artifact built without CNA_DEVICES has no camera layer at all.
        // Say so and stop: there is nothing here to measure, and reporting it
        // as a dangling override would be a lie about a build that cannot
        // dangle anything.
        let camera = match Camera::with_test_backend(game) {
            Ok(camera) => camera,
            Err(CnaError::Native { category: ErrorCategory::NotSupported, ref message, .. }) => {
                println!("REPRO: no device layer -- {message}");
                return Ok(());
            }
            Err(error) => return Err(error),
        };
        camera.set_test_state(cna::extensions::devices::CameraState::Ready)?;
        println!("REPRO: created, state={:?}", camera.state()?);
        camera.release()?;
        println!("REPRO: destroyed");
        if stage == "after-destroy" {
            // The one extra call. It walks Camera::getAvailableCamerasProperty,
            // which consults the override the destroyed resource still owns.
            let count = Camera::count(game)?;
            println!("REPRO: count after destroy = {count}");
        }
        println!("REPRO: survived");
        Ok(())
    }
}

fn run_stage() -> Result<()> {
    let game = CameraGame {
        state: std::sync::Arc::new(GameState::default()),
    };
    run_for_frames(game, 1)
}

#[test]
fn destroying_a_test_camera_clears_the_platform_override() {
    if std::env::var_os("CNA_NATIVE_LIBRARY").is_none() {
        return;
    }
    if std::env::var(STAGE).is_ok() {
        // We are the child: run the stage and let the fault speak for itself.
        run_stage().expect("the staged camera sequence");
        return;
    }

    let exe = std::env::current_exe().expect("this test binary");
    let mut outcomes = Vec::new();
    for stage in ["baseline", "after-destroy"] {
        let output = Command::new(&exe)
            .args([
                "--test-threads=1",
                "--nocapture",
                "--exact",
                "destroying_a_test_camera_clears_the_platform_override",
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
            "--- stage {stage}: status={status:?} survived={} ---",
            text.contains("REPRO: survived")
        );
        for line in text.lines().filter(|l| l.starts_with("REPRO:")) {
            println!("    {line}");
        }
    }

    let (_, baseline_status, baseline_text) = &outcomes[0];
    if baseline_text.contains("REPRO: no device layer") {
        println!(
            "this artifact was built without the extended device layer, so there is no \
             camera to destroy; the camera lifecycle is measured on an artifact that has one"
        );
        assert!(
            baseline_status.success(),
            "an artifact without the device layer must still refuse cleanly: {baseline_status:?}"
        );
        return;
    }
    assert!(
        baseline_text.contains("REPRO: destroyed"),
        "the baseline created and destroyed a test-backend camera"
    );
    assert!(
        baseline_status.success() && baseline_text.contains("REPRO: survived"),
        "creating and destroying a test camera is itself fine: {baseline_status:?}"
    );

    // The one call that used to read the freed provider.
    let (_, after_status, after_text) = &outcomes[1];
    assert!(
        after_status.success() && after_text.contains("REPRO: survived"),
        "querying the camera list after a destroy failed -- RUST-UPSTREAM-020 is back: \
         {after_status:?}"
    );
}
