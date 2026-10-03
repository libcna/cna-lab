# GamePad native capability inventory

This inventory is the pre-implementation gate for Foundation Milestone 6. It was measured against
the pinned XNA 4.0 Windows runtime assembly and canonical CNA 0.7.0 source/library pair; it does not
infer controller behavior from SDL or from another binding.

## Authorities

- XNA assembly: `/tmp/Microsoft.Xna.Framework.dll`, SHA-256
  `38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`.
- CNA source: commit `a09196a6477f69a7a57c8364f990658d31531a5b`.
- CNA library: `/tmp/cna-java-native-working-070/modules/c-api/libcna_c_api.so`, SHA-256
  `42e099146bf3b470f82fd963a516f8bdd7ff0406da8c37dd53747699117db086`.
- Public headers: `CNA/C/input.h` and `CNA/C/input_gamepad.h` from that source commit.
- Export check: GNU `nm -D --defined-only` against that exact library.

## Required semantic routes

| XNA_REQUIREMENT | CANONICAL_CNA_SYMBOL | HEADER_EXISTS | LIBRARY_EXPORT_EXISTS | C_PROTOTYPE | STATE_OR_CAPABILITY_LAYOUT | SEMANTIC_FIT | THREAD_REQUIREMENT | OWNERSHIP | BACKEND_LIMIT | STATUS |
|---|---|---:|---:|---|---|---|---|---|---|---|
| `GamePad.GetState(PlayerIndex)` | `cna_gamepad_get_state` | YES | YES | `CNA_Result (CNA_Handle, CNA_PlayerIndex, CNA_GamePadState*)` | Versioned 48-byte state POD | Calls CNA `GamePad::GetState(player, IndependentAxes)` and captures a real snapshot | Qualification proved the active Game handle is owner-thread-affine; Swift validates owner thread before entry | Caller-owned copied POD | Positive path needs controller hardware | AVAILABLE |
| `GamePad.GetState(PlayerIndex, GamePadDeadZone)` | `cna_gamepad_get_state_with_dead_zone` | YES | YES | `CNA_Result (CNA_Handle, CNA_PlayerIndex, CNA_GamePadDeadZone, CNA_GamePadState*)` | Same state POD | Passes `None`, `IndependentAxes`, or `Circular` into CNA's XNA GamePad implementation; no Swift-side double processing | Same owner-thread preflight as the default route | Caller-owned copied POD | Positive and boundary observations need controllable hardware; implementation shape is complete | AVAILABLE |
| `GamePad.GetCapabilities(PlayerIndex)` | `cna_gamepad_get_capabilities` | YES | YES | `CNA_Result (CNA_Handle, CNA_PlayerIndex, CNA_GamePadCapabilities*)` | Versioned 48-byte capabilities POD | Copies every selected XNA capability getter, including voice and both vibration motors | Qualification proved owner-thread preflight is required before CNA entry | Caller-owned copied POD | Disconnected is a successful all-false/Unknown snapshot; positive values need hardware | AVAILABLE |
| `GamePad.SetVibration(PlayerIndex, Float, Float)` | `cna_gamepad_set_vibration` | YES | YES | `CNA_Result (CNA_Handle, CNA_PlayerIndex, float, float, CNA_Bool*)` | Scalar inputs plus copied Boolean answer | Calls CNA `GamePad::SetVibration`; `out_applied` is true only when the device accepts the command, while disconnected/unsupported is successful false | Qualification proved owner-thread preflight is required before CNA entry | No retained pointer or owned native object | Positive acceptance and physical motor effect need rumble hardware; input quantization must retain pinned XNA behavior | AVAILABLE; HARDWARE_PENDING |
| Connection status | `cna_gamepad_get_state`, `cna_gamepad_get_capabilities` | YES | YES | As above | `state.is_connected`; `capabilities.is_connected` | Direct values from the canonical managed snapshots | As enclosing query | Copied Boolean | Disconnected is a real negative result | AVAILABLE |
| Packet/change number | `cna_gamepad_get_state`, `cna_gamepad_get_state_with_dead_zone` | YES | YES | As above | `int32_t packet_number` at state offset 12 | Header names this the native packet number; capture copies `GamePadState.PacketNumber` | As state query | Copied signed 32-bit value | Changes require actual input/backend updates | AVAILABLE |
| Physical and virtual button bits | State routes | YES | YES | As state query | `uint32_t pressed_buttons` at state offset 16 | Capture evaluates `GamePadState.IsButtonDown` for bits 0 through 30; selected XNA bit values are identical and mapped explicitly | As state query | Copied mask | CNA extension bits are deliberately not exposed as XNA identities | AVAILABLE |
| DPad | State routes | YES | YES | As state query | Four exact DPad bits in `pressed_buttons` | Up/down/left/right bits are distinct and XNA-compatible | As state query | Copied mask | None | AVAILABLE |
| Left/right sticks | State routes | YES | YES | As state query | Four binary32 fields in `analog`, offsets 24 through 36 | CNA returns XNA-normalized, selected-dead-zone stick values with XNA Y orientation | As state query | Copied binary32 values | Hardware axes determine positive observations | AVAILABLE |
| Left/right triggers | State routes | YES | YES | As state query | Two binary32 fields in `analog`, offsets 40 and 44 | CNA returns XNA-normalized, selected-dead-zone trigger values | As state query | Copied binary32 values | Hardware axes determine positive observations | AVAILABLE |
| Controller type | Capabilities route | YES | YES | As capabilities query | `uint32_t gamepad_type` at offset 8 | CNA explicitly maps canonical device kinds; Swift must map compact CNA value 9 to XNA `BigButtonPad` raw value 768 | As capabilities query | Copied value | Unknown/unrepresentable kinds map to `Unknown` in canonical CNA | AVAILABLE |
| Per-control button capabilities | Capabilities route | YES | YES | As capabilities query | Individual one-byte Booleans for A/B/X/Y, Back/Start/BigButton, DPad, shoulders, and stick clicks | Each field copies its matching CNA/XNA capability getter; connection does not imply presence | As capabilities query | Copied values | Positive diversity requires representative hardware | AVAILABLE |
| Stick-axis and trigger capabilities | Capabilities route | YES | YES | As capabilities query | Individual one-byte Booleans for four stick axes and two triggers | Each field copies its matching CNA/XNA capability getter | As capabilities query | Copied values | Positive diversity requires representative hardware | AVAILABLE |
| Vibration motor capabilities | Capabilities route | YES | YES | As capabilities query | Individual left/right motor Booleans | Both fields copy the canonical managed capability properties | As capabilities query | Copied values | Current backend reports ordinary rumble for both motors together; physical proof is hardware-dependent | AVAILABLE; HARDWARE_PENDING |
| Voice support | Capabilities route | YES | YES | As capabilities query | One-byte `has_voice_support` | Captured from `GamePadCapabilities.HasVoiceSupport`; absence of representation is not being converted to false | As capabilities query | Copied value | Positive proof depends on a backend/device that reports voice | AVAILABLE; HARDWARE_PENDING |
| Player/controller slot identity | All four routes | YES | YES | `CNA_PlayerIndex` is `uint32_t` | Canonical constants One/Two/Three/Four are exactly 0/1/2/3; CNA validates the inclusive four-slot domain before mapping | As enclosing call | Scalar value | No fifth slot | AVAILABLE |

