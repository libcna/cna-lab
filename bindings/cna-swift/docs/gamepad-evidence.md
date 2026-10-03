# Foundation Milestone 6 — XNA GamePad evidence

## Result and authority

The dependency-complete public closure is exactly these eleven XNA types:

`ButtonState`, `Buttons`, `GamePad`, `GamePadButtons`, `GamePadCapabilities`,
`GamePadDPad`, `GamePadDeadZone`, `GamePadState`, `GamePadThumbSticks`,
`GamePadTriggers`, and `GamePadType`.

The closure was regenerated from the pinned XNA metadata. Its only external
public-signature dependencies are the already-complete `PlayerIndex` and
`Vector2`; neither Mouse nor Touch is required. The selected contract contains
132 CLR identities and 128 mapped Swift identities. The four omitted identities
are the synthetic `value__` storage fields on `ButtonState`, `Buttons`,
`GamePadDeadZone`, and `GamePadType`. This is a formal language projection, not
a diagnostic suppression.

Behavior comes from the pinned XNA reference IL. Native routing comes only from
the canonical CNA 0.7.0 public C headers and exact loaded library described in
`gamepad-native-inventory.md`. No CNA source, SDL API, platform controller API,
local native semantic shim, packet cache, or fabricated capability is used.

## Structural matrix

| Type | CLR source members | Expected Swift | Target Swift | Local diagnostics | Swift kind | Behavior | Native |
|---|---:|---:|---:|---:|---|---|---|
| ButtonState | 3 | 2 | 2 | 0 | `enum: Int32` | VERIFIED_MANAGED | N/A |
| Buttons | 26 | 25 | 25 | 0 | `OptionSet<Int32>` | VERIFIED_MANAGED | Explicit bit mapping |
| GamePad | 4 | 4 | 4 | 0 | `final class`, private init | VERIFIED_MANAGED | VERIFIED_NATIVE_ROUTE |
| GamePadButtons | 17 | 17 | 17 | 0 | `struct` | VERIFIED_MANAGED | Snapshot value |
| GamePadCapabilities | 26 | 26 | 26 | 0 | `struct` | VERIFIED_MANAGED | VERIFIED_NATIVE_ROUTE |
| GamePadDPad | 10 | 10 | 10 | 0 | `struct` | VERIFIED_MANAGED | Snapshot value |
| GamePadDeadZone | 4 | 3 | 3 | 0 | `enum: Int32` | VERIFIED_MANAGED | Explicit mode mapping |
| GamePadState | 15 | 15 | 15 | 0 | `struct` | VERIFIED_MANAGED | VERIFIED_NATIVE_ROUTE |
| GamePadThumbSticks | 8 | 8 | 8 | 0 | `struct` | VERIFIED_MANAGED | Snapshot value |
| GamePadTriggers | 8 | 8 | 8 | 0 | `struct` | VERIFIED_MANAGED | Snapshot value |
| GamePadType | 11 | 10 | 10 | 0 | `enum: Int32` | VERIFIED_MANAGED | Explicit type mapping |
| **Total** | **132** | **128** | **128** | **0** |  |  |  |

For every selected type, type-kind, base, interface, field, property, method,
parameter, return, overload, generic, enum, flags, event, operator, ref/out, and
language-mapping diagnostics are zero. No selected type is partial. Manual and
applied allowlists remain zero, and no structural category is unmeasured.

## Enums and flags

`ButtonState` is the non-flags `Int32` enum `Released=0`, `Pressed=1`.
`GamePadDeadZone` is the non-flags `Int32` enum `None=0`,
`IndependentAxes=1`, `Circular=2`. `GamePadType` is the non-flags `Int32` enum
`Unknown=0`, `GamePad=1`, `Wheel=2`, `ArcadeStick=3`, `FlightStick=4`,
`DancePad=5`, `Guitar=6`, `AlternateGuitar=7`, `DrumKit=8`, and
`BigButtonPad=768`.

`Buttons` uses the established `[Flags]` → Swift `OptionSet` projection with an
`Int32` raw value. Every constant is explicit:

