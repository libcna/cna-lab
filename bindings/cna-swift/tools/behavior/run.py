#!/usr/bin/env python3
"""Execute the deterministic pure-XNA-derived Swift behavior corpus."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REFERENCE = ROOT / "tools/api_compat/reference/xna40-windows-runtime-contract.json"

# Foundation 14 pure managed batch. Each entry is a pinned XNA metadata
# closure; the table below is read back out of the pinned contract rather
# than transcribed, so the report cannot drift from the reference.
FOUNDATION_17_BATCH = [
    "Microsoft.Xna.Framework.Audio.AudioStopOptions",
    "Microsoft.Xna.Framework.Input.Touch.GestureType",
    "Microsoft.Xna.Framework.Input.Touch.TouchLocationState",
    "Microsoft.Xna.Framework.Media.VideoSoundtrackType",
]
FOUNDATION_16_BATCH = [
    "Microsoft.Xna.Framework.Audio.MicrophoneState",
    "Microsoft.Xna.Framework.Media.MediaSourceType",
    "Microsoft.Xna.Framework.Media.MediaState",
]
FOUNDATION_14_BATCH = [
    "Microsoft.Xna.Framework.Audio.AudioChannels",
    "Microsoft.Xna.Framework.Audio.SoundState",
    "Microsoft.Xna.Framework.Graphics.Blend",
    "Microsoft.Xna.Framework.Graphics.BlendFunction",
    "Microsoft.Xna.Framework.Graphics.ClearOptions",
    "Microsoft.Xna.Framework.Graphics.ColorWriteChannels",
    "Microsoft.Xna.Framework.Graphics.CompareFunction",
    "Microsoft.Xna.Framework.Graphics.CubeMapFace",
    "Microsoft.Xna.Framework.Graphics.CullMode",
    "Microsoft.Xna.Framework.Graphics.EffectParameterClass",
    "Microsoft.Xna.Framework.Graphics.EffectParameterType",
    "Microsoft.Xna.Framework.Graphics.GraphicsDeviceStatus",
    "Microsoft.Xna.Framework.Graphics.GraphicsProfile",
    "Microsoft.Xna.Framework.Graphics.IndexElementSize",
    "Microsoft.Xna.Framework.Graphics.PresentInterval",
    "Microsoft.Xna.Framework.Graphics.PrimitiveType",
    "Microsoft.Xna.Framework.Graphics.SetDataOptions",
    "Microsoft.Xna.Framework.Graphics.StencilOperation",
    "Microsoft.Xna.Framework.Graphics.TextureAddressMode",
    "Microsoft.Xna.Framework.Graphics.TextureFilter",
    "Microsoft.Xna.Framework.Graphics.VertexElementFormat",
    "Microsoft.Xna.Framework.Graphics.VertexElementUsage",
]
TEST_SOURCES = [
    ROOT / "Tests/CNATests/PureValueTests.swift",
    ROOT / "Tests/CNATests/LinearAlgebraTests.swift",
    ROOT / "Tests/CNATests/GeometryIntersectionTests.swift",
    ROOT / "Tests/CNATests/ColorPackedProtocolTests.swift",
    ROOT / "Tests/CNATests/PackedVectorProtocolTests.swift",
    ROOT / "Tests/CNATests/Packed16BitTests.swift",
    ROOT / "Tests/CNATests/Packed32And64BitTests.swift",
    ROOT / "Tests/CNATests/HalfPackedTests.swift",
    ROOT / "Tests/CNATests/NormalizedPackedTests.swift",
    ROOT / "Tests/CNATests/ShortPackedTests.swift",
    ROOT / "Tests/CNATests/PackedValueSemanticsTests.swift",
    ROOT / "Tests/CNATests/CurveTests.swift",
    ROOT / "Tests/CNATests/GamePadTests.swift",
    ROOT / "Tests/CNATests/DisplayOrientationContractTests.swift",
    ROOT / "Tests/CNATests/BufferUsageContractTests.swift",
    ROOT / "Tests/CNATests/DepthFormatContractTests.swift",
    ROOT / "Tests/CNATests/FillModeContractTests.swift",
    ROOT / "Tests/CNATests/RenderTargetUsageContractTests.swift",
    ROOT / "Tests/CNATests/SurfaceFormatContractTests.swift",
    ROOT / "Tests/CNATests/DisplayModeContractTests.swift",
    ROOT / "Tests/CNATests/Foundation43VertexDeclarationTests.swift",
    ROOT / "Tests/CNATests/Foundation44VertexTypeTests.swift",
    ROOT / "Tests/CNATests/Foundation45DeviceStateTests.swift",
    ROOT / "Tests/CNATests/Foundation14GraphicsEnumContractTests.swift",
    ROOT / "Tests/CNATests/Foundation14ManagedTypeContractTests.swift",
    ROOT / "Tests/CNATests/PresentationParametersContractTests.swift",
    ROOT / "Tests/CNATests/Foundation16ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation17ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation18ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation19ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation20ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation22ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation23ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation24ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation28ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation29ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation30XnaExceptionTests.swift",
    ROOT / "Tests/CNATests/Foundation31XnaLaunchParametersTests.swift",
    ROOT / "Tests/CNATests/Foundation32AttributeTests.swift",
    ROOT / "Tests/CNATests/Foundation36ServiceContainerTests.swift",
    ROOT / "Tests/CNATests/Foundation39GameMembersTests.swift",
    ROOT / "Tests/CNATests/Foundation40GraphicsDeviceServiceTests.swift",
]

# Observations whose authority is the admitted Microsoft .NET Framework 4.0
# `mscorlib`, NOT an XNA assembly. They run in the same suite and are held to
# the same standard, but they are counted separately: folding BCL behaviour
# into `OBSERVATIONS` would relabel it as XNA-derived, which it is not.
BCL_TEST_SOURCES = [
    ROOT / "Tests/CNATests/Foundation27ContractTests.swift",
    ROOT / "Tests/CNATests/Foundation30BclExceptionTests.swift",
    ROOT / "Tests/CNATests/Foundation31BclDictionaryTests.swift",
    ROOT / "Tests/CNATests/Foundation37ExceptionPayloadTests.swift",
    ROOT / "Tests/CNATests/Foundation42ObjectDisposedTests.swift",
]

# `assertProjected` asserts FOUR separately observable facts about one
# projected exception -- its class, its composed Message, its ParamName and its
# HResult -- so a call site is four assertions and not one. Counting it as one
# would have made this corpus appear to shrink while it grew, which is exactly
# what happened the first time these conversions landed.
PROJECTED_ASSERTION_FACTS = 4


def count_assertions(source: str) -> int:
    direct = len(re.findall(r"\bXCTAssert\w*\s*\(", source))
    projected = len(re.findall(r"\bassertProjected\s*\(", source))
    return direct + projected * PROJECTED_ASSERTION_FACTS


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--swift-test", default=None,
                        help="the swift-test executable. Defaults to whichever "
                             "of `swift-test` or `swift test` is on PATH.")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    # `swift-test` is a SwiftPM executable that only exists on PATH once a
    # toolchain is sourced, so defaulting to its bare name made this tool fail
    # with FileNotFoundError anywhere else -- which is how the status gate,
    # invoked without a sourced toolchain, silently regenerated nothing while
    # the committed corpus trailed the suite by nearly two hundred
    # observations. The `swift` driver reaches the same code and is what a
    # plain toolchain install provides.
    if args.swift_test is not None:
        command = [args.swift_test]
    elif shutil.which("swift-test"):
        command = ["swift-test"]
    elif shutil.which("swift"):
        command = ["swift", "test"]
    else:
        print("BEHAVIOR_CORPUS=FAIL neither swift-test nor swift is on PATH")
        return 1

    completed = subprocess.run(
        command + ["--filter", "PureValueTests"],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    source = "\n".join(path.read_text(encoding="utf-8") for path in TEST_SOURCES)
    assertions = count_assertions(source)
    tests = re.findall(r"\bfunc\s+(test\w+)\s*\(", source)
    bcl_source = "\n".join(
        path.read_text(encoding="utf-8") for path in BCL_TEST_SOURCES)
    bcl_assertions = count_assertions(bcl_source)
    bcl_tests = re.findall(r"\bfunc\s+(test\w+)\s*\(", bcl_source)
    failures = 0 if completed.returncode == 0 else 1
    contract = json.loads(REFERENCE.read_text(encoding="utf-8"))
    pinned = {item["name"]: item for item in contract["types"]}
    def enum_contracts(names: list[str]) -> dict[str, dict]:
        return {
            name: {
                "kind": "OptionSet" if pinned[name]["flags"] else "enum",
                "flags": pinned[name]["flags"],
                "underlyingType": pinned[name]["underlyingType"],
                "values": {
                    member["name"]: int(member["value"])
                    for member in pinned[name]["members"]
                    if member["kind"] == "field" and member["name"] != "value__"
                },
                "swiftProjectionQualificationCountedAsXnaBehavior": False,
            }
            for name in names
        }

    batch_contracts = enum_contracts(FOUNDATION_14_BATCH)
    foundation16_contracts = enum_contracts(FOUNDATION_16_BATCH)
    foundation17_contracts = enum_contracts(FOUNDATION_17_BATCH)
    report = {
        "schemaVersion": 1,
        "authority": "PURE_XNA_DERIVED",
        "OBSERVATIONS": assertions,
        "ASSERTIONS": assertions,
        "FAILURES": failures,
        "testCases": tests,
        # The BCL half, on its own axis. Its authority is the admitted
        # Microsoft mscorlib recorded in tools/api_compat/bcl-authorities.json,
        # never an XNA assembly, and it is counted in no XNA total.
        "BCL_AUTHORITY": "PURE_BCL_DERIVED",
        "BCL_OBSERVATIONS": bcl_assertions,
        "BCL_ASSERTIONS": bcl_assertions,
        "bclTestCases": bcl_tests,
        "groupCounts": {
            group: len(re.findall(rf"\bfunc\s+test{pattern}\w*\s*\(", source, re.IGNORECASE))
            for group, pattern in {
                "Vector2": "Vector2", "Vector3": "Vector3", "Vector4": "Vector4",
                "Quaternion": "Quaternion", "Matrix": "Matrix", "Viewport": "Viewport",
                "Plane": "Plane", "Ray": "Ray", "BoundingBox": "BoundingBox",
                "BoundingSphere": "BoundingSphere", "BoundingFrustum": "BoundingFrustum",
                "GeometryEnums": "GeometryEnums",
                "Color": r"Color(?!WriteChannels)",
                "PACKED_ALPHA": "PackedAlpha",
                "PACKED_565_4444_5551": "Packed565_4444_5551",
                "PACKED_BYTE": "PackedByte", "PACKED_HALF": "PackedHalf",
                "PACKED_NORMALIZED_BYTE": "PackedNormalizedByte",
                "PACKED_NORMALIZED_SHORT": "PackedNormalizedShort",
                "PACKED_RG_RGBA": "PackedRgRgba", "PACKED_SHORT": "PackedShort",
                "PACKED_PROTOCOL": "PackedProtocol", "PACKED_EQUALITY": "PackedEquality",
                "PACKED_HASH_STRING": "PackedHash",
                "CURVE_ENUMS": "CurveEnums",
                "CURVE_KEY": "CurveKeyConstructors",
                "CURVE_COLLECTION": "CurveCollection",
                "CURVE_TANGENTS": "CurveDefaultsCloneTangents",
                "CURVE_EVALUATE": "CurveEvaluate",
                "CURVE_LOOPS": "CurveLoops",
                "BUTTON_STATE": "GamePadEnums",
                "BUTTONS": "GamePadEnums",
                "GAMEPAD_BUTTONS": "GamePadButtons",
                "GAMEPAD_DPAD": "GamePadDPad",
                "GAMEPAD_TRIGGERS": "GamePadTriggers",
                "GAMEPAD_THUMBSTICKS": "GamePadThumbSticks",
                "GAMEPAD_STATE": "GamePadState",
                "GAMEPAD_ENUMS": "GamePadEnums",
                "GAMEPAD_CAPABILITIES": "GamePadCapabilities",
                "DISPLAY_ORIENTATION": "DisplayOrientationXnaContract",
                "BUFFER_USAGE": "BufferUsageXnaContract",
                "DEPTH_FORMAT": "DepthFormatXnaContract",
                "FILL_MODE": "FillModeXnaContract",
                "RENDER_TARGET_USAGE": "RenderTargetUsageXnaContract",
                "SURFACE_FORMAT": "SurfaceFormatXnaContract",
                "DISPLAY_MODE_PROPERTIES": "DisplayModeProperties",
                "DISPLAY_MODE_ASPECT_RATIO": "DisplayModeAspectRatio",
                "DISPLAY_MODE_TITLE_SAFE_AREA": "DisplayModeTitleSafeArea",
                "DISPLAY_MODE_TO_STRING": "DisplayModeToString",
                "BLEND": "BlendXnaContract",
                "BLEND_FUNCTION": "BlendFunctionXnaContract",
                "CLEAR_OPTIONS": "ClearOptionsXnaContract",
                "COLOR_WRITE_CHANNELS": "ColorWriteChannelsXnaContract",
                "COMPARE_FUNCTION": "CompareFunctionXnaContract",
                "CUBE_MAP_FACE": "CubeMapFaceXnaContract",
                "CULL_MODE": "CullModeXnaContract",
                "EFFECT_PARAMETER_CLASS": "EffectParameterClassXnaContract",
                "EFFECT_PARAMETER_TYPE": "EffectParameterTypeXnaContract",
                "GRAPHICS_DEVICE_STATUS": "GraphicsDeviceStatusXnaContract",
                "GRAPHICS_PROFILE": "GraphicsProfileXnaContract",
                "INDEX_ELEMENT_SIZE": "IndexElementSizeXnaContract",
                "PRESENT_INTERVAL": "PresentIntervalXnaContract",
                "PRIMITIVE_TYPE": "PrimitiveTypeXnaContract",
                "SET_DATA_OPTIONS": "SetDataOptionsXnaContract",
                "STENCIL_OPERATION": "StencilOperationXnaContract",
                "TEXTURE_ADDRESS_MODE": "TextureAddressModeXnaContract",
                "TEXTURE_FILTER": "TextureFilterXnaContract",
                "VERTEX_ELEMENT_FORMAT": "VertexElementFormatXnaContract",
                "VERTEX_ELEMENT_USAGE": "VertexElementUsageXnaContract",
                "VERTEX_ELEMENT": "VertexElementXnaContract",
                "IEFFECT_FOG": "IEffectFogXnaContract",
                "IEFFECT_MATRICES": "IEffectMatricesXnaContract",
                "SOUND_STATE": "SoundStateXnaContract",
                "AUDIO_CHANNELS": "AudioChannelsXnaContract",
                "PRESENTATION_PARAMETERS_DEFAULTS":
                    "PresentationParametersXnaContractDefaults",
                "PRESENTATION_PARAMETERS_MUTATION":
                    "PresentationParametersXnaContractMutation",
                "PRESENTATION_PARAMETERS_IS_FULL_SCREEN":
                    "PresentationParametersXnaContractIsFullScreen",
                "PRESENTATION_PARAMETERS_BOUNDS":
                    "PresentationParametersXnaContractBounds",
                "PRESENTATION_PARAMETERS_DEVICE_WINDOW_HANDLE":
                    "PresentationParametersXnaContractDeviceWindowHandle",
                "PRESENTATION_PARAMETERS_CLONE":
                    "PresentationParametersXnaContractClone",
                "MOUSE_STATE_CONSTRUCTION":
                    "MouseStateXnaContractConstruction",
                "MOUSE_STATE_EQUALITY": "MouseStateXnaContractEquality",
                "MOUSE_STATE_HASH": "MouseStateXnaContractGetHashCode",
                "MOUSE_STATE_TO_STRING": "MouseStateXnaContractToString",
                "MEDIA_STATE": "MediaStateXnaContract",
                "MEDIA_SOURCE_TYPE": "MediaSourceTypeXnaContract",
                "MICROPHONE_STATE": "MicrophoneStateXnaContract",
                "AUDIO_STOP_OPTIONS": "AudioStopOptionsXnaContract",
                "VIDEO_SOUNDTRACK_TYPE": "VideoSoundtrackTypeXnaContract",
                "TOUCH_LOCATION_STATE": "TouchLocationStateXnaContract",
                "GESTURE_TYPE": "GestureTypeXnaContract",
                "TOUCH_PANEL_CAPABILITIES": "TouchPanelCapabilitiesXnaContract",
                "DISPLAY_MODE_COLLECTION": "DisplayModeCollectionXnaContract",
                "GESTURE_SAMPLE": "GestureSampleXnaContract",
                "TOUCH_LOCATION_CONSTRUCTION":
                    "TouchLocationXnaContractConstruction",
                "TOUCH_LOCATION_PREVIOUS":
                    "TouchLocationXnaContractTryGetPreviousLocation",
                "TOUCH_LOCATION_EQUALITY":
                    "TouchLocationXnaContractEqualityAsymmetry",
                "TOUCH_LOCATION_HASH_AND_STRING":
                    "TouchLocationXnaContractGetHashCodeAndToString",
            }.items()
        },
        "displayOrientationContract": {
            "flags": True,
            "underlyingType": "System.Int32",
            "values": {
                "Default": 0,
                "LandscapeLeft": 1,
                "LandscapeRight": 2,
                "Portrait": 4,
            },
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "bufferUsageContract": {
            "flags": True,
            "underlyingType": "System.Int32",
            "values": {
                "None": 0,
                "WriteOnly": 1,
            },
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "depthFormatContract": {
            "kind": "enum",
            "flags": False,
            "underlyingType": "System.Int32",
            "values": {
                "None": 0,
                "Depth16": 1,
                "Depth24": 2,
                "Depth24Stencil8": 3,
            },
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "fillModeContract": {
            "kind": "enum",
            "flags": False,
            "underlyingType": "System.Int32",
            "values": {
                "Solid": 0,
                "WireFrame": 1,
            },
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "renderTargetUsageContract": {
            "kind": "enum",
            "flags": False,
            "underlyingType": "System.Int32",
            "values": {
                "DiscardContents": 0,
                "PreserveContents": 1,
                "PlatformContents": 2,
            },
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "surfaceFormatContract": {
            "kind": "enum",
            "flags": False,
            "underlyingType": "System.Int32",
            "values": {
                "Color": 0,
                "Bgr565": 1,
                "Bgra5551": 2,
                "Bgra4444": 3,
                "Dxt1": 4,
                "Dxt3": 5,
                "Dxt5": 6,
                "NormalizedByte2": 7,
                "NormalizedByte4": 8,
                "Rgba1010102": 9,
                "Rg32": 10,
                "Rgba64": 11,
                "Alpha8": 12,
                "Single": 13,
                "Vector2": 14,
                "Vector4": 15,
                "HalfSingle": 16,
                "HalfVector2": 17,
                "HalfVector4": 18,
                "HdrBlendable": 19,
            },
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "displayModeContract": {
            "kind": "class",
            "sealed": False,
            "baseType": "System.Object",
            "publicConstructors": 0,
            "nonPublicConstructor":
                "assembly .ctor(int32 width, int32 height, "
                "valuetype Microsoft.Xna.Framework.Graphics.SurfaceFormat format)",
            "publicMembers": [
                "ToString", "Format", "Height", "Width", "AspectRatio",
                "TitleSafeArea",
            ],
            "aspectRatio":
                "if (_height != 0 && _width != 0) "
                "return (float)_width / (float)_height; return 0f;",
            "titleSafeArea":
                "Viewport.GetTitleSafeArea(0, 0, _width, _height) == "
                "new Rectangle(0, 0, _width, _height) on the Windows runtime",
            "toString":
                "string.Format(CultureInfo.CurrentCulture, "
                "\"{{Width:{0} Height:{1} Format:{2} AspectRatio:{3}}}\", "
                "_width, _height, Format, AspectRatio)",
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "foundation14PureManagedBatchContracts": batch_contracts,
        "foundation16PureManagedBatchContracts": foundation16_contracts,
        "foundation17PureManagedBatchContracts": foundation17_contracts,
        "foundation17ReferenceInputs":
            "AudioStopOptions from Microsoft.Xna.Framework.Xact.dll; "
            "GestureType and TouchLocationState from "
            "Microsoft.Xna.Framework.Input.Touch.dll; VideoSoundtrackType from "
            "Microsoft.Xna.Framework.Video.dll; TouchPanelCapabilities from "
            "Microsoft.Xna.Framework.Input.Touch.dll; IGameComponent and "
            "IGraphicsDeviceManager from Microsoft.Xna.Framework.Game.dll. "
            "All five assemblies were registered as authoritative reference "
            "inputs in Foundation 17; see docs/generated/pinned-assembly-audit.json",
        "touchLocationContract": {
            "kind": "struct",
            "sealed": True,
            "layout": "sequential",
            "baseType": "System.ValueType",
            "directInterfaces": [
                "System.IEquatable`1[Microsoft.Xna.Framework.Input.Touch.TouchLocation]",
            ],
            "storage": ["id", "state", "x", "y", "prevState", "prevX", "prevY"],
            "positionStorage":
                "two separate float32 fields; get_Position rebuilds a Vector2 "
                "on every read and nothing stores a Vector2",
            "typedEqualsFields": ["id", "x", "y", "prevX", "prevY"],
            "operatorEqualityFields": [
                "id", "state", "x", "y", "prevState", "prevX", "prevY",
            ],
            "equalityAsymmetry":
                "the typed Equals ignores both state fields while op_Equality "
                "compares all seven; the asymmetry is in the pinned IL",
            "getHashCode":
                "id.GetHashCode() + x.GetHashCode() + y.GetHashCode() with "
                "unchecked int32 addition; Single.GetHashCode() is the raw bit "
                "pattern except that both signed zeroes hash to 0",
            "toStringFormat":
                "String.Format(CultureInfo.CurrentCulture, "
                "\"{{Position:{0}}}\", Position)",
            "tryGetPreviousLocation":
                "absent when prevState is the zero literal Invalid; the out "
                "parameter is still written with id -1 and every other field "
                "zeroed, and a returned previous location never itself has a "
                "previous location",
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "displayModeCollectionContract": {
            "kind": "class",
            "sealed": False,
            "baseType": "System.Object",
            "publicConstructors": 0,
            "publicMembers": ["GetEnumerator", "Item"],
            "itemSemantics":
                "get_Item walks the backing list once and returns a NEW list "
                "of every mode whose Format equals the argument, in original "
                "order; an unmatched format yields an empty result",
            "runtimeCapabilityClaimed": None,
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "mouseStateContract": {
            "kind": "struct",
            "sealed": True,
            "layout": "sequential",
            "baseType": "System.ValueType",
            "directInterfaces": [],
            "storage": [
                "x", "y", "leftButton", "rightButton", "middleButton",
                "xb1", "xb2", "wheel",
            ],
            "publicMembers": [
                ".ctor", "GetHashCode", "ToString", "Equals", "op_Equality",
                "op_Inequality", "X", "Y", "LeftButton", "RightButton",
                "MiddleButton", "XButton1", "XButton2", "ScrollWheelValue",
            ],
            "constructorParameterOrder": [
                "x", "y", "scrollWheel", "leftButton", "middleButton",
                "rightButton", "xButton1", "xButton2",
            ],
            "toStringButtonOrder": [
                "Left", "Right", "Middle", "XButton1", "XButton2",
            ],
            "toStringFormat":
                "String.Format(CultureInfo.CurrentCulture, "
                "\"{{X:{0} Y:{1} Buttons:{2} Wheel:{3}}}\", x, y, buttons, wheel)",
            "getHashCode":
                "plain Int32 XOR over all eight fields; no SmartGetHashCode "
                "helper, so a zero result is returned as zero",
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "vertexElementContract": {
            "kind": "struct",
            "sealed": True,
            "layout": "sequential",
            "baseType": "System.ValueType",
            "directInterfaces": [],
            "storage": ["_offset", "_format", "_usage", "_usageIndex"],
            "publicMembers": [
                ".ctor", "GetHashCode", "ToString", "Equals", "op_Equality",
                "op_Inequality", "Offset", "VertexElementFormat",
                "VertexElementUsage", "UsageIndex",
            ],
            "constructorValidation": "none; all four arguments are stored verbatim",
            "propertyAccessors": "plain field load/store",
            "equality":
                "op_Equality compares _offset, then _usageIndex, then _usage, "
                "then _format; op_Inequality is its negation; Equals(object) is "
                "false for null and for a different runtime type",
            "getHashCode":
                "Helpers.SmartGetHashCode XORs Marshal.SizeOf/4 int32 words of "
                "the pinned box and returns 0x7FFFFFFF when the XOR is zero",
            "toString":
                "string.Format(CultureInfo.CurrentCulture, "
                "\"{{Offset:{0} Format:{1} Usage:{2} UsageIndex:{3}}}\", "
                "Offset, VertexElementFormat, VertexElementUsage, UsageIndex)",
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "effectInterfaceContracts": {
            "Microsoft.Xna.Framework.Graphics.IEffectFog": {
                "kind": "interface",
                "baseInterfaces": [],
                "readWriteProperties": {
                    "FogEnabled": "System.Boolean",
                    "FogStart": "System.Single",
                    "FogEnd": "System.Single",
                    "FogColor": "Microsoft.Xna.Framework.Vector3",
                },
                "methods": [],
            },
            "Microsoft.Xna.Framework.Graphics.IEffectMatrices": {
                "kind": "interface",
                "baseInterfaces": [],
                "readWriteProperties": {
                    "World": "Microsoft.Xna.Framework.Matrix",
                    "View": "Microsoft.Xna.Framework.Matrix",
                    "Projection": "Microsoft.Xna.Framework.Matrix",
                },
                "methods": [],
            },
            "conformerRequired": False,
            "swiftProjectionQualificationCountedAsXnaBehavior": False,
        },
        "foundation14PureManagedBatchProvenance":
            "pinned XNA metadata closure per type; the Swift projection "
            "qualification of each type is a separate language-projection "
            "test and is not counted as XNA runtime behavior",
        "colorPaletteGoldenEntries": len(re.findall(
            r'\("[A-Za-z]+",\s*\.[A-Za-z]+,\s*0x[0-9A-Fa-f_]+\)', source,
        )),
        "floatPolicy": "System.Single maps to Swift Float; asserted results use Float bitPattern where exact bits are selected observations",
        "nativeLibraryRequired": False,
        "exhaustiveSweeps": {
            "Alpha8": 256,
            "Bgr565": 65536,
            "Bgra4444": 65536,
            "Bgra5551": 65536,
            "HalfSingle": 65536,
            "failures": 0 if completed.returncode == 0 else 1,
        },
    }
    rendered = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    else:
        print(rendered, end="")
    print(f"OBSERVATIONS={assertions} ASSERTIONS={assertions} FAILURES={failures}")
    print(f"BCL_OBSERVATIONS={bcl_assertions} BCL_ASSERTIONS={bcl_assertions}")
    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
