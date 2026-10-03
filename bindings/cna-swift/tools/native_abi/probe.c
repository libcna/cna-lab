// SPDX-License-Identifier: MIT
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "CNA/C/cna.h"
#include "CNAShim.h"

/* CNA's own rule, from docs/c-api/ABI_VERSIONING.md and the installed
   package's SameMajorVersion compatibility file: reject a different major,
   require a minimum minor. CNA_SWIFT_ABI_MAJOR/MINOR are supplied by
   tools/native_abi/verify.py out of Sources/CNA/Native/NativeFunctions.swift,
   so the C wall and the Swift runtime cannot drift apart. */
_Static_assert(CNA_ABI_VERSION_MAJOR == (uint32_t) (CNA_SWIFT_ABI_MAJOR),
    "canonical header major is outside the admitted CNA C ABI window");
_Static_assert(CNA_ABI_VERSION_MINOR >= (uint32_t) (CNA_SWIFT_ABI_MINOR),
    "canonical header minor is below the admitted CNA C ABI minimum");
_Static_assert(CNA_ABI_VERSION == CNA_ABI_VERSION_ENCODE(
    CNA_ABI_VERSION_MAJOR, CNA_ABI_VERSION_MINOR, CNA_ABI_VERSION_PATCH),
    "canonical ABI encoding is not major<<16 | minor<<8 | patch");
_Static_assert(sizeof(CNA_Bool) == 1, "CNA_Bool width");
_Static_assert(CNA_FALSE == 0 && CNA_TRUE == 1, "CNA_Bool values");
_Static_assert(CNA_RESULT_SUCCESS == 0, "CNA_Result success");
_Static_assert(CNA_RESULT_IO == 5, "CNA_Result IO");
_Static_assert(CNA_RESULT_BUFFER_TOO_SMALL == 14, "CNA_Result buffer too small");
_Static_assert(CNA_SPRITE_SORT_MODE_DEFERRED == 0, "SpriteSortMode.Deferred");
_Static_assert(CNA_SPRITE_EFFECT_NONE == 0, "SpriteEffects.None");
_Static_assert(CNA_SPRITE_EFFECT_FLIP_HORIZONTALLY == 1, "SpriteEffects.FlipHorizontally");
_Static_assert(CNA_SPRITE_EFFECT_FLIP_VERTICALLY == 2, "SpriteEffects.FlipVertically");
_Static_assert(CNA_PLAYER_INDEX_ONE == 0 && CNA_PLAYER_INDEX_TWO == 1 &&
    CNA_PLAYER_INDEX_THREE == 2 && CNA_PLAYER_INDEX_FOUR == 3, "PlayerIndex slots");
_Static_assert(CNA_GAMEPAD_DEAD_ZONE_NONE == 0, "GamePadDeadZone.None");
_Static_assert(CNA_GAMEPAD_DEAD_ZONE_INDEPENDENT_AXES == 1,
    "GamePadDeadZone.IndependentAxes");
_Static_assert(CNA_GAMEPAD_DEAD_ZONE_CIRCULAR == 2, "GamePadDeadZone.Circular");
_Static_assert(CNA_GAMEPAD_BUTTON_NONE == 0, "Buttons empty mask");
_Static_assert(CNA_GAMEPAD_BUTTON_DPAD_UP == 1, "Buttons.DPadUp");
_Static_assert(CNA_GAMEPAD_BUTTON_DPAD_DOWN == 2, "Buttons.DPadDown");
_Static_assert(CNA_GAMEPAD_BUTTON_DPAD_LEFT == 4, "Buttons.DPadLeft");
_Static_assert(CNA_GAMEPAD_BUTTON_DPAD_RIGHT == 8, "Buttons.DPadRight");
_Static_assert(CNA_GAMEPAD_BUTTON_START == 16, "Buttons.Start");
_Static_assert(CNA_GAMEPAD_BUTTON_BACK == 32, "Buttons.Back");
_Static_assert(CNA_GAMEPAD_BUTTON_LEFT_STICK == 64, "Buttons.LeftStick");
_Static_assert(CNA_GAMEPAD_BUTTON_RIGHT_STICK == 128, "Buttons.RightStick");
_Static_assert(CNA_GAMEPAD_BUTTON_LEFT_SHOULDER == 256, "Buttons.LeftShoulder");
_Static_assert(CNA_GAMEPAD_BUTTON_RIGHT_SHOULDER == 512, "Buttons.RightShoulder");
_Static_assert(CNA_GAMEPAD_BUTTON_BIG_BUTTON == 2048, "Buttons.BigButton");
_Static_assert(CNA_GAMEPAD_BUTTON_A == 4096, "Buttons.A");
_Static_assert(CNA_GAMEPAD_BUTTON_B == 8192, "Buttons.B");
_Static_assert(CNA_GAMEPAD_BUTTON_X == 16384, "Buttons.X");
_Static_assert(CNA_GAMEPAD_BUTTON_Y == 32768, "Buttons.Y");
_Static_assert(CNA_GAMEPAD_BUTTON_LEFT_THUMBSTICK_LEFT == 2097152,
    "Buttons.LeftThumbstickLeft");
