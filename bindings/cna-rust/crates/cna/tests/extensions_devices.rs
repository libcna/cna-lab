//! CNA's device layer against the live library.
//!
//! The layer is a build option. This test asks CNA whether it is present and
//! then holds the answers to the matching standard, rather than assuming
//! either state.

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};

use cna::extensions::devices::{
    battery_percent, battery_seconds_remaining, clipboard_text, display_content_scale,
    display_safe_area, is_available, logical_cpu_core_count, power_state, preferred_locales,
    set_clipboard_text, system_ram_megabytes, PowerState,
};
use cna::Microsoft::Xna::Framework::{Game, GameContext};
use cna::{run_for_frames, CnaError, ErrorCategory, GameState, GameStateAccess, Result};

/// CNA hosts one game per process, so the tests here take turns.
static ONE_GAME_AT_A_TIME: Mutex<()> = Mutex::new(());

#[derive(Default)]
struct DeviceGame {
    state: Arc<GameState>,
    observed: Arc<AtomicBool>,
}

impl GameStateAccess for DeviceGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

/// A refusal is only acceptable when CNA says the layer is compiled out.
fn accept(available: bool, result: Result<()>) {
    match result {
        Ok(()) => {}
        Err(CnaError::Native {
            category: ErrorCategory::NotSupported,
            ..
        }) => assert!(
            !available,
            "the device layer reports as available but refused a route",
        ),
        Err(error) => panic!("unexpected device-layer failure: {error}"),
    }
}

impl Game for DeviceGame {
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        let available = is_available()?;

        accept(available, power_state(game).map(|state| {
            assert!(
                !matches!(state, PowerState::Unrecognized(_)),
                "unnamed power state: {state:?}",
            );
        }));

        // A percentage and a remaining time are Option: a host that does not
        // know reports nothing rather than a zero that would read as empty.
        accept(available, battery_percent(game).map(|percent| {
            if let Some(percent) = percent {
                assert!((0..=100).contains(&percent), "percent out of range: {percent}");
            }
        }));
        accept(available, battery_seconds_remaining(game).map(|seconds| {
            if let Some(seconds) = seconds {
                assert!(seconds >= 0);
            }
        }));

        accept(available, logical_cpu_core_count(game).map(|count| {
            assert!(count >= 1, "a host always has at least one core: {count}");
        }));
        accept(available, system_ram_megabytes(game).map(|megabytes| {
            assert!(megabytes >= 0);
        }));

        accept(available, preferred_locales(game).map(|locales| {
            for locale in locales {
                assert!(
                    !locale.language.is_empty(),
                    "a locale always names a language",
                );
            }
        }));

        // A headless session has no window, and CNA's canonical answer for
        // that is zero. The projection reports None so it cannot be mistaken
        // for a scale of zero.
        accept(available, display_content_scale(game).map(|scale| {
            if let Some(scale) = scale {
                assert!(scale > 0.0, "a real content scale is positive: {scale}");
            }
        }));
        accept(available, display_safe_area(game).map(|area| {
            assert!(area.Width >= 0 && area.Height >= 0);
        }));

        // Setting the clipboard succeeds when the request was made, which is
        // not the same as the clipboard changing. The test asserts the former
        // and deliberately does not assert the latter.
        accept(available, set_clipboard_text(game, "cna-rust clipboard probe"));
        accept(available, clipboard_text(game).map(|_| ()));

        self.observed.store(true, Ordering::SeqCst);
        Ok(())
    }
}

#[test]
fn the_device_layer_answers_or_says_it_is_absent() {
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let game = DeviceGame::default();
    let observed = Arc::clone(&game.observed);
    run_for_frames(game, 1).expect("one frame reaches LoadContent");
    assert!(observed.load(Ordering::SeqCst), "LoadContent ran");
}

#[derive(Default)]
struct CameraGame {
    state: Arc<GameState>,
    measured: Arc<Mutex<Option<String>>>,
}

impl GameStateAccess for CameraGame {
    fn game_state(&self) -> &Arc<GameState> {
        &self.state
    }
}

impl Game for CameraGame {
    fn LoadContent(&mut self, game: &mut GameContext<'_>) -> Result<()> {
        use cna::extensions::devices::{Camera, CameraState};
        use cna::Microsoft::Xna::Framework::Color;
        use cna::Microsoft::Xna::Framework::Graphics::Texture2D;

        let camera = match Camera::with_test_backend(game) {
            Ok(camera) => camera,
            Err(CnaError::Native { category: ErrorCategory::NotSupported, .. }) => {
                assert!(!is_available()?, "the device layer is present but refused a camera");
                *self.measured.lock().unwrap() = Some("no device layer".to_owned());
                return Ok(());
            }
            Err(error) => return Err(error),
        };
        camera.set_test_state(CameraState::Ready)?;
        assert_eq!(camera.state()?, CameraState::Ready);

        // A frame the test backend produces arrives in a texture exactly.
        let pixels = [Color::Red, Color::Lime, Color::Blue, Color::White];
        camera.set_test_frame(Some((2, 2, &pixels)))?;
        assert_eq!(camera.frame_size()?, (2, 2));
        let device = game.GraphicsDevice()?;
        let texture = Texture2D::new(&device, 2, 2)?;
        assert!(camera.try_acquire_frame(&texture)?, "a queued test frame is acquired");
        let mut read = [Color::Transparent; 4];
        texture.GetData(&mut read)?;
        assert_eq!(read, pixels);
        // Clearing the frame leaves nothing to acquire.
        camera.set_test_frame(None)?;
        assert!(!camera.try_acquire_frame(&texture)?);

        // The platform list, and each entry's facts, answer after the test
        // camera is gone (RUST-UPSTREAM-020 is fixed upstream).
        camera.release()?;
        let count = Camera::count(game)?;
        for index in 0..count {
            let name = Camera::name_at(game, index)?;
            let _position = Camera::position_at(game, index)?;
            assert!(!name.is_empty(), "camera {index} has a name");
        }
        assert!(Camera::name_at(game, count).is_err(), "an index past the count is refused");
        // With no platform camera the default is refused with CNA's own
        // answer. A real camera is only opened on request: switching on a
        // developer's webcam is not something a test suite does by default.
        let default = if count == 0 {
            match Camera::open_default(game) {
                Ok(_) => panic!("a platform with no camera opened a default one"),
                Err(CnaError::Native { category, message, .. }) => {
                    format!("refused {category:?}: {message}")
                }
                Err(error) => return Err(error),
            }
        } else if std::env::var_os("CNA_RUST_OPEN_REAL_CAMERA").is_some() {
            let camera = Camera::open_default(game)?;
            format!("opened, state {:?}", camera.state()?)
        } else {
            "not opened (set CNA_RUST_OPEN_REAL_CAMERA=1 to open the real camera)".to_owned()
        };
        *self.measured.lock().unwrap() = Some(format!("{count} platform camera(s); default {default}"));
        Ok(())
    }
}

#[test]
fn a_test_camera_delivers_its_frame_and_the_platform_list_survives_it() {
    if std::env::var_os("CNA_NATIVE_LIBRARY").is_none() {
        return;
    }
    let _one_game = ONE_GAME_AT_A_TIME
        .lock()
        .unwrap_or_else(std::sync::PoisonError::into_inner);
    let game = CameraGame::default();
    let measured = Arc::clone(&game.measured);
    run_for_frames(game, 1).expect("one frame with a camera");
    let measured = measured.lock().unwrap().clone().expect("the camera case ran");
    println!("camera: {measured}");
}