| Button | Raw value | Button | Raw value |
|---|---:|---|---:|
| DPadUp | 1 | DPadDown | 2 |
| DPadLeft | 4 | DPadRight | 8 |
| Start | 16 | Back | 32 |
| LeftStick | 64 | RightStick | 128 |
| LeftShoulder | 256 | RightShoulder | 512 |
| BigButton | 2,048 | A | 4,096 |
| B | 8,192 | X | 16,384 |
| Y | 32,768 | LeftThumbstickLeft | 2,097,152 |
| RightTrigger | 4,194,304 | LeftTrigger | 8,388,608 |
| RightThumbstickUp | 16,777,216 | RightThumbstickDown | 33,554,432 |
| RightThumbstickRight | 67,108,864 | RightThumbstickLeft | 134,217,728 |
| LeftThumbstickUp | 268,435,456 | LeftThumbstickDown | 536,870,912 |
| LeftThumbstickRight | 1,073,741,824 |  |  |

Arbitrary and undefined `Int32` combinations retain their exact bit patterns.
Set union and combined constants are not normalized to names. The verifier
adds targeted failures for a normal enum, wrong raw type, wrong ordinary or
high-bit value, missing `BigButton`, and missing/incorrect flags metadata. The
ordinary Swift `OptionSet` initializer, `rawValue`, witnesses, and inherited set
operations are language surface rather than XNA unexpected members.

## Managed value structs

### GamePadButtons and GamePadDPad

`GamePadButtons(Buttons)` projects only the eleven physical properties `A`,
`B`, `Back`, `X`, `Y`, `Start`, `LeftShoulder`, `LeftStick`, `RightShoulder`,
`RightStick`, and `BigButton`. Trigger and directional-stick virtual flags do
not leak as properties. Its string order is A, B, X, Y, LeftShoulder,
RightShoulder, LeftStick, RightStick, Start, Back, BigButton and the empty form
is `{Buttons:None}`.

`GamePadDPad` accepts constructor values in Up, Down, Left, Right order while
the public property order is Up, Down, Right, Left. Asymmetric fixtures prevent
left/right transposition. Its string order is Up, Down, Left, Right and its
empty form is `{DPad:None}`.

Both types implement only `Equals(Any?)`, exact operators, the reference
`SmartGetHashCode` word-XOR behavior (zero XOR maps to `Int32.max`), and their
exact reference strings. No typed `Equals` overload was invented.

### GamePadTriggers

The public constructor evaluates `System.Math.Min(value, 1)` and then
`System.Math.Max(result, 0)` in that order for each binary32 component. Finite
values clamp to `[0,1]`; negative values and negative zero become positive zero;
positive and negative infinity become one and zero; NaN and its payload remain
NaN. Equality uses exact floating comparison, so a NaN-containing value is not
equal to itself. Hashing uses the resulting raw binary32 words through the
reference smart hash, and `ToString` is `{Left:<x> Right:<y>}` with XNA float
formatting.

### GamePadThumbSticks

The public constructor applies `Vector2.Min(value, Vector2.One)` followed by
`Vector2.Max(result, -Vector2.One)` independently to all four components. It is
a square component clamp, not radial normalization: `(1,1)` remains `(1,1)`.
Signed zero is retained. The pinned Vector2 comparison order maps NaN and both
infinities to a bound (`NaN`/`+Inf` to `+1`, `-Inf` to `-1`). Equality, smart
hashing, and `{Left:<Vector2> Right:<Vector2>}` formatting use the resulting
values. Returned vectors and assignments are ordinary Swift value copies.

### GamePadCapabilities

There is no public initializer. The internal snapshot initializer does not
enter the Symbol Graph. The 26 public read-only properties map one-for-one:

| XNA property | Canonical CNA field |
|---|---|
| GamePadType | `gamepad_type` through explicit type mapping |
| IsConnected | `is_connected` |
| HasAButton / HasBackButton / HasBButton | `has_a_button` / `has_back_button` / `has_b_button` |
| HasDPadDownButton / HasDPadLeftButton | `has_dpad_down_button` / `has_dpad_left_button` |
| HasDPadRightButton / HasDPadUpButton | `has_dpad_right_button` / `has_dpad_up_button` |
| HasLeftShoulderButton / HasLeftStickButton | `has_left_shoulder_button` / `has_left_stick_button` |
| HasRightShoulderButton / HasRightStickButton | `has_right_shoulder_button` / `has_right_stick_button` |
| HasStartButton | `has_start_button` |
| HasXButton / HasYButton / HasBigButton | `has_x_button` / `has_y_button` / `has_big_button` |
| HasLeftXThumbStick / HasLeftYThumbStick | `has_left_x_thumb_stick` / `has_left_y_thumb_stick` |
| HasRightXThumbStick / HasRightYThumbStick | `has_right_x_thumb_stick` / `has_right_y_thumb_stick` |
| HasLeftTrigger / HasRightTrigger | `has_left_trigger` / `has_right_trigger` |
| HasLeftVibrationMotor / HasRightVibrationMotor | `has_left_vibration_motor` / `has_right_vibration_motor` |
| HasVoiceSupport | `has_voice_support` |

Connection is never used as a shortcut for control presence. CNA has a real
voice field and two real motor fields, so unknown is not converted to false.
Compact CNA controller types 0…9 map explicitly to XNA types; CNA 9 maps to
`BigButtonPad(768)`, and an unrecognized CNA type maps to `Unknown`.

## GamePadState

Both public constructors are distinct, have no default or variadic collapse,
and set `IsConnected=true`, `PacketNumber=0` exactly as the reference IL. The
component constructor retains its four managed values. The value constructor
square-clamps the sticks, clamps the triggers, ORs every entry of `[Buttons]`
(including combined entries and duplicates), and builds physical buttons and
DPad from the result. Unknown bits remain representable as `Buttons` values but
are not reported down by a state with no matching XNA control.

The internal native initializer alone supplies `IsConnected` and the real
signed 32-bit `PacketNumber`; it is not public and creates no constructor
identity. Native state, capabilities, and all nested properties are copied
snapshots with no handles or retained native memory.

The derived virtual-button thresholds are based on the reference's binary32
to-device quantization:

- left X/Y uses `Int32(component * 32767)` and strict `< -7849` / `> 7849`;
- right X/Y uses strict `< -8689` / `> 8689`;
- triggers use `Int32(component * 255)` and strict `> 30`;
- positive Y means Up and negative Y means Down.

Every physical, DPad, stick-click, BigButton, eight virtual stick-direction,
and two virtual-trigger identity has a direct fixture. For a combined request,
XNA requires **all** requested bits: `(pressed & requested) == requested`.
Therefore zero is down and not up, mixed active/inactive combinations are up,
and unknown nonzero bits are up. `IsButtonUp` is the exact negation.

Equality and operators compare connection, packet number, thumbsticks,
triggers, physical buttons, and DPad. The private derived mask does not
participate. Hashing XORs the corresponding nested reference hashes, Boolean,
and packet number. The exact string intentionally contains only connection:
`{IsConnected:True}` or `{IsConnected:False}`.

## Static GamePad and canonical CNA

`GamePad` is a nonconstructible `final class` with four static throwing Swift
methods. `throws` is the established Swift native-failure projection, not an
extra XNA member.

| XNA member | Canonical symbol | Swift native positions | Thread and result policy |
|---|---|---|---|
| GetState(PlayerIndex) | `cna_gamepad_get_state` | handle, `UInt32`, state pointer → `UInt32` | Owner preflight; defaults to IndependentAxes; copied state |
| GetState(PlayerIndex, GamePadDeadZone) | `cna_gamepad_get_state_with_dead_zone` | handle, `UInt32`, `UInt32`, state pointer → `UInt32` | Owner preflight; explicit mode; copied state |
| GetCapabilities(PlayerIndex) | `cna_gamepad_get_capabilities` | handle, `UInt32`, capabilities pointer → `UInt32` | Owner preflight; copied 26-property snapshot |
| SetVibration(PlayerIndex, Float, Float) | `cna_gamepad_set_vibration` | handle, `UInt32`, `Float`, `Float`, Boolean pointer → `UInt32` | Owner preflight; returns device-accepted Boolean |

PlayerIndex One/Two/Three/Four map through a reviewed switch to canonical slots
0/1/2/3. State button flags map one-by-one from all 25 measured canonical bits;
there is no raw mask cast and no extension-bit leakage. DPad ordering is Up,
Down, Left, Right. Stick X/Y, right/left identity, trigger identity, XNA Y sign,
and binary32 widths are fixed by the canonical POD and headers.