_Static_assert(CNA_GAMEPAD_BUTTON_RIGHT_TRIGGER == 4194304, "Buttons.RightTrigger");
_Static_assert(CNA_GAMEPAD_BUTTON_LEFT_TRIGGER == 8388608, "Buttons.LeftTrigger");
_Static_assert(CNA_GAMEPAD_BUTTON_RIGHT_THUMBSTICK_UP == 16777216,
    "Buttons.RightThumbstickUp");
_Static_assert(CNA_GAMEPAD_BUTTON_RIGHT_THUMBSTICK_DOWN == 33554432,
    "Buttons.RightThumbstickDown");
_Static_assert(CNA_GAMEPAD_BUTTON_RIGHT_THUMBSTICK_RIGHT == 67108864,
    "Buttons.RightThumbstickRight");
_Static_assert(CNA_GAMEPAD_BUTTON_RIGHT_THUMBSTICK_LEFT == 134217728,
    "Buttons.RightThumbstickLeft");
_Static_assert(CNA_GAMEPAD_BUTTON_LEFT_THUMBSTICK_UP == 268435456,
    "Buttons.LeftThumbstickUp");
_Static_assert(CNA_GAMEPAD_BUTTON_LEFT_THUMBSTICK_DOWN == 536870912,
    "Buttons.LeftThumbstickDown");
_Static_assert(CNA_GAMEPAD_BUTTON_LEFT_THUMBSTICK_RIGHT == 1073741824,
    "Buttons.LeftThumbstickRight");
/* ClearOptions crosses the boundary as a bit mask, and no runtime observation
 * can tell which buffers a HEADLESS clear touched -- so these three are the
 * only evidence that NativeStateCodes.clearOptions maps the right bits. */
_Static_assert(CNA_CLEAR_OPTION_TARGET == 1, "ClearOptions.Target");
_Static_assert(CNA_CLEAR_OPTION_DEPTH_BUFFER == 2, "ClearOptions.DepthBuffer");
_Static_assert(CNA_CLEAR_OPTION_STENCIL == 4, "ClearOptions.Stencil");

/* Every draw passes PrimitiveType.rawValue straight into the route's
 * CNA_PrimitiveType, and no observation on this host can catch a swap: a
 * TriangleList drawn as a TriangleStrip is accepted and renders nothing anyone
 * can read back. These four are the whole of the evidence that the
 * pass-through is safe. CNA additionally declares a POINT_LIST extension at 4
 * which XNA has no member for, so the enum is narrower than the ABI and the
 * projection can only ever send 0 through 3. */
_Static_assert(CNA_PRIMITIVE_TRIANGLE_LIST == 0, "PrimitiveType.TriangleList");
_Static_assert(CNA_PRIMITIVE_TRIANGLE_STRIP == 1, "PrimitiveType.TriangleStrip");
_Static_assert(CNA_PRIMITIVE_LINE_LIST == 2, "PrimitiveType.LineList");
_Static_assert(CNA_PRIMITIVE_LINE_STRIP == 3, "PrimitiveType.LineStrip");