## Measured layouts needed by the binding

`CNA_GamePadState` has size 48 and alignment 4. Its fields are `struct_size` 0,
`struct_version` 4, `is_connected` 8, `reserved0` 9, `packet_number` 12,
`pressed_buttons` 16, `reserved1` 20, and `analog` 24. The nested analog POD has size 24 and
alignment 4: left X/Y at 0/4, right X/Y at 8/12, and triggers at 16/20.

`CNA_GamePadCapabilities` has size 48 and alignment 4. The version prefix occupies offsets 0 and
4, `gamepad_type` is at 8, `is_connected` is at 12, the 34 subsequent one-byte feature fields
occupy offsets 13 through 46, and `reserved[0]` is at 47. The selected Swift snapshot consumes
only the 26 XNA properties; the ten `_ext` capability fields remain internal ABI padding/data and
must not enter the public XNA surface.

The shim and ABI verifier must independently measure every field offset, not merely these summary
boundaries, before the routes are accepted.

## Capability decision

Canonical CNA 0.7.0 contains a semantically real route for every selected static GamePad member.
In particular, PacketNumber, voice support, per-control capabilities, controller type, and the
vibration acceptance Boolean are present rather than synthesized. No CNA API addition, SDL bypass,
native shim behavior, cache, fake disconnected state, or synthetic packet counter is required.

Hardware-independent implementation can therefore proceed. Positive controller state,
capability, and rumble observations remain explicitly `HARDWARE_PENDING` on the current headless
qualification host rather than being promoted to verified hardware behavior.

The public headers require an active Game handle but do not spell out an additional thread rule.
Execution qualification found that presenting the handle from another thread produces canonical
`CNA_RESULT_INVALID_HANDLE`; CNA-Swift therefore applies the existing runtime owner-thread check
before all four routes. The wrong-thread regression observes `CNAError.ownerThreadViolation`, so
the unsafe native function is not entered and no state is mutated.