The process-global route resolves `RuntimeRegistry.current` for each operation;
GamePad owns no Game and retains no handle. A Game 1 query, destruction, Game 2
query test proves the second operation uses Game 2's generation. Qualification
also established that the active CNA Game handle is owner-thread-affine even
though the route header does not add a separate thread sentence: a wrong-thread
probe produced CNA invalid-handle. The final mapping validates the owner thread
before native entry and the regression observes `ownerThreadViolation` safely.
Keyboard was not refactored and its two routes remain unchanged.

### Dead zones

The one-argument overload delegates to canonical IndependentAxes. Constants are
left `7849/32768`, right `8689/32768`, and trigger `30/255`, all compiler-measured
binary32 constants.

- None preserves the normalized raw stick components and then square-clamps to
  `[-1,1]`; triggers clamp to `[0,1]`.
- IndependentAxes processes each stick component independently. Values within
  `[-d,d]` become zero; outside it the signed magnitude is rescaled as
  `(abs(value)-d)/(1-d)`, then square-clamped. Left and right use their distinct
  constants. Both triggers use the trigger threshold and the same scalar rule.
- Circular uses vector length. Length `<= d` becomes zero; otherwise the vector
  is multiplied by `((length-d)/(1-d))/length`, then a vector with magnitude
  above one is normalized. Trigger processing is the same as IndependentAxes.

The strict at/adjacent boundaries are retained in the managed virtual-button
fixtures. Native dead-zone implementation and constant shape are canonical CNA
source/header evidence; physical analog boundary observations remain hardware
pending on this host, so none are falsely recorded as measured controller data.

### Vibration

Pinned XNA multiplies each input by 65,535 and stores the unchecked `conv.i2`
word before native entry. CNA accepts normalized floats, so Swift preserves that
16-bit word and divides its unsigned representation by 65,535. This is not a
clamp: examples are `-0.25→49153`, `0.5→32767`, `1→65535`, and
`1.25→16382`; pinned nonfinite conversion yields zero. CNA `out_applied` is
true only if the device accepts the request; disconnected or unsupported is a
successful false. Other CNA failures throw. No no-op success or cache exists.

## Native ABI delta

| Counter | Before | After |
|---|---:|---:|
| Bound functions | 25 | 29 |
| Prototype type positions | 72 | 91 |
| C/Swift measurements | 72 | 91 |
| Layouts | 15 | 18 |
| Callbacks | 2 | 2 |
| Constants | 168 | 214 |

The four new bound functions are exactly the four public static routes. The
three new layouts are `CNASwift_GamePadAnalogState` (24 bytes/alignment 4),
`CNASwift_GamePadState` (48/4), and `CNASwift_GamePadCapabilities` (48/4), with
every field offset measured. The 46 new constants are four player slots, three
dead-zone modes, three thresholds, empty plus 25 button masks, and ten
controller types. Final ABI status is `MISSING_HEADER_SYMBOLS=0`,
`MISSING_LIBRARY_SYMBOLS=0`, `ABI_MISMATCHES=0`.

## Qualification and limitations

The pure XNA-derived corpus moved from 986 to 1,239 observations and assertions
with zero failures. New groups are BUTTON_STATE 1, BUTTONS 1, GAMEPAD_BUTTONS 1,
GAMEPAD_DPAD 1, GAMEPAD_TRIGGERS 1, GAMEPAD_THUMBSTICKS 1, GAMEPAD_STATE 4,
GAMEPAD_ENUMS 1, and GAMEPAD_CAPABILITIES 1. Canonical CNA observations are
deliberately stored in the separate generated GamePad native report.

The current Linux x86-64 HEADLESS/NULL host has no connected controller. It
proved successful real disconnected snapshots for all four state modes,
all-false/Unknown disconnected capabilities, `SetVibration=false`, 50 cycles
per state mode (200 calls), 20 capability calls, generation replacement, and
safe wrong-thread rejection. Positive state, capability diversity, controller
type diversity, voice, motor capability, and physical vibration remain
`HARDWARE_PENDING`. Repeated vibration stress was not run without hardware.
These pending physical observations do not hide an upstream semantic gap: the
canonical routes contain real PacketNumber, per-control fields, voice, type,
and vibration acceptance data.