/* TextureCube.SetData/GetData pass CubeMapFace.rawValue straight into
 * CNA_TextureCubeTransfer::face, so the six values are a boundary dependency,
 * and no observation on the qualified artifact can catch a mismatch: the
 * transfer answers NOT_SUPPORTED whichever face it is given. These six
 * assertions are the only evidence that the pass-through is safe. Foundation
 * 45 found BlendFunction's Min and Max swapped between the two headers, so
 * agreement is checked and never assumed. */
_Static_assert(CNA_CUBE_MAP_FACE_POSITIVE_X == 0, "CubeMapFace.PositiveX");
_Static_assert(CNA_CUBE_MAP_FACE_NEGATIVE_X == 1, "CubeMapFace.NegativeX");
_Static_assert(CNA_CUBE_MAP_FACE_POSITIVE_Y == 2, "CubeMapFace.PositiveY");
_Static_assert(CNA_CUBE_MAP_FACE_NEGATIVE_Y == 3, "CubeMapFace.NegativeY");
_Static_assert(CNA_CUBE_MAP_FACE_POSITIVE_Z == 4, "CubeMapFace.PositiveZ");
_Static_assert(CNA_CUBE_MAP_FACE_NEGATIVE_Z == 5, "CubeMapFace.NegativeZ");

_Static_assert(CNA_GAMEPAD_TYPE_UNKNOWN == 0, "GamePadType.Unknown");
_Static_assert(CNA_GAMEPAD_TYPE_GAMEPAD == 1, "GamePadType.GamePad");
_Static_assert(CNA_GAMEPAD_TYPE_WHEEL == 2, "GamePadType.Wheel");
_Static_assert(CNA_GAMEPAD_TYPE_ARCADE_STICK == 3, "GamePadType.ArcadeStick");
_Static_assert(CNA_GAMEPAD_TYPE_FLIGHT_STICK == 4, "GamePadType.FlightStick");
_Static_assert(CNA_GAMEPAD_TYPE_DANCE_PAD == 5, "GamePadType.DancePad");
_Static_assert(CNA_GAMEPAD_TYPE_GUITAR == 6, "GamePadType.Guitar");
_Static_assert(CNA_GAMEPAD_TYPE_ALTERNATE_GUITAR == 7,
    "GamePadType.AlternateGuitar");
_Static_assert(CNA_GAMEPAD_TYPE_DRUM_KIT == 8, "GamePadType.DrumKit");
_Static_assert(CNA_GAMEPAD_TYPE_BIG_BUTTON_PAD == 9, "CNA BigButtonPad compact identity");

int main(void) {
    /* Three canonical floating-point constants, compared as values because a
       preprocessor comparison of a float expression is not a constant
       expression the C wall above can hold. */
    if (CNA_GAMEPAD_LEFT_DEAD_ZONE != (7849.0f / 32768.0f) ||
        CNA_GAMEPAD_RIGHT_DEAD_ZONE != (8689.0f / 32768.0f) ||
        CNA_GAMEPAD_TRIGGER_THRESHOLD != (30.0f / 255.0f)) {
        return 1;
    }
    /* Only facts the compiler alone can answer are printed. Every count the
       report carries is derived by tools/native_abi/verify.py from the source
       it just compiled, so no number here is a hand-maintained literal. */
    printf("ABI_VERSION=%u\n", (unsigned) CNA_ABI_VERSION);
    printf("ABI_MAJOR=%u\n", (unsigned) CNA_ABI_VERSION_MAJOR);
    printf("ABI_MINOR=%u\n", (unsigned) CNA_ABI_VERSION_MINOR);
    printf("ABI_PATCH=%u\n", (unsigned) CNA_ABI_VERSION_PATCH);
    printf("CNA_BOOL_SIZE=%zu\n", sizeof(CNA_Bool));
    printf("CNA_HANDLE_SIZE=%zu\n", sizeof(CNA_Handle));
    printf("CNA_RESULT_SIZE=%zu\n", sizeof(CNA_Result));
    return 0;
}
