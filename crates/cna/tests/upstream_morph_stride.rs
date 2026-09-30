//! RUST-UPSTREAM-024, now fixed upstream: `cna_morph_target_data_ext_create`
//! accepts every canonical vertex stride.
//!
//! CNA's renderer keeps one table of what a stride means --
//! `InferredLayoutForStride` in `VertexDeclarationFidelity.hpp` -- and it lists
//! eleven: 16, 20, 24, 32, 48, 52, 56, 60, 68, 76 and 80. The C API's
//! `ValidateMorphShape` used to restate a stale literal `{32, 52, 56}`, which
//! refused the strides an ordinary physically based glTF mesh gets (48
//! unskinned, 68 skinned). CNA `BINDFIX-007` made it ask the table; this test
//! pins that every canonical stride is accepted and reads back as itself, and
//! that a stride outside the table is still refused.

use cna::extensions::models::{MorphTargetData, MorphTargetDelta, MorphWeightTrack};
use cna::Microsoft::Xna::Framework::Vector3;

/// Every stride `InferredLayoutForStride` has a canonical layout for.
const CANONICAL_STRIDES: [i32; 11] = [16, 20, 24, 32, 48, 52, 56, 60, 68, 76, 80];

const VERTICES: usize = 3;

fn morph_data_of_stride(stride: i32) -> cna::Result<MorphTargetData> {
    // The bytes are never read here -- validation happens before any blend --
    // so zeroed vertices of the right shape are the whole requirement.
    let base = vec![0_u8; stride as usize * VERTICES];
    let targets = vec![MorphTargetDelta {
        position_deltas: vec![Vector3::from_x_and_y_and_z(1.0, 0.0, 0.0); VERTICES],
        normal_deltas: vec![Vector3::from_x_and_y_and_z(0.0, 1.0, 0.0); VERTICES],
    }];
    MorphTargetData::new(&base, stride, &targets, &[0.0], &MorphWeightTrack::default())
}

#[test]
fn every_canonical_stride_is_accepted_and_no_other() {
    if std::env::var_os("CNA_NATIVE_LIBRARY").is_none() {
        return;
    }

    for stride in CANONICAL_STRIDES {
        let data = morph_data_of_stride(stride)
            .unwrap_or_else(|error| panic!("canonical stride {stride} was refused: {error}"));
        // "Accepted" that silently rewrote the stride would be worse than a
        // refusal.
        assert_eq!(
            data.stride().ok(),
            Some(stride),
            "stride {stride} was accepted but does not read back as itself"
        );
    }

    // A stride with no canonical layout is still refused, for that reason.
    let message = match morph_data_of_stride(36) {
        Ok(_) => panic!("stride 36 names no canonical layout and must be refused"),
        Err(error) => error.to_string(),
    };
    println!("NOTE: refused 36: {message}");
    assert!(
        message.contains("does not name a known vertex layout"),
        "stride 36 was refused for an unexpected reason: {message}"
    );
}
